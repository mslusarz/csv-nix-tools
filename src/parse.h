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

#ifndef CSV_PARSE_H
#define CSV_PARSE_H

#include <stddef.h>
#include <stdio.h>

struct csv_ctx;

struct col_header {
	const char *name;
	const char *type;
};

struct csv_ctx *csv_create_ctx(FILE *in, FILE *err);
struct csv_ctx *csv_create_ctx_nofail(FILE *in, FILE *err);

int csv_read_header(struct csv_ctx *s);
void csv_read_header_nofail(struct csv_ctx *s);

size_t csv_get_headers(struct csv_ctx *s, const struct col_header **headers);

typedef int (*csv_row_cb)(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg);

int csv_read_all(struct csv_ctx *s, csv_row_cb cb, void *arg);
void csv_read_all_nofail(struct csv_ctx *s, csv_row_cb cb, void *arg);

void csv_destroy_ctx(struct csv_ctx *s);

#endif
