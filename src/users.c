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

#include "usr-grp.h"
#include "utils.h"

static const struct option opts[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-users [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
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

	struct column_info columns[] = {
			{ true, 0, "name",        TYPE_STRING, print_name },
			{ true, 0, "passwd",      TYPE_STRING, print_passwd },
			{ true, 0, "uid",         TYPE_INT,    print_uid },
			{ true, 0, "gid",         TYPE_INT,    print_gid },
			{ true, 0, "gecos",       TYPE_STRING, print_gecos },
			{ true, 0, "dir",         TYPE_STRING, print_dir },
			{ true, 0, "shell",       TYPE_STRING, print_shell },
	};
	size_t ncolumns = ARRAY_SIZE(columns);

	while ((opt = getopt_long(argc, argv, "f:s", opts, NULL)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
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

	csvci_print_header(columns, ncolumns);

	load_users();

	for (size_t i = 0; i < nusers; ++i)
		csvci_print_row(&users[i], columns, ncolumns);

	free_users();

	return 0;
}
