---
title: csv-add-exec
section: 1
...

# NAME #

csv-add-exec - XXX

# SYNOPSIS #

**csv-add-exec** [OPTION]... -- command

# DESCRIPTION #

XXX

-c, --column=name
:   XXX

-n, --new-name=name
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

$ csv-ls -c full_path | csv-add-exec -c full_path -n new -- sed 's/.c$/.o/'

```

# SEE ALSO #

**csv-show**(1), **csv-nix-tools**(7)
