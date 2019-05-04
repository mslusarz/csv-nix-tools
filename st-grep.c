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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct option long_options[] = {
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: st-grep [OPTION]...\n");
	printf("Options:\n");
	printf("  -e column=value\n");
	printf("  -v\n");
	printf("      --help\n");
	printf("      --version\n");
}

struct header {
	char *name;
	char *type;
};

static void
add_header(struct header **headers, size_t *nheaders, char *start)
{
	(*nheaders)++;

	*headers = realloc(*headers, (*nheaders) * sizeof((*headers)[0]));
	if (!*headers) {
		perror("realloc");
		exit(2);
	}

	(*headers)[*nheaders - 1].name = start;

	char *pipe = strchr(start, '|');
	if (!pipe) {
		fprintf(stderr, "one of the columns does not have type name\n");
		exit(2);
	}

	*pipe = 0;
	(*headers)[*nheaders - 1].type = pipe + 1;
}

static void
yield_row(const char *buf, const size_t *col_offs, struct header *headers,
		size_t nheaders)
{
#if 0
	for (size_t i = 0; i < nheaders; ++i) {
		printf("column %ld: '", i);
		fputs(&buf[col_offs[i]], stdout);
		printf("' end of column %ld %s\n", i, headers[i].name);
	}
#endif

	for (size_t i = 0; i < nheaders - 1; ++i) {
		fputs(&buf[col_offs[i]], stdout);
		fputs(",", stdout);
	}
	fputs(&buf[col_offs[nheaders - 1]], stdout);
	fputs("\n", stdout);
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	int invert = 0;
	struct condition {
		char *column;
		char *value;
		size_t col_num;
	};

	struct condition *conditions = NULL;
	size_t nconditions = 0;

	while ((opt = getopt_long(argc, argv, "e:v", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'e': {
				const char *eq = strchr(optarg, '=');
				if (!eq) {
					fprintf(stderr, "incorrect -e syntax\n");
					exit(2);
				}
				struct condition cond;
				cond.column = strndup(optarg, (uintptr_t)eq -
						(uintptr_t)optarg);
				if (!cond.column) {
					perror("strndup");
					exit(2);
				}

				const char *start;
				size_t len;
				if (eq[1] == '\"') {
					start = eq + 2;
					len = strlen(start) - 1;
				} else {
					start = eq + 1;
					len = strlen(start);
				}
				cond.value = strndup(start, len);
				if (!cond.value) {
					perror("strndup");
					exit(2);
				}

				nconditions++;
				conditions = realloc(conditions, nconditions *
						sizeof(conditions[0]));
				if (!conditions) {
					perror("realloc");
					exit(2);
				}
				memcpy(&conditions[nconditions - 1], &cond,
						sizeof(cond));

				break;
			}
			case 'v':
				invert = 1;
				break;
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
					case 1:
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

//	for (size_t i = 0; i < nconditions; ++i) {
//		printf("'%s' = '%s'\n", conditions[i].column,
//				conditions[i].value);
//	}

	char *line = NULL;
	size_t buf_size = 0;
	ssize_t line_len = getline(&line, &buf_size, stdin);
	if (line_len < 1) {
		perror("getline");
		exit(2);
	}

	struct header *headers = NULL;
	size_t nheaders = 0;

	char *start = line;
	char *comma;
	do {
		comma = strchr(start, ',');
		if (comma) {
			*comma = 0;

			add_header(&headers, &nheaders, start);

			start = comma + 1;
		}
	} while (comma);

	char *nl = strchr(start, '\n');
	if (!nl) {
		fprintf(stderr, "corrupted input\n");
		exit(2);
	}
	*nl = 0;

	add_header(&headers, &nheaders, start);

	for (size_t i = 0; i < nconditions; ++i) {
		bool found = false;
		for (size_t j = 0; j < nheaders; ++j) {
			found = strcmp(conditions[i].column, headers[j].name)
					== 0;
			if (found) {
				conditions[i].col_num = j;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "column '%s' not found in input\n",
					conditions[i].column);
			exit(2);
		}
	}

	for (size_t i = 0; i < nheaders - 1; ++i)
		printf("%s|%s,", headers[i].name, headers[i].type);
	printf("%s|%s\n", headers[nheaders - 1].name, headers[nheaders - 1].type);

	size_t *col_offs = malloc(nheaders * sizeof(col_offs[0]));
	if (!col_offs) {
		perror("malloc");
		exit(2);
	}

	int eof = 0;
	size_t buflen = 0;
	char *buf = NULL;
	size_t ready = 0;

	int column = 0;
	bool in_quoted_string = false;
	bool last_char_was_quot = false;
	col_offs[0] = 0;
	size_t start_off = 0;

	while (1) {
		size_t i = start_off;
		while (i < ready) {
			if (last_char_was_quot) {
				if (buf[i] == '"') {
					// this character was escaped

					// so switch back to in_quot_string logic
					last_char_was_quot = false;

					// and continue from the next character
					i++;
					continue;
				}

				// last " was end of quoted string

				// so reset quoting logic
				last_char_was_quot = false;
				in_quoted_string = false;

				// and continue from the *same* character
				continue;
			} else if (in_quoted_string) {
				if (buf[i] == '"')
					last_char_was_quot = true;

				i++;
				continue;
			} else {
				if (buf[i] == ',' || buf[i] == '\n') {
					// end of non-quoted column
					buf[i] = 0;
					column++;
					if (column == nheaders) {
						yield_row(buf, col_offs,
							headers, nheaders);

						i++;
						memmove(&buf[0], &buf[i], ready - i);
						ready -= i;
						i = 0;
						column = 0;
						continue;
					}

					// move on to the next column
					i++;
					col_offs[column] = i;
					continue;
				} else if (buf[i] == '"') {
					// if we are not at the beginning of
					// a column, then the stream is corrupted
					if (i != col_offs[column])
						abort(); // XXX

					// switch to quoted string logic
					in_quoted_string = true;

					// and continue from the next character
					i++;
					continue;
				} else {
					// we are in the middle of a column
					i++;
					continue;
				}
			}
		}

		if (eof)
			break;

		if (ready == buflen) {
			buflen += 1024;
			buf = realloc(buf, buflen);
			if (!buf)
				abort();
		}

		size_t nmemb = buflen - ready;
		/*
		 * Don't ask for too much, otherwise memove after each row
		 * becomes a perf problem. TODO: figure out how to fix this
		 * without impacting code readability.
		 */
		if (nmemb > 100)
			nmemb = 100;

		size_t readin = fread(&buf[ready], 1, nmemb, stdin);
		if (readin < nmemb) {
			if (feof(stdin)) {
				eof = 1;
			} else {
				perror("fread");
				exit(2);
			}
		}

		start_off = ready;
		ready += readin;
	}

	free(line);
	free(headers);
	free(col_offs);
	free(buf);

	for (size_t i = 0; i < nconditions; ++i) {
		free(conditions[i].column);
		free(conditions[i].value);
	}

	free(conditions);

	return 0;
}
