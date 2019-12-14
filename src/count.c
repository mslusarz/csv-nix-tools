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

static const struct option opts[] = {
	{"columns",	no_argument,		NULL, 'c'},
	{"rows",	no_argument,		NULL, 'r'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-count [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c, --columns\n");
	fprintf(out, "  -r, --rows\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct cb_params {
	size_t rows;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	params->rows++;

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool show = false;
	bool columns = false;
	bool read_rows = false;
	bool rows = false;

	params.rows = 0;

	while ((opt = getopt_long(argc, argv, "crRsv", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				columns = true;
				break;
			case 'r':
				rows = true;
				break;
			case 'R':
				read_rows = true;
				break;
			case 's':
				show = true;
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
		csv_show();

	if (!columns && !rows) {
		usage(stderr);
		exit(2);
	}

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (rows || read_rows)
		csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	if (columns) {
		printf("columns:int");
		if (rows)
			putc(',', stdout);
	}

	if (rows)
		printf("rows:int");

	putc('\n', stdout);

	if (columns) {
		printf("%lu", nheaders);
		if (rows)
			putc(',', stdout);
	}

	if (rows)
		printf("%lu", params.rows);

	putc('\n', stdout);

	return 0;
}
