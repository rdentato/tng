# 1. Wave for tng

- [1. Wave for tng](#1-wave-for-tng)
  - [1.1. Command Line Interface](#11-command-line-interface)
  - [1.2. Standard Includes](#12-standard-includes)
  - [1.3. Exceptions](#13-exceptions)

```C 
#define _(...)

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
## 1.1. Command Line Interface

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

## 1.2. Standard Includes

```C ("before: includes")
#include <stdio.h>
#include <stdlib.h>

```

## 1.3. Exceptions
  To simplify error handling, we use the exception library `try.h` in
the `extlibs` directory.

```C ("before: includes")
#include "try.h"

_("after:Global vars")
try_t catch; // initialize the try/catch macros

```
