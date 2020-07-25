/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "regex_cache.h"
#include "utils.h"

#define CACHE_SIZE 10

static struct reg {
	char *regex;
	int cflags;
	regex_t comp;
} Cache[CACHE_SIZE];

static unsigned Used;
static unsigned Next_to_use;

int
csv_regex_get(regex_t **preg, const char *regex, int cflags)
{
	struct reg *r;
	for (unsigned i = 0; i < Used; ++i) {
		r = &Cache[i];

		if (r->cflags != cflags)
			continue;
		if (r->regex == NULL)
			continue;
		if (strcmp(r->regex, regex) != 0)
			continue;

		*preg = &r->comp;
		return 0;
	}

	r = &Cache[Next_to_use];
	if (r->regex) {
		regfree(&r->comp);
		free(r->regex);
		memset(r, 0, sizeof(*r));
	}

	int ret = regcomp(&r->comp, regex, cflags);
	if (ret) {
		size_t len = regerror(ret, &r->comp, NULL, 0);
		char *errbuf = xmalloc_nofail(len, 1);
		regerror(ret, &r->comp, errbuf, len);
		fprintf(stderr,
			"compilation of expression '%s' failed: %s\n",
			regex, errbuf);
		free(errbuf);
		memset(&r->comp, 0, sizeof(r->comp));
		*preg = NULL;
		return -1;
	}

	r->cflags = cflags;
	r->regex = xstrdup_nofail(regex);
	if (Used != CACHE_SIZE)
		Used++;
	Next_to_use++;
	if (Next_to_use == CACHE_SIZE)
		Next_to_use = 0;

	*preg = &r->comp;

	return 0;
}

void
csv_regex_clear_cache(void)
{
	for (unsigned i = 0; i < Used; ++i) {
		struct reg *r = &Cache[i];

		if (r->regex == NULL)
			continue;

		free(r->regex);
		regfree(&r->comp);
		memset(r, 0, sizeof(*r));
	}

	Used = 0;
	Next_to_use = 0;
}
