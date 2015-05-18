/*                                                                            */
/* OSTYPE.H  - TYPES header file                                              */
/*                                                                            */
/*   EBS - RTIP                                                               */
/*                                                                            */
/*   Copyright Peter Van Oudenaren , 1993                                     */
/*   All rights reserved.                                                     */
/*   This code may not be redistributed in source or linkable object form     */
/*   without the consent of its author.                                       */
/*                                                                            */
/*    Module description:                                                     */
/*        This file contains defines, types and includes for specific kernels */
/*                                                                            */

#ifndef __RTIP_TYPES__
#define __RTIP_TYPES__ 1

/* ********************************************************************   */
/* SET DEFINES FOR KERNEL BASED UPON KERNEL AND COMPILER                  */
/* ********************************************************************   */

#if (defined(__ghs_board_is_nec_vr41xx))

#define EMBOS           1
#define KS_SUPPORTS_DOS_CALLS 0 /* DOS system calls */
#define AT_MOTHERBOARD  0 /* This means AT motherboard - Advanced Technology */
#define IX86            0 /* Intel X86 (80186, 80286, 80386, etc) */

#else
#error  OSTYPE.H - It is assumed that MIPS_GHS is defined
#endif

#include      "rtos.h"

#endif      /* __RTIP_TYPES__ */
