/* OSINT.C - Porting Layer - Interrupt functions                        */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
#include "os.h"
/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_IRQ_REENTRANT 0   /* turn on to print message if reenter IRQ */

/* ********************************************************************   */
void ks_dispatch_interrupt(int irq);

/* ********************************************************************   */
#if (DEBUG_IRQ_REENTRANT)
#define IRQ_ENTER                                                       \
    {                                                                   \
        in_irq++;                                                       \
        if (in_irq > 1)                                                 \
        {                                                               \
            DEBUG_ERROR("REENTER IRQ: in_irq", EBS_INT1, in_irq, 0);    \
        }                                                               \
    }

#define IRQ_EXIT    in_irq--;
#else
#define IRQ_ENTER   in_irq++;
#define IRQ_EXIT    in_irq--;
#endif

/* ********************************************************************        */
/* INTERRUPTS:                                                                 */
/* ks_hook_interrupt       - This is called by device drivers to install       */
/*                           their interrupt service routines in the interrupt */
/*                           vector table and enable the inerrupts. The        */
/*                           routine takes the hardware interrupt number and   */
/*                           the address of the interrupt handler. In the      */
/*                           PC based reference port the handler is installed  */
/*                           in the interrupt table and the irq is enabled     */
/*                           through the 8259. To make interrupt management    */
/*                           more portable we provide typing macros for        */
/*                           interrupts:                                       */
/*                              KS_INTFN_DECLARE(x)                            */
/*                              KS_INTFN_POINTER(x)                            */
/*                              KS_INTFN_CASTE                                 */
/*                           This routine has implementations for each         */
/*                           supported kernel seperated by #ifs.               */
/* ks_restore_interrupts   - If the RTIP application ever exits it may call    */
/*                           xn_rtip_exit which calls this routine to clean    */
/*                           up interrupt vector table.                        */
/*                           This routine has implementations for each         */
/*                           supported kernel seperated by #ifs.               */
/*                                                                             */
/*   The routine ks_hook_interrupt() is called to hook an interrupt.           */
/*   The interrupt service routine is saved in rtip_isr_strategy[irq]          */
/*   and either the routine rtip_a_isr_x (if KS_USE_ASM_INTERRUPTS is 0)       */
/*   or rtip_c_isr_x (if KS_USE_ASM_INTERRUPTS is 1) is hooked to the          */
/*   interrupt.  The routine rtip_a/c_isr_x will call ks_dispatch_int().       */
/*   ks_dispatch_int() contains any platform specific code and it calls        */
/*   rtip_isr_strategy[irq]().                                                 */
/*                                                                             */
/*   If there is any special processing that needs to be done at               */
/*   the interrupt (i.e. cannot be done by the task layer -                    */
/*   see INCLUDE_TASK_ISRS) then the parameter c_interrupt contains            */
/*   the name of the routine to call when the interrupt occurs.                */
/*   The name of this routine is saved in rtip_isr_interrupt[irq].             */
/*                                                                             */
/*      SUMMARY:                                                               */
/*      --------                                                               */
/*   rtip_a/c_isr_x()      (which routine a/c depends upon USE_ASM_INTERRUPTS) */
/*          |                                                                  */
/*   ks_dispatch_int()     (contains platform specific code)                   */
/*          |                                                                  */
/*   rtip_isr_strategy[irq]     (passed to ks_hook_interrupt)                  */
/*                                                                             */
/*   If INCLUDE_TASK_ISRS is 1, c_interrupt is the routine which               */
/*   is called when the interrupt occurs.  It is not used if                   */
/*   it is 0.                                                                  */
/*                                                                             */
/*   c_stategy is the routine which does the work of receiving the             */
/*   packet.  If INCLUDE_TASK_ISRS is 1, it is called from task                */
/*   layer.  If it is 0, it is called from interrupt service routine.          */
/*                                                                             */
/*   Returns TRUE if successful, FALSE if failure                              */
/*                                                                             */
/* ********************************************************************        */

/* ********************************************************************   */
/* ********************************************************************   */

#if (ARM_IAR || defined(__ghs_board_is_nec_vr41xx)) /*OS*/
RTIP_BOOLEAN ks_hook_interrupt(int irq, PFVOID piface, 
                          RTIPINTFN_POINTER c_strategy, 
                          RTIPINTFN_POINTER c_interrupt, int c_arg)
{
#if (INCLUDE_TASK_ISRS)
BUG - Feature not implemented for this port yet
#endif
    volatile int dummy = 1;
    return(TRUE);
}
void ks_restore_interrupts(void)
{
    volatile int dummy = 1;
}

#else  /*OS*/       /* EXCLUDE REST OF FILE */

/* ********************************************************************   */
/* EXTERNAL DATA                                                          */

/* ********************************************************************   */
/* REGISTER DEFINITIONS                                                   */
/* ********************************************************************   */
/* install the interrupt vector                                           */
#if (AT_MOTHERBOARD)
/****** Interrupt Controller Registers *****  */
#define MR8259A     0x21        /* interrupt controller 1 */
#define IR8259A     0x20
#define MR8259B     0xA1        /* interrupt controller 2 */
#define IR8259B     0xA0

/****** 8259 REGISTER DEFINITIONS *****  */
#define EOI     0x20        /* end of interrupt */
#define RIRR    0x0a        /* read interrupt request register */

/* define 8259 base addresses globally to allow non-PC values to be used   */
#define vec_8259a 0x08
#define vec_8259b 0x70
#endif  /* AT_MOTHERBOARD */

/* ********************************************************************   */
/* GETVECT, SETVECT                                                       */
/* ********************************************************************   */

/* ********************************************************************   */
KS_INTFN_POINTER(oldint[KS_NUM_INTS]);

#if (!ARM7TDMI )
#if (defined(SEGBCP0) )
/* ********************************************************************   */
KS_INTFN_POINTER(rtip_getvect(int vector_no))
{
#if (KS_SUPPORTS_DOS_CALLS)
   /* This is the dos version                        */
   /* get (and save) the current interrupt vector    */
   return((void (INTERRUPT KS_FAR *)(void))_dos_getvect((unsigned)vector_no));

#elif (PPC603)
/* Nothing to do for PPC/Cogent platform   */

#elif (POWERPC)
#pragma warn  Implement rtip_getvect for your system.

#else

#error Implement rtip_getvect for your system.
#endif      /* TARGET_186ES else MC68K else */
}

/* ********************************************************************   */
void rtip_setvect(int vector_no, KS_INTFN_POINTER(isr_func))
{
#if (KS_SUPPORTS_DOS_CALLS)   /* MC68K, elif TARGET_186ES */
   /* This is the dos version   */
    _dos_setvect (vector_no, isr_func);

#elif (PPC603)
/* Nothing to do for PPC/Cogent platform   */

#elif (POWERPC)
#error  Implement rtip_setvect for your system.

#else
#error Implement rtip_setvect for your system.
#endif
}
#endif
#endif

/* ********************************************************************   */
/* DISPATCH INTERRUPTS                                                    */
/* ********************************************************************   */

/* ********************************************************************   */
/* Dispatch the interrupt                                                 */
void ks_dispatch_interrupt(int irq)
{
    IRQ_ENTER   /* track that we are in interrupt service routine */

#if (INCLUDE_TASK_ISRS)
    /* interrupt processing to be done by interrupt task   */
    if (rtip_irq_iface[irq])                      /* Signal this iface */
    {
        /* if interrupt (c_interrupt in ks_hook_interrupt)   */
        if (rtip_isr_interrupt[irq])              /* If it exists then call */
        {
            rtip_isr_interrupt[irq](rtip_args[irq]);    /* special driver */
                                                        /* interrupt entry code   */
        }

        /* wake up the interrupt task so the strategy routine will   */
        /* get called                                                */
        ks_invoke_interrupt(rtip_irq_iface[irq]);
    }
    else
#endif

    {
       if (rtip_isr_strategy[irq])
          rtip_isr_strategy[irq](rtip_args[irq]);
#if (PPC603)
       else /* mask off sources which aren't hooked */
          ks_mask_isr_off(irq);
#endif
    }

    /* Disable interrupts so we IRET with interrupts disabled.. This
       allows the stack swap to occur */
    ks_disable();

#if (!defined(SEGMC16))
    ks_clear_isr(irq);
#endif

    IRQ_EXIT    /* track that we are left interrupt service routine */
}


/* ********************************************************************   */
/* INTERRUPT SERVICE ROUTINES                                             */
/* ********************************************************************   */
KS_INTFN_DECLARE(rtip_c_isr_0)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(0);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_1)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(1);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_2)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(2);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_3)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(3);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_4)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(4);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_5)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(5);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_6)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(6);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_7)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(7);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_8)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(8);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_9)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(9);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_10)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(10);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_11)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(11);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_12)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(12);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_13)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(13);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_14)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(14);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}
KS_INTFN_DECLARE(rtip_c_isr_15)
{
#if (KS_USE_MSPROLOG)
    #include "msprolog.h"       /* Prolog for microsoft PM comiler */
#endif
    KS_ENTER_ISR();
    ks_dispatch_interrupt(15);
    KS_EXIT_ISR();
#if (KS_USE_MSPROLOG)
    #include "msepilog.h"       /* Epilog for microsoft PM comiler */
#endif
}

#if (KS_NUM_INTS>16)
KS_INTFN_DECLARE(rtip_c_isr_16)
{
    KS_ENTER_ISR();
    ks_dispatch_interrupt(16);
    KS_EXIT_ISR();
}
#endif

#if (KS_NUM_INTS>17)
KS_INTFN_DECLARE(rtip_c_isr_17)
{
    KS_ENTER_ISR();
    ks_dispatch_interrupt(17);
    KS_EXIT_ISR();
}
#endif

#if (KS_NUM_INTS>18)
KS_INTFN_DECLARE(rtip_c_isr_18)
{
    KS_ENTER_ISR();
    ks_dispatch_interrupt(18);
    KS_EXIT_ISR();
}
#endif

#if (KS_NUM_INTS>19)
KS_INTFN_DECLARE(rtip_c_isr_19)
{
    KS_ENTER_ISR();
    ks_dispatch_interrupt(19);
    KS_EXIT_ISR();
}
#endif

#if (KS_NUM_INTS>20)
KS_INTFN_DECLARE(rtip_c_isr_20)
{
    KS_ENTER_ISR();
    ks_dispatch_interrupt(20);
    KS_EXIT_ISR();
}
#endif

#if (KS_NUM_INTS > 21)
#error Too many interrupts. Make more interrupt stubs.
#endif

/* ********************************************************************   */
/* intfn_array (pointers to top level interrupt service routines)         */
/* ********************************************************************   */
#if (KS_USE_ASM_INTERRUPTS)
RTIP_CDECL(void rtip_a_isr_0(void);)
RTIP_CDECL(void rtip_a_isr_1(void);)
RTIP_CDECL(void rtip_a_isr_2(void);)
RTIP_CDECL(void rtip_a_isr_3(void);)
RTIP_CDECL(void rtip_a_isr_4(void);)
RTIP_CDECL(void rtip_a_isr_5(void);)
RTIP_CDECL(void rtip_a_isr_6(void);)
RTIP_CDECL(void rtip_a_isr_7(void);)
RTIP_CDECL(void rtip_a_isr_8(void);)
RTIP_CDECL(void rtip_a_isr_9(void);)
RTIP_CDECL(void rtip_a_isr_10(void);)
RTIP_CDECL(void rtip_a_isr_11(void);)
RTIP_CDECL(void rtip_a_isr_12(void);)
RTIP_CDECL(void rtip_a_isr_13(void);)
RTIP_CDECL(void rtip_a_isr_14(void);)
RTIP_CDECL(void rtip_a_isr_15(void);)

RTIP_CDECL(void rtip_a_isr_16(void);)
RTIP_CDECL(void rtip_a_isr_17(void);)
RTIP_CDECL(void rtip_a_isr_18(void);)
RTIP_CDECL(void rtip_a_isr_19(void);)
RTIP_CDECL(void rtip_a_isr_20(void);)

KS_INTFN_POINTER (intfn_array[KS_NUM_INTS]) =
{
    rtip_a_isr_0, rtip_a_isr_1, rtip_a_isr_2, rtip_a_isr_3,
    rtip_a_isr_4, rtip_a_isr_5, rtip_a_isr_6, rtip_a_isr_7,
#if (KS_NUM_INTS>8)
    rtip_a_isr_8, rtip_a_isr_9,
    rtip_a_isr_10, rtip_a_isr_11, rtip_a_isr_12, rtip_a_isr_13,
    rtip_a_isr_14, rtip_a_isr_15
#endif
#if (KS_NUM_INTS>16)
    , rtip_a_isr_16, rtip_a_isr_17,
    rtip_a_isr_18, rtip_a_isr_19, rtip_a_isr_20,
#endif
    };
#if (KS_NUM_INTS>21)
#error Make more asm interrupt routines, or increase number of hooks in arry
#endif
#else
/* Initialize an array of these functions   */
KS_INTFN_POINTER(intfn_array[KS_NUM_INTS]) =
{
    rtip_c_isr_0,rtip_c_isr_1,rtip_c_isr_2,rtip_c_isr_3,
    rtip_c_isr_4,rtip_c_isr_5,rtip_c_isr_6,rtip_c_isr_7,
    rtip_c_isr_8,rtip_c_isr_9,rtip_c_isr_10,rtip_c_isr_11,
    rtip_c_isr_12,rtip_c_isr_13,rtip_c_isr_14,rtip_c_isr_15
#if (KS_NUM_INTS>16)
    ,rtip_c_isr_16
#endif
#if (KS_NUM_INTS>17)
    ,rtip_c_isr_17
#endif
#if (KS_NUM_INTS>18)
    ,rtip_c_isr_18
#endif
#if (KS_NUM_INTS>19)
    ,rtip_c_isr_19
#endif
#if (KS_NUM_INTS>20)
    ,rtip_c_isr_20
#endif
#if (KS_NUM_INTS>21)
#error Make more initializers to intfn_array[] (osint.c).
#endif
    };
#endif

/* ********************************************************************    */
/* HOOK/RESTORE INTERRUPT                                                  */
/* ********************************************************************    */
/* ks_hook_interrupt() - hook interrupt                                    */
/*                                                                         */
/*   This is called by device drivers to install their interrupt service   */
/*   routines in the interrupt vector table and enable the interrupts. The */
/*   routine takes the hardware interrupt number and the address of the    */
/*   interrupt handler. In the PC based reference port the handler is      */
/*   installed in the interrupt table and the irq is enabled through the   */
/*   8259.                                                                 */
/*                                                                         */

#if (defined(SEGMC16))
RTIP_BOOLEAN ks_hook_interrupt(int irq, PFVOID piface,
                          RTIPINTFN_POINTER c_strategy,
                          RTIPINTFN_POINTER c_interrupt, int c_arg)
{
}


#else
RTIP_BOOLEAN ks_hook_interrupt(int irq, PFVOID piface,
                          RTIPINTFN_POINTER c_strategy,
                          RTIPINTFN_POINTER c_interrupt, int c_arg)
{
int vector_no;
KS_INTFN_POINTER(isr_func);
KS_INTERRUPT_CONTEXT sp;

#if (!INCLUDE_SLIP && !INCLUDE_CSLIP && !INCLUDE_PPP && !INCLUDE_TASK_ISRS)
    ARGSUSED_PVOID(piface);
    ARGSUSED_PVOID(c_interrupt);
#endif

    if (irq > (KS_NUM_INTS-1))
        return(FALSE);

    KS_TL_SPLX(sp)

    /* Save the function to call and the argument; this will be used
       by ks_dispatch_interrupt */
#if (INCLUDE_SLIP || INCLUDE_CSLIP || INCLUDE_PPP || INCLUDE_TASK_ISRS)
    /* If an interface is provided then we will wake up the interrupt   */
    /* dispatch task assigned to this task                              */
    rtip_irq_iface[irq] = piface;   /* Signal this iface if non null */
    if (piface)
    {
        ((PIFACE)piface)->irq_no = irq; /* The interrupt task passes this */
                                        /* to the Interrupt service routine   */
        /* wake up the interrupt task to start                                */
        ks_invoke_interrupt(piface);

    }

    /* save the name of the routine to be call from the interrupt   */
    rtip_isr_interrupt[irq] = c_interrupt;  /* This is usually null unless */
                                        /* the driver has to do special    */
                                        /* processing to assure interrupts */
                                        /* won't be rearmed                */
#endif

    /* save the name of the routine to be called either from the interrupt   */
    /* (if INCLUDE_TASK_ISRS is 0) or from the interrupt tasks               */
    /* (if INCLUDE_TASK_ISRS is 1)                                           */
    rtip_isr_strategy[irq] = c_strategy;
    rtip_args[irq] = c_arg;

    isr_func = intfn_array[irq];

#if (AT_MOTHERBOARD )
    /* map the interrupt number to a position in the int table   */
    if ( irq > 7)
        vector_no = vec_8259b + ( irq - 8);
    else
        vector_no = vec_8259a + irq;
#endif

#if (defined(EMBOS))
    oldint[irq] = rtip_getvect (vector_no);
    /* Install the vector. This assumes one adapter for now   */
    rtip_setvect (vector_no, isr_func);

#endif

    KS_TL_SPL(sp)

    /* mask on the interrupt 8259. pc specific   */
    ks_mask_isr_on(irq);

    /* clear any stray interrupts
       NOTE: RTTARGET changed to disable interrupts when leaving
             this routine */
    ks_clear_isr(irq);

    return(TRUE);
}
#endif

/* ********************************************************************   */
/* ks_restore_interrupt() - restore interrupt vector                      */
/*                                                                        */
/*   Restores routines in the interrupt vector table to their origional   */
/*   values before ks_hook_interrupt was called.                          */
/*   This routine is called before program is exited.                     */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
void ks_restore_interrupts(void)
{
#if (!defined(SEGMC16))
int irq;
int vector_no;
KS_INTFN_POINTER(isr_func);

    for (irq = 0; irq < KS_NUM_INTS; irq++)
    {
        if (oldint[irq])
        {
            /* map the interrupt number to a position in the int table   */
#            if (AT_MOTHERBOARD )
                if ( irq > 7)
                    vector_no = vec_8259b + ( irq - 8);
                else
                    vector_no =  vec_8259a + irq;
#            endif

#            if (defined(EMBOS))
                isr_func = oldint[irq];
               rtip_setvect (vector_no, isr_func);
#           endif
        }
    }
#endif  /* !defined(SEGMC16) */
}


#if (!defined(SEGMC16))
/* ********************************************************************   */
/* TURN ON/OFF INTERRUPT                                                  */
/* ********************************************************************   */
/* turn on the interrupt thru 8259 interrupt controller                   */
void ks_mask_isr_on(int irq)
{
    if ( irq > 7)      /* if using second int controller */
    {
        /* Mask in the slave   */
       OUTBYTE(MR8259B, INBYTE(MR8259B) & ~(1 << ( irq - 8)));
       /* Enable IRQ 2 on the master  */
       OUTBYTE (MR8259A, INBYTE(MR8259A) & ~(1 << 2));
    }
    else
    {
       OUTBYTE (MR8259A, INBYTE(MR8259A) & ~(1 << irq));
    }
}

/* turn off the interrupt thru 8259 interrupt controller   */
void ks_mask_isr_off(int irq)
{
    if ( irq > 7)      /* if using second int controller */
    {
       OUTBYTE(MR8259B, INBYTE(MR8259B) | (1 << ( irq - 8)));
    }
    else
    {
       OUTBYTE (MR8259A, INBYTE(MR8259A) | (1 << irq));
    }
}

/* clear any stray interrupts or acknowledge to interrupt controller   */
void ks_clear_isr(int irq)
{
#if (AT_MOTHERBOARD)
    if ( irq > 7)
        OUTBYTE (IR8259B, EOI);
    OUTBYTE (IR8259A, EOI);
#endif  /* AT_MOTHERBOARD */
}
#endif  /* !MC68K && !defined(SEGMC16) */

#endif /*OS*/
