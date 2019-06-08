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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-exec [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
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
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
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
	int longindex;
	struct cb_params params;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage(stderr);
						return 2;
				}
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	size_t args = argc - optind;
	if (args == 0) {
		usage(stderr);
		exit(2);
	}

	params.argv = xmalloc_nofail(args + 1, sizeof(params.argv[0]));
	params.argv[args] = NULL;

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	for (size_t i = optind; i < argc; ++i) {
		size_t idx = i - optind;

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

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	free(params.substs);
	free(params.argv);

	return 0;
}
