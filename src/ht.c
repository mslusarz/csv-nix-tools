/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include "ht.h"
#include "utils.h"

#define OVERHEAD 1.5
#define MAX_COLLISIONS 0.2
#define MIN_ENTRIES 0.01

struct kv {
	char *key;
	void *value;
	struct kv *next;
};

struct csv_ht {
	void (*destroy_value)(void *);
	size_t size;
	size_t used;
	size_t collisions;

	struct kv *kvs;
};

int
csv_ht_init(struct csv_ht **_ht, void (*destroy_value)(void *),
		size_t initial_size)
{
	struct csv_ht *ht = xmalloc(1, sizeof(*ht));
	if (!ht)
		return 2;

	if (initial_size == 0)
		initial_size = 1;

	ht->size = initial_size * OVERHEAD;
	ht->kvs = xcalloc(ht->size, sizeof(*ht->kvs));
	if (!ht->kvs) {
		free(ht);
		return 2;
	}
	ht->destroy_value = destroy_value;
	ht->used = 0;
	ht->collisions = 0;

	*_ht = ht;
	return 0;
}

static uint64_t
hash_str(char *data)
{
	size_t len = strlen(data);
	const uint64_t MD = 4294967291;
	uint64_t a = 1, b = 0;
	size_t index;

	for (index = 0; index < len / 4; ++index) {
		a = (a + ((uint32_t *)data)[index]) % MD;
		b = (b + a) % MD;
	}
	for (index = 4 * (len / 4); index < len; ++index) {
		a = (a + ((unsigned char *)data)[index]) % MD;
		b = (b + a) % MD;
	}

	return (b << 32) | a;
}

void
csv_ht_destroy(struct csv_ht **_ht)
{
	struct csv_ht *ht = *_ht;

	for (size_t i = 0; i < ht->size; ++i) {
		struct kv *kv = &ht->kvs[i];

		free(kv->key);
		if (kv->value)
			ht->destroy_value(kv->value);

		kv = kv->next;
		while (kv) {
			struct kv *next = kv->next;
			free(kv->key);
			ht->destroy_value(kv->value);
			free(kv);
			kv = next;
		}
	}

	free(ht->kvs);
	free(ht);
	*_ht = NULL;
}

static void
linkin(struct csv_ht *ht, size_t h, char *key, void *value)
{
	struct kv *kvs = ht->kvs;
	ht->used++;
	if (kvs[h].key == NULL) {
		kvs[h].key = key;
		kvs[h].value = value;
		kvs[h].next = NULL;
		return;
	}

	struct kv *n = xmalloc_nofail(1, sizeof(*n));
	n->key = key;
	n->value = value;
	n->next = kvs[h].next;
	kvs[h].next = n;
	ht->collisions++;
}

static void
insert(struct csv_ht *ht, char *key, void *value)
{
	uint64_t h = hash_str(key) % ht->size;
	linkin(ht, h, key, value);
}

static void
resize(struct csv_ht *ht)
{
	size_t oldsize = ht->size;
	struct kv *oldkvs = ht->kvs;

	ht->size *= 2;
	ht->kvs = xcalloc_nofail(ht->size, sizeof(ht->kvs[0]));
	ht->collisions = 0;

	for (size_t i = 0; i < oldsize; ++i) {
		struct kv *kv = &oldkvs[i];
		if (!kv->key)
			continue;
		insert(ht, kv->key, kv->value);
		kv = kv->next;
		while (kv) {
			insert(ht, kv->key, kv->value);
			struct kv *next = kv->next;
			free(kv);
			kv = next;
		}
	}
	free(oldkvs);
}

void *
csv_ht_get_value(struct csv_ht *ht, char *key,
		void *(*cb)(void *), void *cb_data)
{
	uint64_t h = hash_str(key) % ht->size;

	struct kv *kv = &ht->kvs[h];

	if (kv->key) {
		while (kv) {
			if (strcmp(key, kv->key) == 0)
				return kv->value;
			kv = kv->next;
		}
	}

	if (ht->collisions > MAX_COLLISIONS * ht->size &&
			ht->used > MIN_ENTRIES * ht->size)
		resize(ht);

	void *v = cb(cb_data);
	linkin(ht, h, xstrdup_nofail(key), v);

	return v;
}
