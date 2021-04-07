/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <getopt.h>
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
	fprintf(out, "Usage: csv-add-concat [OPTION]... -- new_name = [%%name|str]...\n");
	fprintf(out,
"Read CSV stream from standard input and print it back to standard output with\n"
"a new column produced by concatenation of columns and fixed strings.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct elem {
	bool is_column;
	union {
		size_t colnum;
		char *str;
	};
};

struct cb_params {
	struct elem *elements;
	size_t count;

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
			csv_print_line(stdout, buf, col_offs, ncols, false);

			putchar(',');
			putchar('\n');

			return 0;
		}
	}

	csv_print_line(stdout, buf, col_offs, ncols, false);
	fputc(',', stdout);

	size_t len = 0;
	for (size_t i = 0; i < params->count; ++i) {
		if (params->elements[i].is_column)
			/* ignore quoting for now, we need upper bound */
			len += strlen(&buf[col_offs[params->elements[i].colnum]]);
		else
			len += strlen(params->elements[i].str);
	}

	char *outbuf = xmalloc_nofail(len + 1, 1);

	size_t start = 0;
	for (size_t i = 0; i < params->count; ++i) {
		if (params->elements[i].is_column) {
			const char *str = &buf[col_offs[params->elements[i].colnum]];
			const char *unquoted = str;
			if (str[0] == '"')
				unquoted = csv_unquot(str);

			len = strlen(unquoted);
			memcpy(outbuf + start, unquoted, len);

			if (str[0] == '"')
				free((char *)unquoted);

		} else {
			len = strlen(params->elements[i].str);
			memcpy(outbuf + start, params->elements[i].str, len);
		}

		start += len;
	}

	outbuf[start] = 0;

	csv_print_quoted(outbuf, start);

	fputc('\n', stdout);

	free(outbuf);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	char *new_name = NULL;
	struct cb_params params;
	unsigned show_flags = SHOW_DISABLED;

	memset(&params, 0, sizeof(params));
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "sST:", opts, NULL)) != -1) {
		switch (opt) {
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
			case 0:
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (argc - optind < 3) {
		usage(stderr);
		exit(2);
	}

	new_name = argv[optind++];

	if (strcmp(argv[optind++], "=") != 0) {
		usage(stderr);
		exit(2);
	}

	assert(optind <= argc);
	params.count = (size_t)(argc - optind);
	params.elements = xmalloc_nofail(params.count, sizeof(params.elements[0]));

	size_t i = 0;
	while (optind < argc) {
		params.elements[i].is_column = argv[optind][0] == '%';
		params.elements[i].str = argv[optind];

		if (params.elements[i].is_column)
			params.elements[i].str++;
		optind++;
		i++;
	}

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	csv_column_doesnt_exist(headers, nheaders, params.table, new_name);

	for (size_t i = 0; i < params.count; ++i) {
		if (!params.elements[i].is_column)
			continue;

		size_t idx = csv_find_loud(headers, nheaders, params.table,
				params.elements[i].str);
		if (idx == CSV_NOT_FOUND)
			exit(2);

		params.elements[i].colnum = idx;
	}

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	csv_show(show_flags);

	bool any_str_column_had_type = false;
	for (size_t i = 0; i < nheaders; ++i) {
		csv_print_header(stdout, &headers[i], ',');

		if (any_str_column_had_type)
			continue;

		if (headers[i].had_type && strcmp(headers[i].type, "string") == 0)
			any_str_column_had_type = true;
	}

	if (params.table)
		printf("%s.", params.table);

	if (any_str_column_had_type)
		printf("%s:string\n", new_name);
	else
		printf("%s\n", new_name);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	free(params.elements);
	free(params.table);

	return 0;
}
