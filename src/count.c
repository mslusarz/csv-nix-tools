/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"columns",	no_argument,		NULL, 'c'},
	{"rows",	no_argument,		NULL, 'r'},
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-count [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print back to standard output\n"
"the number of columns or rows.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c, --columns              print number of columns\n");
	fprintf(out, "  -r, --rows                 print number of rows\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	size_t rows;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(buf);
	UNUSED(col_offs);
	UNUSED(ncols);
	struct cb_params *params = arg;

	params->rows++;

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool show = false;
	bool show_full;
	bool columns = false;
	bool read_rows = false;
	bool rows = false;

	params.rows = 0;

	while ((opt = getopt_long(argc, argv, "crRsS", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				columns = true;
				break;
			case 'r':
				rows = true;
				break;
			case 'R':
				read_rows = true;
				break;
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

	if (show)
		csv_show(show_full);

	if (!columns && !rows) {
		usage(stderr);
		exit(2);
	}

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (rows || read_rows)
		csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	if (columns) {
		printf("columns:int");
		if (rows)
			putc(',', stdout);
	}

	if (rows)
		printf("rows:int");

	putc('\n', stdout);

	if (columns) {
		printf("%lu", nheaders);
		if (rows)
			putc(',', stdout);
	}

	if (rows)
		printf("%lu", params.rows);

	putc('\n', stdout);

	return 0;
}
