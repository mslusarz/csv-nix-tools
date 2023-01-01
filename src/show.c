/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2022, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "parse.h"
#include "show.h"
#include "utils.h"

#define DEFAULT_SPACING 3

static const struct option opts[] = {
	{"no-header",		no_argument,		NULL, 'H'},
	{"with-types",		no_argument, 		NULL, 't'},
	{"ui",			required_argument,	NULL, 'u'},
	{"spacing",		required_argument,	NULL, 'p'},
	{"use-color-columns",	no_argument,		NULL, 'C' },
	{"set-color",		required_argument,	NULL, 'c' },
	{"interactive",		no_argument,		NULL, 'i' },
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
	fprintf(out, "  -C, --use-color-columns    use columns with _color suffix\n");
	fprintf(out, "  -i, --interactive          line-selection mode (ncurses backend only)\n");
	fprintf(out, "  -p, --spacing NUM          use NUM spaces between columns instead of 3\n");
	fprintf(out, "  -u, --ui TYPE              choose UI TYPE: curses, less, none, auto\n");
	fprintf(out, "  -s                         short for -u none\n");
	fprintf(out, "  -S                         short for -u curses\n");
	fprintf(out, "      --no-header            remove column headers\n");
	fprintf(out, "      --set-color COLNAME:[fg=]COLOR1[,bg=COLOR2]\n");
	fprintf(out, "                             set COLOR1 as foreground and COLOR2 as\n");
	fprintf(out, "                             background of column COLNAME\n");
	fprintf(out, "      --with-types           print types in column headers\n");
	describe_help(out);
	describe_version(out);
}

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

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool print_header = true;
	bool print_types = false;
	bool interactive = false;
	enum {
		GUESS,
		NCURSES,
		LESS,
		NONE
	} ui = GUESS;

	size_t spacing = DEFAULT_SPACING;

	params.lines = NULL;
	params.allocated_lines = 0;
	params.nlines = 0;

	params.tmpbuf = NULL;
	params.tmpbuf_size = 0;

	params.logfd = -1;

	params.split_results = NULL;
	params.split_results_max_size = 0;

	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	char **set_colorpair = NULL;
	size_t set_colorpair_num = 0;
	bool use_color_columns = false;

	while ((opt = getopt_long(argc, argv, "Ciu:p:sS", opts, NULL)) != -1) {
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
				else if (strcmp(optarg, "auto") == 0)
					ui = GUESS;
				else {
					fprintf(stderr, "Invalid value for --ui option\n");
					usage(stderr);
					exit(2);
				}
				break;
			case 'p':
				if (strtoul_safe(optarg, &spacing, 0))
					exit(2);
				break;
			case 's':
				ui = NONE;
				break;
			case 'S':
				ui = NCURSES;
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
				use_color_columns = true;
				break;
			case 'i':
				interactive = true;
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

	if (interactive && ui != NCURSES) {
		fprintf(stderr, "interactive mode is supported only by ncurses backend\n");
		exit(2);
	}

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	params.max_lengths = xcalloc_nofail(nheaders, sizeof(params.max_lengths[0]));

	enum alignment *alignments =
			xmalloc_nofail(nheaders, sizeof(alignments[0]));
	for (size_t i = 0; i < nheaders; ++i) {
		if (strcmp(headers[i].type, "int") == 0 ||
				strcmp(headers[i].type, "float") == 0)
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
				set_colorpair_num, use_color_columns, interactive);
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
	free(params.split_results);

	csv_destroy_ctx(s);

#if defined(NCURSESW_ENABLED)
	if (ui == NCURSES)
		curses_ui_exit();
#endif

	return 0;
}
