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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

struct str_tokens {
	char *in_it;
};

static void
token_init(struct str_tokens *ctx, char *str)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->in_it = str;
}

static char *
token_next(struct str_tokens *ctx)
{
	while (ctx->in_it[0] == ' ')
		ctx->in_it++;

	if (ctx->in_it[0] == 0)
		return NULL;

	char *ret = ctx->in_it;

	if (*ret == '\'') {
		char *in_it = ret + 1;
		char *out_it = ret + 1;

		while (1) {
			while (*in_it != 0 && *in_it != '\'')
				*out_it++ = *in_it++;
			if (*in_it == 0) {
				fprintf(stderr, "unfinished string\n");
				exit(2);
			}

			assert(*in_it == '\'');

			// quoting?
			if (in_it[1] != '\'')
				break;

			// copy '
			*out_it++ = *in_it++;

			// and skip the next one
			in_it++;
		}

		// copy terminating '
		*out_it++ = *in_it++;

		if (in_it[0] == 0) {
			*out_it++ = 0;
			ctx->in_it = in_it;
		} else if (in_it[0] == ' ') {
			*out_it++ = 0;
			ctx->in_it = ++in_it;
		} else {
			fprintf(stderr,
				"unrecognized character after end of string: '%c'\n",
				in_it[0]);
			exit(2);
		}
	} else {
		char *space = index(ctx->in_it, ' ');
		if (space) {
			*space = 0;
			ctx->in_it = space + 1;
		} else {
			ctx->in_it += strlen(ctx->in_it);
		}
	}

	return ret;
}

int
rpn_parse(struct rpn_expression *exp, char *str,
		const struct col_header *headers, size_t nheaders,
		const char *table)
{
	struct str_tokens ctx;
	token_init(&ctx, str);
	char *token = token_next(&ctx);

	while (token) {
		struct rpn_token tkn;

		if (token[0] == '%' && token[1] != 0) {
			tkn.type = RPN_COLUMN;
			tkn.colnum = csv_find_loud(headers, nheaders, table,
					token + 1);
			if (tkn.colnum == CSV_NOT_FOUND)
				goto fail;
		} else if (isdigit(token[0])) {
			tkn.type = RPN_CONSTANT;
			tkn.constant.type = RPN_LLONG;
			int base = 0;

			if (token[0] == '0' && token[1] == 'b') {
				/* 0b-0 is not a valid binary number */
				if (token[2] == '-') {
					fprintf(stderr,
						"value '%s' is not an integer\n",
						token);
					goto fail;
				}

				base = 2;
				token += 2;
			}

			if (strtoll_safe(token, &tkn.constant.llong, base))
				goto fail;
		} else if (token[0] == '-' && isdigit(token[1])) {
			tkn.type = RPN_CONSTANT;
			tkn.constant.type = RPN_LLONG;

			if (token[1] == '0' && token[2] == 'b') {
				/* -0b-0 is not a valid binary number */
				if (token[3] == '-') {
					fprintf(stderr,
						"value '%s' is not an integer\n",
						token);
					goto fail;
				}

				if (strtoll_safe(token + 3, &tkn.constant.llong, 2))
					goto fail;
				tkn.constant.llong = -tkn.constant.llong;
			} else {
				if (strtoll_safe(token, &tkn.constant.llong, 0))
					goto fail;
			}
		} else if (token[0] == '\'') {
			tkn.type = RPN_CONSTANT;
			tkn.constant.type = RPN_PCHAR;
			size_t len = strlen(token);
			tkn.constant.pchar = strndup(token + 1, len - 2);
			if (!tkn.constant.pchar) {
				perror("strdup");
				goto fail;
			}
		} else {
			tkn.type = RPN_OPERATOR;
			if (strcmp(token, "+") == 0)
				tkn.operator = RPN_ADD;
			else if (strcmp(token, "-") == 0)
				tkn.operator = RPN_SUB;
			else if (strcmp(token, "*") == 0)
				tkn.operator = RPN_MUL;
			else if (strcmp(token, "/") == 0)
				tkn.operator = RPN_DIV;
			else if (strcmp(token, "%") == 0)
				tkn.operator = RPN_MOD;
			else if (strcmp(token, "|") == 0)
				tkn.operator = RPN_BIT_OR;
			else if (strcmp(token, "&") == 0)
				tkn.operator = RPN_BIT_AND;
			else if (strcmp(token, "~") == 0)
				tkn.operator = RPN_BIT_NEG;
			else if (strcmp(token, "^") == 0)
				tkn.operator = RPN_BIT_XOR;
			else if (strcmp(token, "<<") == 0)
				tkn.operator = RPN_BIT_LSHIFT;
			else if (strcmp(token, ">>") == 0)
				tkn.operator = RPN_BIT_RSHIFT;
			else if (strcmp(token, "<") == 0)
				tkn.operator = RPN_LT;
			else if (strcmp(token, "<=") == 0)
				tkn.operator = RPN_LE;
			else if (strcmp(token, ">") == 0)
				tkn.operator = RPN_GT;
			else if (strcmp(token, ">=") == 0)
				tkn.operator = RPN_GE;
			else if (strcmp(token, "==") == 0)
				tkn.operator = RPN_EQ;
			else if (strcmp(token, "!=") == 0)
				tkn.operator = RPN_NE;
			else if (strcmp(token, "substr") == 0 ||
					strcmp(token, "substring") == 0)
				tkn.operator = RPN_SUBSTR;
			else if (strcmp(token, "strlen") == 0)
				tkn.operator = RPN_STRLEN;
			else if (strcmp(token, "length") == 0)
				tkn.operator = RPN_STRLEN;
			else if (strcmp(token, "concat") == 0)
				tkn.operator = RPN_CONCAT;
			else if (strcmp(token, "like") == 0)
				tkn.operator = RPN_LIKE;
			else if (strcmp(token, "tostring") == 0)
				tkn.operator = RPN_TOSTRING;
			else if (strcmp(token, "toint") == 0)
				tkn.operator = RPN_TOINT;
			else if (strcmp(token, "lt") == 0)
				tkn.operator = RPN_LT;
			else if (strcmp(token, "le") == 0)
				tkn.operator = RPN_LE;
			else if (strcmp(token, "gt") == 0)
				tkn.operator = RPN_GT;
			else if (strcmp(token, "ge") == 0)
				tkn.operator = RPN_GE;
			else if (strcmp(token, "eq") == 0)
				tkn.operator = RPN_EQ;
			else if (strcmp(token, "ne") == 0)
				tkn.operator = RPN_NE;
			else if (strcmp(token, "and") == 0)
				tkn.operator = RPN_LOGIC_AND;
			else if (strcmp(token, "or") == 0)
				tkn.operator = RPN_LOGIC_OR;
			else if (strcmp(token, "xor") == 0)
				tkn.operator = RPN_LOGIC_XOR;
			else if (strcmp(token, "not") == 0)
				tkn.operator = RPN_LOGIC_NOT;
			else if (strcmp(token, "if") == 0)
				tkn.operator = RPN_IF;
			else {
				fprintf(stderr, "unknown operator '%s'\n",
						token);
				goto fail;
			}
		}

		exp->tokens = xrealloc(exp->tokens, exp->count + 1,
				sizeof(exp->tokens[0]));
		if (!exp->tokens)
			goto fail;

		exp->tokens[exp->count++] = tkn;

		token = token_next(&ctx);
	}

	return 0;

fail:
	free(exp->tokens);
	memset(exp, 0, sizeof(*exp));
	return -1;
}

static const char *
expression_type(const struct rpn_expression *exp,
		const struct col_header *headers,
		size_t fallback_idx)
{
	const struct rpn_token *last_token = &exp->tokens[fallback_idx];

	switch(last_token->type) {
		case RPN_OPERATOR:
			switch (last_token->operator) {
				case RPN_ADD:
				case RPN_SUB:
				case RPN_MUL:
				case RPN_DIV:
				case RPN_MOD:
				case RPN_BIT_OR:
				case RPN_BIT_AND:
				case RPN_BIT_XOR:
				case RPN_BIT_NEG:
				case RPN_BIT_LSHIFT:
				case RPN_BIT_RSHIFT:
				case RPN_LOGIC_AND:
				case RPN_LOGIC_OR:
				case RPN_LOGIC_XOR:
				case RPN_LOGIC_NOT:
				case RPN_TOINT:
				case RPN_STRLEN:
				case RPN_LIKE:
				case RPN_LT:
				case RPN_LE:
				case RPN_GT:
				case RPN_GE:
				case RPN_EQ:
				case RPN_NE:
					return "int";

				case RPN_SUBSTR:
				case RPN_CONCAT:
				case RPN_TOSTRING:
					return "string";
				case RPN_IF:
					return expression_type(exp, headers,
							fallback_idx - 1);
			}
			break;
		case RPN_CONSTANT:
			switch (last_token->constant.type) {
				case RPN_LLONG:
					return "int";
				case RPN_PCHAR:
					return "string";
			}
			break;
		case RPN_COLUMN:
			return headers[last_token->colnum].type;
	}

	abort();

}

const char *
rpn_expression_type(const struct rpn_expression *exp,
		const struct col_header *headers)
{
	return expression_type(exp, headers, exp->count - 1);
}

void
rpn_free(struct rpn_expression *exp)
{
	for (size_t i = 0; i < exp->count; ++i) {
		struct rpn_token *token = &exp->tokens[i];

		if (token->type == RPN_CONSTANT &&
				token->constant.type == RPN_PCHAR)
			free(token->constant.pchar);
	}
	free(exp->tokens);
	exp->tokens = NULL;
}
