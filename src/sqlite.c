/*
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "parse.h"
#include "utils.h"
#include "usr-grp.h"

static const struct option opts[] = {
	{"show",	no_argument,		NULL, 's'},
	{"show-full",	no_argument,		NULL, 'S'},
	{"use-tables",	no_argument,		NULL, 'T'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-sqlite [OPTION] sql-query\n");
	fprintf(out,
"Read CSV stream from standard input, load it into memory-backed sqlite database,\n"
"execute an SQL query and print back to standard output its result.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -i FILE                    ignore standard input and read from FILE instead;\n"
"                             can be used multiple times; '-' means standard\n"
"                             input\n");
	describe_Show(out);
	describe_Show_full(out);
	fprintf(out,
"  -T, --use-tables           interpret input as \"table\" stream (as _table\n"
"                             column and columns with \"table.\" prefixes) and\n"
"                             import each csv table into its own sql table\n");
	describe_help(out);
	describe_version(out);
}

struct cb_params {
	sqlite3 *db;
	bool tables;
	size_t table_column;

	struct table_insert {
		char *name;
		sqlite3_stmt *insert;
	} *inserts;

	size_t ntables;
};

static int
next_row(const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders,
		void *arg)
{
	struct cb_params *params = arg;

	struct table_insert *ins = NULL;

	const char *table = "";
	if (params->tables) {
		assert(params->table_column != SIZE_MAX);
		table = &buf[col_offs[params->table_column]];
		if (table[0] == '"') {
			fprintf(stderr,
				"tables with special characters are not supported\n");
			exit(2);
		}

		for (size_t i = 0; i < params->ntables; ++i) {
			if (strcmp(params->inserts[i].name, table) == 0) {
				ins = &params->inserts[i];
				break;
			}
		}

		if (!ins) {
			fprintf(stderr,
				"can't find prepared query for table '%s'\n",
				table);
			exit(2);
		}
	} else {
		assert(params->ntables == 1);
		ins = &params->inserts[0];
	}

	size_t table_len = strlen(table);
	size_t idx = 0;
	for (size_t i = 0; i < nheaders; ++i) {
		if (params->tables) {
			if (strncmp(headers[i].name, table, table_len) != 0 ||
				headers[i].name[table_len] != TABLE_SEPARATOR)
				continue;
		}

		int ret;
		if (strcmp(headers[i].type, "int") == 0) {
			long long val;
			if (strtoll_safe(&buf[col_offs[i]], &val, 0) < 0)
				exit(2);
			ret = sqlite3_bind_int64(ins->insert, idx + 1, val);
		} else {
			const char *str = &buf[col_offs[i]];
			if (str[0] == '"') {
				char *unquoted = csv_unquot(str);
				ret = sqlite3_bind_text(ins->insert, idx + 1,
						unquoted, -1, SQLITE_TRANSIENT);
				free(unquoted);
			} else {
				ret = sqlite3_bind_text(ins->insert, idx + 1,
						str, -1, SQLITE_STATIC);
			}
		}

		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_*: %s, %lu %s %s\n",
					sqlite3_errmsg(params->db), idx,
					headers[i].name, headers[i].type);
			exit(2);
		}

		idx++;
	}

	if (sqlite3_step(ins->insert) != SQLITE_DONE) {
		fprintf(stderr, "sqlite3_step: %s\n",
				sqlite3_errmsg(params->db));
		exit(2);
	}

	if (sqlite3_reset(ins->insert) != SQLITE_OK) {
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
		return "text";
}

static int
build_queries(const struct col_header *headers, size_t nheaders,
		char **pcreate, char **pins, const char *table_name)
{
	size_t create_len = strlen("create table ''();") + strlen(table_name);
	size_t insert_len = strlen("insert into ''() values();") +
			strlen(table_name);

	for (size_t i = 0; i < nheaders; ++i) {
		create_len += strlen(headers[i].name) + strlen(" '") +
			strlen(type(headers[i].type)) + strlen("', ");

		insert_len += strlen("'") + strlen(headers[i].name) +
				strlen("', ") + strlen("?, ");
	}

	create_len -= 2; /* last ", " */
	insert_len -= 4; /* last ", " x2 */

	char *create = xmalloc(create_len + 1/*NUL*/, 1);
	if (!create)
		return -1;

	char *insert = xmalloc(insert_len + 1/*NUL*/, 1);
	if (!insert) {
		free(create);
		return -1;
	}

	int cr_written = sprintf(create, "create table '%s'(", table_name);
	int ins_written = sprintf(insert, "insert into '%s'(", table_name);
	for (size_t i = 0; i < nheaders - 1; ++i) {
		cr_written += sprintf(&create[cr_written], "'%s' %s, ",
				headers[i].name, type(headers[i].type));

		ins_written += sprintf(&insert[ins_written], "'%s', ",
				headers[i].name);
	}

	cr_written += sprintf(&create[cr_written], "'%s' %s);",
			headers[nheaders - 1].name,
			type(headers[nheaders - 1].type));

	ins_written += sprintf(&insert[ins_written], "'%s') values(",
			headers[nheaders - 1].name);

	for (size_t i = 0; i < nheaders - 1; ++i) {
		ins_written += sprintf(&insert[ins_written], "?, ");
	}
	ins_written += sprintf(&insert[ins_written], "?);");

	if (cr_written != create_len) {
		fprintf(stderr,
			"miscalculated needed buffer size 1 %d != %lu\n",
			cr_written, create_len);
		goto err;
	}

	if (ins_written != insert_len) {
		fprintf(stderr,
			"miscalculated needed buffer size 2 %d != %lu\n",
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
sqlite_type_to_str(int t)
{
	if (t == SQLITE_FLOAT)
		return "SQLITE_FLOAT";
	if (t == SQLITE_BLOB)
		return "SQLITE_BLOB";
	if (t == SQLITE_NULL)
		return "SQLITE_NULL";
	if (t == SQLITE_INTEGER)
		return "SQLITE_INTEGER";
	if (t == SQLITE_TEXT)
		return "SQLITE_TEXT";

	return "?";
}

static const char *
sqlite_type_to_csv_name(int t)
{
	if (t == SQLITE_INTEGER)
		return "int";
	else if (t == SQLITE_TEXT)
		return "string";
	else if (t == SQLITE_NULL)
		return NULL;

	fprintf(stderr, "unsupported sqlite type: %d [%s]\n", t,
			sqlite_type_to_str(t));
	exit(2);
}

static void
add_file(FILE *f, size_t num, sqlite3 *db, bool load_tables)
{
	struct csv_ctx *s = csv_create_ctx_nofail(f, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	struct table {
		char *name;

		struct col_header *headers;
		size_t nheaders;

		char *create;
		char *insert;
	} *tables = NULL;

	size_t ntables = 0;

	size_t table_column = SIZE_MAX;

	if (load_tables) {
		for (size_t i = 0; i < nheaders; ++i) {
			if (strcmp(headers[i].name, TABLE_COLUMN) == 0) {
				table_column = i;
				continue;
			}

			const char *sep =
				strchr(headers[i].name, TABLE_SEPARATOR);
			if (!sep) {
				fprintf(stderr,
					"column '%s' doesn't contain table separator (%c)\n",
					headers[i].name, TABLE_SEPARATOR);
				exit(2);
			}
			size_t table_len = (uintptr_t)sep
					- (uintptr_t)headers[i].name;

			struct table *t = NULL;
			/* XXX optimize? */
			for (size_t j = 0; j < ntables; ++j) {
				if (strncmp(headers[i].name,
						tables[j].name,
						table_len) == 0) {
					t = &tables[j];
					break;
				}
			}

			if (!t) {
				tables = xrealloc_nofail(tables,
						ntables + 1,
						sizeof(tables[0]));
				t = &tables[ntables++];
				t->name = xstrndup_nofail(headers[i].name,
						table_len);
				t->headers = NULL;
				t->nheaders = 0;
				t->create = NULL;
				t->insert = NULL;
			}

			t->headers = xrealloc_nofail(t->headers,
					t->nheaders + 1, sizeof(t->headers[0]));
			t->headers[t->nheaders].name = headers[i].name
					+ table_len + 1;
			t->headers[t->nheaders].type = headers[i].type;
			t->nheaders++;
		}

		for (size_t i = 0; i < ntables; ++i) {
			struct table *t = &tables[i];

			if (build_queries(t->headers, t->nheaders, &t->create,
					&t->insert, t->name))
				exit(2);
		}
	} else {
		struct table *t;

		ntables = 1;
		tables = t = xmalloc_nofail(1, sizeof(tables[0]));

		/* not used when tables are disabled */
		t->headers = NULL;
		t->nheaders = 0;
		t->name = NULL;

		char table_name[32];
		if (num == SIZE_MAX)
			strcpy(table_name, "input");
		else
			sprintf(table_name, "input%ld", num);

		if (build_queries(headers, nheaders, &t->create, &t->insert,
				table_name))
			exit(2);
	}

	struct cb_params params;
	params.db = db;
	params.tables = load_tables;
	params.ntables = ntables;
	params.inserts = xcalloc_nofail(ntables, sizeof(params.inserts[0]));
	params.table_column = table_column;

	for (size_t i = 0; i < ntables; ++i) {
		struct table *t = &tables[i];
		if (sqlite3_exec(db, t->create, NULL, NULL, NULL) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_exec(create='%s'): %s\n",
					t->create,
					sqlite3_errmsg(db));
			exit(2);
		}
		free(t->create);
		t->create = NULL;

		params.inserts[i].name = t->name;
		if (sqlite3_prepare_v2(db, t->insert, -1,
				&params.inserts[i].insert, NULL) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_prepare_v2(insert='%s'): %s\n",
					t->insert, sqlite3_errmsg(db));
			exit(2);
		}

	}

	csv_read_all_nofail(s, &next_row, &params);

	for (size_t i = 0; i < ntables; ++i) {
		struct table *t = &tables[i];

		if (sqlite3_finalize(params.inserts[i].insert) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_finalize(insert): %s\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		free(t->insert);
		t->insert = NULL;

		free(t->name);
		t->name = NULL;
	}

	free(tables);

	free(params.inserts);

	csv_destroy_ctx(s);
}

static void
print_col(sqlite3_stmt *select, size_t i)
{
	const char *name = sqlite3_column_name(select, i);
	int type = sqlite3_column_type(select, i);
	const char *type_str = sqlite_type_to_csv_name(type);

	if (type_str) {
		printf("%s:%s", name, type_str);
		return;
	}

	/* guess */
	printf("%s:string", name);
}

static void
print_val(sqlite3_stmt *select, size_t i)
{
	int type = sqlite3_column_type(select, i);
	if (type == SQLITE_INTEGER) {
		printf("%lld", sqlite3_column_int64(select, i));
	} else if (type == SQLITE_TEXT) {
		const char *txt = (const char *)sqlite3_column_text(select, i);
		csv_print_quoted(txt, strlen(txt));
	} else if (type == SQLITE_NULL) {
		/* nothing to do here */
	} else {
		fprintf(stderr, "unsupported return type: %d [%s]\n", type,
				sqlite_type_to_str(type));
		exit(2);
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	bool show = false;
	bool show_full;
	bool tables = false;
	char **inputs = NULL;
	size_t ninputs = 0;

	while ((opt = getopt_long(argc, argv, "i:sST", opts, NULL)) != -1) {
		switch (opt) {
			case 'i':
				inputs = xrealloc_nofail(inputs,
						++ninputs, sizeof(inputs[0]));

				inputs[ninputs - 1] = xstrdup_nofail(optarg);
				break;
			case 'T':
				tables = true;
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (optind != argc - 1) {
		usage(stderr);
		exit(2);
	}

	if (show)
		csv_show(show_full);

	sqlite3 *db;
	if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
		exit(2);
	}

	if (ninputs == 0) {
		add_file(stdin, SIZE_MAX, db, tables);
	} else {
		bool stdin_used = false;
		for (size_t i = 0; i < ninputs; ++i) {
			FILE *f;
			char *path = inputs[i];

			if (strcmp(path, "-") == 0) {
				if (stdin_used) {
					fprintf(stderr,
						"stdin used multiple times\n");
					exit(2);
				}

				f = stdin;
				stdin_used = true;
			} else
				f = fopen(path, "r");

			if (!f) {
				perror("fopen");
				exit(2);
			}

			add_file(f, i + 1, db, tables);

			if (strcmp(path, "-") != 0)
				fclose(f);

			free(path);
		}

		free(inputs);
		inputs = NULL;
	}

	sqlite3_stmt *select;
	int cnt;
	int ret;
	const char *stmt = argv[optind];

	do {
		const char *tail = NULL;
		if (sqlite3_prepare_v2(db, stmt, -1, &select, &tail) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_prepare_v2(select='%s'): %s\n",
					argv[optind], sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_step(select);
		if (ret != SQLITE_ROW && ret != SQLITE_DONE) {
			fprintf(stderr, "sqlite3_step(select): %s\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		cnt = sqlite3_column_count(select);
		if (cnt == 0 && tail) {
			/*
			 * This means the previous statement wasn't a select
			 * statement (it could be eg create index). Ignore it
			 * and parse the next statement.
			 */
			if (sqlite3_finalize(select) != SQLITE_OK) {
				fprintf(stderr, "sqlite3_finalize(select): %s\n",
						sqlite3_errmsg(db));
				exit(2);
			}

			stmt = tail;
			continue;
		}

		if (cnt < 1) {
			fprintf(stderr, "column count(%d) < 1\n", cnt);
			exit(2);
		}

		break;
	} while (1);

	for (size_t i = 0; i < cnt - 1; ++i) {
		print_col(select, i);
		fputc(',', stdout);
	}

	print_col(select, cnt - 1);
	fputc('\n', stdout);

	while (ret == SQLITE_ROW) {
		for (size_t i = 0; i < cnt - 1; ++i) {
			print_val(select, i);
			fputc(',', stdout);
		}

		print_val(select, cnt - 1);
		fputc('\n', stdout);

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

	return 0;
}
