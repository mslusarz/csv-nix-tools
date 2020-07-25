/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "show.h"

static inline void
print_spaces(size_t s)
{
	for (size_t i = 0; i < s; ++i)
		fputc(' ', stdout);
}

static void
print_line(char *line, size_t *max_lengths, size_t columns, size_t spacing,
		enum alignment *alignments)
{
	const char *buf = line;

	for (size_t i = 0; i < columns; ++i) {
		size_t len = strlen(buf);

		if (alignments[i] == RIGHT)
			print_spaces(max_lengths[i] - len);

		fwrite(buf, len, 1, stdout);
		buf += len + 1;

		if (alignments[i] == LEFT)
			if (i < columns - 1)
				print_spaces(max_lengths[i] - len);

		if (i < columns - 1)
			print_spaces(spacing);
	}
	fputc('\n', stdout);
}

void
static_ui(bool less, struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		size_t spacing, enum alignment *alignments)
{
	if (less)  {
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
			size_t len = strlen(headers[i].name);
			if (print_types)
				len += 1 + strlen(headers[i].type);

			if (alignments[i] == RIGHT)
				print_spaces(params->max_lengths[i] - len);

			if (print_types) {
				printf("%s:%s", headers[i].name,
						headers[i].type);
			} else {
				printf("%s", headers[i].name);
			}

			if (alignments[i] == LEFT)
				if (i < nheaders - 1)
					print_spaces(params->max_lengths[i] - len);

			if (i < nheaders - 1)
				print_spaces(spacing);
		}
		fputc('\n', stdout);
	}

	for (size_t i = 0; i < params->nlines; ++i) {
		print_line(params->lines[i], params->max_lengths, nheaders,
				spacing, alignments);
		free(params->lines[i]);
	}
}
