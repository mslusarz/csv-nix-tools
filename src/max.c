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

#include "int-agg.h"
#include "utils.h"

struct state {
	long long *maxs;
};

int
init_state(void *state, size_t ncolumns)
{
	struct state *st = state;
	st->maxs = xmalloc_nofail(ncolumns, sizeof(st->maxs[0]));
	for (size_t i = 0; i < ncolumns; ++i)
		st->maxs[i] = LLONG_MIN;

	return 0;
}

int
new_data(void *state, size_t col, long long llval)
{
	struct state *st = state;

	if (llval > st->maxs[col])
		st->maxs[col] = llval;

	return 0;
}

long long
aggregate(void *state, size_t col)
{
	struct state *st = state;

	return st->maxs[col];
}

void
free_state(void *state)
{
	struct state *st = state;

	free(st->maxs);
}

int
main(int argc, char *argv[])
{
	struct state state;

	return int_agg_main(argc, argv, "max", &state,
			init_state, new_data, aggregate, free_state);
}
