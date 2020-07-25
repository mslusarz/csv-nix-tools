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

#endif
