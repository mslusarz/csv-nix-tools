**csv-nix-tools**
=================

[![Build Status](https://github.com/mslusarz/csv-nix-tools/workflows/Full/badge.svg)](https://github.com/mslusarz/csv-nix-tools/actions?query=workflow%3AFull+branch%3Amaster)
[![Coverage Status](https://codecov.io/github/mslusarz/csv-nix-tools/coverage.svg?branch=master)](https://codecov.io/gh/mslusarz/csv-nix-tools/branch/master)

csv-nix-tools is a collection of tools for gathering and processing system
information using CSV as an intermediate format.

# Status
This project is in alpha stage. Everything listed below is functional,
but in the future many things can change (IOW don't run this in production yet).
Currently it builds and runs on Linux with glibc only, but portability patches
are welcomed.

# Dependencies
- **cmake** >= 3.3
- **glibc-devel**
- **pandoc** (optional, required to generate man pages)
- **bison** (optional, required by csv-sql, csv-grep-sql and csv-add-sql)
- **flex** (optional, required by csv-sql, csv-grep-sql and csv-add-sql)
- **libmnl-devel** (optional, required by csv-netstat)
- **libncurses-devel** (optional, used by csv-show)
- **libprocps-devel** (optional, required by csv-ps)
- **libsqlite3-devel** (optional, required by csv-sqlite)

# Installation
```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

# Available tools

Source:
- **csv-env** - lists environment variables
- **csv-groups** - lists system groups
- **csv-group-members** - lists system groups and users that belong to them
- **csv-ls** - lists files
- **csv-netstat** - lists network connections
- **csv-ps** - lists processes (WIP)
- **csv-users** - lists system users

Filtering/processing:
- **csv-add-concat** - adds a new column by concatenation of columns and fixed strings
- **csv-add-exec** - pipes data to standard input of an external command and creates a new column from its standard output
- **csv-add-replace** - adds a new column by performing a string substitution on another column (similar to sed s/$str/$str/)
- **csv-add-rev** - adds a new column by reversing another column characterwise
- **csv-add-rpn** - adds a new column from RPN expression
- **csv-add-split** - adds two new columns by splitting another one using a delimiter
- **csv-add-sql** - adds a new column from SQL expression
- **csv-add-substring** - adds a new column by extracting a substring of another column
- **csv-avg** - takes an average of numerical column(s)
- **csv-cat** - concatenates multiple csv files
- **csv-count** - counts the number of columns and/or rows
- **csv-cut** - removes columns and reorders them
- **csv-grep** - filters rows matching a pattern
- **csv-grep-rpn** - filters rows using RPN expression
- **csv-grep-sql** - filters rows using SQL expression
- **csv-head** - outputs the first N rows
- **csv-header** - processes data header
- **csv-max** - takes a maximum value of numerical or string column(s)
- **csv-merge** - merges multiple input streams
- **csv-min** - takes a minimum value of numerical or string column(s)
- **csv-sort** - sorts input by column(s)
- **csv-sql** - processes input data using simplified (but very fast) SQL-based syntax
- **csv-sqlite** - processes input data using SQLite (requires loading the whole input before processing)
- **csv-sum** - takes a sum of numerical or string column(s)
- **csv-tac** - concatenates files in reverse
- **csv-tail** - outputs the last N rows
- **csv-uniq** - merges adjacent duplicate rows

Sink:
- **csv-exec** - executes an external command for each row
- **csv-show** - formats data in human-readable form (also available as a "-s/-S" option in all source and processing tools)

# Examples

See [EXAMPLES](EXAMPLES.md)

# TODO

See [TODO](TODO.md)
