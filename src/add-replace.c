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

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "regex_cache.h"
#include "utils.h"

static const struct option opts[] = {
	{"ignore-case",		no_argument,		NULL, 'i'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"table",		required_argument,	NULL, 'T'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-add-replace [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print it back to standard output with\n"
"a new column produced by performing string substitution either using fixed\n"
"strings or regular expression.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c NAME                    use column NAME as an input data\n");
	fprintf(out, "  -e REGEX                   use REGEX as a basic regular expression\n");
	fprintf(out, "  -E EREGEX                  use EREGEX as an extended regular expression\n");
	fprintf(out, "  -F PATTERN                 use PATTERN as a fixed string pattern\n");
	fprintf(out, "  -i, --ignore-case          perform matching ignoring case distinction\n");
	fprintf(out, "  -n NEW-NAME                create column NEW-NAME as an output\n");
	fprintf(out,
"  -r REPLACEMENT             use REPLACEMENT as an replacement for pattern;\n"
"                             for fixed string pattern it is not interpreted,\n"
"                             but for regular expression %%1 to %%9 are replaced\n"
"                             by corresponding matching sub-expression,\n"
"                             and %%0 is the whole matching expression\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	describe_help(out);
	describe_version(out);
}

struct replacement_element {
	enum {CONST_STR, SUBEXPR } type;
	union {
		struct {
			char *const_str;
			size_t const_str_len;
		};
		size_t subexpr;
	};
};

struct cb_params {
	size_t col;

	enum {csv_invalid, csv_regexp, csv_eregexp, csv_string} type;

	union {
		struct {
			regex_t *regex;

#define MAX_MATCHES 9
			regmatch_t matches[MAX_MATCHES + 1];

			struct replacement_element *elements;
			size_t nelements;
			size_t max_expr;
		} r; /* regex/eregex */

		struct {
			char *pattern;
			size_t pattern_len;
			bool ignore_case;
			char *replacement;
			size_t replacement_len;
		} s; /* string */
	};

	char *buf;
	size_t buf_len;

	size_t table_column;
	char *table;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, ncols, false);

			putchar(',');
			putchar('\n');

			return 0;
		}
	}

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	bool print_buf = false;
	size_t used;

	if (str[0] == '"')
		unquoted = csv_unquot(str);
	params->buf[0] = 0;

	if (params->type == csv_string) {
		print_buf = csv_str_replace(unquoted, params->s.pattern,
				params->s.replacement, !params->s.ignore_case,
				&params->buf, &params->buf_len);
		if (print_buf)
			used = strlen(params->buf);
	} else {
		print_buf = regexec(params->r.regex, unquoted, MAX_MATCHES,
				params->r.matches, 0) == 0;

		if (print_buf) {
			if (params->r.matches[params->r.max_expr].rm_so == -1) {
				size_t cnt = params->r.max_expr;
				while (cnt > 0 && params->r.matches[cnt].rm_so == -1)
					cnt--;

				fprintf(stderr,
					"expression %ld doesn't exist, last valid expressions number is %ld\n",
					params->r.max_expr, cnt);
				exit(2);
			}

			used = 0;
			for (size_t i = 0; i < params->r.nelements; ++i) {
				struct replacement_element *e =
					&params->r.elements[i];

				if (e->type == CONST_STR) {
					csv_check_space(&params->buf,
							&params->buf_len,
							used,
							e->const_str_len);

					memcpy(&params->buf[used], e->const_str,
							e->const_str_len);

					used += e->const_str_len;
				} else if (e->type == SUBEXPR) {
					regmatch_t *m =
						&params->r.matches[e->subexpr];
					regoff_t start = m->rm_so;
					regoff_t end = m->rm_eo;
					assert(end - start >= 0);
					size_t len = (size_t)(end - start);

					csv_check_space(&params->buf,
							&params->buf_len,
							used,
							len);

					memcpy(&params->buf[used],
						unquoted + start, len);

					used += len;
				} else {
					fprintf(stderr,
						"unknown type %d\n", e->type);
					abort();
				}
			}
		}
	}

	csv_print_line(stdout, buf, col_offs, ncols, false);
	fputc(',', stdout);

	if (print_buf)
		csv_print_quoted(params->buf, used);
	else
		printf("%s", str);

	fputc('\n', stdout);

	if (str[0] == '"')
		free((char *)unquoted);

	return 0;
}

static struct replacement_element *
new_element(struct cb_params *p)
{
	p->r.nelements++;
	p->r.elements = xrealloc_nofail(p->r.elements, p->r.nelements,
			sizeof(p->r.elements[0]));

	return &p->r.elements[p->r.nelements - 1];
}

static void
parse_replacement(const char *str, struct cb_params *p)
{
	size_t len = strlen(str);
	size_t start = 0;
	size_t end = 0;
	size_t max_match = 0;

	for (size_t i = 0; i < len; ++i) {
		if (str[i] != '%') {
			end++;
			continue;
		}

		if (start != end) {
			struct replacement_element *r = new_element(p);

			r->type = CONST_STR;
			r->const_str_len = end - start;
			r->const_str = xstrndup_nofail(&str[start],
					end - start);
			start = end;
		}

		if (i + 1 >= len) {
			fprintf(stderr, "dangling '%%'\n");
			exit(2);
		}

		if (str[i + 1] == '%') {
			i++;
			start = i;
			end = i + 1;
		} else if (isdigit(str[i + 1])) {
			struct replacement_element *r = new_element(p);

			r->type = SUBEXPR;
			i++;
			r->subexpr = (unsigned)(str[i] - '0');

			if (r->subexpr >= max_match)
				max_match = r->subexpr;

			start = i + 1;
			end = i + 1;
		} else {
			fprintf(stderr,
				"incorrect syntax - character after %% is not a digit or %%\n");
			exit(2);
		}
	}

	if (start != end) {
		struct replacement_element *r = new_element(p);

		r->type = CONST_STR;
		r->const_str_len = end - start;
		r->const_str = xstrndup_nofail(&str[start], end - start);
	}

	p->r.max_expr = max_match;
}

int
main(int argc, char *argv[])
{
	int opt;
	char *input_col = NULL;
	char *new_name = NULL;
	char *regex = NULL;
	char *replacement = NULL;
	struct cb_params params;
	bool show = false;
	bool show_full;
	bool ignore_case = false;

	memset(&params, 0, sizeof(params));
	params.table = NULL;
	params.table_column = SIZE_MAX;

	while ((opt = getopt_long(argc, argv, "c:e:E:F:in:r:sST:", opts,
			NULL)) != -1) {
		switch (opt) {
			case 'c':
				input_col = xstrdup_nofail(optarg);
				break;
			case 'e':
				regex = xstrdup_nofail(optarg);
				params.type = csv_regexp;
				break;
			case 'E':
				regex = xstrdup_nofail(optarg);
				params.type = csv_eregexp;
				break;
			case 'F':
				params.s.pattern = xstrdup_nofail(optarg);
				params.s.pattern_len = strlen(params.s.pattern);
				params.type = csv_string;
				break;
			case 'i':
				ignore_case = true;
				break;
			case 'n':
				new_name = xstrdup_nofail(optarg);
				break;
			case 'r':
				replacement = xstrdup_nofail(optarg);
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'T':
				params.table = xstrdup_nofail(optarg);
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

	if (!input_col || !new_name || params.type == csv_invalid || !replacement) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	csv_column_doesnt_exist(headers, nheaders, params.table, new_name);

	params.col = csv_find_loud(headers, nheaders, params.table, input_col);
	if (params.col == CSV_NOT_FOUND)
		exit(2);

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	free(input_col);
	input_col = NULL;

	if (params.type == csv_string) {
		params.s.ignore_case = ignore_case;
		params.s.replacement = replacement;
		params.s.replacement_len = strlen(replacement);
	} else {
		int ret = csv_regex_get(&params.r.regex, regex,
				(ignore_case ? REG_ICASE : 0) |
				(params.type == csv_eregexp ? REG_EXTENDED : 0));
		if (ret)
			exit(2);

		free(regex);
		parse_replacement(replacement, &params);
	}

	for (size_t i = 0; i < nheaders; ++i)
		printf("%s:%s,", headers[i].name, headers[i].type);

	if (params.table)
		printf("%s.", params.table);
	printf("%s:string\n", new_name);

	free(new_name);
	new_name = NULL;

	csv_check_space(&params.buf, &params.buf_len, 0, 1);

	csv_read_all_nofail(s, &next_row, &params);

	csv_destroy_ctx(s);
	free(replacement);

	if (params.type == csv_string) {
		free(params.s.pattern);
	} else {
		for (size_t i = 0; i < params.r.nelements; ++i) {
			if (params.r.elements[i].type == CONST_STR)
				free(params.r.elements[i].const_str);
		}
		free(params.r.elements);
		csv_regex_clear_cache();
	}

	free(params.buf);
	free(params.table);

	return 0;
}
