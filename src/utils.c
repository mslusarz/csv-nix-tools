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
#include <time.h>
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
strtoll_safe(const char *str, long long *val, int base)
{
	char *end;

	errno = 0;
	long long llval = strtoll(str, &end, base);

	if (llval == LLONG_MIN && errno) {
		fprintf(stderr,
			"value '%s' is too small to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(llval));
		return -1;
	}

	if (llval == LLONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(llval));
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
strtoull_safe(const char *str, unsigned long long *val, int base)
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
	unsigned long long ullval = strtoull(str, &end, base);

	if (ullval == ULLONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit unsigned integer\n",
			str, 8 * sizeof(ullval));
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
strtol_safe(const char *str, long *val, int base)
{
	char *end;

	errno = 0;
	long lval = strtol(str, &end, base);

	if (lval == LONG_MIN && errno) {
		fprintf(stderr,
			"value '%s' is too small to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(lval));
		return -1;
	}

	if (lval == LONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(lval));
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
strtoul_safe(const char *str, unsigned long *val, int base)
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
	unsigned long ulval = strtoul(str, &end, base);

	if (ulval == ULONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit unsigned integer\n",
			str, 8 * sizeof(ulval));
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

int
strtoi_safe(const char *str, int *val, int base)
{
	long l;
	if (strtol_safe(str, &l, base))
		return -1;

	if (l < INT_MIN) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(*val));
		return -1;
	}

	if (l > INT_MAX) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(*val));
		return -1;
	}

	*val = (int)l;
	return 0;
}

int
strtou_safe(const char *str, unsigned *val, int base)
{
	unsigned long l;
	if (strtoul_safe(str, &l, base))
		return -1;

	if (l > UINT_MAX) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit unsigned integer\n",
			str, 8 * sizeof(*val));
		return -1;
	}

	*val = (unsigned)l;
	return 0;
}

char *
strnchr(const char *str, int c, size_t len)
{
	char *r = memchr(str, c, len);
	if (!r)
		return NULL;
	char *nul = memchr(str, 0, len);
	if (nul && r > nul)
		return NULL;
	return r;
}

void
print_timespec(struct timespec *ts, bool nsec)
{
	int ret;
	struct tm t;

	if (localtime_r(&ts->tv_sec, &t) == NULL)
		goto fallback;

	char buf[50];
	ret = strftime(buf, 30, "%F %T", &t);
	if (ret == 0)
		goto fallback;

	if (nsec)
		printf("%s.%09ld", buf, ts->tv_nsec);
	else
		printf("%s.%03ld", buf, ts->tv_nsec / 1000000);

	return;
fallback:
	if (nsec)
		printf("%lu.%09lu", ts->tv_sec, ts->tv_nsec);
	else
		printf("%lu.%03lu", ts->tv_sec, ts->tv_nsec / 1000000);
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
	char *n = xmalloc(len, 1);
	size_t idx = 0;

	if (!n)
		return NULL;

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

#if 0
void
csv_unquot_in_place(char *str)
{
	size_t len = strlen(str);
	size_t idx = 0;

	if (str[0] != '"' || str[len - 1] != '"') {
		fprintf(stderr, "internal error - can't unquot string that is not quoted\n");
		abort();
	}

	for (size_t i = 1; i < len - 1; ++i) {
		str[idx++] = str[i];
		if (str[i] == '"')
			++i;
	}
	str[idx] = 0;
}
#endif

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

	/* count characters from 1, like sql does */
	if (*start == 0)
		(*len)--; /* that's what sqlite does */
	else if (*start > 0)
		(*start)--;

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

char *
xstrdup(const char *str)
{
	char *ret = strdup(str);
	if (!ret)
		perror("strdup");

	return ret;
}

char *
xstrdup_nofail(const char *str)
{
	char *ret = xstrdup(str);
	if (!ret)
		exit(2);

	return ret;
}

char *
xstrndup(const char *str, size_t n)
{
	char *ret = strndup(str, n);
	if (!ret)
		perror("strndup");

	return ret;
}

char *
xstrndup_nofail(const char *str, size_t n)
{
	char *ret = xstrndup(str, n);
	if (!ret)
		exit(2);

	return ret;
}

void *
xmalloc(size_t count, size_t size)
{
	void *ret = malloc(count * size);
	if (!ret)
		perror("malloc");

	return ret;
}

void *
xmalloc_nofail(size_t count, size_t size)
{
	void *ret = xmalloc(count, size);
	if (!ret)
		exit(2);

	return ret;
}

void *
xcalloc(size_t count, size_t size)
{
	void *ret = calloc(count, size);
	if (!ret)
		perror("calloc");

	return ret;
}

void *
xcalloc_nofail(size_t count, size_t size)
{
	void *ret = xcalloc(count, size);
	if (!ret)
		exit(2);

	return ret;
}

void *
xrealloc(void *ptr, size_t count, size_t size)
{
	void *ret = realloc(ptr, count * size);
	if (!ret)
		perror("realloc");

	return ret;
}

void *
xrealloc_nofail(void *ptr, size_t count, size_t size)
{
	void *ret = xrealloc(ptr, count, size);
	if (!ret)
		exit(2);

	return ret;
}

static int
compare_columns(const void *p1, const void *p2)
{
	const struct column_info *c1 = p1;
	const struct column_info *c2 = p2;
	if (c1->order < c2->order)
		return -1;
	if (c1->order > c2->order)
		return 1;
	return 0;
}

int
csvci_parse_cols(char *cols, struct column_info *columns, size_t *ncolumns)
{
	int ret = 0;
	char *name = strtok(cols, ",");
	size_t order = 0;

	for (size_t i = 0; i < *ncolumns; ++i) {
		columns[i].vis = false;
		columns[i].order = SIZE_MAX;
	}

	while (name) {
		int found = 0;
		for (size_t i = 0; i < *ncolumns; ++i) {
			if (strcmp(name, columns[i].name) == 0) {
				columns[i].vis = true;
				columns[i].order = order++;
				found = 1;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "column %s not found\n", name);
			ret++;
		}

		name = strtok(NULL, ",");
	}

	if (ret == 0) {
		qsort(columns, *ncolumns, sizeof(columns[0]), compare_columns);
		memset(&columns[order], 0, sizeof(columns[0]) * (*ncolumns - order));
		*ncolumns = order;
	}

	return ret;
}

/* order must match the one in enum output_types */
static const char *output_types_str[] = {
	"string", "int", "string[]", "int[]",
};

void
csvci_print_header(struct column_info *columns, size_t ncolumns)
{
	for (size_t i = 0; i < ncolumns - 1; ++i) {
		printf("%s:%s,", columns[i].name,
				output_types_str[columns[i].type]);
	}

	printf("%s:%s\n", columns[ncolumns - 1].name,
			output_types_str[columns[ncolumns - 1].type]);
}

void
csvci_print_row(const void *row, const struct column_info *columns,
		size_t ncolumns)
{
	for (size_t i = 0; i < ncolumns - 1; ++i) {
		columns[i].print(row);
		putc(',', stdout);
	}

	columns[ncolumns - 1].print(row);
	putc('\n', stdout);
}
