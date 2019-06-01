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

#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>

#include "usr-grp.h"

void
load_users_into_db(sqlite3 *db)
{
	static const char *cr = "create table users (name text, passwd text, "
			"uid int, gid int, gecos text, dir text, shell text);";
	if (sqlite3_exec(db, cr, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec(create): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	static const char *ins = "insert into users (name, passwd, uid, gid, "
			"gecos, dir, shell) values(?, ?, ?, ?, ?, ?, ?);";

	sqlite3_stmt *insert;

	if (sqlite3_prepare_v2(db, ins, -1, &insert, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_prepare_v2(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	for (size_t i = 0; i < nusers; ++i) {
		struct csv_user *u = &users[i];
		int ret;

		ret = sqlite3_bind_text(insert, 1, u->name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, name\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_text(insert, 2, u->passwd, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, passwd\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_int64(insert, 3, u->uid);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_int64: %s, uid\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_int64(insert, 4, u->gid);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_int64: %s, gid\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_text(insert, 5, u->gecos, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, gecos\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_text(insert, 6, u->dir, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, dir\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_text(insert, 7, u->shell, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, shell\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		if (sqlite3_step(insert) != SQLITE_DONE) {
			fprintf(stderr, "sqlite3_step: %s\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		if (sqlite3_reset(insert) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_reset: %s\n",
					sqlite3_errmsg(db));
			exit(2);
		}
	}

	if (sqlite3_finalize(insert) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_finalize(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}
}

void
load_groups_into_db(sqlite3 *db)
{
	static const char *cr =
			"create table groups (name text, passwd text, gid int);";
	if (sqlite3_exec(db, cr, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec(create): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	static const char *ins =
			"insert into groups (name, passwd, gid) values(?, ?, ?);";

	sqlite3_stmt *insert;

	if (sqlite3_prepare_v2(db, ins, -1, &insert, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_prepare_v2(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	for (size_t i = 0; i < ngroups; ++i) {
		struct csv_group *g = &groups[i];
		int ret;

		ret = sqlite3_bind_text(insert, 1, g->name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, name\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_text(insert, 2, g->passwd, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, passwd\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		ret = sqlite3_bind_int64(insert, 3, g->gid);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_int64: %s, gid\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		if (sqlite3_step(insert) != SQLITE_DONE) {
			fprintf(stderr, "sqlite3_step: %s\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		if (sqlite3_reset(insert) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_reset: %s\n",
					sqlite3_errmsg(db));
			exit(2);
		}
	}

	if (sqlite3_finalize(insert) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_finalize(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}
}

void
load_group_members_into_db(sqlite3 *db)
{
	static const char *cr =
		"create table group_members (group_name text, user_name text);";
	if (sqlite3_exec(db, cr, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec(create): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	static const char *ins =
		"insert into group_members (group_name, user_name) values(?, ?);";

	sqlite3_stmt *insert;

	if (sqlite3_prepare_v2(db, ins, -1, &insert, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_prepare_v2(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	for (size_t i = 0; i < ngroups; ++i) {
		struct csv_group *g = &groups[i];
		int ret;

		ret = sqlite3_bind_text(insert, 1, g->name, -1, SQLITE_STATIC);
		if (ret != SQLITE_OK) {
			fprintf(stderr, "sqlite3_bind_text: %s, group_name\n",
					sqlite3_errmsg(db));
			exit(2);
		}

		for (size_t k = 0; k < g->nmembers; ++k) {
			ret = sqlite3_bind_text(insert, 2, g->members[k], -1,
					SQLITE_STATIC);
			if (ret != SQLITE_OK) {
				fprintf(stderr, "sqlite3_bind_text: %s, user_name\n",
						sqlite3_errmsg(db));
				exit(2);
			}

			if (sqlite3_step(insert) != SQLITE_DONE) {
				fprintf(stderr, "sqlite3_step: %s\n",
						sqlite3_errmsg(db));
				exit(2);
			}

			if (sqlite3_reset(insert) != SQLITE_OK) {
				fprintf(stderr, "sqlite3_reset: %s\n",
						sqlite3_errmsg(db));
				exit(2);
			}
		}
	}

	if (sqlite3_finalize(insert) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_finalize(insert): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

	static const char *ins2 =
		"insert into group_members (user_name, group_name) \n"
		"select users.name, groups.name \n"
		"  from users, groups \n"
		" where users.gid = groups.gid \n"
		"   and (users.name, groups.name) not in \n"
		"       (select user_name, group_name from group_members)";
	if (sqlite3_exec(db, ins2, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "sqlite3_exec(ins2): %s\n",
				sqlite3_errmsg(db));
		exit(2);
	}

}
