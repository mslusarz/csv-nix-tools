/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "show.h"
#include "utils.h"

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
print_color(const char *col)
{
	bool is_hex = false;
	if (strlen(col) == 6) {
		is_hex = true;
		for (unsigned i = 0; i < 6; ++i) {
			if (!isxdigit(col[i])) {
				is_hex = false;
				break;
			}
		}
	}
	if (is_hex)
		printf("#");
	printf("%s; ", col);
}

static void
print_style(struct cb_params *params, const char *col)
{
	char *copy = xstrdup_nofail(col);
	size_t nresults;
	util_split_term(copy, ",", &params->split_results, &nresults,
			&params->split_results_max_size);

	for (size_t i = 0; i < nresults; ++i) {
		char *desc = copy + params->split_results[i].start;

		if (strncmp(desc, "fg=", 3) == 0) {
			printf("color: ");
			print_color(desc + 3);
		} else if (strncmp(desc, "bg=", 3) == 0) {
			printf("background-color: ");
			print_color(desc + 3);
		} else {
			printf("color: ");
			print_color(desc);
		}
	}
	free(copy);
}

void
html_ui(struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		enum alignment *alignments,
		char **set_colorpair, size_t set_colorpairs_num,
		bool use_color_columns,
		enum html_mode mode)
{
	printf("%s",
		"<!DOCTYPE html>\n"
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

	bool *is_color_column = NULL;
	size_t *color_column_index = NULL;
	size_t *col_offsets = xmalloc_nofail(nheaders + 1, sizeof(col_offsets[0]));

	if (use_color_columns) {
		is_color_column = xcalloc_nofail(nheaders, sizeof(is_color_column[0]));
		color_column_index = xmalloc_nofail(nheaders, sizeof(color_column_index[0]));

		for (size_t i = 0; i < nheaders; ++i) {
			size_t len = strlen(headers[i].name);
			is_color_column[i] = len >= strlen("_color") &&
					strcmp(headers[i].name + len - strlen("_color"), "_color") == 0;

			color_column_index[i] = SIZE_MAX;
			for (size_t j = 0; j < nheaders; ++j) {
				if (strncmp(headers[i].name, headers[j].name, len) != 0)
					continue;
				if (strcmp(headers[j].name + len, "_color") != 0)
					continue;

				color_column_index[i] = j;
				break;
			}
		}
	}

	if (set_colorpairs_num) {
		size_t idx = 0;
		for (size_t i = 0; i < nheaders; ++i) {
			if (use_color_columns && is_color_column[i])
				continue;

			const char *name = headers[i].name;
			size_t len = strlen(name);

			bool found = false;
			const char *spec;

			for (size_t j = 0; j < set_colorpairs_num && !found; ++j) {
				spec = set_colorpair[j];
				found = strncmp(name, spec, len) == 0 && spec[len] == ':';
			}

			if (found) {
				printf("td:nth-child(%zu), th:nth-child(%zu) { ",
						idx + 1, idx + 1);
				print_style(params, spec + len + 1);
				printf("}\n");
			}

			idx++;
		}
	}

	printf("%s",
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<table id=\"myTable\">\n");

	if (print_header) {
		printf("<thead>\n");
		printf(" <tr>\n");
		for (size_t i = 0; i < nheaders; ++i) {
			if (use_color_columns && is_color_column[i])
				continue;

			if (alignments[i] == LEFT)
				printf("  <th style=\"text-align: left;\">");
			else
				printf("  <th>");

			print(headers[i].name, SIZE_MAX);

			if (print_types) {
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
	for (size_t l = 0; l < params->nlines; ++l) {
		printf(" <tr>\n");

		const char *buf = params->lines[l];
		col_offsets[0] = 0;
		for (size_t i = 1; i < nheaders + 1; ++i) {
			size_t len = strlen(buf + col_offsets[i - 1]);
			col_offsets[i] = col_offsets[i - 1] + len + 1;
		}

		for (size_t i = 0; i < nheaders; ++i) {
			size_t len = strlen(buf);

			if (use_color_columns && is_color_column[i]) {
				buf += len + 1;
				continue;
			}

			bool use_color = color_column_index && color_column_index[i] != SIZE_MAX;
			if (alignments[i] == LEFT || use_color) {
				printf("  <td style=\"");
				if (alignments[i] == LEFT)
					printf("text-align: left; ");
				if (use_color) {
					const char *col = params->lines[l] +
							col_offsets[color_column_index[i]];
					if (col[0])
						print_style(params, col);
				}
				printf("\">");
			} else {
				printf("  <td>");
			}

			print(buf, len);
			buf += len + 1;

			printf("</td>\n");
		}

		free(params->lines[l]);
		printf(" </tr>\n");
	}
	printf("</tbody>\n");
	printf("%s",
		"</table>\n"
		"</body>\n"
		"</html>\n");

	free(is_color_column);
	free(color_column_index);
	free(col_offsets);
}
