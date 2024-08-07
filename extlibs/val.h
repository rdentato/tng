// Tue Jun  4 12:37:56 CEST 2024
#line 1 "val_.h"
//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.1.0 Beta

#ifndef VAL_VERSION
#define VAL_VERSION 0x0001000B

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// DBG
#if defined(NDEBUG) && defined(DEBUG)
#undef DEBUG
#endif

#ifdef DEBUG
  #define valdbg(...) (fprintf(stderr,"      INFO|  "),fprintf(stderr, __VA_ARGS__),fprintf(stderr," [%s:%d]\n",__FILE__,__LINE__))
#else
  #define valdbg(...)
#endif
#define val_dbg(...)

#define VAL_cnt(x1,x2,x3,x4,xN, ...) xN
#define VAL_n(...)       VAL_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define VAL_join(x ,y)   x ## y
#define VAL_cat(x, y)    VAL_join(x, y)
#define VAL_vrg(f, ...)  VAL_cat(f, VAL_n(__VA_ARGS__))(__VA_ARGS__)
#define VAL_VRG(f, ...)  VAL_cat(f, VAL_n(__VA_ARGS__))(__VA_ARGS__)

// ## REFERENCES
//
// "Crafting Interpreters" by Robert Nystrom 
// https://craftinginterpreters.com/optimization.html
//
// "NaN boxing or how to make the world dynamic"
// https://piotrduperas.com/posts/nan-boxing
//
// Nanobox implementation by Viktor Söderqvist 
// https://github.com/zuiderkwast/nanbox/blob/master/nanbox.h
//
// Intel® 64 and IA-32 Architectures Software Developer’s Manual vol 1.
// https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol1/o_7281d5ea06a5b67a.html
// (see table 4.3 on pages 90)

//                  ╭─── Quiet NaN
//                  │╭── QNaN Floating-Point Indefinites has 0 fron here up to the end
//                  ▼▼ 
//   x111 1111 1111 1xxx FF FF FF FF FF FF
//    ╰──────┬──────╯    ╰───────┬───────╯
//           │                   │ 
//    12 bits set to 1   Payload (48 bits)
//          7FF8

typedef struct {uint64_t v;} val_t;

typedef struct vec_s *vec_t;
typedef struct buf_s *buf_t;

// Some bitmask
#define VAL_TYPE_MASK ((uint64_t)0xFFFF000000000000)
#define VAL_NAN_MASK  ((uint64_t)0x7FF0000000000000)
#define VAL_PAYLOAD   ((uint64_t)0x0000FFFFFFFFFFFF)

#define VAL_BUF_MASK  ((uint64_t)0xFFFE000000000000)
#define VAL_VEC_MASK  ((uint64_t)0xFFFF000000000000)

#define VAL_STR_MASK  ((uint64_t)0x7FFC000000000000)
#define VAL_PTR_MASK  ((uint64_t)0x7FFD000000000000)
#define VAL_UNS_MASK  ((uint64_t)0x7FFE000000000000)
#define VAL_INT_MASK  ((uint64_t)0x7FFF000000000000)
#define VAL_CST_MASK  ((uint64_t)0xFFF9000000000000)
#define VAL_BOL_MASK  ((uint64_t)0xFFF8FFFFFFFFFF00)
#define VAL_STO_MASK  ((uint64_t)0x7FF9000000000000)

#define VALDOUBLE   0x0001
#define VALBOOL     0x0002
#define VALNIL      0x0004
#define VALSTORED   0x7FF9
#define VALSTRING   0x7FFC
#define VALPOINTER  0x7FFD
#define VALINTEGER  0x7FFF
#define VALCONST    0xFFF9
#define VALBUF      0xFFFE
#define VALVEC      0xFFFF
#define VALPREDEF   0xFFF8


// Check the type of a val_t variable
#define val_isbool(x)    (((x).v & (uint64_t)0xFFFFFFFFFFFFFFFE) == VAL_BOL_MASK )
#define val_isdouble(x)  (((x).v & VAL_NAN_MASK)  != VAL_NAN_MASK)
#define val_isnil(x)     ((x).v == valnil.v)

#define valisinteger(x) ((val(x).v & (uint64_t)0xFFFE000000000000) == VAL_UNS_MASK)
#define valissigned(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_INT_MASK)
#define valisbool(x)    ((val(x).v & (uint64_t)0xFFFFFFFFFFFFFFFE) == VAL_BOL_MASK )
#define valisdouble(x)  ((val(x).v & VAL_NAN_MASK)  != VAL_NAN_MASK)
#define valispointer(x) ((val(x).v & VAL_TYPE_MASK) == VAL_PTR_MASK)
#define valisstring(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_STR_MASK)
#define valisvec(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_VEC_MASK)
#define valisbuf(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_BUF_MASK)
#define valisstored(x)  valisstored_(val(x))
int valisstored_(val_t x);

#define valisownedvec(x)  ((val(x).v & (VAL_TYPE_MASK | 1)) == (VAL_VEC_MASK | 1))
#define valisownedbuf(x)  ((val(x).v & (VAL_TYPE_MASK | 1)) == (VAL_BUF_MASK | 1))

#define valisnil(x)     (val(x).v == valnil.v)
#define valiszero(x)    ((val(x).v & VAL_PAYLOAD) == 0)
#define valiserror(x)   (val(x).v == valerror.v)

// Store a value into a val_t variable
static inline val_t val_fromchar(char v)             {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromuchar(unsigned char v)   {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromint(int v)               {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromuint(unsigned int v)     {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromshort(short v)           {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromushort(unsigned short v) {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromlong(long v)             {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromulong(unsigned long v)   {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_frombool(_Bool v)            {val_t ret; ret.v = VAL_BOL_MASK | ((uint64_t)((v) & 1));          return ret;}
static inline val_t val_fromptr(void *v)             {val_t ret; ret.v = VAL_PTR_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromstr(void *v)             {val_t ret; ret.v = VAL_STR_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromvec(void *v)             {val_t ret; ret.v = VAL_VEC_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_frombuf(void *v)             {val_t ret; ret.v = VAL_BUF_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}

static inline val_t val_fromdouble(double v)         {val_t ret; memcpy(&ret,&v,sizeof(val_t)); return ret;}
static inline val_t val_fromfloat(float f)           {return val_fromdouble((double)f);}

static inline val_t val_fromval(val_t v)             {return v;}

#define val_(x) _Generic((x), int: val_fromint,    \
                             char: val_fromchar,   \
                            short: val_fromshort,  \
                             long: val_fromlong,   \
                            _Bool: val_frombool,   \
                     unsigned int: val_fromuint,   \
                    unsigned char: val_fromuchar,  \
                   unsigned short: val_fromushort, \
                    unsigned long: val_fromulong,  \
                           double: val_fromdouble, \
                            float: val_fromfloat,  \
                            val_t: val_fromval,    \
                            vec_t: val_fromvec,    \
                            buf_t: val_frombuf,    \
                  unsigned char *: val_fromstr,    \
                           char *: val_fromstr,    \
                           void *: val_fromptr,    \
                          default: val_fromptr) (x)

#define val(x) _Generic((x), val_t: x, default: val_(x))

#define valeq(x,y)      valeq_(val(x),val(y))

static inline int valeq_(val_t x, val_t y)
{
  if (x.v == y.v) return 1;
  if (valisinteger(x))
    return (x.v & ((uint64_t)0xFFFEFFFFFFFFFFFF)) == (y.v & ((uint64_t)0xFFFEFFFFFFFFFFFF));
  return 0;
}

#define valpayload(x) (val(x).v & VAL_PAYLOAD)

// Retrieve a value from a val_t variable
#define valtodouble(v) val_todouble(val(v))
static inline   double val_todouble(val_t v)  { double d; memcpy(&d,&v,8); return d;}

#define valtofloat(v) val_tofloat(val(v))
static inline  float  val_tofloat(val_t v)    { return (float)val_todouble(v);}

#define valtobool(v) val_tobool(val(v))
static inline  _Bool val_tobool(val_t v)      { return (_Bool)((v.v)&1);}

#define VAL_SIGNED_MASK ((uint64_t)0x0001800000000000)
#define valtointeger(x) val_tointeger(val(x))
static inline      int64_t val_tointeger(val_t v) \
                                  { int64_t ret = ((v.v) & VAL_PAYLOAD); 
                                    if ( ((v.v) & VAL_SIGNED_MASK) == VAL_SIGNED_MASK) // if signed and negative
                                       ret |= (uint64_t)0xFFFF000000000000;            // then extend sign
                                    return ret;
                                  }

static inline   void *val_topointer(val_t v, uint64_t mask) { return (void *)((uintptr_t)((v.v) & mask));}

#define val_to_pointer(v_, m_) ((void *)((uintptr_t)((val(v_).v) & (m_))))

#define valtopointer(v) val_to_pointer(v,VAL_PAYLOAD)
#define valtocleanpointer(v) ((void *)val_to_pointer(v,(uint64_t)0x0000FFFFFFFFFFFE))

#define valtovec(v)    ((vec_t)valtocleanpointer(v))
#define valtobuf(v)    ((buf_t)valtocleanpointer(v))

char *valtostring(val_t v);

// Some constant
#define valfalse      ((val_t){0xFFF8FFFFFFFFFF00})
#define valtrue       ((val_t){0xFFF8FFFFFFFFFF01})
#define valnil        ((val_t){0xFFF80FFFFFFFFFE0})

#define valundefined  ((val_t){0xFFF80FFFFFFFFFD0})
#define valerror      ((val_t){0xFFF80FFFFFFFFFD1})
#define valdeleted    ((val_t){0xFFF80FFFFFFFFFC0})
#define valempty      ((val_t){0xFFF80FFFFFFFFFB0})
#define valmarked     ((val_t){0xFFF80FFFFFFFFFE1})
#define valnewvec     ((val_t){0xFFF80AAAAAAAAA01})
#define valnewbuf     ((val_t){0xFFF80AAAAAAAAA02})
#define valown        ((val_t){0xFFF80AAAAAAAAAA1})
#define valdisown     ((val_t){0xFFF80AAAAAAAAAA2})
#define valnilpointer ((val_t){0xFFFD000000000000})


#define val_first(x,...) x
#define val_rest(x,...) __VA_ARGS__

#define valconst(...) valconst_(val_first(__VA_ARGS__),val_rest(__VA_ARGS__) +0)
#define valconst_(x,y)  ((val_t){((0xFFF9000000000000 + (y) * 0x1000000000000)) | ((uint64_t)((uintptr_t)(x)) & VAL_PAYLOAD)})

#define valisconst(...) valisconst_(val_first(__VA_ARGS__),val_rest(__VA_ARGS__) +0)
#define valisconst_(x,y) ((val(x).v & (uint64_t)0xFFFF000000000000) == ((uint64_t)0xFFF9000000000000 + (y) * 0x1000000000000))

//#define valconst(x)   ((val_t){0xFFF9000000000000 | ((uint64_t)((uintptr_t)(x)) & VAL_PAYLOAD)})
//#define valisconst(x) ((val(x).v & (uint64_t)0xFFFF000000000000) == ((uint64_t)0xFFF8000000000000))

extern char *valemptystr;

#define valnilstr   val_nilstr()
static inline val_t val_nilstr() {return val_fromstr(valemptystr);}

extern char *VALDOUBLE_fmt  ;
extern char *VALSIGNED_fmt  ;
extern char *VALUNSIGNED_fmt;
extern char *VALSTRING_fmt  ;
extern char *VALCONST_fmt   ;
#define valfmt(t,s) (VAL##t##_fmt = s)

void valprintf(val_t v, FILE *f);

int  valtype(val_t v);
int  valcmp(val_t a, val_t b);

#define valreturnif(x,r,e) if (!(x)) errno=0; else return (errno = (e), r)

#define valaux(...) VAL_vrg(valaux_,__VA_ARGS__)
#define valaux_1(v)   val_getaux(v)
#define valaux_2(v,n) val_setaux(v,n)

uint32_t val_getaux(val_t v);
uint32_t val_setaux(val_t v, uint32_t n);

#define valhash(x) valhash_(val(x))
uint32_t valhash_(val_t v);

#endif
#line 1 "buf_.h"
//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef BUF_VERSION
#define BUF_VERSION 0x0001000B

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>

#ifndef VAL_VERSION
#include "val_.h"
#endif

#define BUF_cnt(x1,x2,x3,x4,xN, ...) xN
#define BUF_n(...)       BUF_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define BUF_join(x ,y)   x ## y
#define BUF_cat(x, y)    BUF_join(x, y)
#define BUF_vrg(f, ...)  BUF_cat(f, BUF_n(__VA_ARGS__))(__VA_ARGS__)
#define BUF_VRG(f, ...)  BUF_cat(f, BUF_n(__VA_ARGS__))(__VA_ARGS__)

typedef struct buf_s {
  char    *buf;
  uint32_t sze;
  uint32_t end;
  uint32_t pos;
  uint32_t aux;
} *buf_t;

extern int64_t bufallocatedmem;

#define BUFNONDX   0xFFFFFFFF
#define BUFLOADALL 0xFFFFFFF1
#define BUFLOADLN  0xFFFFFFF2
#define BUFMAXNDX  0xFFFFFFF0

#define bufnew(...) bufnew_(__VA_ARGS__ +0)
val_t bufnew_(uint32_t sze);
val_t buffree(val_t bb);

uint32_t bufsize(val_t bb, uint32_t sze);

#define bufpos(...) BUF_vrg(bufpos_,__VA_ARGS__)
uint32_t bufpos_1(val_t bb);
uint32_t bufpos_2(val_t bb, int32_t pos);

#define buflen(...) BUF_vrg(buflen_,__VA_ARGS__)
#define buflen_1(bb) buflen_2(bb,BUFMAXNDX)
uint32_t buflen_2(val_t bb, uint32_t len);

#define bufsize(...) BUF_vrg(bufsize_,__VA_ARGS__)
#define bufsize_1(bb) bufsize_2(bb,0)
uint32_t bufsize_2(val_t bb, uint32_t sze);

uint32_t bufprintf(val_t bb, const char *fmt, ...);

#define bufputs(...) BUF_vrg(bufputs_,__VA_ARGS__)
#define bufputs_2(bb,s) bufputs_3(bb,s,0)
uint32_t bufputs_3(val_t bb, const char *src, uint32_t len);

#define bufload(...) BUF_vrg(bufload_,__VA_ARGS__)
#define bufload_1(bb) bufload_3(bb,0,stdin)
#define bufload_2(bb, f) bufload_3(bb,0,f)
uint32_t bufload_3(val_t bb, uint32_t n, FILE *f);

uint32_t bufloadln(val_t bb, FILE *f);

#define bufsave(...) BUF_vrg(bufsave_,__VA_ARGS__)
#define bufsave_1(bb) bufsave_3(bb,0,stdout)
#define bufsave_2(bb,f) bufsave_3(bb,0,f)
uint32_t bufsave_3(val_t bb, uint32_t n, FILE *f);

uint32_t bufsaveln(val_t bb, FILE *f);

#define buf(...) BUF_vrg(buf_,__VA_ARGS__)
#define buf_1(b) buf_2(b,BUFMAXNDX)
char *buf_2(val_t bb, uint32_t start);

#define bufdel(...) BUF_vrg(bufdel_,__VA_ARGS__)
#define bufdel_1(bb) bufdel_2(bb,BUFMAXNDX)
uint32_t bufdel_2(val_t bb, uint32_t len);

uint32_t bufins_n(val_t bb,uint32_t len);
uint32_t bufins_s(val_t bb,char *str);

#define bufins(b,x) _Generic((x),          char *: bufins_s, \
                                  unsigned char *: bufins_s, \
                                           void *: bufins_s, \
                                              int: bufins_n, \
                                     unsigned int: bufins_n, \
                                            short: bufins_n, \
                                   unsigned short: bufins_n, \
                                             long: bufins_n, \
                                    unsigned long: bufins_n  \
                            ) (b,x)

#define bufsearch(...) BUF_vrg(bufsearch_,__VA_ARGS__)
#define bufsearch_2(b,s) bufsearch_3(b,s,BUFMAXNDX)
uint32_t bufsearch_3(val_t buf, char *str, uint32_t start);

#define BUF_STORES_NUM 4

extern val_t buf_stores[BUF_STORES_NUM];
extern uint8_t buf_stores_cnt[BUF_STORES_NUM];

#define bufstore(...) BUF_vrg(bufstore_,__VA_ARGS__)
#define bufstore_1(v) bufstore_2(0,v)
val_t bufstore_2(int sto, char *s);

#define bufclearstore(...) bufclearstore_(__VA_ARGS__ + 0)
void bufclearstore_(int sto);

#define bufclearstores() for (int sto=0; sto<BUF_STORES_NUM; sto++) bufclearstore_(sto)

#endif
#line 1 "vec_.h"
//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.4.0 Beta

#ifndef VEC_VERSION
#define VEC_VERSION 0x0004000B

/* ## Index
 *  ### Managing vectors
 *    - vecnew()
 *    - vecfree()
 *    - vecclear()
 *    - vecsize()
 *    - veccount()
 *    - vec()      Returns the array of values
 * 
 *  ### Array style
 *    - vecset(vec, ndx, val)
 *    - vecget(vec, ndx)
 *    - vecins(vec,ndx,val)
 *    - vecdel(vec,from,to)
 * 
 *  ### Stack style
 *    - vecpush(vec, val)
 *    - vectop(vec)
 *    - vecdrop(vec)
 *
 *  ### Queue style
 *    - vecenq(vec, val)
 *    - vecdeq(vec)
 *    - vecdrop(vec)
 *    - vecfirst(vec)
 *    - veclast(vec)
 * 
 *  ### List style
 *    - vechead
 *    - vectail
 *    - veccur
 *    - vecnext
 *    - vecprev
 *    - vecinsnext(vec,val)
 *    - vecinsprev(vec,val)
 *    - vecadd()
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>

#ifndef VAL_VERSION
#include "val_.h"
#endif

#define VEC_ISOWNED ((uint8_t)0x80)
#define VEC_SORTED  ((uint8_t)0x10)
#define VEC_ISVEC   ((uint8_t)0x00)
#define VEC_ISARRAY ((uint8_t)0x01)
#define VEC_ISQUEUE ((uint8_t)0x02)
#define VEC_ISSTACK ((uint8_t)0x03)
#define VEC_ISLIST  ((uint8_t)0x04)
#define VEC_ISMAP   ((uint8_t)0x05)
#define VEC_ISNOT   ((uint8_t)0x0F)
#define VEC_NOTYPE  ((uint8_t)0xE0)

#define VECTOPNDX    0xFFFFFFFE
#define VECHEADNDX   0xFFFFFFFD
#define VECTAILNDX   0xFFFFFFFC
#define VECFIRSTNDX  0xFFFFFFFB
#define VECLASTNDX   0xFFFFFFFA
#define VECCURNDX    0xFFFFFFF9
#define VECNEXTNDX   0xFFFFFFF8
#define VECPREVNDX   0xFFFFFFF7
#define VECCOUNTNDX  0xFFFFFFF4
#define VECSIZENDX   0xFFFFFFF3
#define VECNONDX     0xFFFFFFF2
#define VECERRORNDX  0xFFFFFFF1
#define VECMAXNDX    0xFFFFFFF0

#define VEC_cnt(x1,x2,x3,x4,xN, ...) xN
#define VEC_n(...)       VEC_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define VEC_join(x, y)   x ## y
#define VEC_cat(x, y)    VEC_join(x, y)
#define VEC_vrg(f, ...)  VEC_cat(f, VEC_n(__VA_ARGS__))(__VA_ARGS__)
#define VEC_VRG(f, ...)  VEC_cat(f, VEC_n(__VA_ARGS__))(__VA_ARGS__)

/**
 * @struct vec_s
 *
 * @brief A structure representing a dynamic vector.
 *
 * `vec_s` is designed to represent a dynamic array, providing functionalities
 * to manage a collection of elements with dynamic resizing, while keeping 
 * track of various relevant indices and state-related flags.
 *
 * @typedef vec_t
 * A pointer to a `vec_s` structure, used in API functions to manipulate vectors.
 *
 * @param vec Pointer to an array of `val_t` elements, storing the actual content of the vector.
 *        The array is dynamically allocated and resized as needed.
 *
 * @param sze A `uint32_t` value representing the total capacity of the vector, 
 *        i.e., the maximum number of `val_t` elements that `vec` can currently hold.
 *
 * @param end A `uint32_t` value representing the current end of the vector, 
 *        i.e., the index of the element next to the last one in the vector.
 *
 * @param fst A `uint32_t` value used as an index or pointer to the first element in `vec`.
 *            It is used internally in various data structures (e.e. queues).
 *
 * @param aux A `uint32_t` value used by the valaux() function.
 *
 * @param cur A `uint32_t` value used as an index or pointer to the current element in `vec`.
 *        This may be used to keep track of the element being accessed in the most recent operation.
 *
 * @param flg A `uint16_t` value used to store flags that provide additional information about the state of the vector,
 *        such as whether it is sorted, reversed, or has other special properties.
 *
 * @param typ A `uint8_t` value used to store the type of data structure contained in the vector, 
 *        which may be utilized to validate or identify the type of data structures being used or manipulated.
 *
 * @param opt A `uint8_t` value to be used freely
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew(); // Utilizing the vecnew function as described previously
 *     // Perform operations using my_vector...
 * @endcode
 *
 */
typedef struct vec_s {
        val_t *vec;
      uint32_t sze;
      uint32_t end;  // Pointer to the element past the last one
      uint32_t fst;  // Pointer to the first element
      uint32_t cur;  // Pointer to the current element
      uint32_t aux;  // auxiliary value
      uint16_t flg;  // Flags
       uint8_t typ;  // Type of structure
       uint8_t opt;  
} *vec_t;

/**
 * @fn val_t vecnew()
 * 
 * @brief Create and initialize a new vector.
 *
 * The `vecnew` function is intended to create a new object of type `val_t` which represents a vector.
 * It initializes any necessary properties of the vector, such as its size, 
 * capacity, and any internal data structures used for storing elements of type `struct vec_s`.
 *
 * @return A `val_t` value holding the `vec_t` value for a new vector, initialized and ready for use.
 *         If the vector creation fails (for instance, due to memory allocation issues), 
 *         the function should return valnil and errno is set to ENOMEM.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     if (!valisnil(my_vector)) {
 *         // Handle error (e.g., due to failed memory allocation)
 *     }
 * @endcode
 *
 * Note: Users should check the return value to ensure that the vector was 
 *       created successfully before attempting to use it. This might involve 
 *       checking that the returned value is not valnil and errno is not ENOMEM.
 */
val_t vecnew();

/**
 * @fn val_t vecfree(val_t v)
 * 
 * @brief Safely deallocates the memory occupied by a vector and its internal array.
 *
 * The `vecfree` function is developed to securely release the memory 
 * allocated for a vector `v` of type `val_t`, as well as the memory 
 * for its internal array (`v->vec`). This helps to prevent memory leaks 
 * and ensures the graceful management of vector memory throughout the application.
 *
 * @param v A vector of type `val_t` to be deallocated. 
 *
 * @return Always returns vecnil, facilitating the safe nullification of the 
 *         deallocated vector, e.g., `my_vector = vecfree(my_vector);`.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // Perform operations using my_vector...
 *     my_vector = vecfree(my_vector); // Free memory and update pointer
 * @endcode
 */
val_t vecfree(val_t vv);

/**
 * @fn val_t *vec(val_t v)
 * 
 * @brief Returns a pointer to the array of values stored in the given vector.
 * 
 * This function allows for direct access to the underlying array of the vector,
 * enabling operations that might require bypassing the vector's API.
 *
 * @param v The vector from which the underlying array pointer will be returned.
 *
 * @return A pointer to the array of values in the vector.
 *
 * @warning Directly manipulating the underlying array can cause undefined behavior
 *          if the vector's metadata (e.g., size, count, etc.) is not updated accordingly.
 */
val_t *vec(val_t v);

/**
 * 
 * @fn uint32_t veccount(val_t v [, uint32_t n])
 * 
 * @brief Retrieve the number of elements currently stored in a vector.
 *
 * The `veccount` function provides a mechanism to determine the number
 * of elements currently stored in the provided vector `v` of type `val_t`. 
 * This count is distinct from the total capacity of the vector and 
 * represents the number of actual elements the user has added to the vector.
 *
 * @param v A vector of type `val_t` whose count of elements is to be retrieved. 
 *
 * @return A `uint32_t` value representing the number of elements currently stored in the vector.
 *         The function will return the value stored in `v->end`. If `v` is NULL or not a 
 *         properly initialized vector, the function returns 0 and sets `errno` to `EINVAL`.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // Add some elements to my_vector... 
 *     uint32_t count = veccount(my_vector); // Retrieve the count of elements in my_vector
 * @endcode
 *
 */
#define veccount(...) VEC_vrg(veccount_,__VA_ARGS__)
#define veccount_1(v) veccount_2(v,VECNONDX)
uint32_t veccount_2(val_t v, uint32_t n);

/**
 * @fn uint32_t vecsize(val_t [, uint32_t n])
 * 
 * @brief Gets or sets the capacity of the vector.
 *
 * The `vecsize` function can be used to either retrieve the current capacity of
 * the vector (i.e., the maximum number of elements it can hold without needing 
 * to resize) or to set a new capacity for the vector.
 *
 * If the `n` parameter is specified, the function adjusts the vector's capacity 
 * to ensure that it can hold at least `n` elements. Otherwise, it simply returns 
 * the current capacity.
 *
 * @param v The vector of type `val_t` whose capacity is to be retrieved or set.
 *
 * @param n (Optional) The desired capacity for the vector. If specified, the 
 *          vector's capacity will be adjusted to at least this size.
 *
 * @return The current or updated capacity of the vector. This represents the 
 *         maximum number of elements the vector can hold without needing a resize.
 *
 * Note: 
 * - `vecsize` can not shrink the vector. If you need to size down the vector,
 *    you should create a new one,  copy the elements in the new one and
 *    free up the current vector.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     uint32_t current_capacity = vecsize(my_vector, 0);
 *     printf("Current capacity: %u\n", current_capacity);
 *     
 *     uint32_t new_capacity = vecsize(my_vector, 100);
 *     printf("New capacity: %u\n", new_capacity);
 * @endcode
 */
#define vecsize(...) VEC_vrg(vecsize_,__VA_ARGS__)
#define vecsize_1(v) vecsize_2(v,VECNONDX)
uint32_t vecsize_2(val_t vv, uint32_t n);

#define vectype(...) VEC_vrg(vectype_,__VA_ARGS__)
#define vectype_1(v) vectype_2(v,-1)
int vectype_2(val_t vv,int type);

/**
 * @fn int vecisqueue(val_t v)
 * 
 * @brief Checks if the given vector {@code v} represents a queue.
 * @param v The vector of type {@code val_t} to be checked for queue properties.
 * @return Returns 1 if it is a queue, 0 otherwise.
 */
#define vecisqueue(v) (vectype(v) == VEC_ISQUEUE)

/**
 * @fn int vecisstack(val_t v)
 * 
 * @brief Checks if the given vector {@code v} represents a stack.
 * @param v The vector of type {@code val_t} to be checked for stack properties.
 * @return Returns 1 if it is a stack, 0 otherwise.
 */
#define vecisstack(v) (vectype(v) == VEC_ISSTACK)

/**
 *  @fn int vecisempty(val_t v)
 * 
 *  @brief Checks if the vector is empty.
 *
 * The `vecisempty` function evaluates the state of the provided `val_t` data 
 * structure to determine if it contains any elements or not.
 *
 * @param v The vector of type `val_t` whose state is to be checked.
 *
 * @return Returns a non-zero number if the vector is empty.
 *         Returns 0 if the vector contains at least one element. 
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (some operations on my_vector)
 *     if (vecisempty(my_vector)) {
 *         printf("The vector is empty.\n");
 *     } else {
 *         printf("The vector has elements.\n");
 *     }
 * @endcode
 */
#define vecisempty(v) (veccount(v) == 0)

#define vecindex(...) VEC_vrg(vecindex_,__VA_ARGS__)
#define vecindex_1(v)   vecindex_3(v,0,0)
#define vecindex_2(v,n) vecindex_3(v,n,0)
uint32_t vecindex_3(val_t vv, uint32_t ndx, int32_t delta);


/**
 * @fn val_t vecset(val_t v ,uint32_t i, val_t x)
 * 
 * @brief Set a value at the specified index in the vector, possibly adjusting its size.
 *
 * The function `vecset` is designed to set the value `x` of type `val_t` at the 
 * specified index `i` within the vector `v` of type `val_t`. 
 * The function ensures that there is enough room in the vector to 
 * perform the operation (or fails if there's not enough room).
 *
 * @param v The vector in which the value will be set. 
 *
 * @param i The index at which the value will be set. Iit must be a non-negative 
 *          integer.
 *
 * @param x The value to be set at index `i` in vector `v`. It can be a base
 *          type (e.g. an integer) or a val_t type.
 *
 * @return The value that has been set or `valerror` in case of failure.
 *
 * @note The function `makeroom` is called to ensure that there is enough room 
 *       in `v` to set the value `x` at index `i`. 
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew() // some initialized vector
 *     uint32_t index = 3;
 *     val_t new_value = val(5.4) // value to set
 *     uint32_t set_index = vecset(my_vector, index, new_value);
 *     uint32_t new_index = vecset(my_vector, index+1, 42);
 * @endcode
 */
#define  vecset(v,i,x)     vecset_(v,i,val(x))
val_t vecset_(val_t v, uint32_t i, val_t x);

/**
 * @fn val_t vecadd(val_t v, val_t x)
 * 
 * @brief Append an element to the end of the vector.
 *
 * The `vecadd` function is employed to insert a new element, represented by `x`, 
 * at the end of the vector `v` of type `val_t`.
 *
 * @param v A vector of type `val_t` to which the element is to be appended.
 *          If `v` is not a valid and initialized vector, 
 * 
 * @param x The element of type `val_t` to be appended to the vector.
 *
 * @return A `uint32_t` value representing the index at which the element has been stored,
 *         or `VECNONDX` in case of an error.
 *
 * @note To remove the last appended elements, you can use the `vecdrop()` function
 * 
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     val_t my_value = ...; // Some value of type val_t
 *     uint32_t new_count = vecadd(my_vector, my_value); // Append my_value to my_vector
 * @endcode
 */
#define vecadd(v,x)    vecset_(v,VECNONDX,val(x))

/**
 * @fn val_t vecget(val_t v , val_t ii)
 * 
 * @brief Retrieve an element from the vector at the specified index.
 *
 * This function retrieves the value at the specified index `i` 
 * from the vector `v`. If `i` is out of bounds, the function 
 * returns `valnil` and sets `errno`. Users should  ensure that `i`
 * is within the valid range of indices for the vector.
 *
 * @param v The vector from which to retrieve the value.
 *
 * @param i The index of the element to retrieve. It is of type `uint32_t`,
 *          ensuring that only non-negative integer values are valid.
 *          If it is unspecified (`vecget(v)`),  the last element of the
 *          vector will be returned.
 * 
 * @param delta A displacement (+/-) with respect to the desired index. If the
 *          resulting index is out of bounds, `vecget()` will return `valnil`
 *
 * @return The value of type `val_t` stored at index `i` in the vector `v`,
 *         or `valnil` upon error.
 *
 * Example usage:
 * @code
 *     val_t my_vector = ... // some initialized vector
 *     uint32_t index = 3;
 *     val_t value = vecget(my_vector, index);
 * @endcode * @
 *
 */
#define vecget(...) VEC_vrg(vecget_,__VA_ARGS__)
#define vecget_1(v) vecget_(v,vecindex(VECCURNDX,0),NULL)
#define vecget_2(v,i) vecget_(v,val(i),NULL)
#define vecget_3(v,i,k) vecget_(v,val(i),k)
val_t vecget_(val_t v, val_t ii, val_t *key);

val_t vecgetfirst(val_t v);
val_t vecgetnext(val_t v);
val_t vecgetmin(val_t v);
val_t vecgetmax(val_t v);

/**
 * @fn val_t vecdel(val_t v, uint32_t i [, uint32_t j] )
 * 
 * @brief Delete a range of elements from a vector, shrinking its size.
 *
 * The `vecdel` function is intended to remove elements from the vector `v` 
 * starting at index `i` and ending at index `j`. If the parameter `j` is not 
 * provided, it defaults to `i`, meaning only the element at index `i` will be removed.
 * 
 * After deletion, the vector is effectively shrunk, and any elements after the deleted 
 * range will be shifted to fill the gap. This ensures that the vector remains contiguous.
 *
 * @param v A vector of type `val_t` from which the elements are to be deleted.
 * 
 * @param i The start index from which deletion begins. Ensure that `i` is within 
 *          the bounds of the vector's current count.
 * 
 * @param j The end index where deletion stops. If not specified, defaults to `i`.
 *          Ensure that `j` is within the bounds of the vector's current count and 
 *          that `j >= i`.
 *
 * @return Returns the number of remaining elements in the vector or `VECERRORNDX`
 *         upon error.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (add some elements to my_vector)
 *     val_t last_deleted = vecdel(my_vector, 2, 4); // Delete elements from index 2 to 4
 *     val_t single_deleted = vecdel(my_vector, 3);  // Delete only the element at index 3
 * @endcode
 */
#define vecdel(...) VEC_vrg(vecdel_,__VA_ARGS__)
#define vecdel_2(v,i) vecdel_3_(v,val(i),VECNONDX)
#define vecdel_3(v,i,j) vecdel_3_(v,val(i),j)
uint32_t vecdel_3_(val_t v, val_t ii, uint32_t j);

#define vecmakegap(...)   VEC_vrg(vec_gap_,__VA_ARGS__)
#define vec_gap_1(v)      vec_gap_3(v, VECNONDX, VECNONDX)
#define vec_gap_2(v,l)    vec_gap_3(v,VECNONDX, l)
int vec_gap_3(val_t v, uint32_t i, uint32_t l);

#define vecins(v,i,x)    vecins_(v,i,val(x))
val_t vecins_(val_t vv, uint32_t i, val_t x);

/**
 * @fn val_t vecpush(val_t v, val_t x)
 * 
 * @brief Push a value onto the stack.
 *
 * The `vecpush` function is designed to append a value to the end of the 
 * `val_t` data structure, effectively treating it as the top of a stack.
 *
 * @param v The stack of type `val_t` onto which the value is to be pushed.
 
 * @param x The value of type `val_t` to be pushed onto the stack.
 *
 * @return The index at which the value was set or `VECNONDX` in case of failure.
 *
 * Example usage:
 * @code
 *     val_t my_stack = vecnew();
 *     vecpush(my_stack, 4.3);      // Push a value onto my_stack
 *     vecpush(my_stack, "Total");  // Push another value onto my_stack
 * @endcode
 */
#define vecpush(v,x)    vecpush_(v,val(x),0)
#define vecpushown(v,x) vecpush_(v,val(x),1)
val_t vecpush_(val_t vv, val_t x, int own);

/**
 * @fn val_t vectop(val_t v, int32_t delta)
 * 
 * @brief Retrieve the top value of the stack without removing it.
 *
 * The `vectop` function provides insight into the topmost value of the stack 
 * without actually modifying the stack.
 *
 * @param v The stack of type `val_t` whose top value is to be retrieved.
 *
 * @return The topmost value of type `val_t` in the stack. If the stack is 
 *         empty, the function returns `valnil`.
 *
 * Example usage:
 * @code
 *     val_t my_stack = vecnew();
 *     // ... (push some elements onto my_stack)
 *     val_t topValue = vectop(my_stack);  // Get the top value without popping it
 * @endcode
 */
#define vectop(...)  VEC_vrg(vectop_,__VA_ARGS__)
#define vectop_1(v)   vectop_2(v,0)
static inline val_t vectop_2(val_t v, uint32_t d) {
  return vecget_2(v,vecindex_3(v,VECTOPNDX,d));
}

/**
 * @fn val_t vecdrop(val_t v [, uint32_t n])
 * 
 * @brief Drop the last `n` elements from a vector-like data structure.
 *
 * The `vecdrop` function is developed to remove the last `n` elements from the 
 * data structure `v` of type `val_t`. This operation essentially reduces the size 
 * of the data structure by `n` elements.
 * 
 * This function is versatile and suitable for different data structures such as 
 * vectors, stacks, and queues, given that they all can be represented with the 
 * type `val_t`.
 *
 * @param v A data structure of type `val_t` from which the elements are to be dropped.
 * 
 * @param n The number of elements to be dropped from the end. If `n` exceeds the 
 *          current count of elements in the data structure, the entire structure 
 *          should be emptied or an appropriate error handling mechanism should be 
 *          in place. If `n` is not specified, it defaults to 1.
 *
 * @return Returns the number of remaining elements in the vector or `VECERRORNDX`
 *         upon error.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (add some elements to my_vector)
 *     val_t last_dropped = vecdrop(my_vector, 3);  // Drop the last 3 elements
 * @endcode
 */
#define vecdrop(...) VEC_vrg(vecdrop_,__VA_ARGS__)
#define vecdrop_1(v) vecdrop_2(v,1)
uint32_t vecdrop_2(val_t v, uint32_t n);

#define vecpop(...) vecdrop(__VA_ARGS__)

/**
 * @fn val_t vecenq(val_t v, val_t x);
 * @brief Enqueue a value onto the queue.
 *
 * The `vecenq` function inserts a value at the end of the vector, 
 * effectively treating it as a queue.
 *
 * @param v The queue of type `val_t` onto which the value is to be enqueued.
 *
 * @param x The value of type `val_t` to be enqueued.
 *
 * @return The value that has been enqueued
 *
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     vecenq(my_queue, someValue);  // Enqueue someValue onto my_queue
 * @endcode
 */
#define vecenq(v,x) vecenq_(v,val(x),0)
#define vecenqown(v,x) vecenq_(v,val(x),1)
val_t vecenq_(val_t v, val_t x, int own);

/**
 * @fn uint32_t vecdeq(val_t v [, uint32_t n])
 * 
 * @brief Dequeue multiple values from the front of the queue.
 *
 * The `vecdeq` function removes the next `n` values from the front of the 
 * `val_t` data structure, treating it as a queue. After dequeuing the specified 
 * number of values, it returns the number of remaining elements in the queue.
 *
 * @param v The queue of type `val_t` from which the values are to be dequeued.
 *
 * @param n The number of values to dequeue from the front of the queue. If `n` 
 *          is not specified, it defaults to 1.
 *
 * @return The number of remaining elemnets in the queue. If an error occurs, 
 *         returns `0` and sets `errno`. 
 *
 * @note You can eliminate the last inserted elements in the queue using
 *       the `vecdrop()` function.
 * 
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     // ... (enqueue some elements onto my_queue)
 *     vecenq(my_queue, 100);
 *     vecenq(my_queue, 200);
 *     vecenq(my_queue, 300);
 *     val_t x = vecfirst(my_queue);  // x will be 100;
 *     vecdeq(my_queue);              // Dequeue the first 2 values from my_queue
 *     x = vecfirst(my_queue)         // x will be 300 
 * @endcode
 */
#define vecdeq(...) VEC_vrg(vecdeq_,__VA_ARGS__)
#define vecdeq_1(v) vecdeq_2(v,1)
uint32_t vecdeq_2(val_t v, uint32_t n);

// #define vechead(vv) vecget(vv,VECHEADNDX)
// #define vectail(vv) vecget(vv,VECTAILNDX)

/**
 * @fn val_t veclast(val_t v,int32_t delta)
 * 
 * @brief Retrieve the last inserted value in the queue without removing it.
 *
 * The `vechead` function gives insight into the first value of the queue 
 * without actually modifying the queue.
 *
 * @param v The queue of type `val_t` whose first value is to be retrieved.
 *          Ensure that `v` is a valid and initialized queue.
 *
 * @param delta A displacement to peek the values in the queue.
 *          If the resulting index is out of bounds, it will return `valnil`
 *
 * @return The last inserted value of type `val_t` in the queue. If the queue is 
 *         empty, the function returns `valnil`.
 *
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     // ... (enqueue some elements onto my_queue)
 *     val_t firstValue = veclast(my_queue);  // Get the last value without dequeuing it
 * @endcode
 */
#define veclast(...)  VEC_vrg(veclast_, __VA_ARGS__)
#define veclast_1(v)  veclast_2(v, 0)
static inline val_t   veclast_2(val_t v,uint32_t d) {
  return vecget_2(v,vecindex_3(v,VECLASTNDX,d)); 
}

/**
 * @fn val_t vecfirst(val_t v,int32_t delta)
 * 
 * @brief Retrieve the first value in the queue without removing it.
 *
 * The `vectail` function gives insight into the first value of the queue 
 * without actually modifying the queue.
 *
 * @param v The queue of type `val_t` whose first value is to be retrieved.
 *          Ensure that `v` is a valid and initialized queue.
 *
 * @param d The offset from the first (toward the last) to be retrieved.
 * 
 * @return The first value of type `val_t` in the queue. If the queue is 
 *         empty, or the offset is past the end of the queue, the function
 *         returns `valnil`.
 *
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     // ... (enqueue some elements onto my_queue)
 *     val_t firstValue = vecfirst(my_queue);  // Get the first inserted value
 * @endcode
 */

val_t vecmapfirst_(val_t vv, val_t *key);

#define vecfirst(...)  VEC_vrg(vecfirst_,__VA_ARGS__)
#define vecfirst_1(v)  vecfirst_2_(v,valnil)
#define vecfirst_2(v,d) vecfirst_2_(v,val(d))
static inline val_t vecfirst_2_(val_t v, val_t dd) {
  if (vectype(v) == VEC_ISMAP) {
    val_t k; val_t *kptr = NULL;
    if (valispointer(dd)) kptr = valtocleanpointer(dd);
    if (kptr == NULL) kptr = &k;
    return vecmapfirst_(v, kptr);
  }
  int32_t d = 0;
  if (valisinteger(dd)) d = valtointeger(dd);
  val_dbg("d: %d",d);
  return vecget_2(v,vecindex_3(v,VECFIRSTNDX,d)); 
}

val_t vecmapnext_(val_t vv, val_t *key);

#define vecnext(...)  VEC_vrg(vecnext_,__VA_ARGS__)
#define vecnext_1(v)   vecnext_2_(v,valnil)
#define vecnext_2(v,d) vecnext_2_(v,val(d))
static inline val_t vecnext_2_(val_t v, val_t dd) {
  if (vectype(v) == VEC_ISMAP) {
    val_t k; val_t *kptr = NULL;
    if (valispointer(dd)) kptr = valtocleanpointer(dd);
    if (kptr == NULL) kptr = &k;
    return vecmapnext_(v, kptr);
  }
  uint32_t d = 0;
  if (valisinteger(dd)) d = valtointeger(dd);
  return vecget_2(v,vecindex_3(v,VECNEXTNDX,d)); 
}

/**
 * @fn val_t vecown(val_t vv, uint32_t ndx, val_t x)
 * 
 * @brief Sets the value of the specified element in the vector 'vv' to the value 'x'.
 *        If 'x' is a vector itself, it is marked as "owned" by 'vv'.
 *        An "owned vector" is automatically freed when the owning vector 'vv' is freed.
 *
 * This function is similar to 'vecset()' but with the added functionality of handling
 * ownership of vectors. If 'x' is a vector, it becomes owned by 'vv', meaning that
 * it will be automatically deallocated when 'vv' is freed or when the elements are 
 * deleted from the vector (e.g. with vecdrop()) or overwritten.
 *
 * @param vv The vector in which the element is to be set. This vector becomes the owner
 *           if 'x' is a vector. Must not be NULL.
 * @param ndx The index of the element in 'vv' to be set. 
 * @param x The value to set at the specified index in 'vv'. If 'x' is a vector,
 *          it becomes owned by 'vv'.
 *
 * @return The x value (possibly modified if it has been owned)
 *
 * @note Owned vectors must be treated with extreme care. The general rule is
 *       that, at any given time, there must be ONE AND ONLY ONE owner. This
 *       meaans, for example, that you can't do something like:
 *                x = vecnew();
 *                x = vecown(v, 3, x);
 *                x = vecset(t, 2, x); // THIS WILL RESULT IN TWO OWNERS!!
 *       The same is valid also if you 
 */

#define vecown(...) VEC_vrg(vecown_,__VA_ARGS__)
#define vecown_2(v,i)   vecown_3_(v,i,valown)
#define vecown_3(v,i,x) vecown_3_(v,i,val(x))
val_t vecown_3_(val_t vv, uint32_t i, val_t x);

val_t vecdisown(val_t vv, uint32_t i);

#define vecdisowntop(...) VEC_vrg(vcedisowntop_,__VA_ARGS__)
#define vecdisowntop_1(vv) vecdisownfirst_2(vv,0)
static inline val_t vecdisowntop_2(val_t vv, int32_t delta) {
  return vecdisown(vv,vecindex_3(vv,VECTOPNDX,delta)); 
}

#define vecdisownfirst(...) VEC_vrg(vcedisownfirst_,__VA_ARGS__)
#define vecdisownfirst_1(vv) vecdisownfirst_2(vv,0)
static inline val_t vecdisownfirst_2(val_t vv, int32_t delta) {
  return vecdisown(vv,vecindex_3(vv,VECFIRSTNDX,delta)); 
}

uint32_t vecsearch(val_t v, val_t x, val_t aux);

typedef  int(*vec_cmp_t)(val_t,val_t);

#define vecsort(...) VEC_vrg(vecsort_,__VA_ARGS__)
#define vecsort_1(v)   vecsort_3(val(v),valcmp,valnil)
#define vecsort_2(v,c) vecsort_3(val(v),c,valnil)
int vecsort_3(val_t v, int(*cmp)(val_t,val_t), val_t aux);

#define vecissorted(v) vecissorted_(val(v))
int vecissorted_(val_t vv);

#define vecsearch(v,k) vecsearch_(val(v),val(k))
uint32_t vecsearch_(val_t vv, val_t key);

extern int64_t vecallocatedmem;

#define vecmap(v,K,V) vecmap_(v,val(K),val(V))
uint32_t vecmap_(val_t vv, val_t key, val_t value);

#define vecunmap(v,K) vecunmap_(v,val(K))
uint32_t vecunmap_(val_t vv, val_t key);

#define vec_root(v) ((v)->fst)

void vec_printtree(vec_t v, FILE *f);
void vecprinttree(val_t vv, FILE *f);

#endif
