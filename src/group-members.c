/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merge_utils.h"
#include "usr-grp.h"
#include "utils.h"

static const struct option opts[] = {
	{"columns",		required_argument,	NULL, 'c'},
	{"merge",		no_argument,		NULL, 'M'},
	{"table-name",		required_argument,	NULL, 'N'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"as-table",		no_argument,		NULL, 'T'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-group-members [OPTION]...\n");
	fprintf(out,
"Print to standard output the list of system groups and users that belong to them\n"
"in the CSV format.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Columns(out);
	describe_Merge(out);
	describe_table_Name(out);
	describe_Show(out);
	describe_Show_full(out);
	describe_as_Table(out, "group_member");
	describe_help(out);
	describe_version(out);
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

struct row {
	const struct csv_group *group;
	size_t user_idx;
};

static void
print_group_name(const void *p)
{
	const struct row *row = p;
	printf("%s", row->group->name);
}

static void
print_user_name(const void *p)
{
	const struct row *row = p;
	printf("%s", row->group->members[row->user_idx]);
}

int
main(int argc, char *argv[])
{
	int opt;
	char *cols = NULL;
	bool show = false;
	bool show_full;
	bool merge = false;
	char *table = NULL;

	struct column_info columns[] = {
			{ true, 0, 0, "group_name", TYPE_STRING, print_group_name, 0 },
			{ true, 0, 0, "user_name", TYPE_STRING, print_user_name, 0 },
	};

	size_t ncolumns = ARRAY_SIZE(columns);

	while ((opt = getopt_long(argc, argv, "c:MN:sST", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'M':
				merge = true;
				if (!table)
					table = xstrdup_nofail("group_member");
				break;
			case 'N':
				table = xstrdup_nofail(optarg);
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'T':
				table = xstrdup_nofail("group_member");
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
		csv_show(show_full);

	struct csvmu_ctx ctx;
	ctx.table = table;
	ctx.merge = merge;

	csvmu_print_header(&ctx, columns, ncolumns);

	load_groups();
	load_users();
	merge_users_into_groups();

	for (size_t i = 0; i < ngroups; ++i) {
		struct row r;
		r.group = &groups[i];
		for (size_t j = 0; j < groups[i].nmembers; ++j) {
			r.user_idx = j;

			csvmu_print_row(&ctx, &r, columns, ncolumns);
		}
	}

	free(ctx.table);
	free_users();
	free_groups();

	return 0;
}
