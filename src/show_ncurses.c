/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <curses.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ht.h"
#include "show.h"
#include "utils.h"

struct slow_params {
	struct cb_params *params;
	const char *txt;
};

static int
h2i(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	return c - 'A' + 10;
}

static void *
get_color_slow(void *p_)
{
	struct slow_params *p = p_;
	int color;
	const char *col = p->txt;

	if (strcmp(col, "black") == 0)
		color = COLOR_BLACK;
	else if (strcmp(col, "red") == 0)
		color = COLOR_RED;
	else if (strcmp(col, "green") == 0)
		color = COLOR_GREEN;
	else if (strcmp(col, "yellow") == 0)
		color = COLOR_YELLOW;
	else if (strcmp(col, "blue") == 0)
		color = COLOR_BLUE;
	else if (strcmp(col, "magenta") == 0)
		color = COLOR_MAGENTA;
	else if (strcmp(col, "cyan") == 0)
		color = COLOR_CYAN;
	else if (strcmp(col, "white") == 0)
		color = COLOR_WHITE;
	else if (strcmp(col, "default") == 0)
		color = -1;
	else if (strlen(col) == 6) {
		if (!can_change_color()) {
			endwin();
			fprintf(stderr, "Your terminal doesn't support changing colors\n");
			exit(2);
		}

		if (p->params->used_colors >= COLORS) {
			endwin();
			fprintf(stderr, "reached maximum number of colors %d\n",
					p->params->used_colors);
			exit(2);
		}

		color = p->params->used_colors++;

		for (unsigned i = 0; i < 6; ++i)
			if (!((col[i] >= '0' && col[i] <= '9') ||
			      (col[i] >= 'A' && col[i] <= 'F')))
				goto err;
		int r = h2i(col[0]) * 16 + h2i(col[1]);
		int g = h2i(col[2]) * 16 + h2i(col[3]);
		int b = h2i(col[4]) * 16 + h2i(col[5]);

		r = (int)(1000.0 * r / 255.0);
		g = (int)(1000.0 * g / 255.0);
		b = (int)(1000.0 * b / 255.0);

		init_color(color, r, g, b);
	} else if (strtoi_safe(col, &color, 10) == 0) {
		;
	} else {
		goto err;
	}

	/* we have to allocate, because COLOR_BLACK == 0 (and 0 casted to
	 * void * is NULL, which is considered an error by csv_ht_get_value) */
	int *color_ptr = xmalloc_nofail(1, sizeof(*color_ptr));
	*color_ptr = color;

	return color_ptr;
err:
	endwin();
	fprintf(stderr,
		"Can't parse '%s' as color - it must be either a predefined name, in RRGGBB format or be a decimal number\n",
		col);
	exit(2);
}

static int
get_color(struct cb_params *params, char *txt)
{
	struct slow_params p;
	p.params = params;
	p.txt = txt;
	return *(int *)csv_ht_get_value(params->colors, txt, get_color_slow, &p);
}

static void *
get_color_pair_slow(void *p_)
{
	struct slow_params *p = p_;
	int fg = -1, bg = -1;
	int color_pair;
	char *txt = xstrdup_nofail(p->txt);

	char *desc = strtok(txt, ",");
	while (desc) {
		if (strncmp(desc, "fg=", 3) == 0)
			fg = get_color(p->params, desc + 3);
		else if (strncmp(desc, "bg=", 3) == 0)
			bg = get_color(p->params, desc + 3);
		else
			fg = get_color(p->params, desc);

		desc = strtok(NULL, ",");
	}
	free(txt);

	if (p->params->used_pairs >= COLOR_PAIRS - 1) {
		endwin();
		fprintf(stderr, "reached maximum number of color pairs %d\n",
				p->params->used_pairs);
		exit(2);
	}

	color_pair = p->params->used_pairs++;
	init_pair(color_pair, fg, bg);

	return (void *)(intptr_t)color_pair;
}

static int
get_color_pair(struct cb_params *params, char *txt)
{
	struct slow_params p;
	p.params = params;
	p.txt = txt;
	return (int)(intptr_t)csv_ht_get_value(params->color_pairs, txt,
			get_color_pair_slow, &p);
}

static void
nprint(int y, int x, const char *str, bool *truncated)
{
	size_t len = strlen(str);
	*truncated = false;

	if (x < 0) {
		/* no part of the string will be visible */
		if (len < -x)
			return;

		str += -x;
		len -= -x;
		x = 0;
	}

	if (x + len >= COLS) {
		len = COLS - x;
		*truncated = true;
	}

	mvaddnstr(y, x, str, len);
}

static void
show(struct cb_params *params,
	const struct col_header *headers, size_t nheaders,

	size_t first_line, /* index of the first line to show */
	size_t nlines, /* number of lines to show */
	int xoff,
	size_t spacing,
	bool print_header, bool print_types,
	enum alignment *alignments,
	bool use_color_columns,
	bool *is_color_column,
	size_t *color_column_index,
	size_t *col_offsets)
{
	char **data = params->lines;
	size_t *max_lengths = params->max_lengths;
	int *col_color_pairs = params->col_color_pairs;
	size_t ypos = 0;

	clear();

	if (print_header) {
		int xpos = 0;

		attron(A_BOLD);
		for (size_t i = 0; i < nheaders; ++i) {
			int prev_xpos = xpos;
			bool truncated;

			if (use_color_columns && is_color_column[i])
				continue;

			if (col_color_pairs[i] != -1)
				attron(COLOR_PAIR(col_color_pairs[i]));

			size_t len = strlen(headers[i].name);
			if (print_types)
				len += 1 + strlen(headers[i].type);

			if (alignments[i] == RIGHT)
				for (size_t j = 0; j < max_lengths[i] - len; ++j)
					mvaddch(ypos, xpos - xoff + j, ' ');

			if (alignments[i] == RIGHT)
				xpos += max_lengths[i] - len;

			nprint(0, xpos - xoff, headers[i].name, &truncated);
			if (truncated)
				break;
			xpos += strlen(headers[i].name);

			if (print_types) {
				attroff(A_BOLD);
				nprint(0, xpos - xoff, ":", &truncated);
				if (truncated)
					break;
				xpos++;

				nprint(0, xpos - xoff, headers[i].type,
						&truncated);
				if (truncated)
					break;
				xpos += strlen(headers[i].type);
				attron(A_BOLD);
			}

			if (alignments[i] == LEFT)
				for (size_t j = 0; j < max_lengths[i] - len; ++j)
					mvaddch(ypos, xpos - xoff + j, ' ');

			if (i < nheaders - 1)
				xpos = prev_xpos + max_lengths[i] + spacing;

			if (col_color_pairs[i] != -1)
				attroff(COLOR_PAIR(col_color_pairs[i]));
		}
		attroff(A_BOLD);

		ypos++;
	}

	for (size_t ln = first_line; ln < first_line + nlines; ++ln, ++ypos) {
		char *buf = data[ln];
		int xpos = 0;

		col_offsets[0] = 0;
		for (size_t i = 1; i < nheaders + 1; ++i) {
			size_t len = strlen(buf + col_offsets[i - 1]);
			col_offsets[i] = col_offsets[i - 1] + len + 1;
		}

		for (size_t i = 0; i < nheaders; ++i) {
			bool truncated;

			size_t len = col_offsets[i + 1] - col_offsets[i] - 1;

			if (use_color_columns && is_color_column[i])
				continue;

			int colpair = col_color_pairs[i];

			if (use_color_columns && color_column_index[i] != SIZE_MAX) {
				char *col = buf + col_offsets[color_column_index[i]];
				if (col[0])
					colpair = get_color_pair(params, col);
			}

			if (colpair != -1)
				attron(COLOR_PAIR(colpair));

			if (alignments[i] == RIGHT)
				for (size_t j = 0; j < max_lengths[i] - len; ++j)
					mvaddch(ypos, xpos - xoff + j, ' ');

			if (alignments[i] == RIGHT)
				xpos += max_lengths[i] - len;

			nprint(ypos, xpos - xoff, buf + col_offsets[i], &truncated);

			if (truncated) {
				if (colpair != -1)
					attroff(COLOR_PAIR(colpair));
				break;
			}

			if (alignments[i] == LEFT)
				for (size_t j = len; j < max_lengths[i]; ++j)
					mvaddch(ypos, xpos - xoff + j, ' ');

			xpos += len;

			if (alignments[i] == LEFT)
				if (i < nheaders - 1)
					xpos += max_lengths[i] - len;

			if (i < nheaders - 1)
				xpos += spacing;

			if (colpair != -1)
				attroff(COLOR_PAIR(colpair));
		}
	}
}

void
curses_ui(struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		size_t spacing, enum alignment *alignments,
		char **set_colorpair, size_t set_colorpairs_num,
		bool use_color_columns)
{
	if (close(0)) {
		perror("close");
		exit(2);
	}

	char *tty = ttyname(1);
	if (!tty) {
		perror("ttyname");
		exit(2);
	}

	int fd = open(tty, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(2);
	}

	if (fd != 0) /* impossible */
		abort();

	params->used_pairs = 1;
	params->used_colors = COLOR_WHITE + 1;

	initscr();

	params->col_color_pairs = xmalloc_nofail(nheaders,
			sizeof(params->col_color_pairs[0]));
	for (size_t i = 0; i < nheaders; ++i)
		params->col_color_pairs[i] = -1;

	if (set_colorpairs_num || use_color_columns) {
		if (!has_colors()) {
			endwin();
			fprintf(stderr, "Your terminal doesn't support colors\n");
			exit(2);
		}

		start_color();
		use_default_colors();

		if (csv_ht_init(&params->color_pairs, NULL)) {
			endwin();
			exit(2);
		}
		if (csv_ht_init(&params->colors, free)) {
			endwin();
			exit(2);
		}

		for (size_t i = 0; i < set_colorpairs_num; ++i) {
			char *x = strtok(set_colorpair[i], ":");
			if (!x) {
				endwin();
				fprintf(stderr, "Color not in name:spec format\n");
				exit(2);
			}
			size_t idx = csv_find_loud(headers, nheaders, NULL, x);
			if (idx == CSV_NOT_FOUND) {
				endwin();
				exit(2);
			}
			x = strtok(NULL, ":");
			if (!x) {
				endwin();
				fprintf(stderr, "Color not in name:spec format\n");
				exit(2);
			}

			params->col_color_pairs[idx] = get_color_pair(params, x);
		}
	}

	bool *is_color_column = NULL;
	size_t *color_column_index = NULL;
	size_t *col_offsets_buf = xmalloc_nofail(nheaders + 1, sizeof(col_offsets_buf[0]));

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

	cbreak();
	noecho();
	clear();
	keypad(stdscr, TRUE);
	curs_set(0);

	int ch = 0;
	size_t first_line = 0;
	size_t nlines; /* number of lines to show */
	int xoff = 0;
#define STEP (COLS / 2)

	size_t max_line = 0;
	for (size_t i = 0; i < nheaders; ++i)
		max_line += params->max_lengths[i] + spacing;
	if (max_line >= INT_MAX) {
		fprintf(stderr, "max line length exceeds %d\n", INT_MAX);
		exit(2);
	}

	do {
		/*
		 * determine how many lines of data we can show,
		 * max is the height of the window minus 1 if header must be shown
		 */
		nlines = LINES - print_header;

		/* is there enough data to fill one screen? */
		if (nlines > params->nlines)
			nlines = params->nlines;

		/*
		 * is there enough data to fill one screen starting from
		 * the desired line?
		 */
		if (first_line + nlines > params->nlines) {
			/*
			 * no, so calculate where should we start to fill
			 * one screen
			 */
			if (params->nlines < nlines)
				first_line = 0;
			else
				first_line = params->nlines - nlines;
		}

		show(params, headers, nheaders, first_line, nlines,  xoff,
			spacing, print_header, print_types, alignments,
			use_color_columns, is_color_column, color_column_index,
			col_offsets_buf);
		refresh();

		if (params->logfd >= 0)
			write(params->logfd, &ch, sizeof(ch));

		ch = getch();
		if (ch == KEY_DOWN || ch == 'j') {
			if (first_line + nlines >= params->nlines)
				beep();
			else
				first_line++;
		} else if (ch == KEY_UP || ch == 'k') {
			if (first_line > 0)
				first_line--;
			else
				beep();
		} else if (ch == KEY_RIGHT) {
			if (xoff + COLS < max_line)
				xoff += STEP;
			else
				beep();
		} else if (ch == KEY_LEFT) {
			if (xoff >= STEP)
				xoff -= STEP;
			else if (xoff == 0)
				beep();
			else
				xoff = 0;
		} else if (ch == KEY_NPAGE || ch == ' ') {
			if (first_line + nlines >= params->nlines)
				beep();
			else
				first_line += LINES - 1 - print_header;
		} else if (ch == KEY_PPAGE || ch == KEY_BACKSPACE) {
			if (first_line >= LINES - 1 - print_header)
				first_line -= LINES - 1 - print_header;
			else if (first_line == 0)
				beep();
			else
				first_line = 0;
		} else if (ch == KEY_HOME) {
			if (first_line == 0)
				beep();
			else
				first_line = 0;
		} else if (ch == KEY_END) {
			if (first_line == params->nlines - nlines)
				beep();
			else
				first_line = params->nlines - nlines;
		} else if (ch == KEY_SHOME) {
			if (xoff == 0)
				beep();
			else
				xoff = 0;
		} else if (ch == KEY_SEND) {
			int newxoff;

			if (max_line > COLS)
				newxoff = max_line - COLS;
			else
				newxoff = 0;

			if (newxoff == xoff)
				beep();
			else
				xoff = newxoff;
		} else if (ch == '[') {
			/* move by one column left */
			int old_xoff = xoff;
			int new_off = 0;

			for (size_t i = 0; i < nheaders; ++i) {
				size_t len = params->max_lengths[i] + spacing;

				if (new_off + len >= xoff) {
					xoff = new_off;
					break;
				}

				new_off += len;
			}

			if (old_xoff == xoff)
				beep();
		} else if (ch == ']') {
			/* move by one column right */
			int old_xoff = xoff;
			int new_off = 0;

			for (size_t i = 0; i < nheaders - 1; ++i) {
				new_off += params->max_lengths[i] + spacing;

				if (new_off > xoff) {
					xoff = new_off;
					break;
				}
			}

			if (old_xoff == xoff)
				beep();
		} else if (ch == '/') {
			// TODO implement search
		} else if (ch == 'h') {
			// TODO implement help
		}

	} while (ch != 'q');

	if (params->logfd >= 0) {
		write(params->logfd, &ch, sizeof(ch));
		close(params->logfd);
	}

	for (size_t i = 0; i < params->nlines; ++i)
		free(params->lines[i]);

	endwin();

	if (set_colorpairs_num || use_color_columns) {
		csv_ht_destroy(&params->color_pairs);
		csv_ht_destroy(&params->colors);
	}
	free(col_offsets_buf);
	free(is_color_column);
	free(color_column_index);
	free(params->col_color_pairs);
}

void
curses_ui_exit()
{
#if defined(NCURSESW62_OR_LATER)
	exit_curses(0);
#endif
}
