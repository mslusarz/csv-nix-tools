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
#include "agg.h"

static const struct option long_options[] = {
	{"separator",	required_argument,	NULL, 'e'},
	{"fields",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out, const char *name, bool read_sep)
{
	fprintf(out, "Usage: csv-%s [OPTION]...\n", name);
	fprintf(out, "Options:\n");
	if (read_sep)
		fprintf(out, "  -e, --separator=str\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

struct cb_params {
	size_t *columns;
	enum output_types *types;
	size_t ncolumns;

	agg_new_data_int cb_int;
	agg_new_data_str cb_str;
	void *state;
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

			if (params->cb_int(params->state, i, llval))
				return -1;
		} else {
			const char *unquoted = val;
			if (val[0] == '"')
				unquoted = csv_unquot(val);

			int ret = 0;
			if (params->cb_str(params->state, i, unquoted))
				ret = -1;

			if (val[0] == '"')
				free((char *)unquoted);

			if (ret)
				return ret;
		}
	}

	return 0;
}

static void
type_not_supported(const char *type, const char *col, const char *name)
{
	fprintf(stderr,
		"Type '%s', used by column '%s', is not supported by csv-%s.\n",
		type, col, name);
	exit(2);
}

int
agg_main(int argc, char *argv[],
		const char *name,
		void *state,
		agg_init_state init,
		agg_new_data_int new_data_int,
		agg_int aggregate_int,
		agg_free_state free_state,
		agg_new_data_str new_data_str,
		agg_str aggregate_str,
		bool read_sep)
{
	int opt;
	int longindex;
	struct cb_params params;
	char *cols = NULL;
	bool print_header = true;
	bool show = false;
	char *sep = NULL;

	params.columns = NULL;
	params.types = NULL;
	params.ncolumns = 0;

	const char *optstring;
	if (read_sep)
		optstring = "e:f:sv";
	else
		optstring = "f:sv";

	while ((opt = getopt_long(argc, argv, optstring, long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'e':
				if (read_sep) {
					sep = xstrdup_nofail(optarg);
				} else {
					usage(stdout, name, read_sep);
					return 2;
				}
				break;
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 's':
				show = true;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage(stderr, name, read_sep);
						return 2;
				}
				break;
			case 'h':
			default:
				usage(stdout, name, read_sep);
				return 2;
		}
	}

	if (!cols) {
		usage(stderr, name, read_sep);
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
		if (strcmp(t, "int") == 0) {
			if (new_data_int == NULL)
				type_not_supported(t, col, name);

			params.types[params.ncolumns] = TYPE_INT;
		} else if (strcmp(t, "string") == 0) {
			if (new_data_str == NULL)
				type_not_supported(t, col, name);

			params.types[params.ncolumns] = TYPE_STRING;
		} else {
			type_not_supported(t, col, name);
		}

		params.columns[params.ncolumns++] = idx;

		col = strtok(NULL, ",");
	}

	free(cols);

	init(state, params.ncolumns, sep);

	if (print_header) {
		for (size_t i = 0; i < params.ncolumns - 1; ++i)
			printf("%s(%s):%s,", name,
					headers[params.columns[i]].name,
					headers[params.columns[i]].type);
		printf("%s(%s):%s\n", name,
				headers[params.columns[params.ncolumns - 1]].name,
				headers[params.columns[params.ncolumns - 1]].type);
	}

	params.cb_int = new_data_int;
	params.cb_str = new_data_str;
	params.state = state;
	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	for (size_t i = 0; i < params.ncolumns - 1; ++i) {
		if (params.types[i] == TYPE_INT) {
			printf("%lld,", aggregate_int(state, i));
		} else {
			aggregate_str(state, i);
			putchar(',');
		}
	}

	size_t idx = params.ncolumns - 1;
	if (params.types[idx] == TYPE_INT)
		printf("%lld", aggregate_int(state, idx));
	else
		aggregate_str(state, idx);

	putchar('\n');

	free(params.columns);
	free(params.types);
	free(sep);
	free_state(state);

	return 0;
}
