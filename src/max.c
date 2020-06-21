/*
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
	fprintf(out, "Usage: csv-max [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print back to standard output maximum\n"
"values of chosen columns.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -c, --columns=NAME1[,NAME2...]\n"
"                             use these columns\n");
	fprintf(out,
"  -n NEW-NAME1[,NEW-NAME2...]\n"
"                             create columns with these names, instead\n"
"                             of default max(NAME)\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t *cols;
	enum output_types *types;
	size_t ncols;

	bool *active_cols;

	long long *max_int;
	char **max_str;
	size_t *str_size;

	size_t table_column;
	char *table;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(ncols);
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line_reordered(stdout, buf, col_offs,
					params->ncols, true, params->cols);

			return 0;
		}
	}

	for (size_t i = 0; i < params->ncols; ++i) {
		if (!params->active_cols[i])
			continue;

		const char *val = &buf[col_offs[params->cols[i]]];

		if (params->types[i] == TYPE_INT) {
			long long llval;
			if (strtoll_safe(val, &llval, 0))
				return -1;

			if (llval > params->max_int[i])
				params->max_int[i] = llval;

		} else {
			const char *unquoted = val;
			if (val[0] == '"')
				unquoted = csv_unquot(val);

			if (params->max_str[i] == NULL ||
					strcmp(unquoted, params->max_str[i]) > 0) {
				size_t len = strlen(unquoted);
				if (len + 1 > params->str_size[i]) {
					free(params->max_str[i]);
					params->max_str[i] = xmalloc_nofail(len + 1, 1);
					params->str_size[i] = len + 1;
				}
				memcpy(params->max_str[i], unquoted, len + 1);
			}

			if (val[0] == '"')
				free((char *)unquoted);
		}
	}

	return 0;
}

static void
type_not_supported(const char *type, const char *col)
{
	fprintf(stderr,
		"Type '%s', used by column '%s', is not supported by csv-max.\n",
		type, col);
	exit(2);
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	char *cols = NULL;
	char *new_name = NULL;
	bool show = false;
	bool show_full;
	size_t table_len;

	params.cols = NULL;
	params.types = NULL;
	params.ncols = 0;
	params.active_cols = NULL;
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:n:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'n':
				new_name = xstrdup_nofail(optarg);
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
				table_len = strlen(params.table);
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
	params.types = xmalloc_nofail(nheaders, sizeof(params.types[0]));
	params.active_cols = xcalloc_nofail(nheaders,
			sizeof(params.active_cols[0]));

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
		params.cols[params.ncols++] = params.table_column;
	}

	char *col = strtok(cols, ",");
	while (col) {
		size_t idx =
			csv_find_loud(headers, nheaders, params.table, col);
		if (idx == CSV_NOT_FOUND)
			exit(2);

		if (params.ncols == nheaders) {
			fprintf(stderr, "duplicated columns\n");
			exit(2);
		}

		const char *t = headers[idx].type;
		if (strcmp(t, "int") == 0)
			params.types[params.ncols] = TYPE_INT;
		else if (strcmp(t, "string") == 0)
			params.types[params.ncols] = TYPE_STRING;
		else
			type_not_supported(t, col);

		params.active_cols[params.ncols] = true;
		params.cols[params.ncols++] = idx;

		col = strtok(NULL, ",");
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

	params.max_int = xmalloc_nofail(params.ncols,
			sizeof(params.max_int[0]));
	params.max_str = xmalloc_nofail(params.ncols,
			sizeof(params.max_str[0]));
	params.str_size = xmalloc_nofail(params.ncols,
			sizeof(params.str_size[0]));
	for (size_t i = 0; i < params.ncols; ++i) {
		params.max_int[i] = LLONG_MIN;
		params.max_str[i] = NULL;
		params.str_size[i] = 0;
	}

	char *name;
	if (new_name)
		name = strtok(new_name, ",");
	else
		name = NULL;
	for (size_t i = 0; i < params.ncols - 1; ++i) {
		if (csv_print_table_func_header(&headers[params.cols[i]], "max",
				params.table, table_len, ',', name)) {
			if (name)
				name = strtok(NULL, ",");
		}
	}
	csv_print_table_func_header(&headers[params.cols[params.ncols - 1]],
			"max", params.table, table_len, '\n', name);

	free(new_name);
	new_name = NULL;

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	size_t start_idx = 0;
	if (params.table) {
		printf("%s,", params.table);
		start_idx = 1;
	}

	for (size_t i = start_idx; i < params.ncols - 1; ++i) {
		if (!params.active_cols[i]) {
			putchar(',');
		} else if (params.types[i] == TYPE_INT) {
			printf("%lld,", params.max_int[i]);
		} else {
			csv_print_quoted(params.max_str[i],
					strlen(params.max_str[i]));
			putchar(',');
		}
	}

	size_t idx = params.ncols - 1;
	if (params.active_cols[idx]) {
		if (params.types[idx] == TYPE_INT)
			printf("%lld", params.max_int[idx]);
		else
			csv_print_quoted(params.max_str[idx],
					strlen(params.max_str[idx]));
	}

	putchar('\n');

	free(params.active_cols);
	free(params.cols);
	free(params.types);
	free(params.max_int);
	for (size_t i = 0; i < params.ncols; ++i)
		free(params.max_str[i]);
	free(params.max_str);
	free(params.str_size);
	free(params.table);

	return 0;
}
