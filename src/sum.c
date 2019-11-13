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

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-sum [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct cb_params {
	size_t *columns;
	size_t ncolumns;
	long long *sums;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	for (size_t i = 0; i < params->ncolumns; ++i) {
		const char *val = &buf[col_offs[params->columns[i]]];

		long long llval;
		if (strtoll_safe(val, &llval, 0))
			return -1;

		if (llval > 0 && params->sums[i] > LLONG_MAX - llval) {
			fprintf(stderr, "integer overflow\n");
			return -1;
		}

		if (llval < 0 && params->sums[i] < LLONG_MIN - llval) {
			fprintf(stderr, "integer underflow\n");
			return -1;
		}

		params->sums[i] += llval;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	params.columns = NULL;
	params.ncolumns = 0;
	char *cols = NULL;
	bool print_header = true;
	bool show = false;

	while ((opt = getopt_long(argc, argv, "f:sv", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 's':
				show = true;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage(stderr);
						return 2;
				}
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (!cols) {
		usage(stderr);
		exit(2);
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

	params.columns = xmalloc_nofail(nheaders, sizeof(params.columns[0]));

	char *name = strtok(cols, ",");
	while (name) {
		size_t idx = csv_find(headers, nheaders, name);
		if (idx == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", name);
			exit(2);
		}

		if (strcmp(headers[idx].type, "int") != 0) {
			fprintf(stderr, "column %s is not an integer\n", name);
			exit(2);
		}

		if (params.ncolumns == nheaders) {
			fprintf(stderr, "duplicated columns\n");
			exit(2);
		}

		params.columns[params.ncolumns++] = idx;

		name = strtok(NULL, ",");
	}

	free(cols);

	params.sums = xcalloc_nofail(params.ncolumns, sizeof(params.sums[0]));

	if (print_header) {
		for (size_t i = 0; i < params.ncolumns - 1; ++i)
			printf("sum(%s):%s,", headers[params.columns[i]].name,
					headers[params.columns[i]].type);
		printf("sum(%s):%s\n", headers[params.columns[params.ncolumns - 1]].name,
				headers[params.columns[params.ncolumns - 1]].type);
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < params.ncolumns - 1; ++i)
		printf("%lld,", params.sums[i]);
	printf("%lld\n", params.sums[params.ncolumns - 1]);

	free(params.columns);
	free(params.sums);

	return 0;
}
