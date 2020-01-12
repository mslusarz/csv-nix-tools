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
csv_ht_init(struct csv_ht **htp, void (*destroy_value)(void *))
{
	struct csv_ht *ht;
	ht = *htp = xmalloc(1, sizeof(*ht));
	if (!ht)
		return 2;

	ht->keys_max = 4;
	ht->table_size = 4;
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
