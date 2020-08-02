/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "thread_utils.h"
#include "utils.h"

static const struct option Opts[] = {
	{"columns",		required_argument,	NULL, 'c'},
	{"diffs",		required_argument,	NULL, 'd'},
	{"colors",		required_argument,	NULL, 'C'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"help",		no_argument,		NULL, 'h'},
	{"version",		no_argument,		NULL, 'V'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-diff [OPTION]... FILE1 FILE2 [FILE3] [FILE4]...\n");
	fprintf(out,
"Read CSV streams from 2 or more files, merge them with an indication of\n"
"differences in values and print resulting CSV file to standard output.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -c, --columns=NAME1[,NAME2...]\n"
"                             select only these columns\n");
	fprintf(out,
"  -C, --colors=[0|1]         add color columns (auto enabled with --show-full)\n");
	fprintf(out,
"  -d, --diffs=[0|1]          add diff columns (auto enabled with --show)\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct input {
	FILE *f;
	struct csv_ctx *s;
	size_t *idx;
	thrd_t thrd;

	mtx_t mtx;
	const char *buf;
	const size_t *col_offs;
	cnd_t consumed_cond;
	bool consumed;
	bool eof;
};

static size_t Ninputs;
static size_t Nheaders;
static const struct col_header *Headers;

static void
add_input(FILE *f, struct input *in, size_t file_idx, size_t num_user_headers,
		struct col_header *user_headers)
{
	struct csv_ctx *s = csv_create_ctx_nofail(f, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers_cur;
	size_t nheaders_cur = csv_get_headers(s, &headers_cur);

	in->f = f;
	in->s = s;

	if (Headers == NULL) {
		if (num_user_headers) {
			Nheaders = num_user_headers;
			Headers = user_headers;

			in->idx = xmalloc_nofail(Nheaders, sizeof(in->idx[0]));

			for (size_t j = 0; j < Nheaders; ++j) {
				size_t idx = csv_find(headers_cur, nheaders_cur,
						user_headers[j].name);
				if (idx == CSV_NOT_FOUND) {
					fprintf(stderr,
						"column '%s' not found in input %ld\n",
						user_headers[j].name, file_idx);
					exit(2);
				}

				user_headers[j].type = headers_cur[idx].type;

				in->idx[j] = idx;
			}
		} else {
			Nheaders = nheaders_cur;
			Headers = headers_cur;

			in->idx = xmalloc_nofail(Nheaders, sizeof(in->idx[0]));

			for (size_t j = 0; j < Nheaders; ++j)
				in->idx[j] = j;
		}
	} else {
		in->idx = xmalloc_nofail(Nheaders, sizeof(in->idx[0]));

		for (size_t j = 0; j < Nheaders; ++j) {
			size_t idx = csv_find(headers_cur, nheaders_cur,
					Headers[j].name);
			if (idx == CSV_NOT_FOUND) {
				fprintf(stderr,
					"column '%s' not found in input %ld\n",
					Headers[j].name, file_idx);
				exit(2);
			}

			if (strcmp(Headers[j].type,
					headers_cur[idx].type) != 0) {
				fprintf(stderr,
					"column '%s' have different types in different inputs\n",
					Headers[j].name);
				exit(2);
			}

			in->idx[j] = idx;
		}
	}
}

static mtx_t GlobalDataMtx;
static cnd_t AllThreadsReady;
static size_t ThreadsReady;
static size_t Eofs;

static void
thread_data_ready(struct input *in, bool eof)
{
	mtx_unlock_nofail(&in->mtx);
	mtx_lock_nofail(&GlobalDataMtx);
	if (eof)
		++Eofs;
	if (++ThreadsReady == Ninputs)
		cnd_signal_nofail(&AllThreadsReady);
	mtx_unlock_nofail(&GlobalDataMtx);
	mtx_lock_nofail(&in->mtx);
}

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct input *in = arg;
	(void)ncols;

	in->buf = buf;
	in->col_offs = col_offs;
	in->consumed = false;

	thread_data_ready(in, false);

	while (!in->consumed)
		cnd_wait_nofail(&in->consumed_cond, &in->mtx);

	return 0;
}

static int
thr_work(void *arg)
{
	struct input *in = arg;

	mtx_init_nofail(&in->mtx, mtx_plain);
	cnd_init_nofail(&in->consumed_cond);
	in->eof = false;
	in->consumed = false;

	mtx_lock_nofail(&in->mtx);

	csv_read_all_nofail(in->s, &next_row, in);

	in->eof = true;
	thread_data_ready(in, true);

	mtx_unlock_nofail(&in->mtx);

	cnd_destroy(&in->consumed_cond);
	mtx_destroy(&in->mtx);

	return 0;
}

static const char *
Colors[] = {
	"red",
	"green",
	"blue",
	"magenta",
	"cyan",
	"yellow",
	"black",
};

static void
gather_and_compare(struct input *inputs, bool print_diff, bool print_colors)
{
	bool *diff = xmalloc_nofail(Nheaders, sizeof(diff[0]));

	while (true) {
		mtx_lock_nofail(&GlobalDataMtx);

		while (ThreadsReady != Ninputs)
			cnd_wait_nofail(&AllThreadsReady, &GlobalDataMtx);

		if (Eofs == Ninputs) {
			mtx_unlock_nofail(&GlobalDataMtx);
			free(diff);
			return;
		}

		if (Eofs > 0) {
			for (size_t i = 0; i < Ninputs; ++i) {
				if (!inputs[i].eof)
					continue;
				fprintf(stderr,
					"input file %zu ended before other(s)\n",
					i + 1);
			}
			exit(2);
		}

		ThreadsReady = 0;
		mtx_unlock_nofail(&GlobalDataMtx);

		for (size_t i = 0; i < Ninputs; ++i)
			mtx_lock_nofail(&inputs[i].mtx);

		bool any_diff = false;
		for (size_t i = 0; i < Nheaders; ++i) {
			struct input *in0 = &inputs[0];
			const char *tmpl = in0->buf + in0->col_offs[in0->idx[i]];

			diff[i] = false;
			for (size_t j = 1; j < Ninputs; ++j) {
				struct input *in = &inputs[j];
				const char *data = in->buf + in->col_offs[in->idx[i]];
				if (strcmp(tmpl, data) != 0) {
					diff[i] = true;
					any_diff = true;
					break;
				}
			}
		}

		for (size_t i = 0; i < Ninputs; ++i) {
			struct input *in = &inputs[i];
			if (any_diff) {
				printf("%zu,", i + 1);
				csv_print_line_reordered(stdout, in->buf,
						in->col_offs, Nheaders, false,
						in->idx);

				if (print_diff) {
					for (size_t j = 0; j < Nheaders; ++j) {
						if (diff[j])
							printf(",1");
						else
							printf(",0");
					}
				}

				if (print_colors) {
					for (size_t j = 0; j < Nheaders; ++j) {
						if (diff[j]) {
							printf(",bg=%s",
								Colors[i % ARRAY_SIZE(Colors)]);
						} else {
							printf(",");
						}
					}
				}

				printf("\n");
			}

			in->consumed = true;
			cnd_signal_nofail(&in->consumed_cond);
			mtx_unlock_nofail(&in->mtx);
		}
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	char *cols = NULL;
	unsigned show_flags = SHOW_DISABLED;
	int colors = -1;
	int diffs = -1;

	while ((opt = getopt_long(argc, argv, "c:C:d:sS", Opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'C':
				colors = atoi(optarg) != 0;
				break;
			case 'd':
				diffs = atoi(optarg) != 0;
				break;
			case 's':
				show_flags |= SHOW_SIMPLE;
				break;
			case 'S':
				show_flags |= SHOW_FULL;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	if (diffs == -1)
		diffs = show_flags & SHOW_SIMPLE;

	if (colors == -1)
		colors = show_flags & SHOW_FULL;

	if (colors && (show_flags & SHOW_FULL))
		show_flags |= SHOW_COLORS;

	assert(optind <= argc);
	Ninputs = (size_t)(argc - optind);
	if (Ninputs < 2) {
		usage(stderr);
		exit(2);
	}

	size_t num_user_headers = 0;
	struct col_header *user_headers = NULL;

	if (cols) {
		struct split_result *results = NULL;
		util_split_term(cols, ",", &results, &num_user_headers, NULL);

		user_headers = xmalloc_nofail(num_user_headers, sizeof(user_headers[0]));
		for (size_t i = 0; i < num_user_headers; ++i) {
			user_headers[i].name = cols + results[i].start;
			user_headers[i].type = NULL;
		}
		free(results);
	}

	struct input *inputs = xcalloc_nofail(Ninputs, sizeof(inputs[0]));

	bool stdin_used = false;
	size_t i = 0;
	while (optind < argc) {
		FILE *f;
		if (strcmp(argv[optind], "-") == 0) {
			if (stdin_used) {
				fprintf(stderr,
					"stdin is used more than once\n");
				exit(2);
			}
			f = stdin;
			stdin_used = true;
		} else {
			f = fopen(argv[optind], "r");
			if (!f) {
				fprintf(stderr, "opening '%s' failed: %s\n",
					argv[optind], strerror(errno));
				exit(2);
			}
		}

		add_input(f, &inputs[i], i + 1, num_user_headers, user_headers);

		optind++;
		i++;
	}

	csv_show(show_flags);

	printf("input:int");

	for (size_t i = 0; i < Nheaders; ++i)
		printf(",%s:%s", Headers[i].name, Headers[i].type);

	if (diffs) {
		for (size_t i = 0; i < Nheaders; ++i)
			printf(",%s_diff:int", Headers[i].name);
	}

	if (colors) {
		for (size_t i = 0; i < Nheaders; ++i)
			printf(",%s_color:string", Headers[i].name);
	}

	printf("\n");

	mtx_init_nofail(&GlobalDataMtx, mtx_plain);
	cnd_init_nofail(&AllThreadsReady);

	ThreadsReady = 0;
	Eofs = 0;

	for (size_t i = 0; i < Ninputs; ++i) {
		struct input *in = &inputs[i];

		thrd_create_nofail(&in->thrd, thr_work, in);
	}

	gather_and_compare(inputs, diffs, colors);

	for (size_t i = 0; i < Ninputs; ++i) {
		struct input *in = &inputs[i];
		thrd_join_nofail(in->thrd, NULL);
		csv_destroy_ctx(in->s);
		free(in->idx);
		fclose(in->f);
	}

	cnd_destroy(&AllThreadsReady);
	mtx_destroy(&GlobalDataMtx);

	free(user_headers);
	free(cols);
	free(inputs);

	return 0;
}
