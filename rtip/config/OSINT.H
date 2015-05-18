/*                                                                        */
/* osint.h                                                                */
/*                                                                        */
/*   EBS - RTIP                                                           */
/*                                                                        */
/*   Copyright ebsnet Inc., 1993,1994,1995,1996                           */
/*   All rights reserved.                                                 */
/*   This code may not be redistributed in source or linkable object form */
/*   without the consent of its author.                                   */
/*                                                                        */
/*                                                                        */
/*    Module description:                                                 */
/*         This file contains macros and defines related to interrupts    */
/*         It must be modified to support the kernel/cpu/compiler         */
/*   Date                    Supported Kernel     #define to enable       */
/*                                                                        */
/*                                                                        */

/* Porting issues for this file:
**
** INTERRUPT       - Set this equal to your compilers interrupt modifier. If
**                   there is no special interrupt modifier leave it blank
**                   (eg: #define INTERRUPT)
**
** Interrupt service prolog & epilogue routines macros:
**
** KS_INTFN_DECLARE(x)     - Interrupt function declaration. where X is the
**                         - function.
** KS_INTFN_POINTER(x)     - Pointer to interrupt function.
**
** KS_ISRSV_DECLARE(x)
**
** KS_ISRSV_POINTER(x)      - Pointer to structure or type used to save an
**                                old ISR when a user ISR is hooked
**
** KS_USE_ASM_INTERRUPTS   - If 1 then all interrupts are in 'C' and vectors
**                           are hooked follwoing the conventions in interrupt.c
**
**
** Interrupt service prolog & epilogue routines macros:
** KS_ENTER_ISR()          - Interrupt service routine prolog. Most kernels
**                         - must be told when an interrupt service routine
**                         - is entered, this macro is invoked to do so.
** KS_EXIT_ISR()           - Interrupt service routine epilog. Most kernels
**                         - must be told when an interrupt service routine
**                         - is left, this macro is invoked to do so.
**
**
**/

/* ********************************************************************   */
/* INTERRUPT                                                              */
/* ********************************************************************   */
#if (defined(__ghs_board_is_nec_vr41xx))
/* Define things just to get it to compile                                */
#ifndef INTERRUPT
#define INTERRUPT     /* Function interrupt modifier */
#endif
#define KS_INTFN_DECLARE(x) void x(void)
#define KS_INTFN_POINTER(x) void (*x)(void)
#define KS_ISRSV_DECLARE(x)
#define KS_ISRSV_POINTER(x)
#define KS_USE_ASM_INTERRUPTS 0

#define KS_ENTER_ISR()      /* No ISR ENTER/EXIT pair needed */
#define KS_EXIT_ISR()

#define INCLUDE_TASK_ISRS 0
#define KS_NUM_INTS 0

#define DRIVER_MASK_ISR_OFF(irq)
#define DRIVER_MASK_ISR_ON(irq)

#else
#error OSINT.H - It is assumed that MIPS_GHS is defined
#endif
