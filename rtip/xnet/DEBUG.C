/*                                                                      */
/* DEBUG.C - DISPLAY functions                                          */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#include "rtos.h" /*OS*/ /* added */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DIAGNOSTICS

#include  "rtip.h"
#include  "sock.h"
#include  "rtipext.h"
#if (SEND_TO_FSR && DO_FILE)
#include    <stdio.h>
#endif
#if (CFG_USE_DIAG_MSG)
#include <stdarg.h>
#endif

/* *****************************************************   */
/* YOU MIGHT WANT TO MODIFY THE FOLLOWING:                 */

/* Select comm rate                                      */
/* #define  commRate        1    // 115200 baud divisor  */
#define  commRate        6    /* 19200 baud divisor  */
/* #define  commRate        12   // 9600 baud divisor    */
/* #define  commRate        48   // 2400 baud divisor    */
/* #define  commRate        96   // 1200 baud divisor    */

#define SAY_RADIX 16            /* 16 = integers written in hex */
                                /* 10 = integers written in decimal   */

/* file name to write diagnostic and error messages (only used if   */
/* IO_TYPE_FSR is set to DO_FILE_IO (see debug.h)                   */
#define DEBUG_FILE_NAME "a:debug.out"

/* *****************************************************   */
/* THE REST OF FILE PROBABLY DOES NOT NEED MODIFICATION:   */

#define SAY_STR_LEN 120     /* length of string for formatting string */
                            /* to be printed; NOTE: string on stack   */
                            /* so don't make to too large             */

/* *****************************************************   */
/* UART register offsets from sayportBase                  */

#if (defined(AT_MOTHERBOARD))
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
#endif  /* AT_MOTHERBOARD */

/*Misc   */
#ifdef __TURBOC__
#define _asm      asm
#ifndef _inp
#define _inp      inp
#endif
#ifndef _outp
#define _outp     outp
#endif
#endif

/* *****************************************************   */
/* Protoype functions                                      */
#if (SEND_TO_FSR && DO_RS232 )
    static void  commInit(word comm_port);
    static void  ioDLY(void);
    static void send_a_char(unsigned char c);
    static void send_a_string(PFCHAR p);
#endif
/* *****************************************************   */
#if (!USE_DB_L0)        /* matches up with endif near end of file */

/*KS_EXTERN_GLOBAL_CONSTANT char KS_FAR tc_hmap[17];   */

/* *****************************************************   */
/* global variables                                        */

char KS_FAR hex_string[140];    /* must be at least 123 */


/* *****************************************************   */
/* INITIALIZATION and CLOSE                                */
/* *****************************************************   */
/* For debugging                                           */
void say_init(int to_file, word comm_port)
{
    write_to_file = to_file;  /* save info for all calls to say_file */

    /* initialize rs232 interface   */
    if (write_to_file == DO_RS232_IO)
    {
#        if (SEND_TO_FSR && DO_RS232 )
            commInit(comm_port);
#        else
            ARGSUSED_INT(comm_port);
#        endif
    }
    else if (write_to_file == DO_FILE_IO)
    {
#        if (SEND_TO_FSR && DO_FILE)
            say_fp = 0;
            say_fp = fopen(DEBUG_FILE_NAME, "w+");
            say_init_done = TRUE;
            if (say_fp == 0)
                say(SCREEN_TYPE, "open fail");
            else
                say(FILE_TYPE, "** OPEN FILE WORKED\n");
#        endif
    }
}

void say_close(void)
{
    if (!print_debug_log)
        return;

#    if (SEND_TO_FSR && DO_FILE)
        if (say_fp != 0)
        {
            if (say_init_done)
            {
                fflush(say_fp);
                fclose(say_fp);
                say_fp = NULL;
            }
        }
#    endif
}

#if (SEND_TO_FSR && DO_RS232 )
/* *****************************************************   */
/* COMM PORT ROUTINES                                      */
/* *****************************************************   */

static void ioDLY(void)
{
}

/* Communication initialization. Call from main application   */
/* initialization function.                                   */
static void commInit(word comm_port)
{
#if (defined(AT_MOTHERBOARD))
    switch (comm_port)
    {
        case 1 :
            sayportBase = 0x3F8;   /* I/O address for COM1  */
            saycommIRQ  =     4;   /* IRQ for COM1  */
            break;
        case 2 :
            sayportBase = 0x2F8;   /* I/O address for COM2  */
            saycommIRQ  =     3;   /* IRQ for COM2  */
            break;
        case 3 :
            sayportBase = 0x3E8;   /* I/O address for COM3  */
            saycommIRQ  =     4;   /* IRQ for COM3  */
            break;
        case 4 :
            sayportBase = 0x2E8;   /* I/O address for COM4  */
            saycommIRQ  =     3;   /* IRQ for COM4  */
            break;
        default:
            sayportBase = 0x3F8;   /* I/O address for COM1  */
            saycommIRQ  =     4;   /* IRQ for COM1  */
            say(SCREEN_TYPE, "commInit - illegal comm port - set to 1");
            break;
    }

   /*Initialize UART   */
   ks_disable();

   /*Set communication rate and parameters   */
   OUTBYTE(sayportBase + LCR, 0x80);   /*divisor access */
   ioDLY();
   OUTBYTE(sayportBase + BRDL, commRate);
   ioDLY();
   OUTBYTE(sayportBase + BRDH, 0);
   ioDLY();
   OUTBYTE(sayportBase + LCR, 0x03);   /*divisor off, no parity, 1 stop bit, 8 bits */
   ioDLY();

   /*Clear existing interrupts   */
   INBYTE(sayportBase + LSR);
   ioDLY();
   INBYTE(sayportBase + MSR);
   ioDLY();
   INBYTE(sayportBase + RBR);
   ioDLY();

   /* Disable all interrupts    */
   OUTBYTE(sayportBase + IER, 0);
   ioDLY();

   /*Unmask comm interrupt in PIC                        */
/*   OUTBYTE(IMREG, INBYTE(IMREG) & ~(1 << saycommIRQ)); */
   ioDLY();

   /*Enable interrupt signal and DTR   */
/*   OUTBYTE(sayportBase + MCR, 0x0b); */
   ioDLY();

   ks_enable();

#endif
}

#if (!defined(MC68302))

static void send_a_char(unsigned char c)
{
#if (defined(AT_MOTHERBOARD))
unsigned char l;
unsigned char ctl;

    /* Check for ^q   */
    l = (unsigned char)INBYTE(sayportBase + LSR);
    if (l & 0x01)
    {
        ctl = (unsigned char)INBYTE(sayportBase + RBR);
        if (ctl == 19) /* ^s */
        {
            do
            {
                l = (unsigned char)INBYTE(sayportBase + LSR);
                if (l & 0x01)
                {
                    ctl = (unsigned char)INBYTE(sayportBase + RBR);
                }
            } while (ctl != 17);    /* ^q */
        }

    }

    /* Wait for transmitter emplty   */
    do
    {
         l = (unsigned char)INBYTE(sayportBase + LSR);
    }
    while (!(l & 0x40));


   OUTBYTE(sayportBase + THR, c);
   ioDLY();

#endif
}
#endif /* (!defined(MC68360)) */

/* *****************************************************                */
/* output routine for DEBUG_LOG and DEBUG_ERROR with LOG_LEVEL_1 set to */
/* 1                                                                    */
/* *****************************************************                */
static void send_a_string(PFCHAR p)
{
    while (*p)
    {
        send_a_char(*p++);
    }
}

#endif /* SEND_TO_FSR && DO_RS232 */


/* *****************************************************   */
/* SAY TO SCREEN, RS232, FILE ROUTINES                     */
/* *****************************************************   */
void do_say_screen(PFCHAR string)     /* __fn__ */
{
    /* call back to the application to write the string   */
    CB_WR_SCREEN_STRING(string, (RTIP_BOOLEAN)(in_irq > 0));
    OS_DebugSendString(string); /*OS*/ /* output to embOSView terminal window */
    OS_DebugSendString("\n"); /*OS*/
}

/* *****************************************************   */
void do_say_file(PFCHAR string)
{

    if (write_to_file == DO_FILE_IO)
    {
#        if (DO_FILE && SEND_TO_FSR)
            if (say_fp == 0)
                return;
            else
            {
#if (defined(__BORLANDC__) )
                /* print FAR string   */
                fprintf(say_fp, "%Fs\n", string);
#else
                fprintf(say_fp, "%s\n", string);
#endif
                fflush(say_fp);
            }
#        endif
    }

    /* write over rs232 port   */
    else if (write_to_file == DO_RS232_IO)
    {
#        if (SEND_TO_FSR && DO_RS232 )
            send_a_string(string);
            send_a_string("\r\n");
#        endif

        return;
    }

    /* or write to screen   */
    else if (write_to_file == DO_SCREEN_IO)
    {
        do_say_screen(string);
    }
}

void say_file(PFCHAR string)     /* __fn__ */
{
    do_say_file(string);
}


/* ********************************************************************   */
/* *****************************************************                  */
/* *****************************************************              */
/* NOTE: all say_xxx call this routine after formatting output string */
/* out_type could be: FILE_TYPE, SCREEN_TYPE                          */
void say_out(int out_type, PFCHAR string)
{
    if (!say_init_done)
    {
        say_init(IO_TYPE_FSR, say_port);
        say_init_done = TRUE;
    }

/*    lock_tsk();   */
    if (out_type == FILE_TYPE)
        say_file(string);
    else if (out_type == SCREEN_TYPE)
        do_say_screen(string);
    else
        say_file((PFCHAR)"ERROR:say_out - illegal 1st parameter");
/*    unlock_tsk();   */
}


/* *****************************************************   */
/* FORMAT ROUTINES                                         */
/* *****************************************************   */

void say_hex(int out_type, PFCCHAR comment, PFBYTE p, int len)     /* __fn__ */
{
int i;
int tot_len;
int curr_len;
char s2[120];

    if (tc_strlen(comment) > 0)
    {
        tc_strcpy(s2, comment);
        say_out(out_type, s2);
    }

    tot_len = len;

    len = 25;       /* amount that will fit on a line  */

    hex_string[0]='N';
    hex_string[1]='U';
    hex_string[2]='L';
    hex_string[3]='L';
    hex_string[4]='\0';

    curr_len = 0;
    len *= 3;

    while (curr_len < tot_len)
    {
        /* write one line worth of data   */
        for (i=0; i<len; i=i+3)
        {
            hex_string[i] = tc_hmap[(int)((*p)>>4)];
            hex_string[i+1] = tc_hmap[(int)((*p)&0x0f)];
            hex_string[i+2] = ' ';
            hex_string[i+3] = '\0';
            p++;
            curr_len++;
            if (curr_len >= tot_len)
                break;
        }
        say_out(out_type, hex_string);
    }

}


void say(int out_type, PFCCHAR comment)
{
char s2[120];

    tc_strcpy(s2, comment);
    say_out(out_type, s2);
}

void say_int(int out_type, PFCCHAR string, int val)
{
char s2[120];
int i;

    for (i=0; i<100; i++)
    {
        s2[i] = string[i];
        if (string[i] == '\0')
            break;
    }

    s2[i] = ' ';
    tc_itoa(val, s2+i+1, SAY_RADIX);

    say_out(out_type, s2);
}

#if (INCLUDE_RTIP)

void say_ip_addr(int out_type, PFCCHAR comment, byte KS_FAR *addr)
{
char say_string[120];

    tc_strcpy(say_string, comment);
/*  tc_strcpy(say_string+tc_strlen(say_string), " = ");   */
    tc_itoa(addr[0], say_string+tc_strlen(say_string), 10);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[1], say_string+tc_strlen(say_string), 10);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[2], say_string+tc_strlen(say_string), 10);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[3], say_string+tc_strlen(say_string), 10);

    say_out(out_type, (PFCHAR)say_string);
}

void say_ether_addr(int out_type, PFCCHAR comment, byte KS_FAR *addr)
{
char say_string[120];

    tc_strcpy(say_string, comment);
    tc_strcpy(say_string+tc_strlen(say_string), " = ");
    tc_itoa(addr[0], say_string+tc_strlen(say_string), 16);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[1], say_string+tc_strlen(say_string), 16);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[2], say_string+tc_strlen(say_string), 16);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[3], say_string+tc_strlen(say_string), 16);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[4], say_string+tc_strlen(say_string), 16);
    tc_strcpy(say_string+tc_strlen(say_string), ".");
    tc_itoa(addr[5], say_string+tc_strlen(say_string), 16);

    say_out(out_type, (PFCHAR)say_string);
}
#endif /* (INCLUDE_RTIP */

void say_int2(int out_type, PFCCHAR comment, int val1, int val2)
{
char s2[120];
int i;

    for (i=0; i<100; i++)
    {
        s2[i] = comment[i];
        if (comment[i] == '\0')
            break;
    }

    s2[i] = ' ';
    tc_itoa(val1, s2+i+1, SAY_RADIX);

    i = tc_strlen(s2);
    s2[i] = ' ';
    tc_itoa(val2, s2+i+1, SAY_RADIX);

    say_out(out_type, s2);

}

void say_lint(int out_type, PFCCHAR comment, long val1)
{
char s2[120];
int i;

    for (i=0; i<100; i++)
    {
        s2[i] = comment[i];
        if (comment[i] == '\0')
            break;
    }

    s2[i] = ' ';
    tc_ltoa(val1, s2+i+1, SAY_RADIX);

    say_out(out_type, s2);

}

void say_lint2(int out_type, PFCCHAR comment, long val1, long val2)
{
char s2[120];
int i;

    for (i=0; i<100; i++)
    {
        s2[i] = comment[i];
        if (comment[i] == '\0')
            break;
    }

    s2[i] = ' ';
    tc_ltoa(val1, s2+i+1, SAY_RADIX);

    i = tc_strlen(s2);
    s2[i] = ' ';
    tc_ltoa(val2, s2+i+1, SAY_RADIX);

    say_out(out_type, s2);

}

void say_dint(int out_type, PFCCHAR comment, dword val1)
{
char s2[120];
int i;

    for (i=0; i<100; i++)
    {
        s2[i] = comment[i];
        if (comment[i] == '\0')
            break;
    }

    s2[i] = ' ';
    tc_ultoa(val1, s2+i+1, SAY_RADIX);

    say_out(out_type, s2);

}

void say_dint2(int out_type, PFCCHAR comment, dword val1, dword val2)
{
char s2[120];
int i;

    for (i=0; i<100; i++)
    {
        s2[i] = comment[i];
        if (comment[i] == '\0')
            break;
    }

    s2[i] = ' ';
    tc_ultoa(val1, s2+i+1, SAY_RADIX);

    i = tc_strlen(s2);
    s2[i] = ' ';
    tc_ultoa(val2, s2+i+1, SAY_RADIX);

    say_out(out_type, s2);

}

void say_str(int out_type, PFCCHAR comment, PFCHAR str1)
{
char s2[SAY_STR_LEN];
int len;
int len_copy;

    if (!str1)
        str1 = "OOPS - str1 is 0";

    len = tc_strlen(comment);
    if (str1)
        len += tc_strlen(str1);

    tc_strcpy(s2, comment);
    tc_strcat((PFCHAR)s2, " ");

    if ((len+1) > SAY_STR_LEN)
    {
        say_out(out_type, (PFCHAR)"say_str: message below truncated");
        len_copy = SAY_STR_LEN - tc_strlen(comment) - 2;
        tc_strncpy((PFCHAR)s2+tc_strlen(comment), str1, len_copy);
        s2[SAY_STR_LEN-1] = '\0';
    }
    else
    {
        tc_strcat((PFCHAR)s2, str1);
    }
    say_out(out_type, s2);
}

void say_str2(int out_type, PFCCHAR comment, PFCHAR str1, PFCHAR str2)
{
char s2[SAY_STR_LEN];
int len;

    if (!str1)
        str1 = "OOPS - str1 is 0";
    if (!str2)
        str2 = "OOPS - str2 is 0";

    len = tc_strlen(comment) + tc_strlen(str1) + tc_strlen(str2);
    if ((len+1) > SAY_STR_LEN)
    {
        say_str(out_type, comment, str1);
        say_str(out_type, (PFCHAR)"     ", str2);
        return;
    }

    tc_strcpy((PFCHAR)s2, comment);
    tc_strcat((PFCHAR)s2, " ");
    tc_strcat((PFCHAR)s2, str1);
    say_str(out_type, s2, str2);
}



/* ********************************************************************   */
/* LOCK AND UNLOCK                                                        */
/* ********************************************************************   */
#if (INCLUDE_RTIP)

/* ********************************************************************   */
/* PORTS LISTS                                                            */
/* ********************************************************************   */
void say_ports_tcp(int out_type, PFCCHAR comment, dword val1)
{
#if (INCLUDE_TCP)
PTCPPORT port;
#endif
char s2[120];

    tc_strcpy(s2, comment);
    say_out(out_type, s2);

#if (INCLUDE_TCP)
    OS_CLAIM_TCP(SAY_PORTS_CLAIM_TCP)
    port = (PTCPPORT)root_tcp_lists[MASTER_LIST];
    while (port)
    {
        if (port->ap.list_type != MASTER_LIST)
            say_int(out_type, "say_ports - MASTER LIST - oop - list type = ",
                port->ap.list_type);

        say_int2(out_type, "say_ports - master - state,index = ",
            port->state, port->ap.ctrl.index);
        say_int2(out_type, "          - master - tcp_port_type, port_flags = ",
                port->tcp_port_type, port->ap.port_flags);
        if (port->ap.port_flags & PORT_BOUND)
        {
             DEBUG_ERROR(   "         - master - bound to ", IPADDR,
                 port->out_ip_temp.ip_src, 0);
        }
        say_int2(out_type, "          - master - out.port, in.port = ",
            port->out.port, port->in.port);
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[MASTER_LIST],
                                            (POS_LIST)port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }

    port = (PTCPPORT)root_tcp_lists[LISTEN_LIST];
    while (port)
    {
        if (port->ap.list_type != LISTEN_LIST)
            say_int(out_type, "say_ports - LISTEN LIST - oop - list type = ",
                port->ap.list_type);
        say_int2(out_type, "say_ports - listen - state,index = ",
            port->state, port->ap.ctrl.index);
        if (port->ap.port_flags & PORT_BOUND)
        {
             DEBUG_ERROR(   "         - listen - bound to ", IPADDR,
                 port->out_ip_temp.ip_src, 0);
        }
        say_int2(out_type, "          - listen - out.port, in.port = ",
            port->out.port, port->in.port);
        say_int2(out_type, "          - listen - tcp_port_type, port_flags = ",
                port->tcp_port_type, port->ap.port_flags);
        say_ip_addr(out_type, "          - listen - src IP addr ",
            port->out_ip_temp.ip_src);
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[LISTEN_LIST],
                                                (POS_LIST)port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }

    port = (PTCPPORT)root_tcp_lists[SOCKET_LIST];
    while (port)
    {
        if (port->ap.list_type != SOCKET_LIST)
            say_int(out_type, "say_ports - SOCKET LIST - oop - list type = ",
                port->ap.list_type);
        say_int2(out_type, "say_ports - socket - state,index = ",
                port->state, port->ap.ctrl.index);
        say_int2(out_type, "          - socket - tcp_port_type, port_flags = ",
                port->tcp_port_type, port->ap.port_flags);
        if (port->ap.port_flags & PORT_BOUND)
        {
             DEBUG_ERROR(   "         - socket - bound to ", IPADDR,
                 port->out_ip_temp.ip_src, 0);
        }
        say_int2(out_type, "          - socket - out.port, in.port = ",
            port->out.port, port->in.port);
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[SOCKET_LIST],
                                                (POS_LIST)port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }

    port = (PTCPPORT)root_tcp_lists[ACTIVE_LIST];
    while (port)
    {
        if (port->ap.list_type != ACTIVE_LIST)
            say_int(out_type, "say_ports - ACTIVE LIST - oop - list type = ",
                port->ap.list_type);
        say_int2(out_type, "say_ports - active - state,index = ",
                port->state, port->ap.ctrl.index);
        say_int2(out_type, "          - active - out.port, in.port = ",
            port->out.port, port->in.port);
        say_ip_addr(out_type, "          - active - src IP addr ",
            port->out_ip_temp.ip_src);
        say_ip_addr(out_type, "          - active - dest IP addr ",
            port->out_ip_temp.ip_dest);
        say_dint2(out_type, "          - active - port_flags (0x100=API_CLOSE_DONE),options(0x200=REUSE) = ",
                port->ap.port_flags, port->ap.options);
        say_int2(out_type, "          - active - in.contain, out.contain",
                port->in.contain, port->out.contain);
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST],
                                                (POS_LIST)port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }

    port = (PTCPPORT)root_tcp_lists[FREE_LIST];
    while (port)
    {
        if (port->ap.list_type != FREE_LIST)
            say_int(out_type, "say_ports - FREE LIST - oop - list type = ",
                port->ap.list_type);

        say_int2(out_type, "say_ports - free - state,index = ",
            port->state, port->ap.ctrl.index);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }
    OS_RELEASE_TCP()
#else
    ARGSUSED_INT(val1)
#endif      /* end of if INCLUDE_TCP */
}

void say_ports_udp(int out_type, PFCCHAR comment, dword val1)
{
#if (INCLUDE_UDP)
PUDPPORT udp_port;
PUDPPKT  udp_pkt;    /* Default UDP packet info in our port structure */
PIPPKT   ip_pkt;     /* Default UDP packet info in our port structure */
#if (INCLUDE_TRK_PKTS)
int  udp_exch;
word exchange;
DCU  msg;
DCU  root_msg;
#endif
#endif
char s2[120];

    tc_strcpy(s2, comment);
    say_out(out_type, s2);

#if (INCLUDE_UDP)
    OS_CLAIM_UDP(SAY_PORTS_CLAIM_UDP)
    /* ACTIVE LIST means connected or bound   */
    udp_port = (PUDPPORT)root_udp_lists[ACTIVE_LIST];
    while (udp_port)
    {
        udp_pkt = (PUDPPKT) &(udp_port->udp_connection);
        ip_pkt  = (PIPPKT) &(udp_port->ip_connection);
        say_int(out_type, "say_ports - active UDP port, index =",
                udp_port->ap.ctrl.index);
        say_dint2(out_type, "          - active UDP - dest port, src port = ",
                net2hs(udp_pkt->udp_dest),
                net2hs(udp_pkt->udp_source));
        say_ip_addr(out_type, "          - active UDP - src IP addr = ",
                    ip_pkt->ip_src);
        say_ip_addr(out_type, "          - active UDP - dest IP addr = ",
                    ip_pkt->ip_dest);

#if (INCLUDE_TRK_PKTS)
        exchange = OS_HNDL_TO_EXCH(PO_EX_UDP);

        root_msg = msg = (DCU) udp_port->ap.ctrl.exch_list[exchange];
        udp_exch = 0;
        cnt_pos_list(&udp_exch, msg, root_msg, (PTRACK_DCU)0);
        say_int(out_type, "          - active UDP port, DCUs on exchange ",
            udp_exch);
#endif

        udp_port = (PUDPPORT)os_list_next_entry_off(root_udp_lists[ACTIVE_LIST],
                                                    (POS_LIST)udp_port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }

    udp_port = (PUDPPORT)root_udp_lists[SOCKET_LIST];
    while (udp_port)
    {
        udp_pkt = (PUDPPKT) &(udp_port->udp_connection);
        ip_pkt  = (PIPPKT) &(udp_port->ip_connection);
        say_int(out_type, "say_ports - socket UDP port, index =",
                udp_port->ap.ctrl.index);
        say_dint2(out_type, "          - socket UDP - dest port, src port = ",
                udp_pkt->udp_dest, udp_pkt->udp_source);
        say_ip_addr(out_type, "          - socket UDP - src IP addr = ",
                    ip_pkt->ip_src);
        say_ip_addr(out_type, "          - socket UDP - dest IP addr = ",
                    ip_pkt->ip_dest);
#if (INCLUDE_TRK_PKTS)
        exchange = OS_HNDL_TO_EXCH(PO_EX_UDP);

        root_msg = msg = (DCU) udp_port->ap.ctrl.exch_list[exchange];
        udp_exch = 0;
        cnt_pos_list(&udp_exch, msg, root_msg, (PTRACK_DCU)0);
        say_int(out_type, "          - active UDP port, DCUs on exchange ",
            udp_exch);
#endif
        udp_port = (PUDPPORT)os_list_next_entry_off(root_udp_lists[SOCKET_LIST],
                                                    (POS_LIST)udp_port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }
#endif
    OS_RELEASE_UDP()
}

void say_ports_raw(int out_type, PFCCHAR comment, dword val1)
{
#if (INCLUDE_RAW)
PUDPPORT raw_port;
PUDPPKT  raw_pkt;    /* Default UDP packet info in our port structure */
PIPPKT   ip_pkt;     /* Default UDP packet info in our port structure */
#endif
char s2[120];

    tc_strcpy(s2, comment);
    say_out(out_type, s2);

#if (INCLUDE_RAW)
    raw_port = (PUDPPORT)root_raw_lists[ACTIVE_LIST];
    while (raw_port)
    {
        raw_pkt = (PUDPPKT) &(raw_port->udp_connection);
        ip_pkt  = (PIPPKT) &(raw_port->ip_connection);
        say_int(out_type, "say_ports - active UDP port, index =",
                raw_port->ap.ctrl.index);
        say_int2(out_type, "          - active UDP - dest port, src port = ",
                raw_pkt->udp_dest, raw_pkt->udp_source);
        say_ip_addr(out_type, "          - active UDP - src IP addr = ",
                    ip_pkt->ip_src);
        say_ip_addr(out_type, "          - active UDP - dest IP addr = ",
                    ip_pkt->ip_dest);
        raw_port = (PUDPPORT)os_list_next_entry_off(root_raw_lists[ACTIVE_LIST],
                                                    (POS_LIST)raw_port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }

    raw_port = (PUDPPORT)root_raw_lists[SOCKET_LIST];
    while (raw_port)
    {
        raw_pkt = (PUDPPKT) &(raw_port->udp_connection);
        ip_pkt  = (PIPPKT) &(raw_port->ip_connection);
        say_int(out_type, "say_ports - socket UDP port, index =",
                raw_port->ap.ctrl.index);
        say_int2(out_type, "          - socket UDP - dest port, src port = ",
                raw_pkt->udp_dest, raw_pkt->udp_source);
        say_ip_addr(out_type, "          - socket UDP - src IP addr = ",
                    ip_pkt->ip_src);
        say_ip_addr(out_type, "          - socket UDP - dest IP addr = ",
                    ip_pkt->ip_dest);
        raw_port = (PUDPPORT)os_list_next_entry_off(root_raw_lists[SOCKET_LIST],
                                                    (POS_LIST)raw_port, ZERO_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }
#else
    ARGSUSED_INT(val1)
#endif
}

void say_ports_ping(int out_type, PFCCHAR comment, dword val1)
{
#if (INCLUDE_PING)
PANYPORT port;
PIFACE pi;
int iface_off;
#endif
char s2[120];

    tc_strcpy(s2, comment);
    say_out(out_type, s2);

#if (INCLUDE_PING)
    LOOP_THRU_IFACES(iface_off)
    {
        /* get interface   */
        pi = tc_ino2_iface(iface_off, DONT_SET_ERRNO);
        if (pi)
        {

            port = (PANYPORT)(pi->ctrl.root_ping_ports);
            while (port)
            {
                say_int2(out_type, "say_ports - ping - iface, seq = ",
                    iface_off, port->ping_sequence);
                port = (PANYPORT)os_list_next_entry_off(pi->ctrl.root_ping_ports,
                                                        (POS_LIST)port, ZERO_OFFSET);
                if (val1)
                {
                    tm_cputs("Press any key to continue . . .");
                    tm_getch();
                    tm_cputs("\n");
                }
            }
        }
    }
#endif      /* end of if INCLUDE_PING */
}

#if (INCLUDE_ROUTING_TABLE)
/* ********************************************************************   */
/* ROUTING TABLE                                                          */
/* ********************************************************************   */
void say_rt_entry(int out_type, int i, PROUTE prt)
{
    if (i == -1)
        say_out(out_type, (PFCHAR)"rt entry");
    else
        say_int(out_type, "rt entry ", i);
    say_ip_addr(out_type, "  rt_dest  ", prt->rt_dest);
    say_ip_addr(out_type, "  rt_mask  ", prt->rt_mask);
    say_ip_addr(out_type, "  rt_gw    ", prt->rt_gw);
    say_int(out_type,     "  rt_iface  =", prt->rt_iface);
    say_dint(out_type,    "  rt_metric =", prt->rt_metric);
#if (INCLUDE_RT_TTL)
    say_int(out_type,     "  rt_ttl    =", prt->rt_ttl);
#endif
    say_int(out_type,     "  rt_flags  =", prt->rt_flags);
#if (INCLUDE_RT_LOCK)
    say_int(out_type,     "  rt_usecnt =", prt->rt_usecnt);
#endif
}

void say_rt(int out_type, dword val1)
{
int    i;
PROUTE prt;

    say_out(out_type, (PFCHAR)"ROUTING TABLE");
    for (i=0; i < CFG_RTSIZE; i++)
    {
        prt = (PROUTE)(&(rt_table[i]));

        /* if entry is not valid, continue   */
#if (INCLUDE_RT_TTL)
        if (prt->rt_ttl > 0)
#else
        if (prt->rt_flags & RT_INUSE)
#endif
        {
            say_rt_entry(out_type, i, prt);
            if (val1)
            {
                tm_cputs("Press any key to continue . . .");
                tm_getch();
                tm_cputs("\n");
            }
        }
    }

    say_out(out_type, (PFCHAR)"DEFAULT GATEWAYS");
    prt = (PROUTE)(root_rt_default);
    while (prt)
    {
        say_rt_entry(out_type, -1, prt);
        prt = (PROUTE)os_list_next_entry_off(root_rt_default,
                                             (POS_LIST)prt, ROUTE_OFFSET);
        if (val1)
        {
            tm_cputs("Press any key to continue . . .");
            tm_getch();
            tm_cputs("\n");
        }
    }
}
#endif      /* INCLUDE_ROUTING_TABLE */

#if (INCLUDE_ARP)
/*****************************************************************   */
/*ARP CACHE                                                          */
/*****************************************************************   */
void say_arpcache_entry(int out_type, int index)
{
PARPCACHE p;

    p = tc_arpcache;

    switch (p[index].arpc_state)
    {
    case ARPC_STATE_FREE:
        say_int(out_type, "arpcache index, state = FREE", index);
        break;
    case ARPC_STATE_PENDING:
        say_int(out_type, "arpcache index, state = PENDING", index);
        break;
    case ARPC_STATE_RESOLVED:
        say_int(out_type, "arpcache index, state = RESOLVED", index);
        break;
    }

    if (p[index].arpc_state != ARPC_STATE_FREE)
    {
        if (p[index].pi)
        {
            say_int(out_type, "interface = ", p[index].pi->ctrl.index);
        }
        say_dint2(out_type, "arpcache ttl, nretries= ",
            p[index].arpc_ttl, p[index].arpc_nretries);
        say_ip_addr(out_type, "arpcache IP addr = ", p[index].arpc_ip_addr);
        if (p[index].arpc_state == ARPC_STATE_RESOLVED)
        {
            say_ether_addr(out_type, "arpcache HW addr = ",
                p[index].arpc_hw_addr); /*added ethernet address  */
        }
    }
}
#endif /* INCLUDE_ARP */
#endif /* (INCLUDE_RTIP */

/* ********************************************************************   */
/* DEBUG ROUTINES - i.e. DEBUG_ERROR and DEBUG_LOG map to these routines  */
/* ********************************************************************   */
void do_debug_error(int io_type, PFCCHAR string, int type, dword val1, dword val2)
{
#if (INCLUDE_ROUTING_TABLE)
PROUTE prt;
#endif

    switch (type)
    {
    case NOVAR :
        say(io_type, string);
        break;
    case EBS_INT1  :
        say_int(io_type, string, (int)val1);
        break;
    case EBS_INT2  :
        say_int2(io_type, string, (int)val1, (int)val2);
        break;
    case LINT1 :
        say_lint(io_type, string, (long)val1);
        break;
    case LINT2 :
        say_lint2(io_type, string, (long)val1, (long)val2);
        break;
    case DINT1 :
        say_dint(io_type, string, (dword)val1);
        break;
    case DINT2 :
        say_dint2(io_type, string, (dword)val1, (dword)val2);
        break;
    case STR1 :
        say_str(io_type, string, (PFCHAR)val1);
        break;
    case STR2 :
        say_str2(io_type, string, (PFCHAR)val1, (PFCHAR)val2);
        break;
#if (INCLUDE_RTIP)
    case IPADDR:
        say_ip_addr(io_type, string, (PFBYTE)val1);
        break;
    case ETHERADDR:
        say_ether_addr(io_type, string, (PFBYTE)val1);
        break;
    case PKT:
        say_hex(io_type, string, (PFBYTE)val1, (int)val2);
        break;
#if (INCLUDE_ROUTING_TABLE)
    case RT_ENTRY:
        prt = (PROUTE)(&(rt_table[(int)val1]));
        say_rt_entry(io_type, (int)val1, prt);
        break;
    case RT:
        say_rt(io_type, val1);
#endif
        break;
    case ARPC_ENTRY:
#if (INCLUDE_ARP)
        say_arpcache_entry(io_type, (int)val1);
#endif
        break;
    case PORTS_ALL:
        say_ports_tcp(io_type, string, val1);
        say_ports_udp(io_type, string, val1);
        say_ports_raw(io_type, string, val1);
        say_ports_ping(io_type, string, val1);
        break;
    case PORTS_TCP:
        say_ports_tcp(io_type, string, val1);
        break;
    case PORTS_UDP:
        say_ports_udp(io_type, string, val1);
        break;
    case PORTS_RAW:
        say_ports_raw(io_type, string, val1);
        break;
    case PORTS_PING:
        say_ports_ping(io_type, string, val1);
        break;
#endif /* (INCLUDE_RTIP */
    default:
        say(io_type, "ERROR: debug_error() - invalid type for :");
        say(io_type, string);
        break;
    }

#    if (SEND_TO_FSR)
        /* write to file or RS232 also-level = 1 i.e. always write it   */
        /* NOTE: debug_log not DEBUG_LOG else might get compiled away   */
        debug_log(string, 1, type, val1, val2);
#    endif
}

void debug_error(PFCCHAR string, int type, dword val1, dword val2)
{
    do_debug_error(SCREEN_TYPE, string, type, val1, val2);
}

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
void xn_debug_error(PFCCHAR string, int type, dword val1, dword val2)
{
    do_debug_error(SCREEN_TYPE, string, type, val1, val2);
}



/*OS*/ #if (defined(SEND_TO_FSR)) /* was #if (SEND_TO_FSR) */
void debug_log(PFCCHAR string, int level, int type, dword val1, dword val2)
{
int out_type;
#if (INCLUDE_ROUTING_TABLE)
PROUTE prt;
#endif

    if (!print_debug_log)
        return;

    if (level > DEBUG_LEVEL)
        return;

    /* set up type, i.e. print to FILE or RS232; you can change this     */
    /* to SCREEN_TYPE if you want DEBUG_LOG to be printed to the screen; */
    /* NOTE: DEBUG_LOG will print numerous messages                      */
#if (IO_TYPE_FSR == DO_SCREEN_IO)
    out_type = SCREEN_TYPE;
#else
    out_type = FILE_TYPE;
#endif

    switch (type)
    {
    case NOVAR :
        say(out_type, string);
        break;
    case EBS_INT1  :
        say_int(out_type, string, (int)val1);
        break;
    case EBS_INT2  :
        say_int2(out_type, string, (int)val1, (int)val2);
        break;
    case LINT1 :
        say_lint(out_type, string, (long)val1);
        break;
    case LINT2 :
        say_lint2(out_type, string, (long)val1, (long)val2);
        break;
    case DINT1 :
        say_dint(out_type, string, (dword)val1);
        break;
    case DINT2 :
        say_dint2(out_type, string, (dword)val1, (dword)val2);
        break;
    case STR1 :
        say_str(out_type, string, (PFCHAR)val1);
        break;
    case STR2 :
        say_str2(out_type, string, (PFCHAR)val1, (PFCHAR)val2);
        break;
#if (INCLUDE_RTIP)
    case IPADDR:
        say_ip_addr(out_type, string, (PFBYTE)val1);
        break;
    case ETHERADDR:
        say_ether_addr(out_type, string, (PFBYTE)val1);
        break;
    case PKT:
        say_hex(out_type, string, (PFBYTE)val1, (int)val2);
        break;
#if (INCLUDE_ROUTING_TABLE)
    case RT_ENTRY:
        prt = (PROUTE)(&(rt_table[(int)val1]));
        say_rt_entry(out_type, (int)val1, prt);
    case RT:
        say_rt(out_type, val1);
#endif
        break;
    case ARPC_ENTRY:
#if (INCLUDE_ARP)
        say_arpcache_entry(out_type, (int)val1);
#endif
        break;
    case PORTS_ALL:
        say_ports_tcp(out_type, string, val1);
        say_ports_udp(out_type, string, val1);
        say_ports_raw(out_type, string, val1);
        say_ports_ping(out_type, string, val1);
        break;
    case PORTS_TCP:
        say_ports_tcp(out_type, string, val1);
        break;
    case PORTS_UDP:
        say_ports_udp(out_type, string, val1);
        break;
    case PORTS_RAW:
        say_ports_raw(out_type, string, val1);
        break;
    case PORTS_PING:
        say_ports_ping(out_type, string, val1);
        break;
#endif /* (INCLUDE_RTIP */
    default:
        say(out_type, "ERROR: debug_error() - invalid type for :");
        say(out_type, string);
        break;
    }
}
#endif

#endif  /* end of if (!USE_DB_L0) near beginning of file */

#if (INCLUDE_PROFILING)
/* ********************************************************************     */

/* Profiling package -
   This is an execution profiling package. It is used internally to
   profile code sections.
   Two macros: PROFILE_ENTER(TOKEN) and PROFILE_EXIT(TOKEN) are defined. If
   profiling is turned off they do nothing. If they are on they note the
   start time and end time for the section being profiled. The routine
   dump_profile() dumps the duration for each call. This profiling package
   does not do histogram analysis. It only logs the duration of the last
   pass through the code being profiled.. so it must be used carefully.

   Currently the only platform that supports profiling is the NET186 board.
*/

struct timerecord profile_time_array[PROF_N_RECORDS];
dword tick_every_10000_tenth_micros =0;

struct namerecord profile_name_array[PROF_N_RECORDS] = {
    {PROF_TOKEN_IP_INTERPRET, "IP INTERPRET"},
};

static char *get_name(int token)
{
int i;
    for (i = 0; i < PROF_N_RECORDS; i++)
    {
        if (token == profile_name_array[i].token)
            return(profile_name_array[i].name);
    }
    return("LOST");
}

void dump_profile(void)
{
int i;
dword duration;

    for (i = 0; i < PROF_N_RECORDS; i++)
    {
        if (profile_time_array[i].start_micro)
        {
            duration = profile_time_array[i].end_micro
                    - profile_time_array[i].start_micro;
            tm_printf("%20s ---------------- %8l\n", get_name(i), duration/10);
        }
    }
}


#endif


