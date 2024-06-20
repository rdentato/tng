#include "val.h"
#line 1 "buf.c"
//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef BUF_VERSION
#include "buf_.h"
#endif

#include <stdalign.h>

#ifndef RETURN_IF
#define RETURN_IF valreturnif
#endif

#ifdef _MSC_VER
val_t buf_stores[BUF_STORES_NUM] = {0xFFF80FFFFFFFFFE0, 0xFFF80FFFFFFFFFE0, 0xFFF80FFFFFFFFFE0, 0xFFF80FFFFFFFFFE0};
#else 
val_t buf_stores[BUF_STORES_NUM] = {valnil, valnil, valnil, valnil};
#endif

//val_t buf_stores[BUF_STORES_NUM] = {valnil, valnil, valnil, valnil};
uint8_t buf_stores_cnt[BUF_STORES_NUM] = {0,0,0,0} ;

val_t bufstore_2(int sto, char *s)
{
  val_t v;

  sto &= 3;

  if (!valisbuf(buf_stores[sto])) buf_stores[sto] = bufnew();
  RETURN_IF(!valisbuf(buf_stores[sto]),valnil,EINVAL);

  bufputs(buf_stores[sto],"\0\0\0\0",4);   // Add 4 bytes for the aux

  v.v = VAL_STO_MASK | ((uint64_t)(buf_stores_cnt[sto]) << 40) |  ((uint64_t)sto) << 32 | bufpos(buf_stores[sto]);
  
  bufputs(buf_stores[sto],s);

  uint32_t n = bufpos(buf_stores[sto])+1;
  n = (4 - n % 4) % 4;
  val_dbg("n: %d",n);
  bufputs(buf_stores[sto],"\0\0\0\0",1+n); // Add \0 until next pos will be aligned with 4 bytes.

  return v;
}

void bufclearstore_(int sto)
{
  sto &= 3;
  buf_stores[sto] = buffree(buf_stores[sto]);
  buf_stores_cnt[sto] += 1;
}

// Ensure the buftor is large enough to store a value at index n
static int buf_makeroom(buf_t b, uint32_t n)
{
  uint32_t new_sze;
  char *new_buf;

  errno = 0;
  val_dbg("buf Making room %p (%p)[%d]->[%d]",(void *)b,(void *)(b->buf),b->sze,n);

  RETURN_IF(!b, 0, EINVAL);
  RETURN_IF(n == 0, 1, 0); // success for sure!

  // There should be at least n+1 bytes free:
  n += 1;

  if (n <= b->sze) return 1; // There's enough room

  RETURN_IF(n > BUFMAXNDX,0,ERANGE); // Max number of elments in a buftor reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = b->sze ? b->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1); // ensure is even
  
  RETURN_IF(new_sze <= n,0,ERANGE);

  val_dbg("MKROOM: realloc(%p,%lu) [%u]->[%u]",(b->buf),new_sze ,b->sze, new_sze);
  new_buf = realloc(b->buf, new_sze);
  val_dbg("MKROOM: got(%p,%d) [%d]",new_buf,new_sze ,new_sze);

  RETURN_IF(new_buf == NULL,0,ENOMEM);

  // set the last byte of the buffer to \0;
  new_buf[new_sze-1] = '\0';

  b->buf = new_buf;
  b->sze = new_sze;
  
  return 1;
}

val_t bufnew_(uint32_t sze) {
  errno = 0;
  buf_t b = malloc(sizeof(struct buf_s));
  if (b) {
    b->pos = 0;
    b->end = 0;
    b->sze = sze;
    b->buf = NULL;
    if (!buf_makeroom(b,sze)) {free(b); b = NULL;}
  }
  RETURN_IF(b == NULL, valnil, ENOMEM);
  
  memset(b,0,sizeof(struct buf_s));
  return val(b);
}

val_t buffree(val_t bb)
{
  buf_t b;
  errno = 0;
  RETURN_IF(!valisbuf(bb),bb,EINVAL);
  b = valtocleanpointer(bb); 

  free(b->buf);
  free(b); 
  return valnil;
}

uint32_t bufpos_1(val_t bb)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  return b->pos;    
}

uint32_t bufpos_2(val_t bb, int32_t pos)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (pos >= 0) {
    b->pos =  pos >  b->end ? b->end : pos;
  } else {
    b->pos = -pos >= b->end ? 0 : b->end + pos;
  }
  return b->pos;
}

uint32_t bufputs_3(val_t bb, const char *src, uint32_t len)
{

  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  RETURN_IF(src == NULL, 0, 0);

  buf_t b = valtocleanpointer(bb);

  if (len == 0) len = strlen(src);
  
  if (len > 0) {
    RETURN_IF(!buf_makeroom(b,b->pos+len), 0, ENOMEM);

    memmove(b->buf + b->pos, src, len);

    b->pos += len;
    if (b->pos >= b->end) {
      b->end = b->pos;
      b->buf[b->end] = '\0';
    }
  }

  return len;
}

uint32_t bufprintf(val_t bb, const char *fmt, ...)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  va_list args;
  int len;

  // Determine how much space is needed
  va_start(args, fmt);
  len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  if (len > 0) {
    RETURN_IF(!buf_makeroom(b,b->pos+len), 0, ENOMEM);
    val_dbg("POS[end]: %d", b->buf[b->pos+len]);
    int oldend = b->buf[b->pos+len];
    va_start(args, fmt);
    len = vsnprintf(b->buf+b->pos,len+1,fmt,args);
    va_end(args);
    b->pos += len;
    b->buf[b->pos] = oldend;
    if (b->pos >= b->end) {
      b->end = b->pos;
      b->buf[b->end] = '\0';
    }
  }

  return len;
}

uint32_t bufsize_2(val_t bb, uint32_t sze)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (sze > b->sze) {
    RETURN_IF(!buf_makeroom(b,sze), 0, ENOMEM);
    b->sze = sze;
  }
  return b->sze;
}

char *buf_2(val_t bb, uint32_t start)
{
  RETURN_IF(!valisbuf(bb), NULL, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (start > b->pos) start = b->pos;

  return b->buf? b->buf+start : NULL;
}

uint32_t buflen_2(val_t bb, uint32_t n)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (n < BUFMAXNDX) {
    RETURN_IF(!buf_makeroom(b,n), 0, ENOMEM);
    b->end = n;
    b->pos = b->end;
    if (b->buf) b->buf[b->end] = '\0';
  }

  return b->end;
}

uint32_t bufload_3(val_t bb, uint32_t n, FILE *f)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (n == 0) n = UINT32_MAX;
   
  uint32_t blksize = 128;
 
  if (n<blksize) blksize = n;

  uint32_t l = 0;
  uint32_t r;
  while (!feof(f) && l <= n) {
    RETURN_IF(!buf_makeroom(b,b->pos+blksize), 0, ENOMEM);

    r = fread(b->buf+b->pos,1,blksize,f);
    if (r == 0) break;
    l += r;
    b->pos += r;
    if (b->pos > b->end) {
      b->end = b->pos;
      b->buf[b->pos] = '\0';
    }
  }
  return l;
}


uint32_t bufloadln(val_t bb, FILE *f)
{
  int c;
  char last_c = '\0';
  
  RETURN_IF(!f || feof(f), 0, EINVAL);

  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  uint32_t l = 0;

  while ((c=fgetc(f)) != EOF) {

    if (c == '\n') break;

    RETURN_IF(!buf_makeroom(b,b->pos+32), 0, ENOMEM);

    last_c = b->buf[b->pos];
    b->buf[b->pos] = c;
    b->pos += 1;
    l += 1;
  }
  if (b->pos > 0) {
    if (b->buf[b->pos-1] == '\r') {
      b->buf[b->pos-1] = last_c;
      b->pos -= 1;
      l -= 1;
    }

    if (b->pos >= b->end) {
      b->end = b->pos;
      b->buf[b->pos] = '\0';
    }
  }
  return l;
}

uint32_t bufsave_3(val_t bb, uint32_t n, FILE *f)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (n == 0 || (b->pos + n) > b->end) n = b->end - b->pos;
  if (n > 0) {
    n = fwrite(b->buf + b->pos, 1, n, f);
    b->pos += n;
  }

  return n;
}

uint32_t bufdel_2(val_t bb, uint32_t len)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);
  
  if (b->pos == b->end) return 0;

  if (len == 0) return 0;
  
  if (len >= b->end - b->pos) {
    len = b->end - b->pos;
    b->end = b->pos;
    b->buf[b->end] = '\0';
    return len;
  }

  char *ptr = b->buf + b->pos;
  uint32_t len_to_move = b->end - (b->pos + len);
  memmove(ptr, ptr+len, len_to_move+1);

  b->end -= len;
  b->buf[b->end] = '\0';

  return len;
}

static int makegap(buf_t b, uint32_t l)
{
  uint32_t i = b->pos;

  val_dbg("entering makegap(%d,%d)",i,l);
  RETURN_IF(!buf_makeroom(b, i+l),0,ENOMEM);

  if (i < b->end) {
    /*                     __l__
    **  ABCDEFGH       ABC/-----\DEFGH
    **  |  |    |      |  |      |    |
    **  0  i    len    0  i     i+l   len+l */  
    memmove(b->buf+(i+l), b->buf+i, b->end-i);
  }
  b->pos += l;
  b->end += l;
  b->buf[b->end] = '\0';
  return 1;
}

uint32_t bufins_x(val_t bb, uint32_t len, char *fill)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  uint32_t p = b->pos;

  RETURN_IF(!makegap(b,len), 0, ENOMEM);

  uint32_t k=0;
  while (p < b->pos) {
    b->buf[p++] = fill[k++];
    if (fill[k] == '\0') k = 0;
  }
  return(len);
}

uint32_t bufins_n(val_t bb, uint32_t n)
{
  if (n == 0) return 0;
  else return bufins_x(bb,n,"                                ");
}

uint32_t bufins_s(val_t bb, char *s)
{
  if (s == NULL || *s == '\0') return 0;
  else return bufins_x(bb,strlen(s),s);
}

uint32_t bufsearch_3(val_t bb, char *str, uint32_t start)
{
  RETURN_IF(!valisbuf(bb), BUFNONDX, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (start == BUFMAXNDX) start = b->pos;
 
  RETURN_IF(start >= b->end, BUFNONDX, 0);

  char *heystack = b->buf+start;
  char *found = strstr(heystack,str);

  RETURN_IF(found == NULL, BUFNONDX, 0);

  b->pos = (uint32_t)(found-heystack);
  return b->pos;
}

#line 1 "val.c"
//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef VAL_VERSION
#include "val_.h"
#include "vec_.h"
#include "buf_.h"
#endif

char *valemptystr="\0\0\0";

int valisstored_(val_t v)
{
  if ((v.v & VAL_TYPE_MASK) != VAL_STO_MASK) return 0;

  return ((v.v & 0x0000FF0000000000) >> 40) == buf_stores_cnt[(v.v & 0x0000000300000000) >> 32];
}

int valtype(val_t v)
{
#if 0
  if (valisdouble(v))  return VALDOUBLE;
  if (valisbool(v))    return VALBOOL;
  if (valisnil(v))     return VALNIL;
  if (valispointer(v)) return VALPOINTER;
  if (valisstring(v))  return VALSTRING;
  if (valisvec(v))     return VALVEC; 
  if (valisbuf(v))     return VALBUF; 
  if (valisconst(v))   return VALCONST; 
  if (valisstored(v))  return VALSTORED;
#endif
#if 0
  uint64_t t = v.v & VAL_TYPE_MASK;
  switch (t) {
    case VAL_BUF_MASK: return VALBUF;
    case VAL_VEC_MASK: return VALVEC;
    case VAL_STR_MASK: return VALSTRING;
    case VAL_PTR_MASK: return VALPOINTER;
    case VAL_UNS_MASK: return VALINTEGER;
    case VAL_INT_MASK: return VALINTEGER;
    case VAL_CST_MASK: return VALCONST;
    case VAL_STO_MASK: return VALSTORED;
    case ((uint64_t)0xFFF8000000000000): return (t & 0xF0) == 0xE0 ? VALNIL : VALBOOL;
  }
  if (valisdouble(v))  return VALDOUBLE;
 #endif

  if (val_isdouble(v))  return VALDOUBLE;
  if (val_isnil(v))  return VALNIL;
  if (val_isbool(v)) return VALBOOL;
  return (v.v >> 48);

}

char *valtostring(val_t v)
{
  char *s =NULL;
  uint64_t n;
  val_t b;
  int sto;
  errno = 0;
  switch (valtype(v)) {
    case VALSTRING: s = ((char *)valtopointer(v));  val_dbg("STRING IS STRING OR POINTER (%p) [%s]",(void *)s,s); break; 
    case VALBUF:    s = buf(v,0); val_dbg("STRING IS BUF (%p) [%s]",(void *)s,s); break;
    case VALSTORED: n = v.v & VAL_PAYLOAD;
                    sto = (n & 0x0000000300000000) >> 32;
                    b = buf_stores[sto];
                    if ((n >> 40) == buf_stores_cnt[sto])
                       s = buf(b, (n & 0xFFFFFFFF));
                    break;
  } 
  if (s == NULL) s = valemptystr;
  return s;
}

#if 0
int valcmp(val_t a, val_t b)
{
  double d_a, d_b;
  char *s_a, *s_b;

  int t_a = valtype(a);
  int t_b = valtype(b);

  switch (t_a << 4 | t_b) {

    case VALDOUBLE   << 4 | VALDOUBLE  :   d_a = valtodouble(a);          d_b = valtodouble(b);          goto cmpdbl;
    case VALDOUBLE   << 4 | VALINTEGER :   d_a = valtodouble(a);          d_b = (double)valtointeger(b); goto cmpdbl;
    case VALINTEGER  << 4 | VALDOUBLE  :   d_a = (double)valtointeger(a); d_b = valtodouble(b);          goto cmpdbl;
    cmpdbl:
      return d_a == d_b ? 0 : d_a > d_b ? 1 : -1;
          
    case VALSTRING  << 4 | VALSTRING  :
    case VALSTRING  << 4 | VALBUF     :
    case VALSTRING  << 4 | VALSTORED  :
    case VALBUF     << 4 | VALSTRING  :
    case VALBUF     << 4 | VALBUF     :
    case VALBUF     << 4 | VALSTORED  :
    case VALSTORED  << 4 | VALSTRING  :
    case VALSTORED  << 4 | VALBUF     :
    case VALSTORED  << 4 | VALSTORED  :
      s_a = valtostring(a); s_b = valtostring(b);
      val_dbg("[%s] [%s]",s_a,s_b);
      if (s_a == s_b)  return  0;
      if (s_a == NULL) return -1;
      if (s_b == NULL) return  1;
      return strcmp(s_a, s_b);

    case VALNIL << 4 | VALNIL:  return 0;

    default:
      if (t_a == VALNIL) return -1;
      return a.v == b.v ? 0 : a.v > b.v ? 1 : -1;
  }
  return 1;
}
#endif

int valcmp(val_t a, val_t b)
{
  double d_a, d_b;
  char *s_a, *s_b;

  if (a.v == b.v) return 0;
  if (a.v == valnil.v) return -1;
  if (b.v == valnil.v) return 1;

  uint32_t t_a = valtype(a);
  uint32_t t_b = valtype(b);

  switch (t_a << 16 | t_b) {

    case VALINTEGER  << 16 | VALINTEGER: 
      return valtointeger(a) - valtointeger(b);
    
    case VALSTRING  << 16 | VALSTRING:
    case VALSTRING  << 16 | VALBUF:
    case VALSTRING  << 16 | VALSTORED:
    case VALBUF     << 16 | VALSTRING:
    case VALBUF     << 16 | VALBUF:
    case VALBUF     << 16 | VALSTORED:
    case VALSTORED  << 16 | VALSTRING:
    case VALSTORED  << 16 | VALBUF:
    case VALSTORED  << 16 | VALSTORED:
      s_a = valtostring(a);
      s_b = valtostring(b);
      if (s_a == s_b)  return  0;
      if (s_a == NULL) return -1;
      if (s_b == NULL) return  1;
      return strcmp(s_a, s_b);

    case VALDOUBLE   << 16 | VALDOUBLE:  d_a = valtodouble(a);           d_b = valtodouble(b);   break;
    case VALDOUBLE   << 16 | VALINTEGER: d_a = valtodouble(a);           d_b = (double)valtointeger(b);  break;
    case VALINTEGER  << 16 | VALDOUBLE:  d_a = (double)valtointeger(a);  d_b = valtodouble(b);  break;

    default:
      return a.v > b.v ? 1 : -1;
  }

  return d_a == d_b ? 0 : d_a > d_b ? 1 : -1;
}

char *VALDOUBLE_fmt   = "%f";
char *VALSIGNED_fmt   = "%ld";
char *VALUNSIGNED_fmt = "%lu";
char *VALSTRING_fmt   = "%s";
char *VALCONST_fmt    = "%016X"; 
    
void valprintf(val_t v, FILE *f) 
{
   switch (valtype(v)) {
    case VALBOOL:    fprintf(f,"%s",valeq(v,valtrue)? "true" : "false") ; break;
    case VALNIL:     fprintf(f,"nil") ; break;
    case VALVEC:     fprintf(f,"%p[]",(void *)valtopointer(v)) ; break;
    case VALPOINTER: fprintf(f,"%p",(void *)valtopointer(v)) ; break;
    case VALBUF:
    case VALSTORED:
    case VALSTRING:  fprintf(f,VALSTRING_fmt,valtostring(v)); break;
    case VALDOUBLE:  fprintf(f,VALDOUBLE_fmt,valtodouble(v)); break;
    case VALINTEGER: if (valissigned(v)) fprintf(f,VALSIGNED_fmt,(int64_t)valtointeger(v)); 
                     else fprintf(f,VALUNSIGNED_fmt,(uint64_t)valtointeger(v));
                     break;
    case VALCONST:   fprintf(f,VALCONST_fmt,v.v); break;
    default:         fprintf(f,"%016lX",v.v); break;
   }
}

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)0x811c9dc5)
#define hash fnv_1a
static uint32_t fnv_1a(void *buf, size_t len)
{
    unsigned char *bp = (unsigned char *)buf;
    uint32_t hval = FNV1_32_INIT;
    if (len > 0) {
      while (len--) {
        hval ^= (uint32_t)*bp++;
        hval *= FNV_32_PRIME;
      }
    }
    else {
      while (*bp) {
        hval ^= (uint32_t)*bp++;
        hval *= FNV_32_PRIME;
      }
    }
    return hval;
}


uint32_t valhash_(val_t v)
{
  void *s = valtostring(v);
  uint32_t h;
  if (errno == 0)
    h = hash(s, 0);
  else
    h = hash((void*)&v, sizeof(val_t));

 	h ^= h << 13;
	h ^= h >> 17;
	h ^= h << 5;
  return h;
}

uint32_t val_getaux(val_t v)
{
  errno = 0;
  uint32_t *aux;

  switch(valtype(v)) {
    case VALVEC:    return valtovec(v)->aux;
    case VALBUF:    return valtobuf(v)->aux;
    case VALSTORED: aux = (uint32_t *)valtostring(v);
                    if (aux != (uint32_t *)valemptystr) return aux[-1];
  }
  errno = EINVAL;
  return 0;
}

uint32_t val_setaux(val_t v, uint32_t n)
{
  uint32_t *aux;
  errno = 0;
  switch(valtype(v)) {
    case VALVEC:    return valtovec(v)->aux = n;
    case VALBUF:    return valtobuf(v)->aux = n;
    case VALSTORED: aux = (uint32_t *)valtostring(v);
                    if (aux != (uint32_t *)valemptystr) return aux[-1] = n;
  }
  errno = EINVAL;
  return 0;
}
#line 1 "vec.c"
//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef VEC_VERSION
#include "vec_.h"
#endif

#include <stdalign.h>

#ifndef RETURN_IF
#define RETURN_IF valreturnif
#endif

#define VALS_PER_MAPNODE 4

typedef struct vec_mapnode_s {
  val_t    value;
  val_t    key;
  uint32_t level;
  uint32_t parent;
  uint32_t left;
  uint32_t right;
} vec_mapnode_t;

static_assert(sizeof(vec_mapnode_t) == VALS_PER_MAPNODE * sizeof(val_t));

#define getnode(v,n)   ((vec_mapnode_t *) &((v)->vec[n]))
#define setleft(n,l)   ((n)->left = l)
#define getleft(n)     ((n)->left)
#define setright(n,r)  ((n)->right = r)
#define getright(n)    ((n)->right)
#define getroot(t)     ((t)->fst)
#define setroot(t,n)   ((t)->fst = n)
#define getkey(n)      ((n)->key)
#define getvalue(n)    ((n)->value)
#define setkey(n,k)    ((n)->key = k)
#define setvalue(n,v)  ((n)->value = v)

#define setlevel(n,l)  ((n)->level = l)

#define getlevel(...) VEC_vrg(getlevel_,__VA_ARGS__)
#define getlevel_1(n)    ((n)->level)
#define getlevel_2(v,n)  (getnode(v,n)->level)

#define getparent(...) VEC_vrg(getparent_,__VA_ARGS__)
#define getparent_1(n)   ((n)->parent)
#define getparent_2(v,n)   (getnode(v,n)->parent)

#define setparent(...) VEC_vrg(setparent_,__VA_ARGS__)
#define setparent_2(n,p)   ((n)->parent = p)
#define setparent_3(v,n,p) (getnode(v,n)->parent = p)

#define getnumnodes(v) (((v)->end) / VALS_PER_MAPNODE)
#define incnumnodes(v) ((v)->end += VALS_PER_MAPNODE)
#define decnumnodes(v) ((v)->end -= VALS_PER_MAPNODE)
#define lastnodendx(v) ((v)->end - VALS_PER_MAPNODE)

#define is_emptytree(v) ((v)->end == 0)

#define marknode(n)     ((n)->level |= 0x80000000)
#define unmarknode(n)   ((n)->level &= 0x7FFFFFFF)
#define ismarkednode(n) ((n)->level &  0x80000000)

#define NULLNODE       VECNONDX

static int volatile ZERO = 0;

val_t vecnew() {
  errno = 0;
  vec_t v = malloc(sizeof(struct vec_s));
  
  RETURN_IF(v == NULL, valnil, ENOMEM);
  
  memset(v,0,sizeof(struct vec_s));
  return val(v);
}

static val_t vec_free(val_t vv);

static void freevecs(val_t *v,uint32_t from, uint32_t to)
{
  if (v) for (int k=from; k<to; k++) {
    //val_dbg("free: %016lX %d %d",v[k].v,k,valisownedvec(v[k]));
    if (valisownedvec(v[k])) {
      v[k] = vec_free(v[k]);
    }
  }
}

static val_t vec_free(val_t vv) 
{ 
  vec_t v;
  errno = 0;
  v = (vec_t)valtocleanpointer(vv); 
  //freevecs(v->vec,v->fst,v->end);
  free(v->vec);
  free(v); 
  return valnil;
}

val_t vecfree(val_t vv) {
  errno = 0;
  
  RETURN_IF(!valisvec(vv) || valisownedvec(vv), vv, EINVAL);
  
  return vec_free(vv);
}

// Ensure the vector is large enough to store a value at index n
static int vec_makeroom(vec_t v, uint32_t n)
{
  uint32_t new_sze;
  val_t   *new_vec;

  errno = 0;
  RETURN_IF(!v,0, EINVAL);

  val_dbg("vec Making room %p (%p)[%d]->[%d]",(void *)v,(void *)(v->vec),v->sze,n);

  // There should be at least n+1 slots:
  n += 1;

  if (n <= v->sze) return 1; // There's enough room

  RETURN_IF(n > VECMAXNDX,0,ERANGE); // Max number of elments in a vector reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = v->sze ? v->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1); // ensure is even
  
  RETURN_IF(new_sze <= n,0,ERANGE);

  val_dbg("MKROOM: realloc(%p,%lu) [%u]->[%u]",(v->vec),new_sze * sizeof(val_t),v->sze, new_sze);
  new_vec = realloc(v->vec, new_sze * sizeof(val_t));
  val_dbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  RETURN_IF(new_vec == NULL,0,ENOMEM);

  // set the newly allocated area to 0;
  memset(&(new_vec[v->end]),0,(new_sze-v->sze)*sizeof(val_t));

  v->vec = new_vec;
  v->sze = new_sze;
  
  return 1;
}

int vec_gap_3(val_t vv, uint32_t i, uint32_t l)
{
  vec_t v;
  int n;
  errno = 0;

  RETURN_IF(!valisvec(vv),0,EINVAL);

  v = (vec_t)valtovec(vv);

  if (i == VECNONDX) i = v->end;
  if (l == VECNONDX) l = 1;
  val_dbg("GAP : %d %d",i,l);

  if (i < v->end) {
    n = v->end+l;
    /*                    __l__
    **  ABCDEFGH       ABC-----DEFGH
    **  |  |    |      |  |    |    |
    **  0  i    end    0  i   i+l   end+l
    */
  }
  else {
    n = i+l;
    /*                                           __l__
    **  ABCDEFGH........          ABCDEFGH............
    **  |       |     |           |             |     |
    **  0       end   i           0             i     i+l
    */
  }
  val_dbg("GAP end:%d i:%d l:%d n:%d",v->end,i,l,n);
  if (!vec_makeroom(v, n)) return 0;
  if (i < v->end) {
    memmove(&(v->vec[i+l]),  &(v->vec[i]),  (v->end-i)*sizeof(val_t));
    memset(&(v->vec[i]),0, l * sizeof(val_t));
  }
  v->end = n;
  return 1;
}

val_t *vec(val_t v)
{
  RETURN_IF(!valisvec(v),NULL,EINVAL);
  return ((vec_t)valtocleanpointer(v))->vec;
}

int vectype_2(val_t vv,int type)
{ 
  vec_t v;
  RETURN_IF(!valisvec(vv),VEC_ISNOT,EINVAL);
  v = (vec_t)valtovec(vv);
  if (type >=0 ) v->typ = (type & 0x0F) | (v->typ & 0xF0);
  return (v->typ & 0x0F);
}

uint32_t vecsize_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;
  RETURN_IF(!valisvec(vv),0,EINVAL); 
  v = (vec_t)valtovec(vv);
  RETURN_IF(n != VECNONDX && !vec_makeroom(v,n),0,ENOMEM);
  return v->sze;
}

uint32_t veccount_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;

  RETURN_IF(!valisvec(vv),0,EINVAL);
  
  v = (vec_t)valtovec(vv);

  if (n == 0) {
    v->end = 0;
    v->fst = 0;
    v->flg &= VEC_NOTYPE;
    return 0;
  }

  if (vectype(vv) == VEC_ISMAP) return getnumnodes(v);

  uint32_t first = 0;
  if (vectype(vv) == VEC_ISQUEUE) first = v->fst;

  if (n != VECNONDX && (n < (VECMAXNDX - first)))  {
    n += first;
    RETURN_IF(!vec_makeroom(v, n ), 0, ENOMEM);
    if (n > v->end) v->flg &= ~VEC_SORTED;
    v->end = n;
    if (v->fst > n) v->fst = n;
  }

  return v->end - first;
}

// ADDING VALUES TO VEC

static val_t setval(vec_t v, uint32_t i, val_t x)
{
  // I'm about to overwrite an owned vector. Free it! (unless it is the same!)
  if ((v->fst <= i && i < v->end) && valisownedvec(v->vec[i]))
    if (v->vec[i].v != (x.v | 1)) vec_free(v->vec[i]);
  v->vec[i] = x;
  v->flg &= ~VEC_SORTED;
  return x;
}

val_t vecins_(val_t vv, uint32_t i, val_t x)
{
  vec_t v;

  RETURN_IF(!vecmakegap(vv,i,1),valerror,ENOMEM);

  v = (vec_t)valtocleanpointer(vv);
  return setval(v, i, x);
}

val_t vecset_(val_t vv, uint32_t i, val_t x)
{
  errno = 0;
  RETURN_IF(!valisvec(vv), valerror, EINVAL);

  vec_t v = (vec_t)valtocleanpointer(vv);

  if (i == VECNONDX) i = v->end;
  
  RETURN_IF(!vec_makeroom(v,i), valerror, ENOMEM);

  x=setval(v, i, x);

  if (i >= v->end) v->end = i+1;
  return x;
}

val_t vecown_3_(val_t vv, uint32_t i, val_t x)
{
  if (valeq(x,valown)) x = vecget(vv,i);
  if (valisvec(x)) x.v |= 1;
  return vecset_(vv,i,x);
}

val_t vecdisown(val_t vv, uint32_t i)
{
  val_t x = vecget(vv,i);
 
  if (valisownedvec(x)) {
    //val_dbg("disowned");
    x.v &= ((uint64_t)0xFFFFFFFFFFFFFFFE);
    ((vec_t)valtocleanpointer(vv))->vec[i] = x;
    return x;
  }
  else return x;
}

val_t vecpush_(val_t vv, val_t x, int own) 
{ 
  errno = 0;
  RETURN_IF(!valisvec(vv), valerror, EINVAL);
  vectype(vv, VEC_ISSTACK); 
  if (own) return vecown_3_(vv,VECNONDX,x);
  return vecset_(vv,VECNONDX,x); 
}

val_t vecenq_(val_t vv, val_t x,int own)
{
  vec_t v;
  errno = 0;
  RETURN_IF(!valisvec(vv), valerror, EINVAL);

  vectype(vv,VEC_ISQUEUE);
  v = (vec_t)valtocleanpointer(vv);
  if ((v->fst > (v->end / 2)) && (v->end >= v->sze)) {
    memmove(v->vec, &(v->vec[v->fst]), (v->end - v->fst) * sizeof(val_t));
    v->end -= v->fst;
    v->fst = 0;
  }
  
  if (own) return vecown_3_(vv,VECNONDX,x);
  return vecset_(vv,VECNONDX,x); 
}

#define vec_data(v_) ((v_)->vec)

uint32_t vec_search_map(vec_t v, val_t key, uint32_t *parent_ndx, int *right, int *depth);

val_t vecget_(val_t vv, val_t ii, val_t *key)
{
  vec_t v;
  errno = 0;
  val_t x;
  uint32_t parent_ndx;
  int right;
  int depth;

  RETURN_IF(!valisvec(vv), valnil, EINVAL);
  v = (vec_t)valtovec(vv);

  if (vectype(vv) == VEC_ISMAP) {
    val_dbg("Retrieve from map");
    if (key) *key = valnil;
    uint32_t node_ndx = vec_search_map(v,ii,&parent_ndx,&right,&depth);
    RETURN_IF(node_ndx == NULLNODE, valnil,0);
    vec_mapnode_t *node = getnode(v,node_ndx);
    if (key) *key = getkey(node);
    return getvalue(node);
  }
  else {
    uint32_t i = valtointeger(ii);

    if (i == VECNONDX) i = v->end - 1; // Get the last element
    RETURN_IF(i >= v->end, valnil, ERANGE);

    x = v->vec[i];
  }
  return x;
}

val_t vecgetfirst(val_t vv)
{
  vec_t v;
  errno = 0;
  val_t x;
  uint32_t first = 0;
  int vtype;

  RETURN_IF(!valisvec(vv), valnil, EINVAL);
  v = (vec_t)valtovec(vv);

  RETURN_IF(v->end == 0, valnil, 0);

  vtype = v->flg & 0x0F;
  if (vtype == VEC_ISMAP) {
    return valnil;
  }
  else {
    if (vtype == VEC_ISQUEUE) first = v->fst;
    else if (vtype == VEC_ISSTACK) first = v->end - 1;
    x = v->vec[first];
  }
  return x;
}

val_t vecgetnext(val_t v);
val_t vecgetmin(val_t vv);
val_t vecgetmax(val_t vv);

uint32_t vecdel_3_(val_t vv, val_t ii, uint32_t j)
{
  uint32_t l,i;
  vec_t v;
  errno = 0;
  
  RETURN_IF(!valisvec(vv), 0, EINVAL);
  RETURN_IF(vecisempty(vv), 0, ERANGE);

  v = (vec_t)valtocleanpointer(vv);

  if (vectype(vv) == VEC_ISMAP) return vecunmap_(vv,ii);

  i = valtointeger(ii);

  val_dbg("i = %d, j = %d, end = %d, sze = %d",i,j,v->end,v->sze);
  if (i == VECNONDX) { i = v->end-1; j = i; }
  else if (j == VECNONDX) j = i;
  if (i >= v->end || j<i) return 0;

  if (j >= v->end-1) {
    freevecs(v->vec,i,v->end);
    v->end = i; // Just drop last elements
  }
  else {
    freevecs(v->vec,i,j+1);

    //       __l__
    //    ABCdefghIJKLM              ABCIJKLM
    //    |  |    |    |             |  |    |    
    //    0  i   i+l   end           0  i    end-l  

    l = (j-i)+1;
    memmove(&(v->vec[i]) , &(v->vec[i+l]),  (v->end-(i+l)) * sizeof(val_t));
    v->end -= l;
  }
  val_dbg("i = %d, j = %d, end = %d, sze = %d",i,j,v->end,v->sze);
  return(v->end);
}

uint32_t vecdrop_2(val_t vv, uint32_t n) {
  vec_t v;
  uint32_t cnt;
  errno = 0;

  RETURN_IF(!valisvec(vv), 0, EINVAL);

  cnt = veccount(vv);

  if (n != 0 && cnt != 0) {
    v = (vec_t)valtocleanpointer(vv);
    if (n >= cnt) n = cnt;

    freevecs(v->vec,v->end-n,v->end);

    v->end -= n;
    if (v->fst >= v->end) {
       v->fst = 0; v->end = 0; v->typ = VEC_ISVEC;
    }
    cnt = (v->end);
  }
  return cnt;
}

// Deque (removes the next n elements form the queue)
uint32_t vecdeq_2(val_t vv, uint32_t n) 
{
  uint32_t ret = 0;
  vec_t v;
  errno = 0;

  RETURN_IF(!valisvec(vv), 0, EINVAL);
  RETURN_IF(vectype(vv) != VEC_ISQUEUE, 0, EINVAL);
  
  ret = veccount(vv);

  if (ret > 0) {
    v = (vec_t)valtocleanpointer(vv);
    n += v->fst;
    if (n > v->end) n = v->end;

    freevecs(v->vec, v->fst, n);
    v->fst = n;
    if (v->fst >= v->end) {
      v->fst = 0; v->end = 0;
      v->typ = VEC_ISVEC;
    }
    ret = v->end - v->fst;
  }
  return ret;
}

uint32_t vecindex_3(val_t vv,uint32_t ndx, int32_t delta)
{
  uint32_t ret = VECERRORNDX;
  errno = 0;
  
  RETURN_IF(!valisvec(vv), VECERRORNDX, EINVAL);
  RETURN_IF(vecisempty(vv), VECERRORNDX, EINVAL);

  vec_t v = (vec_t)valtovec(vv);

  if (ndx < VECNONDX)          ret = ndx;
  else if (ndx == VECNONDX)    ret = ndx;
  else if (ndx == VECCOUNTNDX) ret = v->end;
  else if (ndx == VECSIZENDX)  ret = v->sze;
  else switch(vectype(vv)) {
    case VEC_ISSTACK: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECTOPNDX:  ret = v->end-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ret) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = 0; 
                                         if (delta < 0) delta = -delta;
                                         if (delta > v->end-1) ret = VECERRORNDX;
                                         else ret += delta;
                                         break;
    } 
    break;

    case VEC_ISQUEUE: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECTOPNDX:
                        case VECHEADNDX: ret = v->fst;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ((v->end-1) - v->fst)) ret = VECERRORNDX;
                                         else ret += delta;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = v->end-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ((v->end-1) - v->fst)) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;
    } 
    break;

    default: switch(ndx) {
                        case VECNEXTNDX:  ret = v->end;  break;

                        case VECHEADNDX:  
                        case VECFIRSTNDX: ret = 0;  break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = v->end-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ret) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;

                        case VECCURNDX:  ret = v->cur; break;
    }
  }

  return ret;
}

int vec_cmp(const void *a, const void *b)
{
  int ret;
  ret = valcmp(*((val_t *)a),*((val_t *)b));
  val_dbg("comparing %016lX %016lX = %d",((val_t *)a)->v,((val_t *)b)->v, ret);
  return ret;
}

int vecsort_3(val_t vv, vec_cmp_t cmp, val_t aux)
{
  int ret = 0;
  errno = 0;

  RETURN_IF(!valisvec(vv), 1, EINVAL);
  RETURN_IF(vecisempty(vv), 0, 0);

  vec_t v = valtovec(vv);
  val_dbg("About to sort %p[%u]",(void *)v,v->end);

  val_dbg("[0] = %16lX",(v->vec[0]).v);
  qsort(v->vec,v->end,sizeof(val_t),vec_cmp);
  v->flg |= VEC_SORTED;
  return ret;
}

int vecissorted_(val_t vv)
{
  RETURN_IF(!valisvec(vv), 0, EINVAL);
  RETURN_IF(vecisempty(vv), 0, 0);

  vec_t v = valtovec(vv);

  return v->flg & VEC_SORTED;

}

uint32_t vec_search_sorted(vec_t v, val_t key)
{
  val_dbg("Searching in sorted");
  val_t *found = bsearch(&key, v->vec,v->end, sizeof(val_t), vec_cmp);
  RETURN_IF(!found, VECNONDX, 0);
  return (uint32_t)(found - v->vec);
}

uint32_t vec_search_linear(vec_t v, val_t key)
{
  val_dbg("Searching linear");
  for (uint32_t found = 0; found < v->end; found++)
    if (valcmp(v->vec[found], key) == 0) return found;
  return VECNONDX;
}

uint32_t vec_search_map(vec_t v, val_t key, uint32_t *parent_ndx, int *right,int *depth)
{
  uint32_t node_ndx = 0;
  vec_mapnode_t *node;
  *parent_ndx = VECNONDX;
  *right = 0;
  *depth = 0;
  if (!is_emptytree(v)) {
    node_ndx = getroot(v);
    val_dbg("SEARCHIG root: %d",node_ndx);
    while (node_ndx != NULLNODE) {
      node = getnode(v,node_ndx);
      val_dbg("node_ndx: %d left: %d right: %d",node_ndx, getleft(node), getright(node));
      int cmp = valcmp(key, getkey(node));

      if (cmp == 0) {
        val_dbg("FOUND! (%d)",*depth);
        return node_ndx;
      }

      *parent_ndx = node_ndx;
      *depth += 1;

      if (cmp < 0) {
        *right = 0;
        node_ndx = getleft(node);
      } else {
        *right = 1;
        node_ndx = getright(node);
      }
    }
  }
  
  val_dbg("NOT FOUND! (%d)",*depth);
  return VECNONDX;
}

uint32_t vecsearch_(val_t vv, val_t key)
{
  
  RETURN_IF(!valisvec(vv), VECNONDX, EINVAL);
  RETURN_IF(vecisempty(vv), VECNONDX, EINVAL);

  vec_t v = valtovec(vv);
  uint32_t parent_ndx;
  int right;
  int depth;
 
  if (v->flg & VEC_SORTED)
    return vec_search_sorted(v,key);
  else if (vectype(vv) == VEC_ISMAP)
    return vec_search_map(v,key, &parent_ndx, &right, &depth);
 else 
    return vec_search_linear(v,key);
}

#define MAXLEVEL 0xFFFFFFFF

#include <time.h>
#if 1
// Function to generate a random 32-bit number using Xorshift
static uint32_t RND() {
    static uint32_t state = 0; // A non-zero seed value
    if (state == 0) state = 2463534242 * (uint32_t)time(0);
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    // if (x == MAXLEVEL) x -= 1;
    return x & 0x7FFFFFFF;
}
#endif

#if 0
// RNG by burtleburtle http://burtleburtle.net/bob/rand/smallprng.html

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
#define RND_() \
    e = a - rot(b, 27); \
    a = b ^ rot(c, 17);\
    b = c + d;\
    c = d + e;\
    d = e + a;

uint32_t RND( ) {
    static uint32_t a = 0;
    static uint32_t b,c,d;
    uint32_t e;

    if (a == 0) {
      a = 0xf1ea5eed, b = c = d = (uint32_t)time(0);
      for (int i=0; i<20; i++) {
        RND_();
      }
    }
    RND_();
    return d;
}
#endif

#if 0
// Generate 1 random bit (for 50% choices)
static int RNDBIT() {
  static int n = 0;
  static uint32_t state = 0;

  if (n==0) { state = RND(); n = 32; }
  n -= 1;
  return (state & (1<<n));
}
#endif

static uint32_t vec_newmapnode(vec_t v)
{
  uint32_t newnode;
  vec_mapnode_t *node;

  newnode = v->end;
  RETURN_IF(!vec_makeroom(v, newnode+VALS_PER_MAPNODE), VECNONDX, ENOMEM);
  v->end += VALS_PER_MAPNODE;
  node = getnode(v,newnode);

  setleft(node, NULLNODE);
  setright(node, NULLNODE);
  setparent(node, NULLNODE);
  setkey(node, valnil);
  setvalue(node, valnil);
  setlevel(node, RND());

  return newnode;
}

static void moveup(vec_t v, uint32_t node_ndx, uint32_t parent_ndx, int is_right_child)
{
  vec_mapnode_t *parent_node;
  vec_mapnode_t *node;
  uint32_t grandparent;
  vec_mapnode_t *grandparent_node;
  uint32_t chld_ndx;
  int right_next = 0;

  node = getnode(v, node_ndx);
  parent_node = getnode(v,parent_ndx);

  while (parent_ndx != NULLNODE && getlevel(node) >= getlevel(parent_node)) {
    grandparent = getparent(parent_node);
    setparent(node, grandparent);

    if (grandparent != NULLNODE) {
      grandparent_node = getnode(v,grandparent);
      if (getleft(grandparent_node) == parent_ndx) {
        setleft(grandparent_node, node_ndx);
        right_next = 0;
      }
      else {
        setright(grandparent_node, node_ndx);
        right_next = 1;
      }
    }
    else {
      grandparent_node = NULL;
      setroot(v, node_ndx);
      val_dbg("New root: %d", node_ndx);
    }

    if (is_right_child) { // node is parent's right children ROTATE LEFT!
        chld_ndx = getleft(node);
        setright(parent_node, chld_ndx); val_dbg("Setting right children to %d",chld_ndx);
        setleft(node, parent_ndx);
        val_dbg("ROTATED LEFT");
    } else {
        chld_ndx = getright(node); 
        setleft(parent_node, chld_ndx); val_dbg("Setting left children to %d",chld_ndx);
        setright(node, parent_ndx);
        val_dbg("ROTATED RIGHT");
    }
    if (chld_ndx != NULLNODE) setparent(v,chld_ndx,parent_ndx);
    setparent(parent_node, node_ndx);
    parent_ndx = grandparent;
    parent_node = grandparent_node;
    is_right_child = right_next;
  }
}

uint32_t vecmap_(val_t vv, val_t key, val_t value)
{
  RETURN_IF(!valisvec(vv), NULLNODE, EINVAL);
  vec_t v = valtovec(vv);
  uint32_t node_ndx = NULLNODE;
  uint32_t parent_ndx = NULLNODE;
  int is_right_child = 0;
  int depth = 0;
  vec_mapnode_t *node;
  vec_mapnode_t *parent_node;

  int t = vectype(vv);

  if (t == VEC_ISMAP && valisnil(value)) return vecunmap_(vv,key);

  vectype(vv,VEC_ISMAP);

  if (!is_emptytree(v)) {
    node_ndx = vec_search_map(v,key,&parent_ndx,&is_right_child,&depth);
  }
  else val_dbg("INSERT INTO EMPTY TREE");
  
  if (node_ndx == NULLNODE) {
    val_dbg("Adding a new node");
    node_ndx = vec_newmapnode(v);
    RETURN_IF(node_ndx == NULLNODE, NULLNODE, ENOMEM);
  }

  node = getnode(v,node_ndx);
  setkey(node, key);
  setvalue(node, value);
  setparent(node, parent_ndx);

  // Insert the node as a leaf
  if (parent_ndx != NULLNODE) {
    parent_node = getnode(v,parent_ndx);
    if (is_right_child) {
      setright(parent_node, node_ndx);
      val_dbg("MAP INSERT RIGHT parent: %d depth:%d (lvl: %d))",parent_ndx, depth, node_lvl);
    }
    else {
      setleft(parent_node, node_ndx);
      val_dbg("MAP INSERT LEFT parent: %d depth:%d (lvl: %d))",parent_ndx, depth, node_lvl);
    }
  }
  else {
    val_dbg("MAP INSERT ROOT parent: %d depth:%d (lvl: %d))",parent_ndx, depth, node_lvl);
    return node_ndx;
  }

  moveup(v, node_ndx, parent_ndx, is_right_child);

  return node_ndx;
}
      

uint32_t vecunmap_(val_t vv, val_t key)
{
  RETURN_IF(!valisvec(vv), NULLNODE, EINVAL);
  RETURN_IF(vectype(vv) != VEC_ISMAP, NULLNODE, EINVAL);

  vec_t v = valtovec(vv);
  uint32_t node_ndx = NULLNODE;
  uint32_t parent_ndx = NULLNODE;
  int is_right_child = 0;
  int depth = 0;
  vec_mapnode_t *node;
  vec_mapnode_t *parent_node;
  uint32_t left_level = 0, right_level = 0;

  errno = 0;
  
  if (is_emptytree(v)) return NULLNODE;

  node_ndx = vec_search_map(v,key,&parent_ndx,&is_right_child,&depth);

  if (node_ndx == NULLNODE) return NULLNODE;

  // If there is only one node, just remove it
  if (getnumnodes(v) == 1) {
    v->end = 0;
    setroot(v,NULLNODE);
    return NULLNODE;
  }

  node = getnode(v,node_ndx);

#if 0
  if (node_ndx != getroot(v)) {
    setlevel(node,MAXLEVEL);
    moveup(v, node_ndx, parent_ndx, is_right_child);
    val_dbg("Moved up %d",node_ndx);
    vec_printtree(v,stdout);
  }
  else val_dbg("Already at root");
#endif


  // starting from node 
  // if the node is a leaf, remove it and break
  // if the node has a single child, remove it and attach child to its grandparent and breal
  // else, rotate the node (moving it down) so that the children with highest level moves up

  // take the last node and copy it in the freed node.
  uint32_t grandparent_ndx;
  vec_mapnode_t *grandparent_node;
  uint32_t child_ndx;
  vec_mapnode_t *child_node;

  #define set_grandparent() \
      if (grandparent_ndx != NULLNODE) { \
        grandparent_node = getnode(v,grandparent_ndx); \
        if (getright(grandparent_node) == node_ndx) \
          setright(grandparent_node, child_ndx); \
        else \
          setleft(grandparent_node, child_ndx); \
      } \
      else { \
        setroot(v,child_ndx); \
      } \

  while (1) {
    val_dbg("Moving down %d",node_ndx);
    grandparent_ndx = getparent(node);
    
    child_ndx = getleft(node);
    if (child_ndx == NULLNODE) {  // either it's a leaf or only has a right child
      val_dbg("[ No left children");
      child_ndx = getright(node);

      set_grandparent();      

      if (child_ndx != NULLNODE) {  // Node only has a right child
        child_node = getnode(v, child_ndx);
        setparent(child_node, grandparent_ndx);
      }
      else val_dbg("No right children ]");

      break;
    }
    else {
      // Either it has two children or only has the left child
      if (getright(node) == NULLNODE) {
         val_dbg("[ No right children");
        // Only has a left child
  
        set_grandparent();      
  
        child_node = getnode(v, child_ndx);
        setparent(child_node, grandparent_ndx);
        break;
      }
      else {
        // Has both left and right child
        val_dbg("Both children");
        // rotate so that the children with highest level moves up
        left_level = getlevel(v,getleft(node));
        right_level = getlevel(v,getright(node));
        
        if (right_level > left_level)  child_ndx = getright(node); // Rotate left
        else  child_ndx = getleft(node);           // Rotate right

        set_grandparent();      

        child_node = getnode(v,child_ndx);
        setparent(child_node,grandparent_ndx);
        if (right_level > left_level) {
          setright(node,getleft(child_node));
          setleft(child_node,node_ndx);
        }
        else {
          setleft(node,getright(child_node));
          setright(child_node,node_ndx);
        }
        setparent(node,child_ndx);
      }
    }
  }

  // Fix the last node
  child_ndx = v->end - 4; // index of last element
  if (node_ndx != child_ndx) {
    child_node = getnode(v,child_ndx);
    *node = *child_node;
    parent_ndx = getparent(child_node);
    if (parent_ndx == NULLNODE) {
      setroot(v,node_ndx);
    }
    else {
      parent_node = getnode(v,parent_ndx);
      if (getleft(parent_node) == child_ndx)
        setleft(parent_node, node_ndx);
      else
        setright(parent_node, node_ndx);
    }
  }
  
  decnumnodes(v); // Decrement the number of nodes by one

  return 0;
}

void vec_printtree(vec_t v, FILE *f)
{
  fprintf(f,"graph BST {\nnode [shape=circle];\n");
  vec_mapnode_t *node;
  for (int k=0; k < v->end; k+=4) {
    node = getnode(v,k);

    fprintf(f,"%d [label=\"",k);
    valprintf(getkey(node),f);
    fprintf(f,"\n%08X",getlevel(node));
    fprintf(f,"\"];\n");

    fprintf(f,"%d -- ",k);

    if (getleft(node) != NULLNODE) fprintf(f,"%d",getleft(node));
    else fprintf(f,"L%d",k);

    fprintf(f," [label=\"L\"];\n");
    
    fprintf(f,"%d -- ",k);

    if (getright(node) != NULLNODE) fprintf(f,"%d",getright(node));
    else fprintf(f,"R%d",k);

    fprintf(f," [label=\"R\"];\n");

    if (getleft(node) == NULLNODE) fprintf(f,"L%d [shape=point];\n",k);
    if (getright(node) == NULLNODE) fprintf(f,"R%d [shape=point];\n",k);

  }

  fprintf(f,"}\n");
  fflush(f);

}

void vecprinttree(val_t vv, FILE *f)
{
  if (!valisvec(vv)) {errno = EINVAL; return ;};
  vec_t v = valtovec(vv);
  vec_printtree(v,f);
}

val_t vecmapfirst_(val_t vv, val_t *key)
{
  if (!valisvec(vv)) {errno = EINVAL; return valnil ;};
  vec_t v = valtovec(vv);
  
  uint32_t node_ndx = getroot(v);
  vec_mapnode_t *node;
  uint32_t child_ndx;
  while (1) {
    node = getnode(v, node_ndx);
    unmarknode(node);
    child_ndx = getleft(node);
    if (child_ndx == NULLNODE) {
      v->cur = node_ndx;
      marknode(node);
      if (key) *key = getkey(node);
      return (getvalue(node));
    }
    node_ndx = child_ndx;
  }
  return valnil;
}

val_t vecmapnext_(val_t vv, val_t *key)
{
  if (!valisvec(vv)) {errno = EINVAL; return valnil;};

  if (vectype(vv) != VEC_ISMAP) return valnil;

  vec_t v = valtovec(vv);

  if (getnumnodes(v) == 0) return valnil;

  uint32_t node_ndx = getroot(v);
  vec_mapnode_t *node;

  *key = valnil;

  if (v->end == 0 || v->cur >= v->end || v->cur & 0x03) return valnil;
  node_ndx = v->cur;
  node = getnode(v,node_ndx);
  if (getright(node) != NULLNODE) {
    node_ndx = getright(node);
    while (node_ndx != NULLNODE) {
      v->cur = node_ndx;
      node = getnode(v,node_ndx);
      unmarknode(node);
      node_ndx = getleft(node);
    } 
    marknode(node);
    *key = getkey(node);
    return getvalue(node);
  }
  else {
    node_ndx = getparent(node);
    while (node_ndx != NULLNODE) {
      node = getnode(v, node_ndx);
      if (!ismarkednode(node)) {
        v->cur = node_ndx;
        marknode(node);
        *key = getkey(node);
        return getvalue(node);
      }
      node_ndx = getparent(node);
    }
  }

  return valnil;
}
