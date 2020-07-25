/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_USR_GRP_H
#define CSV_USR_GRP_H

#include <stddef.h>
#include <sys/types.h>

struct csv_group;

struct csv_user
{
	char *name;
	char *passwd;
	uid_t uid;
	gid_t gid;
	char *gecos;
	char *dir;
	char *shell;

	struct csv_group **groups;
	size_t ngroups;
};

struct csv_group
{
	char *name;
	char *passwd;
	gid_t gid;
	char **members;
	size_t nmembers;

	struct csv_user **users;
};

extern struct csv_user *users;
extern size_t nusers;

extern struct csv_group *groups;
extern size_t ngroups;

void load_users(void);
void load_groups(void);
struct csv_user *find_user_by_name(const char *name);
struct csv_group *find_group_by_gid(gid_t gid);

void free_users(void);
void free_groups(void);

#endif
