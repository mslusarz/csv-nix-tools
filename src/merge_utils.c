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

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "merge_utils.h"
#include "utils.h"

struct stdin_cfg {
	size_t label_column;
	size_t ncolumns;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct stdin_cfg *cfg = arg;

	fputs(&buf[col_offs[cfg->label_column]], stdout);
	fputc(',', stdout);

	for (size_t i = 0; i < nheaders - 1; ++i) {
		if (i == cfg->label_column)
			continue;
		fputs(&buf[col_offs[i]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[nheaders - 1]], stdout);

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

	cfg.label_column =
			csv_find(input_headers, input_nheaders, LABEL_COLUMN);

	if (cfg.label_column == CSV_NOT_FOUND) {
		fprintf(stderr,
			"column '%s' not found in input\n",
			LABEL_COLUMN);
		exit(2);
	}

	for (size_t i = 0; i < input_nheaders; ++i) {
		if (i == cfg.label_column)
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
csvmu_print_header(struct csvmu_ctx *ctx, const char *default_label,
		struct column_info *columns, size_t ncolumns)
{
	if (ctx->merge_with_stdin && !ctx->label)
		ctx->label = xstrdup_nofail(default_label);

	char *prefix = NULL;
	if (ctx->label) {
		printf("%s:string,", LABEL_COLUMN);

		prefix = xmalloc_nofail(strlen(ctx->label) + 1 + 1, 1);
		sprintf(prefix, "%s_", ctx->label);
	}

	if (ctx->merge_with_stdin) {
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
	if (ctx->label)
		printf("%s,", ctx->label);
	if (ctx->merge_with_stdin)
		csvmu_fill_columns_from_stdin(ctx->input_ncolumns);

	csvci_print_row(row, columns, ncolumns);
}
