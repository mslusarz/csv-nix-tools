/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
	while (isspace(ctx->in_it[0]))
		ctx->in_it++;

	if (ctx->in_it[0] == 0)
		return NULL;

	char *ret = ctx->in_it;

	if (*ret == '\'' || *ret == '"') {
		char term = *ret;
		char *in_it = ret + 1;
		char *out_it = ret + 1;

		while (1) {
			while (*in_it != 0 && *in_it != term)
				*out_it++ = *in_it++;
			if (*in_it == 0) {
				fprintf(stderr, "unfinished string\n");
				exit(2);
			}

			assert(*in_it == term);

			// quoting?
			if (in_it[1] != term)
				break;

			// copy quoted term character
			*out_it++ = *in_it++;

			// and skip the next one
			in_it++;
		}

		// copy term
		*out_it++ = *in_it++;

		if (in_it[0] == 0) {
			*out_it++ = 0;
			ctx->in_it = in_it;
		} else if (isspace(in_it[0])) {
			*out_it++ = 0;
			ctx->in_it = ++in_it;
		} else {
			fprintf(stderr,
				"unrecognized character after end of string: '%c'\n",
				in_it[0]);
			exit(2);
		}
	} else {
		char *cur = ctx->in_it;
		while (*cur && !isspace(*cur))
			cur++;

		if (isspace(*cur)) {
			*cur = 0;
			ctx->in_it = cur + 1;
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
			tkn.col.num = csv_find_loud(headers, nheaders, table,
					token + 1);
			if (tkn.col.num == CSV_NOT_FOUND)
				goto fail;
			tkn.col.type = headers[tkn.col.num].type;
		} else if (isdigit(token[0])) {
			tkn.type = RPN_CONSTANT;

			if (token[0] == '0' && token[1] == 'b') {
				/* 0b-0 is not a valid binary number */
				if (token[2] == '-') {
					fprintf(stderr,
						"value '%s' is not an integer\n",
						token);
					goto fail;
				}

				token += 2;

				tkn.constant.type = RPN_LLONG;
				if (strtoll_safe(token, &tkn.constant.llong, 2))
					goto fail;
			} else {
				if (index(token, '.') || index(token, 'e')) {
					tkn.constant.type = RPN_DOUBLE;
					if (strtod_safe(token, &tkn.constant.dbl))
						goto fail;
				} else {
					tkn.constant.type = RPN_LLONG;
					if (strtoll_safe(token, &tkn.constant.llong, 0))
						goto fail;
				}
			}
		} else if (token[0] == '-' && isdigit(token[1])) {
			tkn.type = RPN_CONSTANT;

			if (token[1] == '0' && token[2] == 'b') {
				/* -0b-0 is not a valid binary number */
				if (token[3] == '-') {
					fprintf(stderr,
						"value '%s' is not an integer\n",
						token);
					goto fail;
				}

				tkn.constant.type = RPN_LLONG;
				if (strtoll_safe(token + 3, &tkn.constant.llong, 2))
					goto fail;
				tkn.constant.llong = -tkn.constant.llong;
			} else {
				if (index(token, '.') || index(token, 'e')) {
					tkn.constant.type = RPN_DOUBLE;
					if (strtod_safe(token, &tkn.constant.dbl))
						goto fail;
				} else {
					tkn.constant.type = RPN_LLONG;
					if (strtoll_safe(token, &tkn.constant.llong, 0))
						goto fail;
				}
			}
		} else if (token[0] == '\'' || token[0] == '"') {
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
			else if (strcmp(token, "tofloat") == 0)
				tkn.operator = RPN_TOFLOAT;
			else if (strcmp(token, "int2str") == 0)
				tkn.operator = RPN_INT2STR;
			else if (strcmp(token, "int2strb") == 0)
				tkn.operator = RPN_INT2STRB;
			else if (strcmp(token, "int2flt") == 0)
				tkn.operator = RPN_INT2FLT;
			else if (strcmp(token, "str2int") == 0)
				tkn.operator = RPN_STR2INT;
			else if (strcmp(token, "strb2int") == 0)
				tkn.operator = RPN_STRB2INT;
			else if (strcmp(token, "str2flt") == 0)
				tkn.operator = RPN_STR2FLT;
			else if (strcmp(token, "flt2int") == 0)
				tkn.operator = RPN_FLT2INT;
			else if (strcmp(token, "flt2str") == 0)
				tkn.operator = RPN_FLT2STR;
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
			else if (strcmp(token, "replace") == 0)
				tkn.operator = RPN_REPLACE;
			else if (strcmp(token, "replace_bre") == 0)
				tkn.operator = RPN_REPLACE_BRE;
			else if (strcmp(token, "replace_ere") == 0)
				tkn.operator = RPN_REPLACE_ERE;
			else if (strcmp(token, "matches_bre") == 0)
				tkn.operator = RPN_MATCHES_BRE;
			else if (strcmp(token, "matches_ere") == 0)
				tkn.operator = RPN_MATCHES_ERE;
			else if (strcmp(token, "next") == 0)
				tkn.operator = RPN_NEXT;
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

	if (exp->count == 0) {
		fprintf(stderr, "empty expression\n");
		goto fail;
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
				case RPN_STR2INT:
				case RPN_STRB2INT:
				case RPN_FLT2INT:
				case RPN_STRLEN:
				case RPN_LIKE:
				case RPN_LT:
				case RPN_LE:
				case RPN_GT:
				case RPN_GE:
				case RPN_EQ:
				case RPN_NE:
				case RPN_MATCHES_BRE:
				case RPN_MATCHES_ERE:
				case RPN_NEXT:
					return "int";

				case RPN_SUBSTR:
				case RPN_CONCAT:
				case RPN_INT2STR:
				case RPN_INT2STRB:
				case RPN_FLT2STR:
				case RPN_TOSTRING:
				case RPN_REPLACE:
				case RPN_REPLACE_BRE:
				case RPN_REPLACE_ERE:
					return "string";
				case RPN_INT2FLT:
				case RPN_STR2FLT:
				case RPN_TOFLOAT:
					return "float";
				case RPN_ADD:
				case RPN_SUB:
				case RPN_MUL:
				case RPN_DIV:
				case RPN_MOD:
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
				case RPN_DOUBLE:
					return "float";
				default:
					abort();
			}
			break;
		case RPN_COLUMN:
			return headers[last_token->col.num].type;
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
