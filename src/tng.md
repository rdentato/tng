 # *tng* - A simple literate programming tangling tool

 ## Table of Contents
- [*tng* - A simple literate programming tangling tool](#tng---a-simple-literate-programming-tangling-tool)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Syntax](#syntax)
  - [main()](#main)
  - [Parsing the files](#parsing-the-files)
    - [Getting the input files names](#getting-the-input-files-names)
    - [Code chunk buffers](#code-chunk-buffers)
    - [Chunk names](#chunk-names)
    - [Setting the output file.](#setting-the-output-file)
    - [Reading the files](#reading-the-files)
    - [Recognizing Tags](#recognizing-tags)
    - [Handling tags](#handling-tags)
  - [Data structure](#data-structure)
  - [Includes](#includes)
  - [Exceptions](#exceptions)
  - [Debuging \& Errors](#debuging--errors)

 ## Introduction

  This is the *tangle* tool for a very simple literate programming
system called `tng`. I will assume you already know about literate
programming and will just recall some basic concepts. See the
[Wikipedia](https://en.wikipedia.org/wiki/Literate_programming)
article for more information on Literate Programming.

  `tng` deviates a little from the traditional Knuth's WEB style
and has been mainly inspired by a post on
[Kartik Agaram's blog](http://akkartik.name/post/wart-layers).

  I do not subscribe entirely to Kartik's point of view on how to
rearrange the code to make it more easily understood (actually I find 
his way more confusing), but his idea of using the concept of *waypoints*
instead of *macros* for literate programming, immediately resonated with
me as *"the right thing to do"*.

  Beside this, I can say that `tng` is pretty aligned with the traditional
literate programming view: chunks of codes are organized in a way that is
useful for the programmer (and the reader!) and re-organized for the
compiler only when needed (and in a transparent way).

  To me, the key points are:

     - Source code must be readable in their native, unwaved form.
       Waving it (i.e. creating a nice HTML or PDF version) is a plus
       but shouldn't be mandatory.
     - tangling helps keeping close, in the file, elements that are
       logically related so that their interaction can be explained
       (and understood) more easily.

 ## Syntax
  
   While the tool poses no restriction on the syntax, it is assumed that
 the file will retain a structure that makes it a more or less avalid
 source file for the language at hand.

   It might not actually run/compile in its untangled form, but its look 
 should be familiar to the reader and, even more importantly, should not
 confuse editors and IDEs that will still be able to recognize lexical
 element in the code and provide their support.

   Text is supposed to be in *comment* sections.

   The tags are in the form: `(<tag>:<arg>)` and there
 are five of them:

    - `(code:[filename])`    What follows will be included in the generated file.
    - `(text:[filename])`
    - `(after:<waypoint>)`   
    - `(before:<waypoint>)`
    - `(:<waypoint>)`
    - '(:)' an empty waypoint marks the end of a code section
    - '(void:xxx)'  stops interpreation until another '(void:xxx)' is found.

   tags can appear anywhere in a line and the entire line will be
 ignored. Please note that this is means you can't write something like:

   By the way, note the use of the `(void:xxx)` tag and quotes to
 be able to "talk" about tags without having them recognized as such.

   There is no limitation no the way you write your *text* sections.
 Remember that the assumption is that the file will be mostly used in its
 raw, untangled form. Your effort should be to make it clear in that form,
 Here I've used mostly MarkDown syntax since many programmers are already
 familiar with that syntax. 

  ## main()  
  The program logic is almost trivial. The files are parsed to collect the
chunk of codes and then the output file is reassembled.

  To simmplify error handling, we use the exception library `try.h` in
  the `extlibs` directory.

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
    _(":Reassemble chunks into output file")
  }
  _(":Handle errors")

  _(":Cleanup")

}
```

  ## Parsing the files

  ### Getting the input files names

  The files to be parsed are passed on the command line.

  The `filelist[]` array will contain the names of the files
in the order they are specified on the command line.

```C
_("after: Global vars")
char **filelist = NULL;
```

  We simply set `filelist` to first argument in the `argv` array
past all the options.

```
 argv──╮                 ╭─ filelist  ╭─ NULL
       ▼                 ▼            ▼
      ╭──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──╮
      ╰──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──╯
          ╰─────┬──────╯ ╰────┬────╯
             options      filenames
```
 For example if `tng` is invoked as 
``` bash
  ./tng -n -o out src1 src2
```
The first element of `filelist` (i.e. `filelist[0]`) will be "src1"

```C
_("after: CLI action to set the list of files to be parsed.")
vrgarg("filename ...\tThe files to be processed") {
  filelist = &argv[vrgargn-1];
  break;
}

```

  ### Code chunk buffers

  The objective of parsing the input files is to identify the chunk of codes
that must be placed after/before the waypoints.

  Each chunk of code is stored in a buffer that is mapped to 
the waypoint, one buffer for the "before" chunks and one for the
the "after" chunks.

  The main chunk of code is what is left from the input files after having
removed all the text parts and all the after/before code chunks and is stored
in a buffer hold in the `code_chunk` variable.


```C
_("after:Global vars")
val_t code_chunk = valnil;  // The code chunk

_("after:Initialize the variables and data structures.")
code_chunk = bufnew();
if (valisnil(code_chunk)) throw(EX_OUTOFMEM,"");

_("after:Cleanup")
code_chunk = buffree(code_chunk);
```

  The after/before chunks are stored in a map hold
by the variable `chunks`.

```C
_("after:Global vars")
val_t chunks = valnil;      // The after/before chunks

_("after:Initialize the variables and data structures.")
chunks = vecnew();
if (valisnil(chunks)) throw(EX_OUTOFMEM,"");

_("after:Cleanup")
chunks = vecfree(chunks);
bufclearstore(); // free up the stored keys.
```

  To access the buffers, the function `getbuffer()` retrieves 
the buffers related to a waypoint (creating it if it does not already exists).

  The `prefix` variable is `A` for **after**, `B` for **before** and `C` for
**code** (the main chunk). In other words, `getbuffer('A',"Initialize engine")`
will return the buffer containing the text that goes after the `Initializae engine`
waypoint.

```C
_("before: Global vars")
val_t getbuffer(char prefix, char *waypoint);

_("after:Functions")
val_t getbuffer(char prefix, char *waypoint)
{
  val_t cname;
  val_t buffer;
  char *name;

  if (prefix == 'C') return code_chunk;

  _dbgblk {
    char *s = waypoint;
    while (*s && *s != '\x1E') s++;
   _dbgtrc("GET: [%c][%.*s]",prefix,(int)(s-waypoint),waypoint);
  }

  name = to_chunkname(prefix, waypoint);

  buffer = vecget(chunks,name,&cname);
  if (valisnil(buffer)) {
    // Buffer does not exists yet. Let's create it.
    buffer = bufnew();
    if (valisnil(buffer)) throw(EX_OUTOFMEM,"");

    // We need a copy of the chunk name to use as a key because the original
    // string will be overwritten when the next line is read
    cname = bufstore(name);
    if (valisnil(cname)) throw(EX_OUTOFMEM,"");
    vecmap(chunks,cname,buffer);
  }

  _dbgtrc("GOT: [%s]",valtostring(cname));

  _dbgchk(!valisnil(cname));
  _dbgchk(valisstored(cname));

  return buffer;
}
```
  ### Chunk names

  We need to provide some flexibility in the way we spell 
waypooints.

  For example all the commands below should refer to the 
same waypoint:

```
  '(:Initialize the ENGINES!!!)'
  '(after: Initialize the engines)'
  '(after:Initialize  the engines)'
  '(after:Initialize The Engines)'
  '(before: Initialize the ENGINES.)'
```
  This way, the programmer is not forced to remember *exactly*
how a certain waypoint had been defined. Of course one should avoid
using different strings for the same waypoint, but subtle variations
(like the space at the beginning) shoud be accomadated for by the tool.


```C
_("after:Global vars")
char *to_chunkname(char prefix, char *s);

_("after:Functions")
char *to_chunkname(char prefix, char *s)
{
  static char name[128];

  int len = 0;

  name[len++] = prefix;
  name[len++] = '~';
  name[len] = '\0';

  char c;
  while(*s && *s != '\x1E' && len < 120) {
    c = *s++;

    if ('0' <= c && c <= '9') {
      name[len++] = c;
    }
    else if ('A' <= c && c <= 'Z') {
      c += 32; 
      name[len++] = c;
    }
    else if ('a' <= c && c <= 'z') {
      name[len++] = c;
    }
    else if (len > 2) {
      if (name[len-1] == ' ') len--;
      name[len++] = ' ';
    }
    name[len] = '\0';
  }
  while ( len>0 && name[len-1] == ' ') name[--len] = '\0';

  return name;
}
```

  ### Setting the output file.

  By default, `tng` will send the output file to stdout. You can 
  use the CLI option `-o` to specify which file should be created.

```C
_("after: Global vars")
char *out_filename = NULL;
FILE *out_file;

_("after:Initialize the variables and data structures.")
out_file = stdout;
out_filename = NULL;

_("after: CLI action to set the output file name")
vrgarg("-o, --outfile outfilename\tThe output file (defaults to stdout)") {
  out_filename = vrgarg;
  out_file = fopen(out_filename,"wb");
  if (out_file == NULL) vrgerror("Unable to write on '%s'\n",out_filename);
}

_("after:cleanup")
if (out_filename) fclose(out_file);


_("after: Global vars")
val_t cur_buffer = valnil;

```

  ### Reading the files

  Since `filelist` is the end portion of the `argv` array, it is guaranteed
by the C standard (5.1.2.2.1) that the last element of the array is a
pointer to `NULL`.

```C
_("after:Parse input files")
{ char **fname = filelist;
  while (*fname) {
    parsefile(*fname);
    fname++;
  }
}

```
  The files will be read one line at the time. The current line 
stored in the `linebuf` buffer.

  The variable `linenum` carries the current line number.

```C
_("after: Global vars")
val_t linebuf = valnil;
int linenum = 0;

_("after:Initialize the variables and data structures.")
linebuf = bufnew();
if (valisnil(linebuf)) throw(EX_OUTOFMEM, "");

_("after:Cleanup")
linebuf = buffree(linebuf);

_("after:Read a line")
buflen(linebuf,0);
bufloadln(linebuf, source_file);
linenum += 1;

```

  The function `parsefile()` will do all the work of reading
the input files, extracting the chunk of codes and putting them
in the appropriate buffers.

```C
_("after:Functions")
int parsefile(char *file_name)
{
  _(":Parsing variables")

  FILE *source_file; 
  source_file = fopen(file_name,"rb");
  if (!source_file) throw(EX_FILENOTFOUND, file_name);
  
  _dbgtrc("Parsing: %s", file_name);

  cur_buffer = code_chunk;

  while (!feof(source_file)) {
    _(":Read a line")
    _(":Identify tags. Sets tag, tag_arg and tag_indent")
    _(":Handle tags")
  }

  fclose(source_file);
  return 0;
}
```

  ### Recognizing Tags

```C
_("before: Global vars")
#define TAG_NONE     0x00
#define TAG_TEXT     0x01
#define TAG_EMPTY    0x02
#define TAG_CODE     0x03
#define TAG_AFTER    0x04
#define TAG_BEFORE   0x05
#define TAG_VOID     0x06
#define TAG_WAYPOINT 0x07
#define TAG_INVOID   0x08
#define TAG_EXVOID   0x09
#define TAG_TICKS    0x0A

_("after: parsing variables")
int   tag = TAG_NONE;
char *tag_arg = NULL;
int   tag_indent = 0;
char  voidstr[32];  
      voidstr[0] = '\0';
char *bufstr;

_("after:Identify tags. Sets tag, tag_arg and tag_indent")
{
  bufstr = buf(linebuf,0);
 _dbgtrc("ln: [%d] %d '%s'",linenum, code, bufstr);
  _dbgtrc("ln: '%d'",linenum);

  tag = TAG_NONE;
  tag_arg = NULL;
  tag_indent = 0;

  if (voidstr[0] != '\0') { // We are in the void
   _dbgtrc("VOID: %d",linenum);
    _(":Check if we are at the end of void")
  }
  else {
    _(":Look for a tag")
  }
}

_("after:Check if we are at the end of void")
{
  tag = TAG_INVOID;
  char *s = bufstr;
  s = strstr(bufstr,"(void:");
  if (s && s > bufstr && isquote(s[-1])) s=NULL;
  if (s) {
    s += 6;
    _dbgtrc("VOID]: '%s' (%s) len:%d",voidstr,s,(int)strlen(voidstr));
    if (strncmp(voidstr,s,strlen(voidstr)) == 0) {
       voidstr[0] = '\0';
       tag = TAG_EXVOID;
    }
  } 
}

_("after:Look for a tag")
{
  char *s = bufstr;
  
  while (*s && *s != '(') {
    if (isquote(s[0]) && s[1] != 0) s++; // Ignore a '(' if preceded by a quote
    s++;
  }
    
  if (*s == '(') {
    _(": Found a possible tag")
  }
  else {
    _(": Look for three backticks")
  }
}

_("after: Found a possible tag")
{
 _dbgtrc("( found! '%s' code:%d line: %d",s+3,code,linenum);
  tag_indent = s-bufstr;
  while (tag_indent > 0 && !isspace(bufstr[tag_indent-1]))
    tag_indent -= 1;
  
  if (isquote(s[1])) s++; // Ignore a quote right after the '('
  s += 1;
           
       if (*s == ':')                   { tag = TAG_WAYPOINT; s += 1; }
  else if (strncmp("after:",s,6) == 0)  { tag = TAG_AFTER;    s += 6; }
  else if (strncmp("before:",s,7) == 0) { tag = TAG_BEFORE;   s += 7; }
  else if (strncmp("code:",s,5) == 0)   { tag = TAG_CODE;     s += 5; }
  else if (strncmp("text:",s,5) == 0)   { tag = TAG_TEXT;     s += 5; }
  else if (strncmp("void:",s,5) == 0)   { tag = TAG_VOID;     s += 5; }
  
  if (tag != TAG_NONE) {
    _(": Zero terminate the argument")
    if (tag == TAG_WAYPOINT && *tag_arg == '\0') {
      tag = TAG_EMPTY;
      tag_arg = NULL;
    }
  }
}

_("after: Look for three backticks")
{
  s = bufstr;
 _dbgtrc("ticks? '%s'",s);
  while (isspace(*s)) s++;
  if (s[0]=='`' && s[1]=='`' && s[2]=='`') {
   _dbgtrc("3 ticks! '%s' code:%d line: %d",s+3,code,linenum);

    tag = TAG_EMPTY;
    if (!code && s[3] != '\0' && !isspace(s[3])) {
      tag_indent = s-bufstr;
      s+=3;
      tag_arg = s;
      while(*s) s++;
      while(s>tag_arg && isspace(s[-1])) s--;
      *s='\0';
      tag = TAG_CODE;
    }
  }
}

_("after: Zero terminate the argument")
{
  char *e = s;

  while (isspace(*e)) e++;
  tag_arg = e;
  while (*e && *e != ')') e++;
  if (*e) {
    if (isquote(e[-1])) e--; // Ignore a quote right before the ')'
    *e = '\0';
  }
  else { // We didn't find the ')'
    throw(EX_SYNTAXERR,"Unterminated tag");
  }
  _dbgtrc("c: %d E: '%s'",tag,tag_arg);
}
```

  ### Handling tags

  There are two main variables that dictate the state of the handling:

  - `code` : If we are in a code block, we'll collect the lines in the 
             approriate buffer.

  - `voidstr` : If we are "in a void", this string will be non empty.

```C
_("after:Parsing Variables")
int code = 0;

_("after:Handle tags")
switch (tag) {

  case TAG_EMPTY:    code = 0; _dbgtrc("EMPTY (%s)",bufstr); break;
  case TAG_TEXT:     code = 0; _dbgtrc("TEXT"); break;

  case TAG_NONE:     if (code) { 
                       bufputs(cur_buffer,bufstr);
                       bufputs(cur_buffer,"\n");
                       _dbgtrc("CODE: [%s] (buf: %p)",bufstr,(void*)buf(cur_buffer,0));
                     }
                     break;

  case TAG_WAYPOINT: if (code) { 
                        bufprintf(cur_buffer, "\x1B%08d %s\x1E\n",tag_indent, tag_arg);
                        _(": Emit line number")
                        _dbgtrc("WAYPOINT: %s",tag_arg ); 
                     }
                     break;

  case TAG_BEFORE:   cur_buffer = getbuffer('B',tag_arg);
                     _(": Emit line number")
                     code = 1;
                     _dbgtrc("BEFORE '%s'",tag_arg); 
                     break;

  case TAG_AFTER:    cur_buffer = getbuffer('A',tag_arg); 
                     _(": Emit line number")
                     code = 1; 
                     _dbgtrc("AFTER '%s'",tag_arg); 
                     break;

  case TAG_CODE:     cur_buffer = code_chunk;
                     _(": Emit line number")
                     code = 1;
                     _dbgtrc("CODE '%s'",tag_arg);
                     break;

  case TAG_VOID:     if (tag_arg && tag_arg[0]) {
                       strncpy(voidstr,tag_arg,31);
                       voidstr[31] ='\0';
                       _(": Emit line number")
                     }
                     _dbgtrc("VOID[: '%s'",voidstr);
                     break;

  case TAG_INVOID:   bufputs(cur_buffer,bufstr);
                     bufputs(cur_buffer,"\n");
                     break;

  case TAG_EXVOID:   
                     _(": Emit line number")
                     _dbgtrc("X] %d", linenum);
                     break;
}

_("after: Emit line number")
if (!nolinenums) 
  bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);

_("after:Reassemble chunks into output file")
// Start from the main code chunk
reassemble('C',"",0);

_("before: Global Vars")
// Count nesting level to detect infinite loops
int count_out_recur = 0;

_("after:functions")
void reassemble(char prefix, char *name, int indent)
{

  val_t c;
  char *b;
  c = getbuffer(prefix,name);
  if (!valisnil(c)) {
    if (count_out_recur++ > 10) throw(EX_INFINITELOOP);
    b = valtostring(c);
    int n = 1;
    int new_indent;
    char *new_name;
    while (*b) {

      if (*b == '\x1F') b++;
      else if (n && indent > 0) { 
        fprintf(out_file,"%*.s",indent," ");
        n = 0; 
      }

      if (*b == '\x1B') {
        new_indent = strtol(b+1,NULL,10);
        b += 10;
        new_name = b;
        while (*b && *b != '\n') b++;
  
        _dbgtrc("name: '%.*s'",(int)(b-new_name-1),new_name);
  
        reassemble('B',new_name,new_indent);
        reassemble('A',new_name,new_indent);

      }
      else {
        fputc(*b,out_file);
      }

      n = (*b == '\n');
      b++;
    }
    count_out_recur--;
  }
}

```

 ## Data structure

 The chunks will be held in a map

 We'll keep a list of chunks :
   - `C~xxx` one of each file (code)
   - `B~xxx` The text to be inserted before waypoint xxx
   - `A~xxx` The text to be inserted after waypoint xxx

```C
_("after:includes")
#include "val.h"

_(:)
// ## Commmand line interface
// Let's use the `vrg.h` facilities for the CLI
_("after: includes")
#define VRGCLI
#include "vrg.h"

_("after: Global vars")
int nolinenums = 0;

_("after: Command line interface")
vrgcli("version 1.0 (c) by Remo Dentato") {
  vrgarg("-n, --nolinenums\tNo line numbers") {
    nolinenums = 1;
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

```

## Includes
Let's park here all the includes needed.

```C
_("before: includes")
#include <stdio.h>

_("before: Functions")
static inline int isquote(int c)
{
  return (c == '\'' || c == '"' || c == '`');
}

```

 ## Exceptions

   Set up the expetions used

```C
_("after: includes")
#define exception_info char *msg;
#include "try.h"

_("after:Global vars")
try_t catch; // initialize the try/catch macros

_("before: Global vars")
// These are the defined exceptions
#define EX_FILENOTFOUND 1
#define EX_OUTOFMEM     2
#define EX_SYNTAXERROR  3
#define EX_NOFILEBUFFER 4
#define EX_SYNTAXERR    5
#define EX_DUPLICATEBUF 6
#define EX_INFINITELOOP 7

_("after:Handle errors")
catch(EX_FILENOTFOUND) {
  err("File not found: '%s'",exception.msg);
}
catch(EX_NOFILEBUFFER) {
  err("Unable to create a buffer for: '%s'",exception.msg);
}
catch(EX_OUTOFMEM) {
  err("Out of memory");
}
catch(EX_SYNTAXERR) {
  err("Unterminated tag %s:%d",out_filename,linenum);
}
catch(EX_DUPLICATEBUF) {
  err("Duplicate file buffer %s",exception.msg);
}
catch(EX_INFINITELOOP) {
  err("Recursive definition detected");
}
catch() {
  err("Unexpected error");
}

```

 ## Debuging & Errors
  Set the default level of debugging information

```C
_("before:includes")
#if !defined(DEBUG) && !defined(NDEBUG)
#define DEBUG DEBUG_TEST
#endif
_("after:includes")
#include "dbg.h"

_("before:global vars")
#define err(...) (fprintf(stderr,"ERROR: " __VA_ARGS__),fputc('\n',stderr))
```

