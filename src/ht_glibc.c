/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */
#define _GNU_SOURCE

#include <stdlib.h>

#include "ht_internal.h"
#include "utils.h"

struct csv_ht_internal {
	struct hsearch_data d;
};

int
csv_hcreate_r(size_t nel, struct csv_ht_internal **ht)
{
	*ht = xcalloc_nofail(1, sizeof(**ht));

	return hcreate_r(nel, &(*ht)->d);
}

int
csv_hsearch_r(ENTRY item, ACTION action, ENTRY **retval,
              struct csv_ht_internal *ht)
{
	return hsearch_r(item, action, retval, &ht->d);
}

void
csv_hdestroy_r(struct csv_ht_internal **ht)
{
	hdestroy_r(&(*ht)->d);
	free(*ht);
	*ht = NULL;
}
