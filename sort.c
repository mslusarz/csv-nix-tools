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

#define _GNU_SOURCE

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
	{"reverse",	no_argument,		NULL, 'r'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-sort [OPTION]...\n");
	printf("Options:\n");
	printf("  -f, --fields=name1[,name2...]\n");
	printf("  -r, --reverse\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct line {
	char *buf;
	size_t *col_offs;
};

struct cb_params {
	struct line *lines;
	size_t size;
	size_t used;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	if (params->used == params->size) {
		if (params->size == 0)
			params->size = 16;
		else
			params->size *= 2;

		struct line *newlines = realloc(params->lines,
				params->size * sizeof(params->lines[0]));
		if (!newlines) {
			fprintf(stderr, "realloc: %s\n", strerror(errno));
			return -1;
		}

		params->lines = newlines;
	}

	struct line *line = &params->lines[params->used];

	size_t len = col_offs[nheaders - 1] +
			strlen(buf + col_offs[nheaders - 1]) + 1;

	line->buf = malloc(len);
	if (!line->buf) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		return -1;
	}

	size_t col_offs_size = nheaders * sizeof(col_offs[0]);
	line->col_offs = malloc(col_offs_size);
	if (!line->col_offs) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		free(line->buf);
		return -1;
	}

	memcpy(line->buf, buf, len);
	memcpy(line->col_offs, col_offs, col_offs_size);
	params->used++;

	return 0;
}

struct sort_params {
	const struct col_header *headers;

	size_t *columns;
	size_t ncolumns;

	struct line *lines;
	size_t nlines;
};

int
cmp(const void *p1, const void *p2, void *arg)
{
	struct sort_params *params = arg;
	size_t idx1 = *(const size_t *)p1;
	size_t idx2 = *(const size_t *)p2;
	const struct col_header *headers = params->headers;
	struct line *lines = params->lines;

	for (size_t i = 0; i < params->ncolumns; ++i) {
		size_t col = params->columns[i];
		const struct line *line1 = &lines[idx1];
		const struct line *line2 = &lines[idx2];

		const char *val1 = &line1->buf[line1->col_offs[col]];
		const char *val2 = &line2->buf[line2->col_offs[col]];

		/* if they are equal by text then they are equal in any type */
		if (strcmp(val1, val2) == 0)
			continue;

		if (strcmp(headers[col].type, "int") != 0)
			return strcmp(val1, val2);

		/* handle integers */
		long long llval1, llval2;

		if (strtoll_safe(val1, &llval1))
			exit(2);

		if (strtoll_safe(val2, &llval2))
			exit(2);

		if (llval1 < llval2)
			return -1;

		if (llval1 > llval2)
			return 1;

		return 0;
	}

	return 0;
}

static void
print_line(struct line *line, const struct col_header *headers, size_t nheaders)
{
	csv_print_line(stdout, line->buf, line->col_offs, headers,
			nheaders, true);

	free(line->buf);
	free(line->col_offs);
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	bool reverse = false;
	params.lines = NULL;
	params.size = 0;
	params.used = 0;
	char *cols = NULL;
	struct sort_params sort_params;
	bool print_header = true;

	sort_params.columns = NULL;
	sort_params.ncolumns = 0;

	while ((opt = getopt_long(argc, argv, "f:rv", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				cols = strdup(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 'r':
				reverse = true;
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

	if (!cols) {
		fprintf(stderr, "missing -f option\n");
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

	char *name = strtok(cols, ",");
	while (name) {
		int found = 0;
		for (size_t i = 0; i < nheaders; ++i) {
			if (strcmp(name, headers[i].name) != 0)
				continue;

			sort_params.ncolumns++;
			sort_params.columns = realloc(sort_params.columns,
					sort_params.ncolumns *
					sizeof(sort_params.columns[0]));
			if (!sort_params.columns) {
				fprintf(stderr, "realloc: %s\n",
						strerror(errno));
				exit(2);
			}

			sort_params.columns[sort_params.ncolumns - 1] = i;

			found = 1;
			break;
		}

		if (!found) {
			fprintf(stderr, "column %s not found\n", name);
			exit(2);
		}

		name = strtok(NULL, ",");
	}
	free(cols);

	if (print_header)
		csv_print_header(stdout, headers, nheaders);

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	size_t *row_idx = malloc(params.used * sizeof(row_idx[0]));
	if (!row_idx) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		exit(2);
	}

	for (size_t i = 0; i < params.used; ++i)
		row_idx[i] = i;

	sort_params.headers = headers;
	sort_params.lines = params.lines;
	sort_params.nlines = params.used;
	qsort_r(row_idx, params.used, sizeof(row_idx[0]), cmp, &sort_params);

	if (reverse) {
		for (size_t i = params.used; i > 0; --i)
			print_line(&params.lines[row_idx[i - 1]], headers, nheaders);
	} else {
		for (size_t i = 0; i < params.used; ++i)
			print_line(&params.lines[row_idx[i]], headers, nheaders);
	}

	free(row_idx);
	free(params.lines);
	free(sort_params.columns);

	csv_destroy_ctx(s);

	return 0;
}
