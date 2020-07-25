/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <stddef.h>
#include <stdlib.h>
#include "parse.h"
#include "utils.h"

struct column {
	struct rpn_expression expr;
	char *name;
};

struct columns {
	struct column *col;
	size_t count;
};

static const struct col_header *Headers;
static size_t Nheaders;
static struct rpn_token *Tokens;
static size_t Ntokens;
static char *Table;
static struct columns *Columns;

void
sql_stack_push(const struct rpn_token *token)
{
	Tokens = xrealloc_nofail(Tokens, Ntokens + 1, sizeof(Tokens[0]));

	Tokens[Ntokens++] = *token;
}

void
sql_stack_push_string(char *str)
{
	if (Columns) {
		size_t found = SIZE_MAX;
		for (size_t i = 0; i < Columns->count; ++i) {
			if (strcmp(Columns->col[i].name, str) == 0) {
				found = i;
				break;
			}
		}

		if (found != SIZE_MAX) {
			const struct rpn_expression *expr = &Columns->col[found].expr;
			for (size_t i = 0; i < expr->count; ++i)
				sql_stack_push(&expr->tokens[i]);
			free(str);
			return;
		}
	}

	struct rpn_token tk;

	tk.type = RPN_COLUMN;
	tk.col.num = csv_find_loud(Headers, Nheaders, Table, str);

	if (tk.col.num == CSV_NOT_FOUND)
		exit(2);
	tk.col.type = Headers[tk.col.num].type;

	sql_stack_push(&tk);

	free(str);
}

void
sql_stack_push_llong(long long l)
{
	struct rpn_token tk;

	tk.type = RPN_CONSTANT;
	tk.constant.type = RPN_LLONG;
	tk.constant.llong = l;

	sql_stack_push(&tk);
}

void
sql_stack_push_literal(char *str)
{
	struct rpn_token tk;

	tk.type = RPN_CONSTANT;
	tk.constant.type = RPN_PCHAR;
	tk.constant.pchar = str;

	sql_stack_push(&tk);
}

void
sql_stack_push_op(enum rpn_operator op)
{
	struct rpn_token tk;
	tk.type = RPN_OPERATOR;
	tk.operator = op;

	sql_stack_push(&tk);
}

