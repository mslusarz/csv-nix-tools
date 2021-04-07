/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_PARSE_H
#define CSV_PARSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct csv_ctx;

struct col_header {
	const char *name;
	const char *type;
	bool had_type;
};

struct csv_ctx *csv_create_ctx(FILE *in, FILE *err);
struct csv_ctx *csv_create_ctx_nofail(FILE *in, FILE *err);

int csv_read_header(struct csv_ctx *s);
void csv_read_header_nofail(struct csv_ctx *s);

size_t csv_get_headers(struct csv_ctx *s, const struct col_header **headers);

typedef int (*csv_row_cb)(const char *buf, const size_t *col_offs,
		size_t ncols, void *arg);

int csv_read_all(struct csv_ctx *s, csv_row_cb cb, void *arg);
void csv_read_all_nofail(struct csv_ctx *s, csv_row_cb cb, void *arg);

void csv_destroy_ctx(struct csv_ctx *s);

#endif
