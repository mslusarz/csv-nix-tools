/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-exec [OPTION]...\n");
	fprintf(out, "Read CSV stream from standard input and execute an external command for each row.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_help(out);
	describe_version(out);
}

struct subst {
	size_t argv_idx;
	size_t col;
};

struct cb_params {
	char **argv;

	struct subst *substs;
	size_t nsubsts;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(ncols);
	struct cb_params *params = arg;

	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}

	if (pid == 0) {
		char **argv = params->argv;

		for (size_t i = 0; i < params->nsubsts; ++i) {
			struct subst *subst = &params->substs[i];
			char *d = (char *)&buf[col_offs[subst->col]];
			if (d[0] == '"')
				d = csv_unquot(d);

			argv[subst->argv_idx] = d;
		}

		execvp(argv[0], argv);
		perror("execvp");
		if (errno == ENOENT)
			fprintf(stderr, "'%s' not installed?\n", argv[0]);
		exit(2);
	}

	int wstatus;
	pid_t termindated = wait(&wstatus);
	if (termindated == -1) {
		perror("wait");
		exit(2);
	}

	if (WIFEXITED(wstatus)) {
		int status = WEXITSTATUS(wstatus);
		if (status) {
			fprintf(stderr,
				"child process terminated with status code %d\n",
				status);
			exit(3);
		}
	} else if (WIFSIGNALED(wstatus)) {
		int sig = WTERMSIG(wstatus);
		fprintf(stderr, "child process terminated by signal %d\n", sig);
		exit(4);
	} else {
		fprintf(stderr,
			"unhandled child termination type (wstatus=0x%x)\n",
			wstatus);
		exit(4);
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
		switch (opt) {
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	assert(optind >= 0);
	assert(argc >= 0);
	assert(optind <= argc);
	size_t args = (size_t)(argc - optind);
	if (args == 0) {
		usage(stderr);
		exit(2);
	}

	params.argv = xmalloc_nofail(args + 1, sizeof(params.argv[0]));
	params.argv[args] = NULL;

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	for (int i = optind; i < argc; ++i) {
		size_t idx = (size_t)(i - optind);

		if (argv[i][0] == '%') {
			size_t col = csv_find(headers, nheaders, argv[i] + 1);
			if (col == CSV_NOT_FOUND) {
				fprintf(stderr, "column '%s' not found\n",
						argv[i] + 1);
				exit(2);
			}

			params.substs = xrealloc_nofail(params.substs,
				++params.nsubsts, sizeof(params.substs[0]));

			params.substs[params.nsubsts - 1].argv_idx = idx;
			params.substs[params.nsubsts - 1].col = col;
		} else {
			params.argv[idx] = argv[i];
		}
	}

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);

	free(params.substs);
	free(params.argv);

	return 0;
}
