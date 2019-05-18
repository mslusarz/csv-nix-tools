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

/* asprintf */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

static int
eval_oper(enum rpn_operator oper, struct rpn_variant **pstack, size_t *pheight)
{
	size_t height = *pheight;
	struct rpn_variant *stack = *pstack;

	switch (oper) {
	case RPN_ADD:
	case RPN_SUB:
	case RPN_MUL:
	case RPN_DIV:
	case RPN_REM:
	case RPN_BIT_OR:
	case RPN_BIT_AND:
	case RPN_BIT_XOR:
	case RPN_BIT_LSHIFT:
	case RPN_BIT_RSHIFT:
	case RPN_LOGIC_AND:
	case RPN_LOGIC_OR:
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height--;
		if (stack[height - 1].type != RPN_LLONG ||
				stack[height].type != RPN_LLONG) {
			fprintf(stderr, "+,-,*,/,%%,|,&,^,<<,>> can operate only on numeric values\n");
			return -1;
		}
		break;
	case RPN_LOGIC_NOT:
		if (height < 1) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		if (stack[height - 1].type != RPN_LLONG) {
			fprintf(stderr, "logic not can operate only on numeric values\n");
			return -1;
		}
		break;
	case RPN_SUBSTR:
		if (height < 3) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height -= 2;
		if (stack[height - 1].type != RPN_PCHAR ||
				stack[height].type != RPN_LLONG ||
				stack[height + 1].type != RPN_LLONG) {
			fprintf(stderr, "invalid types for substr operator\n");
			return -1;
		}
		break;
	case RPN_CONCAT:
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height--;
		if (stack[height - 1].type != RPN_PCHAR ||
				stack[height].type != RPN_PCHAR) {
			fprintf(stderr, "invalid types for concat operator\n");
			return -1;
		}
		break;
	case RPN_TOSTRING_BASE8:
	case RPN_TOSTRING_BASE10:
	case RPN_TOSTRING_BASE16:
		if (height < 1) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}

		if (stack[height - 1].type != RPN_LLONG) {
			fprintf(stderr, "tostring can operate only on numeric values\n");
			return -1;
		}

		break;
	case RPN_TOINT:
		if (height < 1) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}

		if (stack[height - 1].type != RPN_PCHAR) {
			fprintf(stderr, "toint can operate only on string values\n");
			return -1;
		}

		break;
	case RPN_LT:
	case RPN_LE:
	case RPN_GT:
	case RPN_GE:
	case RPN_EQ:
	case RPN_NE:
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height--;

		if (stack[height - 1].type != stack[height].type) {
			fprintf(stderr,
				"comparison operators can operate only on the same type values\n");
			return -1;
		}
		break;
	case RPN_IF:
		if (height < 3) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height -= 2;
		if (stack[height - 1].type != RPN_LLONG ||
				stack[height].type != stack[height + 1].type) {
			fprintf(stderr, "invalid types for substr operator\n");
			return -1;
		}
		break;
	default:
		abort();
	}

	switch (oper) {
	case RPN_ADD:
		stack[height - 1].llong += stack[height].llong;
		break;
	case RPN_SUB:
		stack[height - 1].llong -= stack[height].llong;
		break;
	case RPN_MUL:
		stack[height - 1].llong *= stack[height].llong;
		break;
	case RPN_DIV:
		stack[height - 1].llong /= stack[height].llong;
		break;
	case RPN_REM:
		stack[height - 1].llong %= stack[height].llong;
		break;
	case RPN_BIT_OR:
		stack[height - 1].llong |= stack[height].llong;
		break;
	case RPN_BIT_AND:
		stack[height - 1].llong &= stack[height].llong;
		break;
	case RPN_BIT_XOR:
		stack[height - 1].llong ^= stack[height].llong;
		break;
	case RPN_BIT_LSHIFT:
		stack[height - 1].llong <<= stack[height].llong;
		break;
	case RPN_BIT_RSHIFT:
		stack[height - 1].llong >>= stack[height].llong;
		break;
	case RPN_LOGIC_AND:
		stack[height - 1].llong = stack[height - 1].llong && stack[height].llong ? 1 : 0;
		break;
	case RPN_LOGIC_OR:
		stack[height - 1].llong = stack[height - 1].llong || stack[height].llong ? 1 : 0;
		break;
	case RPN_LOGIC_NOT:
		stack[height - 1].llong = stack[height - 1].llong ? 0 : 1;
		break;
	case RPN_SUBSTR: {
		char *str = stack[height - 1].pchar;
		ssize_t start = (ssize_t)stack[height].llong;
		if (stack[height + 1].llong < 0) {
			fprintf(stderr,
				"negative length (%lld) for substring op\n",
				stack[height + 1].llong);
			exit(2);
		}
		size_t len = (size_t)stack[height + 1].llong;

		csv_substring_sanitize(str, &start, &len);

		char *n = strndup(str + start, len);
		if (!n) {
			perror("strndup");
			return -1;
		}

		free(str);
		stack[height - 1].pchar = n;

		break;
	}
	case RPN_CONCAT: {
		char *str1, *str2;
		str1 = stack[height - 1].pchar;
		str2 = stack[height].pchar;
		char *n = malloc(strlen(str1) + strlen(str2) + 1);
		if (!n) {
			perror("malloc");
			return -1;
		}
		strcpy(n, str1);
		strcat(n, str2);

		free(str1);
		free(str2);
		stack[height - 1].pchar = n;

		break;
	}
	case RPN_TOSTRING_BASE8: {
		char *str;
		if (asprintf(&str, "0%llo", stack[height - 1].llong) < 0) {
			perror("asprintf");
			return -1;
		}

		stack[height - 1].type = RPN_PCHAR;
		stack[height - 1].pchar = str;

		break;
	}
	case RPN_TOSTRING_BASE10: {
		char *str;
		if (asprintf(&str, "%lld", stack[height - 1].llong) < 0) {
			perror("asprintf");
			return -1;
		}

		stack[height - 1].type = RPN_PCHAR;
		stack[height - 1].pchar = str;

		break;
	}
	case RPN_TOSTRING_BASE16: {
		char *str;
		if (asprintf(&str, "0x%llx", stack[height - 1].llong) < 0) {
			perror("asprintf");
			return -1;
		}

		stack[height - 1].type = RPN_PCHAR;
		stack[height - 1].pchar = str;

		break;
	}
	case RPN_TOINT: {
		long long ret;
		if (strtoll_safe(stack[height - 1].pchar, &ret) < 0)
			return -1;

		free(stack[height - 1].pchar);

		stack[height - 1].type = RPN_LLONG;
		stack[height - 1].llong = ret;
		break;
	}
	case RPN_LT:
		if (stack[height - 1].type == RPN_LLONG)
			stack[height - 1].llong = stack[height - 1].llong < stack[height].llong ? 1 : 0;
		else {
			int ret = strcmp(stack[height - 1].pchar, stack[height].pchar) < 0 ? 1 : 0;

			free(stack[height - 1].pchar);
			free(stack[height].pchar);

			stack[height - 1].type = RPN_LLONG;
			stack[height - 1].llong = ret;
		}
		break;
	case RPN_LE:
		if (stack[height - 1].type == RPN_LLONG)
			stack[height - 1].llong = stack[height - 1].llong <= stack[height].llong ? 1 : 0;
		else {
			int ret = strcmp(stack[height - 1].pchar, stack[height].pchar) <= 0 ? 1 : 0;

			free(stack[height - 1].pchar);
			free(stack[height].pchar);

			stack[height - 1].type = RPN_LLONG;
			stack[height - 1].llong = ret;
		}
		break;
	case RPN_GT:
		if (stack[height - 1].type == RPN_LLONG)
			stack[height - 1].llong = stack[height - 1].llong > stack[height].llong ? 1 : 0;
		else {
			int ret = strcmp(stack[height - 1].pchar, stack[height].pchar) > 0 ? 1 : 0;

			free(stack[height - 1].pchar);
			free(stack[height].pchar);

			stack[height - 1].type = RPN_LLONG;
			stack[height - 1].llong = ret;
		}
		break;
	case RPN_GE:
		if (stack[height - 1].type == RPN_LLONG)
			stack[height - 1].llong = stack[height - 1].llong >= stack[height].llong ? 1 : 0;
		else {
			int ret = strcmp(stack[height - 1].pchar, stack[height].pchar) >= 0 ? 1 : 0;

			free(stack[height - 1].pchar);
			free(stack[height].pchar);

			stack[height - 1].type = RPN_LLONG;
			stack[height - 1].llong = ret;
		}
		break;
	case RPN_EQ:
		if (stack[height - 1].type == RPN_LLONG)
			stack[height - 1].llong = stack[height - 1].llong == stack[height].llong ? 1 : 0;
		else {
			int ret = strcmp(stack[height - 1].pchar, stack[height].pchar) == 0 ? 1 : 0;

			free(stack[height - 1].pchar);
			free(stack[height].pchar);

			stack[height - 1].type = RPN_LLONG;
			stack[height - 1].llong = ret;
		}
		break;
	case RPN_NE:
		if (stack[height - 1].type == RPN_LLONG)
			stack[height - 1].llong = stack[height - 1].llong != stack[height].llong ? 1 : 0;
		else {
			int ret = strcmp(stack[height - 1].pchar, stack[height].pchar) != 0 ? 1 : 0;

			free(stack[height - 1].pchar);
			free(stack[height].pchar);

			stack[height - 1].type = RPN_LLONG;
			stack[height - 1].llong = ret;
		}
		break;
	case RPN_IF:
		if (stack[height - 1].llong) {
			stack[height - 1] = stack[height];
			if (stack[height + 1].type == RPN_PCHAR)
				free(stack[height + 1].pchar);
		} else {
			stack[height - 1] = stack[height + 1];
			if (stack[height].type == RPN_PCHAR)
				free(stack[height].pchar);
		}

		break;
	default:
		abort();
	}

	stack = realloc(stack, height * sizeof(stack[0]));
	if (!stack) {
		perror("realloc");
		return -1;
	}

	*pheight = height;
	*pstack = stack;

	return 0;
}

int
rpn_eval(struct rpn_expression *exp,
		const char *buf,
		const size_t *col_offs,
		const struct col_header *headers,
		struct rpn_variant *value)
{
	struct rpn_variant *stack = NULL;
	size_t height = 0;

	for (size_t j = 0; j < exp->count; ++j) {
		struct rpn_token *t = &exp->tokens[j];
		if (t->type == RPN_CONSTANT) {
			stack = realloc(stack, (height + 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				goto fail;
			}

			stack[height++] = t->constant;
			if (t->constant.type == RPN_PCHAR) {
				stack[height - 1].pchar = strdup(stack[height - 1].pchar);
				if (!stack[height - 1].pchar) {
					perror("strdup");
					goto fail;
				}
			}
		} else if (t->type == RPN_COLUMN) {
			stack = realloc(stack, (height + 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				goto fail;
			}
			const char *str = &buf[col_offs[t->colnum]];

			if (strcmp(headers[t->colnum].type, "int") == 0) {
				stack[height].type = RPN_LLONG;
				if (strtoll_safe(str, &stack[height].llong))
					goto fail;
			} else {
				stack[height].type = RPN_PCHAR;
				if (str[0] == '"')
					stack[height].pchar = csv_unquot(str);
				else
					stack[height].pchar = strdup(str);
			}
			height++;
		} else if (t->type == RPN_OPERATOR) {
			if (eval_oper(t->operator, &stack, &height))
				goto fail;
		} else {
			/* impossible */
			abort();
		}
	}

	if (height == 0) {
		fprintf(stderr, "empty stack\n");
		goto fail;
	}

	if (height >= 2) {
		fprintf(stderr, "too many entries on stack (%lu)\n", height);
		goto fail;
	}

	*value = stack[0];

	free(stack);

	return 0;

fail:
	free(stack);
	return -1;
}
