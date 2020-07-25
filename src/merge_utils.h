/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_MERGE_UTILS_H
#define CSV_MERGE_UTILS_H

#include <stddef.h>
#include "utils.h"

struct csvmu_ctx {
	char *table;
	bool merge;
	size_t input_ncolumns;
};

void csvmu_print_header(struct csvmu_ctx *ctx, struct column_info *columns,
		size_t ncolumns);

void csvmu_print_row(struct csvmu_ctx *ctx,
		const void *row,
		const struct column_info *columns,
		size_t ncolumns);

#endif
