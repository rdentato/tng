# 

## Table of content
- [](#)
  - [Table of content](#table-of-content)
  - [Command Line Interface](#command-line-interface)
  - [Standard Includes](#standard-includes)
  - [Exceptions](#exceptions)

```C 
// 
_(":Includes")
_(":Global vars")
_(":Functions")

int main(int argc, char *argv[]) {

  _(":Initialize the variables and data structures.") 
  _(":Command Line Interface")

  try {
    _(":Parse input files")
    _(":Emit HTML files")
  }
  _(":Handle errors")

  _(":Cleanup")

}
```
## Command Line Interface

```C
_("after: includes")
#define VRGCLI
#include "vrg.h"

_("after: Global vars")
char * default_lang = "";

_("after: Command line interface")
vrgcli("version 1.0 (c) by Remo Dentato") {
  vrgarg("-l, --lang\tThe default language for the do block") {
    default_lang = vrgarg;
  }

  _(":CLI action to set the output file name")

  _(":CLI action to set the list of files to be parsed.")

  vrgarg("-h, --help\tHelp") {
    vrghelp();
    exit(1);
  }

  vrgarg() {
    if (vrgarg[0] == '-') vrgerror("Unknown option '%s'\n",vrgarg);
  }
}

_("after: Global vars")
char **filelist = NULL;

_("after: CLI action to set the list of files to be parsed.")
vrgarg("filename ...\tThe files to be processed") {
  filelist = &argv[vrgargn-1];
  break;
}
```

## Standard Includes

```C 
_("before: includes")
#include <stdio.h>
#include <stdlib.h>

```

## Exceptions
  To simplify error handling, we use the exception library `try.h` in
the `extlibs` directory.

```C
_("before: includes")
#include "try.h"

_("after:Global vars")
try_t catch; // initialize the try/catch macros

```
