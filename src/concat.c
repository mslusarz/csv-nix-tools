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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"show",		no_argument,		NULL, 's'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-concat [OPTION]... -- new_name = [%%name|str]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct elem {
	bool is_column;
	union {
		size_t colnum;
		char *str;
	};
};

struct cb_params {
	struct elem *elements;
	size_t count;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
	fputc(',', stdout);

	size_t len = 0;
	for (size_t i = 0; i < params->count; ++i) {
		if (params->elements[i].is_column)
			/* ignore quoting for now, we need upper bound */
			len += strlen(&buf[col_offs[params->elements[i].colnum]]);
		else
			len += strlen(params->elements[i].str);
	}

	char *outbuf = xmalloc_nofail(len + 1, 1);

	size_t start = 0;
	for (size_t i = 0; i < params->count; ++i) {
		if (params->elements[i].is_column) {
			const char *str = &buf[col_offs[params->elements[i].colnum]];
			const char *unquoted = str;
			if (str[0] == '"')
				unquoted = csv_unquot(str);

			len = strlen(unquoted);
			memcpy(outbuf + start, unquoted, len);

			if (str[0] == '"')
				free((char *)unquoted);

		} else {
			len = strlen(params->elements[i].str);
			memcpy(outbuf + start, params->elements[i].str, len);
		}

		start += len;
	}

	outbuf[start] = 0;

	csv_print_quoted(outbuf, start);

	fputc('\n', stdout);

	free(outbuf);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	char *new_name = NULL;
	struct cb_params params;
	bool show = false;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "s", opts, NULL)) != -1) {
		switch (opt) {
			case 's':
				show = true;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (argc - optind < 3) {
		usage(stderr);
		exit(2);
	}

	new_name = argv[optind++];

	if (strcmp(argv[optind++], "=") != 0) {
		usage(stderr);
		exit(2);
	}

	params.count = argc - optind;
	params.elements = xmalloc_nofail(params.count, sizeof(params.elements[0]));

	size_t i = 0;
	while (optind < argc) {
		params.elements[i].is_column = argv[optind][0] == '%';
		params.elements[i].str = argv[optind];

		if (params.elements[i].is_column)
			params.elements[i].str++;
		optind++;
		i++;
	}

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (csv_find(headers, nheaders, new_name) != CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' already exists in input\n",
				new_name);
		exit(2);
	}

	for (size_t i = 0; i < params.count; ++i) {
		if (!params.elements[i].is_column)
			continue;

		size_t idx = csv_find(headers, nheaders, params.elements[i].str);
		if (idx == CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' not found in input\n",
					params.elements[i].str);
			exit(2);
		}

		params.elements[i].colnum = idx;
	}

	if (show)
		csv_show();

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);
	printf("%s:string\n", new_name);

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	free(params.elements);

	return 0;
}
