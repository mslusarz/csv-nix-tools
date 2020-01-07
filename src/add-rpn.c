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
	fprintf(out, "Usage: csv-add-rpn [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -n new_column_name\n");
	fprintf(out, "  -e RPN_expression\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
	fprintf(out, "\n");
	fprintf(out, "                    description                     examples\n");
	fprintf(out, "-----------------------------------------------------------------------\n");
	fprintf(out, "Variables:\n");
	fprintf(out, "  %%[^ ]*            value of column                 %%name\n");
	fprintf(out, "\n");
	fprintf(out, "Constants:\n");
	fprintf(out, "  [-][1-9][0-9]*    decimal integer                 1, 1294, -89\n");
	fprintf(out, "  [-]0x[0-9a-fA-F]+ hexadecimal integer             0x1, 0x1A34, -0x8A\n");
	fprintf(out, "  [-]0[0-9]+        octal integer                   01, 01234, -067\n");
// not supported yet
//	fprintf(out, "  [-]0b[01]+        binary integer                  0b1, 0b1101, -0b10\n");
	fprintf(out, "  '[^']*'           string                          'text'\n");
//	fprintf(out, "  \"[^\"]*\"           string                          \"text\"\n");
	fprintf(out, "\n");
	fprintf(out, "Operators/functions:\n");
	fprintf(out, "  +                 addition                        %%num 5 +\n");
	fprintf(out, "  -                 subtraction                     %%num 5 -\n");
	fprintf(out, "  *                 multiplication                  %%num 5 *\n");
	fprintf(out, "  /                 division                        %%num 5 /\n");
	fprintf(out, "  %%                 modulo                          %%num 5 %%\n");
	fprintf(out, "  |                 bitwise or                      %%num 5 |\n");
	fprintf(out, "  &                 bitwise and                     %%num 5 &\n");
	fprintf(out, "  ~                 bitwise negation                %%num ~\n");
	fprintf(out, "  ^                 bitwise xor                     %%num 5 ^\n");
	fprintf(out, "  <<                bitwise left shift              %%num 5 <<\n");
	fprintf(out, "  >>                bitwise right shift             %%num 5 >>\n");
	fprintf(out, "  lt, <             less                            %%num 5 lt, %%num 5 <\n");
	fprintf(out, "  le, <=            less or equal                   %%num 5 le, %%num 5 <=\n");
	fprintf(out, "  gt, >             greater                         %%num 5 gt, %%num 5 >\n");
	fprintf(out, "  ge, >=            greater or equal                %%num 5 ge, %%num 5 >=\n");
	fprintf(out, "  eq, ==            equal                           %%num 5 eq, %%num 5 ==\n");
	fprintf(out, "  ne, !=            not equal                       %%num 5 ne, %%num 5 !=\n");
	fprintf(out, "  and               logical and                     %%bool1 %%bool2 and\n");
	fprintf(out, "  or                logical or                      %%bool1 %%bool2 or\n");
	fprintf(out, "  xor               logical exclusive or            %%bool1 %%bool2 xor\n");
	fprintf(out, "  not               logical negation                %%bool1 not\n");
	fprintf(out, "  if                if first then second else third %%bool %%val1 %%val2 if\n");
	fprintf(out, "  substr            substring                       %%str 1 3 substr\n");
	fprintf(out, "  strlen            string length                   %%str strlen\n");
	fprintf(out, "  concat            concatenation                   %%str1 %%str2 concat\n");
	fprintf(out, "  like              match pattern                   %%str 'patt%%' like\n");
	fprintf(out, "  tostring          convert to decimal notation     %%num tostring\n");
	fprintf(out, "  tostring_base2    convert to binary notation      %%num tostring_base2\n");
	fprintf(out, "  tostring_base8    convert to octal notation       %%num tostring_base8\n");
	fprintf(out, "  tostring_base10   convert to decimal notation     %%num tostring_base10\n");
	fprintf(out, "  tostring_base16   convert to hexadecimal notation %%num tostring_base16\n");
	fprintf(out, "  toint             convert to integer              %%str toint\n");
}

struct cb_params {
	struct rpn_expression *expressions;
	size_t count;

	size_t table_column;
	char *table;
};

static void
process_exp(struct rpn_expression *exp, const char *buf, const size_t *col_offs,
		const struct col_header *headers, char sep)
{
	struct rpn_variant ret;

	if (rpn_eval(exp, buf, col_offs, headers, &ret))
		exit(2);

	if (ret.type == RPN_LLONG)
		printf("%lld%c", ret.llong, sep);
	else if (ret.type == RPN_PCHAR) {
		csv_print_quoted(ret.pchar, strlen(ret.pchar));
		fputc(sep, stdout);
		free(ret.pchar);
	} else {
		fprintf(stderr, "unknown type %d\n", ret.type);
		exit(2);
	}
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	struct rpn_expression *exp;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, nheaders, false);

			putchar(',');
			putchar('\n');

			return 0;
		}
	}

	csv_print_line(stdout, buf, col_offs, nheaders, false);
	fputc(',', stdout);

	for (size_t i = 0; i < params->count - 1; ++i) {
		exp = &params->expressions[i];

		process_exp(exp, buf, col_offs, headers, ',');
	}

	exp = &params->expressions[params->count - 1];

	process_exp(exp, buf, col_offs, headers, '\n');

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	size_t nexpressions = 0;
	char **expressions = NULL;
	char **names = NULL;
	struct cb_params params;
	bool show = false;
	bool show_full;
	char *new_column = NULL;

	memset(&params, 0, sizeof(params));
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "e:n:sST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'e': {
				if (!new_column) {
					fprintf(stderr, "missing column name\n");
					exit(2);
				}
				names = xrealloc_nofail(names, nexpressions + 1,
						sizeof(names[0]));
				names[nexpressions] = new_column;
				new_column = NULL;

				expressions = xrealloc_nofail(expressions,
						nexpressions + 1,
						sizeof(expressions[0]));
				expressions[nexpressions++] =
						xstrdup_nofail(optarg);

				break;
			}
			case 'n':
				free(new_column);
				new_column = xstrdup_nofail(optarg);
				break;
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

	free(new_column);

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

	params.count = nexpressions;
	params.expressions = xcalloc_nofail(nexpressions, sizeof(params.expressions[0]));

	for (size_t i = 0; i < nexpressions; ++i) {
		csv_column_doesnt_exist(headers, nheaders, params.table, names[i]);

		struct rpn_expression *exp = &params.expressions[i];
		if (rpn_parse(exp, expressions[i], headers, nheaders, params.table))
			exit(2);
	}

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);

	for (size_t i = 0; i < nexpressions - 1; ++i) {
		if (params.table)
			printf("%s.", params.table);

		printf("%s:%s,", names[i],
			rpn_expression_type(&params.expressions[i],
					headers));
	}

	if (params.table)
		printf("%s.", params.table);

	printf("%s:%s\n", names[nexpressions - 1],
		rpn_expression_type(&params.expressions[nexpressions - 1],
					headers));

	for (size_t i = 0; i < nexpressions; ++i) {
		free(expressions[i]);
		free(names[i]);
	}

	free(expressions);
	free(names);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < nexpressions; ++i)
		rpn_free(&params.expressions[i]);

	free(params.expressions);
	free(params.table);

	return 0;
}
