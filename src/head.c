/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2023, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"lines",	required_argument,	NULL, 'n'},
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
	fprintf(out, "Usage: csv-head [OPTION]...\n");
	fprintf(out, "Print the first 10 rows of CSV file from standard input to standard output.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -n, --lines=NUM            print the first NUM rows instead of the first 10,\n"
"                             NUM must be >= 0\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_no_types_in(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t lines;
	size_t printed;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (params->printed >= params->lines)
		return 1;
	params->printed++;

	csv_print_line(stdout, buf, col_offs, ncols, true);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	unsigned show_flags = SHOW_DISABLED;
	bool types = true;

	params.lines = 10;
	params.printed = 0;

	while ((opt = getopt_long(argc, argv, "n:sSX", opts, NULL)) != -1) {
		switch (opt) {
			case 'n':
				if (strtoul_safe(optarg, &params.lines, 0))
					exit(2);
				break;
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

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr, types);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	csv_print_headers(stdout, headers, nheaders);

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
