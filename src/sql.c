/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{"no-types",		no_argument,		NULL, 'X'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-sql [OPTION] sql-query\n");
	fprintf(out,
"Read CSV stream from standard input, process it using simplified SQL query\n"
"and print back to standard output its result.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_no_types_in(out);
	describe_help(out);
	describe_version(out);
}

struct order_condition {
	struct rpn_expression expr;
	bool asc;
};

struct order_conditions {
	struct order_condition *cond;
	size_t count;
};

struct cb_params {
	struct columns columns;
	struct rpn_expression where;
	struct order_conditions order_by;

	struct lines lines;
};

static void
process_exp(const struct rpn_expression *exp, const char *buf,
		const size_t *col_offs, char sep)
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

static void
print_row(const struct columns *columns, const char *buf, const size_t *col_offs)
{
	const struct rpn_expression *exp;
	for (size_t i = 0; i < columns->count - 1; ++i) {
		exp = &columns->col[i].expr;

		process_exp(exp, buf, col_offs, ',');
	}

	exp = &columns->col[columns->count - 1].expr;

	process_exp(exp, buf, col_offs, '\n');
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;
	struct rpn_variant ret;

	if (params->where.count) {
		if (rpn_eval(&params->where, buf, col_offs, &ret))
			exit(2);

		if (ret.type != RPN_LLONG) /* shouldn't be possible - XXX? */
			abort();

		if (ret.llong == 0)
			return 0;
	}

	if (params->order_by.count) {
		struct lines *lines = &params->lines;
		int ret = lines_add(lines, buf, col_offs, ncols);
		if (ret)
			return ret;
		struct line *line = &lines->data[lines->used - 1];

		struct rpn_variant *order = line->user =
			xmalloc(params->order_by.count, sizeof(struct rpn_variant));
		if (!order) {
			lines_free_one(line);
			lines->used--;
			return -1;
		}

		const struct rpn_expression *exp;
		for (size_t i = 0; i < params->order_by.count; ++i) {
			exp = &params->order_by.cond[i].expr;
			if (rpn_eval(exp, buf, col_offs, &order[i]))
				exit(2);
		}

		return 0;
	}

	print_row(&params->columns, buf, col_offs);

	return 0;
}

static struct cb_params Params;

void
sql_column_done(void)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	struct columns *columns = &Params.columns;
	columns->col = xrealloc_nofail(columns->col,
			columns->count + 1,
			sizeof(columns->col[0]));
	struct column *col = &columns->col[columns->count];

	col->expr = exp;
	col->name = NULL;

	if (Ntokens == 1 && Tokens[0].type == RPN_COLUMN)
		col->name = xstrdup_nofail(Headers[Tokens[0].col.num].name);

	columns->count++;
	Tokens = NULL;
	Ntokens = 0;
}

void
sql_named_column_done(char *name)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	struct columns *columns = &Params.columns;
	columns->col = xrealloc_nofail(columns->col,
			columns->count + 1,
			sizeof(columns->col[0]));

	struct column *col = &columns->col[columns->count];
	col->expr = exp;
	col->name = name;

	columns->count++;
	Tokens = NULL;
	Ntokens = 0;
}

void
sql_all_columns(void)
{
	struct columns *columns = &Params.columns;
	columns->col = xmalloc_nofail(Nheaders, sizeof(columns->col[0]));
	columns->count = Nheaders;

	for (size_t i = 0; i < Nheaders; ++i) {
		struct column *col = &columns->col[i];
		col->name = xstrdup_nofail(Headers[i].name);
		col->expr.count = 1;
		col->expr.tokens = xmalloc_nofail(1, sizeof(col->expr.tokens[0]));
		col->expr.tokens[0].type = RPN_COLUMN;
		col->expr.tokens[0].col.num = i;
		col->expr.tokens[0].col.type = Headers[i].type;
	}
}

void
sql_from(char *name)
{
	if (strcmp(name, "input") != 0) {
		fprintf(stderr, "unknown table '%s'\n", name);
		exit(2);
	}

	free(name);
}

void
sql_where()
{
	if (Ntokens > 0) {
		fprintf(stderr, "invalid state\n");
		exit(2);
	}
}

void
sql_end_of_conditions()
{
	Params.where.tokens = Tokens;
	Params.where.count = Ntokens;

	Tokens = NULL;
	Ntokens = 0;
}

void
sql_order_by()
{
	if (Ntokens > 0) {
		fprintf(stderr, "invalid state\n");
		exit(2);
	}
}

void
sql_order_by_expr_done(bool asc)
{
	struct rpn_expression exp;
	exp.tokens = Tokens;
	exp.count = Ntokens;

	struct order_conditions *conds = &Params.order_by;
	conds->cond = xrealloc_nofail(conds->cond,
			conds->count + 1,
			sizeof(conds->cond[0]));
	struct order_condition *cond = &conds->cond[conds->count];

	cond->expr = exp;
	cond->asc = asc;

	conds->count++;
	Tokens = NULL;
	Ntokens = 0;
}

static void
print_column_header(size_t i, char sep, bool any_str_column_had_type)
{
	const struct column *col = &Params.columns.col[i];
	if (col->name)
		printf("%s", col->name);
	else
		printf("unnamed%lu", i);

	const char *type = rpn_expression_type(&col->expr, Headers);
	if (any_str_column_had_type || strcmp(type, "string") != 0)
		printf(":%s", type);

	putchar(sep);
}

struct sort_params {
	const struct lines *lines;

	const struct order_conditions *order_by;
};

int
cmp(const void *p1, const void *p2, void *arg)
{
	struct sort_params *params = arg;
	size_t idx1 = *(const size_t *)p1;
	size_t idx2 = *(const size_t *)p2;

	const struct line *line1 = &params->lines->data[idx1];
	const struct line *line2 = &params->lines->data[idx2];

	for (size_t i = 0; i < params->order_by->count; ++i) {
		bool asc = params->order_by->cond[i].asc;
		const struct rpn_variant *order1 = line1->user;
		const struct rpn_variant *order2 = line2->user;

		const struct rpn_variant *v1 = &order1[i];
		const struct rpn_variant *v2 = &order2[i];

		assert(v1->type == v2->type);
		bool less;
		if (v1->type == RPN_LLONG) {
			if (v1->llong == v2->llong)
				continue;
			less = v1->llong < v2->llong;
		} else if (v1->type == RPN_PCHAR) {
			int c = strcmp(v1->pchar, v2->pchar);
			if (c == 0)
				continue;
			less = c < 0;
		} else if (v1->type == RPN_DOUBLE) {
			if (v1->dbl == v2->dbl)
				continue;
			less = v1->dbl < v2->dbl;
		} else {
			assert(!"unhandled type");
			abort();
		}

		if (less == asc)
			return -1;
		else
			return 1;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	unsigned show_flags = SHOW_DISABLED;
	bool types = true;

// TODO unicode/locale
//	setlocale(LC_ALL, "");
//	setlocale(LC_NUMERIC, "C");

	while ((opt = getopt_long(argc, argv, "sSX", opts, NULL)) != -1) {
		switch (opt) {
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'X':
				types = false;
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (optind != argc - 1) {
		usage(stderr);
		exit(2);
	}

	csv_show(show_flags);

	lines_init(&Params.lines);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr, types);

	csv_read_header_nofail(s);

	Nheaders = csv_get_headers(s, &Headers);
	Columns = &Params.columns;

	FILE *in = fmemopen(argv[optind], strlen(argv[optind]), "r");
	if (!in) {
		perror("fmemopen");
		exit(2);
	}

	yyin = in;

	yyparse();
	yylex_destroy();

	fclose(in);

	bool any_str_column_had_type = false;
	for (size_t i = 0; i < Nheaders; ++i) {
		if (Headers[i].had_type && strcmp(Headers[i].type, "string") == 0) {
			any_str_column_had_type = true;
			break;
		}
	}

	struct columns *columns = &Params.columns;
	for (size_t i = 0; i < columns->count - 1; ++i)
		print_column_header(i, ',', any_str_column_had_type);

	print_column_header(columns->count - 1, '\n', any_str_column_had_type);

	if (columns->count < 1)
		abort();

	csv_read_all_nofail(s, &next_row, &Params);

	struct lines *lines = &Params.lines;
	if (Params.order_by.count) {
		struct sort_params sort_params;
		sort_params.lines = lines;
		sort_params.order_by = &Params.order_by;

		size_t *row_idx = xmalloc_nofail(lines->used, sizeof(row_idx[0]));

		for (size_t i = 0; i < lines->used; ++i)
			row_idx[i] = i;

		csv_qsort_r(row_idx, lines->used, sizeof(row_idx[0]), cmp, &sort_params);

		for (size_t i = 0; i < lines->used; ++i) {
			struct line *line = &lines->data[row_idx[i]];
			print_row(&Params.columns, line->buf, line->col_offs);
		}

		free(row_idx);
	}

	csv_destroy_ctx(s);

	if (Params.order_by.count) {
		for (size_t i = 0; i < lines->used; ++i) {
			struct line *line = &lines->data[i];
			struct rpn_variant *order = line->user;

			for (size_t j = 0; j < Params.order_by.count; ++j)
				if (order[j].type == RPN_PCHAR)
					free(order[j].pchar);
			free(order);

			lines_free_one(line);
		}

		for (size_t i = 0; i < Params.order_by.count; ++i)
			rpn_free(&Params.order_by.cond[i].expr);
		free(Params.order_by.cond);
		lines_fini(&Params.lines);
	}

	for (size_t i = 0; i < columns->count; ++i) {
		rpn_free(&columns->col[i].expr);
		free(columns->col[i].name);
	}

	free(columns->col);
	rpn_free(&Params.where);
	rpn_fini();

	return 0;
}
