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
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-concat [OPTION]...\n");
	printf("Options:\n");
	printf("  -f name\n");
	printf("  -s \"string\"\n");
	printf("  -n new-name\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
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

	char *outbuf = malloc(len + 1);
	if (!outbuf) {
		perror("malloc");
		exit(2);
	}

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
	int longindex;
	bool print_header = true;
	char *new_name = NULL;
	struct cb_params params;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "f:s:n:", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f': {
				params.elements = realloc(params.elements,
						(params.count + 1) *
						sizeof(params.elements[0]));
				if (!params.elements) {
					perror("realloc");
					exit(2);
				}
				params.elements[params.count].is_column = true;
				params.elements[params.count].str = strdup(optarg);
				params.count++;

				break;
			}
			case 'n':
				new_name = strdup(optarg);
				break;
			case 's': {
				params.elements = realloc(params.elements,
						(params.count + 1) *
						sizeof(params.elements[0]));
				if (!params.elements) {
					perror("realloc");
					exit(2);
				}
				params.elements[params.count].is_column = false;
				params.elements[params.count].str = strdup(optarg);
				params.count++;

				break;
			}
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

	if (params.count == 0 || !new_name) {
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

	for (size_t i = 0; i < params.count; ++i) {
		if (!params.elements[i].is_column)
			continue;

		bool found = false;
		for (size_t j = 0; j < nheaders; ++j) {
			if (strcmp(params.elements[i].str, headers[j].name) != 0)
				continue;

			found = true;
			free(params.elements[i].str);
			params.elements[i].colnum = j;

			break;
		}

		if (!found) {
			fprintf(stderr, "column '%s' not found in input\n",
					params.elements[i].str);
			exit(2);
		}
	}

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
