/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "merge_utils.h"
#include "utils.h"

struct stdin_cfg {
	size_t table_column;
	size_t ncolumns;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct stdin_cfg *cfg = arg;

	fputs(&buf[col_offs[cfg->table_column]], stdout);
	fputc(',', stdout);

	for (size_t i = 0; i < ncols - 1; ++i) {
		if (i == cfg->table_column)
			continue;
		fputs(&buf[col_offs[i]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[ncols - 1]], stdout);

	for (size_t i = 0; i < cfg->ncolumns; ++i)
		putchar(',');

	fputc('\n', stdout);

	return 0;
}

static size_t
csvmu_add_columns_to_stdin(const char *prefix, struct column_info *columns,
		size_t ncolumns)
{
	struct csv_ctx *ctx = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(ctx);

	const struct col_header *input_headers;
	size_t input_nheaders = csv_get_headers(ctx, &input_headers);

	struct stdin_cfg cfg;
	cfg.ncolumns = ncolumns;

	cfg.table_column =
			csv_find(input_headers, input_nheaders, TABLE_COLUMN);

	if (cfg.table_column == CSV_NOT_FOUND) {
		fprintf(stderr,
			"column '%s' not found in input\n",
			TABLE_COLUMN);
		exit(2);
	}

	for (size_t i = 0; i < input_nheaders; ++i) {
		if (i == cfg.table_column)
			continue;

		printf("%s:%s,", input_headers[i].name, input_headers[i].type);
	}
	csvci_print_header_with_prefix(columns, ncolumns, prefix);

	csv_read_all_nofail(ctx, &next_row, &cfg);

	csv_destroy_ctx(ctx);
	fclose(stdin);

	return input_nheaders;
}

void
csvmu_print_header(struct csvmu_ctx *ctx, struct column_info *columns,
		size_t ncolumns)
{
	char *prefix = NULL;
	if (ctx->table) {
		printf("%s:string,", TABLE_COLUMN);

		prefix = xmalloc_nofail(strlen(ctx->table) + 1 + 1, 1);
		sprintf(prefix, "%s%c", ctx->table, TABLE_SEPARATOR);
	}

	if (ctx->merge) {
		ctx->input_ncolumns =
			csvmu_add_columns_to_stdin(prefix, columns, ncolumns);
	} else {
		if (prefix)
			csvci_print_header_with_prefix(columns, ncolumns, prefix);
		else
			csvci_print_header(columns, ncolumns);
	}

	free(prefix);
}

void
csvmu_fill_columns_from_stdin(size_t input_ncolumns)
{
	assert(input_ncolumns > 0);
	for (size_t j = 0; j < input_ncolumns - 1; ++j)
		putchar(',');
}

void
csvmu_print_row(struct csvmu_ctx *ctx, const void *row,
		const struct column_info *columns, size_t ncolumns)
{
	if (ctx->table)
		printf("%s,", ctx->table);
	if (ctx->merge)
		csvmu_fill_columns_from_stdin(ctx->input_ncolumns);

	csvci_print_row(row, columns, ncolumns);
}
