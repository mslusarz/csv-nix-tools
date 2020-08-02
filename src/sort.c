/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"columns",	required_argument,	NULL, 'c'},
	{"reverse",	no_argument,		NULL, 'r'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"table",	required_argument,	NULL, 'T'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-sort [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input, sort it by chosen column and print\n"
"resulting file to standard output.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -c, --columns=NAME1[,NAME2...]\n"
"                             sort first by column NAME1, then NAME2, etc.\n");
	fprintf(out, "  -r, --reverse              sort in descending order\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct line {
	char *buf;
	size_t *col_offs;
};

struct cb_params {
	struct line *lines;
	size_t size;
	size_t used;

	size_t table_column;
	char *table;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, ncols, true);

			return 0;
		}
	}

	if (params->used == params->size) {
		if (params->size == 0)
			params->size = 16;
		else
			params->size *= 2;

		struct line *newlines = xrealloc(params->lines,
				params->size, sizeof(params->lines[0]));
		if (!newlines)
			return -1;

		params->lines = newlines;
	}

	struct line *line = &params->lines[params->used];

	size_t len = col_offs[ncols - 1] +
			strlen(buf + col_offs[ncols - 1]) + 1;

	line->buf = xmalloc(len, 1);
	if (!line->buf)
		return -1;

	size_t col_offs_size = ncols * sizeof(col_offs[0]);
	line->col_offs = xmalloc(col_offs_size, 1);
	if (!line->col_offs) {
		free(line->buf);
		return -1;
	}

	memcpy(line->buf, buf, len);
	memcpy(line->col_offs, col_offs, col_offs_size);
	params->used++;

	return 0;
}

struct sort_params {
	const struct col_header *headers;

	size_t *columns;
	size_t ncolumns;

	struct line *lines;
	size_t nlines;
};

int
cmp(const void *p1, const void *p2, void *arg)
{
	struct sort_params *params = arg;
	size_t idx1 = *(const size_t *)p1;
	size_t idx2 = *(const size_t *)p2;
	const struct col_header *headers = params->headers;
	struct line *lines = params->lines;

	for (size_t i = 0; i < params->ncolumns; ++i) {
		size_t col = params->columns[i];
		const struct line *line1 = &lines[idx1];
		const struct line *line2 = &lines[idx2];

		const char *val1 = &line1->buf[line1->col_offs[col]];
		const char *val2 = &line2->buf[line2->col_offs[col]];

		/* if they are equal by text then they are equal in any type */
		if (strcmp(val1, val2) == 0)
			continue;

		if (strcmp(headers[col].type, "int") != 0)
			return strcmp(val1, val2);

		/* handle integers */
		long long llval1, llval2;

		if (strtoll_safe(val1, &llval1, 0))
			exit(2);

		if (strtoll_safe(val2, &llval2, 0))
			exit(2);

		if (llval1 < llval2)
			return -1;

		if (llval1 > llval2)
			return 1;

		return 0;
	}

	return 0;
}

static void
print_line(struct line *line, size_t ncols)
{
	csv_print_line(stdout, line->buf, line->col_offs, ncols, true);

	free(line->buf);
	free(line->col_offs);
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool reverse = false;
	params.lines = NULL;
	params.size = 0;
	params.used = 0;
	char *cols = NULL;
	struct sort_params sort_params;
	unsigned show_flags = SHOW_DISABLED;

	sort_params.columns = NULL;
	sort_params.ncolumns = 0;
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:rsST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'r':
				reverse = true;
				break;
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'T':
				params.table = xstrdup_nofail(optarg);
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (!cols) {
		fprintf(stderr, "missing -c option\n");
		usage(stderr);
		exit(2);
	}

	csv_show(show_flags);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	sort_params.columns = xmalloc_nofail(nheaders, sizeof(sort_params.columns[0]));

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	struct split_result *results = NULL;
	size_t nresults;
	util_split_term(cols, ",", &results, &nresults, NULL);

	for (size_t i = 0; i < nresults; ++i) {
		char *name = cols + results[i].start;

		size_t idx =
			csv_find_loud(headers, nheaders, params.table, name);
		if (idx == CSV_NOT_FOUND)
			exit(2);

		if (sort_params.ncolumns == nheaders) {
			fprintf(stderr, "duplicated columns\n");
			exit(2);
		}

		sort_params.columns[sort_params.ncolumns++] = idx;
	}
	free(cols);
	free(results);

	csv_print_header(stdout, headers, nheaders);

	csv_read_all_nofail(s, &next_row, &params);

	size_t *row_idx = xmalloc_nofail(params.used, sizeof(row_idx[0]));

	for (size_t i = 0; i < params.used; ++i)
		row_idx[i] = i;

	sort_params.headers = headers;
	sort_params.lines = params.lines;
	sort_params.nlines = params.used;
	csv_qsort_r(row_idx, params.used, sizeof(row_idx[0]), cmp, &sort_params);

	if (reverse) {
		for (size_t i = params.used; i > 0; --i)
			print_line(&params.lines[row_idx[i - 1]], nheaders);
	} else {
		for (size_t i = 0; i < params.used; ++i)
			print_line(&params.lines[row_idx[i]], nheaders);
	}

	free(row_idx);
	free(params.lines);
	free(params.table);
	free(sort_params.columns);

	csv_destroy_ctx(s);

	return 0;
}
