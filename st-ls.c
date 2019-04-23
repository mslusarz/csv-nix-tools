/*
 * st-ls.c
 *
 *  Created on: 22.04.2019
 *      Author: marcin
 */

#define _GNU_SOURCE

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct inode {
	dev_t dev;
	ino_t ino;
};

struct dir {
	int fd;
	char *path;
};

void print_stat(const char *dirpath, const char *path, struct stat *st)
{
	fprintf(stdout, "dev 0x%lx ino %ld mode 0%06o nlink %ld uid %d gid %d rdev 0x%lx size %6ld blksize %ld blocks %3ld atime %ld mtime %ld ctime %ld name ",
		st->st_dev, st->st_ino, st->st_mode, st->st_nlink,
		st->st_uid, st->st_gid, st->st_rdev, st->st_size,
		st->st_blksize, st->st_blocks, st->st_atime, st->st_mtime,
		st->st_ctime);

	if (dirpath)
		if (strlen(dirpath) > 0 && dirpath[strlen(dirpath) - 1] == '/')
			printf("%s%s\n", dirpath, path);
		else
			printf("%s/%s\n", dirpath, path);
	else
		printf("%s\n", path);
	// TODO: nsec
	// TODO: user, group
	// TODO: translate mode
	// TODO: separarate parent & name
}

int alphasort_caseinsensitive(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

int list(const char *dirpath, int dirfd, int recursive, int all)
{
	int ret = 0;
	struct dirent **namelist;
	struct dir *dirs = NULL;
	int numdirs = 0;

	int entries = scandirat(dirfd, ".", &namelist, NULL,
			alphasort_caseinsensitive);
	if (entries < 0) {
		perror("scandirat");
		return -1;
	}

	for (int i = 0; i < entries; ++i) {
		struct stat statbuf;
		if (!all && namelist[i]->d_name[0] == '.') {
			/*
			 * there's no point in fstating if we are going to skip
			 * it anyway
			 */
			continue;
		}

		if (fstatat(dirfd, namelist[i]->d_name, &statbuf,
				AT_SYMLINK_NOFOLLOW)) {
			ret = 1;
			continue;
		}

		if (all || namelist[i]->d_name[0] != '.')
			print_stat(dirpath, namelist[i]->d_name, &statbuf);

		if (!recursive)
			continue;
		if (!S_ISDIR(statbuf.st_mode))
			continue;
		if (strcmp(namelist[i]->d_name, ".") == 0)
			continue;
		if (strcmp(namelist[i]->d_name, "..") == 0)
			continue;

		struct dir *newdirs = realloc(dirs,
				(numdirs + 1) * sizeof(dirs[0]));
		if (!newdirs) {
			perror("malloc");
			continue;
		}

		dirs = newdirs;

//		int fd = openat(dirfd, namelist[i]->d_name, O_PATH);
//		if (fd < 0) {
//			perror("openat");
//			continue;
//		}

//		dirs[numdirs].fd = fd;
//		dirs[numdirs].path = malloc(strlen(dirpath) + 1 +
//				strlen(namelist[i]->d_name) + 1);
//		if (strlen(dirpath) > 0 && dirpath[strlen(dirpath) - 1] == '/')
//			sprintf(dirs[numdirs].path, "%s%s", dirpath,
//				namelist[i]->d_name);
//		else
//			sprintf(dirs[numdirs].path, "%s/%s", dirpath,
//				namelist[i]->d_name);
		dirs[numdirs].path = strdup(namelist[i]->d_name);
		numdirs++;
	}

	free(namelist);

	for (int j = 0; j < numdirs; ++j) {
		int fd = openat(dirfd, dirs[j].path, O_PATH);
		if (fd < 0) {
			perror("openat");
			continue;
		}

		/**/
		char *path = malloc(strlen(dirpath) + 1 +
				strlen(dirs[j].path) + 1);
		if (strlen(dirpath) > 0 && dirpath[strlen(dirpath) - 1] == '/')
			sprintf(path, "%s%s", dirpath,
					dirs[j].path);
		else
			sprintf(path, "%s/%s", dirpath,
					dirs[j].path);
		/**/

//		if (list(dirs[j].path, dirs[j].fd, recursive, all, dir_inodes,
//				seen_dir_inodes))
		if (list(path, fd, recursive, all))
			ret = 1;
		free(dirs[j].path);
		free(path);
		close(fd);
//		close(dirs[j].fd);
	}
	free(dirs);

	return ret;
}

int main(int argc, char *argv[])
{
	int opt, ret = 0;
	int dir = 0, recursive = 0, all = 0;

	while ((opt = getopt(argc, argv, "adR")) != -1) {
		// TODO: -U
		switch (opt) {
			case 'a':
				all = 1;
				break;
			case 'd':
				dir = 1;
				break;
			case 'R':
				recursive = 1;
				break;
			default:
				fprintf(stderr,
					"Usage: %s [-a] [-d] [-R] [paths ...]\n",
					argv[0]);
				break;
		}
	}

	for (int i = optind; i < argc; ++i) {
		/* TODO: O_NOFOLLOW */
		int fd = openat(AT_FDCWD, argv[i], O_PATH);
		if (fd < 0) {
			perror("open");
			continue;
		}

		struct stat buf;
		int r = fstat(fd, &buf);
		if (r < 0) {
			perror("fstat");
			close(fd);
			continue;
		}

		if (S_ISDIR(buf.st_mode)) {
			if (dir) {
				print_stat(NULL, argv[i], &buf);
			} else {
				if (list(argv[i], fd, recursive, all))
					ret = 1;
			}
		} else {
			print_stat(NULL, argv[i], &buf);
		}

		close(fd);
	}

	return ret;
}
