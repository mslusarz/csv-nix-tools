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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merge_utils.h"
#include "usr-grp.h"
#include "utils.h"

static const struct option opts[] = {
	{"fields",		required_argument,	NULL, 'f'},
	{"merge-with-stdin",	no_argument,		NULL, 'M'},
	{"label",		required_argument,	NULL, 'L'},
	{"show",		no_argument,		NULL, 's'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-users [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "                             choose the list of columns\n");
	fprintf(out, "  -M, --merge-with-stdin     \n");
	fprintf(out, "  -L, --label label          \n");
	fprintf(out, "  -l                         use a longer listing format (can be used up to 2 times)\n");
	fprintf(out, "  -s, --show                 pipe output to csv-show\n");
	fprintf(out, "      --help                 display this help and exit\n");
	fprintf(out, "      --version              output version information and exit\n");
}

static void
print_name(const void *p)
{
	const struct csv_user *u = p;
	csv_print_quoted(u->name, strlen(u->name));
}

static void
print_passwd(const void *p)
{
	const struct csv_user *u = p;
	csv_print_quoted(u->passwd, strlen(u->passwd));
}

static void
print_uid(const void *p)
{
	const struct csv_user *u = p;
	printf("%u", u->uid);
}

static void
print_gid(const void *p)
{
	const struct csv_user *u = p;
	printf("%u", u->gid);
}

static void
print_gecos(const void *p)
{
	const struct csv_user *u = p;
	csv_print_quoted(u->gecos, strlen(u->gecos));
}

static void
print_dir(const void *p)
{
	const struct csv_user *u = p;
	csv_print_quoted(u->dir, strlen(u->dir));
}

static void
print_shell(const void *p)
{
	const struct csv_user *u = p;
	csv_print_quoted(u->shell, strlen(u->shell));
}

int
main(int argc, char *argv[])
{
	int opt;
	char *cols = NULL;
	bool show = false;
	bool merge_with_stdin = false;
	char *label = NULL;

	struct column_info columns[] = {
			{ true,  0, 0, "name",   TYPE_STRING, print_name },
			{ false, 0, 2, "passwd", TYPE_STRING, print_passwd },
			{ true,  0, 0, "uid",    TYPE_INT,    print_uid },
			{ true,  0, 0, "gid",    TYPE_INT,    print_gid },
			{ false, 0, 1, "gecos",  TYPE_STRING, print_gecos },
			{ true,  0, 0, "dir",    TYPE_STRING, print_dir },
			{ true,  0, 0, "shell",  TYPE_STRING, print_shell },
	};
	size_t ncolumns = ARRAY_SIZE(columns);
	int level = 0;

	while ((opt = getopt_long(argc, argv, "f:lL:Ms", opts, NULL)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'l':
				level++;
				for (size_t i = 0; i < ncolumns; ++i)
					if (columns[i].level <= level)
						columns[i].vis = true;
				break;
			case 'L':
				label = strdup(optarg);
				break;
			case 'M':
				merge_with_stdin = true;
				break;
			case 's':
				show = true;
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

	if (cols) {
		csvci_parse_cols_nofail(cols, columns, &ncolumns);

		free(cols);
	} else {
		csvci_set_columns_order(columns, &ncolumns);
	}

	if (show)
		csv_show();

	struct csvmu_ctx ctx;
	ctx.label = label;
	ctx.merge_with_stdin = merge_with_stdin;

	csvmu_print_header(&ctx, "user", columns, ncolumns);

	load_users();

	for (size_t i = 0; i < nusers; ++i)
		csvmu_print_row(&ctx, &users[i], columns, ncolumns);

	free(ctx.label);
	free_users();

	return 0;
}
