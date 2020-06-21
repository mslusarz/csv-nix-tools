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
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ht.h"
#include "regex_cache.h"
#include "utils.h"

static char *
tostring(long long val, long long base)
{
	char *str;

	if (base == 2) {
		if (val == 0)
			return xstrdup_nofail("0b0");

		str = xmalloc_nofail(68, 1);
		int idx = 0;
		if (val < 0) {
			str[idx++] = '-';
			val = -val;
		}
		unsigned long long uval = (unsigned long long)val;
		str[idx++] = '0';
		str[idx++] = 'b';
		unsigned long long msb = 1ULL << (63 - __builtin_clzll(uval));
		while (msb) {
			str[idx++] = (uval & msb) ? '1' : '0';
			msb >>= 1;
		}
		str[idx] = 0;
	} else if (base == 8) {
		if (csv_asprintf(&str, "0%llo", val) < 0) {
			perror("asprintf");
			str = NULL;
		}
	} else if (base == 10) {
		if (csv_asprintf(&str, "%lld", val) < 0) {
			perror("asprintf");
			str = NULL;
		}
	} else if (base == 16) {
		if (csv_asprintf(&str, "0x%llx", val) < 0) {
			perror("asprintf");
			str = NULL;
		}
	} else {
		/* not possible, checked earlier */
		abort();
	}

	return str;
}

static char *
replace_re(const char *str, const char *replacement, const regmatch_t *matches)
{
	size_t len = strlen(replacement);

	size_t buflen = len + 1;
	char *buf = xmalloc_nofail(buflen, 1);
	size_t buf_used = 0;

	for (size_t i = 0; i < len; ++i) {
		if (replacement[i] != '%') {
			csv_check_space(&buf, &buflen, buf_used, 1);
			buf[buf_used++] = replacement[i];
			continue;
		}

		if (i + 1 >= len) {
			fprintf(stderr, "dangling '%%'\n");
			exit(2);
		}

		if (replacement[i + 1] == '%') {
			i++;
			csv_check_space(&buf, &buflen, buf_used, 1);
			buf[buf_used++] = '%';
		} else if (isdigit(replacement[i + 1])) {
			i++;
			unsigned match_num = (unsigned)(replacement[i] - '0');

			const regmatch_t *m = &matches[match_num];
			if (m->rm_so == -1) {
				unsigned cnt = match_num;
				while (cnt > 0 && matches[cnt].rm_so == -1)
					cnt--;

				fprintf(stderr,
					"expression %u doesn't exist, last valid expressions number is %u\n",
					match_num, cnt);
					exit(2);
			}

			regoff_t start = m->rm_so;
			regoff_t end = m->rm_eo;
			assert(end - start >= 0);
			size_t len = (size_t)(end - start);

			csv_check_space(&buf, &buflen, buf_used, len);

			memcpy(&buf[buf_used], str + start, len);

			buf_used += len;
		} else {
			fprintf(stderr,
				"incorrect syntax - character after %% is not a digit or %%\n");
			exit(2);
		}
	}

	csv_check_space(&buf, &buflen, buf_used, 1);
	buf[buf_used] = 0;

	return buf;
}

static struct csv_ht *Seq;

static void *
first_value(void *arg)
{
	UNUSED(arg);

	long long *l = xmalloc_nofail(1, sizeof(*l));
	*l = 0;
	return l;
}

long long
next(char *name)
{
	if (!Seq && csv_ht_init(&Seq, free))
		exit(2);

	long long *v = csv_ht_get_value(Seq, name, first_value, NULL);

	(*v)++;

	return *v;
}

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
	case RPN_TOSTRING:
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}

		height--;
		if (stack[height - 1].type != RPN_LLONG) {
			fprintf(stderr, "tostring can operate only on numeric values\n");
			return -1;
		}

		if (stack[height].type != RPN_LLONG) {
			fprintf(stderr, "base must be a numeric value\n");
			return -1;
		}

		if (stack[height].llong != 2 && stack[height].llong != 8 &&
				stack[height].llong != 10 && stack[height].llong != 16) {
			fprintf(stderr,
				"base must be one of these values: 2, 8, 10, 16\n");
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
		if (height < 2) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height--;

		if (stack[height - 1].type != RPN_PCHAR) {
			fprintf(stderr, "1st argument of toint must be a string\n");
			return -1;
		}

		if (stack[height].type != RPN_LLONG) {
			fprintf(stderr, "2nd argument of toint must be a number\n");
			return -1;
		}

		/* 2 and 36 come from strtoll man page */
		if (stack[height].llong < 2 || stack[height].llong > 36) {
			fprintf(stderr,
				"2nd argument of toint must be between 2 and 36\n");
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
	case RPN_REPLACE:
	case RPN_REPLACE_BRE:
	case RPN_REPLACE_ERE:
		if (height < 4) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height -= 3;
		if (stack[height - 1].type != RPN_PCHAR ||
				stack[height].type != RPN_PCHAR ||
				stack[height + 1].type != RPN_PCHAR ||
				stack[height + 2].type != RPN_LLONG) {
			fprintf(stderr, "invalid types for replace operator\n");
			return -1;
		}
		break;
	case RPN_MATCHES_BRE:
	case RPN_MATCHES_ERE:
		if (height < 3) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}
		height -= 2;
		if (stack[height - 1].type != RPN_PCHAR ||
				stack[height].type != RPN_PCHAR ||
				stack[height + 1].type != RPN_LLONG) {
			fprintf(stderr, "invalid types for matches* operator\n");
			return -1;
		}
		break;
	case RPN_NEXT:
		if (height < 1) {
			fprintf(stderr, "not enough stack entries\n");
			return -1;
		}

		if (stack[height - 1].type != RPN_PCHAR) {
			fprintf(stderr, "invalid type for next operator\n");
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

		regex_t *preg;

		int ret = csv_regex_get(&preg, n, REG_EXTENDED | REG_NOSUB);
		if (ret)
			return -1;

		if (regexec(preg, str, 0, NULL, 0) == 0)
			stack[height - 1].llong = 1;
		else
			stack[height - 1].llong = 0;
		stack[height - 1].type = RPN_LLONG;

		free(str);
		free(pattern);
		free(n);

		break;
	}
	case RPN_TOSTRING: {
		long long val = stack[height - 1].llong;
		long long base = stack[height].llong;
		char *str = tostring(val, base);
		if (!str)
			return -1;

		stack[height - 1].type = RPN_PCHAR;
		stack[height - 1].pchar = str;

		break;
	}
	case RPN_TOINT: {
		long long ret;
		const char *str = stack[height - 1].pchar;
		int base = (int)stack[height].llong;
		if (str[0] == '0' && str[1] == 'b' && base == 2)
			str += 2;

		if (strtoll_safe(str, &ret, base) < 0)
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
	case RPN_REPLACE: {
		char *str = stack[height - 1].pchar;
		char *pattern = stack[height].pchar;
		char *replacement = stack[height + 1].pchar;
		long long case_sensitive = stack[height + 2].llong;
		char *buf = NULL;
		size_t buflen = 0;

		if (csv_str_replace(str, pattern, replacement, case_sensitive,
				&buf, &buflen)) {
			stack[height - 1].pchar = buf;
			free(str);
		} else {
			stack[height - 1].pchar = str;
			free(buf);
		}

		free(pattern);
		free(replacement);

		break;
	}
	case RPN_REPLACE_BRE:
	case RPN_REPLACE_ERE: {
		char *str = stack[height - 1].pchar;
		char *pattern = stack[height].pchar;
		char *replacement = stack[height + 1].pchar;
		long long case_sensitive = stack[height + 2].llong;

		regex_t *preg;

		int ret = csv_regex_get(&preg, pattern,
				(case_sensitive ? 0 : REG_ICASE) |
				(oper == RPN_REPLACE_ERE ? REG_EXTENDED : 0));
		if (ret)
			return -1;

#define MAX_MATCHES 9
		regmatch_t matches[MAX_MATCHES + 1];
		if (regexec(preg, str, MAX_MATCHES, matches, 0) == 0) {
			stack[height - 1].pchar =
					replace_re(str, replacement, matches);
			free(str);
		} else {
			stack[height - 1].pchar = str;
		}
#undef MAX_MATCHES

		free(pattern);
		free(replacement);

		break;
	}
	case RPN_MATCHES_BRE:
	case RPN_MATCHES_ERE: {
		char *str = stack[height - 1].pchar;
		char *pattern = stack[height].pchar;
		long long case_sensitive = stack[height + 1].llong;

		regex_t *preg;

		int ret = csv_regex_get(&preg, pattern,
				REG_NOSUB |
				(case_sensitive ? 0 : REG_ICASE) |
				(oper == RPN_MATCHES_ERE ? REG_EXTENDED : 0));
		if (ret)
			return -1;
		free(pattern);

		stack[height - 1].type = RPN_LLONG;

		if (regexec(preg, str, 0, NULL, 0) == 0)
			stack[height - 1].llong = 1;
		else
			stack[height - 1].llong = 0;

		free(str);

		break;
	}
	case RPN_NEXT: {
		char *name = stack[height - 1].pchar;

		stack[height - 1].type = RPN_LLONG;
		stack[height - 1].llong = next(name);

		free(name);

		break;
	}
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
rpn_eval(const struct rpn_expression *exp,
		const char *buf,
		const size_t *col_offs,
		const struct col_header *headers,
		struct rpn_variant *value)
{
	struct rpn_variant *stack = NULL;
	size_t height = 0;

	for (size_t j = 0; j < exp->count; ++j) {
		const struct rpn_token *t = &exp->tokens[j];
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

void
rpn_fini(void)
{
	csv_regex_clear_cache();
	if (Seq)
		csv_ht_destroy(&Seq);
}
