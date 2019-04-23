/*
 * st-ls.c
 *
 *  Created on: 22.04.2019
 *      Author: marcin
 */

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
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

	if (hcreate_r(ht->keys_max, &ht->ht) == 0) {
		perror("hcreate_r");
		return 2;
	}

	ht->table = malloc(ht->table_size * sizeof(ht->table[0]));
	if (!ht->table) {
		perror("malloc");
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
			exit(2);
		}

		for (size_t u = 0; u < ht->inserted; ++u) {
			e.key = ht->table[u].key;
			e.data = NULL;

			if (hsearch_r(e, ENTER, &entry, &ht->ht) == 0)
				exit(1);

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
		if (!ht->table)
			abort();
	}

	entry->key = strdup(key);
	if (!entry->key)
		abort();

	entry->data = cb(cb_data);
	if (!entry->data)
		abort();

	ht->table[ht->inserted].key = entry->key;
	ht->table[ht->inserted].data = entry->data;
	ht->inserted++;

	return entry->data;
}

static void *
get_user_slow(void *uidp)
{
	uid_t uid = *(uid_t *)uidp;
	struct passwd *passwd = getpwuid(uid);
	if (passwd)
		return strdup(passwd->pw_name);

	char *ret;
	asprintf(&ret, "%d", uid);
	return ret;
}

static const char *
get_user(uid_t uid)
{
	char key[10];
	sprintf(key, "%d", uid);

	return get_value(&users_ht, key, get_user_slow, &uid);
}

static void *
get_group_slow(void *gidp)
{
	gid_t gid = *(uid_t *)gidp;
	struct group *gr = getgrgid(gid);
	if (gr)
		return strdup(gr->gr_name);

	char *ret;
	asprintf(&ret, "%d", gid);
	return ret;
}

static const char *
get_group(gid_t gid)
{
	char key[10];
	sprintf(key, "%d", gid);

	return get_value(&groups_ht, key, get_group_slow, &gid);
}

static void
print_stat(const char *dirpath, const char *path, struct stat *st,
		const char *symlink)
{
#if 1
	fprintf(stdout, "mode 0%06o nlink %2ld uid %5d gid %5d size %8ld mtime %ld ",
		st->st_mode, st->st_nlink, st->st_uid, st->st_gid, st->st_size,
		st->st_mtime);
#else
	fprintf(stdout, "dev 0x%lx ino %ld mode 0%06o nlink %ld uid %d gid %d rdev 0x%lx size %6ld blksize %ld blocks %3ld atime %ld mtime %ld ctime %ld ",
		st->st_dev, st->st_ino, st->st_mode, st->st_nlink,
		st->st_uid, st->st_gid, st->st_rdev, st->st_size,
		st->st_blksize, st->st_blocks, st->st_atime, st->st_mtime,
		st->st_ctime);
#endif
	printf("owner %8s ", get_user(st->st_uid));
	printf("group %8s ", get_group(st->st_gid));

	if (dirpath)
		printf("parent %s ", dirpath);

	if (S_ISLNK(st->st_mode) && symlink)
		printf("name %s -> %s\n", path, symlink);
	else
		printf("name %s\n", path);
	// TODO: nsec
	// TODO: translate mode
}

static int
alphasort_caseinsensitive(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

static int
list(const char *dirpath, int dirfd, int recursive, int all, int sort)
{
	int ret = 0;
	struct dirent **namelist;
	const char **dirs;
	int numdirs = 0;
	char *path;
	size_t pos = strlen(dirpath);

	if (pos < 1)
		return -1;

	int (*compar)(const struct dirent **, const struct dirent **);
	if (sort)
		compar = alphasort_caseinsensitive;
	else
		compar = NULL;

	int entries = scandirat(dirfd, ".", &namelist, NULL, compar);
	if (entries < 0) {
		fprintf(stderr, "reading %s failed: %s\n", dirpath,
				strerror(errno));
		return -1;
	}

	dirs = malloc(entries * sizeof(dirs[0]));
	if (!dirs) {
		perror("malloc");
		ret = -1;
		goto dirs_alloc_fail;
	}

	path = malloc(pos + 1 + sizeof(namelist[0]->d_name));
	if (!path) {
		perror("malloc");
		ret = -1;
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
			ret = 1;
			continue;
		}

		char *symlink = NULL;
		if (S_ISLNK(statbuf.st_mode)) {
			size_t bufsiz;
			if (statbuf.st_size)
				bufsiz = statbuf.st_size + 1;
			else
				bufsiz = PATH_MAX;
restart_readlink:
			symlink = malloc(bufsiz);
			ssize_t symlink_len = readlinkat(dirfd,
					namelist[i]->d_name, symlink,
					bufsiz);
			if (symlink_len < 0) {
				perror("readlinkat");
				free(symlink);
				symlink = NULL;
				ret = 1;
			} else if (symlink_len == bufsiz) {
				free(symlink);
				bufsiz *= 2;
				goto restart_readlink;
			} else {
				symlink[symlink_len] = 0;
			}
		}

		print_stat(dirpath, namelist[i]->d_name, &statbuf, symlink);

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
			perror("openat");
			continue;
		}

		strcpy(path + pos, dirs[j]);

		if (list(path, fd, recursive, all, sort))
			ret = 1;

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

int
main(int argc, char *argv[])
{
	int opt, ret = 0;
	int dir = 0, recursive = 0, all = 0, sort = 1;

	while ((opt = getopt(argc, argv, "adRU")) != -1) {
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
			case 'U':
				sort = 0;
				break;
			default:
				fprintf(stderr,
					"Usage: %s [-a] [-d] [-R] [-U] [paths ...]\n",
					argv[0]);
				break;
		}
	}

	if (ht_init(&users_ht))
		return 2;

	if (ht_init(&groups_ht))
		return 2;

	for (int i = optind; i < argc; ++i) {
		int fd = openat(AT_FDCWD, argv[i], O_PATH | O_NOFOLLOW);
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

		if (S_ISDIR(buf.st_mode) && !dir) {
			if (list(argv[i], fd, recursive, all, sort))
				ret = 1;
		} else {
			char *symlink = NULL;
			if (S_ISLNK(buf.st_mode)) {
				size_t bufsiz;
				if (buf.st_size)
					bufsiz = buf.st_size + 1;
				else
					bufsiz = PATH_MAX;

restart_readlink:
				symlink = malloc(bufsiz);
				ssize_t symlink_len = readlinkat(fd,
						"", symlink, bufsiz);
				if (symlink_len < 0) {
					perror("readlinkat");
					free(symlink);
					symlink = NULL;
					ret = 1;
				} else if (symlink_len == bufsiz) {
					free(symlink);
					bufsiz *= 2;
					goto restart_readlink;
				} else {
					symlink[symlink_len] = 0;
				}
			}

			print_stat(NULL, argv[i], &buf, symlink);
		}

		close(fd);
	}

	ht_destroy(&users_ht);
	ht_destroy(&groups_ht);

	return ret;
}
