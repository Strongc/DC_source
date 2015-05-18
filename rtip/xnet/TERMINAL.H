/*                                                                      */
/* TERMINAL.H - Header file for terminal input and output functions     */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright EBSNet Inc. 1998                                           */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/* This file provides a simple terminal package.                        */
/*                                                                      */

#ifndef __TERMINALH__
#define __TERMINALH__ 1

/*#include "xnconf.h"     */
/*#if (!BUILD_NEW_BINARY) */
/*#include "ostype.h"     */
/*#else                   */
/*#include "rtipapi.h"    */
/*#endif                  */

#if (KS_USE_TC_PRINTF)
#include <stdarg.h>         /* needed for printf */
#endif

/* ************************************************************************    */
/* TERMINAL I/O CONFIGURATION                                                  */
/* ************************************************************************    */

/* We have provided low level drivers for the following devices:               */
/*                                                                             */
/* CONDEV_VT100C   - The rtip vt100 terminal emulator                          */
/* CONDEV_PCVIDEO  - Direct mapped pc video                                    */
/* CONDEV_PC_STDIO - Use putch() for output. Compiler specific kbhit for input */
/* CONDEV_TELNET   - Send over a telnet port                               */
/* CONDEV_16550_TX - Send over the 16550 port of a Toshiba TX39 eval board */
/* CONDEV_16550_1  - Send over Com port 1 of a PC                          */
/* CONDEV_16550_2  - Send over Com port 2 of a PC                          */
/* CONDEV_16550_3  - Send over Com port 3 of a PC                          */
/* CONDEV_16550_4  - Send over Com port 4 of a PC                          */
#if (defined(MCF52XX))
/* CONDEV_MCF52XX      - Send over Com port MCF5272 UART   */
#endif
/* Define the output device   */
#define CONDEV_VT100C   1   /* - The rtip vt100 terminal emulator */
#define CONDEV_PCVIDEO  2   /* - Direct mapped pc video */
#define CONDEV_PC_STDIO 3   /* - putch() for output, kbhit for input */
#define CONDEV_TELNET   5   /* - Send over a telnet port */
#define CONDEV_16550_TX 6   /* - Send over the 16550 port of a Toshiba TX39 eval board */
#define CONDEV_16550_1  7   /* - Send over Com port 1 of a PC */
#define CONDEV_16550_2  8   /* - Send over Com port 2 of a PC */
#define CONDEV_16550_3  9   /* - Send over Com port 3 of a PC */
#define CONDEV_16550_4  10  /* - Send over Com port 4 of a PC */
#if (defined(MCF5272) && !EMBOS)
#define CONDEV_MCF5272     23
#endif

#if (defined(TARGET_SWAT9155800) )
#define CONDEV_AT9155800 31 /* - Use AT91M55800 serial driver */
#endif

#define CONDEV_SMX_BSP  32  /* Send over Com port using SMX BSP routines in new SMX ports */ /*DM: 11-27-02: added */

#define CONDEV_NODEV    99  /* - No output device but don't complain about it */

/* *********************************************************************   */
/* Select the console driver.
   Note: You may choose to override the selected console driver */
/* *********************************************************************   */
#if (defined(MCF5272) && !EMBOS)
#define CONSOLE_DEVICE CONDEV_MCF5272
#define TERMPORT 2 /* Not used right now but will be */

#elif (defined(WIN32RTIP))
#define CONSOLE_DEVICE CONDEV_PC_STDIO

#elif (defined(TARGET_SWAT9155800) )
#define CONSOLE_DEVICE CONDEV_AT9155800

#elif (defined(ARM_IAR) || defined(__ghs_board_is_nec_vr41xx))  /*OS*/ /* added */
#define CONSOLE_DEVICE CONDEV_NODEV            /*OS*/ /* added */

#else
/* Note: you could change this to any of the supported output drivers
   such as TELNET, 16550 etcs */
#if (INCLUDE_TELNET_CLI)
#define CONSOLE_DEVICE CONDEV_VT100C
#else
#define CONSOLE_DEVICE CONDEV_PCVIDEO  /* Default to PCVIDEO */
#endif
#endif

#if (!AT_MOTHERBOARD )
/* a pc specific VT100 emulator       */
#define INCLUDE_VT100C  0    /* DO NOT CHANGE */
#elif (CONSOLE_DEVICE == CONDEV_VT100C)
/* a pc specific VT100 emulator used by Telnet Client   */
#define INCLUDE_VT100C  1
#else
#define INCLUDE_VT100C  0
#endif

/* *********************************************************************   */
#define TERMINAL_UP_ARROW       1
#define TERMINAL_DOWN_ARROW     2

/* *********************************************************************   */
/* API MACROS                                                              */
/* *********************************************************************   */

/* These calls are optimized for use with consoles.  They are not meant for   */
/* other abstracted stream I/O.  We don't use abstracted stream I/O, and      */
/* won't in the near future.                                                  */
/* All the (file) I/O oriented applications/servers we have                   */
/* Are dedicated to a certain I/O device, i.e. they know at build time what   */
/* they are going to be attached to.  The only thing for which we need this   */
/* kind of I/O device abstraction is the console.                             */

#ifdef __cplusplus
extern "C" {
#endif

void    tm_cputs(PFCCHAR string);
int     tm_getch(void);
int     tm_kbhit(void);
void    tm_putc(char ch);
void    tm_puts(PFCCHAR string);
int     tm_promptstring( PFCHAR string, RTIP_BOOLEAN handle_arrows);
#if (INCLUDE_SSL)
/* [i_a] 'limited' eq. which prevent buffer overrun   */
int     tml_promptstring(PFCHAR string, int size, RTIP_BOOLEAN handle_arrows);
int     tml_gets(PFCHAR string, int size);
#endif
#if (KS_USE_TC_PRINTF)
int     tm_printf(PFCCHAR fmt, ...);
int     tm_vprintf(PFCCHAR f, va_list ap);          /* vprintf.c */
#else
#define tm_printf           printf
#endif
int     tm_gets( PFCHAR string);

/* [i_a] 'limited' eq. which prevent buffer overrun   */
int     tml_promptstring(PFCHAR string, int size, RTIP_BOOLEAN handle_arrows);
int     tml_gets(PFCHAR string, int size);

#if (CONSOLE_DEVICE==CONDEV_TELNET)
/* Called by the application to tel the telnet server that it can print   */
void    tm_stack_is_up(void);
#endif

#ifdef __cplusplus
}
#endif

#endif      /* TERMINALH */

