---
title: csv-add-replace
section: 1
...

# NAME #

csv-add-replace - XXX

# SYNOPSIS #

**csv-add-replace** [OPTION]...

# DESCRIPTION #

XXX

-c name
:   XXX

-e regex
:   XXX

-E eregex
:   XXX

-F pattern
:   XXX

-i, --ignore-case
:   XXX

-n new-name
:   XXX

-r replacement
:   XXX

-s, --show
:   print output in table format

-S, --show-full
:   print output in table format with pager

-T, --table=name
:   XXX

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLE #

```
csv-ls -c full_path | csv-add-replace -c full_path -E '(.*)\.c$' -r '%1.o' -n new
```

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
