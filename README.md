# tng

- [tng](#tng)
  - [Introduction](#introduction)
  - [Command line options:](#command-line-options)
  - [Syntax](#syntax)
  - [Single-file distribution.](#single-file-distribution)
  - [Historical note.](#historical-note)
  - [Copyright note (MIT Licence)](#copyright-note-mit-licence)


## Introduction
A simple 'tangle' tool in the Literate Programming spirit.

`tng` is written in `tng` itself (the source code is in `tng.md`),
to bootstrap and compile everything, just go to the `src` directory
and execute the `boot.sh` script:

``` bash
  ~/tng/src$ sh ./boot.sh
```

  This will create the source `tng.c`, the `makefile` and will compile
the latest version of `tng`.

  You can also create a native Windows version installing the MSVC compiler `cl`
and running the script `boot.bat` in the `src` directory.

## Command line options:

```
tng/src$ ./tng -h
USAGE:
  tng [OPTIONS] [filename ...] ...
  version 1.0 (c) by Remo Dentato

ARGUMENTS:
  [filename ...]                The files to be processed. (defaults to stdin)

OPTIONS:
  -n, --nolinenums              No line numbers
  -o, --outfile outfilename     The output file (defaults to stdout)
  -i, --indent                  Keep indentation
  -h, --help                    Help
```

## Syntax

For `tng`, each source files are MarkDown files.

The syntax is inspired by a post on [Kartik Agaram's blog](http://akkartik.name/post/wart-layers).

   - `(code:[filename])`     Start of a code chunk in the main file
   - `(text:[filename])` (or just `(:)`) Start of a text chunk
   - `(after:<waypoint>)`    Start of a code chunk to be placed *after* the waypoint
   - `(before:<waypoint>)`   Start of a code chunk to be placed *before* the waypoint
   - `(:<waypoint>)`         Marks the point where the chunks of code will be placed.
   - `(void:<string>)` suspend interpretation of directives until next `(void:<string>)` (with the *same* string)

The file [`tng.md`](src/tng.md) is a good place to start for further details and for a live demonstration of how they work.

  You can also use quotes *within* the parenthesis to please your editor/IDE.
For example: `(':my way point')` or `("after: my  way  point")`.
(Remember: placing the quote *outside* the parenthesis will avoid them to be interpreted as directives.)

  To take advantage of the MarkDown code highlighters, rather than enclosing chunks of code
between `(code:..)` and `(text:)` commands, you can use the usual *triple backticks* syntax:

```
      ```C
            C code here
      ```

      ``` C
            Not considered as code due to the space after the backticks!!!
      ```
```

If in your text you want to insert a piece of code that do not really belong
to the source code, you can add a space after the backticks. Markdown processors
will still consider it a block of code but `tng` will not!

## Single-file distribution.
  If you want use `tng` for your project, you may want to distribute it along
with your project. 

  To avoid the hassle of adding a bunch of source code files that are not directly
related to your project, you can add just the `tng.c` file in the `distr` directory.

  It can be easily compiled with:

``` bash
  cc -O2 -Wall -o tng tng.c
```

  This will generate the `tng` utility you may need to tangle your own source code.

## Historical note.
The `src/script` directory preserves, for historical reasoons, the first
shell script version of tng:
  - `tangle_boot.sh` to bootstrap the system
  - `tangle_lit.sh`  the literate version of the script above

## Copyright note (MIT Licence)


Copyright 2024 Remo Dentato (rdentato@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
