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

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"table-name",		required_argument,	NULL, 'N'},
	{"path-without-table",	required_argument,	NULL, 'p'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{"path-with-table",	required_argument,	NULL, 'P'},
	{NULL,			0,			NULL, 0},
};

static struct input {
	FILE *f;
	struct csv_ctx *ctx;
	size_t start;
	size_t columns;

	char *table;
	size_t table_column;
} *Inputs;
static size_t Ninputs;

static size_t Nheaders;
static struct col_header *Headers;

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-merge [OPTION]...\n");
	fprintf(out,
"Read multiple CSV streams (from standard input or files)\n"
"and print back to standard output a merged CSV with tables.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -N, --table-name NAME      set NAME as a table name for future table-less\n"
"                             file (-p)\n");
	fprintf(out,
"  -p, --path-without-table FILE\n"
"                             read CSV stream without table from FILE and use\n"
"                             name set by -N as its name; '-' means standard\n"
"                             input\n");
	describe_Show(out);
	describe_Show_full(out);
	fprintf(out,
"      --path-with-table FILE\n"
"                             read CSV stream with table from FILE;\n"
"                             '-' means standard input\n");
	describe_help(out);
	describe_version(out);
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t ncols,
		void *arg)
{
	struct input *in = arg;

	if (in->table) {
		fputs(in->table, stdout);
		assert(ncols == in->columns);
	} else {
		fputs(&buf[col_offs[in->table_column]], stdout);
		assert(ncols == in->columns + 1);
	}
	putchar(',');

	for (size_t i = 0; i < in->start; ++i)
		putchar(',');

	for (size_t i = 0; i < ncols - 1; ++i) {
		if (i == in->table_column)
			continue;
		fputs(&buf[col_offs[i]], stdout);
		putchar(',');
	}
	if (ncols - 1 != in->table_column)
		fputs(&buf[col_offs[ncols - 1]], stdout);

	for (size_t i = in->start + in->columns; i < Nheaders; ++i)
		putchar(',');

	putchar('\n');

	return 0;
}

static void
add_nameless_input(const char *table, FILE *f)
{
	const struct col_header *headers_cur;
	size_t tsize = strlen(table);
	struct csv_ctx *ctx = csv_create_ctx_nofail(f, stderr);

	csv_read_header_nofail(ctx);

	size_t nheaders_cur = csv_get_headers(ctx, &headers_cur);

	Headers = xrealloc_nofail(Headers, Nheaders + nheaders_cur,
			sizeof(struct col_header));

	for (size_t i = 0; i < nheaders_cur; ++i) {
		struct col_header *h = &Headers[Nheaders + i];
		char *name = xmalloc_nofail(tsize + 1 + strlen(headers_cur[i].name) + 1, 1);
		sprintf(name, "%s%c%s", table, TABLE_SEPARATOR, headers_cur[i].name);

		size_t idx = csv_find(Headers, Nheaders + i, name);
		if (idx != CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' already exists\n", h->name);
			exit(2);
		}

		h->name = name;
		h->type = xstrdup_nofail(headers_cur[i].type);
	}

	Inputs = xrealloc_nofail(Inputs, Ninputs + 1, sizeof(Inputs[0]));

	Inputs[Ninputs].f = f;
	Inputs[Ninputs].ctx = ctx;
	Inputs[Ninputs].start = Nheaders;
	Inputs[Ninputs].columns = nheaders_cur;
	Inputs[Ninputs].table = xstrdup_nofail(table);
	Inputs[Ninputs].table_column = SIZE_MAX;

	Ninputs++;

	Nheaders += nheaders_cur;
}

static void
add_input(FILE *f)
{
	const struct col_header *headers_cur;
	struct csv_ctx *ctx = csv_create_ctx_nofail(f, stderr);

	csv_read_header_nofail(ctx);

	size_t nheaders_cur = csv_get_headers(ctx, &headers_cur);

	Headers = xrealloc_nofail(Headers, Nheaders + nheaders_cur,
			sizeof(struct col_header));

	size_t table_idx = SIZE_MAX;
	size_t out_idx = 0;
	for (size_t i = 0; i < nheaders_cur; ++i) {
		const struct col_header *h = &headers_cur[i];
		struct col_header *nh = &Headers[Nheaders + out_idx];

		if (strcmp(h->name, TABLE_COLUMN) == 0) {
			table_idx = i;
			continue;
		}

		nh->name = xstrdup_nofail(h->name);
		nh->type = xstrdup_nofail(h->type);

		size_t idx = csv_find(Headers, Nheaders + out_idx, h->name);
		if (idx != CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' already exists\n", h->name);
			exit(2);
		}
		out_idx++;
	}

	if (table_idx == SIZE_MAX) {
		fprintf(stderr, "column '%s' not found\n", TABLE_COLUMN);
		exit(2);
	}

	Inputs = xrealloc_nofail(Inputs, Ninputs + 1, sizeof(Inputs[0]));

	Inputs[Ninputs].f = f;
	Inputs[Ninputs].ctx = ctx;
	Inputs[Ninputs].start = Nheaders;
	Inputs[Ninputs].columns = out_idx;
	Inputs[Ninputs].table = NULL;
	Inputs[Ninputs].table_column = table_idx;

	Ninputs++;

	Nheaders += out_idx;
}

static FILE *
get_file(const char *path, bool *stdin_used)
{
	if (strcmp(path, "-") == 0) {
		if (*stdin_used) {
			fprintf(stderr,
				"stdin is used more than once\n");
			exit(2);
		}

		*stdin_used = true;
		return stdin;
	}

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr,
			"opening '%s' failed: %s\n",
			path, strerror(errno));
		exit(2);
	}
	return f;
}

int
main(int argc, char *argv[])
{
	int opt;
	bool show = false;
	bool show_full;
	bool stdin_used = false;
	char *table = NULL;

	while ((opt = getopt_long(argc, argv, "N:p:P:sS", opts, NULL)) != -1) {
		switch (opt) {
			case 'N':
				free(table);
				table = strdup(optarg);
				break;
			case 'p': {
				FILE *f = get_file(optarg, &stdin_used);
				add_nameless_input(table, f);
				break;
			}
			case 'P': {
				FILE *f = get_file(optarg, &stdin_used);
				add_input(f);
				break;
			}
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
	free(table);
	table = NULL;

	if (Nheaders == 0) {
		fprintf(stderr, "no input specified\n");
		exit(2);
	}

	if (show)
		csv_show(show_full);

	printf("%s:string,", TABLE_COLUMN);

	csv_print_header(stdout, Headers, Nheaders);

	for (size_t i = 0; i < Ninputs; ++i) {
		struct input *in = &Inputs[i];
		csv_read_all_nofail(in->ctx, &next_row, in);
		csv_destroy_ctx(in->ctx);
		free(in->table);
		fclose(in->f);
	}

	free(Inputs);

	for (size_t i = 0; i < Nheaders; ++i) {
		free((void *)Headers[i].name);
		free((void *)Headers[i].type);
	}

	free(Headers);

	return 0;
}
