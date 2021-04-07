/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "utils.h"

void
csv_print_header(FILE *out, const struct col_header *header, char sep)
{
	fprintf(out, "%s", header->name);

	if (header->had_type)
		fprintf(out, ":%s", header->type);
	else
		assert(strcmp(header->type, "string") == 0);

	if (sep)
		fputc(sep, out);
}

void
csv_print_headers(FILE *out, const struct col_header *headers, size_t nheaders)
{
	for (size_t i = 0; i < nheaders - 1; ++i)
		csv_print_header(out, &headers[i], ',');
	csv_print_header(out, &headers[nheaders - 1], '\n');
}

void
csv_print_line(FILE *out, const char *buf, const size_t *col_offs,
		size_t ncols, bool nl)
{
	for (size_t i = 0; i < ncols - 1; ++i) {
		fputs(&buf[col_offs[i]], out);
		fputc(',', out);
	}
	fputs(&buf[col_offs[ncols - 1]], out);
	if (nl)
		fputc('\n', out);
}

void
csv_print_line_reordered(FILE *out, const char *buf, const size_t *col_offs,
		size_t ncols, bool nl, const size_t *idx)
{
	for (size_t i = 0; i < ncols - 1; ++i) {
		fputs(&buf[col_offs[idx[i]]], out);
		fputc(',', out);
	}
	fputs(&buf[col_offs[idx[ncols - 1]]], out);
	if (nl)
		fputc('\n', out);
}

void
csv_check_space(char **buf, size_t *buflen, size_t used, size_t len)
{
	if (used + len < *buflen)
		return;

	*buflen += len;
	*buf = xrealloc_nofail(*buf, *buflen, 1);
}

bool
csv_str_replace(const char *str, const char *pattern, const char *replacement,
		bool case_sensitive, char **buf, size_t *buflen)
{
	const char *found;
	size_t plen = strlen(pattern);
	size_t rlen = strlen(replacement);
	size_t buf_used = 0;
	bool replaced = false;

	while (1) {
		if (case_sensitive)
			found = strstr(str, pattern);
		else
			found = csv_strcasestr(str, pattern);

		if (found == NULL) {
			if (!replaced)
				return false;

			size_t slen = strlen(str);
			csv_check_space(buf, buflen, buf_used, slen + 1);
			memcpy((*buf) + buf_used, str, slen + 1);
			return true;
		}

		replaced = true;
		size_t unmatched = (uintptr_t)found - (uintptr_t)str;
		if (unmatched) {
			csv_check_space(buf, buflen, buf_used, unmatched + 1);
			memcpy((*buf) + buf_used, str, unmatched);
			buf_used += unmatched;
			str += unmatched; /* skip copied part */
		}

		str += plen; /* skip found pattern */

		if (rlen) {
			csv_check_space(buf, buflen, buf_used, rlen + 1);
			memcpy((*buf) + buf_used, replacement, rlen);
			buf_used += rlen;
		}
	}
}

int
strtoll_safe2(const char *str, long long *val, int base, bool verbose)
{
	const char *origstr = str;
	char *end;

	while (isspace(str[0]))
		str++;

	bool neg = false;
	if (base == 0 || base == 2) {
		if (str[0] == '0' && str[1] == 'b') {
			str += 2;
			base = 2;
		} else if (str[0] == '-' && str[1] == '0' && str[2] == 'b') {
			str += 3;
			base = 2;
			neg = true;
		}
	}

	errno = 0;
	long long llval = strtoll(str, &end, base);

	if (llval == LLONG_MIN && errno) {
		if (verbose) {
			fprintf(stderr,
				"value '%s' is too small to be held in %ld-bit signed integer\n",
				origstr, 8 * sizeof(llval));
		}
		return -1;
	}

	if (llval == LLONG_MAX && errno) {
		if (verbose) {
			fprintf(stderr,
				"value '%s' is too big to be held in %ld-bit signed integer\n",
				origstr, 8 * sizeof(llval));
		}
		return -1;
	}

	if (*end) {
		if (verbose) {
			fprintf(stderr,
				"value '%s' is not an integer\n", origstr);
		}
		return -1;
	}

	if (neg) {
		if (llval <= 0) {
			if (verbose) {
				fprintf(stderr,
					"value '%s' is not an integer\n", origstr);
			}
			return -1;
		}
		llval = -llval;
	}

	*val = llval;
	return 0;
}

int
strtoll_safe(const char *str, long long *val, int base)
{
	return strtoll_safe2(str, val, base, true);
}

int
strtoull_safe(const char *str, unsigned long long *val, int base)
{
	const char *origstr = str;
	while (isspace(str[0]))
		str++;

	if (*str == '-') {
		fprintf(stderr,
			"value '%s' doesn't fit in an unsigned variable\n", str);
		return -1;
	}

	if (base == 0 || base == 2) {
		if (str[0] == '0' && str[1] == 'b') {
			str += 2;
			base = 2;
		}
	}

	char *end;

	errno = 0;
	unsigned long long ullval = strtoull(str, &end, base);

	if (ullval == ULLONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit unsigned integer\n",
			origstr, 8 * sizeof(ullval));
		return -1;
	}

	if (*end) {
		fprintf(stderr,
			"value '%s' is not an integer\n", origstr);
		return -1;
	}

	*val = ullval;
	return 0;
}

int
strtol_safe(const char *str, long *val, int base)
{
	long long l;
	if (strtoll_safe(str, &l, base))
		return -1;

#if LONG_MIN != LLONG_MIN
	if (l < LONG_MIN) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(*val));
		return -1;
	}
#endif

#if LONG_MAX != LLONG_MAX
	if (l > LONG_MAX) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit signed integer\n",
			str, 8 * sizeof(*val));
		return -1;
	}
#endif

	*val = (long)l;
	return 0;
}

int
strtoul_safe(const char *str, unsigned long *val, int base)
{
	unsigned long long l;
	if (strtoull_safe(str, &l, base))
		return -1;

#if ULONG_MAX != ULLONG_MAX
	if (l > ULONG_MAX) {
		fprintf(stderr,
			"value '%s' is too big to be held in %ld-bit unsigned integer\n",
			str, 8 * sizeof(*val));
		return -1;
	}
#endif

	*val = (unsigned long)l;
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

int
strtod_safe2(const char *str, double *val, bool verbose)
{
	char *end;

	errno = 0;
	double dbl = strtod(str, &end);

	if (dbl == 0 && errno) {
		if (verbose) {
			/* should we just assume 0? */
			fprintf(stderr,
				"value '%s' caused a double underflow (%s)\n",
				str, strerror(errno));
		}
		return -1;
	}

	if ((dbl == HUGE_VAL || dbl == -HUGE_VAL) && errno) {
		if (verbose) {
			fprintf(stderr,
				"value '%s' caused a double overflow (%s)\n",
				str, strerror(errno));
		}
		return -1;
	}

	if (*end) {
		if (verbose) {
			fprintf(stderr,
				"value '%s' is not a double value\n", str);
		}
		return -1;
	}

	*val = dbl;
	return 0;
}

int
strtod_safe(const char *str, double *val)
{
	return strtod_safe2(str, val, true);
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
print_timespec(const struct timespec *ts, bool nsec)
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
csv_find(const struct col_header *headers, size_t nheaders, const char *column)
{
	for (size_t i = 0; i < nheaders; ++i) {
		if (strcmp(column, headers[i].name) == 0)
			return i;
	}

	return CSV_NOT_FOUND;
}

size_t
csv_find_by_table(const struct col_header *headers, size_t nheaders,
		const char *table, const char *column)
{
	size_t table_len = strlen(table);
	for (size_t i = 0; i < nheaders; ++i) {
		const char *hname = headers[i].name;

		if (strncmp(table, hname, table_len) != 0)
			continue;

		if (hname[table_len] != TABLE_SEPARATOR)
			continue;

		if (strcmp(column, hname + table_len + 1) == 0)
			return i;
	}

	return CSV_NOT_FOUND;
}

size_t
csv_find_loud(const struct col_header *headers, size_t nheaders,
		const char *table, const char *column)
{
	size_t ret;
	if (table)
		ret = csv_find_by_table(headers, nheaders, table, column);
	else
		ret = csv_find(headers, nheaders, column);

	if (ret == CSV_NOT_FOUND) {
		if (table) {
			fprintf(stderr, "column '%s%c%s' not found\n",
				table, TABLE_SEPARATOR, column);
		} else {
			fprintf(stderr, "column '%s' not found\n", column);
		}
	}

	return ret;
}

void
csv_column_doesnt_exist(const struct col_header *headers, size_t nheaders,
		const char *table, const char *column)
{
	size_t ret;
	if (table)
		ret = csv_find_by_table(headers, nheaders, table, column);
	else
		ret = csv_find(headers, nheaders, column);

	if (ret != CSV_NOT_FOUND) {
		if (table) {
			fprintf(stderr, "column '%s%c%s' already exists\n",
				table, TABLE_SEPARATOR, column);
		} else {
			fprintf(stderr, "column '%s' already exists\n", column);
		}

		exit(2);
	}
}

/* returns 1 when "name" was consumed, 0 otherwise */
int
csv_print_table_func_header(const struct col_header *h, const char *func,
		const char *table, size_t table_len, char sep,
		const char *name)
{
	if (table) {
		if (strncmp(h->name, table, table_len) == 0 &&
				h->name[table_len] == TABLE_SEPARATOR) {
			if (name) {
				if (h->had_type)
					printf("%s.%s:%s%c", table, name, h->type, sep);
				else
					printf("%s.%s%c", table, name, sep);
				return 1;
			} else {
				if (h->had_type)
					printf("%s.%s(%s):%s%c", table, func,
							h->name + table_len + 1,
							h->type, sep);
				else
					printf("%s.%s(%s)%c", table, func,
							h->name + table_len + 1,
							sep);
				return 0;
			}
		} else {
			/* other table or _table column*/
			if (h->had_type)
				printf("%s:%s%c", h->name, h->type, sep);
			else
				printf("%s%c", h->name, sep);
			return 0;
		}
	} else {
		if (name) {
			if (h->had_type)
				printf("%s:%s%c", name, h->type, sep);
			else
				printf("%s%c", name, sep);
			return 1;
		} else {
			if (h->had_type)
				printf("%s(%s):%s%c", func, h->name, h->type, sep);
			else
				printf("%s(%s)%c", func, h->name, sep);
			return 0;
		}
	}
}

void
csv_show(unsigned flags)
{
	int fds[2];
	pid_t pid;
	if (flags == SHOW_DISABLED)
		return;
	if ((flags & SHOW_SIMPLE) && (flags & SHOW_FULL)) {
		fprintf(stderr,
			"simple and full show can't be enabled at the same time\n");
		exit(2);
	}

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
		char *show[4];
		unsigned idx = 1;
		show[0] = "csv-show";

		if (flags & SHOW_COLORS)
			show[idx++] = "--use-color-columns";

		if (flags & SHOW_SIMPLE)
			show[idx++] = "-s";
		else if (flags & SHOW_FULL)
			show[idx++] = "-S";

		show[idx++] = NULL;

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
	size_t ustart;

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
	assert(*start >= 0);
	/* now *start is not negative */
	ustart = (size_t)*start;

	if (ustart + *len < ustart || ustart + *len >= max_len)
		*len = max_len - ustart;
}

void
util_split(const char *str, const char *sep,
		struct split_result **results_buf, size_t *nresults_buf,
		size_t *results_maxsize_buf)
{
	size_t len = strlen(str);
	size_t seplen = strlen(sep);

	struct split_result *results = *results_buf;
	size_t nresults = 0;
	size_t results_maxsize = results_maxsize_buf ? *results_maxsize_buf : 0;

	if (str[0]) {
		if (nresults + 1 > results_maxsize) {
			results_maxsize++;
			results = xrealloc_nofail(results,
					results_maxsize, sizeof(results[0]));
		}

		results[0].start = 0;
		nresults = 1;
	}

	for (size_t i = 0; i < len; ++i) {
		for (size_t j = 0; j < seplen; ++j) {
			if (str[i] != sep[j])
				continue;

			if (nresults + 1 > results_maxsize) {
				assert(results_maxsize > 0);
				results_maxsize *= 2;
				results = xrealloc_nofail(results,
						results_maxsize, sizeof(results[0]));
			}

			results[nresults].start = i + 1;
			results[nresults - 1].len = i - results[nresults - 1].start;
			nresults++;
			break;
		}
	}

	if (nresults > 0)
		results[nresults - 1].len = len - results[nresults - 1].start;

	*results_buf = results;
	*nresults_buf = nresults;
	if (results_maxsize_buf)
		*results_maxsize_buf = results_maxsize;
}

/* version of util_split that terminates all strings */
void
util_split_term(char *str, const char *sep,
		struct split_result **results_buf, size_t *nresults_buf,
		size_t *results_maxsize_buf)
{
	util_split(str, sep, results_buf, nresults_buf, results_maxsize_buf);
	struct split_result *res = *results_buf;

	for (size_t i = 0; i < *nresults_buf; ++i)
		str[res[i].start + res[i].len] = 0;
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

unsigned
util_sprintf(char *str, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vsprintf(str, format, args);
	va_end(args);
	assert(ret >= 0);
	return (unsigned)ret;
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

static void
csvci_sort_columns(struct column_info *columns, size_t *ncolumns,
		size_t new_ncolumns)
{
	qsort(columns, *ncolumns, sizeof(columns[0]), compare_columns);
	memset(&columns[new_ncolumns], 0,
			sizeof(columns[0]) * (*ncolumns - new_ncolumns));
	*ncolumns = new_ncolumns;
}

int
csvci_parse_cols(char *cols, struct column_info *columns, size_t *ncolumns)
{
	int ret = 0;
	size_t order = 0;

	for (size_t i = 0; i < *ncolumns; ++i) {
		columns[i].vis = false;
		columns[i].order = SIZE_MAX;
	}

	struct split_result *results = NULL;
	size_t nresults;
	util_split_term(cols, ",", &results, &nresults, NULL);

	for (size_t j = 0; j < nresults; ++j) {
		char *name = cols + results[j].start;

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
	}
	free(results);

	if (ret == 0)
		csvci_sort_columns(columns, ncolumns, order);

	return ret;
}

void
csvci_parse_cols_nofail(char *cols, struct column_info *columns,
		size_t *ncolumns)
{
	if (csvci_parse_cols(cols, columns, ncolumns))
		exit(2);
}

void
csvci_set_columns_order(struct column_info *columns, size_t *ncolumns)
{
	size_t order = 0;
	for (size_t i = 0; i < *ncolumns; ++i) {
		if (columns[i].vis)
			columns[i].order = order++;
		else
			columns[i].order = SIZE_MAX;
	}

	csvci_sort_columns(columns, ncolumns, order);
}

/* order must match the one in enum output_types */
static const char *output_types_str[] = {
	"string",
	"string[]",
	"int",
	"int[]",
	"float",
	"float[]",
};

void
csvci_print_header_with_prefix(struct column_info *columns,
		size_t ncolumns, const char *prefix)
{
	for (size_t i = 0; i < ncolumns - 1; ++i) {
		printf("%s%s:%s,", prefix, columns[i].name,
				output_types_str[columns[i].type]);
	}

	printf("%s%s:%s\n", prefix, columns[ncolumns - 1].name,
			output_types_str[columns[ncolumns - 1].type]);
}

void
csvci_print_header(struct column_info *columns, size_t ncolumns)
{
	csvci_print_header_with_prefix(columns, ncolumns, "");
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

void
describe_Merge(FILE *out)
{
	fprintf(out,
"  -M, --merge                merge output with a CSV stream in table form from\n"
"                             standard input\n");
}

void
describe_table_Name(FILE *out)
{
	fprintf(out, "  -N, --table-name NAME      produce output as table NAME\n");
}

void
describe_as_Table(FILE *out, const char *name)
{
	fprintf(out, "  -T, --as-table             produce output as table '%s'\n", name);
}

void
describe_Columns(FILE *out)
{
	fprintf(out, "  -c, --columns=NAME1[,NAME2...]\n");
	fprintf(out, "                             choose the list of columns\n");
}

void
describe_Show(FILE *out)
{
	fprintf(out, "  -s, --show                 print output in table format\n");
}

void
describe_Show_full(FILE *out)
{
	fprintf(out, "  -S, --show-full            print output in table format with pager\n");
}

void
describe_Table(FILE *out)
{
	fprintf(out, "  -T, --table=NAME           apply to rows only with _table column equal NAME\n");
}

void
describe_help(FILE *out)
{
	fprintf(out, "      --help                 display this help and exit\n");
}

void
describe_version(FILE *out)
{
	fprintf(out, "      --version              output version information and exit\n");
}

void
lines_init(struct lines *lines)
{
	lines->data = NULL;
	lines->size = 0;
	lines->used = 0;
}

int
lines_add(struct lines *lines, const char *buf, const size_t *col_offs,
	  size_t ncols)
{
	if (lines->used == lines->size) {
		if (lines->size == 0)
			lines->size = 16;
		else
			lines->size *= 2;

		struct line *newlines =
			xrealloc(lines->data, lines->size, sizeof(struct line));
		if (!newlines)
			return -1;

		lines->data = newlines;
	}

	struct line *line = &lines->data[lines->used];

	size_t len = col_offs[ncols - 1] + strlen(buf + col_offs[ncols - 1]) + 1;

	line->buf = xmalloc(len, 1);
	if (!line->buf)
		return -1;

	size_t col_offs_size = ncols * sizeof(col_offs[0]);
	line->col_offs = xmalloc(col_offs_size, 1);
	if (!line->col_offs) {
		free(line->buf);
		return -1;
	}

	memcpy(line->buf, buf, len);
	memcpy(line->col_offs, col_offs, col_offs_size);
	lines->used++;

	return 0;
}

void
lines_free_one(struct line *line)
{
	free(line->buf);
	free(line->col_offs);
}

void
lines_fini(struct lines *lines)
{
	free(lines->data);
}
