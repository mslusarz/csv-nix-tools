/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{"no-types",	no_argument,		NULL, 'X'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-cat [OPTION]... [FILE]...\n");
	fprintf(out,
"Concatenate CSV FILE(s) to standard output. With no FILE, or when FILE is -,\n"
"read standard input.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_no_types_in(out);
	describe_help(out);
	describe_version(out);
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	size_t *idx = arg;

	csv_print_line_reordered(stdout, buf, col_offs, ncols, true, idx);

	return 0;
}

struct input {
	FILE *f;
	struct csv_ctx *s;
	size_t *idx;
};

static size_t nheaders;
static const struct col_header *headers;

static void
add_input(FILE *f, struct input *in, size_t file_idx, bool types)
{
	struct csv_ctx *s = csv_create_ctx_nofail(f, stderr, types);

	csv_read_header_nofail(s);

	const struct col_header *headers_cur;
	size_t nheaders_cur = csv_get_headers(s, &headers_cur);

	in->f = f;
	in->s = s;
	in->idx = xmalloc_nofail(nheaders_cur, sizeof(in->idx[0]));

	if (headers == NULL) {
		nheaders = nheaders_cur;
		headers = headers_cur;

		for (size_t j = 0; j < nheaders; ++j)
			in->idx[j] = j;
	} else {
		if (nheaders != nheaders_cur) {
			fprintf(stderr,
				"files have different number of columns\n");
			exit(2);
		}

		for (size_t j = 0; j < nheaders; ++j) {
			size_t idx = csv_find(headers_cur, nheaders_cur,
					headers[j].name);
			if (idx == CSV_NOT_FOUND) {
				fprintf(stderr,
					"column '%s' not found in input %ld\n",
					headers[j].name, file_idx);
				exit(2);
			}

			if (strcmp(headers[j].type,
					headers_cur[idx].type) != 0) {
				fprintf(stderr,
					"column '%s' have different types in different inputs\n",
					headers[j].name);
				exit(2);
			}

			in->idx[j] = idx;
		}
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	unsigned show_flags = SHOW_DISABLED;
	bool types = true;

	while ((opt = getopt_long(argc, argv, "sSX", opts, NULL)) != -1) {
		switch (opt) {
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'X':
				types = false;
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	csv_show(show_flags);

	bool use_stdin_only = false;
	assert(optind <= argc);
	size_t ninputs = (size_t)(argc - optind);
	if (ninputs == 0) {
		ninputs++;
		use_stdin_only = true;
	}

	struct input *inputs = xcalloc_nofail(ninputs, sizeof(inputs[0]));

	if (use_stdin_only) {
		add_input(stdin, &inputs[0], 1, types);
	} else {
		bool stdin_used = false;

		size_t i = 0;
		while (optind < argc) {
			FILE *f;
			if (strcmp(argv[optind], "-") == 0) {
				if (stdin_used) {
					fprintf(stderr,
						"stdin is used more than once\n");
					exit(2);
				}
				f = stdin;
				stdin_used = true;
			} else {
				f = fopen(argv[optind], "r");
				if (!f) {
					fprintf(stderr, "opening '%s' failed: %s\n",
						argv[optind], strerror(errno));
					exit(2);
				}
			}

			add_input(f, &inputs[i], i + 1, types);

			optind++;
			i++;
		}
	}

	csv_print_headers(stdout, headers, nheaders);

	for (size_t i = 0; i < ninputs; ++i) {
		struct input *in = &inputs[i];
		csv_read_all_nofail(in->s, &next_row, in->idx);
		csv_destroy_ctx(in->s);
		free(in->idx);
		fclose(in->f);
	}

	free(inputs);

	return 0;
}
