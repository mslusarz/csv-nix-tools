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

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
//	{"add",		required_argument,	NULL, 'a'},
//	{"add-types",	required_argument,	NULL, 'A'},
//	{"change-type",	required_argument,	NULL, 'C'},
//	{"guess-types",	no_argument,		NULL, 'G'},
	{"remove",	no_argument,		NULL, 'm'},
//	{"remove-types",no_argument,		NULL, 'M'},
//	{"rename",	required_argument,	NULL, 'n'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-header [OPTION]...\n");
	fprintf(out, "Options:\n");
//	fprintf(out, "  -a, --add col1:type1[,col2:type2]\n");
//	fprintf(out, "  -A, --add-types col1:type1[,col2:type2]\n");
//	fprintf(out, "  -C, --change-type col:type\n");
//	fprintf(out, "  -G, --guess-types\n");
	fprintf(out, "  -m, --remove\n");
//	fprintf(out, "  -M, --remove-types\n");
//	fprintf(out, "  -n, --rename original,new\n");
	describe_show(out);
	describe_show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
//	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, headers, nheaders, true);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool print_header = true;
	bool show = false;
	bool show_full;

//	char *rename_from = NULL;
//	char *rename_to = NULL;

//	char *change_name = NULL;
//	char *change_type = NULL;

//	char **columns = NULL;

	while ((opt = getopt_long(argc, argv, "a:A:C:GmMn:sS", opts, NULL)) != -1) {
		switch (opt) {
//			case 'a': /* add col1:type1[,col2:type2] */
//				break;
//			case 'A': /* add-types col1:type1[,col2:type2] */
//				break;
//			case 'C': /* change-type col:type */
//				break;
//			case 'G': /* guess-types */
//				break;
			case 'm': /* remove */
				print_header = false;
				break;
//			case 'M': /* remove-types */
//				break;
//			case 'n': /* rename original,new */
//				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
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

	if (show) {
		if (!print_header) {
			fprintf(stderr,
				"--remove and --show can't be used at the same time\n");
			exit(2);
		}

		csv_show(show_full);
	}

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (print_header)
		csv_print_header(stdout, headers, nheaders);

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
