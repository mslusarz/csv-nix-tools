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

/*
 * required to get:
 *  O_PATH
 *  hcreate_r
 *  hdestroy_t
 *  hsearch_r
 *  asprintf
 *  scandirat
 */
#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <search.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

struct ht {
	size_t keys_max;
	struct hsearch_data ht;

	ENTRY *table;
	size_t table_size;
	size_t inserted;
};

struct visible_columns {
	struct {
		char size;
		char type;
		char mode;
		char owner_id;
		char group_id;
		char nlink;
		char mtime_sec;
		char mtime_nsec;
		char ctime_sec;
		char ctime_nsec;
		char atime_sec;
		char atime_nsec;
		char dev;
		char ino;
		char rdev;
		char blksize;
		char blocks;

		char symlink;
		char parent;
		char name;
	} base;

	struct {
		char type_name;
		char owner_name;
		char group_name;
		char owner_read;
		char owner_write;
		char owner_execute;
		char group_read;
		char group_write;
		char group_execute;
		char other_read;
		char other_write;
		char other_execute;
		char setuid;
		char setgid;
		char sticky;
		char mtime;
		char ctime;
		char atime;
		char full_path;
	} ext;
};

struct visibility_info {
	struct visible_columns cols;
	size_t count;
};

static struct ht users_ht;
static struct ht groups_ht;

static int
ht_init(struct ht *ht)
{
	memset(ht, 0, sizeof(*ht));
	ht->keys_max = 4;
	ht->table_size = 4;

	ht->table = malloc(ht->table_size * sizeof(ht->table[0]));
	if (!ht->table) {
		perror("malloc");
		return 2;
	}

	if (hcreate_r(ht->keys_max, &ht->ht) == 0) {
		perror("hcreate_r");
		free(ht->table);
		ht->table = NULL;
		return 2;
	}

	return 0;
}

static void
ht_destroy(struct ht *ht)
{
	hdestroy_r(&ht->ht);
	for (size_t u = 0; u < ht->inserted; ++u) {
		free(ht->table[u].key);
		free(ht->table[u].data);
	}
	free(ht->table);
}

static const char *
get_value(struct ht *ht, char *key, void *(*cb)(void *), void *cb_data)
{
	ENTRY e;
	ENTRY *entry = NULL;

	e.key = key;
	e.data = NULL;

	if (hsearch_r(e, ENTER, &entry, &ht->ht) == 0) {
		/* hash map too small, recreate it */
		hdestroy_r(&ht->ht);
		memset(&ht->ht, 0, sizeof(ht->ht));
		ht->keys_max *= 2;

		if (hcreate_r(ht->keys_max, &ht->ht) == 0) {
			perror("hcreate_r");
			exit(2); /* no sane way to handle */
		}

		for (size_t u = 0; u < ht->inserted; ++u) {
			e.key = ht->table[u].key;
			e.data = NULL;

			if (hsearch_r(e, ENTER, &entry, &ht->ht) == 0)
				exit(2); /* impossible */

			entry->key = ht->table[u].key;
			entry->data = ht->table[u].data;
		}

		return get_value(ht, key, cb, cb_data);
	}

	if (entry->data != NULL)
		return entry->data;

	if (ht->inserted == ht->table_size) {
		ht->table_size *= 2;
		ht->table = realloc(ht->table,
				ht->table_size * sizeof(ht->table[0]));
		if (!ht->table) {
			perror("realloc");
			exit(2); /* no sane way to handle */
		}
	}

	entry->key = strdup(key);
	if (!entry->key) {
		perror("strdup");
		exit(2); /* no sane way to handle */
	}

	entry->data = cb(cb_data);
	if (!entry->data)
		exit(2); /* no sane way to handle */

	ht->table[ht->inserted].key = entry->key;
	ht->table[ht->inserted].data = entry->data;
	ht->inserted++;

	return entry->data;
}

static void *
get_user_slow(void *uidp)
{
	char *ret;
	uid_t uid = *(uid_t *)uidp;
	struct passwd *passwd = getpwuid(uid);
	if (passwd) {
		ret = strdup(passwd->pw_name);
		if (!ret)
			perror("strdup");

		return ret;
	}

	if (asprintf(&ret, "%d", uid) < 0) {
		perror("asprintf");
		ret = NULL;
	}

	return ret;
}

static const char *
get_user(uid_t uid)
{
	char key[30];
	sprintf(key, "%d", uid);

	return get_value(&users_ht, key, get_user_slow, &uid);
}

static void *
get_group_slow(void *gidp)
{
	char *ret;
	gid_t gid = *(uid_t *)gidp;
	struct group *gr = getgrgid(gid);
	if (gr) {
		ret = strdup(gr->gr_name);
		if (!ret)
			perror("strdup");

		return ret;
	}

	if (asprintf(&ret, "%d", gid) < 0) {
		perror("asprintf");
		ret = NULL;
	}

	return ret;
}

static const char *
get_group(gid_t gid)
{
	char key[30];
	sprintf(key, "%d", gid);

	return get_value(&groups_ht, key, get_group_slow, &gid);
}

static const char *
get_file_type_long(mode_t m)
{
	if (S_ISREG(m))
		return "reg";
	if (S_ISDIR(m))
		return "dir";
	if (S_ISLNK(m))
		return "slnk";
	if (S_ISFIFO(m))
		return "fifo";
	if (S_ISSOCK(m))
		return "sock";
	if (S_ISCHR(m))
		return "chr";
	if (S_ISBLK(m))
		return "blk";

	fprintf(stderr, "unknown file type 0%o\n", m);
	abort();
}

static void
print_timespec(struct timespec *ts)
{
	int ret;
	struct tm t;

	if (localtime_r(&ts->tv_sec, &t) == NULL)
		goto fallback;

	char buf[50];
	ret = strftime(buf, 30, "%F %T", &t);
	if (ret == 0)
		goto fallback;

	printf("%s.%09ld", buf, ts->tv_nsec);
	return;
fallback:
	printf("%lu.%09lu", ts->tv_sec, ts->tv_nsec);
}

struct stat_ctx {
	size_t printed;
	const struct visibility_info *visinfo;
};

static void
stat_printf(struct stat_ctx *ctx, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);

	if (++ctx->printed < ctx->visinfo->count)
		fputc(',', stdout);
}

static void
print_stat(const char *dirpath, const char *path, struct stat *st,
		const char *symlink, const struct visibility_info *visinfo)
{
	struct stat_ctx ctx = {0, visinfo};

	if (visinfo->cols.base.size)
		stat_printf(&ctx, "%ld", st->st_size);
	if (visinfo->cols.base.type)
		stat_printf(&ctx, "0%o", st->st_mode & S_IFMT);
	if (visinfo->cols.base.mode)
		stat_printf(&ctx, "0%o", st->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX));
	if (visinfo->cols.base.owner_id)
		stat_printf(&ctx, "%d", st->st_uid);
	if (visinfo->cols.base.group_id)
		stat_printf(&ctx, "%d", st->st_gid);
	if (visinfo->cols.base.nlink)
		stat_printf(&ctx, "%ld", st->st_nlink);
	if (visinfo->cols.base.mtime_sec)
		stat_printf(&ctx, "%ld", st->st_mtim.tv_sec);
	if (visinfo->cols.base.mtime_nsec)
		stat_printf(&ctx, "%ld", st->st_mtim.tv_nsec);
	if (visinfo->cols.base.ctime_sec)
		stat_printf(&ctx, "%ld", st->st_ctim.tv_sec);
	if (visinfo->cols.base.ctime_nsec)
		stat_printf(&ctx, "%ld", st->st_ctim.tv_nsec);
	if (visinfo->cols.base.atime_sec)
		stat_printf(&ctx, "%ld", st->st_atim.tv_sec);
	if (visinfo->cols.base.atime_nsec)
		stat_printf(&ctx, "%ld", st->st_atim.tv_nsec);
	if (visinfo->cols.base.dev)
		stat_printf(&ctx, "0x%lx", st->st_dev);
	if (visinfo->cols.base.ino)
		stat_printf(&ctx, "%ld", st->st_ino);
	if (visinfo->cols.base.rdev) {
		if (st->st_rdev)
			stat_printf(&ctx, "0x%lx", st->st_rdev);
		else
			stat_printf(&ctx, "0");
	}
	if (visinfo->cols.base.blksize)
		stat_printf(&ctx, "%ld", st->st_blksize);
	if (visinfo->cols.base.blocks)
		stat_printf(&ctx, "%ld", st->st_blocks);

	if (visinfo->cols.ext.type_name)
		stat_printf(&ctx, "%s", get_file_type_long(st->st_mode));
	if (visinfo->cols.ext.owner_name)
		stat_printf(&ctx, "%s", get_user(st->st_uid));
	if (visinfo->cols.ext.group_name)
		stat_printf(&ctx, "%s", get_group(st->st_uid));
	if (visinfo->cols.ext.owner_read)
		stat_printf(&ctx, "%s", (st->st_mode & S_IRUSR) ? "1" : "0");
	if (visinfo->cols.ext.owner_write)
		stat_printf(&ctx, "%s", (st->st_mode & S_IWUSR) ? "1" : "0");
	if (visinfo->cols.ext.owner_execute)
		stat_printf(&ctx, "%s", (st->st_mode & S_IXUSR) ? "1" : "0");
	if (visinfo->cols.ext.group_read)
		stat_printf(&ctx, "%s", (st->st_mode & S_IRGRP) ? "1" : "0");
	if (visinfo->cols.ext.group_write)
		stat_printf(&ctx, "%s", (st->st_mode & S_IWGRP) ? "1" : "0");
	if (visinfo->cols.ext.group_execute)
		stat_printf(&ctx, "%s", (st->st_mode & S_IXGRP) ? "1" : "0");
	if (visinfo->cols.ext.other_read)
		stat_printf(&ctx, "%s", (st->st_mode & S_IROTH) ? "1" : "0");
	if (visinfo->cols.ext.other_write)
		stat_printf(&ctx, "%s", (st->st_mode & S_IWOTH) ? "1" : "0");
	if (visinfo->cols.ext.other_execute)
		stat_printf(&ctx, "%s", (st->st_mode & S_IXOTH) ? "1" : "0");
	if (visinfo->cols.ext.setuid)
		stat_printf(&ctx, "%s", (st->st_mode & S_ISUID) ? "1" : "0");
	if (visinfo->cols.ext.setgid)
		stat_printf(&ctx, "%s", (st->st_mode & S_ISGID) ? "1" : "0");
	if (visinfo->cols.ext.sticky)
		stat_printf(&ctx, "%s", (st->st_mode & S_ISVTX) ? "1" : "0");

	if (visinfo->cols.ext.mtime) {
		print_timespec(&st->st_mtim);
		stat_printf(&ctx, "");
	}

	if (visinfo->cols.ext.ctime) {
		print_timespec(&st->st_ctim);
		stat_printf(&ctx, "");
	}

	if (visinfo->cols.ext.atime) {
		print_timespec(&st->st_atim);
		stat_printf(&ctx, "");
	}

	if (visinfo->cols.base.symlink) {
		if (S_ISLNK(st->st_mode) && symlink)
			csv_print_quoted(symlink, strlen(symlink));
		stat_printf(&ctx, "");
	}

	if (visinfo->cols.base.parent) {
		if (dirpath)
			csv_print_quoted(dirpath, strlen(dirpath));
		stat_printf(&ctx, "");
	}

	if (visinfo->cols.base.name) {
		csv_print_quoted(path, strlen(path));
		stat_printf(&ctx, "");
	}

	if (visinfo->cols.ext.full_path) {
		if (dirpath) {
			size_t dirpath_len = strlen(dirpath);
			size_t path_len = strlen(path);

			if (csv_requires_quoting(dirpath, dirpath_len) ||
					csv_requires_quoting(path, path_len)) {
				size_t len = dirpath_len + 1 + path_len;
				char *buf = malloc(len + 1);
				sprintf(buf, "%s/%s", dirpath, path);
				csv_print_quoted(buf, len);
				free(buf);
			} else {
				fwrite(dirpath, 1, dirpath_len, stdout);
				fputc('/', stdout);
				fwrite(path, 1, path_len, stdout);
			}
		} else {
			csv_print_quoted(path, strlen(path));
		}
		stat_printf(&ctx, "");
	}

	fputc('\n', stdout);
}

static int
alphasort_caseinsensitive(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

static int
list(const char *dirpath, int dirfd, int recursive, int all, int sort,
		const struct visibility_info *visinfo)
{
	int ret = 0;
	struct dirent **namelist;
	const char **dirs;
	int numdirs = 0;
	char *path;
	size_t pos = strlen(dirpath);

	if (pos < 1) {
		fprintf(stderr, "empty path\n");
		return 2;
	}

	int (*compar)(const struct dirent **, const struct dirent **);
	if (sort)
		compar = alphasort_caseinsensitive;
	else
		compar = NULL;

	int entries = scandirat(dirfd, ".", &namelist, NULL, compar);
	if (entries < 0) {
		fprintf(stderr, "listing directory '%s' failed: %s\n", dirpath,
				strerror(errno));
		return 1;
	}

	dirs = malloc(entries * sizeof(dirs[0]));
	if (!dirs) {
		perror("malloc");
		ret = 2;
		goto dirs_alloc_fail;
	}

	path = malloc(pos + 1 + sizeof(namelist[0]->d_name));
	if (!path) {
		perror("malloc");
		ret = 2;
		goto path_alloc_fail;
	}

	strcpy(path, dirpath);
	if (path[pos - 1] != '/')
		path[pos++] = '/';

	for (int i = 0; i < entries; ++i) {
		struct stat statbuf;
		/* skip hidden files */
		if (!all && namelist[i]->d_name[0] == '.')
			continue;

		if (fstatat(dirfd, namelist[i]->d_name, &statbuf,
				AT_SYMLINK_NOFOLLOW)) {
			if (errno == EACCES || errno == ENOENT)
				ret |= 1;
			else
				ret |= 2;
			continue;
		}

		char *symlink = NULL;
		if (S_ISLNK(statbuf.st_mode)) do {
			size_t bufsiz;
			if (statbuf.st_size)
				bufsiz = statbuf.st_size + 1;
			else
				bufsiz = PATH_MAX;
restart_readlink:
			symlink = malloc(bufsiz);
			if (!symlink) {
				perror("malloc");
				ret |= 2;
				break;
			}
			ssize_t symlink_len = readlinkat(dirfd,
					namelist[i]->d_name, symlink,
					bufsiz);
			if (symlink_len < 0) {
				fprintf(stderr,
					"reading symlink '%s/%s' failed: %s\n",
					dirpath, namelist[i]->d_name,
					strerror(errno));
				free(symlink);
				symlink = NULL;
				ret |= 1;
			} else if (symlink_len == bufsiz) {
				free(symlink);
				bufsiz *= 2;
				goto restart_readlink;
			} else {
				symlink[symlink_len] = 0;
			}
		} while (0);

		print_stat(dirpath, namelist[i]->d_name, &statbuf, symlink,
				visinfo);

		if (S_ISLNK(statbuf.st_mode))
			free(symlink);

		if (!recursive)
			continue;
		if (!S_ISDIR(statbuf.st_mode))
			continue;
		if (strcmp(namelist[i]->d_name, ".") == 0)
			continue;
		if (strcmp(namelist[i]->d_name, "..") == 0)
			continue;

		dirs[numdirs++] = namelist[i]->d_name;
	}


	for (int j = 0; j < numdirs; ++j) {
		int fd = openat(dirfd, dirs[j], O_PATH | O_DIRECTORY);
		if (fd < 0) {
			fprintf(stderr, "open '%s/%s' failed: %s\n", dirpath,
					dirs[j], strerror(errno));
			ret |= 1;
			continue;
		}

		strcpy(path + pos, dirs[j]);

		ret |= list(path, fd, recursive, all, sort, visinfo);

		if (close(fd)) {
			perror("close");
			ret = 2;
			break;
		}
	}

	free(path);
path_alloc_fail:
	free(dirs);
dirs_alloc_fail:
	for (int i = 0; i < entries; ++i)
		free(namelist[i]);
	free(namelist);

	return ret;
}

static const struct option long_options[] = {
	{"all",		no_argument, 		NULL, 'a'},
	{"directory",	no_argument,		NULL, 'd'},
	{"fields",	required_argument,	NULL, 'f'},
	{"no-header",	no_argument,		NULL, 'H'},
	{"recursive",	no_argument,		NULL, 'R'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: csv-ls [OPTION]... [FILE]...\n");
	printf("Options:\n");
	printf("  -a, --all\n");
	printf("  -d, --directory\n");
	printf("  -f, --fields=name1[,name2...]\n");
	printf("  -l\n");
	printf("  -R, --recursive\n");
	printf("  -s, --show\n");
	printf("  -U\n");
	printf("      --no-header\n");
	printf("      --help\n");
	printf("      --version\n");
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
	int opt, ret = 0;
	int dir = 0, recursive = 0, all = 0, sort = 1;
	int longindex;
	char *cols = NULL;
	struct visible_columns vis;
	bool print_header = true;
	bool show = false;

	memset(&vis, 0, sizeof(vis));
	memset(&vis.base, 1, sizeof(vis.base));

	while ((opt = getopt_long(argc, argv, "adf:lRsU", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'a':
				all = 1;
				break;
			case 'd':
				dir = 1;
				break;
			case 'f':
				cols = strdup(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 'l':
				memset(&vis.ext, 1, sizeof(vis.ext));
				break;
			case 'R':
				recursive = 1;
				break;
			case 's':
				show = true;
				break;
			case 'U':
				sort = 0;
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

	tzset();

	if (ht_init(&users_ht))
		return 2;

	if (ht_init(&groups_ht)) {
		ht_destroy(&users_ht);
		return 2;
	}

	if (optind == argc) {
		argv[optind] = ".";
		argc++;
	}

	if (cols) {
		memset(&vis, 0, sizeof(vis));

		const struct {
			const char *name;
			char *vis;
		} map[] = {
				{ "size", &vis.base.size },
				{ "type", &vis.base.type },
				{ "mode", &vis.base.mode },
				{ "owner_id", &vis.base.owner_id },
				{ "group_id", &vis.base.group_id },
				{ "nlink", &vis.base.nlink },
				{ "mtime_sec", &vis.base.mtime_sec },
				{ "mtime_nsec", &vis.base.mtime_nsec },
				{ "ctime_sec", &vis.base.ctime_sec },
				{ "ctime_nsec", &vis.base.ctime_nsec },
				{ "atime_sec", &vis.base.atime_sec },
				{ "atime_nsec", &vis.base.atime_nsec },
				{ "dev", &vis.base.dev },
				{ "ino", &vis.base.ino },
				{ "rdev", &vis.base.rdev },
				{ "blksize", &vis.base.blksize },
				{ "blocks", &vis.base.blocks },

				{ "symlink", &vis.base.symlink },
				{ "parent", &vis.base.parent },
				{ "name", &vis.base.name },

				{ "type_name", &vis.ext.type_name },
				{ "owner_name", &vis.ext.owner_name },
				{ "group_name", &vis.ext.group_name },
				{ "owner_read", &vis.ext.owner_read },
				{ "owner_write", &vis.ext.owner_write },
				{ "owner_execute", &vis.ext.owner_execute },
				{ "group_read", &vis.ext.group_read },
				{ "group_write", &vis.ext.group_write },
				{ "group_execute", &vis.ext.group_execute },
				{ "other_read", &vis.ext.other_read },
				{ "other_write", &vis.ext.other_write },
				{ "other_execute", &vis.ext.other_execute },
				{ "setuid", &vis.ext.setuid },
				{ "setgid", &vis.ext.setgid },
				{ "sticky", &vis.ext.sticky },
				{ "mtime", &vis.ext.mtime },
				{ "ctime", &vis.ext.ctime },
				{ "atime", &vis.ext.atime },
				{ "full_path", &vis.ext.full_path },
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
		eval_col(vis.base.size, "size:int", print, &visible, count);
		eval_col(vis.base.type, "type:int", print, &visible, count);
		eval_col(vis.base.mode, "mode:int", print, &visible, count);
		eval_col(vis.base.owner_id, "owner_id:int", print, &visible, count);
		eval_col(vis.base.group_id, "group_id:int", print, &visible, count);
		eval_col(vis.base.nlink, "nlink:int", print, &visible, count);
		eval_col(vis.base.mtime_sec, "mtime_sec:int", print, &visible, count);
		eval_col(vis.base.mtime_nsec, "mtime_nsec:int", print, &visible, count);
		eval_col(vis.base.ctime_sec, "ctime_sec:int", print, &visible, count);
		eval_col(vis.base.ctime_nsec, "ctime_nsec:int", print, &visible, count);
		eval_col(vis.base.atime_sec, "atime_sec:int", print, &visible, count);
		eval_col(vis.base.atime_nsec, "atime_nsec:int", print, &visible, count);
		eval_col(vis.base.dev, "dev:int", print, &visible, count);
		eval_col(vis.base.ino, "ino:int", print, &visible, count);
		eval_col(vis.base.rdev, "rdev:int", print, &visible, count);
		eval_col(vis.base.blksize, "blksize:int", print, &visible, count);
		eval_col(vis.base.blocks, "blocks:int", print, &visible, count);

		eval_col(vis.ext.type_name, "type_name:string", print, &visible, count);
		eval_col(vis.ext.owner_name, "owner_name:string", print, &visible, count);
		eval_col(vis.ext.group_name, "group_name:string", print, &visible, count);
		eval_col(vis.ext.owner_read, "owner_read:bool", print, &visible, count);
		eval_col(vis.ext.owner_write, "owner_write:bool", print, &visible, count);
		eval_col(vis.ext.owner_execute, "owner_execute:bool", print, &visible, count);
		eval_col(vis.ext.group_read, "group_read:bool", print, &visible, count);
		eval_col(vis.ext.group_write, "group_write:bool", print, &visible, count);
		eval_col(vis.ext.group_execute, "group_execute:bool", print, &visible, count);
		eval_col(vis.ext.other_read, "other_read:bool", print, &visible, count);
		eval_col(vis.ext.other_write, "other_write:bool", print, &visible, count);
		eval_col(vis.ext.other_execute, "other_execute:bool", print, &visible, count);
		eval_col(vis.ext.setuid, "setuid:bool", print, &visible, count);
		eval_col(vis.ext.setgid, "setgid:bool", print, &visible, count);
		eval_col(vis.ext.sticky, "sticky:bool", print, &visible, count);
		eval_col(vis.ext.mtime, "mtime:string", print, &visible, count);
		eval_col(vis.ext.ctime, "ctime:string", print, &visible, count);
		eval_col(vis.ext.atime, "atime:string", print, &visible, count);

		eval_col(vis.base.symlink, "symlink:string", print, &visible, count);
		eval_col(vis.base.parent, "parent:string", print, &visible, count);
		eval_col(vis.base.name, "name:string", print, &visible, count);
		eval_col(vis.ext.full_path, "full_path:string", print, &visible, count);
		count = visible;
		visible = 0;
		if (print_header)
			print++;
	} while (print == 1);

	if (print_header)
		printf("\n");

	struct visibility_info visinfo = {vis, count};

	for (int i = optind; i < argc; ++i) {
		int fd = openat(AT_FDCWD, argv[i], O_PATH | O_NOFOLLOW);
		if (fd < 0) {
			fprintf(stderr, "open '%s' failed: %s\n", argv[i],
					strerror(errno));
			ret |= 2;
			continue;
		}

		struct stat buf;
		int r = fstat(fd, &buf);
		if (r < 0) {
			fprintf(stderr, "fstat '%s' failed: %s\n", argv[i],
					strerror(errno));
			ret |= 2;
			if (close(fd))
				perror("close");
			continue;
		}

		if (S_ISDIR(buf.st_mode) && !dir) {
			ret |= list(argv[i], fd, recursive, all, sort, &visinfo);
		} else {
			char *symlink = NULL;
			if (S_ISLNK(buf.st_mode)) do {
				size_t bufsiz;
				if (buf.st_size)
					bufsiz = buf.st_size + 1;
				else
					bufsiz = PATH_MAX;

restart_readlink:
				symlink = malloc(bufsiz);
				if (!symlink) {
					perror("malloc");
					ret |= 2;
					break;
				}
				ssize_t symlink_len = readlinkat(fd,
						"", symlink, bufsiz);
				if (symlink_len < 0) {
					fprintf(stderr,
						"reading symlink '%s' failed: %s\n",
						argv[i], strerror(errno));

					free(symlink);
					symlink = NULL;
					ret |= 2;
				} else if (symlink_len == bufsiz) {
					free(symlink);
					bufsiz *= 2;
					goto restart_readlink;
				} else {
					symlink[symlink_len] = 0;
				}
			} while (0);

			print_stat(NULL, argv[i], &buf, symlink, &visinfo);
		}

		if (close(fd)) {
			perror("close");
			ret |= 2;
			break;
		}
	}

	ht_destroy(&users_ht);
	ht_destroy(&groups_ht);

	if (ret & 2)
		return 2;
	if (ret & 1)
		return 1;

	return 0;
}
