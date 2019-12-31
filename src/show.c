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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef NCURSES_ENABLED
#include <curses.h>
#endif

#include "parse.h"
#include "utils.h"

#define DEFAULT_SPACING 3

static const struct option opts[] = {
	{"no-header",	no_argument,		NULL, 'H'},
	{"with-types",	no_argument, 		NULL, 't'},
	{"ui",		required_argument,	NULL, 'u'},
	{"spacing",	required_argument,	NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{"debug",	required_argument,	NULL, 'D'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-show [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -u, --ui curses/less/none\n");
	fprintf(out, "  -s, --spacing NUM\n");
	fprintf(out, "      --with-types\n");
	fprintf(out, "      --no-header\n");
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	char **lines;
	size_t max_nlines;
	size_t nlines;
	size_t *max_lengths;

	char *tmpbuf;
	size_t tmpbuf_size;

	int logfd;
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
				idx += sprintf(&out[idx], "\\x%02X", str[i]);
				break;
		}
	}

	out[idx] = 0;

	assert(idx < 4 * len + 1);

	return idx;
}

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	if (params->nlines == params->max_nlines) {
		if (params->max_nlines == 0)
			params->max_nlines = 16;
		else
			params->max_nlines *= 2;

		char **newlines = xrealloc(params->lines,
				params->max_nlines, sizeof(params->lines[0]));
		if (!newlines)
			return -1;

		params->lines = newlines;
	}

	size_t last_col_len = strlen(buf + col_offs[nheaders - 1]);
	size_t len = col_offs[nheaders - 1] + last_col_len + 1;

	if (params->tmpbuf_size < 4 * len + 1) {
		char *buf = xrealloc(params->tmpbuf, 4 * len + 1, 1);
		if (!buf)
			return -1;

		params->tmpbuf = buf;
		params->tmpbuf_size = 4 * len + 1;
	}

	char *cur = params->tmpbuf;
	for (size_t i = 0; i < nheaders; ++i) {
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

#ifdef NCURSES_ENABLED

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
show(char **data, size_t ndata,
	const struct col_header *headers, size_t nheaders,
	size_t *max_lengths,

	size_t first_line, size_t nlines,
	int xoff,
	size_t spacing,
	bool print_header, bool print_types)
{
	size_t ypos = 0;
	clear();

	if (print_header) {
		int xpos = 0;

		for (size_t i = 0; i < nheaders; ++i) {
			int prev_xpos = xpos;
			bool truncated;

			nprint(0, xpos - xoff, headers[i].name, &truncated);
			if (truncated)
				break;
			xpos += strlen(headers[i].name);

			if (print_types) {
				nprint(0, xpos - xoff, ":", &truncated);
				if (truncated)
					break;
				xpos++;

				nprint(0, xpos - xoff, headers[i].type,
						&truncated);
				if (truncated)
					break;
				xpos += strlen(headers[i].type);
			}

			if (i < nheaders - 1)
				xpos = prev_xpos + max_lengths[i] + spacing;
		}

		ypos++;
	}


	for (size_t ln = first_line; ln < first_line + nlines; ++ln, ++ypos) {
		const char *buf = data[ln];
		int xpos = 0;

		for (size_t i = 0; i < nheaders; ++i) {
			bool truncated;

			nprint(ypos, xpos - xoff, buf, &truncated);

			if (truncated)
				break;

			buf += strlen(buf) + 1;

			if (i < nheaders - 1)
				xpos += max_lengths[i] + spacing;
		}
	}
}

static void
curses_ui(struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		size_t spacing)
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

	initscr();
	cbreak();
	noecho();
	clear();
	keypad(stdscr, TRUE);
	curs_set(0);

	int ch = 0;
	size_t first_line = 0;
	size_t nlines;
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
		nlines = LINES;
		if (nlines > params->nlines)
			nlines = params->nlines;
		if (first_line + nlines > params->nlines) {
			if (params->nlines < LINES)
				first_line = 0;
			else
				first_line = params->nlines - LINES;
		}

		show(params->lines, params->nlines,
			headers, nheaders,
			params->max_lengths,
			first_line, nlines,
			xoff, spacing, print_header, print_types);
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
				first_line += LINES - 1;
		} else if (ch == KEY_PPAGE || ch == KEY_BACKSPACE) {
			if (first_line >= LINES - 1)
				first_line -= LINES - 1;
			else if (first_line == 0)
				beep();
			else
				first_line = 0;
		} else if (ch == KEY_HOME) {
			if (xoff == 0)
				beep();
			else
				xoff = 0;
		} else if (ch == KEY_END) {
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
}
#endif

static void
print_line(char *line, size_t *max_lengths, size_t columns, size_t spacing)
{
	const char *buf = line;

	for (size_t i = 0; i < columns; ++i) {
		size_t len = strlen(buf);

		fwrite(buf, len, 1, stdout);
		buf += len + 1;

		if (i < columns - 1)
			while (len++ < max_lengths[i] + spacing)
				fputc(' ', stdout);
	}
	fputc('\n', stdout);
}

static void
static_ui(bool less, struct cb_params *params, const struct col_header *headers,
		size_t nheaders, bool print_header, bool print_types,
		size_t spacing)
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
			int written;

			if (print_types) {
				written = printf("%s:%s", headers[i].name,
						headers[i].type);
			} else {
				written = printf("%s", headers[i].name);
			}

			if (i < nheaders - 1)
				while (written++ < params->max_lengths[i] + spacing)
					fputc(' ', stdout);
		}
		fputc('\n', stdout);
	}

	for (size_t i = 0; i < params->nlines; ++i) {
		print_line(params->lines[i], params->max_lengths, nheaders,
				spacing);
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
	params.max_nlines = 0;
	params.nlines = 0;

	params.tmpbuf = NULL;
	params.tmpbuf_size = 0;

	params.logfd = -1;

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
#ifdef NCURSES_ENABLED
			ui = NCURSES;
#else
			ui = LESS;
#endif
		else
			ui = NONE;
	}

#ifndef NCURSES_ENABLED
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

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i) {
			params.max_lengths[i] = strlen(headers[i].name);
			if (print_types)
				params.max_lengths[i] += 1 + strlen(headers[i].type);
		}
	}

	csv_read_all_nofail(s, &next_row, &params);

	if (ui == NCURSES) {
#ifdef NCURSES_ENABLED
		curses_ui(&params, headers, nheaders, print_header,
				print_types, spacing);
#endif
	} else {
		static_ui(ui == LESS, &params, headers, nheaders, print_header,
				print_types, spacing);
	}

	free(params.lines);
	free(params.max_lengths);
	free(params.tmpbuf);

	csv_destroy_ctx(s);

	return 0;
}
