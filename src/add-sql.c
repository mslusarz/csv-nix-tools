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

#include <assert.h>
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
	fprintf(out, "Usage: csv-add-sql [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -n new_column_name\n");
	fprintf(out, "  -e SQL_expression\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct column {
	struct rpn_expression expr;
	char *name;
};

struct cb_params {
	struct column *columns;
	size_t columns_count;

	size_t table_column;
	char *table;
};

#include "sql-shared.h"

static struct cb_params Params;
static char *New_name;

void
sql_column_done(void)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	if (!New_name) {
		fprintf(stderr,
			"No column name. Either specify it using -n parameter or \"expression AS name\" syntax.\n");
		exit(2);
	}

	csv_column_doesnt_exist(Headers, Nheaders, Table, New_name);

	Params.columns = xrealloc_nofail(Params.columns,
			Params.columns_count + 1,
			sizeof(Params.columns[0]));
	struct column *col = &Params.columns[Params.columns_count];

	col->expr = exp;
	if (Table) {
		if (csv_asprintf(&col->name, "%s.%s", Table, New_name) < 0) {
			perror("asprintf");
			exit(2);
		}
		free(New_name);
	} else {
		col->name = New_name;
	}

	Params.columns_count++;
	Tokens = NULL;
	Ntokens = 0;
	New_name = NULL;
}

void
sql_named_column_done(char *name)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	if (New_name) {
		fprintf(stderr,
			"Column name conflict (-n \"%s\" vs AS \"%s\").\n",
			New_name, name);
		exit(2);
	}

	csv_column_doesnt_exist(Headers, Nheaders, Table, name);

	Params.columns = xrealloc_nofail(Params.columns,
			Params.columns_count + 1,
			sizeof(Params.columns[0]));

	struct column *col = &Params.columns[Params.columns_count];
	col->expr = exp;
	if (Table) {
		if (csv_asprintf(&col->name, "%s.%s", Table, name) < 0) {
			perror("asprintf");
			exit(2);
		}
		free(name);
	} else {
		col->name = name;
	}

	Params.columns_count++;
	Tokens = NULL;
	Ntokens = 0;
}

void
sql_all_columns(void)
{
	fprintf(stderr, "* is forbidden in csv-add-sql\n");
	exit(2);
}

static void
process_exp(struct rpn_expression *exp, const char *buf, const size_t *col_offs,
		const struct col_header *headers, char sep)
{
	struct rpn_variant ret;

	if (rpn_eval(exp, buf, col_offs, headers, &ret))
		exit(2);

	if (ret.type == RPN_LLONG)
		printf("%lld%c", ret.llong, sep);
	else if (ret.type == RPN_PCHAR) {
		csv_print_quoted(ret.pchar, strlen(ret.pchar));
		fputc(sep, stdout);
		free(ret.pchar);
	} else {
		fprintf(stderr, "unknown type %d\n", ret.type);
		exit(2);
	}
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	struct rpn_expression *exp;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, nheaders, false);

			for (size_t i = 0; i < params->columns_count; ++i)
				putchar(',');

			putchar('\n');

			return 0;
		}
	}

	csv_print_line(stdout, buf, col_offs, nheaders, false);
	fputc(',', stdout);

	for (size_t i = 0; i < params->columns_count - 1; ++i) {
		exp = &params->columns[i].expr;

		process_exp(exp, buf, col_offs, headers, ',');
	}

	exp = &params->columns[params->columns_count - 1].expr;

	process_exp(exp, buf, col_offs, headers, '\n');

	return 0;
}

static void
describe_column(struct column *column, char sep)
{
	printf("%s:%s%c",
			column->name,
			rpn_expression_type(&column->expr, Headers),
			sep);
}

int
main(int argc, char *argv[])
{
	int opt;
	size_t nexpressions = 0;
	char **expressions = NULL;
	char **names = NULL;
	bool show = false;
	bool show_full;
	char *new_column = NULL;

	Params.columns = NULL;
	Params.columns_count = 0;
	Params.table = NULL;
	Params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "e:n:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'e': {
				names = xrealloc_nofail(names, nexpressions + 1,
						sizeof(names[0]));
				names[nexpressions] = new_column;
				new_column = NULL;

				expressions = xrealloc_nofail(expressions,
						nexpressions + 1,
						sizeof(expressions[0]));
				expressions[nexpressions++] =
						xstrdup_nofail(optarg);

				break;
			}
			case 'n':
				free(new_column);
				new_column = xstrdup_nofail(optarg);
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
				Params.table = xstrdup_nofail(optarg);
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

	free(new_column);

	if (nexpressions == 0) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	Nheaders = csv_get_headers(s, &Headers);

	if (Params.table) {
		Params.table_column = csv_find(Headers, Nheaders, TABLE_COLUMN);
		if (Params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	Table = Params.table;
	for (size_t i = 0; i < nexpressions; ++i) {
		FILE *in = fmemopen(expressions[i], strlen(expressions[i]), "r");
		if (!in) {
			perror("fmemopen");
			exit(2);
		}

		New_name = names[i];
		yyin = in;

		yyparse();
		yylex_destroy();

		fclose(in);

		assert(New_name == NULL);
	}
	Table = NULL;

	for (size_t i = 0; i < nexpressions; ++i)
		free(expressions[i]);

	free(expressions);
	free(names);

	for (size_t i = 0; i < Nheaders; ++i)
		printf("%s:%s,", Headers[i].name, Headers[i].type);

	for (size_t i = 0; i < Params.columns_count - 1; ++i)
		describe_column(&Params.columns[i], ',');
	describe_column(&Params.columns[Params.columns_count - 1], '\n');

	csv_read_all_nofail(s, &next_row, &Params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < Params.columns_count; ++i) {
		rpn_free(&Params.columns[i].expr);
		free(Params.columns[i].name);
	}

	free(Params.columns);
	free(Params.table);
	rpn_fini();

	return 0;
}
