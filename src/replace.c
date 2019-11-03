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

/* for strcasestr */
#define _GNU_SOURCE

#include <ctype.h>
#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"no-header",		no_argument,		NULL, 'H'},
	{"ignore-case",		no_argument,		NULL, 'i'},
	{"show",		no_argument,		NULL, 's'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-replace [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -e regex\n");
	fprintf(out, "  -E eregex\n");
	fprintf(out, "  -F pattern\n");
	fprintf(out, "  -f name\n");
	fprintf(out, "  -i, --ignore-case\n");
	fprintf(out, "  -n new-name\n");
	fprintf(out, "  -r replacement\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
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
			regex_t regex;

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
};

static void
check_space(struct cb_params *params, size_t used, size_t len)
{
	if (used + len < params->buf_len)
		return;

	params->buf_len += len + 1;
	params->buf = xrealloc_nofail(params->buf, params->buf_len, 1);
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	const char *str = &buf[col_offs[params->col]];
	const char *unquoted = str;
	bool print_buf = false;
	size_t used;

	if (str[0] == '"')
		unquoted = csv_unquot(str);
	params->buf[0] = 0;

	if (params->type == csv_string) {
		const char *found;
		if (params->s.ignore_case)
			found = strcasestr(unquoted, params->s.pattern);
		else
			found = strstr(unquoted, params->s.pattern);

		print_buf = found != NULL;

		if (print_buf) {
			used = strlen(unquoted) - params->s.pattern_len
					+ params->s.replacement_len;
			check_space(params, 0, used);
			size_t pos = found - unquoted;

			memcpy(params->buf, unquoted, pos);
			memcpy(params->buf + pos, params->s.replacement,
					params->s.replacement_len);
			strcpy(params->buf + pos + params->s.replacement_len,
					unquoted + pos + params->s.pattern_len);

		}
	} else {
		print_buf = regexec(&params->r.regex, unquoted, MAX_MATCHES,
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
					check_space(params, used,
							e->const_str_len);

					memcpy(&params->buf[used], e->const_str,
							e->const_str_len);

					used += e->const_str_len;
				} else if (e->type == SUBEXPR) {
					regmatch_t *m =
						&params->r.matches[e->subexpr];
					regoff_t start = m->rm_so;
					regoff_t end = m->rm_eo;
					size_t len = end - start;

					check_space(params, used, len);

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

	csv_print_line(stdout, buf, col_offs, headers, nheaders, false);
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
			r->subexpr = str[i] - '0';

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
	int longindex;
	bool print_header = true;
	char *input_col = NULL;
	char *new_name = NULL;
	char *regex = NULL;
	char *replacement = NULL;
	struct cb_params params;
	bool show = false;
	bool ignore_case = false;

	memset(&params, 0, sizeof(params));

	while ((opt = getopt_long(argc, argv, "e:E:F:f:ir:sn:", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				input_col = xstrdup_nofail(optarg);
				break;
			case 'n':
				new_name = xstrdup_nofail(optarg);
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
			case 'r':
				replacement = xstrdup_nofail(optarg);
				break;
			case 's':
				show = true;
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

	if (!input_col || !new_name || params.type == csv_invalid || !replacement) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show();

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (csv_find(headers, nheaders, new_name) != CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' already exists in input\n",
				new_name);
		exit(2);
	}

	params.col = csv_find(headers, nheaders, input_col);
	if (params.col == CSV_NOT_FOUND) {
		fprintf(stderr, "column '%s' not found in input\n", input_col);
		exit(2);
	}

	free(input_col);
	input_col = NULL;

	if (params.type == csv_string) {
		params.s.ignore_case = ignore_case;
		params.s.replacement = replacement;
		params.s.replacement_len = strlen(replacement);
	} else {
		int ret = regcomp(&params.r.regex, regex,
				(ignore_case ? REG_ICASE : 0) |
				(params.type == csv_eregexp ? REG_EXTENDED : 0));
		if (ret) {
			size_t len = regerror(ret, &params.r.regex, NULL, 0);
			char *errbuf = xmalloc_nofail(len, 1);
			regerror(ret, &params.r.regex, errbuf, len);
			fprintf(stderr,
				"compilation of expression '%s' failed: %s\n",
				regex, errbuf);
			exit(2);
		}

		free(regex);
		parse_replacement(replacement, &params);
	}

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i)
			printf("%s:%s,", headers[i].name, headers[i].type);
		printf("%s:string\n", new_name);
	}

	free(new_name);
	new_name = NULL;

	check_space(&params, 0, 1);

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	csv_destroy_ctx(s);
	free(replacement);

	if (params.type == csv_string) {
		free(params.s.pattern);
	} else {
		regfree(&params.r.regex);

		for (size_t i = 0; i < params.r.nelements; ++i) {
			if (params.r.elements[i].type == CONST_STR)
				free(params.r.elements[i].const_str);
		}
		free(params.r.elements);
	}

	free(params.buf);

	return 0;
}
