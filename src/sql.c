/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

struct line {
	char *buf;
	size_t *col_offs;
	struct rpn_variant *order;
};

struct cb_params {
	struct columns columns;
	struct rpn_expression where;
	struct order_conditions order_by;

	struct line *lines;
	size_t size;
	size_t used;
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
	UNUSED(ncols);
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
		if (params->used == params->size) {
			if (params->size == 0)
				params->size = 16;
			else
				params->size *= 2;

			struct line *newlines = xrealloc(params->lines,
					params->size, sizeof(params->lines[0]));
			if (!newlines)
				return -1;

			params->lines = newlines;
		}

		struct line *line = &params->lines[params->used];

		size_t len = col_offs[ncols - 1] +
				strlen(buf + col_offs[ncols - 1]) + 1;

		line->buf = xmalloc(len, 1);
		if (!line->buf)
			return -1;

		size_t col_offs_size = ncols * sizeof(col_offs[0]);
		line->col_offs = xmalloc(col_offs_size, 1);
		if (!line->col_offs) {
			free(line->buf);
			return -1;
		}

		line->order = xmalloc(params->order_by.count, sizeof(line->order[0]));
		if (!line->order) {
			free(line->buf);
			free(line->col_offs);
			return -1;
		}

		memcpy(line->buf, buf, len);
		memcpy(line->col_offs, col_offs, col_offs_size);

		const struct rpn_expression *exp;
		for (size_t i = 0; i < params->order_by.count; ++i) {
			exp = &params->order_by.cond[i].expr;
			if (rpn_eval(exp, buf, col_offs, &line->order[i]))
				exit(2);
		}

		params->used++;

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
print_column_header(size_t i, char sep)
{
	const struct column *col = &Params.columns.col[i];
	if (col->name)
		printf("%s", col->name);
	else
		printf("unnamed%lu", i);

	printf(":%s%c", rpn_expression_type(&col->expr, Headers), sep);
}

struct sort_params {
	const struct line *lines;
	size_t nlines;

	const struct order_conditions *order_by;
};

int
cmp(const void *p1, const void *p2, void *arg)
{
	struct sort_params *params = arg;
	size_t idx1 = *(const size_t *)p1;
	size_t idx2 = *(const size_t *)p2;

	const struct line *line1 = &params->lines[idx1];
	const struct line *line2 = &params->lines[idx2];

	for (size_t i = 0; i < params->order_by->count; ++i) {
		bool asc = params->order_by->cond[i].asc;

		const struct rpn_variant *v1 = &line1->order[i];
		const struct rpn_variant *v2 = &line2->order[i];

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

// TODO unicode/locale
//	setlocale(LC_ALL, "");
//	setlocale(LC_NUMERIC, "C");

	while ((opt = getopt_long(argc, argv, "sS", opts, NULL)) != -1) {
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

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

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

	struct columns *columns = &Params.columns;
	for (size_t i = 0; i < columns->count - 1; ++i)
		print_column_header(i, ',');

	print_column_header(columns->count - 1, '\n');

	if (columns->count < 1)
		abort();

	csv_read_all_nofail(s, &next_row, &Params);

	if (Params.order_by.count) {
		struct sort_params sort_params;
		sort_params.lines = Params.lines;
		sort_params.nlines = Params.used;
		sort_params.order_by = &Params.order_by;

		size_t *row_idx = xmalloc_nofail(Params.used, sizeof(row_idx[0]));

		for (size_t i = 0; i < Params.used; ++i)
			row_idx[i] = i;

		csv_qsort_r(row_idx, Params.used, sizeof(row_idx[0]), cmp, &sort_params);

		for (size_t i = 0; i < Params.used; ++i) {
			struct line *line = &Params.lines[row_idx[i]];
			print_row(&Params.columns, line->buf, line->col_offs);
		}

		free(row_idx);
	}

	csv_destroy_ctx(s);

	if (Params.order_by.count) {
		for (size_t i = 0; i < Params.used; ++i) {
			struct line *line = &Params.lines[i];
			free(line->buf);
			free(line->col_offs);
			for (size_t j = 0; j < Params.order_by.count; ++j)
				if (line->order[j].type == RPN_PCHAR)
					free(line->order[j].pchar);
			free(line->order);
		}

		for (size_t i = 0; i < Params.order_by.count; ++i)
			rpn_free(&Params.order_by.cond[i].expr);
		free(Params.order_by.cond);
		free(Params.lines);
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
