/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>

#include "parse.h"

#define TABLE_COLUMN ("_table")
#define TABLE_SEPARATOR ('.')
#define UNUSED(expr) do { (void)(expr); } while (0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void csv_print_header(FILE *out, const struct col_header *headers,
		size_t nheaders);

void csv_print_line(FILE *out, const char *buf, const size_t *col_offs,
		size_t ncols, bool nl);

void csv_print_line_reordered(FILE *out, const char *buf,
		const size_t *col_offs, size_t ncols, bool nl,
		const size_t *idx);

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

#define SHOW_DISABLED (0)
#define SHOW_SIMPLE (1u << 0)
#define SHOW_FULL (1u << 1)
void csv_show(unsigned flags);

void csv_substring_sanitize(const char *str, ssize_t *start, size_t *len);

char *csv_unquot(const char *str);
#if 0
/* currently not used */
void csv_unquot_in_place(char *str);
#endif

#define CSV_NOT_FOUND SIZE_MAX
size_t csv_find(const struct col_header *headers,
		size_t nheaders,
		const char *column);
size_t csv_find_by_table(const struct col_header *headers, size_t nheaders,
		const char *table, const char *column);
size_t csv_find_loud(const struct col_header *headers, size_t nheaders,
		const char *table, const char *column);
void csv_column_doesnt_exist(const struct col_header *headers, size_t nheaders,
		const char *table, const char *column);

int csv_print_table_func_header(const struct col_header *h, const char *func,
		const char *table, size_t table_len, char sep,
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
	RPN_TOSTRING,		/* convert int to string */
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
	RPN_REPLACE,		/* replace string */
	RPN_REPLACE_BRE,	/* replace string using basic regular expression */
	RPN_REPLACE_ERE,	/* replace string using extended regular expression */
	RPN_MATCHES_BRE,	/* string matches basic regular expression */
	RPN_MATCHES_ERE,	/* string matches extended regular expression */
	RPN_NEXT,		/* next integer from named sequence */
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
		struct {
			size_t num;
			const char *type;
		} col;
		struct rpn_variant constant;
		enum rpn_operator operator;
	};
};

struct rpn_expression {
	struct rpn_token *tokens;
	size_t count;
};

int rpn_parse(struct rpn_expression *exp, char *str,
		const struct col_header *headers, size_t nheaders,
		const char *table);
int rpn_eval(const struct rpn_expression *exp,
		const char *buf,
		const size_t *col_offs,
		struct rpn_variant *value);

const char *rpn_expression_type(const struct rpn_expression *exp,
		const struct col_header *headers);

void rpn_free(struct rpn_expression *exp);
void rpn_fini(void);

typedef void (*print_fn)(const void *ptr);

enum output_types { TYPE_STRING, TYPE_INT, TYPE_STRING_ARR, TYPE_INT_ARR };

struct column_info {
	bool vis;
	size_t order;
	size_t level;
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
void csvci_print_header_with_prefix(struct column_info *columns,
		size_t ncolumns, const char *prefix);
void csvci_print_row(const void *row, const struct column_info *columns,
		size_t ncolumns);
void csv_check_space(char **buf, size_t *buflen, size_t used, size_t len);
bool csv_str_replace(const char *str, const char *pattern,
		const char *replacement, bool case_sensitive, char **buf,
		size_t *buflen);

void describe_Merge(FILE *out);
void describe_table_Name(FILE *out);
void describe_as_Table(FILE *out, const char *name);

void describe_Columns(FILE *out);
void describe_help(FILE *out);
void describe_version(FILE *out);
void describe_Show(FILE *out);
void describe_Show_full(FILE *out);
void describe_Table(FILE *out);

int csv_asprintf(char **strp, const char *fmt, ...);
char *csv_strcasestr(const char *haystack, const char *needle);
void csv_qsort_r(void *base, size_t nmemb, size_t size,
                  int (*compar)(const void *, const void *, void *),
                  void *arg);

struct split_result {
	size_t start;
	size_t len;
};
void util_split(const char *str, const char *sep,
		struct split_result **results_buf, size_t *nresults_buf,
		size_t *results_maxsize_buf);
void util_split_term(char *str, const char *sep,
		struct split_result **results_buf, size_t *nresults_buf,
		size_t *results_maxsize_buf);

unsigned
util_sprintf(char *str, const char *format, ...) __attribute__ ((format(printf, 2, 3)));

#endif
