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

#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>

#include "parse.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void csv_print_header(FILE *out, const struct col_header *headers,
		size_t nheaders);

void csv_print_line(FILE *out, const char *buf, const size_t *col_offs,
		const struct col_header *headers, size_t nheaders, bool nl);

int strtoll_safe(const char *str, long long *val, int base);
int strtoull_safe(const char *str, unsigned long long *val, int base);
int strtol_safe(const char *str, long *val, int base);
int strtoul_safe(const char *str, unsigned long *val, int base);
int strtoi_safe(const char *str, int *val, int base);
int strtou_safe(const char *str, unsigned *val, int base);

char *strnchr(const char *str, int c, size_t len);
void print_timespec(const struct timespec *ts, bool nsec);

bool csv_requires_quoting(const char *str, size_t len);
void csv_print_quoted(const char *str, size_t len);
void csv_show(void);
void csv_substring_sanitize(const char *str, ssize_t *start, size_t *len);

char *csv_unquot(const char *str);
#if 0
/* currently not used */
void csv_unquot_in_place(char *str);
#endif

#define CSV_NOT_FOUND SIZE_MAX
size_t csv_find(const struct col_header *headers,
		size_t nheaders,
		const char *name);

char *xstrdup(const char *str);
char *xstrdup_nofail(const char *str);

char *xstrndup(const char *str, size_t n);
char *xstrndup_nofail(const char *str, size_t n);

void *xmalloc(size_t count, size_t size);
void *xmalloc_nofail(size_t count, size_t size);

void *xcalloc(size_t count, size_t size);
void *xcalloc_nofail(size_t count, size_t size);

void *xrealloc(void *ptr, size_t count, size_t size);
void *xrealloc_nofail(void *ptr, size_t count, size_t size);

enum rpn_token_type {
	RPN_COLUMN,		/* column */
	RPN_CONSTANT,		/* constant */
	RPN_OPERATOR,		/* rpn_operator */
};

enum rpn_operator {
	RPN_ADD,		/* numeric add */
	RPN_SUB,		/* numeric subtract */
	RPN_MUL,		/* numeric multiplication */
	RPN_DIV,		/* numeric division */
	RPN_MOD,		/* numeric modulo */
	RPN_BIT_OR,		/* bitwise or */
	RPN_BIT_AND,		/* bitwise and */
	RPN_BIT_XOR,		/* bitwise xor */
	RPN_BIT_NEG,		/* bitwise negation */
	RPN_BIT_LSHIFT,		/* bitwise left shift */
	RPN_BIT_RSHIFT,		/* bitwise right shift */
	RPN_SUBSTR,		/* substring */
	RPN_STRLEN,		/* string length */
	RPN_CONCAT,		/* concatenation of 2 strings */
	RPN_LIKE,		/* SQL LIKE operator */
	RPN_TOSTRING_BASE2,	/* convert int to string encoded as binary */
	RPN_TOSTRING_BASE8,	/* convert int to string encoded as octal */
	RPN_TOSTRING_BASE10,	/* convert int to string encoded as decimal */
	RPN_TOSTRING_BASE16,	/* convert int to string encoded as hexadecimal */
	RPN_TOINT,		/* convert string to int */
	RPN_LT,			/* less */
	RPN_LE,			/* less or equal */
	RPN_GT,			/* greater */
	RPN_GE,			/* greater or equal */
	RPN_EQ,			/* equal */
	RPN_NE,			/* no equal */
	RPN_LOGIC_OR,		/* logic or */
	RPN_LOGIC_AND,		/* logic and */
	RPN_LOGIC_NOT,		/* logic not */
	RPN_LOGIC_XOR,		/* logic xor */
	RPN_IF,			/* if then else */
};

enum rpn_variant_type {
	RPN_LLONG,
	RPN_PCHAR,
};

struct rpn_variant {
	enum rpn_variant_type type;
	union {
		long long llong;
		char *pchar;
	};
};

struct rpn_token {
	enum rpn_token_type type;
	union {
		size_t colnum;
		struct rpn_variant constant;
		enum rpn_operator operator;
	};
};

struct rpn_expression {
	struct rpn_token *tokens;
	size_t count;
};

int rpn_parse(struct rpn_expression *exp, char *str,
		const struct col_header *headers, size_t nheaders);
int rpn_eval(struct rpn_expression *exp,
		const char *buf,
		const size_t *col_offs,
		const struct col_header *headers,
		struct rpn_variant *value);

const char *rpn_expression_type(const struct rpn_expression *exp,
		const struct col_header *headers);

void rpn_free(struct rpn_expression *exp);

typedef void (*print_fn)(const void *ptr);

enum output_types { TYPE_STRING, TYPE_INT, TYPE_STRING_ARR, TYPE_INT_ARR };

struct column_info {
	bool vis;
	size_t order;
	const char *name;
	enum output_types type;
	print_fn print;
	uint64_t data; /* tool-specific data */
};

int csvci_parse_cols(char *cols, struct column_info *columns, size_t *ncolumns);
void csvci_parse_cols_nofail(char *cols, struct column_info *columns,
		size_t *ncolumns);
void csvci_set_columns_order(struct column_info *columns, size_t *ncolumns);

void csvci_print_header(struct column_info *columns, size_t ncolumns);
void csvci_print_row(const void *row, const struct column_info *columns,
		size_t ncolumns);

#endif
