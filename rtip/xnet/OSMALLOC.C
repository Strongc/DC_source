
/*                                                                      */
/* OSMALLOC.C - malloc/free functions                                   */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module consists of malloc free routines                    */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_BGET)
#include "bget.h"
#elif (INCLUDE_WINSOCK || INCLUDE_BSDSOCK)
#include <malloc.h>
#endif
#if ( defined(__BORLANDC__) ) /* real mode */
#include <malloc.h>
#endif


/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DISPLAY_MALLOC 0                // FKA !!

/* ********************************************************************   */
#if (INCLUDE_MALLOC)
/* ********************************************************************   */
PFBYTE _ks_malloc(int num_elements, int size)
{
PFBYTE ptr;

#    if (defined (RTKBCPP))
        ptr = ks_dpmi_alloc((word)(num_elements * size));
#    elif (INCLUDE_BGET)
        ptr = ((PFBYTE)bget(num_elements * size));
#    elif (INCLUDE_WINSOCK || INCLUDE_BSDSOCK)
        ptr = malloc(num_elements * size);
#    elif (defined(SEG_IAR))
        /* protected mode   */
        ptr = malloc(num_elements * size);
#    elif ( defined(__BORLANDC__) )   /* real mode */
        ptr = _fmalloc(num_elements * size);

#    else
        #error: ks_malloc needs to be implemented
#    endif

#if (DISPLAY_MALLOC)
    DEBUG_ERROR("ks_malloc allocs: ", DINT1, ptr, 0);
#endif
    return(ptr);
}

/* ********************************************************************   */
PFBYTE _ks_realloc(PFBYTE ptr, int num_elements, int size)
{
#if (DISPLAY_MALLOC)
    DEBUG_ERROR("ks_realloc free: ", DINT1, ptr, 0);
#endif

#    if (defined (RTKBCPP))
/*#error: needs to be implemented   */
        ptr = ks_dpmi_alloc((word)(num_elements * size));
#    elif (INCLUDE_BGET)
        ptr = ((PFBYTE)bgetr(ptr, num_elements * size));

#    elif (NACT_OS || __TM1__ && INCLUDE_DE4X5)
#error: needs to be implemented
        ptr2 = ks_malloc(ptr, num_elements * size);
        tc_movebytes(ptr2, ptr, ?);

#    elif (INCLUDE_WINSOCK || INCLUDE_BSDSOCK)
        ptr = realloc(ptr, num_elements * size);

#    elif (defined(SEG_IAR))
        /* protected mode   */
        ptr = realloc(ptr, num_elements * size);
#    elif ( defined(__BORLANDC__) )   /* real mode */
        ptr = _frealloc(ptr, num_elements * size);

#    else
        #error: ks_realloc needs to be implemented
#    endif

    return(ptr);
}

/* ********************************************************************   */
void _ks_free(PFBYTE ptr, int num_elements, int size)
{
#if (DISPLAY_MALLOC)
    DEBUG_ERROR("ks_free returns: ", DINT1, ptr, 0);
#endif

#    if (defined (RTKBCPP))
        /* Was done by ks_dpmi_release_all();   */
        ARGSUSED_PVOID(ptr)
        ARGSUSED_INT(num_elements)
        ARGSUSED_INT(size)

#    elif (INCLUDE_BGET)
        ARGSUSED_INT(num_elements)
        ARGSUSED_INT(size)
        brel(ptr);

#    elif (INCLUDE_WINSOCK || INCLUDE_BSDSOCK)
        ARGSUSED_INT(num_elements);
        ARGSUSED_INT(size);
        free(ptr);

#    elif (defined(SEG_IAR))
        /* protected mode   */
        ARGSUSED_INT(num_elements);
        ARGSUSED_INT(size);
        free(ptr);

#    elif ( defined(__BORLANDC__) )   /* real mode */
        ARGSUSED_INT(num_elements);
        ARGSUSED_INT(size);
        _ffree(ptr);


#    else
        #error: ks_free needs to be implemented
#    endif
}

#if (INCLUDE_DEBUG_MALLOC)
/* ********************************************************************   */
/* DEBUG MALLOC                                                           */
/* ********************************************************************   */
typedef struct debug_malloc_info
{
    PFBYTE addr;        /* address returned by malloc (0 if unused) */
    int    size;        /* amount of bytes allocated */
    int    num_ele;     /* amount of bytes allocated */
    int    who;         /* which call in system called ks_malloc (see rtip.h) */
} DEBUG_MALLOC_INFO;

/* information about active memory allocations   */
DEBUG_MALLOC_INFO debug_malloc_info[CFG_DEBUG_MALLOC_SIZE] = {{0}};

/* current amount of memory allocated (by each call)   */
unsigned long int debug_malloc_totals_info[CONFIG_MAX_WHO_MALLOC];

/* maximum amount of memory allocated at one time (by each call)   */
unsigned long int debug_malloc_max_info[CONFIG_MAX_WHO_MALLOC];

#define GUARD_BYTE 0xed

/* ********************************************************************   */
void break_malloc(void)
{
}

/* ********************************************************************   */
PFBYTE debug_ks_malloc(int num_elements, int size, int who)
{
PFBYTE ptr;
int i;

    ptr = _ks_malloc(num_elements, size+GUARD_MALLOC_SIZE);

    if (ptr)
    {
        for (i=0; i < CFG_DEBUG_MALLOC_SIZE; i++)
        {
            if (debug_malloc_info[i].addr == 0)
            {
                debug_malloc_info[i].addr = ptr;
                debug_malloc_info[i].size = size;
                debug_malloc_info[i].num_ele = num_elements;
                debug_malloc_info[i].who  = who;
                break;
            }
        }
        if (i == CFG_DEBUG_MALLOC_SIZE)
        {
            DEBUG_ERROR("debug_malloc: CFG_DEBUG_MALLOC_SIZE too small", NOVAR, 0, 0);
        }
#if (INCLUDE_GUARD_MALLOC)
        tc_memset((PFBYTE)ptr+(size*num_elements), GUARD_BYTE,
            GUARD_MALLOC_SIZE);
#endif

        /* update stats   */
        {
            int idx = who - CONFIG_WHO_BASE_MALLOC;
            if (idx < 0 ||
                idx >= sizeof(debug_malloc_totals_info)/sizeof(debug_malloc_totals_info[0]))
            {
                idx = 0;
            }
            debug_malloc_totals_info[idx] += (size*num_elements);
            if (debug_malloc_totals_info[idx] > debug_malloc_max_info[idx])
                debug_malloc_max_info[idx] = debug_malloc_totals_info[idx];
        }
    }

    return(ptr);
}

/* ********************************************************************   */
PFBYTE debug_ks_realloc(PFBYTE ptr1, int num_elements, int size, int who)
{
PFBYTE ptr;
int i;
//XPC: PFBYTE ptr_old;
int   sze = 0;
int   num_ele = 0;
int   who_old = 0;

    /* FIRST DO DEBUG STUFF FOR ALLOC ADDRESS FREEING   */
    if (ptr1)
    {
        for (i=0; i < CFG_DEBUG_MALLOC_SIZE; i++)
        {
            if (ptr1 && debug_malloc_info[i].addr == ptr1)
            {
                /* ptr1 was freed so update debug_malloc_info   */
                /*XPC: ptr_old = debug_malloc_info[i].addr;*/
                sze = debug_malloc_info[i].size;
                num_ele = debug_malloc_info[i].num_ele;
                who = debug_malloc_info[i].who;

                debug_malloc_info[i].addr = (void *)0;
                debug_malloc_info[i].who  = 0;
                debug_malloc_info[i].size = 0;
                debug_malloc_info[i].num_ele = 0;
                break;
            }
        }
        if (i == CFG_DEBUG_MALLOC_SIZE)
        {
            break_malloc();
            DEBUG_ERROR("debug_realloc: attempt to free a nonallocated pointer: ",
                DINT1, ptr1, 0);
            return(0);
        }

#if (INCLUDE_GUARD_MALLOC)
        for (i=0; i<GUARD_MALLOC_SIZE; i++)
        {
            if (*((PFBYTE)ptr1+(sze*num_ele)+i) != GUARD_BYTE)
            {
                DEBUG_ERROR("debug_realloc: wrote past malloced area: address = ",
                    DINT1, ptr1, 0);
                DEBUG_ERROR("            sze, num_ele", EBS_INT2, sze, num_ele);
                DEBUG_ERROR("            who", EBS_INT1, who, 0);
                DEBUG_ERROR("            DATA: ", PKT, ptr1, sze+GUARD_MALLOC_SIZE);
                *((PFBYTE)ptr1 + sze+GUARD_MALLOC_SIZE-1) = 0;
                DEBUG_ERROR("            STR: ", STR1, ptr1, 0);
                break_malloc();
                break;
            }
        }
#endif
    }       /* end of if ptr1 */

    /* ********************************************************************   */
    /* DO THE REALLOC                                                         */
    /* ********************************************************************   */
    ptr = _ks_realloc(ptr1, num_elements, size+GUARD_MALLOC_SIZE);

    /* ********************************************************************   */
    /* NO DO DEBUG STUFF FOR NEW ALLOC ADDRESS                                */
    if (ptr)
    {
        for (i=0; i < CFG_DEBUG_MALLOC_SIZE; i++)
        {
            if (debug_malloc_info[i].addr == 0)
            {
                debug_malloc_info[i].addr = ptr;
                debug_malloc_info[i].size = size;
                debug_malloc_info[i].num_ele = num_elements;
                debug_malloc_info[i].who  = who;
                break;
            }
        }
        if (i == CFG_DEBUG_MALLOC_SIZE)
        {
            DEBUG_ERROR("debug_malloc: CFG_DEBUG_MALLOC_SIZE too small", NOVAR, 0, 0);
        }
    }

#if (INCLUDE_GUARD_MALLOC)
    tc_memset((PFBYTE)ptr+(size*num_elements), GUARD_BYTE,
        GUARD_MALLOC_SIZE);
#endif

    /* update stats   */
    {
        int idx = who - CONFIG_WHO_BASE_MALLOC;
        int idx_old = who_old - CONFIG_WHO_BASE_MALLOC;
        if (idx < 0 ||
            idx >= sizeof(debug_malloc_totals_info)/sizeof(debug_malloc_totals_info[0]))
        {
            idx = 0;
        }
        if (idx_old < 0 ||
            idx_old >= sizeof(debug_malloc_totals_info)/sizeof(debug_malloc_totals_info[0]))
        {
            idx_old = 0;
        }

        debug_malloc_totals_info[idx] += (size*num_elements);
        debug_malloc_totals_info[idx_old] -= sze;

        if (debug_malloc_totals_info[idx] > debug_malloc_max_info[idx])
            debug_malloc_max_info[idx] = debug_malloc_totals_info[idx];
    }

    return(ptr);
}

/* ********************************************************************   */
void debug_ks_free(PFBYTE buf, int num_elements, int size)
{
int   i;
void *ptr;
int   sze = 0;
int   num_ele = 0;
int   who = 0;

    ptr = 0;

    for (i=0; i < CFG_DEBUG_MALLOC_SIZE; i++)
    {
        if (buf && debug_malloc_info[i].addr == buf)
        {
            ptr = debug_malloc_info[i].addr;
            sze = debug_malloc_info[i].size;
            num_ele = debug_malloc_info[i].num_ele;
            who = debug_malloc_info[i].who;
            debug_malloc_info[i].addr = (void *)0;
            debug_malloc_info[i].who  = 0;
            debug_malloc_info[i].size = 0;
            debug_malloc_info[i].num_ele = 0;
            break;
        }
    }
    if (i == CFG_DEBUG_MALLOC_SIZE)
    {
        break_malloc();
        DEBUG_ERROR("debug_free: attempt to free a nonallocated pointer: ",
            DINT1, buf, 0);
        return;
    }

#if (INCLUDE_GUARD_MALLOC)
    if (ptr)
    {
        for (i=0; i<GUARD_MALLOC_SIZE; i++)
        {
            if (*((PFBYTE)ptr+(sze*num_elements)+i) != GUARD_BYTE)
            {
                DEBUG_ERROR("debug_free: wrote past malloced area: address = ",
                    DINT1, ptr, 0);
                DEBUG_ERROR("            sze, num_ele", EBS_INT2, sze, num_ele);
                DEBUG_ERROR("            who", EBS_INT1, who, 0);
                DEBUG_ERROR("            DATA: ", PKT, ptr, sze+GUARD_MALLOC_SIZE);
                *((PFBYTE)ptr + sze+GUARD_MALLOC_SIZE-1) = 0;
                DEBUG_ERROR("            STR: ", STR1, ptr, 0);
                break_malloc();
                break;
            }
        }
    }
#endif

    /* update stats   */
    {
        int idx = who - CONFIG_WHO_BASE_MALLOC;
        if (idx < 0
            || idx >= sizeof(debug_malloc_totals_info)/sizeof(debug_malloc_totals_info[0]))
        {
            idx = 0;
        }
        debug_malloc_totals_info[idx] -= (sze*num_ele);
    }

    _ks_free (buf, num_elements, size);
}

/* ********************************************************************   */
/* STATISTICS                                                             */
/* ********************************************************************   */
/* display amount of dynamic memory currently allocated by each call      */
/* in the system to ks_malloc()                                           */
void display_malloc(void)
{
int i;

    tc_memset(debug_malloc_totals_info, 0, sizeof(debug_malloc_totals_info));
    tc_memset(debug_malloc_max_info, 0, sizeof(debug_malloc_max_info));

    DEBUG_ERROR("\nMALLOC INFORMATION", NOVAR, 0, 0);
    DEBUG_ERROR("----------------", NOVAR, 0, 0);
    for (i=0; i < CFG_DEBUG_MALLOC_SIZE; i++)
    {
        if (debug_malloc_info[i].addr != (void *)0)
        {
            /* calculate 'total usage' too for 'who'   */
            int idx = debug_malloc_info[i].who - CONFIG_WHO_BASE_MALLOC;
            if (idx < 0
                || idx >= sizeof(debug_malloc_totals_info)/sizeof(debug_malloc_totals_info[0]))
            {
                idx = 0;
            }
            debug_malloc_totals_info[idx] += debug_malloc_info[i].size;

            DEBUG_ERROR("MALLOC addr, who: ", DINT2,
                debug_malloc_info[i].addr, idx);
            DEBUG_ERROR("            size, num_elements:", EBS_INT2,
                debug_malloc_info[i].size,
                debug_malloc_info[i].num_ele);
        }
    }

    tc_movebytes(debug_malloc_max_info, debug_malloc_totals_info, sizeof(debug_malloc_max_info));

    DEBUG_ERROR("--- TOTALS (NON-ZERO ONLY) ----", NOVAR, 0, 0);
    for (i=0; i < sizeof(debug_malloc_totals_info)/sizeof(debug_malloc_totals_info[0]); i++)
    {
        if (debug_malloc_totals_info[i] != 0)
        {
            DEBUG_ERROR("MALLOC TOTAL who, size: ", DINT2,
                i, debug_malloc_totals_info[i]);
        }
    }
}

/* display maximum amount of dynamic memory allocated by each call in   */
/* the system to ks_malloc()                                            */
void display_malloc_max(void)
{
int i;

    /* assume malloc et al kept track of the memory consumed.   */

    DEBUG_ERROR("\n     MALLOC MAX INFORMATION", NOVAR, 0, 0);
    DEBUG_ERROR("   --- TOTALS (NON-ZERO ONLY) ----", NOVAR, 0, 0);
    for (i=0;
         i < sizeof(debug_malloc_max_info)/sizeof(debug_malloc_max_info[0]);
         i++)
    {
        if (debug_malloc_max_info[i] != 0)
        {
            DEBUG_ERROR("MAX TOTAL who, size: ", DINT2,
                i, debug_malloc_max_info[i]);
        }
    }
}
#endif  /* (INCLUDE_DEBUG_MALLOC) */

#endif  /* INCLUDE_MALLOC */ 

