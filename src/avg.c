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

#include "agg.h"
#include "utils.h"

struct state {
	long long *sums;
	size_t rows;
};

static int
init_state(void *state, size_t ncolumns, const char *unused)
{
	struct state *st = state;
	st->sums = xcalloc_nofail(ncolumns, sizeof(st->sums[0]));
	st->rows = 0;
	return 0;
}

static int
new_data_int(void *state, size_t col, long long llval)
{
	struct state *st = state;

	if (llval > 0 && st->sums[col] > LLONG_MAX - llval) {
		fprintf(stderr, "integer overflow\n");
		return -1;
	}

	if (llval < 0 && st->sums[col] < LLONG_MIN - llval) {
		fprintf(stderr, "integer underflow\n");
		return -1;
	}

	st->sums[col] += llval;

	if (col == 0)
		st->rows++;

	return 0;
}

static long long
aggregate_int(void *state, size_t col)
{
	struct state *st = state;

	return st->sums[col] / (long long)st->rows;
}

static void
free_state(void *state)
{
	struct state *st = state;

	free(st->sums);
}

int
main(int argc, char *argv[])
{
	struct state state;

	return agg_main(argc, argv, "avg", &state, init_state, new_data_int,
			aggregate_int, free_state, NULL, NULL, false);
}