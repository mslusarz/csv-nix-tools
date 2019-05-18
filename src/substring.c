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

static const struct option long_options[] = {
	{"no-header",		no_argument,		NULL, 'H'},
	{"show",		no_argument,		NULL, 's'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-substring [OPTION]...\n");
	printf("Options:\n");
	printf("  -f name\n");
	printf("  -n new-name\n");
	printf("  -p start-pos\n");
	printf("  -s, --show\n");
	printf("  -l length\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct cb_params {
	size_t col;
	ssize_t start_pos;
	size_t length;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
	fputc(',', stdout);

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	if (str[0] == '"')
		unquoted = csv_unquot(str);

	size_t start;
	size_t len = strlen(unquoted);

	if (params->start_pos < 0) {
		if (params->start_pos + (ssize_t)len < 0)
			start = 0;
		else
			start = (size_t)(params->start_pos + (ssize_t)len);

		if (start + params->length < start ||
				start + params->length >= len)
			len = len - start;
		else
			len = params->length;
	} else {
		if (params->start_pos >= len)
			start = len;
		else
			start = params->start_pos;

		if (start + params->length < start ||
				start + params->length >= len)
			len = len - start;
		else
			len = params->length;
	}

	csv_print_quoted(unquoted + start, len);

	fputc('\n', stdout);

	if (str[0] == '"')
		free((char *)unquoted);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	bool print_header = true;
	char *input_col = NULL;
	char *new_name = NULL;
	struct cb_params params;
	bool show = false;

	memset(&params, 0, sizeof(params));
	params.length = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "f:l:p:sn:", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				input_col = strdup(optarg);
				break;
			case 'n':
				new_name = strdup(optarg);
				break;
			case 'p':
				if (strtol_safe(optarg, &params.start_pos))
					exit(2);
				break;
			case 's':
				show = true;
				break;
			case 'l':
				if (strtoul_safe(optarg, &params.length))
					exit(2);
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

	if (!input_col || !new_name) {
		usage();
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

	if (csv_find(headers, nheaders, new_name) != CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' already exists in input\n",
				new_name);
		exit(2);
	}

	params.col = csv_find(headers, nheaders, input_col);
	if (params.col == CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' not found in input\n", input_col);
		exit(2);
	}

	free(input_col);
	input_col = NULL;

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i)
			printf("%s:%s,", headers[i].name, headers[i].type);
		printf("%s:string\n", new_name);
	}
	free(new_name);
	new_name = NULL;

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
