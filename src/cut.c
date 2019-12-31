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

static const struct option opts[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"reverse",	no_argument,		NULL, 'r'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-cut [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -r, --reverse\n");
	describe_show(out);
	describe_show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t *columns;
	size_t ncolumns;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	for (size_t i = 0; i < params->ncolumns - 1; ++i) {
		fputs(&buf[col_offs[params->columns[i]]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[params->columns[params->ncolumns - 1]]], stdout);
	fputc('\n', stdout);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	params.columns = NULL;
	params.ncolumns = 0;
	char *cols = NULL;
	bool reverse = false;
	bool show = false;
	bool show_full;

	while ((opt = getopt_long(argc, argv, "f:rsS", opts, NULL)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'r':
				reverse = true;
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

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	int ret = 0;
	if (cols == NULL) {
		usage(stderr);
		ret = 2;
		goto end;
	}

	params.columns = xmalloc_nofail(nheaders, sizeof(params.columns[0]));

	if (reverse) {
		char *cols2 = xmalloc_nofail(strlen(cols) + 3, 1);
		sprintf(cols2, ",%s,", cols);

		size_t longest_header = 0;
		for (size_t i = 0; i < nheaders; ++i) {
			size_t hlen = strlen(headers[i].name);
			if (hlen > longest_header)
				longest_header = hlen;
		}
		char *tmp = xmalloc_nofail(longest_header + 3, 1);

		for (size_t i = 0; i < nheaders; ++i) {
			sprintf(tmp, ",%s,", headers[i].name);
			if (strstr(cols2, tmp) == NULL)
				params.columns[params.ncolumns++] = i;
		}
	} else {
		char *name = strtok(cols, ",");
		while (name) {
			params.columns[params.ncolumns] =
					csv_find(headers, nheaders, name);

			if (params.columns[params.ncolumns] == CSV_NOT_FOUND) {
				fprintf(stderr, "column %s not found\n", name);
				exit(2);
			}

			params.ncolumns++;
			name = strtok(NULL, ",");
		}
	}

	free(cols);

	if (params.ncolumns == 0) {
		fprintf(stderr, "no columns left\n");
		ret = 2;
		goto cleanup;
	}

	for (size_t i = 0; i < params.ncolumns - 1; ++i)
		printf("%s:%s,", headers[params.columns[i]].name,
				headers[params.columns[i]].type);
	printf("%s:%s\n", headers[params.columns[params.ncolumns - 1]].name,
			headers[params.columns[params.ncolumns - 1]].type);

	csv_read_all_nofail(s, &next_row, &params);

cleanup:
	free(params.columns);
end:
	csv_destroy_ctx(s);

	return ret;
}
