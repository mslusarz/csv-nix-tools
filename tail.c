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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"lines",	required_argument,	NULL, 'n'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-tail [OPTION]...\n");
	printf("Options:\n");
	printf("  -n, --lines=count\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct cb_params {
	size_t first_empty;
	size_t count;
	size_t nlines;
	char **lines;
	size_t *sizes;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	size_t len = col_offs[nheaders - 1] +
			strlen(buf + col_offs[nheaders - 1]) + 1;
	if (len > params->sizes[params->first_empty]) {
		free(params->lines[params->first_empty]);

		params->lines[params->first_empty] = malloc(len);
		if (!params->lines[params->first_empty]) {
			fprintf(stderr, "malloc: %s\n", strerror(errno));
			return -1;
		}

		params->sizes[params->first_empty] = len;
	}

	memcpy(params->lines[params->first_empty], buf, len);

	params->first_empty++;
	if (params->first_empty == params->nlines)
		params->first_empty = 0;

	params->count++;

	return 0;
}

static void
print_line(const char *buf, size_t ncols)
{
	for (size_t j = 0; j < ncols - 1; ++j) {
		fputs(buf, stdout);
		fputc(',', stdout);
		buf += strlen(buf) + 1;
	}
	fputs(buf, stdout);
	fputc('\n', stdout);
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	params.count = 0;
	params.nlines = 0;

	while ((opt = getopt_long(argc, argv, "n:v", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'n': {
				params.nlines = atoi(optarg);

				break;
			}
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

	params.first_empty = 0;
	params.lines = calloc(params.nlines, sizeof(params.lines[0]));
	params.sizes = calloc(params.nlines, sizeof(params.sizes[0]));

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	csv_print_header(stdout, headers, nheaders);

	if (params.nlines > 0)
		if (csv_read_all(s, &next_row, &params) < 0)
			exit(2);

	csv_destroy_ctx(s);

	if (params.count >= params.nlines) {
		for (size_t i = params.first_empty; i < params.nlines; ++i)
			print_line(params.lines[i], nheaders);
		for (size_t i = 0; i < params.first_empty; ++i)
			print_line(params.lines[i], nheaders);
	} else {
		for (size_t i = 0; i < params.first_empty; ++i)
			print_line(params.lines[i], nheaders);
	}

	for (size_t i = 0; i < params.nlines; ++i)
		free(params.lines[i]);

	free(params.lines);
	free(params.sizes);

	return 0;
}
