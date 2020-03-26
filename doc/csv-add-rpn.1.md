---
title: csv-add-rpn
section: 1
...

# NAME #

csv-add-rpn - add a new column from RPN expression

# SYNOPSIS #

**csv-add-rpn** [OPTION]...

# DESCRIPTION #

Read CSV stream from standard input and print it back to standard output with
a new column produced by evaluation of RPN (reverse Polish notation) expression.

-e *RPN-EXPR*
:   use expression *RPN-EXPR* to create new column; RPN expressions use space
as a separator, so this needs to be quoted

-n *NEW-NAME*
:   create column *NEW-NAME* as an output

-s, \--show
:   print output in table format

-S, \--show-full
:   print output in table format with pager

-T, \--table=*NAME*
:   apply to rows only with _table column equal *NAME*

\--help
:   display this help and exit

\--version
:   output version information and exit

# RPN SYNTAX #

Variables:

| syntax | description     | example |
|--------|-----------------|---------|
| %.*    | value of column | %name   |

Constants:

| syntax           | description                   | example                   |
|------------------|-------------------------------|---------------------------|
| [-]\[1-9\]\[0-9\]*| decimal integer              | 1, 1294, -89              |
| [-]0x[0-9a-fA-F]+| hexadecimal integer           | 0x1, 0x1A34, -0x8A        |
| [-]0[0-9]+       | octal integer                 | 01, 01234, -067           |
| [-]0b[01]+       | binary integer                | 0b1, 0b1101, -0b10        |
| '[^']*'          | string                        | 'text'                    |
| "[^"]*"          | string                        | "text"                    |

Operators:

| name        | description                   | example                   |
|-------------|-------------------------------|---------------------------|
| +           | addition                      | %num 5 +                  |
| -           | subtraction                   | %num 5 -                  |
| *           | multiplication                | %num 5 *                  |
| /           | division                      | %num 5 /                  |
| %           | modulo                        | %num 5 %                  |
| \|          | bitwise or                    | %num 5 \|                 |
| &           | bitwise and                   | %num 5 &                  |
| ~           | bitwise negation              | %num ~                    |
| ^           | bitwise xor                   | %num 5 ^                  |
| <<          | bitwise left shift            | %num 5 <<                 |
| >>          | bitwise right shift           | %num 5 >>                 |
| lt, <       | less                          | %num 5 lt, %num 5 <       |
| le, <=      | less or equal                 | %num 5 le, %num 5 <=      |
| gt, >       | greater                       | %num 5 gt, %num 5 >       |
| ge, >=      | greater or equal              | %num 5 ge, %num 5 >=      |
| eq, ==      | equal                         | %num 5 eq, %num 5 ==      |
| ne, !=      | not equal                     | %num 5 ne, %num 5 !=      |
| and         | logical and                   | %bool1 %bool2 and         |
| or          | logical or                    | %bool1 %bool2 or          |
| xor         | logical exclusive or          | %bool1 %bool2 xor         |
| not         | logical negation              | %bool1 not                |

Functions:

| name         | description                   | example                        |
|--------------|-------------------------------|--------------------------------|
| if           | if then else                  | %bool %val1 %val2 if           |
| substr       | substring                     | %str 1 3 substr                |
| strlen,length| string length                 | %str strlen, %str length       |
| concat       | concatenation                 | %str1 %str2 concat             |
| like         | match pattern                 | %str 'patt%' like              |
| tostring     | convert to string             | %num %base tostring            |
| toint        | convert to integer            | %str %base toint               |
| replace      | replace string                | %str 'pat' 'repl' 1 replace    |
| replace_bre  | replace string using basic RE | %str 'pat' 'bre' 1 replace_bre |
| replace_ere  | replace string using ext. RE  | %str 'pat' 'ere' 1 replace_ere |
| matches_bre  | string matches basic RE       | %str 'pat' 1 matches_bre       |
| matches_ere  | string matches extended RE    | %str 'pat' 1 matches_ere       |
| next         | next integer from sequence    | 'sequence name' next           |

# EXAMPLES #

csv-ls -c name,size,blocks | csv-add-rpn -n space_used -e `"`%blocks 512 *`"` -s
:   list files and real space they use

# SEE ALSO #

**csv-grep-rpn**(1), **csv-show**(1), **csv-nix-tools**(7)
