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

#include <ctype.h>
#include <regex.h>
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
	case RPN_MOD:
	case RPN_BIT_OR:
	case RPN_BIT_AND:
	case RPN_BIT_XOR:
	case RPN_BIT_LSHIFT:
	case RPN_BIT_RSHIFT:
	case RPN_LOGIC_AND:
	case RPN_LOGIC_OR:
	case RPN_LOGIC_XOR:
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height--;
		if (stack[height - 1].type != RPN_LLONG ||
				stack[height].type != RPN_LLONG) {
			fprintf(stderr, "+,-,*,/,%%,|,&,^,<<,>>,and,or,xor can operate only on numeric values\n");
			return -1;
		}
		break;
	case RPN_BIT_NEG:
	case RPN_LOGIC_NOT:
		if (height < 1) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		if (stack[height - 1].type != RPN_LLONG) {
			fprintf(stderr, "not/bit_neg can operate only on numeric values\n");
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
	case RPN_LIKE:
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height--;
		if (stack[height - 1].type != RPN_PCHAR ||
				stack[height].type != RPN_PCHAR) {
			fprintf(stderr, "invalid types for like operator\n");
			return -1;
		}
		break;
	case RPN_TOSTRING_BASE2:
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
	case RPN_STRLEN:
		if (height < 1) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}

		if (stack[height - 1].type != RPN_PCHAR) {
			fprintf(stderr, "strlen can operate only on string values\n");
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
			fprintf(stderr, "invalid types for if operator\n");
			return -1;
		}
		break;
	default:
		abort();
	}

	switch (oper) {
	case RPN_LOGIC_AND:
	case RPN_LOGIC_OR:
	case RPN_LOGIC_XOR:
		if (stack[height].llong != 0 && stack[height].llong != 1) {
			fprintf(stderr, "logic operators can operate only on 0/1 values\n");
			return -1;
		}
		/* fall through */
	case RPN_LOGIC_NOT:
		if (stack[height - 1].llong != 0 && stack[height - 1].llong != 1) {
			fprintf(stderr, "logic operators can operate only on 0/1 values\n");
			return -1;
		}
		break;
	default:
		break;
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
		if (stack[height].llong == 0) {
			fprintf(stderr, "division by 0\n");
			return -1;
		}
		stack[height - 1].llong /= stack[height].llong;
		break;
	case RPN_MOD:
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
	case RPN_BIT_NEG:
		stack[height - 1].llong = ~stack[height-1].llong;
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
	case RPN_LOGIC_XOR:
		stack[height - 1].llong = stack[height - 1].llong != stack[height].llong ? 1 : 0;
		break;
	case RPN_LOGIC_NOT:
		stack[height - 1].llong = stack[height - 1].llong ? 0 : 1;
		break;
	case RPN_SUBSTR: {
		char *str = stack[height - 1].pchar;
		ssize_t start = (ssize_t)stack[height].llong;
		if (stack[height + 1].llong < 0) {
			fprintf(stderr,
				"negative length (%lld) for substring operator\n",
				stack[height + 1].llong);
			return -1;
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
	case RPN_STRLEN: {
		long long ret = (long long)strlen(stack[height - 1].pchar);

		free(stack[height - 1].pchar);

		stack[height - 1].type = RPN_LLONG;
		stack[height - 1].llong = ret;
		break;
	}
	case RPN_CONCAT: {
		char *str1, *str2;
		str1 = stack[height - 1].pchar;
		str2 = stack[height].pchar;
		char *n = xmalloc(strlen(str1) + strlen(str2) + 1, 1);
		if (!n)
			return -1;
		strcpy(n, str1);
		strcat(n, str2);

		free(str1);
		free(str2);
		stack[height - 1].pchar = n;

		break;
	}
	case RPN_LIKE: {
		char *str, *pattern;
		str = stack[height - 1].pchar;
		pattern = stack[height].pchar;
		size_t patlen = strlen(pattern);

		char *n = xmalloc(patlen * 3, 1);
		if (!n)
			return -1;

		char *o = n;
		static const char escape[256] = {
				['.'] = 1,
				['?'] = 1,
				['*'] = 1,
				['+'] = 1,
				['['] = 1,
				[']'] = 1,
				['('] = 1,
				[')'] = 1,
				['$'] = 1,
				['^'] = 1,
				['\\'] = 1,
				['|'] = 1,
		};

		*o++ = '^';

		const char *c = pattern;
		while (*c) {
			if (*c == '%') {
				*o++ = '.';
				*o++ = '*';
				c++;
			} else if (escape[(unsigned char)*c]) {
				*o++ = '\\';
				*o++ = *c++;
			} else {
				*o++ = *c++;
			}
		}

		*o++ = '$';
		*o++ = 0;

		regex_t preg;

		/* TODO: cache compiled regex */
		int ret = regcomp(&preg, n, REG_EXTENDED | REG_NOSUB);
		if (ret) {
			size_t len = regerror(ret, &preg, NULL, 0);
			char *errbuf = xmalloc_nofail(len, 1);
			regerror(ret, &preg, errbuf, len);
			fprintf(stderr,
				"compilation of expression '%s' failed: %s\n",
				n, errbuf);
			free(errbuf);
			return -1;
		}

		if (regexec(&preg, str, 0, NULL, 0) == 0)
			stack[height - 1].llong = 1;
		else
			stack[height - 1].llong = 0;
		stack[height - 1].type = RPN_LLONG;

		regfree(&preg);
		free(str);
		free(pattern);
		free(n);

		break;
	}
	case RPN_TOSTRING_BASE2: {
		long long l = stack[height - 1].llong;
		if (!l) {
			stack[height - 1].type = RPN_PCHAR;
			stack[height - 1].pchar = xstrdup_nofail("0b0");
			break;
		}

		char *buf = xmalloc_nofail(68, 1);
		int idx = 0;
		if (stack[height - 1].llong < 0) {
			buf[idx++] = '-';
			l = -l;
		}
		buf[idx++] = '0';
		buf[idx++] = 'b';
		long long msb = 1LL << (63 - __builtin_clzll(l));
		while (msb) {
			buf[idx++] = (l & msb) ? '1' : '0';
			msb >>= 1;
		}
		buf[idx] = 0;

		stack[height - 1].type = RPN_PCHAR;
		stack[height - 1].pchar = buf;

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
		if (strtoll_safe(stack[height - 1].pchar, &ret, 0) < 0)
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

	stack = xrealloc(stack, height, sizeof(stack[0]));
	if (!stack)
		return -1;

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
			stack = xrealloc(stack, height + 1, sizeof(stack[0]));
			if (!stack)
				goto fail;

			stack[height++] = t->constant;
			if (t->constant.type == RPN_PCHAR) {
				stack[height - 1].pchar = xstrdup(stack[height - 1].pchar);
				if (!stack[height - 1].pchar)
					goto fail;
			}
		} else if (t->type == RPN_COLUMN) {
			stack = xrealloc(stack, height + 1, sizeof(stack[0]));
			if (!stack)
				goto fail;

			const char *str = &buf[col_offs[t->colnum]];

			if (strcmp(headers[t->colnum].type, "int") == 0) {
				stack[height].type = RPN_LLONG;
				if (strtoll_safe(str, &stack[height].llong, 0))
					goto fail;
			} else {
				stack[height].type = RPN_PCHAR;
				if (str[0] == '"')
					stack[height].pchar = csv_unquot(str);
				else
					stack[height].pchar = xstrdup(str);
				if (!stack[height].pchar)
					goto fail;
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
