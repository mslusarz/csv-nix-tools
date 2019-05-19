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
	{"field",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
	{"new-name",	required_argument,	NULL, 'n'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-exec-edit [OPTION]... -- command\n");
	printf("Options:\n");
	printf("  -f, --field=name\n");
	printf("  -n, --new-name=name\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct subst {
	size_t argv_idx;
	size_t col;
};

struct cb_params {
	char **argv;

	struct subst *substs;
	size_t nsubsts;

	size_t stdin_col;
};

static void
write_all(int fd, const char *buf, size_t len)
{
	int written = 0;

	while (len) {
		int w = write(fd, buf, len);
		if (w < 0) {
			perror("write");
			exit(2);
		}

		written += w;
		buf += w;
		len -= w;
	}
}

static size_t
read_all(int fd, char **buf, size_t *buf_len)
{
	size_t readin = 0;
	if (*buf_len < 1) {
		*buf_len = 1;
		*buf = realloc(*buf, *buf_len);
		if (!*buf) {
			perror("malloc");
			exit(2);
		}
	}

	ssize_t ret;

	ret = read(fd, (*buf) + readin, *buf_len - readin);
	while (ret > 0) {
		readin += (size_t)ret;

		if (*buf_len - readin == 0) {
			*buf_len *= 2;
			*buf = realloc(*buf, *buf_len);
			if (!*buf) {
				perror("malloc");
				exit(2);
			}
		}

		ret = read(fd, (*buf) + readin, *buf_len - readin);
	}

	if (ret < 0) {
		perror("read");
		exit(2);
	}

	return readin;
}

static char *input_buf;
static size_t input_buf_len;

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;
	int child_in[2]; /* 0=read, 1=write*/
	int child_out[2];

	if (pipe(child_in) < 0) {
		perror("pipe");
		exit(2);
	}

	if (pipe(child_out) < 0) {
		perror("pipe");
		exit(2);
	}

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

		if (close(child_in[1])) {
			perror("close");
			exit(2);
		}

		if (dup2(child_in[0], 0) < 0) {
			perror("dup2");
			exit(2);
		}

		if (close(child_out[0])) {
			perror("close");
			exit(2);
		}

		if (dup2(child_out[1], 1) < 0) {
			perror("dup2");
			exit(2);
		}

		execvp(argv[0], argv);
		perror("execvp");
		exit(2);
	}

	if (close(child_in[0])) {
		perror("close");
		exit(2);
	}

	if (close(child_out[1])) {
		perror("close");
		exit(2);
	}

	if (params->stdin_col != SIZE_MAX) {
		char *str = (char *)&buf[col_offs[params->stdin_col]];
		char *unquot = str;
		if (str[0] == '"')
			unquot = csv_unquot(str);

		write_all(child_in[1], unquot, strlen(unquot));

		if (str[0] == '"')
			free(unquot);
	}

	if (close(child_in[1])) {
		perror("close");
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

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
	fputc(',', stdout);

	size_t readin = read_all(child_out[0], &input_buf, &input_buf_len);
	csv_print_quoted(input_buf, readin);
	fputc('\n', stdout);

	if (close(child_out[0])) {
		perror("close");
		exit(2);
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	char *stdin_colname = NULL;
	char *new_colname = NULL;
	bool print_header = true;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "f:n:", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				stdin_colname = strdup(optarg);
				break;
			case 'n':
				new_colname = strdup(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage();
						return 2;
				}
				break;
			case 'h':
			default:
				usage();
				return 2;
		}
	}

	if (!new_colname) {
		fprintf(stderr, "missing -n/--new-name argument\n");
		exit(2);
	}

	size_t args = argc - optind;
	if (args == 0) {
		usage();
		exit(2);
	}

	params.argv = malloc((args + 1) * sizeof(params.argv[0]));
	if (!params.argv) {
		perror("malloc");
		exit(2);
	}
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

			params.substs = realloc(params.substs,
				++params.nsubsts * sizeof(params.substs[0]));
			if (!params.substs) {
				perror("realloc");
				exit(2);
			}

			params.substs[params.nsubsts - 1].argv_idx = idx;
			params.substs[params.nsubsts - 1].col = col;
		} else {
			params.argv[idx] = argv[i];
		}
	}

	if (stdin_colname) {
		params.stdin_col = csv_find(headers, nheaders, stdin_colname);
		if (params.stdin_col == CSV_NOT_FOUND) {
			fprintf(stderr, "column '%s' not found\n", stdin_colname);
			exit(2);
		}
	} else {
		params.stdin_col = SIZE_MAX;
	}

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i)
			printf("%s:%s,", headers[i].name, headers[i].type);
		printf("%s:string\n", new_colname);
	}
	free(new_colname);

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);

	free(params.substs);
	free(params.argv);
	free(stdin_colname);
	free(input_buf);

	return 0;
}
