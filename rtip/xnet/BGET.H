/*

    Interface definitions for bget.c, the memory management package.

*/

#ifndef __BGET_H__
#define __BGET_H__


#include "xnconf.h"

/* type 'bufsize' must be a SIGNED type   */
typedef int bufsize;


#if (INCLUDE_BGET)
void    bpool   (PFVOID buffer, bufsize len);
PFVOID  bget    (bufsize size);
PFVOID  bgetz   (bufsize size);
PFVOID  bgetr   (PFVOID buffer, bufsize newsize);
void    brel    (PFVOID buf);
void    bectl   (int (*compact)(bufsize sizereq, int sequence),
                PFVOID (*acquire)(bufsize size),
                void (*release)(PFVOID buf), bufsize pool_incr);
void    bstats  (bufsize *curalloc, bufsize *totfree, bufsize *maxfree,
                long *nget, long *nrel);
void    bstatse (bufsize *pool_incr, long *npool, long *npget,
                long *nprel, long *ndget, long *ndrel);
void    bufdump (PFVOID buf);
void    bpoold  (PFVOID pool, int dumpalloc, int dumpfree);
int     bpoolv  (PFVOID pool);

/*
#define malloc bget
#define free   brel
*/

#else
PFVOID  bget    (bufsize size);
void    brel    (PFVOID buf);
PFVOID  bgetz   (bufsize size);
PFVOID  bgetr   (PFVOID buffer, bufsize newsize);
#endif

#if (INCLUDE_DEBUG_BGET)
PFVOID  debug_bget      (bufsize size, int who);
void    debug_brel      (PFVOID buf);
PFVOID  debug_bgetr     (PFVOID buf, bufsize size, int who);
void    display_bget    (void);
void    display_bget_max(void);
#endif


#endif

