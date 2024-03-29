/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2021, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "utils.h"

static const struct option opts[] = {
	{"add",		required_argument,	NULL, 'a'},
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
"  -a, --add NAME1[:TYPE1][,NAME2[:TYPE2]...]\n"
"                             add header\n");
	fprintf(out,
"  -e, --set-types NAME1:TYPE1[,NAME2:TYPE2...]\n"
"                             set type of column NAME1 to TYPE1, etc.\n");
	fprintf(out,
"  -G, --guess-types          add types to columns by guessing based on their\n"
"                             contents\n");
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
	bool guess_types;
	struct lines lines;
	enum data_type *types;
};

static int
next_row(const char *buf, const size_t *col_offs, size_t ncols, void *arg)
{
	struct cb_params *params = arg;

	if (!params->guess_types) {
		csv_print_line(stdout, buf, col_offs, ncols, true);

		return 0;
	}

	for (size_t i = 0; i < ncols; ++i) {
		if (params->types[i] == TYPE_STRING)
			continue;

		const char *data = &buf[col_offs[i]];

		if (params->types[i] == TYPE_INT) {
			long long val;
			if (strtoll_safe2(data, &val, 0, false) == 0)
				continue;
			params->types[i] = TYPE_FLOAT;
		}

		if (params->types[i] == TYPE_FLOAT) {
			double val;
			if (strtod_safe2(data, &val, false) == 0)
				continue;
			params->types[i] = TYPE_STRING;
		}
	}

	return lines_add(&params->lines, buf, col_offs, ncols);
}

int
main(int argc, char *argv[])
{
	int opt;
	struct cb_params params;
	bool print_header = true;
	bool print_types = true;
	unsigned show_flags = SHOW_DISABLED;
	char *header = NULL;

	struct column_change {
		char *name;
		char *new_name;
		char *new_type;
	};

	struct column_change *changes = NULL;
	size_t nchanges = 0;
	params.guess_types = false;

	lines_init(&params.lines);

	while ((opt = getopt_long(argc, argv, "a:e:GmMn:sS", opts, NULL)) != -1) {
		switch (opt) {
			case 'a': {
				size_t len = strlen(optarg);
				free(header);
				header = xmalloc_nofail(len + 2, 1);
				strncpy(header, optarg, len);
				header[len] = '\n';
				header[len + 1] = 0;
				break;
			}
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
			case 'G': /* guess-types */
				params.guess_types = true;
				break;
			case 'm': /* remove */
				print_header = false;
				break;
			case 'M': /* remove-types */
				print_types = false;
				break;
			case 'n': { /* rename original,new */
				struct split_result *results = NULL;
				size_t nresults;
				util_split(optarg, ",", &results, &nresults, NULL);

				if (nresults != 2) {
					usage(stderr);
					exit(2);
				}

				changes = xrealloc_nofail(changes, nchanges + 1, sizeof(changes[0]));
				struct column_change *ch = &changes[nchanges++];

				ch->name = xstrndup_nofail(optarg + results[0].start, results[0].len);
				ch->new_name = xstrndup_nofail(optarg + results[1].start, results[1].len);
				ch->new_type = NULL;

				free(results);
				break;
			}
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

	if (show_flags && !print_header) {
		fprintf(stderr,
			"--remove and --show can't be used together\n");
		exit(2);
	}

	if (params.guess_types && !print_header) {
		fprintf(stderr,
			"--remove and --guess-types can't be used together\n");
		exit(2);
	}

	if (params.guess_types && !print_types) {
		fprintf(stderr,
			"--remove-types and --guess-types can't be used together\n");
		exit(2);
	}

	csv_show(show_flags);

	struct csv_ctx *s = csv_create_ctx_nofail(stdin, stderr);

	if (header) {
		csv_insert_header(s, header);
		if (csv_parse_header(s))
			exit(2);
	} else {
		csv_read_header_nofail(s);
	}

	const struct col_header *headers;
	size_t nheaders = csv_get_headers(s, &headers);

	if (params.guess_types) {
		params.types = xmalloc_nofail(nheaders, sizeof(params.types[0]));
		for (size_t i = 0; i < nheaders; ++i)
			params.types[i] = TYPE_INT;

		if (csv_read_all(s, &next_row, &params) < 0)
			exit(2);

		for (size_t i = 0; i < nheaders; ++i) {
			bool found = false;
			struct column_change *ch = NULL;

			for (size_t j = 0; j < nchanges; ++j) {
				ch = &changes[j];
				if (strcmp(ch->name, headers[i].name) != 0)
					continue;
				found = true;

				break;
			}

			if (!found) {
				changes = xrealloc_nofail(changes, nchanges + 1, sizeof(changes[0]));
				ch = &changes[nchanges++];

				ch->name = xstrdup_nofail(headers[i].name);
				ch->new_name = NULL;
			}

			if (params.types[i] == TYPE_INT)
				ch->new_type = xstrdup_nofail("int");
			else if (params.types[i] == TYPE_FLOAT)
				ch->new_type = xstrdup_nofail("float");
			else if (params.types[i] == TYPE_STRING)
				ch->new_type = xstrdup_nofail("string");
			else
				abort();
		}
	}

	if (print_header) {
		bool any_str_column_had_type = false;
		for (size_t i = 0; i < nheaders; ++i) {
			if (headers[i].had_type &&
					strcmp(headers[i].type, "string") == 0) {
				any_str_column_had_type = true;
				break;
			}
		}

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
					if (ch->new_type) {
						if (any_str_column_had_type ||
							strcmp(ch->new_type, "string") != 0)
							printf(":%s", ch->new_type);
					} else if (headers[i].had_type) {
						printf(":%s", headers[i].type);
					}
				}

				found = true;
				break;
			}

			if (!found) {
				printf("%s", headers[i].name);

				if (print_types && headers[i].had_type)
					printf(":%s", headers[i].type);
			}

			if (i == nheaders - 1)
				printf("\n");
			else
				printf(",");
		}
	}

	if (params.guess_types) {
		struct lines *lines = &params.lines;
		for (size_t i = 0; i < lines->used; ++i) {
			struct line *line = &lines->data[i];
			csv_print_line(stdout, line->buf, line->col_offs, nheaders, true);

			lines_free_one(line);
		}
		lines_fini(lines);

		free(params.types);
	} else {
		if (csv_read_all(s, &next_row, &params) < 0)
			exit(2);
	}

	csv_destroy_ctx(s);

	for (size_t j = 0; j < nchanges; ++j) {
		struct column_change *ch = &changes[j];
		free(ch->name);
		free(ch->new_name);
		free(ch->new_type);
	}
	free(changes);

	return 0;
}
