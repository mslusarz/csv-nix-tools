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

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

int
rpn_eval(struct rpn_expression *exp, const char *buf, const size_t *col_offs,
		long long *value)
{
	long long *stack = NULL;
	size_t height = 0;

	for (size_t j = 0; j < exp->count; ++j) {
		struct rpn_token *t = &exp->tokens[j];
		if (t->type == RPN_CONSTANT) {
			stack = realloc(stack, (height + 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				goto fail;
			}

			stack[height++] = t->constant;
		} else if (t->type == RPN_COLUMN) {
			stack = realloc(stack, (height + 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				goto fail;
			}

			if (strtoll_safe(&buf[col_offs[t->colnum]],
					&stack[height++]))
				goto fail;
		} else if (t->type == RPN_OPERATOR) {
			if (height < 2) {
				fprintf(stderr, "not enough stack entries\n");
				goto fail;
			}

			switch (t->operator) {
			case RPN_ADD:
				stack[height - 2] += stack[height - 1];
				break;
			case RPN_SUB:
				stack[height - 2] -= stack[height - 1];
				break;
			case RPN_MUL:
				stack[height - 2] *= stack[height - 1];
				break;
			case RPN_DIV:
				stack[height - 2] /= stack[height - 1];
				break;
			case RPN_REM:
				stack[height - 2] %= stack[height - 1];
				break;
			case RPN_BIT_OR:
				stack[height - 2] |= stack[height - 1];
				break;
			case RPN_BIT_AND:
				stack[height - 2] &= stack[height - 1];
				break;
			case RPN_BIT_XOR:
				stack[height - 2] ^= stack[height - 1];
				break;
			case RPN_BIT_LSHIFT:
				stack[height - 2] <<= stack[height - 1];
				break;
			case RPN_BIT_RSHIFT:
				stack[height - 2] >>= stack[height - 1];
				break;
			default:
				abort();
			}

			stack = realloc(stack, (height - 1) * sizeof(stack[0]));
			if (!stack) {
				perror("realloc");
				goto fail;
			}

			height--;

		} else {
			/* impossible */
			abort();
		}
	}

	if (height == 0) {
		fprintf(stderr, "empty stack\n");
		goto fail;
	}

	if (height >= 2) {
		fprintf(stderr, "too many entries on stack (%lu)\n", height);
		goto fail;
	}

	*value = stack[0];

	free(stack);

	return 0;

fail:
	free(stack);
	return -1;
}
