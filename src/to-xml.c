/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <getopt.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

#include "parse.h"
#include "utils.h"

struct cb_params {
	const struct col_header *headers;
	bool generic_names;
};

static const struct option opts[] = {
	{"generic-names",	no_argument,		NULL, 'G'},
	{"no-header",		no_argument,		NULL, 'H'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-to-xml [OPTION]...\n");
	fprintf(out,
"Convert CSV file from standard input to XML.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "      --generic-names        don't use column names for nodes\n");
	fprintf(out, "      --no-header            remove column headers\n");
	describe_help(out);
	describe_version(out);
}

static void
print(const char *str)
{
	while (*str) {
		switch(*str) {
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
			printf("%c", *str);
			break;
		}
		str++;
	}
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	printf("  <row>\n");

	for (size_t i = 0; i < ncols; ++i) {
		if (params->generic_names)
			printf("   <column n=\"%zu\">", i);
		else
			printf("   <%s>", params->headers[i].name);

		const char *str = buf + col_offs[i];
		const char *unquoted = str;
		if (str[0] == '"')
			unquoted = csv_unquot(str);

		print(unquoted);

		if (str[0] == '"')
			free((char *)unquoted);

		if (params->generic_names)
			printf("</column>\n");
		else
			printf("</%s>\n", params->headers[i].name);
	}

	printf("  </row>\n");

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	bool print_header = true;
	struct cb_params params;

	const char *loc = setlocale(LC_ALL, "");

	params.generic_names = false;

	while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
		switch (opt) {
			case 'G':
				params.generic_names = true;
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
	params.headers = headers;

	if (!params.generic_names) {
		for (size_t i = 0; i < nheaders; ++i) {
			const char *name = headers[i].name;

			size_t len = mbstowcs(NULL, name, 0);

			if (len == (size_t)-1) {
				fprintf(stderr,
					"Name of column %zu contains invalid multibyte sequence.\n",
					i);
				exit(2);
			}

			wchar_t *wcs = xmalloc_nofail(len, sizeof(wcs[0]));

			size_t converted = mbstowcs(wcs, name, len);
			assert (converted == len);
			(void) converted;

			wchar_t allowed[3] = { L'_', L'-', L'.' };

			if (!(iswalpha((wint_t)wcs[0]) || wcs[0] == allowed[0])) {
				fprintf(stderr,
					"Name of column %zu doesn't start with a letter or underscore.\n",
					i);
				exit(2);
			}

			if ((name[0] == 'x' || name[0] == 'X') &&
				(name[1] == 'm' || name[1] == 'M') &&
				(name[2] == 'l' || name[2] == 'L')) {
				fprintf(stderr,
					"Name of column %zu starts with 'xml'.\n",
					i);
				exit(2);
			}

			for (size_t j = 0; j < len; ++j) {
				if (iswalpha((wint_t)wcs[j]) ||
						iswdigit((wint_t)wcs[j]) ||
						wcs[j] == allowed[0] ||
						wcs[j] == allowed[1] ||
						wcs[j] == allowed[2] )
					continue;

				fprintf(stderr,
					"Name of column %zu contains characters other than: letters, digits, hyphens, underscores or periods.\n",
					i);
				exit(2);
			}

			free(wcs);
		}
	}

	printf("<?xml version=\"1.0\"");

	while (*loc != 0 && *loc != '.')
		loc++;
	if (*loc == '.')
		loc++;
	if (*loc) {
		printf(" encoding=\"");
		while (*loc != 0 && *loc != '@')
			printf("%c", *loc++);
		printf("\"");
	}

	printf("?>\n");
	printf("<root>\n");

	if (print_header) {
		printf(" <header>\n");
		if (params.generic_names) {
			for (size_t i = 0; i < nheaders; ++i) {
				printf("  <column n=\"%zu\" type=\"%s\">", i, headers[i].type);
				print(headers[i].name);
				printf("</column>\n");
			}
		} else {
			for (size_t i = 0; i < nheaders; ++i)
				printf("  <%s type=\"%s\" />\n",
					headers[i].name, headers[i].type);
		}
		printf(" </header>\n");
	}

	printf(" <data>\n");

	csv_read_all_nofail(s, &next_row, &params);

	printf(" </data>\n"
		"</root>\n");

	csv_destroy_ctx(s);

	return 0;
}
