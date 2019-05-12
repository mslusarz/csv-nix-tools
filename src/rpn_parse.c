/*
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

int
rpn_parse(struct rpn_expression *exp, char *str,
		const struct col_header *headers, size_t nheaders)
{
	char *token = strtok(str, " ");
	while (token) {
		struct rpn_token tkn;

		if (isdigit(token[0])) {
			tkn.type = RPN_CONSTANT;
			if (strtoll_safe(token, &tkn.constant))
				goto fail;
		} else if (isalpha(token[0])) {
			tkn.type = RPN_COLUMN;
			tkn.colnum = csv_find(headers, nheaders, token);
			if (tkn.colnum == CSV_NOT_FOUND) {
				fprintf(stderr,
					"column '%s' not found in input\n",
					token);
				goto fail;
			}
		} else if (strcmp(token, "+") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_ADD;
		} else if (strcmp(token, "-") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_SUB;
		} else if (strcmp(token, "*") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_MUL;
		} else if (strcmp(token, "/") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_DIV;
		} else if (strcmp(token, "%") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_REM;
		} else if (strcmp(token, "|") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_BIT_OR;
		} else if (strcmp(token, "&") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_BIT_AND;
		} else if (strcmp(token, "^") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_BIT_XOR;
		} else if (strcmp(token, "<<") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_BIT_LSHIFT;
		} else if (strcmp(token, ">>") == 0) {
			tkn.type = RPN_OPERATOR;
			tkn.operator = RPN_BIT_RSHIFT;
		} else {
			fprintf(stderr,
				"don't know how to interpret '%s'\n",
				token);
			goto fail;
		}

		exp->tokens = realloc(exp->tokens,
				(exp->count + 1) * sizeof(exp->tokens[0]));
		if (!exp->tokens) {
			perror("realloc");
			goto fail;
		}

		exp->tokens[exp->count++] = tkn;

		token = strtok(NULL, " ");
	}

	return 0;

fail:
	free(exp->tokens);
	memset(exp, 0, sizeof(*exp));
	return -1;
}

void
rpn_free(struct rpn_expression *exp)
{
	free(exp->tokens);
	exp->tokens = NULL;
}
