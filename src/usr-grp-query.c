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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ht.h"
#include "utils.h"
#include "usr-grp-query.h"

static struct csv_ht *users_ht;
static struct csv_ht *groups_ht;

static void *
get_user_slow(void *uidp)
{
	char *ret;
	uid_t uid = *(uid_t *)uidp;
	struct passwd *passwd = getpwuid(uid);
	if (passwd)
		return xstrdup(passwd->pw_name);

	if (csv_asprintf(&ret, "%d", uid) < 0) {
		perror("asprintf");
		ret = NULL;
	}

	return ret;
}

const char *
get_user(uid_t uid)
{
	char key[30];
	sprintf(key, "%d", uid);

	return csv_ht_get_value(users_ht, key, get_user_slow, &uid);
}

static void *
get_group_slow(void *gidp)
{
	char *ret;
	gid_t gid = *(uid_t *)gidp;
	struct group *gr = getgrgid(gid);
	if (gr)
		return xstrdup(gr->gr_name);

	if (csv_asprintf(&ret, "%d", gid) < 0) {
		perror("asprintf");
		ret = NULL;
	}

	return ret;
}

const char *
get_group(gid_t gid)
{
	char key[30];
	sprintf(key, "%d", gid);

	return csv_ht_get_value(groups_ht, key, get_group_slow, &gid);
}

static void
destroy_value(void *v)
{
	/* the stuff allocated by get_user_slow & get_group_slow */
	free(v);
}

int
usr_grp_query_init(void)
{
	if (csv_ht_init(&users_ht, &destroy_value))
		return 2;

	if (csv_ht_init(&groups_ht, &destroy_value)) {
		csv_ht_destroy(&users_ht);
		return 2;
	}

	return 0;
}

void
usr_grp_query_fini(void)
{
	csv_ht_destroy(&users_ht);
	csv_ht_destroy(&groups_ht);
}
