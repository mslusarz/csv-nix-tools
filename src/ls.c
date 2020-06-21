/*
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

#include <assert.h>
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

#include "merge_utils.h"
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

static char
get_file_type(mode_t m)
{
	if (S_ISREG(m))
		return '-';
	if (S_ISDIR(m))
		return 'd';
	if (S_ISLNK(m))
		return 'l';
	if (S_ISFIFO(m))
		return 'p';
	if (S_ISSOCK(m))
		return 's';
	if (S_ISCHR(m))
		return 'c';
	if (S_ISBLK(m))
		return 'b';

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

static inline char
get_ugx(bool xset, bool setid)
{
	if (xset) {
		if (setid)
			return 's';
		else
			return 'x';
	} else {
		if (setid)
			return 'S';
		else
			return '-';
	}
}

static inline char
get_ox(bool xset, bool sticky)
{
	if (xset) {
		if (sticky)
			return 't';
		else
			return 'x';
	} else {
		if (sticky)
			return 'T';
		else
			return '-';
	}
}

static void
print_type_mode(const void *p)
{
	const struct file_info *f = p;
	mode_t m = f->st->st_mode;
	bool suid = f->st->st_mode & S_ISUID;
	bool sgid = f->st->st_mode & S_ISGID;
	bool sticky = f->st->st_mode & S_ISVTX;

	printf("%c%c%c%c%c%c%c%c%c%c ", get_file_type(m),
			m & S_IRUSR ? 'r' : '-',
			m & S_IWUSR ? 'w' : '-',
			get_ugx(m & S_IXUSR, suid),
			m & S_IRGRP ? 'r' : '-',
			m & S_IWGRP ? 'w' : '-',
			get_ugx(m & S_IXGRP, sgid),
			m & S_IROTH ? 'r' : '-',
			m & S_IWOTH ? 'w' : '-',
			get_ox(m & S_IXOTH, sticky)
		);
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
		const struct column_info *columns, size_t ncolumns,
		struct csvmu_ctx *ctx)
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

	int scandirret = scandirat(dirfd, ".", &namelist, NULL, compar);
	if (scandirret < 0) {
		fprintf(stderr, "listing directory '%s' failed: %s\n", dirpath,
				strerror(errno));
		return 1;
	}
	size_t entries = (size_t)scandirret;

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

	for (size_t i = 0; i < entries; ++i) {
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
			assert(statbuf.st_size >= 0);
			if (statbuf.st_size)
				bufsiz = (size_t)statbuf.st_size + 1;
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
			} else if ((size_t)symlink_len == bufsiz) {
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
		csvmu_print_row(ctx, &info, columns, ncolumns);

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

		ret |= list(path, fd, recursive, all, sort, columns, ncolumns,
				ctx);

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
	for (size_t i = 0; i < entries; ++i)
		free(namelist[i]);
	free(namelist);

	return ret;
}

static const struct option opts[] = {
	{"all",			no_argument, 		NULL, 'a'},
	{"directory",		no_argument,		NULL, 'd'},
	{"columns",		required_argument,	NULL, 'c'},
	{"merge",		no_argument,		NULL, 'M'},
	{"table-name",		required_argument,	NULL, 'N'},
	{"recursive",		no_argument,		NULL, 'R'},
	{"show",		no_argument,		NULL, 's'},
	{"show-full",		no_argument,		NULL, 'S'},
	{"as-table",		no_argument,		NULL, 'T'},
	{"version",		no_argument,		NULL, 'V'},
	{"help",		no_argument,		NULL, 'h'},
	{NULL,			0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-ls [OPTION]... [FILE]...\n");
	fprintf(out,
"List information about the FILEs (the current directory by default).\n"
"Sort entries alphabetically if -U is not specified.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -a, --all                  do not ignore entries starting with .\n");
	describe_Columns(out);
	fprintf(out, "  -d, --directory            list directories themselves, not their contents\n");
	fprintf(out, "  -l                         use a longer listing format (can be used up to 3 times)\n");
	describe_Merge(out);
	describe_table_Name(out);
	fprintf(out, "  -R, --recursive            list subdirectories recursively\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_as_Table(out, "file");
	fprintf(out, "  -U                         do not sort; list entries in directory order\n");
	describe_help(out);
	describe_version(out);
}

int
main(int argc, char *argv[])
{
	int opt, ret = 0;
	int dir = 0, recursive = 0, all = 0, sort = 1;
	char *cols = NULL;
	bool show = false;
	bool show_full;
	bool merge = false;
	char *table = NULL;

	struct column_info columns[] = {
		{ false, 0, 1, "type_mode",     TYPE_STRING, print_type_mode, 0 },
		{ false, 0, 1, "nlink",         TYPE_INT,    print_nlink, 0 },
		{ false, 0, 1, "owner_name",    TYPE_STRING, print_owner_name, 0 },
		{ false, 0, 1, "group_name",    TYPE_STRING, print_group_name, 0 },
		{ false, 0, 1, "size",          TYPE_INT,    print_size, 0 },
		{ false, 0, 1, "mtime",         TYPE_STRING, print_mtime, 0 },
		{ true,  0, 0, "name",          TYPE_STRING, print_name, 0 },
		{ false, 0, 1, "symlink",       TYPE_STRING, print_symlink, 0 },
		{ false, 0, 2, "parent",        TYPE_STRING, print_parent, 0 },

		{ false, 0, 2, "type",          TYPE_INT,    print_type, 0 },
		{ false, 0, 2, "type_name",     TYPE_STRING, print_type_name, 0 },

		{ false, 0, 2, "mode",          TYPE_INT,    print_mode, 0 },

		{ false, 0, 2, "owner_read",    TYPE_INT,    print_owner_read, 0 },
		{ false, 0, 2, "owner_write",   TYPE_INT,    print_owner_write, 0 },
		{ false, 0, 2, "owner_execute", TYPE_INT,    print_owner_execute, 0 },

		{ false, 0, 2, "group_read",    TYPE_INT,    print_group_read, 0 },
		{ false, 0, 2, "group_write",   TYPE_INT,    print_group_write, 0 },
		{ false, 0, 2, "group_execute", TYPE_INT,    print_group_execute, 0 },

		{ false, 0, 2, "other_read",    TYPE_INT,    print_other_read, 0 },
		{ false, 0, 2, "other_write",   TYPE_INT,    print_other_write, 0 },
		{ false, 0, 2, "other_execute", TYPE_INT,    print_other_execute, 0 },

		{ false, 0, 2, "setuid",        TYPE_INT,    print_setuid, 0 },
		{ false, 0, 2, "setgid",        TYPE_INT,    print_setgid, 0 },
		{ false, 0, 2, "sticky",        TYPE_INT,    print_sticky, 0 },

		{ false, 0, 2, "owner_id",      TYPE_INT,    print_owner_id, 0 },
		{ false, 0, 2, "group_id",      TYPE_INT,    print_group_id, 0 },

		{ false, 0, 2, "ctime",         TYPE_STRING, print_ctime, 0 },
		{ false, 0, 2, "atime",         TYPE_STRING, print_atime, 0 },

		{ false, 0, 2, "full_path",     TYPE_STRING, print_full_path, 0 },

		{ false, 0, 3, "mtime_sec",     TYPE_INT,    print_mtime_sec, 0 },
		{ false, 0, 3, "mtime_nsec",    TYPE_INT,    print_mtime_nsec, 0 },

		{ false, 0, 3, "ctime_sec",     TYPE_INT,    print_ctime_sec, 0 },
		{ false, 0, 3, "ctime_nsec",    TYPE_INT,    print_ctime_nsec, 0 },

		{ false, 0, 3, "atime_sec",     TYPE_INT,    print_atime_sec, 0 },
		{ false, 0, 3, "atime_nsec",    TYPE_INT,    print_atime_nsec, 0 },

		{ false, 0, 3, "dev",           TYPE_INT,    print_dev, 0 },
		{ false, 0, 3, "ino",           TYPE_INT,    print_ino, 0 },

		{ false, 0, 3, "rdev",          TYPE_INT,    print_rdev, 0 },

		{ false, 0, 3, "blksize",       TYPE_INT,    print_blksize, 0 },
		{ false, 0, 3, "blocks",        TYPE_INT,    print_blocks, 0 },
	};

	size_t ncolumns = ARRAY_SIZE(columns);
	size_t level = 0;

	while ((opt = getopt_long(argc, argv, "adc:lMN:RsSTU", opts, NULL)) != -1) {
		switch (opt) {
			case 'a':
				all = 1;
				break;
			case 'c':
				cols = xstrdup_nofail(optarg);
				break;
			case 'd':
				dir = 1;
				break;
			case 'l':
				level++;
				for (size_t i = 0; i < ncolumns; ++i)
					if (columns[i].level <= level)
						columns[i].vis = true;
				break;
			case 'M':
				merge = true;
				if (!table)
					table = xstrdup_nofail("file");
				break;
			case 'N':
				table = xstrdup_nofail(optarg);
				break;
			case 'R':
				for (size_t i = 0; i < ncolumns; ++i)
					if (strcmp(columns[i].name, "parent") == 0) {
						columns[i].vis = true;
						break;
					}
				recursive = 1;
				break;
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
				break;
			case 'T':
				table = xstrdup_nofail("file");
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
		csvci_parse_cols_nofail(cols, columns, &ncolumns);

		free(cols);
	} else {
		csvci_set_columns_order(columns, &ncolumns);
	}

	if (show)
		csv_show(show_full);

	struct csvmu_ctx ctx;
	ctx.table = table;
	ctx.merge = merge;

	csvmu_print_header(&ctx, columns, ncolumns);

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
					ncolumns, &ctx);
		} else {
			char *symlink = NULL;
			if (S_ISLNK(buf.st_mode)) do {
				size_t bufsiz;
				assert(buf.st_size >= 0);
				if (buf.st_size)
					bufsiz = (size_t)buf.st_size + 1;
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
				} else if ((size_t)symlink_len == bufsiz) {
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

			csvmu_print_row(&ctx, &info, columns, ncolumns);
		}

		if (close(fd)) {
			perror("close");
			ret |= 2;
			break;
		}
	}

	usr_grp_query_fini();
	free(ctx.table);

	if (ret & 2)
		return 2;
	if (ret & 1)
		return 1;

	return 0;
}
