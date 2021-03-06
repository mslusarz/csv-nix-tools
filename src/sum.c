/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"separator",	required_argument,	NULL, 'e'},
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
	fprintf(out, "Usage: csv-sum [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print back to standard output the sum of\n"
"values of chosen columns.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -c, --columns=NAME1[,NAME2...]\n"
"                             use these columns\n");
	fprintf(out, "  -e, --separator=STR        put STR between (string) values\n");
	fprintf(out,
"  -n NEW-NAME1[,NEW-NAME2...]\n"
"                             create columns with these names, instead\n"
"                             of default sum(NAME)\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t *cols;
	enum data_type *types;
	size_t ncols;

	bool *active_cols;

	long long *sums_int;
	char **sums_str;
	double *sums_dbl;
	size_t *str_used;
	size_t *str_sizes;
	const char *sep;
	size_t sep_len;

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

			if (llval > 0 && params->sums_int[i] > LLONG_MAX - llval) {
				fprintf(stderr, "integer overflow\n");
				return -1;
			}

			if (llval < 0 && params->sums_int[i] < LLONG_MIN - llval) {
				fprintf(stderr, "integer underflow\n");
				return -1;
			}

			params->sums_int[i] += llval;
		} else if (params->types[i] == TYPE_FLOAT) {
			double dbl;
			if (strtod_safe(val, &dbl))
				return -1;

			params->sums_dbl[i] += dbl;
		} else {
			const char *unquoted = val;
			if (val[0] == '"')
				unquoted = csv_unquot(val);

			size_t len = strlen(unquoted);

			size_t req = params->sep_len + len + 1;
			if (req > params->str_sizes[i] - params->str_used[i]) {
				params->str_sizes[i] *= 2;
				if (params->str_sizes[i] - params->str_used[i] < req)
					params->str_sizes[i] += req;

				params->sums_str[i] =
					xrealloc_nofail(params->sums_str[i],
							params->str_sizes[i], 1);
			}

			if (params->str_used[i] > 0 && params->sep_len) {
				strcpy(&params->sums_str[i][params->str_used[i]],
						params->sep);
				params->str_used[i] += params->sep_len;
			}

			memcpy(&params->sums_str[i][params->str_used[i]],
					unquoted, len + 1);
			params->str_used[i] += len;

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
		"Type '%s', used by column '%s', is not supported by csv-sum.\n",
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
	unsigned show_flags = SHOW_DISABLED;
	char *sep = NULL;
	size_t table_len = 0;

// TODO unicode/locale
//	setlocale(LC_ALL, "");
//	setlocale(LC_NUMERIC, "C");

	params.cols = NULL;
	params.types = NULL;
	params.ncols = 0;
	params.active_cols = NULL;
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:e:n:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'e':
				sep = xstrdup_nofail(optarg);
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

	csv_show(show_flags);

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

	struct split_result *results = NULL;
	size_t results_max_size = 0;
	size_t nresults;
	util_split_term(cols, ",", &results, &nresults, &results_max_size);

	for (size_t i = 0; i < nresults; ++i) {
		char *col = cols + results[i].start;

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
		else if (strcmp(t, "float") == 0)
			params.types[params.ncols] = TYPE_FLOAT;
		else
			type_not_supported(t, col);

		params.active_cols[params.ncols] = true;
		params.cols[params.ncols++] = idx;
	}

	free(cols);

	if (params.table) {
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

	params.sums_int = xcalloc_nofail(params.ncols,
			sizeof(params.sums_int[0]));
	params.sums_str = xcalloc_nofail(params.ncols,
			sizeof(params.sums_str[0]));
	params.str_used = xcalloc_nofail(params.ncols,
			sizeof(params.str_used[0]));
	params.str_sizes = xcalloc_nofail(params.ncols,
			sizeof(params.str_sizes[0]));
	params.sums_dbl = xmalloc_nofail(params.ncols,
			sizeof(params.sums_dbl[0]));
	params.sep = sep;
	if (sep)
		params.sep_len = strlen(sep);
	else
		params.sep_len = 0;

	for (size_t i = 0; i < params.ncols; ++i)
		params.sums_dbl[i] = 0.0;

	char *name;
	size_t consumed = 0;

	if (new_name) {
		util_split_term(new_name, ",", &results, &nresults, &results_max_size);
		assert(nresults > 0);
		name = new_name + results[consumed].start;
		consumed++;
	} else {
		name = NULL;
		nresults = 0;
	}

	for (size_t i = 0; i < params.ncols - 1; ++i) {
		if (csv_print_table_func_header(&headers[params.cols[i]], "sum",
				params.table, table_len, ',', name)) {
			if (consumed < nresults) {
				name = new_name + results[consumed].start;
				consumed++;
			} else {
				name = NULL;
			}
		}
	}
	csv_print_table_func_header(&headers[params.cols[params.ncols - 1]],
			"sum", params.table, table_len, '\n', name);

	free(new_name);
	new_name = NULL;

	free(results);
	results = NULL;

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
			printf("%lld,", params.sums_int[i]);
		} else if (params.types[i] == TYPE_FLOAT) {
			printf("%f,", params.sums_dbl[i]);
		} else {
			csv_print_quoted(params.sums_str[i],
					params.str_used[i]);
			putchar(',');
		}
	}

	size_t idx = params.ncols - 1;
	if (params.active_cols[idx]) {
		if (params.types[idx] == TYPE_INT)
			printf("%lld", params.sums_int[idx]);
		else if (params.types[idx] == TYPE_FLOAT)
			printf("%f", params.sums_dbl[idx]);
		else
			csv_print_quoted(params.sums_str[idx],
					params.str_used[idx]);
	}

	putchar('\n');

	free(params.active_cols);
	free(params.cols);
	free(params.types);
	free(sep);
	free(params.sums_int);
	for (size_t i = 0; i < params.ncols; ++i)
		free(params.sums_str[i]);
	free(params.sums_str);
	free(params.str_used);
	free(params.str_sizes);
	free(params.sums_dbl);
	free(params.table);

	return 0;
}
