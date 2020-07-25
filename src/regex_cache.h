/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_REGEX_CACHE_H
#define CSV_REGEX_CACHE_H

#include <regex.h>

int csv_regex_get(regex_t **preg, const char *regex, int cflags);
void csv_regex_clear_cache(void);

#endif
