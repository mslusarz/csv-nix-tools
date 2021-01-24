/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-printf [OPTION]... FORMAT [COLUMNS]...\n");
	fprintf(out,
"Read CSV stream from standard input and print back data using printf(3)\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	char *format;
	size_t formatlen;
	struct arg {
		size_t col;
		enum data_type type;
	} *args;
	size_t nargs;
};

static bool skips[255] = {
	['+'] = true,
	['-'] = true,
	['0'] = true,
	['1'] = true,
	['2'] = true,
	['3'] = true,
	['4'] = true,
	['5'] = true,
	['6'] = true,
	['7'] = true,
	['8'] = true,
	['9'] = true,
	['.'] = true,
	['#'] = true,
	[' '] = true,
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;
	(void)ncols;

	char *fmt = params->format;
	size_t next_arg = 0;
	struct {
		const char *start;
		int size;
		bool intmax, sizet, ptrdiff;
		bool in_format;
	} st;

	memset(&st, 0, sizeof(st));
	struct arg *a = &params->args[0];

	for (size_t i = 0; i < params->formatlen; ++i) {
		const char c = fmt[i];
		const char next = fmt[i + 1];
		if (st.in_format) {
			if (skips[(unsigned char)c])
				(void) c;
			else if (c == 'h')
				st.size--;
			else if (c == 'l')
				st.size++;
			else if (c == 'z')
				st.intmax = true;
			else if (c == 'j')
				st.sizet = true;
			else if (c == 't')
				st.ptrdiff = true;
			else if (c == '%') {
				fmt[i + 1] = 0;
				printf(st.start, "");
				fmt[i + 1] = next;
				memset(&st, 0, sizeof(st));
			} else if (c == 'd' || c == 'i') {
				fmt[i + 1] = 0;
				if (next_arg == params->nargs) {
					fprintf(stderr, "not enough columns for format string\n");
					exit(2);
				}

				if (a->type == TYPE_INT) {
					long long val;
					if (strtoll_safe(&buf[col_offs[a->col]], &val, 0))
						exit(2);
					if (st.intmax)
						printf(st.start, (intmax_t)val);
					else if (st.sizet)
						printf(st.start, (size_t)val);
					else if (st.ptrdiff)
						printf(st.start, (ptrdiff_t)val);
					else if (st.size >= 2)
						printf(st.start, (long long)val);
					else if (st.size == 1)
						printf(st.start, (long)val);
					else if (st.size == 0)
						printf(st.start, (int)val);
					else if (st.size == -1)
						printf(st.start, (short)val);
					else // if (st.size <= -2)
						printf(st.start, (char)val);
				} else if (a->type == TYPE_FLOAT) {
					double val;
					if (strtod_safe(&buf[col_offs[a->col]], &val))
						exit(2);
					if (st.intmax)
						printf(st.start, (intmax_t)val);
					else if (st.sizet)
						printf(st.start, (size_t)val);
					else if (st.ptrdiff)
						printf(st.start, (ptrdiff_t)val);
					else if (st.size >= 2)
						printf(st.start, (long long)val);
					else if (st.size == 1)
						printf(st.start, (long)val);
					else if (st.size == 0)
						printf(st.start, (int)val);
					else if (st.size == -1)
						printf(st.start, (short)val);
					else // if (st.size <= -2)
						printf(st.start, (char)val);
				} else {
					fprintf(stderr, "printing string as number is not supported\n");
					exit(2);
				}
				fmt[i + 1] = next;
				a++;
				next_arg++;
				memset(&st, 0, sizeof(st));
			} else if (c == 'u' || c == 'o' || c == 'x' || c == 'X') {
				fmt[i + 1] = 0;
				if (next_arg == params->nargs) {
					fprintf(stderr, "not enough columns for format string\n");
					exit(2);
				}

				if (a->type == TYPE_INT) {
					long long val;
					if (strtoll_safe(&buf[col_offs[a->col]], &val, 0))
						exit(2);
					if (st.intmax)
						printf(st.start, (uintmax_t)val);
					else if (st.sizet)
						printf(st.start, (size_t)val);
					else if (st.ptrdiff)
						printf(st.start, (ptrdiff_t)val);
					else if (st.size >= 2)
						printf(st.start, (unsigned long long)val);
					else if (st.size == 1)
						printf(st.start, (unsigned long)val);
					else if (st.size == 0)
						printf(st.start, (unsigned int)val);
					else if (st.size == -1)
						printf(st.start, (unsigned short)val);
					else // if (st.size <= -2)
						printf(st.start, (unsigned char)val);
				} else if (a->type == TYPE_FLOAT) {
					double val;
					if (strtod_safe(&buf[col_offs[a->col]], &val))
						exit(2);
					if (st.intmax)
						printf(st.start, (uintmax_t)val);
					else if (st.sizet)
						printf(st.start, (size_t)val);
					else if (st.ptrdiff)
						printf(st.start, (ptrdiff_t)val);
					else if (st.size >= 2)
						printf(st.start, (unsigned long long)val);
					else if (st.size == 1)
						printf(st.start, (unsigned long)val);
					else if (st.size == 0)
						printf(st.start, (unsigned int)val);
					else if (st.size == -1)
						printf(st.start, (unsigned short)val);
					else // if (st.size <= -2)
						printf(st.start, (unsigned char)val);
				} else {
					fprintf(stderr, "printing string as number is not supported\n");
					exit(2);
				}
				fmt[i + 1] = next;
				a++;
				next_arg++;
				memset(&st, 0, sizeof(st));
			} else if (c == 'f' || c == 'F' || c == 'e' || c == 'E' || c == 'g' || c == 'G' || c == 'a' || c == 'A') {
				fmt[i + 1] = 0;
				if (next_arg == params->nargs) {
					fprintf(stderr, "not enough columns for format string\n");
					exit(2);
				}

				if (a->type == TYPE_INT) {
					long long val;
					if (strtoll_safe(&buf[col_offs[a->col]], &val, 0))
						exit(2);
					printf(st.start, (double)val);
				} else if (a->type == TYPE_FLOAT) {
					double val;
					if (strtod_safe(&buf[col_offs[a->col]], &val))
						exit(2);
					printf(st.start, val);
				} else {
					fprintf(stderr, "printing string as float is not supported\n");
					exit(2);
				}
				fmt[i + 1] = next;
				a++;
				next_arg++;
				memset(&st, 0, sizeof(st));
			} else if (c == 'c') {
				fmt[i + 1] = 0;
				if (next_arg == params->nargs) {
					fprintf(stderr, "not enough columns for format string\n");
					exit(2);
				}

				if (a->type == TYPE_INT) {
					long long val;
					if (strtoll_safe(&buf[col_offs[a->col]], &val, 0))
						exit(2);
					if (st.size >= 1)
						printf(st.start, (wint_t)val);
					else
						printf(st.start, (int)val);
				} else if (a->type == TYPE_FLOAT) {
					double val;
					if (strtod_safe(&buf[col_offs[a->col]], &val))
						exit(2);
					if (st.size >= 1)
						printf(st.start, (wint_t)val);
					else
						printf(st.start, (int)val);
				} else {
					fprintf(stderr, "printing string as number is not supported\n");
					exit(2);
				}
				fmt[i + 1] = next;
				a++;
				next_arg++;
				memset(&st, 0, sizeof(st));
			} else if (c == 's') {
				fmt[i + 1] = 0;
				if (next_arg == params->nargs) {
					fprintf(stderr, "not enough columns for format string\n");
					exit(2);
				}

				if (a->type == TYPE_INT) {
					fprintf(stderr, "printing integer as string is not supported\n");
					exit(2);
				} else if (a->type == TYPE_FLOAT) {
					fprintf(stderr, "printing float as string is not supported\n");
					exit(2);
				} else {
					if (st.size >= 1)
						printf(st.start, (wchar_t *)&buf[col_offs[a->col]]);
					else
						printf(st.start, (char *)&buf[col_offs[a->col]]);
				}
				fmt[i + 1] = next;
				a++;
				next_arg++;
				memset(&st, 0, sizeof(st));
			} else if (c == 'p') {
				fmt[i + 1] = 0;
				if (next_arg == params->nargs) {
					fprintf(stderr, "not enough columns for format string\n");
					exit(2);
				}

				if (a->type == TYPE_INT) {
					long long val;
					if (strtoll_safe(&buf[col_offs[a->col]], &val, 0))
						exit(2);
					printf(st.start, (void *)val);
				} else if (a->type == TYPE_FLOAT) {
					fprintf(stderr, "printing float as pointer is not supported\n");
					exit(2);
				} else {
					fprintf(stderr, "printing string as pointer is not supported\n");
					exit(2);
				}
				fmt[i + 1] = next;
				a++;
				next_arg++;
				memset(&st, 0, sizeof(st));
			}
		} else if (c == '%') {
			st.in_format = true;
			st.start = fmt + i;
		} else if (c == '\\') {
			if (i + 1 < params->formatlen) {
				i++;
				switch (fmt[i]) {
				case 'n': putchar('\n'); break;
				case 'r': putchar('\r'); break;
				case '\\': putchar('\\'); break;
				case 'a': putchar('\a'); break;
				case 'b': putchar('\b'); break;
				case 't': putchar('\t'); break;
				case 'f': putchar('\f'); break;
				case 'v': putchar('\v'); break;
				default:
					fprintf(stderr, "\\%c not supported\n", fmt[i]);
					exit(2);
				}
			} else {
				putchar(c);
			}
		} else {
			putchar(c);
		}
	}

	if (st.in_format)
		printf("%s", st.start);

	return 0;
}

static void
type_not_supported(const char *type, const char *col)
{
	fprintf(stderr,
		"Type '%s', used by column '%s', is not supported by csv-printf.\n",
		type, col);
	exit(2);
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;

	while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
		switch (opt) {
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	assert(optind >= 0);
	assert(argc >= 0);
	assert(optind <= argc);
	size_t args = (size_t)(argc - optind);
	if (args == 0) {
		usage(stderr);
		exit(2);
	}

	/* allocate copy of the format string because we'll modify it */
	params.format = xstrdup_nofail(argv[optind]);
	params.formatlen = strlen(params.format);

	params.nargs = (size_t)(argc - optind - 1);
	if (params.nargs)
		params.args = xmalloc_nofail(params.nargs, sizeof(params.args[0]));

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	for (int i = optind + 1; i < argc; ++i) {
		size_t idx = (size_t)(i - optind - 1);

		size_t col = csv_find_loud(headers, nheaders, NULL, argv[i]);
		if (col == CSV_NOT_FOUND)
			exit(1);
		params.args[idx].col = col;

		const char *t = headers[col].type;
		if (strcmp(t, "int") == 0)
			params.args[idx].type = TYPE_INT;
		else if (strcmp(t, "string") == 0)
			params.args[idx].type = TYPE_STRING;
		else if (strcmp(t, "float") == 0)
			params.args[idx].type = TYPE_FLOAT;
		else
			type_not_supported(t, argv[i]);
	}

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	csv_destroy_ctx(s);
	free(params.format);
	if (params.nargs)
		free(params.args);

	return 0;
}
