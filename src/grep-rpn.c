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

#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"table",		required_argument,	NULL, 'T'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-grep-rpn [OPTION]...\n");
	fprintf(out,
"ead CSV stream from standard input, compute RPN expression for each row\n"
"and print back to standard output rows for which expression is non-zero.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -e RPN-EXPR                use expression RPN-EXPR to filter;\n"
"                             RPN expressions use space as a separator,\n"
"                             so this needs to be quoted\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	struct rpn_expression *expressions;
	size_t count;

	char *table;
	size_t table_column;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, ncols, true);

			return 0;
		}
	}

	for (size_t i = 0; i < params->count; ++i) {
		struct rpn_expression *exp = &params->expressions[i];
		struct rpn_variant ret;

		if (rpn_eval(exp, buf, col_offs, &ret))
			exit(2);

		if (ret.type != RPN_LLONG) /* shouldn't be possible */
			abort();

		if (ret.llong) {
			csv_print_line(stdout, buf, col_offs, ncols, true);
			return 0;
		}
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	size_t nexpressions = 0;
	char **expressions = NULL;
	struct cb_params params;
	bool show = false;
	bool show_full;

	params.expressions = NULL;
	params.count = 0;
	params.table = NULL;

	while ((opt = getopt_long(argc, argv, "e:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'e': {
				expressions = xrealloc_nofail(expressions,
						nexpressions + 1,
						sizeof(expressions[0]));

				expressions[nexpressions++] =
						xstrdup_nofail(optarg);

				break;
			}
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'T':
				params.table = xstrdup_nofail(optarg);
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (nexpressions == 0) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	params.count = nexpressions;
	params.expressions = xcalloc_nofail(nexpressions, sizeof(params.expressions[0]));

	for (size_t i = 0; i < nexpressions; ++i) {
		char *expstr = expressions[i];

		struct rpn_expression *exp = &params.expressions[i];
		if (rpn_parse(exp, expstr, headers, nheaders, params.table))
			exit(2);

		if (strcmp(rpn_expression_type(exp, headers), "int") != 0) {
			fprintf(stderr, "expression %ld is not numeric\n", i);
			exit(2);
		}
	}

	for (size_t i = 0; i < nexpressions; ++i)
		free(expressions[i]);
	free(expressions);

	csv_print_header(stdout, headers, nheaders);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < nexpressions; ++i)
		rpn_free(&params.expressions[i]);
	free(params.expressions);
	free(params.table);
	rpn_fini();

	return 0;
}
