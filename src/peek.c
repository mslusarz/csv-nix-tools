/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-peek [OPTION]... FORMAT [COLUMNS]...\n");
	fprintf(out,
"Read CSV stream from standard input and ...\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Show(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	FILE *side;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, ncols, true);
	csv_print_line(params->side, buf, col_offs, ncols, true);

	return 0;
}

static void
csv_peek_show(FILE **subprocess_in, FILE **subprocess_out)
{
	int subprocess_stdin[2], subprocess_stdout[2];
	pid_t pid;

	if (pipe(subprocess_stdin) < 0) {
		perror("pipe");
		exit(2);
	}

	if (pipe(subprocess_stdout) < 0) {
		perror("pipe");
		exit(2);
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}

	if (pid == 0) {
		char *show[3];

		show[0] = "csv-show";
		show[1] = "-s";
		show[2] = NULL;

		if (close(subprocess_stdin[1])) {
			perror("close");
			exit(2);
		}

		if (dup2(subprocess_stdin[0], 0) < 0) {
			perror("dup2");
			exit(2);
		}

		if (close(subprocess_stdin[0])) {
			perror("close");
			exit(2);
		}


		if (close(subprocess_stdout[0])) {
			perror("close");
			exit(2);
		}

		if (dup2(subprocess_stdout[1], 1) < 0) {
			perror("dup2");
			exit(2);
		}

		if (close(subprocess_stdout[1])) {
			perror("close");
			exit(2);
		}

		execvp(show[0], show);

		perror("execvp(csv-show)");
		if (errno == ENOENT)
			fprintf(stderr, "csv-show not installed?\n");
		exit(2);
	}

	if (close(subprocess_stdin[0])) {
		perror("close");
		exit(2);
	}

	*subprocess_in = fdopen(subprocess_stdin[1], "a");
	if (!*subprocess_in) {
		perror("fdopen");
		exit(2);
	}

	if (close(subprocess_stdout[1])) {
		perror("close");
		exit(2);
	}

	*subprocess_out = fdopen(subprocess_stdout[0], "r");
	if (!*subprocess_out) {
		perror("fdopen");
		exit(2);
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	bool show = false;
	struct cb_params params;

	while ((opt = getopt_long(argc, argv, "s", opts, NULL)) != -1) {
		switch (opt) {
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

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	FILE *out;
	if (show)
		csv_peek_show(&params.side, &out);
	else
		params.side = stderr;

	csv_print_headers(stdout, headers, nheaders);
	csv_print_headers(params.side, headers, nheaders);

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	csv_destroy_ctx(s);

	if (show) {
		/*
		 * Note: there's no point in doing this as we read from stdin,
		 * because csv-show -s requires reading everything before
		 * anything is printed.
		 */
		fclose(params.side);

		char buf[4096];
		size_t rd;
		while ((rd = fread(buf, 1, sizeof(buf), out)) > 0) {
			if (fwrite(buf, 1, rd, stderr) != rd) {
				perror("fwrite");
				exit(2);
			}
		}
		if (rd == 0 && ferror(out)) {
			perror("fread");
			exit(2);
		}

		fclose(out);
	}

	return 0;
}
