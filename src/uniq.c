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
	fprintf(out, "Usage: csv-uniq [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct cb_params {
	struct buf {
		char *space;
		size_t sz;
	} *bufs;

	size_t *cols;
	size_t ncols;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	bool print = false;

	for (size_t i = 0; i < params->ncols; ++i) {
		const char *col = &buf[col_offs[params->cols[i]]];

		const char *unquoted = col;

		if (col[0] == '"')
			unquoted = csv_unquot(col);

		if (params->bufs[i].sz == 0 ||
				strcmp(params->bufs[i].space, unquoted) != 0) {
			print = true;

			size_t len = strlen(unquoted);
			if (len + 1 > params->bufs[i].sz) {
				free(params->bufs[i].space);
				params->bufs[i].space =
						xmalloc_nofail(len + 1, 1);
				params->bufs[i].sz = len + 1;
			}

			memcpy(params->bufs[i].space, unquoted, len + 1);
		}

		if (col[0] == '"')
			free((char *)unquoted);
	}

	if (print) {
		struct buf *b;
		for (size_t i = 0; i < params->ncols - 1; ++i) {
			b = &params->bufs[i];
			csv_print_quoted(b->space, strlen(b->space));
			putchar(',');
		}

		b = &params->bufs[params->ncols - 1];

		csv_print_quoted(b->space, strlen(b->space));
		putchar('\n');
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	char *cols = NULL;
	bool print_header = true;
	bool show = false;

	while ((opt = getopt_long(argc, argv, "f:rsv", long_options,
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
		fprintf(stderr, "missing -f option\n");
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

	params.cols = xmalloc_nofail(nheaders, sizeof(params.cols[0]));
	params.ncols = 0;
	params.bufs = NULL;

	char *name = strtok(cols, ",");
	while (name) {
		size_t idx = csv_find(headers, nheaders, name);
		if (idx == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", name);
			exit(2);
		}

		params.cols[params.ncols++] = idx;

		name = strtok(NULL, ",");
	}
	free(cols);

	params.cols = xrealloc_nofail(params.cols, params.ncols,
			sizeof(params.cols[0]));
	params.bufs = xcalloc_nofail(params.ncols, sizeof(params.bufs[0]));

	if (print_header) {
		for (size_t i = 0; i < params.ncols - 1; ++i) {
			printf("%s:%s,", headers[params.cols[i]].name,
					headers[params.cols[i]].type);
		}

		printf("%s:%s\n", headers[params.cols[params.ncols - 1]].name,
				headers[params.cols[params.ncols - 1]].type);
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	for (size_t i = 0; i < params.ncols; ++i)
		free(params.bufs[i].space);

	free(params.cols);
	free(params.bufs);

	csv_destroy_ctx(s);

	return 0;
}
