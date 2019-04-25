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
#include <stdio.h>
#include <string.h>

#include <libxml/xmlreader.h>

enum outformat {FMT_TEXT, FMT_XML, FMT_JSON};

static const struct option long_options[] = {
	{"st-format", 	required_argument,	NULL, 0},
	{"version",	no_argument,		NULL, 'V'},
	{"help",	no_argument,		NULL, 'h'},
	{NULL,		0,			NULL, 0},
};

static void
usage(void)
{
	printf("Usage: st-grep [OPTION]...\n");
	printf("Options:\n");
	printf("      --st-format=xml/json/text\n");
	printf("      --help\n");
	printf("      --version\n");
}

/*
 * type == 'reg'
 * owner_name != 'name'
 * int(size) >= 1024
 * timespec(mtime) > timespec_from_datetime('2019-01-01 01:02:03.012345')
 * int(mode) & 0600 == 0600
 * bool(owner_read) != bool(group_read)
 * name[0, 2] == 'st'
 * name[-2, 2] == 'ls'
 */
enum value_op {none, to_int, to_boolean, datetime_to_timespec, timespec_to_datetime};
enum compare_op {EQ, NE, LE, LT, GE, GT};

struct condition {
	enum value_op attr_op;
	const char *attr;

	enum compare_op compare_op;

	enum value_op value_op;
	const char *value;
};

static void
processNode(xmlTextReaderPtr reader) {
	const xmlChar *name, *value;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

	int depth = xmlTextReaderDepth(reader);
	if (depth == 0) {
		if (strcmp(name, "st") != 0) {
			fprintf(stderr,
				"this is not st document (top-level='%s')\n",
				name);
			exit(2);
		}

		return;
	}

	if (depth != 1) {
		fprintf(stderr, "this document is too deep\n");
		exit(2);
	}

	int type = xmlTextReaderNodeType(reader);

	if (type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
		return;
	if (type != XML_READER_TYPE_ELEMENT) {
		fprintf(stderr, "unhandled type: %d\n", type);
		exit(2);
	}

	if (strcmp(name, "entry") != 0) {
		fprintf(stderr, "unknown tag: '%s'\n", name);
		exit(2);
	}

	xmlNodePtr node = xmlTextReaderCurrentNode(reader);
	xmlAttr *attr = node->properties;
	while (attr) {
		xmlChar* value = xmlNodeListGetString(node->doc, attr->children, 1);
		printf ("A %s: %s\n",attr->name, value);
		xmlFree(value);

		attr = attr->next;
	}
}

int
main(int argc, char *argv[])
{
	int opt;
	int longindex;
	enum outformat format = FMT_TEXT;

	while ((opt = getopt_long(argc, argv, "", long_options,
			&longindex)) != -1) {
		switch (opt) {
			case 'V':
				printf("git\n");
				return 0;
			case 0:
				switch (longindex) {
					case 0:
						if (strcmp(optarg, "xml") == 0)
							format = FMT_XML;
						else if (strcmp(optarg, "json") == 0)
							format = FMT_JSON;
						else if (strcmp(optarg, "text") == 0)
							format = FMT_TEXT;
						else {
							fprintf(stderr,
								"unknown format '%s'\n",
								optarg);
							return 2;
						}

						break;
					case 1:
						break;
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

	for (int i = optind; i < argc; ++i) {

	}

	if (format == FMT_JSON) {
		fprintf(stderr, "json format not implemented yet\n");
		return 2;
	}

	xmlTextReader *reader = xmlReaderForFd(0, NULL, NULL, 0);
	if (!reader)
		abort();

	int ret;
	do {
		ret = xmlTextReaderRead(reader);
		if (ret == 1)
			processNode(reader);
	} while (ret == 1);

	xmlFreeTextReader(reader);

	if (ret != 0) {
		fprintf(stderr, "failed to parse\n");
	}

	return 0;
}
