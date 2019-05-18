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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"no-header",	no_argument,		NULL, 'H'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-cat [OPTION]... [FILE]...\n");
	printf("Options:\n");
	printf("  -s, --show\n");
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

static int
print_row(const char *buf, const size_t *col_offs, size_t ncols, size_t *idx)
{
	for (size_t i = 0; i < ncols - 1; ++i) {
		fputs(&buf[col_offs[idx[i]]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[idx[ncols - 1]]], stdout);
	fputc('\n', stdout);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	bool print_header = true;
	bool show = false;

	while ((opt = getopt_long(argc, argv, "sv", long_options,
			&longindex)) != -1) {
		switch (opt) {
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

	size_t ninputs = argc - optind;

	struct inputs {
		FILE *f;
		struct csv_ctx *s;
		size_t *idx;
	} *inputs = calloc(ninputs, sizeof(inputs[0]));

	bool stdin_used = false;
	size_t nheaders;
	const struct col_header *headers;

	size_t i = 0;
	while (optind < argc) {
		FILE *f;
		if (strcmp(argv[optind], "-") == 0) {
			if (stdin_used) {
				fprintf(stderr,
					"stdin is used more than once\n");
				exit(2);
			}
			f = stdin;
			stdin_used = true;
		} else {
			f = fopen(argv[optind], "r");
			if (!f) {
				fprintf(stderr, "opening '%s' failed: %s\n",
					argv[optind], strerror(errno));
				exit(2);
			}
		}

		struct csv_ctx *s = csv_create_ctx(f, stderr);
		if (!s)
			exit(2);
		if (csv_read_header(s))
			exit(2);

		const struct col_header *headers_cur;
		size_t nheaders_cur = csv_get_headers(s, &headers_cur);

		inputs[i].f = f;
		inputs[i].s = s;
		inputs[i].idx = malloc(nheaders_cur * sizeof(inputs->idx[0]));
		if (!inputs->idx) {
			perror("malloc");
			exit(2);
		}

		if (i == 0) {
			nheaders = nheaders_cur;
			headers = headers_cur;

			for (size_t j = 0; j < nheaders; ++j)
				inputs[i].idx[j] = j;
		} else {
			if (nheaders != nheaders_cur) {
				fprintf(stderr,
					"files have different number of columns\n");
				exit(2);
			}

			for (size_t j = 0; j < nheaders; ++j) {
				size_t idx = csv_find(headers_cur, nheaders_cur,
						headers[j].name);
				if (idx == CSV_NOT_FOUND) {
					fprintf(stderr,
						"column '%s' not found in input %ld\n",
						headers[j].name, i + 1);
					exit(2);
				}

				if (strcmp(headers[j].type,
						headers_cur[idx].type) != 0) {
					fprintf(stderr,
						"column '%s' have different types in different inputs\n",
						headers[j].name);
					exit(2);
				}

				inputs[i].idx[j] = idx;
			}
		}

		optind++;
		i++;
	}

	if (print_header)
		csv_print_header(stdout, headers, nheaders);

	for (i = 0; i < ninputs; ++i) {
		struct inputs *in = &inputs[i];
		struct cb_params params;
		memset(&params, 0, sizeof(params));

		if (csv_read_all(in->s, &next_row, &params))
			exit(2);

		for (size_t j = params.used; j > 0; --j) {
			size_t k = j - 1;
			print_row(params.lines[k].buf, params.lines[k].col_offs,
					nheaders, in->idx);
			free(params.lines[k].buf);
			free(params.lines[k].col_offs);
		}

		csv_destroy_ctx(in->s);
		free(params.lines);
		free(in->idx);
		fclose(in->f);
	}

	free(inputs);

	return 0;
}
