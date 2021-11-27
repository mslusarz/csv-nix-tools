/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

char *
xstrdup(const char *str)
{
	char *ret = strdup(str);
	if (!ret)
		perror("strdup");

	return ret;
}

char *
xstrndup(const char *str, size_t n)
{
	char *ret = strndup(str, n);
	if (!ret)
		perror("strndup");

	return ret;
}

void *
xmalloc(size_t count, size_t size)
{
	void *ret = malloc(count * size);
	if (!ret)
		perror("malloc");

	return ret;
}

void *
xcalloc(size_t count, size_t size)
{
	void *ret = calloc(count, size);
	if (!ret)
		perror("calloc");

	return ret;
}

void *
xrealloc(void *ptr, size_t count, size_t size)
{
	void *ret = realloc(ptr, count * size);
	if (!ret)
		perror("realloc");

	return ret;
}
