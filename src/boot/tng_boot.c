#line 4 "tng.md"
  // SPDX-FileCopyrightText: © 2024 Remo Dentato <rdentato@gmail.com>
  // SPDX-License-Identifier: MIT
#line 109 "tng.md"
#line 787 "tng.md"
#include <stdio.h>

#line 849 "tng.md"
#if !defined(DEBUG) && !defined(NDEBUG)
#define DEBUG DEBUG_TEST
#endif
#line 743 "tng.md"
#include "val.h"
#line 751 "tng.md"
#define VRGCLI
#include "vrg.h"

#line 804 "tng.md"
#define exception_info char *msg; int aux;
#include "try.h"

#line 853 "tng.md"
#ifdef DEBUG
#include "dbg.h"
#endif 

#line 110 "tng.md"
#line 226 "tng.md"
val_t getbuffer(char prefix, char *waypoint);

#line 424 "tng.md"
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

#line 705 "tng.md"
int count_out_recur = 0;
#line 815 "tng.md"
// These are the defined exceptions
#define EX_FILENOTFOUND 1
#define EX_OUTOFMEM     2
#define EX_SYNTAXERROR  3
#define EX_NOFILEBUFFER 4
#define EX_SYNTAXERR    5
#define EX_DUPLICATEBUF 6
#define EX_INFINITELOOP 7

#line 858 "tng.md"
#define err(...) (fflush(stdout),fprintf(stderr,"ERROR: " __VA_ARGS__),fputc('\n',stderr))
#line 145 "tng.md"
char **filelist = NULL;
#line 191 "tng.md"
val_t code_chunk;  // The code chunk

#line 205 "tng.md"
val_t chunks;      // The after/before chunks

#line 278 "tng.md"
char *to_chunkname(char prefix, char *s);

#line 323 "tng.md"
char *out_filename = NULL;
FILE *out_file;

#line 342 "tng.md"
val_t cur_buffer;

#line 373 "tng.md"
val_t linebuf;
int linenum = 0;

#line 493 "tng.md"
int global_indent = 0;

#line 755 "tng.md"
int nolinenums = 0;
int buildndx = 0;

#line 808 "tng.md"
try_t catch; // initialize the try/catch macros
#line 111 "tng.md"
#line 790 "tng.md"
static inline int isquote(int c)
{
  return (c == '\'' || c == '"' || c == '`');
}

#line 229 "tng.md"
val_t getbuffer(char prefix, char *waypoint)
{
  val_t cname;
  val_t buffer;
  char *name;

  if (prefix == 'C') return code_chunk;

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

  return buffer;
}
#line 281 "tng.md"
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
      name[len++] = c + 32; // tolower
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
#line 397 "tng.md"
int parsefile(char *file_name)
{
#line 437 "tng.md"
int   tag = TAG_NONE;
char *tag_arg = NULL;
int   tag_indent = 0;
char  voidstr[32];  
      voidstr[0] = '\0';
char *bufstr;

#line 572 "tng.md"
int code = 0;

#line 400 "tng.md"

  FILE *source_file = stdin;
  if (file_name != NULL)  {
    source_file = fopen(file_name,"rb");
    if (!source_file) throw(EX_FILENOTFOUND, file_name);
  }
  
  cur_buffer = code_chunk;

  while (!feof(source_file)) {
#line 384 "tng.md"
buflen(linebuf,0);
bufloadln(linebuf, source_file);
linenum += 1;

#line 411 "tng.md"
#line 445 "tng.md"
{
  bufstr = buf(linebuf,0);

  tag = TAG_NONE;
  tag_arg = NULL;
  tag_indent = 0;

  if (voidstr[0] != '\0') { // We are in the void
#line 461 "tng.md"
{
  tag = TAG_INVOID;
  char *s = bufstr;
  s = strstr(bufstr,"(void:");
  if (s && s > bufstr && isquote(s[-1])) s=NULL;
  if (s) {
    s += 6;
    if (strncmp(voidstr,s,strlen(voidstr)) == 0) {
       voidstr[0] = '\0';
       tag = TAG_EXVOID;
    }
  } 
}

#line 454 "tng.md"
  }
  else {
#line 476 "tng.md"
{
  char *s = bufstr;
  
  while (*s && *s != '(') {
    if (isquote(s[0]) && s[1] != 0) s++; // Ignore a '(' if preceded by a quote
    s++;
  }
    
  if (*s == '(') {
#line 501 "tng.md"
{
  tag_indent = (s-bufstr) * global_indent;
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
#line 545 "tng.md"
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
}
#line 518 "tng.md"
    if (tag == TAG_WAYPOINT && *tag_arg == '\0') {
      tag = TAG_EMPTY;
      tag_arg = NULL;
    }
  }
}

#line 486 "tng.md"
  }
  else {
#line 526 "tng.md"
{
  s = bufstr;
  while (isspace(*s)) s++;

  if (s[0]=='`' && s[1]=='`' && s[2]=='`') {
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

#line 489 "tng.md"
  }
}

#line 457 "tng.md"
  }
}

#line 412 "tng.md"
#line 575 "tng.md"
switch (tag) {

  case TAG_EMPTY:    code = 0; break;
  case TAG_TEXT:     code = 0; break;

  case TAG_NONE:     if (code) { 
                       bufputs(cur_buffer,bufstr);
                       bufputs(cur_buffer,"\n");
                     }
                     break;

  case TAG_WAYPOINT: if (code) { 
                        // Insert a special code in the buffer (0x1B) to signal this is a waypoint
                        bufprintf(cur_buffer, "\x1B%08d %s\x1E\n",tag_indent, tag_arg);
#line 624 "tng.md"
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
#line 590 "tng.md"
                     }
                     break;

  case TAG_BEFORE:   cur_buffer = getbuffer('B',tag_arg);
#line 624 "tng.md"
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
#line 595 "tng.md"
                     code = 1;
                     break;

  case TAG_AFTER:    cur_buffer = getbuffer('A',tag_arg); 
#line 624 "tng.md"
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
#line 600 "tng.md"
                     code = 1; 
                     break;

  case TAG_CODE:     cur_buffer = code_chunk;
#line 624 "tng.md"
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
#line 605 "tng.md"
                     code = 1;
                     break;

  case TAG_VOID:     if (tag_arg && tag_arg[0]) {
                       strncpy(voidstr,tag_arg,31);
                       voidstr[31] ='\0';
                     }
                     break;

  case TAG_INVOID:   bufputs(cur_buffer,bufstr);
                     bufputs(cur_buffer,"\n");
                     break;

  case TAG_EXVOID:   
#line 624 "tng.md"
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
#line 620 "tng.md"
                     break;
}

#line 413 "tng.md"
  }

  if (file_name != NULL) fclose(source_file);
  return 0;
}
#line 640 "tng.md"
void reassemble(char prefix, char *name, int indent)
{

  val_t c;
  char *b;
  c = getbuffer(prefix,name);
  if (!valisnil(c)) {
#line 712 "tng.md"
if (count_out_recur++ > 10) throw(EX_INFINITELOOP,name,prefix);

#line 648 "tng.md"
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
  
        reassemble('B',new_name,new_indent);
        reassemble('A',new_name,new_indent);

      }
      else {
        fputc(*b,out_file);
      }

      n = (*b == '\n');
      b++;
    }
#line 729 "tng.md"
count_out_recur--;
#line 678 "tng.md"
  }
}

#line 112 "tng.md"

int main(int argc, char *argv[]) {

#line 194 "tng.md"
code_chunk = bufnew();
if (valisnil(code_chunk)) throw(EX_OUTOFMEM,"");

#line 208 "tng.md"
chunks = vecnew();
if (valisnil(chunks)) throw(EX_OUTOFMEM,"");

#line 327 "tng.md"
out_file = stdout;
out_filename = NULL;

#line 377 "tng.md"
linebuf = bufnew();
if (valisnil(linebuf)) throw(EX_OUTOFMEM, "");

#line 116 "tng.md"
#line 759 "tng.md"
vrgcli("version 1.0 (c) by Remo Dentato") {
  vrgarg("-n, --nolinenums\tNo line numbers") {
    nolinenums = 1;
  }

#line 331 "tng.md"
vrgarg("-o, --outfile outfilename\tThe output file (defaults to stdout)") {
  out_filename = vrgarg;
  out_file = fopen(out_filename,"wb");
  if (out_file == NULL) vrgerror("Unable to write on '%s'\n",out_filename);
}

#line 765 "tng.md"

#line 167 "tng.md"
vrgarg("[filename ...]\tThe files to be processed. (defaults to stdin)") {
  filelist = &argv[vrgargn-1];
  break;
}
#line 767 "tng.md"
  
#line 496 "tng.md"
vrgarg("-i, --indent\tKeep indentation") {
  global_indent = 1;
}

#line 769 "tng.md"

  vrgarg("-h, --help\tHelp") {
    vrghelp();
    exit(1);
  }

  vrgarg() {
    if (vrgarg[0] == '-') vrgerror("Unknown option '%s'\n",vrgarg);
  }
}

#line 117 "tng.md"

  try {
#line 354 "tng.md"
if (filelist == NULL) {
  parsefile(NULL);
} else {
  char **fname = filelist;
  while (*fname) {
    parsefile(*fname);
    fname++;
  }
}

#line 120 "tng.md"
#line 636 "tng.md"
// Start from the main code chunk
reassemble('C',"",0);

#line 121 "tng.md"
  }
#line 825 "tng.md"
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
#line 715 "tng.md"
catch(EX_INFINITELOOP) {
  char *tag="";
  if (exception.aux == 'A') tag="after";
  else if (exception.aux == 'B') tag="before";
  char *e=exception.msg;
  while (*e && *e != '\n') e++;
  while (e>exception.msg && isspace(e[-1])) e--;
  err("Infinite recursion while expanding: (%s:%.*s)",tag,(int)(e-exception.msg),exception.msg);
}
#line 841 "tng.md"

#line 123 "tng.md"
  catch() {
    err("Unexpected error");
  }

#line 198 "tng.md"
code_chunk = buffree(code_chunk);
#line 212 "tng.md"
chunks = vecfree(chunks);
bufclearstore(); // free up the stored keys.
#line 338 "tng.md"
if (out_filename) fclose(out_file);


#line 381 "tng.md"
linebuf = buffree(linebuf);

#line 128 "tng.md"

  exit(0);
}
#line 144 "tng.md"
#line 166 "tng.md"
#line 190 "tng.md"
#line 204 "tng.md"
#line 225 "tng.md"
#line 277 "tng.md"
#line 322 "tng.md"
#line 353 "tng.md"
#line 372 "tng.md"
#line 396 "tng.md"
#line 423 "tng.md"
#line 571 "tng.md"
#line 635 "tng.md"
#line 704 "tng.md"
#line 711 "tng.md"
#line 728 "tng.md"
#line 742 "tng.md"
#line 750 "tng.md"
#line 786 "tng.md"
#line 803 "tng.md"
#line 814 "tng.md"
#line 848 "tng.md"
