# tng

## Introduction
A simple 'tangle' tool in the Literate Programming spirit

The `src` directory contains two files:
  - `tangle_boot.sh` to bootstrap the system
  - `tangle_lit.sh`  the literate version of the script above

To build the *final* version of the script (`tng.sh`), use
the `bld` script in the root directory:

```
  ~/tng $ bld
```

To test that everything is working properly use:

```
  ~/tng $ bld test
```

To cleanup:

```
  ~/tng $ bld clean
```

## Syntax
The syntax is inspired by a post on [Kartik Agaram's blog](http://akkartik.name/post/wart-layers).

   - `(code:[filename])`
   - `(text:[filename])`  or just `(:)`
   - `(after:<waypoint>)`
   - `(before:<waypoint>)`
   - `(:<waypoint>)`

Read the source code of `tangle_lit.sh` for further details
and for a live demonstration of how they work.

  Note that you can also use quotes *within* the parenthesis to please your editor/IDE.
For example: `(':my way point')` or `("after: my  way  point")`.
(Remember: placing the quote *outside* the parenthesis will avoid them to be interpreted as directives.)


**WARNING** Even if this version is language independent, it won't work with Python or any 
            other language for which leading blanks are meaningful.
