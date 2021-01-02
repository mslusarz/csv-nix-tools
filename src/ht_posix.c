/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <search.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht.h"

#include "ht_internal.h"
#include "utils.h"

struct csv_ht {
	size_t keys_max;
	struct csv_ht_internal *ht;

	ENTRY *table;
	size_t table_size;
	size_t inserted;
	void (*destroy_value)(void *);
};

int
csv_ht_init(struct csv_ht **htp, void (*destroy_value)(void *),
		size_t initial_size)
{
	struct csv_ht *ht;
	ht = *htp = xmalloc(1, sizeof(*ht));
	if (!ht)
		return 2;

	if (initial_size == 0)
		initial_size = 4;
	ht->keys_max = initial_size;
	ht->table_size = initial_size;
	ht->ht = NULL;
	ht->inserted = 0;
	ht->destroy_value = destroy_value;

	ht->table = xmalloc(ht->table_size, sizeof(ht->table[0]));
	if (!ht->table) {
		free(ht);
		*htp = NULL;
		return 2;
	}

	if (csv_hcreate_r(ht->keys_max, &ht->ht) == 0) {
		perror("hcreate_r");
		free(ht->table);
		free(ht);
		*htp = NULL;
		return 2;
	}

	return 0;
}

void
csv_ht_destroy(struct csv_ht **htp)
{
	struct csv_ht *ht = *htp;

	csv_hdestroy_r(&ht->ht);
	for (size_t u = 0; u < ht->inserted; ++u) {
		free(ht->table[u].key);
		if (ht->destroy_value)
			ht->destroy_value(ht->table[u].data);
	}
	free(ht->table);
	free(ht);

	*htp = NULL;
}

void *
csv_ht_get_value(struct csv_ht *ht, char *key, void *(*cb)(void *), void *cb_data)
{
	ENTRY e;
	ENTRY *entry = NULL;

	e.key = key;
	e.data = NULL;

	if (csv_hsearch_r(e, ENTER, &entry, ht->ht) == 0) {
		/* hash map too small, recreate it */
		csv_hdestroy_r(&ht->ht);
		ht->keys_max *= 2;

		if (csv_hcreate_r(ht->keys_max, &ht->ht) == 0) {
			perror("hcreate_r");
			exit(2); /* no sane way to handle */
		}

		for (size_t u = 0; u < ht->inserted; ++u) {
			e.key = ht->table[u].key;
			e.data = NULL;

			if (csv_hsearch_r(e, ENTER, &entry, ht->ht) == 0)
				exit(2); /* impossible */

			entry->key = ht->table[u].key;
			entry->data = ht->table[u].data;
		}

		return csv_ht_get_value(ht, key, cb, cb_data);
	}

	if (entry->data != NULL)
		return entry->data;

	if (ht->inserted == ht->table_size) {
		ht->table_size *= 2;
		ht->table = xrealloc_nofail(ht->table, ht->table_size,
				sizeof(ht->table[0]));
	}

	entry->key = xstrdup_nofail(key);

	entry->data = cb(cb_data);
	if (!entry->data)
		exit(2); /* no sane way to handle */

	ht->table[ht->inserted].key = entry->key;
	ht->table[ht->inserted].data = entry->data;
	ht->inserted++;

	return entry->data;
}
