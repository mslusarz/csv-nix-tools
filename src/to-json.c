/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <ctype.h>
#include <getopt.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

struct cb_params {
	const struct col_header *headers;
	const bool *is_int;
	bool first_row;
};

static const struct option opts[] = {
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-to-json [OPTION]...\n");
	fprintf(out,
"Convert CSV file from standard input to JSON.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_help(out);
	describe_version(out);
}

static void
print(const char *str)
{
	printf("\"");
	while (*str) {
		switch(*str) {
		case '\\':
			printf("\\\\");
			break;
		case '"':
			printf("\\\"");
			break;
		case '\r':
			printf("\\r");
			break;
		case '\b':
			printf("\\b");
			break;
		case '\f':
			printf("\\f");
			break;
		case '\t':
			printf("\\t");
			break;
		case '\n':
			printf("\\n");
			break;
		default:
			if (iscntrl(*str))
				printf("\\u%04x", *str);
			else
				printf("%c", *str);
			break;
		}
		str++;
	}
	printf("\"");
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (!params->first_row)
		printf(",\n");

	printf("  {\n");

	for (size_t i = 0; i < ncols; ++i) {
		printf("   \"%s\": ", params->headers[i].name);

		const char *str = buf + col_offs[i];
		if (params->is_int[i]) {
			printf("%s", str);
		} else {
			const char *unquoted = str;
			if (str[0] == '"')
				unquoted = csv_unquot(str);

			print(unquoted);

			if (str[0] == '"')
				free((char *)unquoted);
		}

		if (i < ncols - 1)
			printf(",\n");
		else
			printf("\n");
	}

	printf("  }");

	params->first_row = false;

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;

	setlocale(LC_ALL, "");

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

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);
	params.headers = headers;
	params.first_row = true;

	bool *is_int = xmalloc_nofail(nheaders, sizeof(is_int[0]));
	for (size_t i = 0; i < nheaders; ++i)
		is_int[i] = strcmp(headers[i].type, "int") == 0;
	params.is_int = is_int;

	printf("{\n");

	printf(" \"rows\": [\n");

	csv_read_all_nofail(s, &next_row, &params);

	printf("\n");

	printf(" ]\n"
	       "}\n");

	csv_destroy_ctx(s);
	free(is_int);

	return 0;
}
