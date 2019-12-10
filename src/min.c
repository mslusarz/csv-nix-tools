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
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-min [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct cb_params {
	size_t *columns;
	enum output_types *types;
	size_t ncolumns;

	long long *min_int;
	char **min_str;
	size_t *str_size;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	for (size_t i = 0; i < params->ncolumns; ++i) {
		const char *val = &buf[col_offs[params->columns[i]]];

		if (params->types[i] == TYPE_INT) {
			long long llval;
			if (strtoll_safe(val, &llval, 0))
				return -1;

			if (llval < params->min_int[i])
				params->min_int[i] = llval;

		} else {
			const char *unquoted = val;
			if (val[0] == '"')
				unquoted = csv_unquot(val);

			if (params->min_str[i] == NULL ||
					strcmp(unquoted, params->min_str[i]) < 0) {
				size_t len = strlen(unquoted);
				if (len + 1 > params->str_size[i]) {
					free(params->min_str[i]);
					params->min_str[i] = xmalloc_nofail(len + 1, 1);
					params->str_size[i] = len + 1;
				}
				memcpy(params->min_str[i], unquoted, len + 1);
			}

			if (val[0] == '"')
				free((char *)unquoted);
		}
	}

	return 0;
}

static void
type_not_supported(const char *type, const char *col)
{
	fprintf(stderr,
		"Type '%s', used by column '%s', is not supported by csv-min.\n",
		type, col);
	exit(2);
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	char *cols = NULL;
	bool show = false;

	params.columns = NULL;
	params.types = NULL;
	params.ncolumns = 0;

	while ((opt = getopt_long(argc, argv, "f:sv", opts, NULL)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
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

	if (!cols) {
		usage(stderr);
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

	params.columns = xmalloc_nofail(nheaders, sizeof(params.columns[0]));
	params.types = xmalloc_nofail(nheaders, sizeof(params.types[0]));

	char *col = strtok(cols, ",");
	while (col) {
		size_t idx = csv_find(headers, nheaders, col);
		if (idx == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", col);
			exit(2);
		}

		if (params.ncolumns == nheaders) {
			fprintf(stderr, "duplicated columns\n");
			exit(2);
		}

		const char *t = headers[idx].type;
		if (strcmp(t, "int") == 0)
			params.types[params.ncolumns] = TYPE_INT;
		else if (strcmp(t, "string") == 0)
			params.types[params.ncolumns] = TYPE_STRING;
		else
			type_not_supported(t, col);

		params.columns[params.ncolumns++] = idx;

		col = strtok(NULL, ",");
	}

	free(cols);

	params.min_int = xmalloc_nofail(params.ncolumns,
			sizeof(params.min_int[0]));
	params.min_str = xmalloc_nofail(params.ncolumns,
			sizeof(params.min_str[0]));
	params.str_size = xmalloc_nofail(params.ncolumns,
			sizeof(params.str_size[0]));
	for (size_t i = 0; i < params.ncolumns; ++i) {
		params.min_int[i] = LLONG_MAX;
		params.min_str[i] = NULL;
		params.str_size[i] = 0;
	}

	for (size_t i = 0; i < params.ncolumns - 1; ++i)
		printf("min(%s):%s,",
				headers[params.columns[i]].name,
				headers[params.columns[i]].type);
	printf("min(%s):%s\n",
			headers[params.columns[params.ncolumns - 1]].name,
			headers[params.columns[params.ncolumns - 1]].type);

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < params.ncolumns - 1; ++i) {
		if (params.types[i] == TYPE_INT) {
			printf("%lld,", params.min_int[i]);
		} else {
			csv_print_quoted(params.min_str[i],
					strlen(params.min_str[i]));
			putchar(',');
		}
	}

	size_t idx = params.ncolumns - 1;
	if (params.types[idx] == TYPE_INT)
		printf("%lld", params.min_int[idx]);
	else
		csv_print_quoted(params.min_str[idx],
				strlen(params.min_str[idx]));

	putchar('\n');

	free(params.columns);
	free(params.types);
	free(params.min_int);
	for (size_t i = 0; i < params.ncolumns; ++i)
		free(params.min_str[i]);
	free(params.min_str);
	free(params.str_size);

	return 0;
}
