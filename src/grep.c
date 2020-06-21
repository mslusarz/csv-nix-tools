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

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"ignore-case",		no_argument,		NULL, 'i'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"table",		required_argument,	NULL, 'T'},
	{"invert",		no_argument,		NULL, 'v'},
	{"version",		no_argument,		NULL, 'V'},
	{"whole",		no_argument,		NULL, 'x'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-grep [OPTION]...\n");
	fprintf(out,
"Searches for PATTERN in column of CSV file read from standard input and print\n"
"to standard output only rows matching that pattern.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -c NAME                    apply the filter to column NAME\n");
	fprintf(out, "  -e PATTERN                 use PATTERN as a basic regular expression pattern\n");
	fprintf(out, "  -E PATTERN                 use PATTERN as an extended regular expression pattern\n");
	fprintf(out, "  -F STRING                  use STRING as a fixed string pattern\n");
	fprintf(out, "  -i, --ignore-case          ignore case distinction\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_Table(out);
	fprintf(out,
"  -v, --invert               invert the sense of matching, selecting\n"
"                             non-matching rows\n");
	fprintf(out,
"  -x, --whole                the pattern used by -e, -E or -F options must\n"
"                             match exactly (no preceding or succeeding\n"
"                             characters before/after pattern)\n");
	describe_help(out);
	describe_version(out);
}

struct condition {
	char *column;
	char *value;
	bool ignore_case;
	bool whole;
	enum {csv_match_regexp, csv_match_eregexp, csv_match_string} type;
	size_t col_num;
	regex_t preg;
};

struct cb_params {
	struct condition *conditions;
	size_t nconditions;
	bool invert;

	char *table;
	size_t table_column;
};

static bool
matches(const char *str, const struct condition *c)
{
	if (c->type != csv_match_string)
		return regexec(&c->preg, str, 0, NULL, 0) == 0;

	bool ret;
	if (c->whole) {
		if (c->ignore_case) {
			ret = strcasecmp(str, c->value) == 0;
		} else {
			ret = strcmp(str, c->value) == 0;
		}
	} else {
		if (c->ignore_case) {
			ret = csv_strcasestr(str, c->value) != NULL;
		} else {
			ret = strstr(str, c->value) != NULL;
		}
	}

	return ret;
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t ncols,
		void *arg)
{
	struct cb_params *params = arg;
	struct condition *conditions = params->conditions;
	size_t nconditions = params->nconditions;
	bool invert = params->invert;

	if (params->table) {
		const char *table = &buf[col_offs[params->table_column]];
		if (strcmp(table, params->table) != 0) {
			csv_print_line(stdout, buf, col_offs, ncols, true);

			return 0;
		}
	}

	if (invert) {
		// !AA && !BB && !CC -> AA || BB || CC
		for (size_t i = 0; i < nconditions; ++i) {
			const struct condition *c = &conditions[i];
			const char *val = &buf[col_offs[c->col_num]];
			const char *unquoted = val;
			bool omit;

			if (val[0] == '"')
				unquoted = csv_unquot(val);

			omit = matches(unquoted, c);

			if (val[0] == '"')
				free((char *)unquoted);

			if (omit)
				return 0;
		}
	} else {
		// AA || BB || CC -> !AA && !BB && !CC
		size_t numfalse = 0;
		for (size_t i = 0; i < nconditions; ++i) {
			const struct condition *c = &conditions[i];
			const char *val = &buf[col_offs[c->col_num]];
			const char *unquoted = val;

			if (val[0] == '"')
				unquoted = csv_unquot(val);

			if (!matches(unquoted, c))
				numfalse++;

			if (val[0] == '"')
				free((char *)unquoted);
		}

		if (numfalse == nconditions)
			return 0;
	}


	csv_print_line(stdout, buf, col_offs, ncols, true);

	return 0;
}

static void
add_condition(struct condition **pconditions, size_t *pnconditions,
		const struct condition *cond)
{
	struct condition *conditions = *pconditions;
	size_t nconditions = *pnconditions;

	nconditions++;
	conditions = xrealloc_nofail(conditions, nconditions,
			sizeof(conditions[0]));

	memcpy(&conditions[nconditions - 1], cond, sizeof(*cond));

	*pnconditions = nconditions;
	*pconditions = conditions;
}

int
main(int argc, char *argv[])
{
	int opt;
	bool invert = false;
	struct condition *conditions = NULL;
	size_t nconditions = 0;
	bool show = false;
	bool show_full;
	bool ignore_case = false;
	char *current_column = NULL;
	bool whole = false;
	struct cb_params params;

	params.table = NULL;

	while ((opt = getopt_long(argc, argv, "c:e:E:F:isST:vx", opts,
			NULL)) != -1) {
		switch (opt) {
			case 'c':
				free(current_column);
				current_column = xstrdup_nofail(optarg);
				break;
			case 'e': {
				struct condition cond;

				if (!current_column) {
					fprintf(stderr, "Missing column name!\n");
					usage(stderr);
					exit(2);
				}

				cond.type = csv_match_regexp;
				cond.ignore_case = ignore_case;
				cond.whole = whole;
				cond.column = xstrdup_nofail(current_column);
				cond.value = xstrdup_nofail(optarg);

				add_condition(&conditions, &nconditions, &cond);

				break;
			}
			case 'E': {
				struct condition cond;

				if (!current_column) {
					fprintf(stderr, "Missing column name!\n");
					usage(stderr);
					exit(2);
				}

				cond.type = csv_match_eregexp;
				cond.ignore_case = ignore_case;
				cond.whole = whole;
				cond.column = xstrdup_nofail(current_column);
				cond.value = xstrdup_nofail(optarg);

				add_condition(&conditions, &nconditions, &cond);

				break;
			}
			case 'F': {
				struct condition cond;

				if (!current_column) {
					fprintf(stderr, "Missing column name!\n");
					usage(stderr);
					exit(2);
				}

				cond.type = csv_match_string;
				cond.ignore_case = ignore_case;
				cond.whole = whole;
				cond.column = xstrdup_nofail(current_column);
				cond.value = xstrdup_nofail(optarg);

				add_condition(&conditions, &nconditions, &cond);

				break;
			}
			case 'i':
				ignore_case = true;
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
			case 'v':
				invert = true;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'x':
				whole = true;
				break;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (nconditions == 0) {
		usage(stderr);
		exit(2);
	}

	free(current_column);

	if (show)
		csv_show(show_full);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (params.table) {
		params.table_column = csv_find(headers, nheaders, TABLE_COLUMN);
		if (params.table_column == CSV_NOT_FOUND) {
			fprintf(stderr, "column %s not found\n", TABLE_COLUMN);
			exit(2);
		}
	}

	for (size_t i = 0; i < nconditions; ++i) {
		struct condition *c = &conditions[i];
		c->col_num = csv_find_loud(headers, nheaders, params.table,
				c->column);
		if (c->col_num == CSV_NOT_FOUND)
			exit(2);

		if (c->type == csv_match_string)
			continue;

		char *pattern = c->value;
		if (c->whole) {
			if (csv_asprintf(&pattern, "^%s$", c->value) == -1) {
				perror("asprintf");
				exit(2);
			}
		}

		int ret = regcomp(&c->preg, pattern,
				REG_NOSUB |
				(c->ignore_case ? REG_ICASE : 0) |
				(c->type == csv_match_eregexp ? REG_EXTENDED : 0));
		if (ret) {
			size_t len = regerror(ret, &c->preg, NULL, 0);
			char *errbuf = xmalloc_nofail(len, 1);
			regerror(ret, &c->preg, errbuf, len);
			fprintf(stderr,
				"compilation of expression '%s' failed: %s\n",
				pattern, errbuf);
			free(errbuf);
			exit(2);
		}

		if (c->whole)
			free(pattern);
	}

	csv_print_header(stdout, headers, nheaders);

	params.conditions = conditions;
	params.nconditions = nconditions;
	params.invert = invert;

	csv_read_all_nofail(s, &next_row, &params);

	for (size_t i = 0; i < nconditions; ++i) {
		struct condition *c = &conditions[i];

		free(c->column);
		free(c->value);
		if (c->type != csv_match_string)
			regfree(&conditions[i].preg);
	}

	free(conditions);
	free(params.table);

	csv_destroy_ctx(s);

	return 0;
}
