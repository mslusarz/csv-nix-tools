**csv-nix-tools**
=================

[![Build Status](https://github.com/mslusarz/csv-nix-tools/workflows/Full/badge.svg)](https://github.com/mslusarz/csv-nix-tools/actions?query=workflow%3AFull+branch%3Amaster)
[![Coverage Status](https://codecov.io/github/mslusarz/csv-nix-tools/coverage.svg?branch=master)](https://codecov.io/gh/mslusarz/csv-nix-tools/branch/master)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/21704.svg)](https://scan.coverity.com/projects/mslusarz-csv-nix-tools)

csv-nix-tools is a collection of tools for gathering and processing system
information using CSV as an intermediate format.

Although CSV is in the name of the project and this format is used for
information storage, it's not **that** critical aspect of this project.

What is more important is that it allows **safe** processing of **structured**
data using *NIX-like philosophy by employing pipes and tools with familiar names
(such as grep, sort, etc, see [examples](EXAMPLES.md)). "Safe" processing means
that the handling of special characters, like new lines, is effort-less - it
just works, without bothering users with hacks like IFS.

This project also solves the problem of combining data from multiple sources
into a single stream (see the [tables tutorial]) and processing it in any way
user wants to, including powerful SQL syntax, without leaving the shell.

# Status
This project is in alpha stage. Everything listed below is functional,
but in the future, many things can change (IOW don't run this in production yet).
Currently, it builds and runs on Linux with glibc only, but portability patches
are welcomed. Although the project has NIX in the name and many source tools
are Unix or even Linux-specific, sane portability patches, even for non-Unix
systems, will be accepted.

If you discover any issue in the project or a missing feature, don't hesitate
to file an issue. Any feedback is welcomed.

# Dependencies
- **cmake** >= 3.3
- **glibc-devel**
- **pkg-config**
- **pandoc** (optional, required to generate man pages)
- **bison** (optional, required by csv-sql, csv-grep-sql and csv-add-sql)
- **flex** (optional, required by csv-sql, csv-grep-sql and csv-add-sql)
- **libmnl-devel** (optional, required by csv-netstat)
- **libncursesw-devel** (optional, used by csv-show)
- **libprocps-devel** (optional, required by csv-ps)
- **libsqlite3-devel** (optional, required by csv-sqlite)
- **tmux** (optional, needed by some csv-show tests)

On Debian-based distros you can install all dependencies by:
```sh
apt install build-essential cmake pkg-config flex bison libsqlite3-dev \
            libprocps-dev libncursesw5-dev libmnl-dev pandoc tmux
```

On Fedora-based distros you can install all dependencies by:
```sh
yum install gcc make glibc-devel cmake pkg-config flex bison sqlite-devel \
            procps-ng-devel ncurses-devel libmnl-devel pandoc tmux
```

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
- **[csv-env]** - lists environment variables
- **[csv-groups]** - lists system groups
- **[csv-group-members]** - lists system groups and users that belong to them
- **[csv-ls]** - lists files
- **[csv-lstree]** - list files in hierachical manner (wrapper around **[csv-ls]** and **[csv-tree]**)
- **[csv-netstat]** - lists network connections
- **[csv-ps]** - lists processes (WIP)
- **[csv-pstree]** - list processes in hierachical manner (wrapper around **[csv-ps]** and **[csv-tree]**)
- **[csv-users]** - lists system users

Filtering/processing:
- **[csv-add-concat]** - adds a new column by concatenation of columns and fixed strings
- **[csv-add-exec]** - pipes data to standard input of an external command and creates a new column from its standard output
- **[csv-add-replace]** - adds a new column by performing a string substitution on another column (similar to sed s/$str/$str/)
- **[csv-add-rev]** - adds a new column by reversing another column characterwise
- **[csv-add-rpn]** - adds a new column from RPN expression
- **[csv-add-split]** - adds two new columns by splitting another one using a delimiter
- **[csv-add-sql]** - adds a new column from SQL expression
- **[csv-add-substring]** - adds a new column by extracting a substring of another column
- **[csv-avg]** - takes an average of numerical column(s)
- **[csv-cat]** - concatenates multiple csv files
- **[csv-count]** - counts the number of columns and/or rows
- **[csv-cut]** - removes columns and reorders them
- **[csv-diff]** - compares 2 or more files
- **[csv-grep]** - filters rows matching a pattern
- **[csv-grep-rpn]** - filters rows using RPN expression
- **[csv-grep-sql]** - filters rows using SQL expression
- **[csv-head]** - outputs the first N rows
- **[csv-header]** - processes data header
- **[csv-max]** - takes a maximum value of numerical or string column(s)
- **[csv-merge]** - merges multiple input streams
- **[csv-min]** - takes a minimum value of numerical or string column(s)
- **[csv-sort]** - sorts input by column(s)
- **[csv-sql]** - processes input data using simplified (but very fast) SQL-based syntax
- **[csv-sqlite]** - processes input data using SQLite (requires loading the whole input before processing)
- **[csv-sum]** - takes a sum of numerical or string column(s)
- **[csv-tac]** - concatenates files in reverse
- **[csv-tail]** - outputs the last N rows
- **[csv-tree]** - process hierarchical data
- **[csv-uniq]** - merges adjacent duplicate rows

Sink:
- **[csv-exec]** - executes an external command for each row
- **[csv-plot]** - generates 2D or 3D graph using gnuplot
- **[csv-show]** - formats data in human-readable form (also available as a "-s/-S" option in all source and processing tools)
- **[csv-to-html]** - converts data to HTML
- **[csv-to-json]** - converts data to JSON
- **[csv-to-xml]** - converts data to XML

# Documentation

See the [man pages].

# Examples

See [EXAMPLES](EXAMPLES.md)

# TODO

See [TODO](TODO.md)

[man pages]:         https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-nix-tools.7.html
[tables tutorial]:   https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-tables-tut.7.html

[csv-env]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-env.1.html
[csv-groups]:        https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-groups.1.html
[csv-group-members]: https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-group-members.1.html
[csv-ls]:            https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-ls.1.html
[csv-lstree]:        https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-lstree.1.html
[csv-netstat]:       https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-netstat.1.html
[csv-ps]:            https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-ps.1.html
[csv-pstree]:        https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-pstree.1.html
[csv-users]:         https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-users.1.html

[csv-add-concat]:    https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-concat.1.html
[csv-add-exec]:      https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-exec.1.html
[csv-add-replace]:   https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-replace.1.html
[csv-add-rev]:       https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-rev.1.html
[csv-add-rpn]:       https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-rpn.1.html
[csv-add-split]:     https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-split.1.html
[csv-add-sql]:       https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-sql.1.html
[csv-add-substring]: https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-add-substring.1.html
[csv-avg]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-avg.1.html
[csv-cat]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-cat.1.html
[csv-count]:         https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-count.1.html
[csv-cut]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-cut.1.html
[csv-diff]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-diff.1.html
[csv-grep]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-grep.1.html
[csv-grep-rpn]:      https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-grep-rpn.1.html
[csv-grep-sql]:      https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-grep-sql.1.html
[csv-head]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-head.1.html
[csv-header]:        https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-header.1.html
[csv-max]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-max.1.html
[csv-merge]:         https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-merge.1.html
[csv-min]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-min.1.html
[csv-sort]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-sort.1.html
[csv-sql]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-sql.1.html
[csv-sqlite]:        https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-sqlite.1.html
[csv-sum]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-sum.1.html
[csv-tac]:           https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-tac.1.html
[csv-tail]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-tail.1.html
[csv-tree]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-tree.1.html
[csv-uniq]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-uniq.1.html

[csv-exec]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-exec.1.html
[csv-plot]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-plot.1.html
[csv-show]:          https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-show.1.html
[csv-to-html]:       https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-to-html.1.html
[csv-to-json]:       https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-to-json.1.html
[csv-to-xml]:        https://marcin.slusarz.eu/csv-nix-tools/manpages/csv-to-xml.1.html
