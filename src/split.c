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

#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"print-separator",	required_argument,	NULL, 'p'},
	{"reverse",		no_argument,		NULL, 'r'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-split [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c name\n");
	fprintf(out, "  -e separator\n");
	fprintf(out, "  -n name1,name2\n");
	fprintf(out, "  -r, --reverse\n");
	fprintf(out, "  -p  --print-separator=yes/no/auto\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t col;
	char *separators;
	bool reverse;
	bool print_separators;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
	fputc(',', stdout);

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	if (str[0] == '"')
		unquoted = csv_unquot(str);

	const char *sep = params->separators;

	if (params->reverse) {
		ptrdiff_t pos = PTRDIFF_MIN;
		while (*sep) {
			char *sep_pos = rindex(unquoted, *sep);
			if (sep_pos && sep_pos - unquoted > pos)
				pos = sep_pos - unquoted;
			sep++;
		}

		if (pos != PTRDIFF_MIN) {
			csv_print_quoted(unquoted, pos);
			fputc(',', stdout);
			const char *p = unquoted + pos + 1 - params->print_separators;
			csv_print_quoted(p, strlen(p));
		} else {
			printf("%s,", str);
		}

	} else {
		ptrdiff_t pos = PTRDIFF_MAX;
		while (*sep) {
			char *sep_pos = index(unquoted, *sep);
			if (sep_pos && sep_pos - unquoted < pos)
				pos = sep_pos - unquoted;
			sep++;
		}

		if (pos != PTRDIFF_MAX) {
			csv_print_quoted(unquoted, pos);
			fputc(',', stdout);
			const char *p = unquoted + pos + 1 - params->print_separators;
			csv_print_quoted(p, strlen(p));
		} else {
			printf("%s,", str);
		}
	}

	fputc('\n', stdout);

	if (str[0] == '"')
		free((char *)unquoted);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	char *col = NULL;
	struct cb_params params;
	char *name1 = NULL;
	char *name2 = NULL;
	int print_separators = -1;
	bool show = false;
	bool show_full;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "c:e:p:sSn:r", opts,
			NULL)) != -1) {
		switch (opt) {
			case 'c':
				col = xstrdup_nofail(optarg);
				break;
			case 'e':
				params.separators = xstrdup_nofail(optarg);
				break;
			case 'n': {
				char *comma = index(optarg, ',');
				if (!comma) {
					fprintf(stderr, "Invalid format for -n option\n");
					usage(stderr);
					exit(2);
				}
				name1 = strndup(optarg, comma - optarg);
				if (index(comma + 1, ',')) {
					fprintf(stderr, "Invalid format for -n option\n");
					usage(stderr);
					exit(2);
				}
				name2 = xstrdup_nofail(comma + 1);
				break;
			}
			case 'p':
				if (strcmp(optarg, "yes") == 0)
					print_separators = 1;
				else if (strcmp(optarg, "no") == 0)
					print_separators = 0;
				else if (strcmp(optarg, "auto") == 0)
					print_separators = -1;
				else {
					fprintf(stderr, "Invalid value for --print-separators option\n");
					usage(stderr);
					exit(2);
				}
				break;
			case 'r':
				params.reverse = true;
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
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

	if (!col || !name1 || !name2 || !params.separators) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	if (print_separators >= 0)
		params.print_separators = print_separators;
	else
		params.print_separators = params.separators[1] != 0;

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (csv_find(headers, nheaders, name1) != CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' already exists in input\n",
				name1);
		exit(2);
	}

	if (csv_find(headers, nheaders, name2) != CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' already exists in input\n",
				name2);
		exit(2);
	}

	params.col = csv_find(headers, nheaders, col);
	if (params.col == CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' not found in input\n", col);
		exit(2);
	}

	free(col);
	col = NULL;

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);
	printf("%s:string,%s:string\n", name1, name2);

	free(name1);
	free(name2);
	name1 = NULL;
	name2 = NULL;

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	free(params.separators);

	return 0;
}
