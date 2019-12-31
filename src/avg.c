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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-avg [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	describe_show(out);
	describe_show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t *columns;
	size_t ncolumns;

	long long *sums;
	size_t rows;
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

	params->rows++;

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	char *cols = NULL;
	bool show = false;
	bool show_full;

	params.columns = NULL;
	params.ncolumns = 0;

	while ((opt = getopt_long(argc, argv, "f:sS", opts, NULL)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
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

	if (!cols) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.columns = xmalloc_nofail(nheaders, sizeof(params.columns[0]));

	char *col = strtok(cols, ",");
	while (col) {
		size_t idx = csv_find(headers, nheaders, col);
		if (idx == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", col);
			exit(2);
		}

		if (params.ncolumns == nheaders) {
			fprintf(stderr, "duplicated columns\n");
			exit(2);
		}

		const char *t = headers[idx].type;
		if (strcmp(t, "int") != 0) {
			fprintf(stderr,
				"Type '%s', used by column '%s', is not supported by csv-avg.\n",
				t, col);
			exit(2);
		}

		params.columns[params.ncolumns++] = idx;

		col = strtok(NULL, ",");
	}

	free(cols);

	params.sums = xcalloc_nofail(params.ncolumns, sizeof(params.sums[0]));
	params.rows = 0;

	for (size_t i = 0; i < params.ncolumns - 1; ++i)
		printf("avg(%s):%s,",
				headers[params.columns[i]].name,
				headers[params.columns[i]].type);
	printf("avg(%s):%s\n",
			headers[params.columns[params.ncolumns - 1]].name,
			headers[params.columns[params.ncolumns - 1]].type);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < params.ncolumns - 1; ++i)
		printf("%lld,", params.sums[i] / (long long)params.rows);

	printf("%lld", params.sums[params.ncolumns - 1] /
			(long long)params.rows);

	putchar('\n');

	free(params.columns);
	free(params.sums);

	return 0;
}
