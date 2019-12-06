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
	long long *sums_int;
	char **sums_str;
	size_t *str_used;
	size_t *str_sizes;
	size_t ncolumns;
	const char *sep;
	size_t sep_len;
};

static int
init_state(void *state, size_t ncolumns, const char *sep)
{
	struct state *st = state;
	st->ncolumns = ncolumns;
	st->sums_int = xcalloc_nofail(ncolumns, sizeof(st->sums_int[0]));
	st->sums_str = xcalloc_nofail(ncolumns, sizeof(st->sums_str[0]));
	st->str_used = xcalloc_nofail(ncolumns, sizeof(st->str_used[0]));
	st->str_sizes = xcalloc_nofail(ncolumns, sizeof(st->str_sizes[0]));
	st->sep = sep;
	if (sep)
		st->sep_len = strlen(sep);
	else
		st->sep_len = 0;
	return 0;
}

static int
new_data_int(void *state, size_t col, long long llval)
{
	struct state *st = state;

	if (llval > 0 && st->sums_int[col] > LLONG_MAX - llval) {
		fprintf(stderr, "integer overflow\n");
		return -1;
	}

	if (llval < 0 && st->sums_int[col] < LLONG_MIN - llval) {
		fprintf(stderr, "integer underflow\n");
		return -1;
	}

	st->sums_int[col] += llval;

	return 0;
}

static long long
aggregate_int(void *state, size_t col)
{
	struct state *st = state;

	return st->sums_int[col];
}

static void
free_state(void *state)
{
	struct state *st = state;

	free(st->sums_int);
	for (size_t i = 0; i < st->ncolumns; ++i)
		free(st->sums_str[i]);
	free(st->sums_str);
	free(st->str_used);
	free(st->str_sizes);
}

static int
new_data_str(void *state, size_t col, const char *str)
{
	struct state *st = state;

	size_t len = strlen(str);

	size_t req = st->sep_len + len + 1;
	if (req > st->str_sizes[col] - st->str_used[col]) {
		st->str_sizes[col] *= 2;
		if (st->str_sizes[col] - st->str_used[col] < req)
			st->str_sizes[col] += req;

		st->sums_str[col] =
			xrealloc_nofail(st->sums_str[col],
					st->str_sizes[col], 1);
	}

	if (st->str_used[col] > 0 && st->sep_len) {
		strcpy(&st->sums_str[col][st->str_used[col]], st->sep);
		st->str_used[col] += st->sep_len;
	}

	memcpy(&st->sums_str[col][st->str_used[col]], str, len + 1);
	st->str_used[col] += len;

	return 0;
}

static void
aggregate_str(void *state, size_t col)
{
	struct state *st = state;

	csv_print_quoted(st->sums_str[col], st->str_used[col]);
}

int
main(int argc, char *argv[])
{
	struct state state;

	return agg_main(argc, argv, "sum", &state, init_state, new_data_int,
			aggregate_int, free_state, new_data_str, aggregate_str,
			true);
}
