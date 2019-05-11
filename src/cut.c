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
	printf("Usage: csv-cut [OPTION]...\n");
	printf("Options:\n");
	printf("  -f, --fields=name1[,name2...]\n");
	printf("  -r, --reverse\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
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
	int longindex;
	struct cb_params params;
	params.columns = NULL;
	params.ncolumns = 0;
	char *cols = NULL;
	bool print_header = true;
	bool reverse = false;

	while ((opt = getopt_long(argc, argv, "f:r", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				cols = strdup(optarg);
				break;
			case 'r':
				reverse = true;
				break;
			case 'H':
				print_header = false;
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

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (cols == NULL) {
		usage();
		return 2;
	}

	params.columns = malloc(nheaders * sizeof(params.columns[0]));
	if (!params.columns) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		exit(2);
	}

	if (reverse) {
		char *cols2 = malloc(strlen(cols) + 3);
		sprintf(cols2, ",%s,", cols);

		size_t longest_header = 0;
		for (size_t i = 0; i < nheaders; ++i) {
			size_t hlen = strlen(headers[i].name);
			if (hlen > longest_header)
				longest_header = hlen;
		}
		char *tmp = malloc(longest_header + 3);

		for (size_t i = 0; i < nheaders; ++i) {
			sprintf(tmp, ",%s,", headers[i].name);
			if (strstr(cols2, tmp) == NULL)
				params.columns[params.ncolumns++] = i;
		}
	} else {
		char *name = strtok(cols, ",");
		while (name) {
			int found = 0;
			for (size_t i = 0; i < nheaders; ++i) {
				if (strcmp(name, headers[i].name) != 0)
					continue;

				params.columns[params.ncolumns++] = i;
				found = 1;

				break;
			}

			if (!found) {
				fprintf(stderr, "column %s not found\n", name);
				exit(2);
			}

			name = strtok(NULL, ",");
		}
	}

	free(cols);

	if (params.ncolumns == 0) {
		fprintf(stderr, "no columns left\n");
		exit(2);
	}

	if (print_header) {
		for (size_t i = 0; i < params.ncolumns - 1; ++i)
			printf("%s:%s,", headers[params.columns[i]].name,
					headers[params.columns[i]].type);
		printf("%s:%s\n", headers[params.columns[params.ncolumns - 1]].name,
				headers[params.columns[params.ncolumns - 1]].type);
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
