/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

struct csv_ctx {
	FILE *in;
	FILE *err;

	char *header_line;
	size_t header_line_size;

	struct col_header *headers;
	size_t nheaders;
};

struct csv_ctx *
csv_create_ctx(FILE *in, FILE *err)
{
	struct csv_ctx *s = xcalloc(1, sizeof(*s));
	if (!s)
		return NULL;
	s->in = in;
	s->err = err;

	return s;
}

struct csv_ctx *
csv_create_ctx_nofail(FILE *in, FILE *err)
{
	struct csv_ctx *s = csv_create_ctx(in, err);
	if (!s)
		exit(2);
	return s;
}

void
csv_destroy_ctx(struct csv_ctx *ctx)
{
	free(ctx->header_line);
	free(ctx->headers);
	memset(ctx, 0, sizeof(*ctx));
	free(ctx);
}

static int
add_header(struct csv_ctx *ctx, char *start)
{
	char *type = strchr(start, ':');
	if (type)
		*type++ = 0;
	else
		type = "string";

	struct col_header *headers = realloc(ctx->headers,
			(ctx->nheaders + 1) * sizeof(ctx->headers[0]));
	if (!headers) {
		fprintf(ctx->err, "realloc failed: %s\n", strerror(errno));
		return -1;
	}
	ctx->headers = headers;

	if (strcmp(type, "int") != 0 &&
			strcmp(type, "int[]") != 0 &&
			strcmp(type, "string") != 0 &&
			strcmp(type, "string[]") != 0) {
		fprintf(ctx->err, "unsupported type '%s'\n", type);
		return -1;
	}

	ctx->headers[ctx->nheaders].name = start;
	ctx->headers[ctx->nheaders].type = type;
	ctx->nheaders++;

	return 0;
}

int
csv_read_header(struct csv_ctx *ctx)
{
	ssize_t line_len;

	errno = 0;
	line_len = getline(&ctx->header_line, &ctx->header_line_size, ctx->in);
	if (line_len < 1) {
		if (errno)
			fprintf(ctx->err, "getline: %s\n", strerror(errno));
		else if (feof(ctx->in))
			fprintf(ctx->err, "EOF while reading header\n");
		else
			fprintf(ctx->err, "unrecognized error from getline: %ld\n", line_len);
		return -1;
	}

	char *start = ctx->header_line;
	char *comma;
	do {
		comma = strchr(start, ',');
		if (comma) {
			*comma = 0;

			if (add_header(ctx, start))
				return -1;

			start = comma + 1;
		}
	} while (comma);

	char *nl = strchr(start, '\n');
	if (!nl) {
		fprintf(ctx->err, "corrupted input (missing new line)\n");
		return -1;
	}
	*nl = 0;

	return add_header(ctx, start);
}

void
csv_read_header_nofail(struct csv_ctx *ctx)
{
	if (csv_read_header(ctx))
		exit(2);
}

size_t
csv_get_headers(struct csv_ctx *ctx, const struct col_header **headers)
{
	*headers = ctx->headers;
	return ctx->nheaders;
}

int
csv_read_all(struct csv_ctx *ctx, csv_row_cb cb, void *arg)
{
	size_t *col_offs = malloc(ctx->nheaders * sizeof(col_offs[0]));
	if (!col_offs) {
		fprintf(ctx->err, "malloc: %s\n", strerror(errno));
		return -1;
	}

	int ret = 0;
	int eof = 0;
	size_t buflen = 0;
	char *buf = NULL;
	size_t ready = 0;

	size_t column = 0;
	bool in_quoted_string = false;
	bool last_char_was_quot = false;
	col_offs[0] = 0;
	size_t start_off = 0;

	while (1) {
		size_t i = start_off;
		while (i < ready) {
			if (last_char_was_quot) {
				if (buf[i] == '"') {
					// this character was escaped

					// so switch back to in_quot_string logic
					last_char_was_quot = false;

					// and continue from the next character
					i++;
				} else {
					// last " was end of quoted string

					// so reset quoting logic
					last_char_was_quot = false;
					in_quoted_string = false;

					// and continue from the *same* character
				}
			} else if (in_quoted_string) {
				if (buf[i] == '"')
					last_char_was_quot = true;

				i++;
			} else if (buf[i] == ',' || buf[i] == '\n') {
				// end of non-quoted column
				buf[i] = 0;
				column++;
				if (column == ctx->nheaders) {
					if (cb(buf, col_offs, ctx->nheaders, arg)) {
						ret = 1;
						goto end;
					}

					i++;
					memmove(&buf[0], &buf[i], ready - i);
					ready -= i;
					i = 0;
					column = 0;
				} else {
					// move on to the next column
					i++;
					col_offs[column] = i;
				}
			} else if (buf[i] == '"') {
				// if we are not at the beginning of
				// a column, then the stream is corrupted
				if (i != col_offs[column]) {
					fprintf(ctx->err,
						"corrupted stream - \" in the middle of unquoted string\n");
					ret = -1;
					goto end;
				}

				// switch to quoted string logic
				in_quoted_string = true;

				// and continue from the next character
				i++;
			} else {
				// we are in the middle of a column
				i++;
			}
		}

		if (eof)
			break;

		if (ready == buflen) {
			buflen += 1024;
			buf = realloc(buf, buflen);
			if (!buf) {
				fprintf(ctx->err, "realloc: %s\n",
					strerror(errno));
				ret = -1;
				goto end;
			}
		}

		size_t nmemb = buflen - ready;
		/*
		 * Don't ask for too much, otherwise memmove after each row
		 * becomes a perf problem. TODO: figure out how to fix this
		 * without impacting code readability.
		 */
		if (nmemb > 100)
			nmemb = 100;

		size_t readin = fread(&buf[ready], 1, nmemb, ctx->in);
		if (readin < nmemb) {
			if (!feof(ctx->in)) {
				fprintf(ctx->err, "fread: %s\n",
					strerror(errno));
				ret = -1;
				goto end;
			}

			eof = 1;
		}

		start_off = ready;
		ready += readin;
	}

end:
	free(col_offs);
	free(buf);

	return ret;
}

void
csv_read_all_nofail(struct csv_ctx *ctx, csv_row_cb cb, void *arg)
{
	if (csv_read_all(ctx, cb, arg))
		exit(2);
}
