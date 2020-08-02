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
	fprintf(out, "Usage: csv-groups [OPTION]...\n");
	fprintf(out, "Print to standard output the list of system groups in the CSV format.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Columns(out);
	fprintf(out, "  -l                         use a longer listing format (can be used once)\n");
	describe_Merge(out);
	describe_table_Name(out);
	describe_Show(out);
	describe_Show_full(out);
	describe_as_Table(out, "group");
	describe_help(out);
	describe_version(out);
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

int
main(int argc, char *argv[])
{
	int opt;
	char *cols = NULL;
	unsigned show_flags = SHOW_DISABLED;
	bool merge = false;
	char *table = NULL;

	struct column_info columns[] = {
			{ true,  0, 0, "name",        TYPE_STRING, print_name, 0 },
			{ true,  0, 0, "gid",         TYPE_INT,    print_gid, 0 },
			{ false, 0, 1, "passwd",      TYPE_STRING, print_passwd, 0 },
	};

	size_t ncolumns = ARRAY_SIZE(columns);
	size_t level = 0;

	while ((opt = getopt_long(argc, argv, "c:lMN:sST", opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'l':
				level++;
				for (size_t i = 0; i < ncolumns; ++i)
					if (columns[i].level <= level)
						columns[i].vis = true;
				break;
			case 'M':
				merge = true;
				if (!table)
					table = xstrdup_nofail("group");
				break;
			case 'N':
				table = strdup(optarg);
				break;
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'T':
				table = strdup("group");
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

	csv_show(show_flags);

	struct csvmu_ctx ctx;
	ctx.table = table;
	ctx.merge = merge;

	csvmu_print_header(&ctx, columns, ncolumns);

	load_groups();

	for (size_t i = 0; i < ngroups; ++i)
		csvmu_print_row(&ctx, &groups[i], columns, ncolumns);

	free(ctx.table);
	free_groups();

	return 0;
}
