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
 *  scandirat
 */
#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "usr-grp-query.h"
#include "utils.h"

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

struct file_info {
	struct stat *st;
	const char *path;
	const char *dirpath;
	const char *symlink;
};

static void
print_size(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_size);
}

static void
print_type(const void *p)
{
	const struct file_info *f = p;
	mode_t v = f->st->st_mode & S_IFMT;
	if (v)
		printf("0%o", v);
	else
		printf("0");
}

static void
print_mode(const void *p)
{
	const struct file_info *f = p;
	mode_t v = f->st->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX);
	if (v)
		printf("0%o", v);
	else
		printf("0");
}

static void
print_owner_id(const void *p)
{
	const struct file_info *f = p;
	printf("%d", f->st->st_uid);
}

static void
print_group_id(const void *p)
{
	const struct file_info *f = p;
	printf("%d", f->st->st_gid);
}

static void
print_nlink(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_nlink);
}

static void
print_mtime_sec(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_mtim.tv_sec);
}

static void
print_mtime_nsec(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_mtim.tv_nsec);
}

static void
print_ctime_sec(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_ctim.tv_sec);
}

static void
print_ctime_nsec(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_ctim.tv_nsec);
}

static void
print_atime_sec(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_atim.tv_sec);
}

static void
print_atime_nsec(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_atim.tv_nsec);
}

static void
print_dev(const void *p)
{
	const struct file_info *f = p;
	printf("0x%lx", f->st->st_dev);
}

static void
print_ino(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_ino);
}

static void
print_rdev(const void *p)
{
	const struct file_info *f = p;
	if (f->st->st_rdev)
		printf("0x%lx", f->st->st_rdev);
	else
		printf("0");
}

static void
print_blksize(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_blksize);
}

static void
print_blocks(const void *p)
{
	const struct file_info *f = p;
	printf("%ld", f->st->st_blocks);
}

static void
print_type_name(const void *p)
{
	const struct file_info *f = p;
	printf("%s", get_file_type_long(f->st->st_mode));
}

static void
print_owner_name(const void *p)
{
	const struct file_info *f = p;
	const char *user = get_user(f->st->st_uid);
	csv_print_quoted(user, strlen(user));
}

static void
print_group_name(const void *p)
{
	const struct file_info *f = p;
	const char *group = get_group(f->st->st_gid);
	csv_print_quoted(group, strlen(group));
}

static void
print_owner_read(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IRUSR) ? "1" : "0");
}

static void
print_owner_write(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IWUSR) ? "1" : "0");
}

static void
print_owner_execute(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IXUSR) ? "1" : "0");
}

static void
print_group_read(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IRGRP) ? "1" : "0");
}

static void
print_group_write(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IWGRP) ? "1" : "0");
}

static void
print_group_execute(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IXGRP) ? "1" : "0");
}

static void
print_other_read(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IROTH) ? "1" : "0");
}

static void
print_other_write(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IWOTH) ? "1" : "0");
}

static void
print_other_execute(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_IXOTH) ? "1" : "0");
}

static void
print_setuid(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_ISUID) ? "1" : "0");
}

static void
print_setgid(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_ISGID) ? "1" : "0");
}

static void
print_sticky(const void *p)
{
	const struct file_info *f = p;
	printf("%s", (f->st->st_mode & S_ISVTX) ? "1" : "0");
}

static void
print_mtime(const void *p)
{
	const struct file_info *f = p;
	print_timespec(&f->st->st_mtim, true);
}

static void
print_ctime(const void *p)
{
	const struct file_info *f = p;
	print_timespec(&f->st->st_ctim, true);
}

static void
print_atime(const void *p)
{
	const struct file_info *f = p;
	print_timespec(&f->st->st_atim, true);
}

static void
print_symlink(const void *p)
{
	const struct file_info *f = p;
	if (S_ISLNK(f->st->st_mode) && f->symlink)
		csv_print_quoted(f->symlink, strlen(f->symlink));
}

static void
print_parent(const void *p)
{
	const struct file_info *f = p;
	if (f->dirpath)
		csv_print_quoted(f->dirpath, strlen(f->dirpath));
}

static void
print_name(const void *p)
{
	const struct file_info *f = p;
	csv_print_quoted(f->path, strlen(f->path));
}

static void
print_full_path(const void *p)
{
	const struct file_info *f = p;
	if (f->dirpath) {
		size_t dirpath_len = strlen(f->dirpath);
		size_t path_len = strlen(f->path);

		if (csv_requires_quoting(f->dirpath, dirpath_len) ||
				csv_requires_quoting(f->path, path_len)) {
			size_t len = dirpath_len + 1 + path_len;
			char *buf = xmalloc_nofail(len + 1, 1);
			sprintf(buf, "%s/%s", f->dirpath, f->path);
			csv_print_quoted(buf, len);
			free(buf);
		} else {
			fwrite(f->dirpath, 1, dirpath_len, stdout);
			fputc('/', stdout);
			fwrite(f->path, 1, path_len, stdout);
		}
	} else {
		csv_print_quoted(f->path, strlen(f->path));
	}
}

static int
alphasort_caseinsensitive(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

static int
list(const char *dirpath, int dirfd, int recursive, int all, int sort,
		const struct column_info *columns, size_t ncolumns)
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

	dirs = xmalloc(entries, sizeof(dirs[0]));
	if (!dirs) {
		ret = 2;
		goto dirs_alloc_fail;
	}

	path = xmalloc(pos + 1 + sizeof(namelist[0]->d_name), 1);
	if (!path) {
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
			symlink = xmalloc(bufsiz, 1);
			if (!symlink) {
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

		struct file_info info = {
				&statbuf,
				namelist[i]->d_name,
				dirpath,
				symlink
		};
		csvci_print_row(&info, columns, ncolumns);

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

		ret |= list(path, fd, recursive, all, sort, columns, ncolumns);

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

static const struct option opts[] = {
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
usage(FILE *out)
{
	fprintf(out, "Usage: csv-ls [OPTION]... [FILE]...\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -a, --all\n");
	fprintf(out, "  -d, --directory\n");
	fprintf(out, "  -f, --fields=name1[,name2...]\n");
	fprintf(out, "  -l\n");
	fprintf(out, "  -R, --recursive\n");
	fprintf(out, "  -s, --show\n");
	fprintf(out, "  -U\n");
	fprintf(out, "      --no-header\n");
	fprintf(out, "      --help\n");
	fprintf(out, "      --version\n");
}

int
main(int argc, char *argv[])
{
	int opt, ret = 0;
	int dir = 0, recursive = 0, all = 0, sort = 1;
	char *cols = NULL;
	bool print_header = true;
	bool show = false;

	struct column_info columns[] = {
		{ true, 0, "size",           TYPE_INT,    print_size },
		{ true, 0, "type",           TYPE_INT,    print_type },
		{ true, 0, "mode",           TYPE_INT,    print_mode },
		{ true, 0, "owner_id",       TYPE_INT,    print_owner_id },
		{ true, 0, "group_id",       TYPE_INT,    print_group_id },
		{ true, 0, "nlink",          TYPE_INT,    print_nlink },
		{ true, 0, "mtime_sec",      TYPE_INT,    print_mtime_sec },
		{ true, 0, "mtime_nsec",     TYPE_INT,    print_mtime_nsec },
		{ true, 0, "ctime_sec",      TYPE_INT,    print_ctime_sec },
		{ true, 0, "ctime_nsec",     TYPE_INT,    print_ctime_nsec },
		{ true, 0, "atime_sec",      TYPE_INT,    print_atime_sec },
		{ true, 0, "atime_nsec",     TYPE_INT,    print_atime_nsec },
		{ true, 0, "dev",            TYPE_INT,    print_dev },
		{ true, 0, "ino",            TYPE_INT,    print_ino },
		{ true, 0, "rdev",           TYPE_INT,    print_rdev },
		{ true, 0, "blksize",        TYPE_INT,    print_blksize },
		{ true, 0, "blocks",         TYPE_INT,    print_blocks },

		{ false, 0, "type_name",     TYPE_STRING, print_type_name },
		{ false, 0, "owner_name",    TYPE_STRING, print_owner_name },
		{ false, 0, "group_name",    TYPE_STRING, print_group_name },
		{ false, 0, "owner_read",    TYPE_INT,    print_owner_read },
		{ false, 0, "owner_write",   TYPE_INT,    print_owner_write },
		{ false, 0, "owner_execute", TYPE_INT,    print_owner_execute },
		{ false, 0, "group_read",    TYPE_INT,    print_group_read },
		{ false, 0, "group_write",   TYPE_INT,    print_group_write },
		{ false, 0, "group_execute", TYPE_INT,    print_group_execute },
		{ false, 0, "other_read",    TYPE_INT,    print_other_read },
		{ false, 0, "other_write",   TYPE_INT,    print_other_write },
		{ false, 0, "other_execute", TYPE_INT,    print_other_execute },
		{ false, 0, "setuid",        TYPE_INT,    print_setuid },
		{ false, 0, "setgid",        TYPE_INT,    print_setgid },
		{ false, 0, "sticky",        TYPE_INT,    print_sticky },
		{ false, 0, "mtime",         TYPE_STRING, print_mtime },
		{ false, 0, "ctime",         TYPE_STRING, print_ctime },
		{ false, 0, "atime",         TYPE_STRING, print_atime },

		{ true, 0, "symlink",        TYPE_STRING, print_symlink },
		{ true, 0, "parent",         TYPE_STRING, print_parent },
		{ true, 0, "name",           TYPE_STRING, print_name },
		{ false, 0, "full_path",     TYPE_STRING, print_full_path },
	};

	size_t ncolumns = ARRAY_SIZE(columns);

	while ((opt = getopt_long(argc, argv, "adf:lRsU", opts, NULL)) != -1) {
		switch (opt) {
			case 'a':
				all = 1;
				break;
			case 'd':
				dir = 1;
				break;
			case 'f':
				cols = xstrdup_nofail(optarg);
				break;
			case 'H':
				print_header = false;
				break;
			case 'l':
				for (size_t i = 0; i < ncolumns; ++i)
					columns[i].vis = true;
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
			case 'h':
			default:
				usage(stdout);
				return 2;
		}
	}

	tzset();

	ret = usr_grp_query_init();
	if (ret)
		return ret;

	if (optind == argc) {
		argv[optind] = ".";
		argc++;
	}

	if (cols) {
		int r = csvci_parse_cols(cols, columns, &ncolumns);

		free(cols);

		if (r)
			exit(2);
	} else {
		csvci_set_columns_order(columns, &ncolumns);
	}

	if (show)
		csv_show();

	if (print_header)
		csvci_print_header(columns, ncolumns);

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
			ret |= list(argv[i], fd, recursive, all, sort, columns,
					ncolumns);
		} else {
			char *symlink = NULL;
			if (S_ISLNK(buf.st_mode)) do {
				size_t bufsiz;
				if (buf.st_size)
					bufsiz = buf.st_size + 1;
				else
					bufsiz = PATH_MAX;

restart_readlink:
				symlink = xmalloc(bufsiz, 1);
				if (!symlink) {
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

			struct file_info info = {
					&buf,
					argv[i],
					NULL,
					symlink
			};
			csvci_print_row(&info, columns, ncolumns);
		}

		if (close(fd)) {
			perror("close");
			ret |= 2;
			break;
		}
	}

	usr_grp_query_fini();

	if (ret & 2)
		return 2;
	if (ret & 1)
		return 1;

	return 0;
}
