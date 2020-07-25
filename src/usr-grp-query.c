/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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
