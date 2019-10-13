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

## Syntax
The syntax is inspired by a post on [Kartik Agaram's blog](http://akkartik.name/post/wart-layers).

   - `(code:[filename])`
   - `(text:[filename])`
   - `(after:<waypoint>)`
   - `(before:<waypoint>)`
   - `(:<waypoint>)`

Read the source code of `tangle_lit.sh` for further details
and for a live demonstration of how they work.
