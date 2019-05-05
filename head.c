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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

static const struct option long_options[] = {
	{"lines",	required_argument,	NULL, 'n'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-head [OPTION]...\n");
	printf("Options:\n");
	printf("  -n, --lines=count\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct cb_params {
	size_t lines;
	size_t printed;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	if (params->printed >= params->lines)
		return 1;
	params->printed++;

	for (size_t i = 0; i < nheaders - 1; ++i) {
		fputs(&buf[col_offs[i]], stdout);
		fputs(",", stdout);
	}
	fputs(&buf[col_offs[nheaders - 1]], stdout);
	fputs("\n", stdout);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	params.printed = 0;

	while ((opt = getopt_long(argc, argv, "n:v", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'n': {
				params.lines = atoi(optarg);

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

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	for (size_t i = 0; i < nheaders - 1; ++i)
		printf("%s|%s,", headers[i].name, headers[i].type);
	printf("%s|%s\n", headers[nheaders - 1].name, headers[nheaders - 1].type);

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
