/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_HT_INTERNAL_H
#define CSV_HT_INTERNAL_H

#include <search.h>
#include <stddef.h>

struct csv_ht_internal;
int csv_hcreate_r(size_t nel, struct csv_ht_internal **htab);
int csv_hsearch_r(ENTRY item, ACTION action, ENTRY **retval, struct csv_ht_internal *ht);
void csv_hdestroy_r(struct csv_ht_internal **ht);

#endif
