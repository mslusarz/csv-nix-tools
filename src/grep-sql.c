/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "sql.h"
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
	fprintf(out, "Usage: csv-grep-sql [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input, compute SQL expression for each row\n"
"and print back to standard output rows for which expression is non-zero.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -e SQL-EXPR                use expression SQL-EXPR to filter;\n"
"                             SQL expressions use space as a separator,\n"
"                             so this needs to be quoted\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	struct rpn_expression *expressions;
	size_t count;

	char *table;
	size_t table_column;
};

#include "sql-shared.h"

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

	for (size_t i = 0; i < params->count; ++i) {
		struct rpn_expression *exp = &params->expressions[i];
		struct rpn_variant ret;

		if (rpn_eval(exp, buf, col_offs, &ret))
			exit(2);

		if (ret.type != RPN_LLONG) /* shouldn't be possible */
			abort();

		if (ret.llong) {
			csv_print_line(stdout, buf, col_offs, ncols, true);
			return 0;
		}
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	size_t nexpressions = 0;
	char **expressions = NULL;
	struct cb_params params;
	unsigned show_flags = SHOW_DISABLED;

// TODO unicode/locale
//	setlocale(LC_ALL, "");
//	setlocale(LC_NUMERIC, "C");

	params.expressions = NULL;
	params.count = 0;
	params.table = NULL;

	while ((opt = getopt_long(argc, argv, "e:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'e': {
				expressions = xrealloc_nofail(expressions,
						nexpressions + 1,
						sizeof(expressions[0]));

				expressions[nexpressions++] =
						xstrdup_nofail(optarg);

				break;
			}
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

	if (nexpressions == 0) {
		usage(stderr);
		exit(2);
	}

	csv_show(show_flags);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	Nheaders = csv_get_headers(s, &Headers);

	if (params.table) {
		params.table_column = csv_find(Headers, Nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}

		Table = params.table;
	}

	params.count = nexpressions;
	params.expressions = xcalloc_nofail(nexpressions, sizeof(params.expressions[0]));

	for (size_t i = 0; i < nexpressions; ++i) {
		char *expstr = expressions[i];

		FILE *in = fmemopen(expstr, strlen(expstr), "r");
		if (!in) {
			perror("fmemopen");
			exit(2);
		}

		yyin = in;

		yyparse();
		yylex_destroy();

		fclose(in);

		struct rpn_expression *exp = &params.expressions[i];

		exp->tokens = Tokens;
		exp->count = Ntokens;

		Tokens = NULL;
		Ntokens = 0;

		if (strcmp(rpn_expression_type(exp, Headers), "int") != 0) {
			fprintf(stderr, "expression %ld is not numeric\n", i);
			exit(2);
		}
	}
	Table = NULL;

	for (size_t i = 0; i < nexpressions; ++i)
		free(expressions[i]);
	free(expressions);

	csv_print_header(stdout, Headers, Nheaders);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < nexpressions; ++i)
		rpn_free(&params.expressions[i]);
	free(params.expressions);
	free(params.table);
	rpn_fini();

	return 0;
}
