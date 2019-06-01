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
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"extended-regexp",	no_argument,	NULL, 'E'},
	{"no-header",		no_argument,	NULL, 'H'},
	{"ignore-case",		no_argument,	NULL, 'i'},
	{"show",		no_argument,	NULL, 's'},
	{"version",		no_argument,	NULL, 'V'},
	{"help",		no_argument,	NULL, 'h'},
	{NULL,			0,		NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-grep [OPTION]...\n");
	printf("Options:\n");
	printf("  -e column=value\n");
	printf("  -E, --extended-regexp\n");
	printf("  -i, --ignore-case\n");
	printf("  -s, --show\n");
	printf("  -v\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct condition {
	char *column;
	char *value;
	size_t col_num;
	regex_t preg;
};

struct cb_params {
	struct condition *conditions;
	size_t nconditions;
	bool invert;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	struct condition *conditions = params->conditions;
	size_t nconditions = params->nconditions;
	bool invert = params->invert;

	if (invert) {
		// !AA && !BB && !CC -> AA || BB || CCC
		for (size_t i = 0; i < nconditions; ++i) {
			const char *val = &buf[col_offs[conditions[i].col_num]];
			const char *unquoted = val;
			bool omit = false;

			if (val[0] == '"')
				unquoted = csv_unquot(val);

			if (regexec(&conditions[i].preg, unquoted, 0, NULL, 0)
					== 0)
				omit = true;

			if (val[0] == '"')
				free((char *)unquoted);

			if (omit)
				return 0;
		}
	} else {
		// AA || BB || CC -> !AA && !BB && !CC
		size_t numfalse = 0;
		for (size_t i = 0; i < nconditions; ++i) {
			const char *val = &buf[col_offs[conditions[i].col_num]];
			const char *unquoted = val;

			if (val[0] == '"')
				unquoted = csv_unquot(val);

			if (regexec(&conditions[i].preg, unquoted, 0, NULL, 0)
					== REG_NOMATCH)
				numfalse++;

			if (val[0] == '"')
				free((char *)unquoted);
		}

		if (numfalse == nconditions)
			return 0;
	}


	csv_print_line(stdout, buf, col_offs, headers, nheaders, true);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	bool invert = false;
	struct condition *conditions = NULL;
	size_t nconditions = 0;
	bool print_header = true;
	bool show = false;
	bool extended_regexp = false;
	bool ignore_case = false;

	while ((opt = getopt_long(argc, argv, "e:Eisv", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'e': {
				const char *eq = strchr(optarg, '=');
				if (!eq) {
					fprintf(stderr, "incorrect -e syntax\n");
					exit(2);
				}
				struct condition cond;
				cond.column = strndup(optarg, (uintptr_t)eq -
						(uintptr_t)optarg);
				if (!cond.column) {
					perror("strndup");
					exit(2);
				}

				const char *start;
				size_t len;
				if (eq[1] == '\"') {
					start = eq + 2;
					len = strlen(start) - 1;
				} else {
					start = eq + 1;
					len = strlen(start);
				}
				cond.value = strndup(start, len);
				if (!cond.value) {
					perror("strndup");
					exit(2);
				}

				nconditions++;
				conditions = xrealloc_nofail(conditions,
						nconditions,
						sizeof(conditions[0]));

				memcpy(&conditions[nconditions - 1], &cond,
						sizeof(cond));

				break;
			}
			case 'E':
				extended_regexp = true;
				break;
			case 'H':
				print_header = false;
				break;
			case 'i':
				ignore_case = true;
				break;
			case 's':
				show = true;
				break;
			case 'v':
				invert = true;
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

	if (show)
		csv_show();

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	for (size_t i = 0; i < nconditions; ++i) {
		conditions[i].col_num =
			csv_find(headers, nheaders, conditions[i].column);

		if (conditions[i].col_num == CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' not found in input\n",
					conditions[i].column);
			exit(2);
		}

		int ret = regcomp(&conditions[i].preg, conditions[i].value,
				REG_NOSUB |
				(ignore_case ? REG_ICASE : 0) |
				(extended_regexp ? REG_EXTENDED : 0));
		if (ret) {
			size_t len = regerror(ret, &conditions[i].preg, NULL, 0);
			char *errbuf = xmalloc_nofail(len, 1);
			regerror(ret, &conditions[i].preg, errbuf, len);
			fprintf(stderr,
				"compilation of expression '%s' failed: %s\n",
				conditions[i].value, errbuf);
			free(errbuf);
			exit(2);
		}
	}

	if (print_header)
		csv_print_header(stdout, headers, nheaders);

	struct cb_params params;
	params.conditions = conditions;
	params.nconditions = nconditions;
	params.invert = invert;

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	for (size_t i = 0; i < nconditions; ++i) {
		free(conditions[i].column);
		free(conditions[i].value);
		regfree(&conditions[i].preg);
	}

	free(conditions);

	csv_destroy_ctx(s);

	return 0;
}
