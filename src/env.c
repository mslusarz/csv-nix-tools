/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "merge_utils.h"
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
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-env [OPTION]...\n");
	fprintf(out, "Print to standard output the list of environment variables in the CSV format.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	describe_Columns(out);
	describe_Merge(out);
	describe_table_Name(out);
	describe_Show(out);
	describe_Show_full(out);
	describe_as_Table(out, "env");
	describe_help(out);
	describe_version(out);
}

static void
print_name(const void *p)
{
	const char *env = p;
	const char *eq = strchr(env, '=');
	csv_print_quoted(env, (uintptr_t)eq - (uintptr_t)env);
}

static void
print_value(const void *p)
{
	const char *env = p;
	const char *eq = strchr(env, '=');
	csv_print_quoted(eq + 1, strlen(eq + 1));
}

int
main(int argc, char *argv[], char *envp[])
{
	int opt;
	char *cols = NULL;
	unsigned show_flags = SHOW_DISABLED;
	bool merge = false;
	char *table = NULL;

	struct column_info columns[] = {
			{ true, 0, 0, "name",  TYPE_STRING, print_name, 0 },
			{ true, 0, 0, "value", TYPE_STRING, print_value, 0 },
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
					table = xstrdup_nofail("env");
				break;
			case 'N':
				free(table);
				table = xstrdup_nofail(optarg);
				break;
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'T':
				free(table);
				table = xstrdup_nofail("env");
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

	char **env = envp;
	while (*env)
		csvmu_print_row(&ctx, *env++, columns, ncolumns);

	free(ctx.table);

	return 0;
}
