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

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "parse.h"
#include "utils.h"

static const struct option long_options[] = {
	{"no-header",	no_argument,		NULL, 'H'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-sqlite [OPTION] sql-query\n");
	printf("Options:\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct cb_params {
	sqlite3 *db;
	sqlite3_stmt *insert;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	for (size_t i = 0; i < nheaders; ++i) {
		int ret;
		if (headers[i].type[0] == 'i') {
			long long val;
			if (strtoll_safe(&buf[col_offs[i]], &val) < 0)
				exit(2);
			ret = sqlite3_bind_int64(params->insert, i + 1, val);
		} else {
			const char *str = &buf[col_offs[i]];
			if (str[0] == '"') {
				char *unquoted = csv_unquot(str);
				ret = sqlite3_bind_text(params->insert, i + 1,
						unquoted, -1, SQLITE_TRANSIENT);
				free(unquoted);
			} else {
				ret = sqlite3_bind_text(params->insert, i + 1,
						str, -1, SQLITE_STATIC);
			}
		}

		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_*: %s, %lu %s %s\n",
					sqlite3_errmsg(params->db), i,
					headers[i].name, headers[i].type);
			exit(2);
		}
	}

	if (sqlite3_step(params->insert) != SQLITE_DONE) {
		fprintf(stderr, "sqlite3_step: %s\n",
				sqlite3_errmsg(params->db));
		exit(2);
	}

	if (sqlite3_reset(params->insert) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_reset: %s\n",
				sqlite3_errmsg(params->db));
		exit(2);
	}

	return 0;
}

static const char *
type(const char *t)
{
	if (strcmp(t, "int") == 0)
		return "int";
	else
		return "string";
}

static int
build_queries(const struct col_header *headers, size_t nheaders,
		char **pcreate, char **pins)
{
	size_t create_len = strlen("create table input();");
	size_t insert_len = strlen("insert into input() values();");

	for (size_t i = 0; i < nheaders; ++i) {
		create_len += strlen(headers[i].name) + strlen(" ") +
			strlen(type(headers[i].type)) + strlen(", ");

		insert_len += strlen(headers[i].name) + strlen(", ") + strlen("?, ");
	}

	create_len -= 2; /* last ", " */
	insert_len -= 4; /* last ", " x2 */

	char *create = malloc(create_len + 1/*NUL*/);
	if (!create) {
		perror("malloc");
		return -1;
	}
	char *insert = malloc(insert_len + 1/*NUL*/);
	if (!insert) {
		perror("malloc");
		free(create);
		return -1;
	}

	int cr_written = sprintf(create, "create table input(");
	int ins_written = sprintf(insert, "insert into input(");
	for (size_t i = 0; i < nheaders - 1; ++i) {
		cr_written += sprintf(&create[cr_written], "%s %s, ",
				headers[i].name, type(headers[i].type));

		ins_written += sprintf(&insert[ins_written], "%s, ",
				headers[i].name);
	}

	cr_written += sprintf(&create[cr_written], "%s %s);",
			headers[nheaders - 1].name,
			type(headers[nheaders - 1].type));

	ins_written += sprintf(&insert[ins_written], "%s) values(",
			headers[nheaders - 1].name);

	for (size_t i = 0; i < nheaders - 1; ++i) {
		ins_written += sprintf(&insert[ins_written], "?, ");
	}
	ins_written += sprintf(&insert[ins_written], "?);");

	if (cr_written != create_len) {
		fprintf(stderr, "miscalculated needed buffer size 1 %d != %lu\n",
				cr_written, create_len);
		goto err;
	}

	if (ins_written != insert_len) {
		fprintf(stderr, "miscalculated needed buffer size 2 %d != %lu\n",
				ins_written, insert_len);
		goto err;
	}

	*pcreate = create;
	*pins = insert;

	return 0;
err:
	free(create);
	free(insert);
	return -1;
}

static const char *
sqlite_type_to_csv_name(int t)
{
	if (t == SQLITE_INTEGER)
		return "int";
	else if (t == SQLITE_TEXT)
		return "string";
	else if (t == SQLITE_NULL)
		return "string";

	fprintf(stderr, "unsupported sqlite type: %d\n", t);
	exit(2);
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	struct cb_params params;
	bool print_header = true;

	while ((opt = getopt_long(argc, argv, "v", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'H':
				print_header = false;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
					default:
						usage();
						return 2;
				}
				break;
			case 'h':
			default:
				usage();
				return 2;
		}
	}

	if (optind != argc - 1) {
		usage();
		exit(2);
	}

	struct csv_ctx *s = csv_create_ctx(stdin, stderr);
	if (!s)
		exit(2);
	if (csv_read_header(s))
		exit(2);
	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	sqlite3 *db;
	if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		exit(2);
	}
	params.db = db;

	char *create, *insert;
	if (build_queries(headers, nheaders, &create, &insert))
		exit(2);

	if (sqlite3_exec(db, create, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec(create): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}
	free(create);

	if (sqlite3_prepare_v2(db, insert, -1, &params.insert, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_prepare_v2(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	if (csv_read_all(s, &next_row, &params))
		exit(2);

	if (sqlite3_finalize(params.insert) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_finalize(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}
	free(insert);

	sqlite3_stmt *select;
	if (sqlite3_prepare_v2(db, argv[optind], -1, &select, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_prepare_v2(select): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	int ret = sqlite3_step(select);
	if (ret != SQLITE_ROW && ret != SQLITE_DONE) {
		fprintf(stderr, "sqlite3_step(select): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	int cnt = sqlite3_column_count(select);
	if (cnt < 1) {
		fprintf(stderr, "column count(%d) < 1\n", cnt);
		exit(2);
	}

	if (print_header) {
		for (size_t i = 0; i < cnt - 1; ++i) {
			const char *name = sqlite3_column_name(select, i);
			size_t col = csv_find(headers, nheaders, name);
			if (col == CSV_NOT_FOUND) {
				int type = sqlite3_column_type(select, i);
				printf("%s:%s,", name,
						sqlite_type_to_csv_name(type));
			} else {
				printf("%s:%s,", name, headers[col].type);
			}
		}

		const char *name = sqlite3_column_name(select, cnt - 1);
		size_t col = csv_find(headers, nheaders, name);
		if (col == CSV_NOT_FOUND) {
			int type = sqlite3_column_type(select, cnt - 1);
			printf("%s:%s\n", name,
					sqlite_type_to_csv_name(type));
		} else {
			printf("%s:%s\n", name, headers[col].type);
		}
	}

	while (ret == SQLITE_ROW) {
		for (size_t i = 0; i < cnt - 1; ++i) {
			int type = sqlite3_column_type(select, i);
			if (type == SQLITE_INTEGER)
				printf("%lld,", sqlite3_column_int64(select, i));
			else if (type == SQLITE_TEXT) {
				const char *txt =
					(const char *)sqlite3_column_text(select, i);
				csv_print_quoted(txt, strlen(txt));
				fputc(',', stdout);
			} else {
				fprintf(stderr, "unsupported return type: %d\n",
						type);
				exit(2);
			}
		}

		int type = sqlite3_column_type(select, cnt - 1);
		if (type == SQLITE_INTEGER)
			printf("%lld\n", sqlite3_column_int64(select, cnt - 1));
		else if (type == SQLITE_TEXT) {
			const char *txt =
				(const char *)sqlite3_column_text(select, cnt - 1);
			csv_print_quoted(txt, strlen(txt));
			fputc('\n', stdout);
		} else {
			fprintf(stderr, "unsupported return type: %d\n", type);
			exit(2);
		}

		ret = sqlite3_step(select);
	}

	if (ret != SQLITE_DONE) {
		fprintf(stderr, "sqlite3_step(select): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	if (sqlite3_finalize(select) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_finalize(select): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_close: %s\n", sqlite3_errmsg(db));
		exit(2);
	}

	csv_destroy_ctx(s);

	return 0;
}
