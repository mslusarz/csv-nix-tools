/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2021-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht.h"
#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"filter",	required_argument,	NULL, 'f'},
	{"indent",	required_argument,	NULL, 'i'},
	{"key",		required_argument,	NULL, 'k'},
	{"level",	no_argument,		NULL, 'L'},
	{"parent",	required_argument,	NULL, 'p'},
	{"show",	no_argument,		NULL, 's'},
	{"sum",		required_argument,	NULL, 'm'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-tree [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input, build hierarchical structure and print\n"
"back data with hierarchical information\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -f, --filter=KEY           print only the matching row and its descendants\n");
	fprintf(out,
"  -i, --indent=NAME[,NEW-NAME]\n"
"                             indent data from column NAME and put it in a new\n"
"                             column NEW-NAME, if NEW-NAME is omitted column\n"
"                             NAME is replaced\n");
	fprintf(out,
"  -k, --key=NAME             use column NAME as a unique key identifying\n"
"                             each row\n");
	fprintf(out,
"  -L, --level                add 'level' column\n");
	fprintf(out,
"  -m, --sum=NAME[,NEW-NAME]  sum data from column NAME and put it in a new\n"
"                             column NEW-NAME, if NEW-NAME is omitted column\n"
"                             NAME is replaced\n");
	fprintf(out,
"  -p, --parent=NAME          use column NAME as a pointer to parent row\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	struct lines lines;
};

struct line_info {
	size_t cur_idx;
	struct line_info *parent;

	struct line_info **children_idx;
	size_t nchildren;

	long long int isum;
	double fsum;

	bool filtered;
};

static void
destroy_line_info(void *d)
{
	struct line_info *info = d;
	free(info->children_idx);
	free(info);
}

static void *
get_line_info_slow(void *not_used)
{
	(void)not_used;
	struct line_info *info = xcalloc_nofail(1, sizeof(struct line_info));
	info->fsum = 0.0;
	return info;
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	return lines_add(&params->lines, buf, col_offs, ncols);
}

static void
indent_column(struct line *line, size_t col, size_t lvl)
{
	const char *buf = line->buf;
	size_t *col_offs = line->col_offs;

	const char *str = &buf[col_offs[col]];

	if (str[0] == '"') {
		putchar('"');
		for (size_t j = 0; j < lvl * 4; ++j)
			putchar(' ');
		fputs(str + 1, stdout);
	} else {
		for (size_t j = 0; j < lvl * 4; ++j)
			putchar(' ');
		fputs(str, stdout);
	}
}

static void
print_col(struct line_info *cur_row, struct line *line, size_t col,
		size_t lvl, size_t indent_col, bool indent_replace,
		size_t sum_col, bool sum_replace, bool sum_is_int)
{
	const char *buf = line->buf;
	size_t *col_offs = line->col_offs;

	const char *str = &buf[col_offs[col]];
	if (indent_replace && col == indent_col) {
		indent_column(line, col, lvl);
	} else if (sum_replace && col == sum_col) {
		if (sum_is_int)
			printf("%lld", cur_row->isum);
		else
			printf("%f", cur_row->fsum);
	} else {
		fputs(str, stdout);
	}
}

static void
aggregate(struct lines *lines, struct line_info *cur_row, size_t sum_col,
		bool sum_is_int)
{
	struct line *line = &lines->data[cur_row->cur_idx];
	const char *buf = line->buf;
	size_t *col_offs = line->col_offs;
	const char *col = &buf[col_offs[sum_col]];

	if (sum_is_int) {
		long long llval;
		if (strtoll_safe(col, &llval, 0))
			exit(1);

		cur_row->isum = llval;

		for (size_t j = 0; j < cur_row->nchildren; ++j) {
			struct line_info *child = cur_row->children_idx[j];

			aggregate(lines, child, sum_col, sum_is_int);

			if (child->isum > 0 &&
					cur_row->isum > LLONG_MAX - child->isum) {
				fprintf(stderr, "integer overflow\n");
				exit(1);
			}

			if (child->isum < 0 &&
					cur_row->isum < LLONG_MIN - child->isum) {
				fprintf(stderr, "integer underflow\n");
				exit(1);
			}
			cur_row->isum += child->isum;
		}
	} else {
		double fval;
		if (strtod_safe(col, &fval))
			exit(1);

		cur_row->fsum = fval;

		for (size_t j = 0; j < cur_row->nchildren; ++j) {
			struct line_info *child = cur_row->children_idx[j];

			aggregate(lines, child, sum_col, sum_is_int);

			cur_row->fsum += child->fsum;
		}
	}
}

static void
process(struct lines *lines, struct line_info *cur_row, size_t ncols, size_t lvl,
		size_t indent_col, bool indent_replace,
		size_t sum_col, bool sum_replace, bool sum_is_int,
		bool filter_set, bool print, bool print_lvl)
{
	struct line *line = &lines->data[cur_row->cur_idx];
	if (filter_set && !print)
		print = cur_row->filtered;

	if (print) {
		for (size_t i = 0; i < ncols - 1; ++i) {
			print_col(cur_row, line, i, lvl, indent_col,
					indent_replace, sum_col, sum_replace,
					sum_is_int);
			putchar(',');
		}

		print_col(cur_row, line, ncols - 1, lvl, indent_col,
				indent_replace, sum_col, sum_replace,
				sum_is_int);

		if (!indent_replace && indent_col != CSV_NOT_FOUND) {
			putchar(',');

			indent_column(line, indent_col, lvl);
		}
		if (!sum_replace && sum_col != CSV_NOT_FOUND) {
			putchar(',');

			if (sum_is_int)
				printf("%lld", cur_row->isum);
			else
				printf("%f", cur_row->fsum);
		}

		if (print_lvl) {
			putchar(',');
			printf("%zu", lvl);
		}

		putchar('\n');
	}

	for (size_t j = 0; j < cur_row->nchildren; ++j) {
		process(lines, cur_row->children_idx[j], ncols, lvl + print,
			indent_col, indent_replace,
			sum_col, sum_replace, sum_is_int,
			filter_set, print, print_lvl);
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	unsigned show_flags = SHOW_DISABLED;
	char *indent = NULL;
	char *new_indent_name = NULL;
	char *key = NULL;
	char *sum = NULL;
	char *new_sum_name = NULL;
	char *parent = NULL;
	char *filter = NULL;
	bool print_lvl = false;

	lines_init(&params.lines);

	while ((opt = getopt_long(argc, argv, "f:i:k:Lm:p:sS", opts, NULL)) != -1) {
		switch (opt) {
			case 'f':
				free(filter);
				filter = xstrdup_nofail(optarg);;
				break;
			case 'i':
				free(indent);
				indent = xstrdup_nofail(optarg);
				break;
			case 'k':
				free(key);
				key = xstrdup_nofail(optarg);
				break;
			case 'L':
				print_lvl = true;
				break;
			case 'm':
				free(sum);
				sum = xstrdup_nofail(optarg);
				break;
			case 'p':
				free(parent);
				parent = xstrdup_nofail(optarg);
				break;
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
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

	if (!key || !parent) {
		fprintf(stderr, "Missing option --key or --parent\n");
		exit(1);
	}

	if (indent) {
		char *comma = index(indent, ',');
		if (comma) {
			new_indent_name = comma + 1;
			*comma = 0;
		} else {
			new_indent_name = indent;
		}
	}

	if (sum) {
		char *comma = index(sum, ',');
		if (comma) {
			new_sum_name = comma + 1;
			*comma = 0;
		} else {
			new_sum_name = sum;
		}
	}

	csv_show(show_flags);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	size_t key_col = csv_find_loud(headers, nheaders, NULL, key);
	if (key_col == CSV_NOT_FOUND)
		exit(1);

	size_t parent_col = csv_find_loud(headers, nheaders, NULL, parent);
	if (parent_col == CSV_NOT_FOUND)
		exit(1);

	size_t indent_col = CSV_NOT_FOUND;
	if (indent) {
		indent_col = csv_find_loud(headers, nheaders, NULL, indent);
		if (indent_col == CSV_NOT_FOUND)
			exit(1);
	}

	size_t sum_col = CSV_NOT_FOUND;
	const char *sum_type = NULL;
	bool sum_is_int;
	if (sum) {
		sum_col = csv_find_loud(headers, nheaders, NULL, sum);
		if (sum_col == CSV_NOT_FOUND)
			exit(1);
		sum_type = headers[sum_col].type;
		sum_is_int = strcmp(sum_type, "int") == 0;
		if (!sum_is_int && strcmp(sum_type, "float") != 0) {
			fprintf(stderr, "Column %s is not numeric\n", sum);
			exit(1);
		}
	}

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	struct lines *lines = &params.lines;

	struct line_info **line_infos;
	line_infos = xmalloc_nofail(lines->used, sizeof(line_infos[0]));

	struct csv_ht *ht = NULL;
	if (csv_ht_init(&ht, destroy_line_info, lines->used))
		exit(2);

	bool filter_found = false;
	for (size_t i = 0; i < lines->used; ++i) {
		struct line *line = &lines->data[i];
		char *key = &line->buf[line->col_offs[key_col]];

		struct line_info *cur_row = csv_ht_get_value(ht,
				key,
				get_line_info_slow,
				NULL);
		struct line_info *parent = csv_ht_get_value(ht,
				&line->buf[line->col_offs[parent_col]],
				get_line_info_slow,
				NULL);

		if (filter && !filter_found && strcmp(key, filter) == 0) {
			cur_row->filtered = true;
			filter_found = true;
		}

		line_infos[i] = cur_row;

		cur_row->cur_idx = i;
		cur_row->parent = parent;

		parent->nchildren++;
	}

	for (size_t i = 0; i < lines->used; ++i) {
		struct line_info *cur_row = line_infos[i];

		struct line_info *parent = cur_row->parent;

		if (parent->children_idx == NULL) {
			parent->children_idx = xcalloc_nofail(parent->nchildren,
					sizeof(parent->children_idx[0]));
			parent->nchildren = 0;
		}

		parent->children_idx[parent->nchildren++] = cur_row;
	}

	bool any_str_column_had_type = false;
	for (size_t i = 0; i < nheaders; ++i) {
		csv_print_header(stdout, &headers[i], i == nheaders - 1 ? 0 : ',');

		if (any_str_column_had_type)
			continue;

		if (headers[i].had_type && strcmp(headers[i].type, "string") == 0)
			any_str_column_had_type = true;
	}

	if (new_indent_name != indent) {
		if (any_str_column_had_type)
			printf(",%s:string", new_indent_name);
		else
			printf(",%s", new_indent_name);
	}

	if (new_sum_name != sum) {
		if (sum_is_int)
			printf(",%s:int", new_sum_name);
		else
			printf(",%s:float", new_sum_name);
	}

	if (print_lvl)
		printf(",level:int");

	printf("\n");

	for (size_t i = 0; i < lines->used; ++i) {
		struct line_info *cur_row = line_infos[i];

		if (cur_row->parent->parent != NULL)
			continue;

		if (sum)
			aggregate(lines, cur_row, sum_col, sum_is_int);

		process(lines, cur_row, nheaders, 0,
			indent_col, new_indent_name == indent,
			sum_col, new_sum_name == sum, sum_is_int,
			filter != NULL, filter == NULL, print_lvl);
	}

	for (size_t i = 0; i < lines->used; ++i) {
		struct line *line = &lines->data[i];

		lines_free_one(line);
	}
	lines_fini(lines);

	csv_destroy_ctx(s);

	csv_ht_destroy(&ht);
	free(line_infos);
	free(key);
	free(parent);
	free(sum);
	free(indent);
	free(filter);

	return 0;
}
