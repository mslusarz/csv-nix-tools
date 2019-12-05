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

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "agg.h"
#include "utils.h"

struct state {
	long long *max_int;
	char **max_str;
	size_t *str_size;
	size_t ncolumns;
};

static int
init_state(void *state, size_t ncolumns)
{
	struct state *st = state;
	st->ncolumns = ncolumns;
	st->max_int = xmalloc_nofail(ncolumns, sizeof(st->max_int[0]));
	st->max_str = xmalloc_nofail(ncolumns, sizeof(st->max_str[0]));
	st->str_size = xmalloc_nofail(ncolumns, sizeof(st->str_size[0]));
	for (size_t i = 0; i < ncolumns; ++i) {
		st->max_int[i] = LLONG_MIN;
		st->max_str[i] = NULL;
		st->str_size[i] = 0;
	}

	return 0;
}

static int
new_data_int(void *state, size_t col, long long llval)
{
	struct state *st = state;

	if (llval > st->max_int[col])
		st->max_int[col] = llval;

	return 0;
}

static long long
aggregate_int(void *state, size_t col)
{
	struct state *st = state;

	return st->max_int[col];
}

static void
free_state(void *state)
{
	struct state *st = state;

	free(st->max_int);
	for (size_t i = 0; i < st->ncolumns; ++i)
		free(st->max_str[i]);
	free(st->max_str);
	free(st->str_size);
}

static int
new_data_str(void *state, size_t col, const char *str)
{
	struct state *st = state;

	if (st->max_str[col] == NULL || strcmp(str, st->max_str[col]) > 0) {
		size_t len = strlen(str);
		if (len + 1 > st->str_size[col]) {
			free(st->max_str[col]);
			st->max_str[col] = malloc(len + 1);
			st->str_size[col] = len + 1;
		}
		memcpy(st->max_str[col], str, len + 1);
	}

	return 0;
}

static void
aggregate_str(void *state, size_t col)
{
	struct state *st = state;

	csv_print_quoted(st->max_str[col], strlen(st->max_str[col]));
}

int
main(int argc, char *argv[])
{
	struct state state;

	return agg_main(argc, argv, "max", &state, init_state, new_data_int,
			aggregate_int, free_state, new_data_str, aggregate_str);
}
