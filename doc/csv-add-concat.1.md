---
title: csv-add-concat
section: 1
...

# NAME #

csv-add-concat - XXX

# SYNOPSIS #

**csv-add-concat** [OPTION]... -- new_name = [%name|str]...

# DESCRIPTION #

XXX

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
$ csv-ls -R -c parent,name . | csv-add-concat full_path = %parent / %name
```

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
