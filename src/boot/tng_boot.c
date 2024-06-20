  // SPDX-FileCopyrightText: © 2024 Remo Dentato <rdentato@gmail.com>
  // SPDX-License-Identifier: MIT
#include <stdio.h>

#if !defined(DEBUG) && !defined(NDEBUG)
#define DEBUG DEBUG_TEST
#endif
#include "val.h"

#define VRGCLI
#include "vrg.h"

#define exception_info char *msg; int aux;
#include "try.h"

#ifdef DEBUG
#include "dbg.h"
#endif 

val_t getbuffer(char prefix, char *waypoint);

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

int count_out_recur = 0;
// These are the defined exceptions
#define EX_FILENOTFOUND 1
#define EX_OUTOFMEM     2
#define EX_SYNTAXERROR  3
#define EX_NOFILEBUFFER 4
#define EX_SYNTAXERR    5
#define EX_DUPLICATEBUF 6
#define EX_INFINITELOOP 7

#define err(...) (fflush(stdout),fprintf(stderr,"ERROR: " __VA_ARGS__),fputc('\n',stderr))
char **filelist = NULL;
val_t code_chunk = valnil;  // The code chunk

val_t chunks = valnil;      // The after/before chunks

char *to_chunkname(char prefix, char *s);

char *out_filename = NULL;
FILE *out_file;

val_t cur_buffer = valnil;

val_t linebuf = valnil;
int linenum = 0;

int global_indent = 0;

int nolinenums = 0;
int buildndx = 0;

try_t catch; // initialize the try/catch macros
static inline int isquote(int c)
{
  return (c == '\'' || c == '"' || c == '`');
}

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
int parsefile(char *file_name)
{
int   tag = TAG_NONE;
char *tag_arg = NULL;
int   tag_indent = 0;
char  voidstr[32];  
      voidstr[0] = '\0';
char *bufstr;

int code = 0;


  FILE *source_file = stdin;
  if (file_name != NULL)  {
    source_file = fopen(file_name,"rb");
    if (!source_file) throw(EX_FILENOTFOUND, file_name);
  }
  
  cur_buffer = code_chunk;

  while (!feof(source_file)) {
buflen(linebuf,0);
bufloadln(linebuf, source_file);
linenum += 1;

{
  bufstr = buf(linebuf,0);

  tag = TAG_NONE;
  tag_arg = NULL;
  tag_indent = 0;

  if (voidstr[0] != '\0') { // We are in the void
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

  }
  else {
{
  char *s = bufstr;
  
  while (*s && *s != '(') {
    if (isquote(s[0]) && s[1] != 0) s++; // Ignore a '(' if preceded by a quote
    s++;
  }
    
  if (*s == '(') {
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
    if (tag == TAG_WAYPOINT && *tag_arg == '\0') {
      tag = TAG_EMPTY;
      tag_arg = NULL;
    }
  }
}

  }
  else {
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

  }
}

  }
}

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
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
                     }
                     break;

  case TAG_BEFORE:   cur_buffer = getbuffer('B',tag_arg);
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
                     code = 1;
                     break;

  case TAG_AFTER:    cur_buffer = getbuffer('A',tag_arg); 
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
                     code = 1; 
                     break;

  case TAG_CODE:     cur_buffer = code_chunk;
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
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
if (!nolinenums) {
  if (file_name != NULL)
    bufprintf(cur_buffer, "\x1F#line %d \"%s\"\n",linenum+1,file_name);
  else
    bufprintf(cur_buffer, "\x1F#line %d\n",linenum+1);
}
                     break;
}

  }

  if (file_name != NULL) fclose(source_file);
  return 0;
}
void reassemble(char prefix, char *name, int indent)
{

  val_t c;
  char *b;
  c = getbuffer(prefix,name);
  if (!valisnil(c)) {
if (count_out_recur++ > 10) throw(EX_INFINITELOOP,name,prefix);

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
count_out_recur--;
  }
}


int main(int argc, char *argv[]) {

code_chunk = bufnew();
if (valisnil(code_chunk)) throw(EX_OUTOFMEM,"");

chunks = vecnew();
if (valisnil(chunks)) throw(EX_OUTOFMEM,"");

out_file = stdout;
out_filename = NULL;

linebuf = bufnew();
if (valisnil(linebuf)) throw(EX_OUTOFMEM, "");

vrgcli("version 1.0 (c) by Remo Dentato") {
  vrgarg("-n, --nolinenums\tNo line numbers") {
    nolinenums = 1;
  }

vrgarg("-o, --outfile outfilename\tThe output file (defaults to stdout)") {
  out_filename = vrgarg;
  out_file = fopen(out_filename,"wb");
  if (out_file == NULL) vrgerror("Unable to write on '%s'\n",out_filename);
}


vrgarg("[filename ...]\tThe files to be processed. (defaults to stdin)") {
  filelist = &argv[vrgargn-1];
  break;
}
  
vrgarg("-i, --indent\tKeep indentation") {
  global_indent = 1;
}


  vrgarg("-h, --help\tHelp") {
    vrghelp();
    exit(1);
  }

  vrgarg() {
    if (vrgarg[0] == '-') vrgerror("Unknown option '%s'\n",vrgarg);
  }
}


  try {
if (filelist == NULL) {
  parsefile(NULL);
} else {
  char **fname = filelist;
  while (*fname) {
    parsefile(*fname);
    fname++;
  }
}

// Start from the main code chunk
reassemble('C',"",0);

  }
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
  char *tag="";
  if (exception.aux == 'A') tag="after";
  else if (exception.aux == 'B') tag="before";
  char *e=exception.msg;
  while (*e && *e != '\n') e++;
  while (e>exception.msg && isspace(e[-1])) e--;
  err("Infinite recursion while expanding: (%s:%.*s)",tag,(int)(e-exception.msg),exception.msg);
}

  catch() {
    err("Unexpected error");
  }

code_chunk = buffree(code_chunk);
chunks = vecfree(chunks);
bufclearstore(); // free up the stored keys.
if (out_filename) fclose(out_file);


linebuf = buffree(linebuf);


  exit(0);
}
