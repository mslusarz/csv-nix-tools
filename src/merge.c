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
	{"label",	required_argument,	NULL, 'l'},
	{"path",	required_argument,	NULL, 'p'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{"labeled-path",required_argument,	NULL, 'P'},
	{NULL,		0,			NULL, 0},
};

static struct input {
	FILE *f;
	struct csv_ctx *ctx;
	size_t start;
	size_t columns;

	char *label;
	size_t label_column;
} *Inputs;
static size_t Ninputs;

static size_t Nheaders;
static struct col_header *Headers;

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-merge [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -l, --label label\n");
	fprintf(out, "  -p, --path path\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --labeled-path path\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct input *in = arg;

	if (in->label) {
		fputs(in->label, stdout);
		assert(nheaders == in->columns);
	} else {
		fputs(&buf[col_offs[in->label_column]], stdout);
		assert(nheaders == in->columns + 1);
	}
	putchar(',');

	for (size_t i = 0; i < in->start; ++i)
		putchar(',');

	for (size_t i = 0; i < nheaders - 1; ++i) {
		if (i == in->label_column)
			continue;
		fputs(&buf[col_offs[i]], stdout);
		putchar(',');
	}
	if (nheaders - 1 != in->label_column)
		fputs(&buf[col_offs[nheaders - 1]], stdout);

	for (size_t i = in->start + in->columns; i < Nheaders; ++i)
		putchar(',');

	putchar('\n');

	return 0;
}

static void
add_input_with_label(const char *label, FILE *f)
{
	const struct col_header *headers_cur;
	size_t lsize = strlen(label);
	struct csv_ctx *ctx = csv_create_ctx_nofail(f, stderr);

	csv_read_header_nofail(ctx);

	size_t nheaders_cur = csv_get_headers(ctx, &headers_cur);

	Headers = xrealloc_nofail(Headers, Nheaders + nheaders_cur,
			sizeof(struct col_header));

	for (size_t i = 0; i < nheaders_cur; ++i) {
		struct col_header *h = &Headers[Nheaders + i];
		char *name = xmalloc_nofail(lsize + 1 + strlen(headers_cur[i].name) + 1, 1);
		sprintf(name, "%s%c%s", label, LABEL_SEPARATOR, headers_cur[i].name);

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
	Inputs[Ninputs].label = xstrdup_nofail(label);
	Inputs[Ninputs].label_column = SIZE_MAX;

	Ninputs++;

	Nheaders += nheaders_cur;
}

static void
add_labeled_input(FILE *f)
{
	const struct col_header *headers_cur;
	struct csv_ctx *ctx = csv_create_ctx_nofail(f, stderr);

	csv_read_header_nofail(ctx);

	size_t nheaders_cur = csv_get_headers(ctx, &headers_cur);

	Headers = xrealloc_nofail(Headers, Nheaders + nheaders_cur,
			sizeof(struct col_header));

	size_t label_idx = SIZE_MAX;
	size_t out_idx = 0;
	for (size_t i = 0; i < nheaders_cur; ++i) {
		const struct col_header *h = &headers_cur[i];
		struct col_header *nh = &Headers[Nheaders + out_idx];

		if (strcmp(h->name, LABEL_COLUMN) == 0) {
			label_idx = i;
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

	if (label_idx == SIZE_MAX) {
		fprintf(stderr, "column 'label' not found\n");
		exit(2);
	}

	Inputs = xrealloc_nofail(Inputs, Ninputs + 1, sizeof(Inputs[0]));

	Inputs[Ninputs].f = f;
	Inputs[Ninputs].ctx = ctx;
	Inputs[Ninputs].start = Nheaders;
	Inputs[Ninputs].columns = out_idx;
	Inputs[Ninputs].label = NULL;
	Inputs[Ninputs].label_column = label_idx;

	Ninputs++;

	Nheaders += out_idx;
}

static FILE *
get_file(const char *path, bool *stdin_used)
{
	if (strcmp(optarg, "-") == 0) {
		if (*stdin_used) {
			fprintf(stderr,
				"stdin is used more than once\n");
			exit(2);
		}

		*stdin_used = true;
		return stdin;
	}

	FILE *f = fopen(optarg, "r");
	if (!f) {
		fprintf(stderr,
			"opening '%s' failed: %s\n",
			optarg, strerror(errno));
		exit(2);
	}
	return f;
}

int
main(int argc, char *argv[])
{
	int opt;
	bool show = false;
	bool stdin_used = false;
	char *label = NULL;

	while ((opt = getopt_long(argc, argv, "l:p:P:s", opts, NULL)) != -1) {
		switch (opt) {
			case 'l':
				free(label);
				label = strdup(optarg);
				break;
			case 'p': {
				FILE *f = get_file(optarg, &stdin_used);
				add_input_with_label(label, f);
				break;
			}
			case 'P': {
				FILE *f = get_file(optarg, &stdin_used);
				add_labeled_input(f);
				break;
			}
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
	free(label);
	label = NULL;

	if (Nheaders == 0) {
		fprintf(stderr, "no input specified\n");
		exit(2);
	}

	if (show)
		csv_show();

	printf("%s:string,", LABEL_COLUMN);

	csv_print_header(stdout, Headers, Nheaders);

	for (size_t i = 0; i < Ninputs; ++i) {
		struct input *in = &Inputs[i];
		csv_read_all_nofail(in->ctx, &next_row, in);
		csv_destroy_ctx(in->ctx);
		free(in->label);
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
