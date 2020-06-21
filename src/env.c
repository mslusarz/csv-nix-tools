/*
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
	bool show = false;
	bool show_full;
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
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
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

	if (show)
		csv_show(show_full);

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
