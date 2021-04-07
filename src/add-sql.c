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
#include "sql.h"
#include "sql-shared.h"
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
	fprintf(out,
"Read CSV stream from standard input and print it back to standard output with\n"
"a new column produced by evaluation of SQL expression.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -e SQL-EXPR                use expression SQL-EXPR to create new column;\n"
"                             SQL expressions use space as a separator, so this\n"
"                             needs to be quoted\n");
	fprintf(out, "  -n NEW-NAME                create column NEW-NAME as an output\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	struct columns columns;

	size_t table_column;
	char *table;
};

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

	struct columns *columns = &Params.columns;
	columns->col = xrealloc_nofail(columns->col,
			columns->count + 1,
			sizeof(columns->col[0]));
	struct column *col = &columns->col[columns->count];

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

	columns->count++;
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

	struct columns *columns = &Params.columns;
	columns->col = xrealloc_nofail(columns->col,
			columns->count + 1,
			sizeof(columns->col[0]));

	struct column *col = &columns->col[columns->count];
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

	columns->count++;
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
		char sep)
{
	struct rpn_variant ret;

	if (rpn_eval(exp, buf, col_offs, &ret))
		exit(2);

	if (ret.type == RPN_LLONG) {
		printf("%lld%c", ret.llong, sep);
	} else if (ret.type == RPN_PCHAR) {
		csv_print_quoted(ret.pchar, strlen(ret.pchar));
		fputc(sep, stdout);
		free(ret.pchar);
	} else if (ret.type == RPN_DOUBLE) {
		printf("%f%c", ret.dbl, sep);
	} else {
		fprintf(stderr, "unknown type %d\n", ret.type);
		exit(2);
	}
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;
	struct rpn_expression *exp;

	struct columns *columns = &Params.columns;
	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, ncols, false);

			for (size_t i = 0; i < columns->count; ++i)
				putchar(',');

			putchar('\n');

			return 0;
		}
	}

	csv_print_line(stdout, buf, col_offs, ncols, false);
	fputc(',', stdout);

	for (size_t i = 0; i < columns->count - 1; ++i) {
		exp = &columns->col[i].expr;

		process_exp(exp, buf, col_offs, ',');
	}

	exp = &columns->col[columns->count - 1].expr;

	process_exp(exp, buf, col_offs, '\n');

	return 0;
}

static void
describe_column(struct column *column, char sep, bool any_str_column_had_type)
{
	const char *type = rpn_expression_type(&column->expr, Headers);
	if (any_str_column_had_type || strcmp(type, "string") != 0)
		printf("%s:%s%c", column->name, type, sep);
	else
		printf("%s%c", column->name, sep);
}

int
main(int argc, char *argv[])
{
	int opt;
	size_t nexpressions = 0;
	char **expressions = NULL;
	char **names = NULL;
	unsigned show_flags = SHOW_DISABLED;
	char *new_column = NULL;

// TODO unicode/locale
//	setlocale(LC_ALL, "");
//	setlocale(LC_NUMERIC, "C");

	Params.columns.col = NULL;
	Params.columns.count = 0;
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
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
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

	csv_show(show_flags);

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

	bool any_str_column_had_type = false;
	for (size_t i = 0; i < Nheaders; ++i) {
		csv_print_header(stdout, &Headers[i], ',');

		if (any_str_column_had_type)
			continue;

		if (Headers[i].had_type && strcmp(Headers[i].type, "string") == 0)
			any_str_column_had_type = true;
	}

	struct columns *columns = &Params.columns;
	for (size_t i = 0; i < columns->count - 1; ++i)
		describe_column(&columns->col[i], ',', any_str_column_had_type);
	describe_column(&columns->col[columns->count - 1], '\n', any_str_column_had_type);

	csv_read_all_nofail(s, &next_row, &Params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < columns->count; ++i) {
		rpn_free(&columns->col[i].expr);
		free(columns->col[i].name);
	}

	free(columns->col);
	free(Params.table);
	rpn_fini();

	return 0;
}
