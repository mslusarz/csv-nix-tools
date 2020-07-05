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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef NCURSESW_ENABLED
#include <curses.h>
#endif

#include "ht.h"
#include "parse.h"
#include "utils.h"

#define DEFAULT_SPACING 3

enum alignment {
	LEFT,
	RIGHT,
};

static const struct option opts[] = {
	{"no-header",		no_argument,		NULL, 'H'},
	{"with-types",		no_argument, 		NULL, 't'},
	{"ui",			required_argument,	NULL, 'u'},
	{"spacing",		required_argument,	NULL, 's'},
	{"use-color-columns",	no_argument,		NULL, 'U' },
	{"set-color",		required_argument,	NULL, 'C' },
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{"debug",		required_argument,	NULL, 'D'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-show [OPTION]...\n");
	fprintf(out,
"Print CSV file from standard input in human-readable format, using tables,\n"
"without types.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -s, --spacing NUM          use NUM spaces between columns instead of 3\n");
	fprintf(out, "  -u, --ui curses/less/none  use UI based on ncurses, less or nothing(tm)\n");
	fprintf(out, "      --no-header            remove column headers\n");
	fprintf(out, "      --set-color col:[fg=]color1[,bg=color2]\n");
	fprintf(out, "                             set color1 as foreground and color2 as\n");
	fprintf(out, "                             background of column col\n");
	fprintf(out, "      --use-color-columns    use columns with _color suffix\n");
	fprintf(out, "      --with-types           print types in column headers\n");
	describe_help(out);
	describe_version(out);
}

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

/* returns number of printed characters (NOT length of string) */
static size_t
sprint(const char *str, char *out)
{
	size_t len = strlen(str);
	size_t idx = 0;
	size_t start = 0;
	size_t end = len;
	bool quot = false;

	if (str[0] == '"') {
		assert(str[len - 1] == '"');
		start++;
		end--;
		quot = true;
	}

	for (size_t i = start; i < end; ++i) {
		if (quot && str[i] == '"') {
			out[idx++] = str[i];
			assert(str[i + 1] == '"');
			/* skip next " */
			i++;
			continue;
		}

		if (str[i] == '\\') {
			out[idx++] = str[i];
			out[idx++] = str[i];
			continue;
		}

		if (!iscntrl(str[i])) {
			out[idx++] = str[i];
			continue;
		}

		switch(str[i]) {
			case '\a': out[idx++] = '\\'; out[idx++] = 'a'; break;
			case '\b': out[idx++] = '\\'; out[idx++] = 'b'; break;
			case '\e': out[idx++] = '\\'; out[idx++] = 'e'; break;
			case '\f': out[idx++] = '\\'; out[idx++] = 'f'; break;
			case '\n': out[idx++] = '\\'; out[idx++] = 'n'; break;
			case '\r': out[idx++] = '\\'; out[idx++] = 'r'; break;
			case '\t': out[idx++] = '\\'; out[idx++] = 't'; break;
			case '\v': out[idx++] = '\\'; out[idx++] = 'v'; break;
			default:
				idx += util_sprintf(&out[idx], "\\x%02X", str[i]);
				break;
		}
	}

	out[idx] = 0;

	assert(idx < 4 * len + 1);

	return idx;
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (params->nlines == params->allocated_lines) {
		if (params->allocated_lines == 0)
			params->allocated_lines = 16;
		else
			params->allocated_lines *= 2;

		char **newlines = xrealloc(params->lines,
				params->allocated_lines, sizeof(params->lines[0]));
		if (!newlines)
			return -1;

		params->lines = newlines;
	}

	size_t last_col_len = strlen(buf + col_offs[ncols - 1]);
	size_t len = col_offs[ncols - 1] + last_col_len + 1;

	if (params->tmpbuf_size < 4 * len + 1) {
		char *buf = xrealloc(params->tmpbuf, 4 * len + 1, 1);
		if (!buf)
			return -1;

		params->tmpbuf = buf;
		params->tmpbuf_size = 4 * len + 1;
	}

	char *cur = params->tmpbuf;
	for (size_t i = 0; i < ncols; ++i) {
		size_t printed = sprint(&buf[col_offs[i]], cur);

		if (printed > params->max_lengths[i])
			params->max_lengths[i] = printed;

		cur += printed + 1;
	}

	len = (uintptr_t)cur - (uintptr_t)params->tmpbuf;
	assert(len <= params->tmpbuf_size);

	char *line = xmalloc(len, 1);
	if (!line)
		return -1;

	memcpy(line, params->tmpbuf, len);

	params->lines[params->nlines++] = line;

	return 0;
}

#ifdef NCURSESW_ENABLED

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
	else {
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

		if (strlen(col) != 6)
			goto err;

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
	}

	/* we have to allocate, because COLOR_BLACK == 0 (and 0 casted to
	 * void * is NULL, which is considered an error by csv_ht_get_value) */
	int *color_ptr = xmalloc_nofail(1, sizeof(*color_ptr));
	*color_ptr = color;

	return color_ptr;
err:
	endwin();
	fprintf(stderr,
		"Can't parse '%s' as color - it must be either a predefined name or in RRGGBB format\n",
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

static void
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
				fprintf(stderr, "\n");
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
				fprintf(stderr, "\n");
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
			is_color_column[i] = strcmp(headers[i].name + len - strlen("_color"), "_color") == 0;

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
#endif

static inline void
print_spaces(size_t s)
{
	for (size_t i = 0; i < s; ++i)
		fputc(' ', stdout);
}

static void
print_line(char *line, size_t *max_lengths, size_t columns, size_t spacing,
		enum alignment *alignments)
{
	const char *buf = line;

	for (size_t i = 0; i < columns; ++i) {
		size_t len = strlen(buf);

		if (alignments[i] == RIGHT)
			print_spaces(max_lengths[i] - len);

		fwrite(buf, len, 1, stdout);
		buf += len + 1;

		if (alignments[i] == LEFT)
			if (i < columns - 1)
				print_spaces(max_lengths[i] - len);

		if (i < columns - 1)
			print_spaces(spacing);
	}
	fputc('\n', stdout);
}

static void
static_ui(bool less, struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		size_t spacing, enum alignment *alignments)
{
	if (less)  {
		int fds[2];
		pid_t pid;

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
			char *less[] = {"less", "-S", NULL};

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
			execvp(less[0], less);

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

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i) {
			size_t len = strlen(headers[i].name);
			if (print_types)
				len += 1 + strlen(headers[i].type);

			if (alignments[i] == RIGHT)
				print_spaces(params->max_lengths[i] - len);

			if (print_types) {
				printf("%s:%s", headers[i].name,
						headers[i].type);
			} else {
				printf("%s", headers[i].name);
			}

			if (alignments[i] == LEFT)
				if (i < nheaders - 1)
					print_spaces(params->max_lengths[i] - len);

			if (i < nheaders - 1)
				print_spaces(spacing);
		}
		fputc('\n', stdout);
	}

	for (size_t i = 0; i < params->nlines; ++i) {
		print_line(params->lines[i], params->max_lengths, nheaders,
				spacing, alignments);
		free(params->lines[i]);
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool print_header = true;
	bool print_types = false;
	enum { GUESS, NCURSES, LESS, NONE } ui = GUESS;
	size_t spacing = DEFAULT_SPACING;

	params.lines = NULL;
	params.allocated_lines = 0;
	params.nlines = 0;

	params.tmpbuf = NULL;
	params.tmpbuf_size = 0;

	params.logfd = -1;

	setlocale(LC_ALL, "");

	char **set_colorpair = NULL;
	size_t set_colorpair_num = 0;
	bool use_color_columns = false;

	while ((opt = getopt_long(argc, argv, "u:s:", opts, NULL)) != -1) {
		switch (opt) {
			case 'D':
				params.logfd = open(optarg,
						O_WRONLY | O_CREAT | O_SYNC,
						0600);
				if (params.logfd < 0) {
					fprintf(stderr,
						"opening '%s' failed: %s\n",
						optarg, strerror(errno));
					exit(2);
				}
				break;
			case 'u':
				if (strcmp(optarg, "curses") == 0)
					ui = NCURSES;
				else if (strcmp(optarg, "less") == 0)
					ui = LESS;
				else if (strcmp(optarg, "none") == 0)
					ui = NONE;
				else {
					fprintf(stderr, "Invalid value for --ui option\n");
					usage(stderr);
					exit(2);
				}
				break;
			case 's':
				if (strtoul_safe(optarg, &spacing, 0))
					exit(2);
				break;
			case 't':
				print_types = true;
				break;
			case 'C':
				set_colorpair = xrealloc_nofail(set_colorpair,
						++set_colorpair_num,
						sizeof(set_colorpair[0]));
				set_colorpair[set_colorpair_num - 1] =
						xstrdup_nofail(optarg);
				break;
			case 'U':
				use_color_columns = true;
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
	if (ui == GUESS) {
		if (isatty(1))
#ifdef NCURSESW_ENABLED
			ui = NCURSES;
#else
			ui = LESS;
#endif
		else
			ui = NONE;
	}

#ifndef NCURSESW_ENABLED
	if (ui == NCURSES) {
		fprintf(stderr, "curses ui not compiled in\n");
		exit(2);
	}
#endif

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.max_lengths = xcalloc_nofail(nheaders, sizeof(params.max_lengths[0]));

	enum alignment *alignments =
			xmalloc_nofail(nheaders, sizeof(alignments[0]));
	for (size_t i = 0; i < nheaders; ++i) {
		if (strcmp(headers[i].type, "int") == 0)
			alignments[i] = RIGHT;
		else
			alignments[i] = LEFT;
	}

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i) {
			params.max_lengths[i] = strlen(headers[i].name);
			if (print_types)
				params.max_lengths[i] += 1 + strlen(headers[i].type);
		}
	}

	csv_read_all_nofail(s, &next_row, &params);

	if (ui == NCURSES) {
#ifdef NCURSESW_ENABLED
		curses_ui(&params, headers, nheaders, print_header,
				print_types, spacing, alignments, set_colorpair,
				set_colorpair_num, use_color_columns);
#endif
	} else {
		static_ui(ui == LESS, &params, headers, nheaders, print_header,
				print_types, spacing, alignments);
	}

	for (size_t i = 0; i < set_colorpair_num; ++i)
		free(set_colorpair[i]);
	free(set_colorpair);
	free(alignments);
	free(params.lines);
	free(params.max_lengths);
	free(params.tmpbuf);

	csv_destroy_ctx(s);

#ifdef NCURSESW_ENABLED
	if (ui == NCURSES)
		exit_curses(0);
#endif

	return 0;
}
