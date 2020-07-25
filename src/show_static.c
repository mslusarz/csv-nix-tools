/*
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
