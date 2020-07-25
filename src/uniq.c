/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"columns",	required_argument,	NULL, 'c'},
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
	fprintf(out, "Usage: csv-uniq [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input, remove adjacent duplicate rows and\n"
"print back to standard output the remaining rows.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -c, --columns=NAME1[,NAME2...]\n"
"                             use these columns\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	struct buf {
		char *space;
		size_t sz;
	} *bufs;

	size_t *cols;
	size_t ncols;

	size_t table_column;
	char *table;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(ncols);
	struct cb_params *params = arg;
	bool print = false;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line_reordered(stdout, buf, col_offs,
					params->ncols, true, params->cols);

			return 0;
		}
	}

	for (size_t i = 0; i < params->ncols; ++i) {
		const char *col = &buf[col_offs[params->cols[i]]];

		const char *unquoted = col;

		if (col[0] == '"')
			unquoted = csv_unquot(col);

		if (params->bufs[i].sz == 0 ||
				strcmp(params->bufs[i].space, unquoted) != 0) {
			print = true;

			size_t len = strlen(unquoted);
			if (len + 1 > params->bufs[i].sz) {
				free(params->bufs[i].space);
				params->bufs[i].space =
						xmalloc_nofail(len + 1, 1);
				params->bufs[i].sz = len + 1;
			}

			memcpy(params->bufs[i].space, unquoted, len + 1);
		}

		if (col[0] == '"')
			free((char *)unquoted);
	}

	if (print) {
		struct buf *b;
		for (size_t i = 0; i < params->ncols - 1; ++i) {
			b = &params->bufs[i];
			csv_print_quoted(b->space, strlen(b->space));
			putchar(',');
		}

		b = &params->bufs[params->ncols - 1];

		csv_print_quoted(b->space, strlen(b->space));
		putchar('\n');
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	char *cols = NULL;
	bool show = false;
	bool show_full;

	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:rsST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
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

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.cols = xmalloc_nofail(nheaders, sizeof(params.cols[0]));
	params.ncols = 0;
	params.bufs = NULL;

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
		params.cols[params.ncols++] = params.table_column;
	}

	char *name = strtok(cols, ",");
	while (name) {
		size_t idx = csv_find_loud(headers, nheaders, params.table,
				name);
		if (idx == CSV_NOT_FOUND)
			exit(2);

		params.cols[params.ncols++] = idx;

		name = strtok(NULL, ",");
	}
	free(cols);

	if (params.table) {
		size_t table_len = strlen(params.table);

		for (size_t i = 0; i < nheaders; ++i) {
			const char *hname = headers[i].name;

			/* skip columns from the same table */
			if (strncmp(params.table, hname, table_len) == 0 &&
					hname[table_len] == TABLE_SEPARATOR)
				continue;

			/* skip column with the table name */
			if (i == params.cols[0])
				continue;

			params.cols[params.ncols++] = i;
		}
	}

	params.cols = xrealloc_nofail(params.cols, params.ncols,
			sizeof(params.cols[0]));
	params.bufs = xcalloc_nofail(params.ncols, sizeof(params.bufs[0]));

	for (size_t i = 0; i < params.ncols - 1; ++i) {
		printf("%s:%s,", headers[params.cols[i]].name,
				headers[params.cols[i]].type);
	}

	printf("%s:%s\n", headers[params.cols[params.ncols - 1]].name,
			headers[params.cols[params.ncols - 1]].type);

	csv_read_all_nofail(s, &next_row, &params);

	for (size_t i = 0; i < params.ncols; ++i)
		free(params.bufs[i].space);

	free(params.cols);
	free(params.bufs);
	free(params.table);

	csv_destroy_ctx(s);

	return 0;
}
