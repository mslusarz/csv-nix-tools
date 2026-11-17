/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2026, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

enum { OPT_COUNT = 256 };

static const struct option opts[] = {
	{"columns",	required_argument,	NULL, 'c'},
	{"count",	no_argument,		NULL, OPT_COUNT},
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
	fprintf(out, "      --count                print the number of rows in each run\n");
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
	bool count;
	size_t run_length;

	size_t table_column;
	char *table;
};

static void
print_row(const struct cb_params *params)
{
	if (params->count && !params->table)
		printf("%zu,", params->run_length);

	for (size_t i = 0; i < params->ncols; ++i) {
		const char *value = params->bufs[i].space;
		if (i)
			putchar(',');
		csv_print_quoted(value, strlen(value));
		if (params->count && params->table && i == 0)
			printf(",%zu", params->run_length);
	}
	putchar('\n');
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(ncols);
	struct cb_params *params = arg;
	bool changed = false;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			if (params->count) {
				if (params->run_length)
					print_row(params);
				params->run_length = 0;

				printf("%s,", table);
				if (params->ncols > 1) {
					putchar(',');
					csv_print_line_reordered(stdout, buf, col_offs,
							params->ncols - 1, false,
							params->cols + 1);
				}
				putchar('\n');
			} else {
				csv_print_line_reordered(stdout, buf, col_offs,
						params->ncols, true, params->cols);
			}

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
			/* Emit the pending run before overwriting its first changed field. */
			if (params->count && !changed && params->run_length)
				print_row(params);
			changed = true;

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

	if (params->count)
		params->run_length = changed ? 1 : params->run_length + 1;
	else if (changed)
		print_row(params);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params = { .table_column = SIZE_MAX };
	char *cols = NULL;
	unsigned show_flags = SHOW_DISABLED;

	while ((opt = getopt_long(argc, argv, "c:rsST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case OPT_COUNT:
				params.count = true;
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

	if (!cols || (!*cols && !params.table)) {
		fprintf(stderr, "missing -c option\n");
		usage(stderr);
		exit(2);
	}

	csv_show(show_flags);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	struct split_result *results = NULL;
	size_t nresults;
	util_split_term(cols, ",", &results, &nresults, NULL);
	params.cols = xmalloc_nofail(nresults + (params.table ? nheaders : 0),
			sizeof(params.cols[0]));

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' not found\n", TABLE_COLUMN);
			exit(2);
		}
		params.cols[params.ncols++] = params.table_column;
	}

	for (size_t i = 0; i < nresults; ++i) {
		char *name = cols + results[i].start;

		size_t idx = csv_find_loud(headers, nheaders, params.table,
				name);
		if (idx == CSV_NOT_FOUND)
			exit(2);

		params.cols[params.ncols++] = idx;
	}
	free(cols);
	free(results);

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

	params.bufs = xcalloc_nofail(params.ncols, sizeof(params.bufs[0]));

	char *qualified_count = NULL;
	struct col_header count_header = { "count", "int", true };
	if (params.count) {
		if (params.table) {
			size_t len = strlen(params.table) + sizeof(".count");
			qualified_count = xmalloc_nofail(len, 1);
			snprintf(qualified_count, len, "%s%ccount", params.table,
					TABLE_SEPARATOR);
			count_header.name = qualified_count;
		}

		for (size_t i = 0; i < params.ncols; ++i) {
			if (strcmp(headers[params.cols[i]].name, count_header.name) == 0) {
				fprintf(stderr, "column '%s' already exists\n",
						count_header.name);
				exit(2);
			}
		}

		if (!params.table)
			csv_print_header(stdout, &count_header, ',');
	}

	for (size_t i = 0; i < params.ncols; ++i) {
		if (i)
			putchar(',');
		csv_print_header(stdout, &headers[params.cols[i]], 0);
		if (params.count && params.table && i == 0) {
			putchar(',');
			csv_print_header(stdout, &count_header, 0);
		}
	}
	putchar('\n');
	free(qualified_count);

	csv_read_all_nofail(s, &next_row, &params);
	if (params.count && params.run_length)
		print_row(&params);

	for (size_t i = 0; i < params.ncols; ++i)
		free(params.bufs[i].space);

	free(params.cols);
	free(params.bufs);
	free(params.table);

	csv_destroy_ctx(s);

	return 0;
}
