/*
 * Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usr-grp.h"
#include "utils.h"

struct visible_columns {
	char name;
	char passwd;
	char uid;
	char gid;
	char gecos;
	char dir;
	char shell;
	char group_ids;
	char group_names;
};

struct visibility_info {
	struct visible_columns cols;
	size_t count;
};

static const struct option long_options[] = {
	{"fields",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-users [OPTION]...\n");
	printf("Options:\n");
	printf("  -f, --fields=name1[,name2...]\n");
	printf("  -s, --show\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
}

static void
merge_groups_into_users(void)
{
	for (size_t i = 0; i < ngroups; ++i) {
		struct csv_group *g = &groups[i];
		char **members = g->members;

		for (size_t j = 0; j < g->nmembers; ++j) {
			struct csv_user *u = find_user_by_name(members[j]);
			if (u) {
				u->groups = xrealloc_nofail(u->groups,
						u->ngroups + 1,
						sizeof(u->groups[0]));

				u->groups[u->ngroups++] = g;
			} else {
				fprintf(stderr,
					"warning: group '%s' contains unknown user '%s'\n",
					g->name, *members);
			}
		}
	}

	for (size_t i = 0; i < nusers; ++i) {
		struct csv_user *u = &users[i];

		struct csv_group *g = find_group_by_gid(u->gid);
		if (!g) {
			fprintf(stderr,
				"warning: user '%s' belongs to unknown group %d\n",
				u->name, u->gid);
			continue;
		}

		bool found = false;
		for (size_t j = 0; j < u->ngroups; ++j) {
			if (u->groups[j] == g) {
				found = true;
				break;
			}
		}

		if (!found) {
			u->groups = xrealloc_nofail(u->groups,
					u->ngroups + 1,
					sizeof(u->groups[0]));

			u->groups[u->ngroups++] = g;
		}
	}
}

struct print_ctx {
	size_t printed;
	const struct visibility_info *visinfo;
};

static void
cprint(struct print_ctx *ctx, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);

	if (++ctx->printed < ctx->visinfo->count)
		fputc(',', stdout);
	else
		fputc('\n', stdout);
}

static void
eval_col(char vis, const char *str, int print, size_t *visible_count, size_t count)
{
	if (!vis)
		return;
	(*visible_count)++;
	if (!print)
		return;

	fputs(str, stdout);

	if (*visible_count < count)
		fputc(',', stdout);
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	char *cols = NULL;
	struct visible_columns vis;
	bool print_header = true;
	bool show = false;

	memset(&vis, 1, sizeof(vis));

	while ((opt = getopt_long(argc, argv, "f:s", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 's':
				show = true;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					default:
						usage();
						return 2;
				}
				break;
			case 'h':
			default:
				usage();
				return 2;
		}
	}

	if (cols) {
		memset(&vis, 0, sizeof(vis));

		const struct {
			const char *name;
			char *vis;
		} map[] = {
				{ "name", &vis.name },
				{ "passwd", &vis.passwd },
				{ "uid", &vis.uid },
				{ "gid", &vis.gid },
				{ "gecos", &vis.gecos },
				{ "dir", &vis.dir },
				{ "shell", &vis.shell },
				{ "group_ids", &vis.group_ids },
				{ "group_names", &vis.group_names },
		};

		char *name = strtok(cols, ",");
		while (name) {
			int found = 0;
			for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); ++i) {
				if (strcmp(name, map[i].name) == 0) {
					*map[i].vis = 1;
					found = 1;
					break;
				}
			}

			if (!found) {
				fprintf(stderr, "column %s not found\n", name);
				exit(2);
			}

			name = strtok(NULL, ",");
		}

		free(cols);
	}

	if (show)
		csv_show();

	size_t visible = 0;
	size_t count = sizeof(vis) + 1;
	int print = 0;

	do {
		eval_col(vis.name, "name:string", print, &visible, count);
		eval_col(vis.passwd, "passwd:string", print, &visible, count);
		eval_col(vis.uid, "uid:int", print, &visible, count);
		eval_col(vis.gid, "gid:int", print, &visible, count);
		eval_col(vis.gecos, "gecos:string", print, &visible, count);
		eval_col(vis.dir, "dir:string", print, &visible, count);
		eval_col(vis.shell, "shell:string", print, &visible, count);
		eval_col(vis.group_ids, "group_ids:string", print, &visible, count);
		eval_col(vis.group_names, "group_names:string", print, &visible, count);

		count = visible;
		visible = 0;
		if (print_header)
			print++;
	} while (print == 1);

	if (print_header)
		printf("\n");

	struct visibility_info visinfo = {vis, count};

	load_users();
	if (vis.group_ids || vis.group_names) {
		load_groups();
		merge_groups_into_users();
	}

	for (size_t i = 0; i < nusers; ++i) {
		const struct csv_user *u = &users[i];
		struct print_ctx ctx = {0, &visinfo};
		if (vis.name) {
			csv_print_quoted(u->name, strlen(u->name));
			cprint(&ctx, "");
		}

		if (vis.passwd) {
			csv_print_quoted(u->passwd, strlen(u->passwd));
			cprint(&ctx, "");
		}

		if (vis.uid)
			cprint(&ctx, "%lu", u->uid);

		if (vis.gid)
			cprint(&ctx, "%lu", u->gid);

		if (vis.gecos) {
			csv_print_quoted(u->gecos, strlen(u->gecos));
			cprint(&ctx, "");
		}

		if (vis.dir) {
			csv_print_quoted(u->dir, strlen(u->dir));
			cprint(&ctx, "");
		}

		if (vis.shell) {
			csv_print_quoted(u->shell, strlen(u->shell));
			cprint(&ctx, "");
		}

		if (vis.group_ids) {
			if (u->ngroups) {
				if (u->ngroups > 1)
					putc('"', stdout);

				for (size_t j = 0; j < u->ngroups - 1; ++j)
					printf("%u,", u->groups[j]->gid);

				printf("%u", u->groups[u->ngroups - 1]->gid);

				if (u->ngroups > 1)
					putc('"', stdout);
			}

			cprint(&ctx, "");
		}

		if (vis.group_names) {
			if (u->ngroups) {
				if (u->ngroups > 1)
					putc('"', stdout);

				for (size_t j = 0; j < u->ngroups - 1; ++j)
					printf("%s,", u->groups[j]->name);

				printf("%s", u->groups[u->ngroups - 1]->name);

				if (u->ngroups > 1)
					putc('"', stdout);
			}

			cprint(&ctx, "");
		}
	}

	free_users();
	free_groups();

	return 0;
}
