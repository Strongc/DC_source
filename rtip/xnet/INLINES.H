/*                                                                      */
/* INLINES.H - contains inline functions                                */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#ifndef __INLINES__
#define __INLINES__ 1

#include "rtip.h"

#if (INLINE_SUPPORT)

#if (KS_LITTLE_ENDIAN)
/* ********************************************************************   */
/* Net to intel and intel to net conversion routines                      */
/* ********************************************************************   */
/*                                                                        */
INLINE word net2hs(word w)          /*__fn__*/
{
#    if (KS_LITTLE_ENDIAN)
        w = (word)((w>>8) | (w<<8));    /* Compiler warning acceptable here */
#    endif
    return(w);
}

/* ********************************************************************   */
INLINE word hs2net(word w)          /*__fn__*/
{
#    if (KS_LITTLE_ENDIAN)
        w = (word)((w>>8) | (w<<8));    /* Compiler warning acceptable here */
#    endif
    return(w);
}
#endif      /* KS_LITTLE_ENDIAN */

/* ********************************************************************   */
/* Memory set,compare routines                                            */
/* ********************************************************************   */

/* ********************************************************************   */
/* memset function that takes far pointers regardless of model            */
#ifndef tc_memset
INLINE void tc_memset(PFBYTE p, byte b, int n)                      /*__fn__*/
{
    int i;  
    for(i = 0; i < n; i++) 
        *p++ = b;
}
#endif

/* ********************************************************************   */
/* compare function that takes far pointers regardless of model           */
#ifndef tc_comparen
INLINE RTIP_BOOLEAN tc_comparen(PFCBYTE a, PFCBYTE b, int n)            /*__fn__*/
{
    int i;  
    for (i = 0; i < n; i++) 
        if (*a++ != *b++) 
            return(FALSE); 
    return(TRUE);
}
#endif

#ifndef tc_memchr
INLINE PFVOID tc_memchr(PFCBYTE str, int chr, int n)            /*__fn__*/
{
    int i;
    
    for (i = 0; i < n; i++) 
        if (*str++ == (byte)chr) 
            return str-1; 
    return 0;
}
#endif



/* ********************************************************************   */
/* compare function that takes far pointers regardless of model           */
#ifndef tc_movebytes
INLINE void tc_movebytes(PFBYTE to, PFBYTE fr, int n)            /*__fn__*/
{
    int i;  
    for (i = 0; i < n; i++) 
        *to++ = *fr++;
}
#endif

#endif      /* INLINE_SUPPORT */

#endif      /* __INLINES__  */

