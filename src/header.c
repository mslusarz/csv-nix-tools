/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"set-types",	required_argument,	NULL, 'e'},
	{"guess-types",	no_argument,		NULL, 'G'},
	{"remove",	no_argument,		NULL, 'm'},
	{"remove-types",no_argument,		NULL, 'M'},
	{"rename",	required_argument,	NULL, 'n'},
	{"show",	no_argument,		NULL, 's'},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(FILE *out)
{
	fprintf(out, "Usage: csv-header [OPTION]...\n");
	fprintf(out,
"Read CSV stream from standard input and print back to standard output with\n"
"the column header transformed.\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out,
"  -e, --set-types NAME1:TYPE1[,NAME1:TYPE2...]\n"
"                             set type of column NAME1 to TYPE1, etc.\n");
	fprintf(out,
"  -G, --guess-types          add types to columns by guessing based on their\n"
"                             contents\n"
"                             (NOT IMPLEMENTED YET)\n");
	fprintf(out, "  -m, --remove               remove column header\n");
	fprintf(out,
"  -M, --remove-types         remove types from columns\n");
	fprintf(out,
"  -n, --rename NAME,NEW-NAME rename column NAME to NEW-NAME\n");
	describe_Show(out);
	describe_Show_full(out);
	describe_help(out);
	describe_version(out);
}

struct cb_params {
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	UNUSED(arg);
//	struct cb_params *params = arg;

	csv_print_line(stdout, buf, col_offs, ncols, true);

	return 0;
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool print_header = true;
	bool print_types = true;
	bool show = false;
	bool show_full;

	struct column_change {
		char *name;
		char *new_name;
		char *new_type;
	};

	struct column_change *changes = NULL;
	size_t nchanges = 0;

	while ((opt = getopt_long(argc, argv, "e:GmMn:sS", opts, NULL)) != -1) {
		switch (opt) {
			case 'e': { /* set-types col1:type1[,col2:type2] */
				char *saveptr1;
				char *tmp = strtok_r(optarg, ",", &saveptr1);
				if (!tmp) {
					usage(stderr);
					exit(2);
				}

				do {
					changes = xrealloc_nofail(changes, nchanges + 1, sizeof(changes[0]));
					struct column_change *ch = &changes[nchanges++];

					char *saveptr2;

					tmp = strtok_r(tmp, ":", &saveptr2);
					if (!tmp) {
						usage(stderr);
						exit(2);
					}
					ch->name = xstrdup_nofail(tmp);

					tmp = strtok_r(NULL, ":", &saveptr2);
					if (!tmp) {
						usage(stderr);
						exit(2);
					}
					ch->new_type = xstrdup_nofail(tmp);

					tmp = strtok_r(NULL, ":", &saveptr2);
					if (tmp) {
						usage(stderr);
						exit(2);
					}

					ch->new_name = NULL;

					tmp = strtok_r(NULL, ",", &saveptr1);
				} while (tmp);

				break;
			}
//			case 'G': /* guess-types */
//				break;
			case 'm': /* remove */
				print_header = false;
				break;
			case 'M': /* remove-types */
				print_types = false;
				break;
			case 'n': { /* rename original,new */
				char *tmp = strtok(optarg, ",");
				if (!tmp) {
					usage(stderr);
					exit(2);
				}

				changes = xrealloc_nofail(changes, nchanges + 1, sizeof(changes[0]));
				struct column_change *ch = &changes[nchanges++];

				ch->name = xstrdup_nofail(tmp);

				tmp = strtok(NULL, ",");
				if (!tmp) {
					usage(stderr);
					exit(2);
				}
				ch->new_name = xstrdup_nofail(tmp);
				ch->new_type = NULL;
				break;
			}
			case 's':
				show = true;
				show_full = false;
				break;
			case 'S':
				show = true;
				show_full = true;
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

	if (show) {
		if (!print_header) {
			fprintf(stderr,
				"--remove and --show can't be used at the same time\n");
			exit(2);
		}

		csv_show(show_full);
	}

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	csv_read_header_nofail(s);

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (print_header) {
		for (size_t i = 0; i < nheaders; ++i) {
			bool found = false;
			for (size_t j = 0; j < nchanges; ++j) {
				struct column_change *ch = &changes[j];

				if (strcmp(ch->name, headers[i].name) != 0)
					continue;

				if (ch->new_name)
					printf("%s", ch->new_name);
				else
					printf("%s", ch->name);

				if (print_types) {
					if (ch->new_type)
						printf(":%s", ch->new_type);
					else
						printf(":%s", headers[i].type);
				}

				found = true;
				break;
			}

			if (!found) {
				printf("%s", headers[i].name);

				if (print_types)
					printf(":%s", headers[i].type);
			}

			if (i == nheaders - 1)
				printf("\n");
			else
				printf(",");
		}
	}

	if (csv_read_all(s, &next_row, &params) < 0)
		exit(2);

	csv_destroy_ctx(s);

	return 0;
}
