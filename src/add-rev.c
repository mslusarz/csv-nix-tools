/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <getopt.h>
#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"table",		required_argument,	NULL, 'T'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-add-rev [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print it back to standard output with\n"
"a new column produced by reversing another column characterwise.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c NAME                    use column NAME as an input\n");
	fprintf(out, "  -n NEW-NAME                create column NEW-NAME as an output\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t col;

	size_t table_column;
	char *table;

	wchar_t *wcs;
	size_t wcs_size;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, ncols, false);

			putchar(',');
			putchar('\n');

			return 0;
		}
	}

	csv_print_line(stdout, buf, col_offs, ncols, false);
	fputc(',', stdout);

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	if (str[0] == '"') {
		unquoted = csv_unquot(str);
		fputc('"', stdout);
	}

	size_t len = mbstowcs(NULL, unquoted, 0);

	if (len == (size_t)-1) {
		len = strlen(unquoted);
		for (size_t i = 0; i < len; ++i)
			fputc(unquoted[len - i - 1], stdout);
	} else {
		if (len + 1 > params->wcs_size) {
			free(params->wcs);
			params->wcs_size = 2 * len + 1;
			params->wcs = xmalloc_nofail(params->wcs_size,
					sizeof(params->wcs[0]));
		}

		size_t converted = mbstowcs(params->wcs, unquoted, len);
		assert (converted == len);
		(void) converted;

		for (size_t i = 0; i < len; ++i)
			fprintf(stdout, "%lc", params->wcs[len - i - 1]);
	}

	if (str[0] == '"') {
		fputc('"', stdout);
		free((char *)unquoted);
	}

	fputc('\n', stdout);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	char *input_col = NULL;
	char *new_name = NULL;
	struct cb_params params;
	unsigned show_flags = SHOW_DISABLED;

	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	params.table = NULL;
	params.table_column = SIZE_MAX;
	params.wcs = NULL;
	params.wcs_size = 0;

	while ((opt = getopt_long(argc, argv, "c:n:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				input_col = xstrdup_nofail(optarg);
				break;
			case 'n':
				new_name = xstrdup_nofail(optarg);
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

	if (!input_col || !new_name) {
		usage(stderr);
		exit(2);
	}

	csv_show(show_flags);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	csv_column_doesnt_exist(headers, nheaders, params.table, new_name);

	params.col = csv_find_loud(headers, nheaders, params.table, input_col);
	if (params.col == CSV_NOT_FOUND)
		exit(2);

	if (strcmp(headers[params.col].type, "string") != 0) {
		fprintf(stderr, "column '%s' is not a string\n", input_col);
		exit(2);
	}

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	free(input_col);
	input_col = NULL;

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);

	if (params.table)
		printf("%s.", params.table);
	printf("%s:string\n", new_name);

	free(new_name);
	new_name = NULL;

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	free(params.table);
	free(params.wcs);

	return 0;
}
