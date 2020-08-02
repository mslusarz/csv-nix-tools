/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_THREAD_UTILS_H
#define CSV_THREAD_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static inline void
thrd_create_nofail(thrd_t *t, thrd_start_t f, void *arg)
{
	int ret = thrd_create(t, f, arg);
	if (ret != thrd_success) {
		fprintf(stderr, "thrd_create failed (%d)\n", ret);
		exit(2);
	}
}

static inline void
thrd_join_nofail(thrd_t t, int *res)
{
	int ret = thrd_join(t, res);
	if (ret != thrd_success) {
		fprintf(stderr, "thrd_join failed (%d)\n", ret);
		exit(2);
	}
}

static inline void
cnd_init_nofail(cnd_t *c)
{
	int ret = cnd_init(c);
	if (ret != thrd_success) {
		fprintf(stderr, "cnd_init failed: %d\n", ret);
		exit(2);
	}
}

static inline void
cnd_signal_nofail(cnd_t *c)
{
	int ret = cnd_signal(c);
	if (ret != thrd_success) {
		fprintf(stderr, "cnd_signal failed: %d\n", ret);
		exit(2);
	}
}

static inline void
cnd_wait_nofail(cnd_t *c, mtx_t *m)
{
	int ret = cnd_wait(c, m);
	if (ret != thrd_success) {
		fprintf(stderr, "cnd_wait failed: %d\n", ret);
		exit(2);
	}
}

static inline void
mtx_init_nofail(mtx_t *m, int type)
{
	int ret = mtx_init(m, type);
	if (ret != thrd_success) {
		fprintf(stderr, "mtx_init failed: %d\n", ret);
		exit(2);
	}
}

static inline void
mtx_lock_nofail(mtx_t *m)
{
	int ret = mtx_lock(m);
	if (ret != thrd_success) {
		fprintf(stderr, "mtx_lock failed: %d\n", ret);
		exit(2);
	}
}

static inline void
mtx_unlock_nofail(mtx_t *m)
{
	int ret = mtx_unlock(m);
	if (ret != thrd_success) {
		fprintf(stderr, "mtx_unlock failed: %d\n", ret);
		exit(2);
	}
}

#endif
