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

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"lines",	required_argument,	NULL, 'n'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-tail [OPTION]...\n");
	fprintf(out, "Print the last 10 rows of CSV file from standard input to standard output.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -n, --lines=NUM            print the last NUM rows instead of the last 10,\n"
"                             NUM must be >= 0\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t first_empty;
	size_t count;
	size_t nlines;
	char **lines;
	size_t *sizes;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	size_t len = col_offs[ncols - 1] +
			strlen(buf + col_offs[ncols - 1]) + 1;
	if (len > params->sizes[params->first_empty]) {
		free(params->lines[params->first_empty]);

		params->lines[params->first_empty] = xmalloc(len, 1);
		if (!params->lines[params->first_empty])
			return -1;

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
	struct cb_params params;
	bool show = false;
	bool show_full;

	params.count = 0;
	params.nlines = 10;

	while ((opt = getopt_long(argc, argv, "n:sS", opts, NULL)) != -1) {
		switch (opt) {
			case 'n':
				if (strtoul_safe(optarg, &params.nlines, 0))
					exit(2);
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

	params.first_empty = 0;
	params.lines = xcalloc_nofail(params.nlines, sizeof(params.lines[0]));
	params.sizes = xcalloc_nofail(params.nlines, sizeof(params.sizes[0]));

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	csv_print_header(stdout, headers, nheaders);

	if (params.nlines > 0)
		csv_read_all_nofail(s, &next_row, &params);

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
