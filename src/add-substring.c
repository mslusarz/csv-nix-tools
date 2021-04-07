/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

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
	fprintf(out, "Usage: csv-add-substring [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print it back to standard output with\n"
"a new column produced by extracting substring of another column.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c NAME                    use column NAME as an input data\n");
	fprintf(out, "  -n NEW-NAME                create column NEW-NAME as an output\n");
	fprintf(out,
"  -p START-POS               start from position START-POS;\n"
"                             first character has position 1; negative value\n"
"                             mean starting from the end of string\n");
	fprintf(out,
"  -l LENGTH                  take LENGTH characters from string; must not be\n"
"                             negative\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t col;
	ssize_t start_pos;
	size_t length;

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

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	if (str[0] == '"')
		unquoted = csv_unquot(str);

	ssize_t start = params->start_pos;
	size_t len = params->length;
	csv_substring_sanitize(unquoted, &start, &len);
	csv_print_quoted(unquoted + (size_t)start, len);

	fputc('\n', stdout);

	if (str[0] == '"')
		free((char *)unquoted);

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

	memset(&params, 0, sizeof(params));
	params.length = SIZE_MAX;
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:l:n:p:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				input_col = xstrdup_nofail(optarg);
				break;
			case 'l':
				if (strtoul_safe(optarg, &params.length, 0))
					exit(2);
				break;
			case 'n':
				new_name = xstrdup_nofail(optarg);
				break;
			case 'p':
				if (strtol_safe(optarg, &params.start_pos, 0))
					exit(2);
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

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	free(input_col);
	input_col = NULL;

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

	free(new_name);
	new_name = NULL;

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	free(params.table);

	return 0;
}
