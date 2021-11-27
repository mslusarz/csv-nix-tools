/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <errno.h>
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
	static bool params_loaded = false;
	static size_t fail_on_size = SIZE_MAX;
	if (!params_loaded) {
		const char *v = getenv("CSVNIXTOOLS_XMALLOC_FAIL_ON_SIZE");
		if (v)
			fail_on_size = strtoul(v, NULL, 0);
		params_loaded = true;
	}
	size *= count;

#if 0
	fprintf(stderr, "xmalloc(%zu)\n", size);
#endif

	void *ret;
	if (size == fail_on_size) {
		ret = NULL;
		errno = ENOMEM;
	} else {
		ret = malloc(size);
	}

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
