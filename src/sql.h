/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
 */

#ifndef CSV_SQL_H
#define CSV_SQL_H

#include <stdbool.h>
#include <stdio.h>
#include "utils.h"

extern FILE *yyin;

int yylex();
int yyerror(const char *);
int yyparse(void);
int yylex_destroy(void);

void sql_stack_push(const struct rpn_token *token);
void sql_stack_push_string(char *str);
void sql_stack_push_llong(long long l);
void sql_stack_push_literal(char *str);
void sql_stack_push_dbl(double d);
void sql_stack_push_op(enum rpn_operator);

void sql_column_done(void);
void sql_named_column_done(char *name);
void sql_all_columns(void);
void sql_from(char *name);
void sql_where();
void sql_end_of_conditions();

void sql_order_by();
void sql_order_by_expr_done(bool asc);

#endif
