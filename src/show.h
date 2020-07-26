/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_SHOW_H
#define CSV_SHOW_H

#include <stdbool.h>
#include <stddef.h>
#include "ht.h"
#include "parse.h"

enum alignment {
	LEFT,
	RIGHT,
};

struct cb_params {
	char **lines; /* array of lines */
	size_t allocated_lines; /* number of allocated line slots */
	size_t nlines; /* number of lines of data */
	size_t *max_lengths; /* maximum number of characters in each column */

	char *tmpbuf;
	size_t tmpbuf_size;

	int logfd;

	struct csv_ht *color_pairs;
	struct csv_ht *colors;
	int used_pairs;
	int used_colors;
	int *col_color_pairs;
};

#ifdef NCURSESW_ENABLED
void curses_ui(struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		size_t spacing, enum alignment *alignments,
		char **set_colorpair, size_t set_colorpairs_num,
		bool use_color_columns);
void curses_ui_exit();
#endif

void static_ui(bool less, struct cb_params *params,
		const struct col_header *headers, size_t nheaders,
		bool print_header, bool print_types,
		size_t spacing, enum alignment *alignments);

enum html_mode {
	BASIC,
	DATATABLES,
	TABULATOR,
};

void html_ui(struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		enum alignment *alignments,
		char **set_colorpair, size_t set_colorpairs_num,
		bool use_color_columns,
		enum html_mode mode);

#endif
