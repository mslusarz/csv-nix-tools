/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_USR_GRP_QUERY_H
#define CSV_USR_GRP_QUERY_H

#include <grp.h>
#include <pwd.h>

const char *get_user(uid_t uid);
const char *get_group(gid_t gid);

int usr_grp_query_init(void);
void usr_grp_query_fini(void);

#endif
