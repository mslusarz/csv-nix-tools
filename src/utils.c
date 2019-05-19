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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

void
csv_print_header(FILE *out, const struct col_header *headers, size_t nheaders)
{
	for (size_t i = 0; i < nheaders - 1; ++i)
		fprintf(out, "%s:%s,", headers[i].name, headers[i].type);
	fprintf(out, "%s:%s\n", headers[nheaders - 1].name,
			headers[nheaders - 1].type);
}

void
csv_print_line(FILE *out, const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders, bool nl)
{
	for (size_t i = 0; i < nheaders - 1; ++i) {
		fputs(&buf[col_offs[i]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[nheaders - 1]], stdout);
	if (nl)
		fputc('\n', stdout);
}

int
strtoll_safe(const char *str, long long *val)
{
	char *end;

	errno = 0;
	long long llval = strtoll(str, &end, 0);

	if (llval == LLONG_MIN && errno) {
		fprintf(stderr,
			"value '%s' is too small\n", str);
		return -1;
	}

	if (llval == LLONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big\n", str);
		return -1;
	}

	if (*end) {
		fprintf(stderr,
			"value '%s' is not an integer\n", str);
		return -1;
	}

	*val = llval;
	return 0;
}

int
strtoull_safe(const char *str, unsigned long long *val)
{
	while (isspace(str[0]))
		str++;

	if (*str == '-') {
		fprintf(stderr,
			"value '%s' doesn't fit in an unsigned variable\n", str);
		return -1;
	}

	char *end;

	errno = 0;
	unsigned long long ullval = strtoull(str, &end, 0);

	if (ullval == ULLONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big\n", str);
		return -1;
	}

	if (*end) {
		fprintf(stderr,
			"value '%s' is not an integer\n", str);
		return -1;
	}

	*val = ullval;
	return 0;
}

int
strtol_safe(const char *str, long *val)
{
	char *end;

	errno = 0;
	long lval = strtol(str, &end, 0);

	if (lval == LONG_MIN && errno) {
		fprintf(stderr,
			"value '%s' is too small\n", str);
		return -1;
	}

	if (lval == LONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big\n", str);
		return -1;
	}

	if (*end) {
		fprintf(stderr,
			"value '%s' is not an integer\n", str);
		return -1;
	}

	*val = lval;
	return 0;
}

int
strtoul_safe(const char *str, unsigned long *val)
{
	while (isspace(str[0]))
		str++;

	if (*str == '-') {
		fprintf(stderr,
			"value '%s' doesn't fit in an unsigned variable\n", str);
		return -1;
	}

	char *end;

	errno = 0;
	unsigned long ulval = strtoul(str, &end, 0);

	if (ulval == ULONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big\n", str);
		return -1;
	}

	if (*end) {
		fprintf(stderr,
			"value '%s' is not an integer\n", str);
		return -1;
	}

	*val = ulval;
	return 0;
}

static char *
strnchr(const char *str, int c, size_t len)
{
	char *r = strchr(str, c);
	if (r >= str + len)
		return NULL;
	return r;
}

bool
csv_requires_quoting(const char *str, size_t len)
{
	const char *comma = strnchr(str, ',', len);
	const char *nl = strnchr(str, '\n', len);
	const char *quot = strnchr(str, '"', len);

	return comma || nl || quot;
}

void
csv_print_quoted(const char *str, size_t len)
{
	const char *comma = strnchr(str, ',', len);
	const char *nl = strnchr(str, '\n', len);
	const char *quot = strnchr(str, '"', len);
	if (!comma && !nl && !quot) {
		fwrite(str, 1, len, stdout);
		return;
	}
	fputc('"', stdout);
	if (!quot) {
		fwrite(str, 1, len, stdout);
		fputc('"', stdout);
		return;
	}

	do {
		size_t curlen = (uintptr_t)quot - (uintptr_t)str + 1;
		fwrite(str, 1, curlen, stdout);
		str += curlen;
		fputc('"', stdout);
		len -= curlen;
		quot = strnchr(str, '"', len);
	} while (quot);

	fwrite(str, 1, len, stdout);
	fputc('"', stdout);
}

char *
csv_unquot(const char *str)
{
	size_t len = strlen(str);
	char *n = malloc(len);
	size_t idx = 0;
	if (str[0] != '"' || str[len - 1] != '"') {
		fprintf(stderr, "internal error - can't unquot string that is not quoted\n");
		abort();
	}

	for (size_t i = 1; i < len - 1; ++i) {
		n[idx++] = str[i];
		if (str[i] == '"')
			++i;
	}
	n[idx] = 0;

	return n;
}

size_t
csv_find(const struct col_header *headers, size_t nheaders, const char *name)
{
	for (size_t i = 0; i < nheaders; ++i) {
		if (strcmp(name, headers[i].name) == 0)
			return i;
	}

	return CSV_NOT_FOUND;
}

void
csv_show(void)
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
		char *show[] = {"csv-show", NULL};

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
		execvp(show[0], show);

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

void
csv_substring_sanitize(const char *str, ssize_t *start, size_t *len)
{
	size_t max_len = strlen(str);

	if (*start < 0) {
		if (*start + (ssize_t)max_len < 0)
			*start = 0;
		else
			*start += (ssize_t)max_len;
	} else {
		if (*start >= (ssize_t)max_len)
			*start = (ssize_t)max_len;
	}

	if ((size_t)*start + *len < (size_t)*start ||
			(size_t)*start + *len >= max_len)
		*len = max_len - *start;
}
