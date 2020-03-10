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
	{"columns",	required_argument,	NULL, 'c'},
	{"reverse",	no_argument,		NULL, 'r'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"table",	required_argument,	NULL, 'T'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-cut [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input, remove and/or reorder columns and print\n"
"resulting file to standard output.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -c, --columns=NAME1[,NAME2...]\n"
"                             select only these columns\n");
	fprintf(out,
"  -r, --reverse              apply --columns filter in reverse,\n"
"                             removing only selected columns\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t *cols;
	size_t ncols;

	size_t table_column;
	char *table;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line_reordered(stdout, buf, col_offs,
					params->ncols, true, params->cols);

			return 0;
		}
	}

	csv_print_line_reordered(stdout, buf, col_offs, params->ncols, true,
			params->cols);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	char *cols = NULL;
	bool reverse = false;
	bool show = false;
	bool show_full;

	params.cols = NULL;
	params.ncols = 0;
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:rsST:", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
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
			case 'T':
				params.table = xstrdup_nofail(optarg);
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

	params.cols = xmalloc_nofail(nheaders, sizeof(params.cols[0]));

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	if (reverse) {
		for (size_t i = 0; i < nheaders; ++i)
			params.cols[i] = i;
		params.ncols = nheaders;

		char *name = strtok(cols, ",");
		while (name) {
			size_t idx = csv_find_loud(headers, nheaders,
					params.table, name);

			if (idx == CSV_NOT_FOUND)
				exit(2);

			bool found = false;
			for (size_t s = idx + 1; s > 0; s--) {
				if (params.cols[s - 1] != idx)
					continue;

				memmove(&params.cols[s - 1], &params.cols[s],
						(params.ncols - s) *
						sizeof(params.cols[0]));
				params.ncols--;
				params.cols[params.ncols] = SIZE_MAX;
				found = true;
				break;
			}

			if (!found) {
				if (params.table) {
					fprintf(stderr,
						"tried to remove column '%s%c%s' twice?\n",
						params.table, TABLE_SEPARATOR,
						name);
				} else {
					fprintf(stderr,
						"tried to remove column '%s' twice?\n",
						name);
				}
				exit(2);
			}

			name = strtok(NULL, ",");
		}
	} else {
		if (params.table)
			params.cols[params.ncols++] = params.table_column;

		char *name = strtok(cols, ",");
		while (name) {
			params.cols[params.ncols] =
				csv_find_loud(headers, nheaders, params.table,
						name);

			if (params.cols[params.ncols] == CSV_NOT_FOUND)
				exit(2);

			params.ncols++;
			name = strtok(NULL, ",");
		}

		if (params.table) {
			size_t table_len = strlen(params.table);

			for (size_t i = 0; i < nheaders; ++i) {
				const char *hname = headers[i].name;

				/* skip columns from the same table */
				if (strncmp(params.table, hname, table_len) == 0 &&
						hname[table_len] == TABLE_SEPARATOR)
					continue;

				/* skip column with the table name */
				if (i == params.cols[0])
					continue;

				params.cols[params.ncols++] = i;
			}
		}
	}

	free(cols);

	if (params.ncols > 0) {
		for (size_t i = 0; i < params.ncols - 1; ++i)
			printf("%s:%s,", headers[params.cols[i]].name,
					headers[params.cols[i]].type);
		printf("%s:%s\n", headers[params.cols[params.ncols - 1]].name,
				headers[params.cols[params.ncols - 1]].type);

		csv_read_all_nofail(s, &next_row, &params);
	}

	free(params.cols);
	free(params.table);
end:
	csv_destroy_ctx(s);

	return ret;
}
