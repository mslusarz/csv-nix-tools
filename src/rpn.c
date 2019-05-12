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

struct cb_params {
	struct rpn_expression *expressions;
	size_t count;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	struct rpn_expression *exp;
	struct rpn_variant ret;

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
	fputc(',', stdout);

	for (size_t i = 0; i < params->count - 1; ++i) {
		exp = &params->expressions[i];

		if (rpn_eval(exp, buf, col_offs, headers, &ret))
			exit(2);

		if (ret.type == RPN_LLONG)
			printf("%lld,", ret.llong);
		else if (ret.type == RPN_PCHAR) {
			csv_print_quoted(ret.pchar, strlen(ret.pchar));
			fputc(',', stdout);
			free(ret.pchar);
		} else {
			fprintf(stderr, "unknown type %d\n", ret.type);
			exit(2);
		}
	}

	exp = &params->expressions[params->count - 1];

	if (rpn_eval(exp, buf, col_offs, headers, &ret))
		exit(2);

	if (ret.type == RPN_LLONG)
		printf("%lld\n", ret.llong);
	else if (ret.type == RPN_PCHAR) {
		csv_print_quoted(ret.pchar, strlen(ret.pchar));
		fputc('\n', stdout);
		free(ret.pchar);
	} else {
		fprintf(stderr, "unknown type %d\n", ret.type);
		exit(2);
	}

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

		if (csv_find(headers, nheaders, expstr) != CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' already exists in input\n",
					expstr);
			exit(2);
		}

		cur++;

		struct rpn_expression *exp = &params.expressions[i];
		if (rpn_parse(exp, cur, headers, nheaders))
			exit(2);
	}

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i)
			printf("%s:%s,", headers[i].name, headers[i].type);
		for (size_t i = 0; i < nexpressions - 1; ++i)
			printf("%s:%s,", expressions[i],
				rpn_expression_type(&params.expressions[i],
						headers));
		printf("%s:%s\n", expressions[nexpressions - 1],
			rpn_expression_type(&params.expressions[nexpressions - 1],
						headers));
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < nexpressions; ++i) {
		rpn_free(&params.expressions[i]);
		free(expressions[i]);
	}

	free(expressions);
	free(params.expressions);

	return 0;
}
