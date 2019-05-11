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
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"no-header",		no_argument,		NULL, 'H'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-rpn [OPTION]...\n");
	printf("Options:\n");
	printf("  -e name=\"RPN expression\"\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct token {
	enum { COLUMN, CONSTANT, OPERATOR } type;
	union {
		size_t colnum;
		long long constant;
		enum { ADD, SUB, MUL, DIV, REM, OR, AND, XOR, LSHIFT, RSHIFT } operator;
	};
};

struct cb_params {
	struct expression {
		struct token *tokens;
		size_t count;
	} *expressions;
	size_t count;
};

static long long
eval(struct expression *exp, const char *buf, const size_t *col_offs)
{
	long long *stack = NULL;
	size_t height = 0;

	for (size_t j = 0; j < exp->count; ++j) {
		struct token *t = &exp->tokens[j];
		if (t->type == CONSTANT) {
			stack = realloc(stack,
				(height + 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				exit(2);
			}

			stack[height++] = t->constant;
		} else if (t->type == COLUMN) {
			stack = realloc(stack,
				(height + 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				exit(2);
			}

			if (strtoll_safe(&buf[col_offs[t->colnum]],
					&stack[height++]))
				exit(2);
		} else if (t->type == OPERATOR) {
			if (height < 2) {
				fprintf(stderr, "not enough stack entries\n");
				exit(2);
			}
			switch (t->operator) {
			case ADD:
				stack[height - 2] += stack[height - 1];
				break;
			case SUB:
				stack[height - 2] -= stack[height - 1];
				break;
			case MUL:
				stack[height - 2] *= stack[height - 1];
				break;
			case DIV:
				stack[height - 2] /= stack[height - 1];
				break;
			case REM:
				stack[height - 2] %= stack[height - 1];
				break;
			case OR:
				stack[height - 2] |= stack[height - 1];
				break;
			case AND:
				stack[height - 2] &= stack[height - 1];
				break;
			case XOR:
				stack[height - 2] ^= stack[height - 1];
				break;
			case LSHIFT:
				stack[height - 2] <<= stack[height - 1];
				break;
			case RSHIFT:
				stack[height - 2] >>= stack[height - 1];
				break;
			default:
				abort();
			}

			stack = realloc(stack, (height - 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				exit(2);
			}

			height--;

		} else {
			/* impossible */
			abort();
		}
	}

	if (height == 0) {
		fprintf(stderr, "empty stack\n");
		exit(2);
	}

	if (height >= 2) {
		fprintf(stderr, "too many entries on stack (%lu)\n", height);
		exit(2);
	}

	long long ret = stack[0];
	free(stack);

	return ret;
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
	fputc(',', stdout);
	struct expression *exp;

	for (size_t i = 0; i < params->count - 1; ++i) {
		exp = &params->expressions[i];

		printf("%lld,", eval(exp, buf, col_offs));
	}

	exp = &params->expressions[params->count - 1];
	printf("%lld\n", eval(exp, buf, col_offs));

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	bool print_header = true;
	size_t nexpressions = 0;
	char **expressions = NULL;
	struct cb_params params;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "e:", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'e': {
				expressions = realloc(expressions,
						(nexpressions + 1) *
						sizeof(expressions[0]));
				if (!expressions) {
					perror("realloc");
					exit(2);
				}
				expressions[nexpressions++] = strdup(optarg);

				break;
			}
			case 'H':
				print_header = false;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage();
						return 2;
				}
				break;
			case 'h':
			default:
				usage();
				return 2;
		}
	}

	if (nexpressions == 0) {
		usage();
		exit(2);
	}

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.count = nexpressions;
	params.expressions = calloc(nexpressions, sizeof(params.expressions[0]));
	if (!params.expressions) {
		perror("calloc");
		exit(2);
	}

	for (size_t i = 0; i < nexpressions; ++i) {
		char *expstr = expressions[i];
		char *cur = index(expstr, '=');
		if (!cur) {
			fprintf(stderr, "invalid expression\n");
			usage();
			exit(2);
		}
		*cur = 0;
		struct expression *exp = &params.expressions[i];

		if (csv_find(headers, nheaders, expstr) != CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' already exists in input\n",
					expstr);
			exit(2);
		}

		cur++;

		char *token = strtok(cur, " ");
		while (token) {
			struct token tkn;

			if (isdigit(token[0])) {
				tkn.type = CONSTANT;
				if (strtoll_safe(token, &tkn.constant))
					exit(2);
			} else if (isalpha(token[0])) {
				tkn.type = COLUMN;
				tkn.colnum = csv_find(headers, nheaders, token);
				if (tkn.colnum == CSV_NOT_FOUND) {
					fprintf(stderr,
						"column '%s' not found in input\n",
						token);
					exit(2);
				}
			} else if (strcmp(token, "+") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = ADD;
			} else if (strcmp(token, "-") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = SUB;
			} else if (strcmp(token, "*") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = MUL;
			} else if (strcmp(token, "/") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = DIV;
			} else if (strcmp(token, "%") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = REM;
			} else if (strcmp(token, "|") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = OR;
			} else if (strcmp(token, "&") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = AND;
			} else if (strcmp(token, "^") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = XOR;
			} else if (strcmp(token, "<<") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = LSHIFT;
			} else if (strcmp(token, ">>") == 0) {
				tkn.type = OPERATOR;
				tkn.operator = RSHIFT;
			} else {
				fprintf(stderr,
					"don't know how to interpret '%s'\n",
					token);
				exit(2);
			}

			exp->tokens = realloc(exp->tokens,
					(exp->count + 1) * sizeof(exp->tokens[0]));
			if (!exp->tokens) {
				perror("realloc");
				exit(2);
			}

			exp->tokens[exp->count++] = tkn;

			token = strtok(NULL, " ");
		}

	}

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i)
			printf("%s:%s,", headers[i].name, headers[i].type);
		for (size_t i = 0; i < nexpressions - 1; ++i)
			printf("%s:int,", expressions[i]);
		printf("%s:int\n", expressions[nexpressions - 1]);
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
