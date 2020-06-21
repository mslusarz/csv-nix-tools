---
title: csv-sql
section: 1
...

# NAME #

csv-sql - process CSV input data using simplified SQL syntax

# SYNOPSIS #

**csv-sql** [OPTION]... sql-query

# DESCRIPTION #

Read CSV stream from standard input, process it using simplified SQL query
and print back to standard output its result.

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

\--help
:   display this help and exit

\--version
:   output version information and exit

# SIMPLIFIED SQL SYNTAX #

Only queries in this form are supported:

`SELECT columns [FROM input] [WHERE condition] [ORDER BY expr1 ASC|DESC[, expr2 ASC|DESC]]`

Only **columns** are required. FROM supports only "input" table and is thus
optional. "WHERE condition" is optional. "ORDER BY" is optional.

No aggregate or window functions are supported.

Columns is comma separated list of column names or expressions, with each one
optionally followed by "AS new-column-name" giving it a new name.

Constants:

| syntax           | description                   | example                   |
|------------------|-------------------------------|---------------------------|
| [-]\[1-9\]\[0-9\]*| decimal integer              | 1, 1294, -89              |
| [-]0x[0-9a-fA-F]+| hexadecimal integer           | 0x1, 0x1A34, -0x8A        |
| [-]0[0-9]+       | octal integer                 | 01, 01234, -067           |
| [-]0b[01]+       | binary integer                | 0b1, 0b1101, -0b10        |
| \'[^']*\'        | string                        | \'text\'                  |
| \"[^"]*\"        | string                        | \"text\"                  |

Operators:

| name        | description                   | example                   |
|-------------|-------------------------------|---------------------------|
| +           | addition                      | expr + 5                  |
| -           | subtraction                   | expr - 5                  |
| *           | multiplication                | expr * 5                  |
| /           | division                      | expr / 5                  |
| %           | modulo                        | expr % 5                  |
| \|          | bitwise or                    | expr \| 5                 |
| &           | bitwise and                   | expr & 5                  |
| ~           | bitwise negation              | ~ expr                    |
| ^           | bitwise xor                   | expr ^ 5                  |
| <<          | bitwise left shift            | expr << 5                 |
| >>          | bitwise right shift           | expr >> 5                 |
| <           | less                          | expr < 5                  |
| <=          | less or equal                 | expr <= 5                 |
| >           | greater                       | expr > 5                  |
| >=          | greater or equal              | expr >= 5                 |
| ==, =       | equal                         | expr = 5, expr == 5       |
| !=, <>      | not equal                     | expr <> 5, expr != 5      |
| and         | logical and                   | expr1 and expr2           |
| or          | logical or                    | expr1 or expr2            |
| xor         | logical exclusive or          | expr1 xor expr2           |
| not         | logical negation              | not expr                  |
| \|\|        | concatenation                 | expr1 \|\| expr2          |
| like        | match pattern                 | expr like \'patt%\'       |
| ()          | expression grouping           | (expr1 + 5) == 7          |

Functions:

| name           | description                   | example                                    |
|----------------|-------------------------------|--------------------------------------------|
| if             | if then else                  | if(bool_expr, expr2, expr3)                |
| substr         | substring                     | substr(str_expr, 1, 3)                     |
| strlen, length | string length                 | strlen(str_expr), length(str_expr)         |
| tostring       | convert to string             | tostring(int_expr, base_expr)              |
|                |                               | tostring(int_expr)                         |
| toint          | convert to integer            | toint(str_expr, base_expr)                 |
|                |                               | toint(str_expr)                            |
| replace        | replace string                | replace(str_expr, \'pat\', \'repl\', 1)    |
|                |                               | replace(str_expr, \'pat\', \'repl\')       |
| replace_bre    | replace string using          | replace_bre(str_expr, \'pat\', \'bre\', 1) |
|                | basic regular expression      | replace_bre(str_expr, \'pat\', \'bre\')    |
| replace_ere    | replace string using          | replace_ere(str_expr, \'pat\', \'ere\', 1) |
|                | extended regular expression   | replace_ere(str_expr, \'pat\', \'ere\')    |
| matches_bre    | string matches basic          | matches_bre(str_expr, \'pat\', 1)          |
|                | regular expression            | matches_bre(str_expr, \'pat\')             |
| matches_ere    | string matches extended       | matches_ere(str_expr, \'pat\', 1)          |
|                | regular expression            | matches_ere(str_expr, \'pat\')             |
| next           | next integer from sequence    | next(\'sequence name\')                    |
|                |                               | next()                                     |


# EXAMPLES #

`csv-ls -c size,name | csv-sql "select size, name from input where size > 2000 and size < 3000" -s`
:    print files whose size is between 2000 and 3000 bytes

`csv-ls -c size,name | csv-sql "select size, name, matches_ere(name, '\.txt$') as is_txt where name like 'a%'" -s`
:    print file names and a boolean saying whether name ends with '.txt' for files whose name starts with 'a'

`csv-ls -c name,mtime,mtime_sec,mtime_nsec | csv-sql "select name, mtime order by mtime_sec desc, mtime_nsec desc" -s`
:    print file names and their modification time ordered by modification time (newest first)

# SEE ALSO #

**csv-sqlite**(1), **csv-add-sql**(1), **csv-grep-sql**(1), **csv-show**(1),
**csv-nix-tools**(7)
