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

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "utils.h"

void
csv_print_header(FILE *out, const struct col_header *headers, size_t nheaders)
{
	for (size_t i = 0; i < nheaders - 1; ++i)
		fprintf(out, "%s:%s,", headers[i].name, headers[i].type);
	fprintf(out, "%s:%s\n", headers[nheaders - 1].name,
			headers[nheaders - 1].type);
}

void
csv_print_line(FILE *out, const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders)
{
	for (size_t i = 0; i < nheaders - 1; ++i) {
		fputs(&buf[col_offs[i]], stdout);
		fputc(',', stdout);
	}
	fputs(&buf[col_offs[nheaders - 1]], stdout);
	fputc('\n', stdout);
}

int
strtoll_safe(const char *str, long long *val)
{
	char *end;

	errno = 0;
	long long llval = strtoll(str, &end, 0);

	if (llval == LLONG_MIN && errno) {
		fprintf(stderr,
			"value '%s' is too small\n", str);
		return -1;
	}

	if (llval == LLONG_MAX && errno) {
		fprintf(stderr,
			"value '%s' is too big\n", str);
		return -1;
	}

	if (*end) {
		fprintf(stderr,
			"value '%s' doesn't convert to integer\n", str);
		return -1;
	}

	*val = llval;
	return 0;
}
