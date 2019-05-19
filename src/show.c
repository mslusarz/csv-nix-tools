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

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "utils.h"

#define DEFAULT_SPACING 3

static const struct option long_options[] = {
	{"no-header",	no_argument,		NULL, 'H'},
	{"with-types",	no_argument, 		NULL, 't'},
	{"pager",	required_argument,	NULL, 'p'},
	{"spacing",	required_argument,	NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-show [OPTION]...\n");
	printf("Options:\n");
	printf("  -p, --pager yes/no/auto\n");
	printf("  -s, --spacing NUM\n");
	printf("      --with-types\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct line {
	char *buf;
	size_t *col_offs;
};

struct cb_params {
	struct line *lines;
	size_t size;
	size_t used;
	size_t *max_lengths;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	if (params->used == params->size) {
		if (params->size == 0)
			params->size = 16;
		else
			params->size *= 2;

		struct line *newlines = realloc(params->lines,
				params->size * sizeof(params->lines[0]));
		if (!newlines) {
			fprintf(stderr, "realloc: %s\n", strerror(errno));
			return -1;
		}

		params->lines = newlines;
	}

	struct line *line = &params->lines[params->used];

	size_t last_col_len = strlen(buf + col_offs[nheaders - 1]);
	size_t len = col_offs[nheaders - 1] + last_col_len + 1;

	line->buf = malloc(len);
	if (!line->buf) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		return -1;
	}

	size_t col_offs_size = nheaders * sizeof(col_offs[0]);
	line->col_offs = malloc(col_offs_size);
	if (!line->col_offs) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		free(line->buf);
		return -1;
	}

	memcpy(line->buf, buf, len);
	memcpy(line->col_offs, col_offs, col_offs_size);

	for (size_t i = 1; i < nheaders; ++i) {
		size_t len = col_offs[i] - col_offs[i - 1] - 1;
		if (len > params->max_lengths[i - 1])
			params->max_lengths[i - 1] = len;
	}
	if (last_col_len > params->max_lengths[nheaders - 1])
		params->max_lengths[nheaders - 1] = last_col_len;

	params->used++;

	return 0;
}

struct sort_params {
	const struct col_header *headers;

	size_t *columns;
	size_t ncolumns;

	struct line *lines;
	size_t nlines;
};

static void
print_line(struct line *line, size_t *max_lengths, size_t columns,
		size_t spacing)
{
	for (size_t i = 0; i < columns; ++i) {
		char *buf = &line->buf[line->col_offs[i]];
		char *unquoted = buf;

		if (buf[0] == '"')
			unquoted = csv_unquot(buf);

		size_t len = strlen(unquoted);
		fputs(unquoted, stdout);

		if (buf[0] == '"')
			free(unquoted);

		while (len++ < max_lengths[i] + spacing)
			fputc(' ', stdout);
	}
	fputc('\n', stdout);

	free(line->buf);
	free(line->col_offs);
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	bool print_header = true;
	bool print_types = false;
	int pager = -1;
	size_t spacing = DEFAULT_SPACING;

	params.lines = NULL;
	params.size = 0;
	params.used = 0;

	while ((opt = getopt_long(argc, argv, "p:s:v", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'p':
				if (strcmp(optarg, "yes") == 0)
					pager = 1;
				else if (strcmp(optarg, "no") == 0)
					pager = 0;
				else if (strcmp(optarg, "auto") == 0)
					pager = -1;
				else {
					fprintf(stderr, "Invalid value for --pager option\n");
					usage();
					exit(2);
				}
				break;
			case 's':
				if (strtoul_safe(optarg, &spacing))
					exit(2);
				break;
			case 't':
				print_types = true;
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
	if (pager == -1)
		pager = isatty(1);

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.max_lengths = calloc(nheaders, sizeof(params.max_lengths[0]));
	if (!params.max_lengths) {
		perror("calloc");
		exit(2);
	}

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i) {
			params.max_lengths[i] = strlen(headers[i].name);
			if (print_types)
				params.max_lengths[i] += 1 + strlen(headers[i].type);
		}
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	if (pager)  {
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
			char *less[] = {"less", "-S", NULL};

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
			execvp(less[0], less);

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

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i) {
			int written;

			if (print_types) {
				written = printf("%s:%s", headers[i].name,
						headers[i].type);
			} else {
				written = printf("%s", headers[i].name);
			}

			while (written++ < params.max_lengths[i] + spacing)
				fputc(' ', stdout);
		}
		fputc('\n', stdout);
	}

	for (size_t i = 0; i < params.used; ++i)
		print_line(&params.lines[i], params.max_lengths, nheaders,
				spacing);
	free(params.lines);
	free(params.max_lengths);

	csv_destroy_ctx(s);

	return 0;
}
