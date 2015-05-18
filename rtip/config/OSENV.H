/*                                                                        */
/* osenv.h                                                                */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright EBSnet Inc , 1993,1994,1995                                */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*                                                                        */
/*    Module description:                                                 */
/*        This file contains macros which must be tailored to the         */
/*        compiler and hardware                                           */
/*
** Required cpu specific and compiler specific macros are implemented in
** xnet\osenv.h
**
**
**                         PROCESSOR CONFIGURATION                       **
** KS_CONSTANT      - How to define constants; usually const but provides
**                    ability to turn constant declarations off
**                    (see rtipcons.c)
** KS_LITTLE_ENDIAN - Set this to one for an Intel byte ordered processor
**                    0 for Motorola ordered
** tc_memset       - If these three macros are defined they override the
**                   definitions of them in tools.c and inlines.h. They
**                   effect performance considerably andneed to be
**                   optimized. We call compiler supplied string functions
**                   assuming those are optimal.
** tc_movebytes
** tc_comparen
** LONG_W_ALLIGN   - Set to 1 if longs must be aligned on 4 byte boundaries.
**                   This is required on many risc processors such as MIPS
**                   and powerPC.
** Note:  If LONG_W_ALLIGN is 1 two macros WARRAY_2_LONG and
** LONG_2_WARRAY are needed. Sample implementations are supplied
** but shoud be optimized.
** WARRAY_2_LONG   - This macro takes the address of two shorts  (non long
**                   alligned) and converts them to a long (in native bytes
**                   order).
** LONG_WARRAY     - This macro takes a long and moves it to a non long alligned
**                   array of words (in native byte order)
** INLINE_SUPPORT  - Set this to one if your compiler supports inlining, other
**                   wise set it to zero. The default is 1.
** INLINE          - If INLINE_SUPPORT is 1 you must set this to your
**                   compiler's inline modifiers. The default is __inline.
** KS_FAR          - This macro specifies how to declare far memory
**                   for Intel platforms
** USE_MEM_MAPPED_IO - This macro defines whether platform uses memory
**                   mapped I/O
*
**                         PROCESSOR CONFIGURATION                       **
** OUTBYTE         - Output a byte
** OUTWORD         - Output a word
** INBYTE          - Input a byte
** INWORD          - Input a word
**
*/

#ifndef __OS_ENV__
#define __OS_ENV__ 1



/* include for memset, memcpy, fmemset, fmemcpy etc   */
#include "string.h"

/* *******************************************************************   */
/* constant definition                                                   */
/* *******************************************************************   */
#define KS_CONSTANT const
#define KS_GLOBAL_CONSTANT 
#define KS_EXTERN_GLOBAL_CONSTANT extern

/* *******************************************************************   */
/* TASK LEVEL LOCKS                                                      */
/* *******************************************************************   */
/* macros used to protect data at task level; for rs232 only, these      */
/* could be changed to locking task                                      */
#define KS_TL_SPLX(sp)  sp = ks_splx(); /* Disable and  push interrupts */
#define KS_TL_SPL(sp)   ks_spl(sp);
#define KS_TL_DISABLE() ks_disable();
#define KS_TL_ENABLE()  ks_enable();

/* *******************************************************************   */
/* PROCESSOR ENVIRONMENT - MIPS (Green Hills)                            */
/* *******************************************************************   */

#define KS_LITTLE_ENDIAN    1  /* 1 is an Intel byte order 0 is Motorola */
#define LONG_W_ALLIGN       1  /* If 1 longs must be on 4 byte boundaries */
#define SHORT_W_ALLIGN      1  /* If 1 shorts must be on 2 byte boundaries */

#define INLINE_SUPPORT      0  /* If 1 compiler supports inline */
#define INLINE                 /* compiler specific inline modifier */

#define USE_MEM_MAPPED_IO   1  /* Use memory mapped IO */

/* FAR MEMORY and MEMORY MOVE definitions                                */
#define KS_FAR
#define near
#define tc_memset(A,B,C)         memset((A), (B), (C))
#define tc_movebytes(A,B,C)      memcpy((A), (B), (C))
#define tc_comparen(A,B,C)      (memcmp((A), (B), (C))==0)
#define tc_memchr(str,chr,len)   memchr(str, chr, len)

/* ********************************************************************   */
/* NEAR, HUGH                                                             */
/* ********************************************************************   */
#if !defined(KS_HUGE)
#define KS_HUGE KS_FAR
#endif
#if !defined(KS_NEAR)
#define KS_NEAR
#endif

/* net2hs, net2hl etc omitted here, they are in inlines.h for little endian */

/* ********************************************************************  */
/* tc_mv4 and tc_cmp4                                                    */
/* *******************************************************************   */
/* Move and compare 4 bytes   */
/* Move compare and assign long to/from 4 byte arrays that are not necessarilly
   long word aligned. For many processors these can just be assigns and
   compares with the byte array casted to a pointer to long. For many
   risk processors the bytes must be accessed one at a time since
   the longs must be long word alligned */
/* ********************************************************************  */
/* WARRAY_2_LONG and LONG_2_WARRAY                                       */
/* *******************************************************************   */
#if (LONG_W_ALLIGN)
#define tc_mv4(A,B,C)  tc_movebytes((A), (B), 4)
#define tc_cmp4(A,B,C) (tc_comparen((A), (B), 4))
unsigned long WARRAY_2_LONG(unsigned short KS_FAR *X);  /* See osenv.c for these functions */
void  LONG_2_WARRAY(unsigned short KS_FAR *TO,unsigned long FR);
#else
#error OSENV.H - Since LONG_W_ALLIGN was assumed, tc_mv4, tc_cmp4, WARRAY_2_LONG, LONG_2_WARRAY are not defined now
#endif

/* *******************************************************************   */
/* PACKET ALIGNMENT                                                      */
/* *******************************************************************   */
/* Define the byte alignment requirements for packets */
#define KS_PKT_ALIGNMENT 16

/* ********************************************************************  */
/* MEMORY MAPPED I/O                                                     */
/* *******************************************************************   */
#undef IOADDRESS
typedef volatile unsigned char * IOADDRESS; /* Memory mapped IO */
/* the alternative is IO mapped IO */

/* ********************************************************************  */
/* USE ASCII LIBRARY FOR STRING FUNCTIONS                                */
/* *******************************************************************   */
#define KS_USE_ASCII_LIB 1

/* ********************************************************************  */
/* USE PRINTF LIBRARY                                                    */
/* ********************************************************************  */
/* Set to non-zero value if you want the RTIP tc/tm_[*]printf functions
   including their extended functionality, instead of the regular RTL
   functions [*]printf() */
#define KS_USE_TC_PRINTF 1

/* ********************************************************************  */
/* ARGSUSED (keep compiler happy macros)                                 */
/* ********************************************************************  */
/* the ARGSUSED_PVOID and ARGSUSED_INT macros are used throughout        */
/* the code where parameters are passed which are never used             */
/* (many times due to turning off pieces of code via INCLUDE_xxx);       */
/* to keep the compiler from compaining about an unreferenced parameter; */
#define ARGSUSED_NOTHING 1      /* ARGSUSED_ macros go away */
#define ARGSUSED_ASSIGN  2      /* ARGSUSED_ macros do assignments */
#define ARGSUSED_FUNC    3      /* ARGSUSED_ macros call function */

#define KS_ARGSUSED ARGSUSED_ASSIGN     /* you may want to modify this */

/* DO NOT CHANGE: change ARGSUSED instead   */
#if (KS_ARGSUSED == ARGSUSED_NOTHING)
#define ARGSUSED_PVOID(p)
#define ARGSUSED_INT(i)
#elif (KS_ARGSUSED == ARGSUSED_ASSIGN)
#define ARGSUSED_PVOID(x)  x=x;
#define ARGSUSED_INT(x)    x=x;
#elif (KS_ARGSUSED == ARGSUSED_FUNC)
#define ARGSUSED_PVOID(x)  argsused_pvoid((PFVOID)x);
#define ARGSUSED_INT(x)    argsused_int((int)x);
#else
#error: KS_ARGUSED must be defined in osenv.h
#endif

/* ********************************************************************  */
/* TYPES                                                                 */
/* ********************************************************************  */
/*
**             ** Typedefs that wont need changing          **
** byte            - Unsigned char
** word            - Unsigned short
** dword           - Unsigned long
** BOOLEAN         - int
*/

typedef int RTIP_BOOLEAN;

#ifndef TRUE
#define TRUE  1                 /* Don't change */
#endif
#ifndef FALSE
#define FALSE 0                 /* Don't change */
#endif

typedef unsigned char byte;     /* Don't change */
typedef unsigned short word;    /* Don't change */
typedef unsigned long dword;    /* Don't change */

/* ********************************************************************  */
/* DERIVED TYPES                                                         */
/* *******************************************************************   */

/* Derived types. These are derived from the values above and
   should not be changed */
typedef byte  KS_FAR * PFBYTE;
typedef int   KS_FAR * PFINT;
typedef word  KS_FAR * PFWORD;
typedef long  KS_FAR * PFLONG;
typedef dword KS_FAR * PFDWORD;
typedef char  KS_FAR * PFCHAR;
typedef void  KS_FAR * PFVOID;
typedef KS_CONSTANT char  KS_FAR * PFCCHAR;
typedef KS_CONSTANT byte  KS_FAR * PFCBYTE;
typedef KS_CONSTANT void  KS_FAR * PFCVOID;
typedef RTIP_BOOLEAN KS_FAR * PFRTIP_BOOLEAN;

/* ********************************************************************  */
/* I/O MACROS                                                            */
/* *******************************************************************   */
/* Rep in and rep out routines. The code is in osenv.c, only needed
   by device drivers. */
void insw(IOADDRESS ioaddr, unsigned short KS_FAR * dst, int n);
void insb(IOADDRESS ioaddr, unsigned char KS_FAR * dst, int n);
void outsw(IOADDRESS ioaddr, unsigned short KS_FAR * src, int n);
void outsb(IOADDRESS ioaddr, KS_CONSTANT unsigned char KS_FAR * src, int n);

/* USE_MEM_MAPPED_IO assumed */
#define OUTBYTE(ADD, VAL) (*(volatile unsigned char *) (ADD)) = (unsigned char)(VAL)
#define OUTWORD(ADD, VAL) (*(volatile unsigned short *)(ADD)) = (unsigned short)(VAL)
#define INBYTE(ADD)       (*(volatile unsigned char *) (ADD))
#define INWORD(ADD)       (*(volatile unsigned short *)(ADD))
#define INDWORD(ADD)       (*(volatile unsigned long *)(ADD))


/* ********************************************************************  */
/*                                                                       */
/* *******************************************************************   */
typedef void (KS_FAR *OS_ISR_POINTER)(void);
typedef OS_ISR_POINTER OS_ISR_SAVE;
void os_mask_isr_on(int irq);
void os_mask_isr_off(int irq);
void os_clear_isr(int irq);

#endif  /* __OS_ENV__ */

