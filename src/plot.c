/*
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
#include <unistd.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"table",	required_argument,	NULL, 'T'},
	{"terminal",	required_argument,	NULL, 't'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-plot [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and output 2D or 3D gnuplot script to\n"
"standard output.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -g                         pipe to gnuplot\n");
	fprintf(out,
"  -t, --terminal TERMINAL    use TERMINAL as gnuplot's output (e.g. png, gif, dumb)\n");
	fprintf(out,
"  -x COLNAME                 use COLNAME as x axis\n");
	fprintf(out,
"  -y COLNAME                 use COLNAME as y axis\n");
	fprintf(out,
"  -z COLNAME                 use COLNAME as z axis\n");
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
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(ncols);
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0)
			return 0;
	}

	csv_print_line_reordered(stdout, buf, col_offs, params->ncols, true,
			params->cols);

	return 0;
}

static void
gnuplot(void)
{
	int fds[2];
	pid_t pid;

	if (pipe(fds) < 0) {
		perror("pipe");
		exit(2);
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}

	if (pid > 0) {
		char *plot[3];
		plot[0] = "gnuplot";
		plot[1] = NULL;

		if (close(fds[1])) {
			perror("close");
			exit(2);
		}
		if (dup2(fds[0], 0) < 0) {
			perror("dup2");
			exit(2);
		}
		if (close(fds[0])) {
			perror("close");
			exit(2);
		}
		execvp(plot[0], plot);

		perror("execvp");
		exit(2);
	}

	if (close(fds[0])) {
		perror("close");
		exit(2);
	}
	if (dup2(fds[1], 1) < 0) {
		perror("dup2");
		exit(2);
	}
	if (close(fds[1])) {
		perror("close");
		exit(2);
	}
}

static void
comment()
{
	printf("# pipe this to gnuplot or use -g option\n");
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;

	char **xcolnames = NULL;
	size_t xcols = 0;

	char **ycolnames = NULL;
	size_t ycols = 0;

	char **zcolnames = NULL;
	size_t zcols = 0;

	bool run_gnuplot = false;
	char *terminal = NULL;

	params.cols = NULL;
	params.ncols = 0;
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "gt:T:x:y:z:", opts, NULL)) != -1) {
		switch (opt) {
			case 'g':
				run_gnuplot = true;
				break;
			case 't':
				terminal = xstrdup_nofail(optarg);
				break;
			case 'T':
				params.table = xstrdup_nofail(optarg);
				break;
			case 'x':
				xcolnames = xrealloc_nofail(ycolnames, ++xcols, sizeof(xcolnames[0]));
				xcolnames[xcols - 1] = xstrdup_nofail(optarg);
				break;
			case 'y':
				ycolnames = xrealloc_nofail(ycolnames, ++ycols, sizeof(ycolnames[0]));
				ycolnames[ycols - 1] = xstrdup_nofail(optarg);
				break;
			case 'z':
				zcolnames = xrealloc_nofail(zcolnames, ++zcols, sizeof(zcolnames[0]));
				zcolnames[zcols - 1] = xstrdup_nofail(optarg);
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

	int ret = 0;

	if (xcols == 0 && ycols == 0 && zcols == 0) {
		usage(stderr);
		ret = 2;
		goto end;
	}

	if (xcols == 0) {
		fprintf(stderr, "Missing X column!\n");
		ret = 2;
		goto end;
	}
	if (xcols > 1) {
		fprintf(stderr, "X column can be used only once!\n");
		ret = 2;
		goto end;
	}

	if (ycols == 0) {
		fprintf(stderr, "Missing Y column!\n");
		ret = 2;
		goto end;
	}

	if (ycols > 1 && zcols > 0) {
		fprintf(stderr, "Only last dimension can be used more than once!\n");
		ret = 2;
		goto end;
	}

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	params.cols = xmalloc_nofail(xcols + ycols + zcols, sizeof(params.cols[0]));

	params.ncols = 0;

	for (size_t c = 0; c < xcols; ++c, ++params.ncols) {
		params.cols[params.ncols] =
			csv_find_loud(headers, nheaders, params.table, xcolnames[c]);

		if (params.cols[params.ncols] == CSV_NOT_FOUND)
			exit(2);
	}

	for (size_t c = 0; c < ycols; ++c, ++params.ncols) {
		params.cols[params.ncols] =
			csv_find_loud(headers, nheaders, params.table, ycolnames[c]);

		if (params.cols[params.ncols] == CSV_NOT_FOUND)
			exit(2);
	}

	for (size_t c = 0; c < zcols; ++c, ++params.ncols) {
		params.cols[params.ncols] =
			csv_find_loud(headers, nheaders, params.table, zcolnames[c]);

		if (params.cols[params.ncols] == CSV_NOT_FOUND)
			exit(2);
	}

	if (run_gnuplot)
		gnuplot();

	comment();

	if (terminal)
		printf("set terminal %s\n", terminal);

	printf("set xlabel '%s'\n", xcolnames[0]);
	if (ycols == 1) {
		printf("set ylabel '%s'\n", ycolnames[0]);
		if (zcols == 1)
			printf("set zlabel '%s'\n", zcolnames[0]);
	}

	printf("set datafile separator ','\n");

	printf("$data << EOD\n");

	csv_read_all_nofail(s, &next_row, &params);

	printf("EOD\n");

	if (zcols > 0) {
		printf("splot '$data' using 1:2:3 with points title ");
		if (zcols == 1)
			printf("''");
		else
			printf("'%s'", zcolnames[0]);

		for (size_t c = 1; c < zcols; ++c)
			printf(", '' using 1:2:%zu with points title '%s'", c + 3, zcolnames[c]);
	} else {
		printf("plot '$data' using 1:2 with linespoints title ");
		if (ycols == 1)
			printf("''");
		else
			printf("'%s'", ycolnames[0]);

		for (size_t c = 1; c < ycols; ++c)
			printf(", '' using 1:%zu with linespoints title '%s'", c + 2, ycolnames[c]);
	}


	printf("\n");

	printf("pause mouse close\n");

	comment();

end:
	free(terminal);

	for (size_t c = 0; c < xcols; ++c)
		free(xcolnames[c]);
	free(xcolnames);

	for (size_t c = 0; c < ycols; ++c)
		free(ycolnames[c]);
	free(ycolnames);

	for (size_t c = 0; c < zcols; ++c)
		free(zcolnames[c]);
	free(zcolnames);

	free(params.cols);
	free(params.table);
	csv_destroy_ctx(s);

	return ret;
}
