/*                                                                      */
/* OSENV.C - OS PORTING functions                                       */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*      This module consists of all the routines that need porting      */
/*      to the different compilers and hardware platforms               */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtip.h"
#if (defined(__BORLANDC__) )
#include <dos.h>        /* FP_SEG etc */
#endif
/* ********************************************************************   */
/* ********************************************************************   */
/* ********************************************************************   */
/* ********************************************************************   */
/* MEASURE INTERRUPT LATENCY (test code only - no need to port)           */
/* ********************************************************************   */
/* ********************************************************************   */

/* test code to pulse a pin on LPT1, COM1, and/or COM2         */
/* when interrupts are enabled or disabled; this should be set */
/* to 0 during normal operation                                */
#define TEST_INT_LATENCY 0      /* turn on to test interrupt latency; */
                                /* High C/C++ users may have problems if 1   */

#if (TEST_INT_LATENCY)
#define SIGNAL_COM1 0           /* set if want to strobe pin 1 of COM1 */
#define SIGNAL_COM2 0           /* set if want to strobe pin 1 of COM1 */
#define SIGNAL_LPT1 1           /* set if want to strobe pin 1 of LPT1 */

/* Set these according to your hardware settings   */
#define COM1_IO_BASE 0x3f8
#define COM2_IO_BASE 0x2f8
#define LPT1_IO_BASE 0x3bc

#define MASK_INT_FLAG 0x0200

#define DO_LATENCY_SIGNAL() pulse_latency_signal()

void pulse_latency_signal(void)
{
#if (SIGNAL_COM1)
    OUTBYTE(COM1_IO_BASE+4, 0x03);
    OUTBYTE(COM1_IO_BASE+4, 0x00);
#endif
#if (SIGNAL_COM2)
    OUTBYTE(COM2_IO_BASE+4, 0x03);
    OUTBYTE(COM2_IO_BASE+4, 0x00);
#endif
#if (SIGNAL_LPT1)
    OUTBYTE(LPT1_IO_BASE+2, 0x0d);
    OUTBYTE(LPT1_IO_BASE+2, 0x0c);
#endif
}
#endif      /* TEST_INT_LATENCY */

/* ********************************************************************   */
/* ********************************************************************   */
/* ASSEMBLY I/O                                                           */
/* ********************************************************************   */
/* ********************************************************************   */
#if (defined(__BORLANDC__) )
#define ASSEMBLY_IO 1       /* do assembly versions of device driver I/O */
#else
#define ASSEMBLY_IO 0       /* do C versions of device driver I/O */
                            /* DO NOT CHANGE   */
#endif

/* ********************************************************************   */
/* ********************************************************************   */
/* spl and splx                                                           */
/* ********************************************************************   */
/* ********************************************************************   */

/* ********************************************************************   */
/* push and pop flag values                                               */
KS_INTERRUPT_CONTEXT ks_splx(void)
{
KS_INTERRUPT_CONTEXT oldlevel;

#if (defined(EMBOS))
    /* Segger doesn't need oldlevel but we need to return something   */
    oldlevel = 0;
    OS_DI();
/*  OS_SDI();   */

#elif (defined(__BORLANDC__) )
__asm {
    pushf
    cli
    pop ax
    mov oldlevel, ax
    }

#else
    #error Must implement splx in OSENV.C
#endif

    return(oldlevel);
}

/* ********************************************************************   */
/* Reenable interrupts to the previous stats                              */
void ks_spl(KS_INTERRUPT_CONTEXT oldlevel)
{
#if (TEST_INT_LATENCY)
    if (oldlevel & MASK_INT_FLAG) /* if entered with IF set */
        DO_LATENCY_SIGNAL();
#endif

#if (defined(EMBOS))
    oldlevel = oldlevel;
/*  OS_RI();   */
/*    OS_EI(); */
      OS_RestoreI();

#elif (defined(__BORLANDC__) )
__asm {
    mov ax, oldlevel
    push ax
    popf
    }

#else
    #error Must implement spl in OSENV.C
#endif
}

/* ********************************************************************   */
/* ********************************************************************   */
/* ENABLE/DISABLE INTERRUPTS                                              */
/* enable interrupts inside an interrupt service routine                  */
/* ********************************************************************   */
/* ********************************************************************   */

/* enable interrupts inside an interrupt service routine   */
void ks_enable(void)        /*__fn__*/
{

#if ( defined(__BORLANDC__) )
__asm sti

#elif (defined(EMBOS))
    OS_EI();
/*  OS_ri();   */

#else
    #error Must implement ks_enable in OSENV.C

#endif

}

/* ********************************************************************   */
void ks_disable(void)
{
#if (IX86)
    __asm cli

#elif (defined(EMBOS))
    OS_DI();
/*  OS_di();   */

#else
#error Implement ks_disable for your CPU in OSENV.C
#endif
}

/* ********************************************************************   */
/* ********************************************************************   */
/* WARRAY_2_LONG and LONG_2_WARRAY                                        */
/* ********************************************************************   */
/* ********************************************************************   */

#if (LONG_W_ALLIGN )
/* Move two shorts into a long. Shorts are not long aligned   */
#ifndef WARRAY_2_LONG
unsigned long WARRAY_2_LONG(unsigned short KS_FAR *x)
{
unsigned long l;
#if defined(ARM)
    l = ((PFBYTE)x)[0]
      | (((PFBYTE)x)[1]<<8)
      | (((PFBYTE)x)[2]<<16)
      | (((PFBYTE)x)[3]<<24);
#else
    tc_movebytes((unsigned char *)&l, (PFBYTE)x, 4);
#endif
    return(l);
}
#endif

/* Move a long into two shorts. Shorts are not long aligned   */
void LONG_2_WARRAY(unsigned short KS_FAR *to, unsigned long val)
{
    tc_movebytes((PFBYTE)to, (unsigned char *)&val, 4);
}

#elif (LONG_W_ALLIGN)
    #error Please implement LONG_2_WARRAY and WARRAY_2_LONG in OSENV.C
#endif

/* ********************************************************************   */
/* ********************************************************************   */
/* I/O ROUTINES                                                           */
/* ********************************************************************   */
/* ********************************************************************   */


#if (EMBOS)
/* Seger defines di as a C macro which interferes with the inline asm   */
#ifdef di
#undef di
#endif
#endif  /* EMBOS */


/* tbd: rep move simulated in 'C'   */
void insw(IOADDRESS ioaddr, PFWORD dst, int n)
{
#if (!ASSEMBLY_IO)
    while (n--)
        *dst++ = (word)INWORD(ioaddr);

#else
#if ( defined(__BORLANDC__) )
static word near s,o,_n, _i;
__asm
{
    pushf
    cli
}
    _n = (word)n;
    s = (word)_FP_SEG(dst);
    o = (word)_FP_OFF(dst);
    _i = ioaddr;
__asm
{
    pop ax              /* get flags back into ax */
    push    es
    push    di
    push    cx
    push    dx
    mov     dx, ds:_i
    mov     es, ds:s
    mov     di, ds:o
    mov     cx, ds:_n
    push ax             /* put flags back */
    popf
    cld
    rep     insw
    pop     dx
    pop     cx
    pop     di
    pop     es
}
#else
    #error ASSEMBLY_IO must be set to 0
#endif /* __BORLANDC__ || _MSC_VER */
#endif

}

void insb(IOADDRESS ioaddr, PFBYTE dst, int n)
{
#if (!ASSEMBLY_IO)
    while (n--)
        *dst++ = INBYTE(ioaddr);
#elif ( defined(__BORLANDC__) )
static word near s,o,_n, _i;
__asm
{
pushf
cli
}
    _n = (word)n;
    _i = ioaddr;

    s = (word)_FP_SEG(dst);
    o = (word)_FP_OFF(dst);

__asm {
    pop ax              /* get flags back into ax */
    push    es
    push    di
    push    cx
    push    dx
    mov     dx, ds:_i
    mov     es, ds:s
    mov     di, ds:o
    mov     cx, ds:_n
    push ax             /* put flags back */
    popf
    cld
    rep     insb
    pop     dx
    pop     cx
    pop     di
    pop     es

}
#else
    #error ASSEMBLY_IO must be set to 0
#endif /* __BORLANDC__ || _MSC_VER */

}

void outsw(IOADDRESS ioaddr, PFWORD src, int n)
{
#if (!ASSEMBLY_IO)
    while (n--)
    {
        OUTWORD(ioaddr, *src++);
    }

#else
#if ( defined(__BORLANDC__) )
static word near s,o,_n,_i;
__asm {
pushf
cli
}
_n = (word)n;
_i = ioaddr;

s = (word)_FP_SEG(src);
o = (word)_FP_OFF(src);

__asm {
    pop ax              /* get flags back into ax */
    push    es
    push    cx
    push    dx
    push    ds
    push    si
    mov     dx, ds:_i
    mov     cx, ds:_n
    mov     si, ds:o
    mov     ds, ds:s
    push ax             /* put flags back */
    popf
    cld
    rep     outsw
    pop     si
    pop     ds
    pop     dx
    pop     cx
    pop     es
}
#else
    #error ASSEMBLY_IO must be set to 0
#endif /* __BORLANDC__ || _MSC_VER */
#endif
}

void outsb(IOADDRESS ioaddr, PFCBYTE src, int n)
{
#if (!ASSEMBLY_IO)
    while (n--)
        OUTBYTE(ioaddr, *src++);
#else
#if ( defined(__BORLANDC__) )
static word near s,o,_n,_i;
__asm
{
    pushf
    cli
}
    _n = (word)n;
    _i = ioaddr;

    s = (word)_FP_SEG(src);
    o = (word)_FP_OFF(src)  ;
__asm
{
    pop ax              /* get flags back into ax */
    push    es
    push    cx
    push    dx
    push    ds
    push    si
    mov     dx, ds:_i
    mov     cx, ds:_n
    mov     si, ds:o
    mov     ds, ds:s
    push ax             /* put flags back */
    popf
    cld
    rep     outsb
    pop     si
    pop     ds
    pop     dx
    pop     cx
    pop     es
}
#else
    #error ASSEMBLY_IO must be set to 0
#endif /* __BORLANDC__ || _MSC_VER */
#endif
}

/* ********************************************************************   */
/* ********************************************************************   */
/* SLOW DOWN I/O (used for fast machines, i.e. a Pentium)                 */
/* ********************************************************************   */
/* ********************************************************************   */
void io_delay(void)     /*__fn__*/
{
#if (AT_MOTHERBOARD)
    /* read the NMI Status Register - this delays ~1 usec;   */
    /* PC specific                                           */
    INBYTE(0x61);
#endif
}

/* ********************************************************************   */
/* ********************************************************************   */
/* MEMORY ADDRESS CONVERSION ROUTINES                                     */
/* ********************************************************************   */
/* ********************************************************************   */

/* ********************************************************************   */
/* ***********************                                                */
/*  !!!!!!!!!!!!!!!!!!!! N    O    T   E    !!!!!!!!!!!!!!!!!!!!!!!!!!    */
/* *** These routines are only needed to support shared memory support    */
/* *** use by some of the drivers                                         */

#if (INCLUDE_ED || USE_PCVID_OUTPUT || INCLUDE_LANCE || INCLUDE_RTLANCE || INCLUDE_PCMCIA || INCLUDE_ERTFS_MEM_MAP)

/* ********************************************************************   */
/* Take the address of a pointer, (virt) and a physical address
   (0xd0000) and map in 64 k  */
int phys_to_virtual(PFBYTE * virt, unsigned long phys)
{
#if (defined(__BORLANDC__) )
unsigned long temp;
#endif

#if (defined(__BORLANDC__) )
    /* Straight real mode versions   */
    /* Take the address of a pointer, (virt) and a physical address
       (0xd0000) and map in 64 k  */
    /* Real mode version   */
    temp = phys & 0xf;
    phys &= 0xfffffff0ul;
    phys <<= 12;
    phys |= temp;
    *virt = (PFBYTE)phys;

#else
#error: Implement phys_to_virtual
#endif

    return(1);
}

/* ********************************************************************   */
unsigned long kvtop(PFBYTE p)
{
dword l;
word w1,w2;

#if (defined(__BORLANDC__) )
    /* Straight real mode versions     */
    l = (dword) p;
    w1 = (word)(l>>16);
    w2 = (word)l;
    l = (((dword)w1) << 4) + w2;

#else
#error Implement kvtophys
#endif

    return(l);
}
#endif   /* INCLUDE_ED or INCLUDE_LANCE or INCLUDE_PCMCIA */

#if (DOS_REAL_MODE)
/* ********************************************************************   */
/****** Interrupt Controller Registers *****                              */
#define MR8259A     0x21    /* interrupt controller 1 */
#define IR8259A     0x20
#define MR8259B     0xA1    /* interrupt controller 2 */
#define IR8259B     0xA0

/****** 8259 REGISTER DEFINITIONS *****  */
#define EOI     0x20        /* end of interrupt */
#define RIRR    0x0a        /* read interrupt request register */

int os_irqtov(int irq)
{
    if ( irq > 7) {
        return (0x70 + (irq - 8));
    }
    return (irq + 8);
}

void os_setvect(int vector_no, OS_ISR_POINTER isr_func)
{
    _dos_setvect(vector_no, isr_func);
}

void os_savevect(int vector_no, OS_ISR_SAVE *vect)
{
    *vect = (OS_ISR_SAVE)_dos_getvect(vector_no);
}

void os_restorevect(int vector_no, OS_ISR_SAVE *vect)
{
    _dos_setvect(vector_no, *vect);
}

void os_mask_isr_on(int irq)
{
    if ( irq > 7) {
    OUTBYTE (MR8259B, INBYTE(MR8259B) & ~(1 << ( irq - 8)));
        return;
    }
    OUTBYTE (MR8259A, INBYTE(MR8259A) & ~(1 << irq));
}

void os_mask_isr_off(int irq)
{
    if ( irq > 7) {
        OUTBYTE (MR8259B, INBYTE(MR8259B) | (1 << ( irq - 8)));
        return;
    }
    OUTBYTE (MR8259A, INBYTE(MR8259A) | (1 << irq));
}

void os_clear_isr(int irq)
{
    if ( irq > 7) {
        OUTBYTE (IR8259B, EOI);
    }
    OUTBYTE (IR8259A, EOI);
}

#else
/* ********************************************************************   */
int  os_irqtov(int irq) {return(irq);}
void os_setvect(int vector_no, OS_ISR_POINTER isr_func)
{
    ARGSUSED_INT(vector_no);
    ARGSUSED_PVOID(isr_func);
}
void os_savevect(int vector_no, OS_ISR_SAVE *vect)
{
    ARGSUSED_INT(vector_no);
    ARGSUSED_PVOID(vect);
}
void os_restorevect(int vector_no, OS_ISR_SAVE *vect)
{
    ARGSUSED_INT(vector_no);
    ARGSUSED_PVOID(vect);
}
void os_mask_isr_on(int irq) {ARGSUSED_INT(irq);}
void os_mask_isr_off(int irq) {ARGSUSED_INT(irq);}
void os_clear_isr(int irq) {ARGSUSED_INT(irq);}
#endif

