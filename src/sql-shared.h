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
			struct rpn_expression *expr = &Columns->col[found].expr;
			for (size_t i = 0; i < expr->count; ++i)
				sql_stack_push(&expr->tokens[i]);
			free(str);
			return;
		}
	}

	struct rpn_token tk;

	tk.type = RPN_COLUMN;
	tk.colnum = csv_find_loud(Headers, Nheaders, Table, str);

	if (tk.colnum == CSV_NOT_FOUND)
		exit(2);

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

