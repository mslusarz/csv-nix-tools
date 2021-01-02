/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_HT_H
#define CSV_HT_H

struct csv_ht;
int csv_ht_init(struct csv_ht **ht, void (*destroy_value)(void *),
		size_t initial_size);
void csv_ht_destroy(struct csv_ht **ht);
void *csv_ht_get_value(struct csv_ht *ht, char *key,
		void *(*cb)(void *), void *cb_data);

#endif
