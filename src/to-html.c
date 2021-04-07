/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

enum alignment {
	LEFT,
	RIGHT,
};

struct cb_params {
	struct split_result *split_results;
	size_t split_results_max_size;

	bool use_color_columns;
	bool *is_color_column;
	size_t *color_column_index;
	enum alignment *alignments;
};

static const struct option opts[] = {
	{"datatables",		no_argument,		NULL, 'D'},
	{"tabulator",		no_argument,		NULL, 'T'},
	{"no-header",		no_argument,		NULL, 'H'},
	{"with-types",		no_argument, 		NULL, 't'},
	{"use-color-columns",	no_argument,		NULL, 'C' },
	{"set-color",		required_argument,	NULL, 'c' },
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-to-html [OPTION]...\n");
	fprintf(out,
"Convert CSV file from standard input to HTML.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -C, --use-color-columns    use columns with _color suffix\n");
	fprintf(out, "      --datatables           use datatables.net JS library\n");
	fprintf(out, "      --tabulator            use tabulator.info JS library\n");
	fprintf(out, "      --no-header            remove column headers\n");
	fprintf(out, "      --set-color COLNAME:[fg=]COLOR1[,bg=COLOR2]\n");
	fprintf(out, "                             set COLOR1 as foreground and COLOR2 as\n");
	fprintf(out, "                             background of column COLNAME\n");
	fprintf(out, "      --with-types           print types in column headers\n");
	describe_help(out);
	describe_version(out);
}

static void
print(const char *str, size_t len)
{
	if (len == SIZE_MAX)
		len = strlen(str);

	for (size_t i = 0; i < len; ++i) {
		switch(str[i]) {
		case '<':
			printf("&lt;");
			break;
		case '>':
			printf("&gt;");
			break;
		case '&':
			printf("&amp;");
			break;
		default:
			printf("%c", str[i]);
			break;
		}
	}
}

static void
print_style(struct cb_params *params, const char *col)
{
	char *copy;

	if (col[0] == '"')
		copy = csv_unquot(col);
	else
		copy = xstrdup_nofail(col);

	size_t nresults;
	util_split_term(copy, ",", &params->split_results, &nresults,
			&params->split_results_max_size);

	for (size_t i = 0; i < nresults; ++i) {
		char *desc = copy + params->split_results[i].start;

		if (strncmp(desc, "fg=", 3) == 0)
			printf("color: %s; ", desc + 3);
		else if (strncmp(desc, "bg=", 3) == 0)
			printf("background-color: %s; ", desc + 3);
		else
			printf("color: %s; ", desc);
	}

	free(copy);
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	printf(" <tr>\n");

	size_t *color_column_index = params->color_column_index;

	for (size_t i = 0; i < ncols; ++i) {
		if (params->use_color_columns && params->is_color_column[i])
			continue;

		bool use_color = color_column_index && color_column_index[i] != SIZE_MAX;
		if (params->alignments[i] == LEFT || use_color) {
			printf("  <td style=\"");
			if (params->alignments[i] == LEFT)
				printf("text-align: left; ");
			if (use_color) {
				const char *color = buf +
						col_offs[color_column_index[i]];
				if (color[0])
					print_style(params, color);
			}
			printf("\">");
		} else {
			printf("  <td>");
		}

		const char *str = buf + col_offs[i];
		const char *unquoted = str;
		if (str[0] == '"')
			unquoted = csv_unquot(str);

		print(unquoted, SIZE_MAX);

		if (str[0] == '"')
			free((char *)unquoted);

		printf("</td>\n");
	}

	printf(" </tr>\n");

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool print_header = true;
	bool print_types = false;
	enum {
		BASIC,
		DATATABLES,
		TABULATOR,
	} mode = BASIC;

	params.split_results = NULL;
	params.split_results_max_size = 0;

	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	char **set_colorpair = NULL;
	size_t set_colorpair_num = 0;
	params.use_color_columns = false;

	while ((opt = getopt_long(argc, argv, "C", opts, NULL)) != -1) {
		switch (opt) {
			case 'D':
				mode = DATATABLES;
				break;
			case 'T':
				mode = TABULATOR;
				break;
			case 't':
				print_types = true;
				break;
			case 'c':
				set_colorpair = xrealloc_nofail(set_colorpair,
						++set_colorpair_num,
						sizeof(set_colorpair[0]));
				set_colorpair[set_colorpair_num - 1] =
						xstrdup_nofail(optarg);
				break;
			case 'C':
				params.use_color_columns = true;
				break;
			case 'H':
				print_header = false;
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

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.alignments = xmalloc_nofail(nheaders, sizeof(params.alignments[0]));
	for (size_t i = 0; i < nheaders; ++i) {
		if (strcmp(headers[i].type, "int") == 0 ||
				strcmp(headers[i].type, "float") == 0)
			params.alignments[i] = RIGHT;
		else
			params.alignments[i] = LEFT;
	}

	printf("<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n");

	if (mode == DATATABLES)
		printf("%s",
			"<link href=\"https://cdn.datatables.net/1.10.21/css/jquery.dataTables.min.css\" rel=\"stylesheet\">\n"
			"<script type=\"text/javascript\" src=\"https://code.jquery.com/jquery-3.5.1.js\"></script>\n"
			"<script type=\"text/javascript\" src=\"https://cdn.datatables.net/1.10.21/js/jquery.dataTables.min.js\"></script>\n"
			"<script type=\"text/javascript\" class=\"init\">\n"
			"$(document).ready( function () {\n"
			"    $('#myTable').DataTable();\n"
			"} );\n"
			"</script>\n");
	else if (mode == TABULATOR)
		printf("%s",
			"<link href=\"https://unpkg.com/tabulator-tables@4.7.2/dist/css/tabulator.min.css\" rel=\"stylesheet\">\n"
			"<script type=\"text/javascript\" src=\"https://code.jquery.com/jquery-3.5.1.js\"></script>\n"
			"<script type=\"text/javascript\" src=\"https://unpkg.com/tabulator-tables@4.7.2/dist/js/tabulator.min.js\"></script>\n"
			"<script type=\"text/javascript\" class=\"init\">\n"
			"$(document).ready( function () {\n"
			"    new Tabulator(\"#myTable\", {\n"
			"         layout:\"fitDataFill\",\n"
			"         pagination: \"local\",\n"
			"         paginationSize: 10,\n"
			"         paginationSizeSelector:[10, 25, 50, 100],\n"
			"      });\n"
			"} );\n"
			"</script>\n");

	printf("%s",
		"<style>\n"
		"table {\n"
		"  border-collapse: collapse;\n"
		"  width: 100%;\n"
		"}\n"
		"\n"
		"td, th {\n"
		"  border: 1px solid #bbbbbb;\n"
		"  text-align: right;\n"
		"  padding: 4px;\n"
		"}\n"
		"tr:nth-child(even) {\n"
		"  background-color: #dddddd;\n"
		"}\n");

	params.is_color_column = NULL;
	params.color_column_index = NULL;

	if (params.use_color_columns) {
		params.is_color_column = xcalloc_nofail(nheaders, sizeof(params.is_color_column[0]));
		params.color_column_index = xmalloc_nofail(nheaders, sizeof(params.color_column_index[0]));

		for (size_t i = 0; i < nheaders; ++i) {
			size_t len = strlen(headers[i].name);
			params.is_color_column[i] = len >= strlen("_color") &&
					strcmp(headers[i].name + len - strlen("_color"), "_color") == 0;

			params.color_column_index[i] = SIZE_MAX;
			for (size_t j = 0; j < nheaders; ++j) {
				if (strncmp(headers[i].name, headers[j].name, len) != 0)
					continue;
				if (strcmp(headers[j].name + len, "_color") != 0)
					continue;

				params.color_column_index[i] = j;
				break;
			}
		}
	}

	if (set_colorpair_num) {
		size_t idx = 0;
		for (size_t i = 0; i < nheaders; ++i) {
			if (params.use_color_columns && params.is_color_column[i])
				continue;

			const char *name = headers[i].name;
			size_t len = strlen(name);

			bool found = false;
			const char *spec;

			for (size_t j = 0; j < set_colorpair_num && !found; ++j) {
				spec = set_colorpair[j];
				found = strncmp(name, spec, len) == 0 && spec[len] == ':';
			}

			if (found) {
				printf("td:nth-child(%zu), th:nth-child(%zu) { ",
						idx + 1, idx + 1);
				print_style(&params, spec + len + 1);
				printf("}\n");
			}

			idx++;
		}

		for (size_t i = 0; i < set_colorpair_num; ++i)
			free(set_colorpair[i]);
		free(set_colorpair);

		set_colorpair = NULL;
		set_colorpair_num = 0;
	}

	printf("</style>\n"
		"</head>\n"
		"<body>\n"
		"<table id=\"myTable\">\n");

	if (print_header) {
		printf("<thead>\n");
		printf(" <tr>\n");
		for (size_t i = 0; i < nheaders; ++i) {
			if (params.use_color_columns && params.is_color_column[i])
				continue;

			if (params.alignments[i] == LEFT)
				printf("  <th style=\"text-align: left;\">");
			else
				printf("  <th>");

			print(headers[i].name, SIZE_MAX);

			if (print_types && headers[i].had_type) {
				printf("<span style=\"font-weight: normal\">");
				printf(" : ");
				print(headers[i].type, SIZE_MAX);
				printf("</span>");
			}

			printf("</th>\n");
		}
		printf(" </tr>\n");
		printf("</thead>\n");
	}

	printf("<tbody>\n");

	csv_read_all_nofail(s, &next_row, &params);

	printf("</tbody>\n"
		"</table>\n"
		"</body>\n"
		"</html>\n");

	free(params.is_color_column);
	free(params.color_column_index);
	free(params.alignments);
	free(params.split_results);

	csv_destroy_ctx(s);

	return 0;
}
