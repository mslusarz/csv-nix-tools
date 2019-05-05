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

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <search.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct ht {
	size_t keys_max;
	struct hsearch_data ht;

	ENTRY *table;
	size_t table_size;
	size_t inserted;
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

static void
print_quoted(const char *str)
{
	const char *comma = strchr(str, ',');
	const char *nl = strchr(str, '\n');
	const char *quot = strchr(str, '"');
	if (!comma && !nl && !quot) {
		fputs(str, stdout);
		return;
	}
	fputc('"', stdout);
	if (!quot) {
		fputs(str, stdout);
		fputc('"', stdout);
		return;
	}

	do {
		size_t len = (uintptr_t)quot - (uintptr_t)str + 1;
		fwrite(str, 1, len, stdout);
		str += len;
		fputc('"', stdout);
		quot = strchr(str, '"');
	} while (quot);

	fputs(str, stdout);
	fputc('"', stdout);
}

static void
print_stat(const char *dirpath, const char *path, struct stat *st,
		const char *symlink, int long_format)
{
	printf("%ld,", st->st_size);
	printf("0%o,", st->st_mode & S_IFMT);
	printf("0%o,", st->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX));
	printf("%d,", st->st_uid);
	printf("%d,", st->st_gid);
	printf("%ld,", st->st_nlink);
	printf("%ld,%ld,", st->st_mtim.tv_sec, st->st_mtim.tv_nsec);
	printf("%ld,%ld,", st->st_ctim.tv_sec, st->st_ctim.tv_nsec);
	printf("%ld,%ld,", st->st_atim.tv_sec, st->st_atim.tv_nsec);
	printf("0x%lx,", st->st_dev);
	printf("%ld,", st->st_ino);
	if (st->st_rdev)
		printf("0x%lx,", st->st_rdev);
	else
		printf("0,");
	printf("%ld,", st->st_blksize);
	printf("%ld,", st->st_blocks);

	if (long_format) {
		printf("%s,", get_file_type_long(st->st_mode));
		printf("%s,", get_user(st->st_uid));
		printf("%s,", get_group(st->st_uid));
		printf("%s,", (st->st_mode & S_IRUSR) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IWUSR) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IXUSR) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IRGRP) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IWGRP) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IXGRP) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IROTH) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IWOTH) ? "1" : "0");
		printf("%s,", (st->st_mode & S_IXOTH) ? "1" : "0");
		printf("%s,", (st->st_mode & S_ISUID) ? "1" : "0");
		printf("%s,", (st->st_mode & S_ISGID) ? "1" : "0");
		printf("%s,", (st->st_mode & S_ISVTX) ? "1" : "0");

		print_timespec(&st->st_mtim);
		fputc(',', stdout);

		print_timespec(&st->st_ctim);
		fputc(',', stdout);

		print_timespec(&st->st_atim);
		fputc(',', stdout);
	}

	if (S_ISLNK(st->st_mode) && symlink)
		print_quoted(symlink);
	fputc(',', stdout);

	if (dirpath)
		print_quoted(dirpath);
	fputc(',', stdout);

	print_quoted(path);

	fputc('\n', stdout);
}

static int
alphasort_caseinsensitive(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

static int
list(const char *dirpath, int dirfd, int recursive, int all, int sort,
		int long_format)
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
				long_format);

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

		ret |= list(path, fd, recursive, all, sort, long_format);

		close(fd);
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
	{"recursive",	no_argument,		NULL, 'R'},
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
	printf("  -l\n");
	printf("  -R, --recursive\n");
	printf("  -U\n");
	printf("      --help\n");
	printf("      --version\n");
}

int
main(int argc, char *argv[])
{
	int opt, ret = 0;
	int dir = 0, long_format = 0, recursive = 0, all = 0, sort = 1;
	int longindex;

	while ((opt = getopt_long(argc, argv, "adlRU", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'a':
				all = 1;
				break;
			case 'd':
				dir = 1;
				break;
			case 'l':
				long_format = 1;
				break;
			case 'R':
				recursive = 1;
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

	printf("size|int,"
		"type|int,"
		"mode|int,"
		"owner_id|int,"
		"group_id|int,"
		"nlink|int,"
		"mtime_sec|int,"
		"mtime_nsec|int,"
		"ctime_sec|int,"
		"ctime_nsec|int,"
		"atime_sec|int,"
		"atime_nsec|int,"
		"dev|int,"
		"ino|int,"
		"rdev|int,"
		"blksize|int,"
		"blocks|int,");

	if (long_format) {
		printf("type|string,"
			"owner_name|string,"
			"group_name|string,"
			"owner_read|bool,"
			"owner_write|bool,"
			"owner_execute|bool,"
			"group_read|bool,"
			"group_write|bool,"
			"group_execute|bool,"
			"other_read|bool,"
			"other_write|bool,"
			"other_execute|bool,"
			"setuid|bool,"
			"setgid|bool,"
			"sticky|bool,"
			"mtime|string,"
			"ctime|string,"
			"atime|string,");
	}

	printf("symlink|string,"
		"parent|string,"
		"name|string\n");

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
			close(fd);
			continue;
		}

		if (S_ISDIR(buf.st_mode) && !dir) {
			ret |= list(argv[i], fd, recursive, all, sort,
					long_format);
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

			print_stat(NULL, argv[i], &buf, symlink, long_format);
		}

		close(fd);
	}

	ht_destroy(&users_ht);
	ht_destroy(&groups_ht);

	if (ret & 2)
		return 2;
	if (ret & 1)
		return 1;

	return 0;
}
