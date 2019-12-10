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
	fprintf(out, "Usage: csv-groups [OPTION]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

static void
merge_users_into_groups(void)
{
	for (size_t i = 0; i < ngroups; ++i) {
		struct csv_group *g = &groups[i];
		char **members = g->members;

		g->users = xmalloc_nofail(g->nmembers, sizeof(g->users[0]));

		size_t idx = 0;
		for (size_t j = 0; j < g->nmembers; ++j) {
			struct csv_user *u = find_user_by_name(members[j]);
			if (u) {
				g->users[idx++] = u;
			} else {
				fprintf(stderr,
					"warning: group '%s' contains unknown user '%s'\n",
					g->name, *members);
			}
		}
	}

	for (size_t i = 0; i < nusers; ++i) {
		struct csv_user *u = &users[i];

		struct csv_group *g = find_group_by_gid(u->gid);
		if (!g) {
			fprintf(stderr,
				"warning: user '%s' belongs to unknown group %d\n",
				u->name, u->gid);
			continue;
		}

		bool found = false;
		for (size_t j = 0; j < g->nmembers; ++j) {
			if (g->users[j] == u) {
				found = true;
				break;
			}
		}

		if (!found) {
			g->members = xrealloc_nofail(g->members,
					g->nmembers + 1,
					sizeof(g->members[0]));
			g->users = xrealloc_nofail(g->users,
					g->nmembers + 1,
					sizeof(g->users[0]));

			g->members[g->nmembers] = xstrdup_nofail(u->name);
			g->users[g->nmembers++] = u;
		}
	}
}

static void
print_name(const void *p)
{
	const struct csv_group *g = p;
	csv_print_quoted(g->name, strlen(g->name));
}

static void
print_passwd(const void *p)
{
	const struct csv_group *g = p;
	csv_print_quoted(g->passwd, strlen(g->passwd));
}

static void
print_gid(const void *p)
{
	const struct csv_group *g = p;
	printf("%u", g->gid);
}

static void
print_user_ids(const void *p)
{
	const struct csv_group *g = p;
	if (g->nmembers) {
		if (g->nmembers > 1)
			putc('"', stdout);

		for (size_t j = 0; j < g->nmembers - 1; ++j)
			printf("%u,", g->users[j]->uid);

		printf("%u", g->users[g->nmembers - 1]->uid);

		if (g->nmembers > 1)
			putc('"', stdout);
	}
}

static void
print_user_names(const void *p)
{
	const struct csv_group *g = p;
	if (g->nmembers) {
		if (g->nmembers > 1)
			putc('"', stdout);

		for (size_t j = 0; j < g->nmembers - 1; ++j)
			printf("%s,", g->users[j]->name);

		printf("%s", g->users[g->nmembers - 1]->name);

		if (g->nmembers > 1)
			putc('"', stdout);
	}
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
			{ true, 0, "gid",         TYPE_INT,    print_gid },
			{ true, 0, "user_ids",    TYPE_STRING, print_user_ids },
			{ true, 0, "user_names",  TYPE_STRING, print_user_names },
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
		int r = csvci_parse_cols(cols, columns, &ncolumns);

		free(cols);

		if (r)
			exit(2);
	} else {
		csvci_set_columns_order(columns, &ncolumns);
	}

	if (show)
		csv_show();

	csvci_print_header(columns, ncolumns);

	load_groups();
	for (size_t i = 0; i < ncolumns; ++i) {
		if (strcmp(columns[i].name, "user_ids") != 0 &&
				strcmp(columns[i].name, "user_names") != 0)
			continue;

		load_users();
		merge_users_into_groups();
	}

	for (size_t i = 0; i < ngroups; ++i)
		csvci_print_row(&groups[i], columns, ncolumns);

	free_users();
	free_groups();

	return 0;
}
