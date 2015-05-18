/*                                                                         */
/* TERMINAL.C - Terminal input and output functions                        */
/*                                                                         */
/* EBS - RTIP                                                              */
/*                                                                         */
/* Copyright EBSNet Inc. 1998                                              */
/* All rights reserved.                                                    */
/* This code may not be redistributed in source or linkable object form    */
/* without the consent of its author.                                      */
/*                                                                         */
/* This file provides a simple terminal package that the RTIP applications */
/* use to communicate with the user. If you can provide the porting layer  */
/* of this file then you can run the rtip demos interactively. You can     */
/* also provide an output port for diagnostic messages.                    */
/* The functions that must be ported are tm_putc(), tm_getch() and         */
/* tm_kbhit(). The other functions in this file are derived from these     */
/* functions.                                                              */
/* Note: tm_cputs() is implemented by calling tm_puts() multiple times.    */
/* In the interest of improving performance you can optionally port        */
/* tm_cputs() to send a string directly to your console port.              */
/*                                                                         */
/* See terminal.h for low level drivers supported.                         */
/*                                                                         */

/*
 * The following low level functions must be modified for your environment.
 * tm_open()  - Make sure the io channel is open
 * tm_putc()  - Output a single character to a terminal.
 * tm_getch() - Receive a single character from a terminal.
 * tm_kbhit() - Check and see if there are any characters waiting.
 *
 * It is not required that you modify the following function but performance
 * will improve if you do so.
 * tm_cputs() - Output a string to a terminal.
 *
 * The following functions are portable
 * tm_puts() - Output a string to a terminal, with a newline at the end.
 * tm_gets() - Input a string from a terminal.
 * tm_printf() - Output a formatted string to a terminal.
 * tm_promptstring() - Allow the user to edit a string interactively.
 */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
#include "terminal.h"

/* ********************************************************************   */
/* Select the console driver. For some ports we already know the driver to
   use based on the port. We put them in here. The final elseif should
   be where you define the driver you need if it does not fit any of the
   others
   Note: You may choose to override the selected console driver
*/

/* Include header files and declare functions and constants based on the
   selected driver */

#if (CONSOLE_DEVICE==CONDEV_PCVIDEO)    /* - ebs screen IO EBS keyboard IO */
#include "pcvid.h"
#define IS_PCVID_DEVICE
#endif

#if (CONSOLE_DEVICE==CONDEV_KROS)
#include "kr_port.h"
#endif

#if (CONSOLE_DEVICE==CONDEV_VT100C)     /* - ebs vt100 emulator library */
#include "vt100.h"
#include "vt100c.h"
#endif

#if (CONSOLE_DEVICE==CONDEV_PC_STDIO)   /* - stdio. note kbhit is environment */
                                        /* - dependent   */
#include <stdio.h>
#endif

#if (CONSOLE_DEVICE==CONDEV_TELNET)     /* - telnet server */
#include "telnet.h"
 void tnserv_putc(char c);
 char tnserv_getch(void);
 int  tnserv_kbhit();

 extern RTIP_BOOLEAN KS_FAR telnet_spawned;
 extern PTELNET_CONTEXT telnet_console_context;
#endif

#if (CONSOLE_DEVICE==CONDEV_MCF52XX)
int  mcf5272_uart_device_open(int portno, int baud_rate, int wordlen, int parity, int stop, int polled, char modem);
void mcf5272_uart_device_polled_send(int portno, int len, char *p);
int  mcf5272_uart_device_polled_kbhit(int portno);
char mcf5272_uart_device_polled_getc(int portno);
void mcf5272_terminal_device_open(void);
#endif

#if (CONSOLE_DEVICE==CONDEV_SH_SCIF)
void open_SCIF(void);
void putc_SCIF(char c);
int  kbhit_SCIF(void);
char getch_SCIF(void);
#endif  /* CONDEV_SH_SCIF */

#if (IS_16550_DEVICE)
/* 16550 uart io package. See implementation at the end of the file   */
void putc_16550(char c);
int  kbhit_16550(void);
char getch_16550(void);
void open_16550(void);
#endif

#if (CONSOLE_DEVICE==CONDEV_EMBOS_GNU)
FILE *console_tty;
#endif

#if (CONSOLE_DEVICE==CONDEV_RTXC)
int rtxcchar = 0;
InitializeUART(){};
ReadUARTN(){};
WriteUARTN(){};
#endif

#if (CONSOLE_DEVICE==CONDEV_AT9155800)
#include <ioat91m55800.h>
static char inbuff_at91[80];
#define ATPORT 2
#endif


/* ********************************************************************   */
extern RTIP_BOOLEAN KS_FAR terminal_is_open;


/*************************************************************************
void  tm_open() - Initialize a terminal for communications.

 void tm_open()
    All of the IO routines call this routine first to make sure the
    terminal driver level is set up. Put any device specific setup
    in this module.

 Returns:
    Nothing
*************************************************************************/

static void tm_open(void)
{
    if (terminal_is_open)
        return;
    terminal_is_open = TRUE;

/* Do device specific inintialization if needed   */
#if (CONSOLE_DEVICE==CONDEV_VT100C)
    /* Nothing   */

#elif (CONSOLE_DEVICE==CONDEV_PCVIDEO)
    pcvid_init_term(0);

#elif (CONSOLE_DEVICE==CONDEV_RTXC)
/* we expect the console device is already opened by the kernel.  */

#elif (CONSOLE_DEVICE==CONDEV_KROS)
    /* KROS console is opened before main.   */

#elif (CONSOLE_DEVICE==CONDEV_TELNET)
    /* Nothing   */

#elif (IS_16550_DEVICE)
    open_16550();

#elif (CONSOLE_DEVICE==CONDEV_PC_STDIO)
    /* Nothing   */

#elif (CONSOLE_DEVICE==CONDEV_NODEV)       /* - USe ecos serial driver */
    /* Nothing   */

#elif (CONSOLE_DEVICE==CONDEV_RTKERNEL)
/* Nothing to do   */

#elif (CONSOLE_DEVICE==CONDEV_MCF52XX)
    mcf5272_terminal_device_open();

#elif (CONSOLE_DEVICE==CONDEV_SH_SCIF)
    open_SCIF();

#elif (CONSOLE_DEVICE==CONDEV_EMBOS_GNU)
    if (!(console_tty = fopen(TERMDEVICE,"rw+")))
     terminal_is_open = FALSE;
     
#elif (CONSOLE_DEVICE==CONDEV_AT9155800)
#if (ATPORT == 2)
  __APMC_PCER = 0x10;   /* enable Uart2 clock */
  __PIO_PER = 0x2000; __PIO_OER = 0x2000;  /* setup IO bit for FORCEON output */
  __PIO_SODR = 0x2000;  /* set FORCEON */
  __PIO_PDR = 0x600000; /* setup Uart2 Tx and Rx pins */

  __US_CR2 = 0x50;      /* enable and reset rx and tx */
  __US_MR2 = 0x28C0;    /* async mode, 8N2 */
  __US_BRGR2 = 8;      /* set baud 115200, regval = 14.7456mhz-xtal / 16 / baudrate */
  
  __US_RCR2 = 0;
  __US_RPR2 = (int) inbuff_at91;
  __US_RCR2 = 80;
#elif (ATPORT == 1)
  __APMC_PCER = 0x08;   /* enable Uart1 clock */
  __PIO_PER = 0x2000; __PIO_OER = 0x2000;  /* setup IO bit for FORCEON output */
  __PIO_SODR = 0x2000;  /* set FORCEON */
  __PIO_PDR = 0x000c0000; /* setup Uart1 Tx and Rx pins */

  __US_CR1 = 0x50;      /* enable and reset rx and tx */
  __US_MR1 = 0x28C0;    /* async mode, 8N2 */
  __US_BRGR1 = 8;      /* set baud 115200, regval = 14.7456mhz-xtal / 16 / baudrate */
  
  __US_RCR1 = 0;
  __US_RPR1 = (int) inbuff_at91;
  __US_RCR1 = 80;
#  else
  __APMC_PCER = 0x04;   /* enable Uart0 clock */
  __PIO_PER = 0x2000; __PIO_OER = 0x2000;  /* setup IO bit for FORCEON output */
  __PIO_SODR = 0x2000;  /* set FORCEON */
  __PIO_PDR = 0x00018000; /* setup Uart0 Tx and Rx pins */

  __US_CR = 0x50;      /* enable and reset rx and tx */
  __US_MR = 0x28C0;    /* async mode, 8N2 */
  __US_BRGR = 8;      /* set baud 115200, regval = 14.7456mhz-xtal / 16 / baudrate */
  
  __US_RCR = 0;
  __US_RPR = (int) inbuff_at91;
  __US_RCR = 80;
#endif

#elif (CONSOLE_DEVICE==CONDEV_SYSLOG)
#error not implemented yet

#else
#error - Implement tm_open in terminal.c
#endif

}

/*************************************************************************
 tm_putc() - Output a single character to a terminal.

 int tm_putc(char ch)

    ch       - The character to output.

 Returns:
    -1 on failure
    0  on success

 Note: This function may block.
*************************************************************************/

void tm_putc(char ch)
{
    tm_open();

    /* Do ASCII translation.  Make sure that \n gets translated to \r\n.   */
    if (ch == '\n')
    {
        tm_putc('\r');
    }

/* Do device specific output   */
#if (CONSOLE_DEVICE==CONDEV_VT100C)
    vt100c_update(ch);      /* See vt100c.c */

#elif (CONSOLE_DEVICE==CONDEV_PCVIDEO)
    pcvid_putc(ch);         /* See pcvid.c */

#elif (CONSOLE_DEVICE==CONDEV_RTXC)
    rtxcputc((int)ch);

#elif (CONSOLE_DEVICE==CONDEV_KROS)
    kr_port_writecons(ch);

#elif (CONSOLE_DEVICE==CONDEV_TELNET)
    tnserv_putc(ch);        /* see telnets.c */

#elif (IS_16550_DEVICE)
    putc_16550(ch);         /* See below */

#elif (CONSOLE_DEVICE==CONDEV_PC_STDIO)
    putch(ch);

#elif (CONSOLE_DEVICE==CONDEV_MCF52XX)
    mcf5272_uart_device_polled_send(TERMPORT, 1, &ch);

#elif (CONSOLE_DEVICE==CONDEV_SH_SCIF)
    putc_SCIF(ch);

#elif (CONSOLE_DEVICE==CONDEV_EMBOS_GNU)
    fputc((int)ch, console_tty);
    fflush(console_tty);

#elif (CONSOLE_DEVICE==CONDEV_NODEV)       /* - USe ecos serial driver */
    /* Nothing   */
#elif (CONSOLE_DEVICE==CONDEV_AT9155800)
#if (ATPORT == 2)
{
int i=0;
  while (((__US_CSR2 & 0x2) == 0) || ((__US_CSR2 & 0x2) == 0))  /* wait for TXREADY */
    if (i++ > 10000)
      break;

  __US_TPR2 = (int)&ch;
  __US_TCR2 = 1;
  while (((__US_CSR2 & 0x2) == 0) || ((__US_CSR2 & 0x2) == 0))  /* wait for TXREADY */
    if (i++ > 10000)
      break;
}
#elif (ATPORT == 1)
{
int i=0;
  while (((__US_CSR1 & 0x2) == 0) || ((__US_CSR1 & 0x2) == 0))  /* wait for TXREADY */
    if (i++ > 10000)
      break;

  __US_TPR1 = (int)&ch;
  __US_TCR1 = 1;
  while (((__US_CSR1 & 0x2) == 0) || ((__US_CSR1 & 0x2) == 0))  /* wait for TXREADY */
    if (i++ > 10000)
      break;
}
#else
{
int i=0;
  while (((__US_CSR & 0x2) == 0) || ((__US_CSR & 0x2) == 0))  /* wait for TXREADY */
    if (i++ > 10000)
      break;

  __US_TPR = (int)&ch;
  __US_TCR = 1;
  while (((__US_CSR & 0x2) == 0) || ((__US_CSR & 0x2) == 0))  /* wait for TXREADY */
    if (i++ > 10000)
      break;
}
#endif

#elif (CONSOLE_DEVICE==CONDEV_SYSLOG)
    syslog_putc(ch);
#else
#   error - Implement tm_putc in terminal.c
#endif
}

/*************************************************************************
 tm_getch() - Receive a single character from a terminal.

 int tm_getch()

 Returns the character that was read

 This means that if a 0xFF is somehow sent over the connection, there is
 a false error condition.  But this will probably never happen. tm_getch
 should never echo the character back to the other side.
 This function will block if there are no characters available.
*************************************************************************/
int tm_getch(void)
{
int ch = 0;

#if (CONSOLE_DEVICE==CONDEV_RTXC)
#else
    /* Wait for a character to be available.   */
    while (!tm_kbhit())
    {
        ks_sleep(2);
    }
#endif

/* Do device specific output   */
#if (CONSOLE_DEVICE==CONDEV_VT100C)
    ch = vt100c_getch();

#elif (CONSOLE_DEVICE==CONDEV_PCVIDEO)
    ch = pcvid_getch() & 0xff;

#elif (CONSOLE_DEVICE==CONDEV_KROS)
    ch = kr_port_readcons() & 0xff;

#elif (CONSOLE_DEVICE==CONDEV_TELNET)
    ch = tnserv_getch();        /* see telnets.c */

#elif (IS_16550_DEVICE)
    ch = getch_16550();         /* See below */

#elif (CONSOLE_DEVICE==CONDEV_PC_STDIO)
    ch = getch();

#elif (CONSOLE_DEVICE==CONDEV_NODEV)       /* - USe ecos serial driver */
    ch = 0;

#elif (CONSOLE_DEVICE==CONDEV_MCF52XX)
    ch = mcf5272_uart_device_polled_getc(TERMPORT);

#elif (CONSOLE_DEVICE==CONDEV_SH_SCIF)
    ch = getch_SCIF();

#elif (CONSOLE_DEVICE==CONDEV_EMBOS_GNU)
      ch = fgetc(console_tty);

#elif (CONSOLE_DEVICE==CONDEV_RTXC)
    if(!rtxcchar)
    {
      /* Wait for a character to be available.     */
      while (!tm_kbhit())
      {
        ks_sleep(2);
      }
    }
    ch = rtxcchar;
    rtxcchar = 0;
    
#elif (CONSOLE_DEVICE==CONDEV_AT9155800)
#if (ATPORT == 2)
  __US_RPR2 = (int) inbuff_at91;
  __US_RCR2 = 80;
    ch= inbuff_at91[0];
#elif (ATPORT == 1)
  __US_RPR1 = (int) inbuff_at91;
  __US_RCR1 = 80;
    ch= inbuff_at91[0];
#else
  __US_RPR = (int) inbuff_at91;
  __US_RCR = 80;
    ch= inbuff_at91[0];
#endif

#elif (CONSOLE_DEVICE==CONDEV_SYSLOG)
    return 0;

#else
#   error - Implement tm_getch
#endif

    return(ch);
}


/*************************************************************************
 tm_kbhit() - Check and see if there are any characters waiting.

 int tm_kbhit()

 Returns:
    Non-zero if there are characters waiting.
    Zero if there are no characters waiting.
*************************************************************************/

int tm_kbhit(void)
{
/* Do device specific output   */
#if (CONSOLE_DEVICE==CONDEV_VT100C)
    return(pcvid_kbhit());

#elif (CONSOLE_DEVICE==CONDEV_PCVIDEO)
    return(pcvid_kbhit());

#elif (CONSOLE_DEVICE==CONDEV_KROS)
    return kr_port_kbhit();

#elif (CONSOLE_DEVICE==CONDEV_TELNET)
    return(tnserv_kbhit());     /* see telnets.c */

#elif (IS_16550_DEVICE)
    return(kbhit_16550());          /* See below */

#elif (CONSOLE_DEVICE==CONDEV_PC_STDIO)
    return(kbhit());

#elif (CONSOLE_DEVICE==CONDEV_NODEV)       /* - USe ecos serial driver */
    return(0);

#elif (CONSOLE_DEVICE==CONDEV_MCF52XX)
    return(mcf5272_uart_device_polled_kbhit(TERMPORT));

#elif (CONSOLE_DEVICE==CONDEV_SH_SCIF)
    return(kbhit_SCIF());

#elif (CONSOLE_DEVICE==CONDEV_EMBOS_GNU)
    return(kbhit(console_tty));

#elif (CONSOLE_DEVICE==CONDEV_RTXC)
int temp = rtxcckinput();
    if( temp < 0) return(0);
    else
    {
      rtxcchar = temp;
      return(1);
    }
    
#elif (CONSOLE_DEVICE==CONDEV_AT9155800)
#if (ATPORT == 2)
  return(__US_RCR2 < 80);
#elif (ATPORT == 1)
  return(__US_RCR1 < 80);
#else
  return(__US_RCR < 80);
#endif  

#elif (CONSOLE_DEVICE==CONDEV_SYSLOG)
    return 0;
#else
#   error - Implement tm_kbhit in terminal.c
#endif
}

/************************************************************************  */
/****************** TERMINAL API ROUTINES *********************************
 These routines are derived from the above routines.  The above routines
 are the only ones that need porting.  The ones below can be left alone.
 *************************************************************************
 *************************************************************************/

/* This routine cm_puts may be optimized for your terminal environment to
   provide faster throughput, otherwise it calls tm_putc() once
   per character */

/*************************************************************************
 tm_cputs() - Output a string to a terminal.

 void tm_cputs(PFCCHAR string)

    string  - The string to output.

 Returns:

 Note: This function may block.
*************************************************************************/

void tm_cputs(PFCCHAR string)
{
    tm_open();
#if (CONSOLE_DEVICE==CONDEV_RTXC)
     rtxcputs(string);
#elif (CONSOLE_DEVICE==CONDEV_EMBOS_GNU)
    fprintf (console_tty,string);
    fflush(console_tty);
#else
    while (*string)
    {
        tm_putc(*string++);
    }
#endif
}

/*************************************************************************
 tm_puts() - Output a string to a terminal, with a newline at the end.

 int tm_puts(PFCCHAR string)

    string  - The string to output.

 Returns:
    -1 on failure
    0  on success

 Note: This function may block.
*************************************************************************/

void tm_puts(PFCCHAR string)
{
    tm_open();

    /* Ignore an error condition from the first call.  It will be   */
    /* caught on the second call.                                   */
#if (CONSOLE_DEVICE==CONDEV_RTXC)
     rtxcputs(string);
     rtxcputc((int)'\n');
#elif (CONSOLE_DEVICE==CONDEV_NODEV)       /* - USe ecos serial driver */
/*  diag_write_string(string);   */
/*  diag_write_string("\n");     */
#else
    tm_cputs(string);
    tm_putc('\n');
#endif
}

/*************************************************************************
 tm_gets() - Input a string from a terminal.

 int tm_gets( PFCCHAR string)

    string  - The string to received to

 Returns:
    -1 on failure
    0  on success

 Note: This function may block.  It reads a string from the terminal
       device, and stops reading when it encounters a return ('\r')
       from the data stream.  It calls tm_promptstring (a line editing
       function) with an empty string.  In the future, this may be a
       duplicate implementation of tm_promptstring, because tm_promptstring
       is at a slightly higher layer than tm_gets.  This is because tm_gets
       is a standard ANSI I/O library function, but tm_promptstring is not.
*************************************************************************/

int tm_gets(PFCHAR string)
{
    tm_open();
    *string = 0;
    return(tm_promptstring(string,FALSE));
}

#if (INCLUDE_SSL)
/* [i_a] 'limited' eq. which prevent buffer overrun   */
int     tml_gets(PFCHAR string, int size)
{
    tm_open();
    *string = 0;
    return(tml_promptstring(string,size,FALSE));
}
#endif


/****************** TERMINAL UTILITIES *********************************  */
/*************************************************************************
 These functions are part of out terminal API, but they do not correspond
 to standard ANSI I/O library functions.  They are utilities commonly used
 by our applications.  In the future, it is possible that these may be
 optionally included.
*************************************************************************/

/*************************************************************************
 tm_promptstring() - Allow the user to edit a string interactively.

 int tm_promptstring(PFCHAR string, RTIP_BOOLEAN handle_arrows)

    ch             - The string to edit.
    handle_arrows  - A flag specifying the level of interactivity.  If
                     handle_arrows is TRUE, tm_promptstring will return
                     a code if Up Arrow, Down Arrow, or ESC are pressed.
                     The codes are:
                     VT100_UP_ARROW
                     VT100_DOWN_ARROW
                     VT100_ESC
                     If handle_arrows is false, tm_promptstring will
                     ignore these keys.

 Returns:
    -1 on failure
    0  on success
    if handle_arrows is TRUE:
    VT100_UP_ARROW if the up arrow key was pressed.
    VT100_DOWN_ARROW if the down arrow key was pressed.
    VT100_ESC if the escape key was pressed.

 Note: This function may block.
       The handle_arrows flag is mostly useful in interactive shells
       and other interactive prompts, where an application might have a
       history of commands that have been entered, or a list of possible
       choices for an answer that may be scrolled through with the arrow
       keys.  This function may be moved to the application layer in the
       future.
*************************************************************************/

int tm_promptstring(PFCHAR string, RTIP_BOOLEAN handle_arrows)
{
#if (INCLUDE_SSL)
    return tml_promptstring(string, 32767, handle_arrows);
}
int tml_promptstring(PFCHAR string, int size, RTIP_BOOLEAN handle_arrows)
{
#endif /* INCLUDE_SSL */

/* Endptr always points to null-terminator   */
PFCHAR endptr = &string[tc_strlen(string)];
int ch;
char clbuff[80];
int len = tc_strlen(string);

#if (INCLUDE_SSL)
        size--;
#endif

    tc_memset((PFBYTE)clbuff, ' ', 79);
    clbuff[0] = '\r';
    clbuff[78] = '\r';
    clbuff[79] = 0;

#define CLEAR_LINE() tm_cputs(clbuff)

    /* Print out the default answer   */
    tm_cputs(string);

    while ((ch = tm_getch()) != -1)
    {
        switch(ch)
        {
            /* Return   */
#if (CONSOLE_DEVICE==CONDEV_TELNET)
        /* Ignore \n for telnet since we always get \r\n   */
        case '\n':
            continue; /* Go back to the while loop */
#else
        case '\n':
#endif
        case '\r':
            tm_putc('\n');
            return(0);

            /* Backspace   */
#if (CONSOLE_DEVICE==CONDEV_VT100C)     /* - ebs vt100 emulator library */
        case VT100_LEFT_ARROW:
#elif (defined(IS_PCVID_DEVICE))
        case PCVID_LEFT_ARROW:
#endif
        case '\b':
            if(endptr > string)
            {
                tm_cputs("\b \b");
                *(--endptr) = 0;
            }
            continue; /* Go back to the while loop */

#if (CONSOLE_DEVICE==CONDEV_VT100C)     /* - ebs vt100 emulator library */
        case VT100_UP_ARROW:
#elif (defined(IS_PCVID_DEVICE))
        case PCVID_UP_ARROW:
#else
        case '>':
#endif
            if(handle_arrows)
            {
                /* erase the current line   */
                CLEAR_LINE();
                return(TERMINAL_UP_ARROW);
            }
            break;

#if (CONSOLE_DEVICE==CONDEV_VT100C)     /* - ebs vt100 emulator library */
        case VT100_DOWN_ARROW:
#elif (defined(IS_PCVID_DEVICE))
        case PCVID_DOWN_ARROW:
#else
        case '<':
#endif
            if(handle_arrows)
            {
                /* erase the current line   */
                CLEAR_LINE();
                return(TERMINAL_DOWN_ARROW);
            }
            break;

        case 27: /*ESC*/
            if(handle_arrows)
            {
                /* erase the current line   */
                CLEAR_LINE();
                return((int)ch);
            }
            break;

#if (CONSOLE_DEVICE==CONDEV_VT100C)     /* - ebs vt100 emulator library */
    case VT100_RIGHT_ARROW:
            ch = (char)' ';
            break;
#endif

#if (defined(IS_PCVID_DEVICE))
        case PCVID_RIGHT_ARROW:
            ch = (char)' ';
            break;
#endif

        }

#if (INCLUDE_SSL)
            if (len < size)
            {
            /* Display the editing   */
            tm_putc((char)ch);
            *endptr++ = (char)ch;
            *endptr = 0;
        }
        else
        {
            /* keep cursor at same position   */
        }
#else
        /* Display the editing   */
        tm_putc((char)ch);
        *endptr++ = (char)ch;
        *endptr = 0;

#endif /* INCLUDE_SSL */

    }
    return(-1);
}


#if (IS_16550_DEVICE)

/*                                                      */
/*  Subroutines to support terminal IO over 16550 uart. */
/*                                                      */

/* *****************************************************   */
/* UART register offsets from sayportBase                  */

#define THR    0
#define RBR    0
#define BRDL   0
#define IER    1
#define BRDH   1
#define IIR    2
#define FCR    2
#define LCR    3
#define MCR    4
#define LSR    5
#define MSR    6


/* Select COMM portBase and interrupt number   */
IOADDRESS sayportBase;   /* I/O address for COM */
#define  commRate        6    /* 19200 baud divisor */

void open_16550(void)
{
#define ioDLY1()
#if (CONSOLE_DEVICE==CONDEV_16550_TX)
#define TX39_ISA_IO_BASE        0xB1000000 /* 0x11000000 */
    init_tx();
    sayportBase = TX39_ISA_IO_BASE+0x3f8;   /* Com 1 */
#elif (CONSOLE_DEVICE==CONDEV_16550_1)
            sayportBase = 0x3F8;   /* I/O address for COM1 */
#elif (CONSOLE_DEVICE==CONDEV_16550_2)
            sayportBase = 0x2F8;   /* I/O address for COM2 */
#elif (CONSOLE_DEVICE==CONDEV_16550_3)
            sayportBase = 0x3E8;   /* I/O address for COM3 */
#elif (CONSOLE_DEVICE==CONDEV_16550_4)
            sayportBase = 0x2E8;   /* I/O address for COM4 */
#endif

   /*Set communication rate and parameters   */
   OUTBYTE(sayportBase + LCR, 0x80);   /*divisor access */
   ioDLY1();
   OUTBYTE(sayportBase + BRDL, commRate);
   ioDLY1();
   OUTBYTE(sayportBase + BRDH, 0);
   ioDLY1();
   OUTBYTE(sayportBase + LCR, 0x03);   /*divisor off, no parity, 1 stop bit, 8 bits */
   ioDLY1();

   /*Clear existing interrupts   */
   INBYTE(sayportBase + LSR);
   ioDLY1();
   INBYTE(sayportBase + MSR);
   ioDLY1();
   INBYTE(sayportBase + RBR);
   ioDLY1();

   /* Disable all interrupts   */
   OUTBYTE(sayportBase + IER, 0);
   ioDLY1();
}

void putc_16550(char c)
{
unsigned char l;
    /* Wait for holding register empty then put into THR   */
    do {l = (unsigned char)INBYTE(sayportBase + LSR); } while (!(l & 0x40));
    OUTBYTE(sayportBase + THR, c);
}

int kbhit_16550(void)
{
    if (((unsigned char)INBYTE(sayportBase + LSR)) & 0x01)
        return(1);
    else
        return(0);
}

char getch_16550(void)
{
    for (;;)
    {
        if (kbhit_16550())
            return((char)INBYTE(sayportBase + RBR));
    }
}

#if (CONSOLE_DEVICE==CONDEV_16550_TX)

#define ISA_BUS_CLOCK *(volatile unsigned char *) 0xb2100000
#define TX39_ISA_IO_INDEX       0x0398
#define TX39_ISA_IO_DATA        0x0399
#define INDEXREG                TX39_ISA_IO_BASE + TX39_ISA_IO_INDEX
#define DATAREG                 TX39_ISA_IO_BASE + TX39_ISA_IO_DATA

byte peekbreg(byte *addr){ return(*addr);}
void pokebreg(byte *addr,byte b){*addr = b;}
byte readreg(byte offset){
    pokebreg((byte *) INDEXREG, offset);
    return(peekbreg((byte *) DATAREG));
}

void writereg(byte offset, byte v)
{
    /* Enter the register you want to write into the Index reg.   */
    pokebreg((byte *) INDEXREG, offset);
    /* Enter the data you want to write into the register into the data reg.   */
    /* On PC87338 need to do 2 consecutive writes                              */
    pokebreg((byte *) DATAREG, v);
    pokebreg((byte *) DATAREG, v);
}

/* Initialize the TX39 Multi IO chip   */
void init_tx(void)
{
byte fer;
  /* Set to four per JMR manual   */
  ISA_BUS_CLOCK = 4;

  /* Enable clock multiplier. Uarts won't work without it   */
  writereg(0x51, 4);
  /* enable UART 1   */
  fer = readreg(0);
  writereg(0, fer | 2);
}
#endif  /* TX39 */

#endif  /* 16550 uart */

#if (CONSOLE_DEVICE==CONDEV_TELNET)     /* - telnet server */

/*                                                 */
/*  Subroutines to support terminal IO over TELNET */
/*                                                 */

/* This must be called by applications when the stack is up and the   */
/* interface is up                                                    */

void tm_telnet_server_main(void)
{
    for(;;)
    {
        if (telnet_server_daemon() == -1)
        {
            DEBUG_ERROR("telnet server failed", NOVAR, 0, 0);
        }
        else
        {
            DEBUG_ERROR("telnet server finished", NOVAR, 0, 0);
        }
    }
}

void tm_stack_is_up()
{
    if (!telnet_spawned)
    {
        telnet_spawned = TRUE;
        /* spawn a telnet server task   */
        if (!os_spawn_task(TASKCLASS_TELNET_DAEMON,tm_telnet_server_main, 0,0,0,0))
        {
            telnet_spawned = FALSE;
            return;
        }
    }
}


/* this is putc for telnet note: still need to buffer send if in interrupt   */
/* rtip_inside_interrupt                                                     */
#define PUT_BUFFER_SIZE 1024
char put_buffer[PUT_BUFFER_SIZE];
int put_pointer = 0;

void tnserv_putc(char c)
{
    if (telnet_console_context)
    {
        if (put_pointer < PUT_BUFFER_SIZE)
            put_buffer[put_pointer++] = c;
    }
}

void tnserv_putc_flush(void)
{
int n;
    if (telnet_console_context)
    {
        if (put_pointer)
        {
            send(telnet_console_context->tns_socket, (PFCCHAR)put_buffer,put_pointer, 0);
            put_pointer = 0;
        }
    }
}

char tnserv_getch(void)
{
char c = 0;
    if (telnet_console_context)
    {
        if (telnet_console_context->tns_bp)
        {
            telnet_console_context->tns_bp -= 1;
            c = telnet_console_context->tns_buf[telnet_console_context->tns_bp];
        }
    }
    return(c);
}

int  tnserv_kbhit()
{
int x = 0;
    if (telnet_console_context)
        x = telnet_console_context->tns_bp;
    return(x);
}

#endif /* (CONSOLE_DEVICE==CONDEV_TELNET) */




#if (CONSOLE_DEVICE==CONDEV_SYSLOG)     /* - syslog client */

/*                                                 */
/*  Subroutines to support terminal IO over SYSLOG */
/*                                                 */

extern int KS_FAR in_irq;

/* this is putc for syslog                         */
/* note: still need to buffer send if in interrupt */
/* rtip_inside_interrupt                           */
#define PUT_BUFFER_SIZE 1024
char put_buffer[PUT_BUFFER_SIZE];
int put_pointer = 0;
int past_newline_pos = 0;


void syslog_putc(char c)
{
    if (put_pointer < PUT_BUFFER_SIZE)
        put_buffer[put_pointer++] = c;

    if ((c == '\n') || (!c))
    {
        past_newline_pos = put_pointer;
        if (!in_irq)
            syslog_putc_flush();
    }
}

void syslog_putc_flush(void)
{
    int n;
    char *p;

    if (past_newline_pos)
    {
        xn_syslog(0, "TERMINAL: %.*s", past_newline_pos-1, put_buffer);
        if (past_newline_pos < put_pointer)
        {
            tc_memmove(put_buffer, put_buffer + past_newline_pos, (put_pointer -= past_newline_pos));
        }
        past_newline_pos = 0;
    }

    if (put_pointer > (sizeof(put_buffer)/2))
    {
        xn_syslog(0, "TERMINAL: %.*s", put_pointer, put_buffer);
        put_pointer = 0;
    }
}

#endif /* (CONSOLE_DEVICE==CONDEV_SYSLOG) */


#if (CONSOLE_DEVICE==CONDEV_SH_SCIF)
/* íçÅFSH3/SH4ÇÃÇ›   */

#define RTIP_SCFSR_ER    0x80                /* Serial Status Register bit : Receive Error */
#define RTIP_SCFSR_TEND  0x40                /* Serial Status Register bit : Transmit End */
#define RTIP_SCFSR_TDFE  0x20                /* Serial Status Register bit : Transmit FIFO data empty */
#define RTIP_SCFSR_BRK   0x10                /* Serial Status Register bit : Break Detection */
#define RTIP_SCFSR_FER   0x08                /* Serial Status Register bit : Framing Error */
#define RTIP_SCFSR_PER   0x04                /* Serial Status Register bit : Parity Error */
#define RTIP_SCFSR_RDF   0x02                /* Serial Status Register bit : Receive FIFO data full */
#define RTIP_SCFSR_DR    0x01                /* Serial Status Register bit : Receive data ready */

#define RTIP_SCSMR_7BIT      0x40        /* Serial Mode : 7bit data length */
#define RTIP_SCSMR_PE        0x20        /* Serial Mode : Parity enable */
#define RTIP_SCSMR_ODD       0x10        /* Serial Mode : Odd parity */
#define RTIP_SCSMR_2STOP     0x08        /* Serial Mode : 2 stop bits. */
#define RTIP_SCSMR_CLK1      0x00        /* Serial Mode : Source clock = Pclock */
#define RTIP_SCSMR_CLK4      0x01        /* Serial Mode : Source clock = Pclock/4 */
#define RTIP_SCSMR_CLK16     0x02        /* Serial Mode : Source clock = Pclock/16 */
#define RTIP_SCSMR_CLK64     0x03        /* Serial Mode : Source clock = Pclock/64 */

#define RTIP_SCSCR_TIE       0x80        /* Serial Control : Transmit interrupt enable */
#define RTIP_SCSCR_RIE       0x40        /* Serial Control : Receive interrupt enable */
#define RTIP_SCSCR_TE        0x20        /* Serial Control : Transmit enable */
#define RTIP_SCSCR_RE        0x10        /* Serial Control : Receive enable */
#define RTIP_SCSCR_REIE      0x08        /* Serial Control : Receive error interrupt enable */
#define RTIP_SCSCR_CKE1      0x02        /* Serial Control : External clock */

#define RTIP_SCFCR_RTRG1     0x00        /* FIFO Control : Receive FIFO trigger = 1 */
#define RTIP_SCFCR_RTRG4     0x40        /* FIFO Control : Receive FIFO trigger = 4 */
#define RTIP_SCFCR_RTRG8     0x80        /* FIFO Control : Receive FIFO trigger = 8 */
#define RTIP_SCFCR_RTRG14    0xc0        /* FIFO Control : Receive FIFO trigger = 14 */
#define RTIP_SCFCR_TTRG8     0x00        /* FIFO Control : Transmit FIFO trigger = 8 */
#define RTIP_SCFCR_TTRG4     0x10        /* FIFO Control : Transmit FIFO trigger = 4 */
#define RTIP_SCFCR_TTRG2     0x20        /* FIFO Control : Transmit FIFO trigger = 2 */
#define RTIP_SCFCR_TTRG1     0x30        /* FIFO Control : Transmit FIFO trigger = 1 */
#define RTIP_SCFCR_MCE       0x08        /* FIFO Control : CTS/RTS signal enable */
#define RTIP_SCFCR_TFRST     0x04        /* FIFO Control : Reset transmit FIFO */
#define RTIP_SCFCR_RFRST     0x02        /* FIFO Control : Reset receive FIFO */
#define RTIP_SCFCR_LOOP      0x01        /* FIFO Control : Loopback test */

void open_SCIF(void)
{
    INT i,j;

    *RTIP_SCSCR2 = 0x00;                             /* stop transmitter and receiver */
    *RTIP_SCFCR2 = RTIP_SCFCR_TFRST | RTIP_SCFCR_RFRST; /* reset fifo */
    *RTIP_SCSCR2 = RTIP_SCSCR_CKE1;                     /* external clock */
    *RTIP_SCSMR2 = 0x00;                             /* set mode to 8N1 */

    for (i=j=0; i<100000; i++) {
        j += i;
    }

    *RTIP_SCFCR2 = RTIP_SCFCR_RTRG1 | RTIP_SCFCR_TTRG8 | RTIP_SCFCR_MCE; /* use rts/cts */
    *RTIP_SCSCR2 = RTIP_SCSCR_TE | RTIP_SCSCR_RE | RTIP_SCSCR_CKE1;      /* start transmitter and receiver / external clock */
}

void putc_SCIF(char c)
{
    while (!(*RTIP_SCSSR2 & RTIP_SCFSR_TDFE));
    *RTIP_SCFTDR2 = c;
    while (!(*RTIP_SCSSR2 & RTIP_SCFSR_TDFE));
    *RTIP_SCSSR2 &= ~RTIP_SCFSR_TDFE;
}
int kbhit_SCIF(void)
{
    return (*RTIP_SCSSR2 & RTIP_SCFSR_RDF);
}
char getch_SCIF(void)
{
    char c;
    while (!(*RTIP_SCSSR2 & RTIP_SCFSR_RDF));
    c = *RTIP_SCFRDR2;
    *RTIP_SCSSR2 &= ~RTIP_SCFSR_RDF;
    return c;
}
#endif  /* CONDEV_SH_SCIF */
