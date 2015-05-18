/*                                                                      */
/* SYSLOG.C - syslog client functions                                   */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Ger 'Insh_Allah' Hobbelt, 2000                             */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_SYSLOG


#include    "sock.h"

#if (INCLUDE_SYSLOG)
#include    "bget.h"   /* malloc et al */
#include    "vfile.h"

#if !defined(DEBUG_SYSLOG)
#define DEBUG_SYSLOG               (DEBUG_LEVEL > 3)
#endif

#ifndef CFG_ADD_SEQUENCE_NUMBER2SYSLOG
#define CFG_ADD_SEQUENCE_NUMBER2SYSLOG  0  /* add '[SEQ:*****]' string to syslog header to track 'dropped' UDP syslog packets at the syslog server. */
#endif

#ifndef CFG_SYSLOG_RETRY_CONNECT_DELAY_COUNT
#define CFG_SYSLOG_RETRY_CONNECT_DELAY_COUNT  4  /* attempt new remote connect N lines 'delayed' after failed connect. */
                                             /* This value is > 0 if you want to reduce 'syslog' load in case parameters   */
                                             /* and/or remote servers are down/faulty.                                     */
#endif

#ifndef CFG_SYSLOG_RETRY_FINIT_DELAY_COUNT
#define CFG_SYSLOG_RETRY_FINIT_DELAY_COUNT    4  /* attempt new filesystem init and/or file open N lines 'delayed' after failed init/open. */
                                             /* This value is > 0 if you want to reduce 'syslog' load in case parameters   */
                                             /* and/or mounted filesystems are down/faulty.                                */
#endif

#if (DEBUG_SYSLOG)
#define SYSLOG_DIRECTDEBUG_LINE(arg)   DIRECTDEBUG_LINE(arg)
#else
#define SYSLOG_DIRECTDEBUG_LINE(arg)   /**/
#endif



#ifndef CFG_SYSLOG_THROUGH_RS232
#define CFG_SYSLOG_THROUGH_RS232 01
#endif

#if (CFG_SYSLOG_THROUGH_RS232 )
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

/* Select comm rate                                     */
/* #define  commRate        1    // 115200 baud divisor */
/* #define  commRate        6    // 19200 baud divisor  */
/* #define  commRate        12   // 9600 baud divisor   */
/* #define  commRate        48   // 2400 baud divisor   */
/* #define  commRate        96   // 1200 baud divisor   */

/*-------------------------------------------------------------------*
    Bit values held in the Line Control Register (LCR).
    bit   meaning
    ---   -------
    0-1   00=5 bits, 01=6 bits, 10=7 bits, 11=8 bits.
    2   Stop bits.
    3   0=parity off, 1=parity on.
    4   0=parity odd, 1=parity even.
    5   Sticky parity.
    6   Set break.
    7   Toggle port addresses.
 *-------------------------------------------------------------------*/
#define NO_PARITY       0x00
#define EVEN_PARITY     0x18
#define ODD_PARITY      0x08


#endif



int __xn_write2syslog(struct tc_printf_io *io, const char *msg, int msglen);

/*
 *  LEVEL      CODE      DESCRIPTION
 *
 *  LOG_EMERG   0       kernel panic
 *  LOG ALERT   1       condition needing immediate attention
 *  LOG_CRIT    2       critical conditions
 *  LOG_ERR     3       errors
 *  LOG_WARNING 4       warning messages
 *  LOG_NOTICE  5       not an error, but may need attention
 *  LOG_INFO    6       informational messages
 *  LOG_DEBUG   7       when debugging a system
 */



struct xd_syslog_node
{
    /* syslog to remote server   */
    int servernode;        /* socket handle; -1 if not opened! */
    char *servername;      /* syslog server name; resolvable through the DNS */
    int serverport;        /* default: XD_SYSLOG_SERVERPORT(514) */

    /* syslog to local file: TBD   */
#if (INCLUDE_VFS)
    int filehandle;        /* file handle; -1 if not opened! */
#elif INCLUDE_ERTFS_API
    PCFD filehandle;
#elif (INCLUDE_DOS_FS)
    int filehandle;
#endif

    char *filename;        /* syslog log filename */
    int retry_open_downcounter;
    int fileline_maxcount;
    int fileline_downcounter;
    RTIP_BOOLEAN localfilesys_is_initialized;

    /* syslog to serial port: TBD   */
    int serial_comm_port;  /* 1..4 for IBM PC compatibles; others may differ; -1 is 'undefined' */
    unsigned int serial_base_io_address;    /* I/O bus type addressing: i86 CPUs et al */
    byte * serial_base_address;             /* memory mapped I/O type addressing: MC68K CPUs et al */
    int serial_irq;                         /* IRQ for given serial port... */
    unsigned long int serial_baudrate;      /* 75 ... 115200 */
    unsigned serial_bits_per_byte: 4;       /* 5..8 */
    unsigned serial_parity       : 2;       /* O,E,N */
    unsigned serial_stopbits     : 2;       /* 1, 1.5, 2 */
    unsigned serial_xon_xoff     : 1;       /* support xon/xoff */
    unsigned serial_cts_rts      : 1;       /* support cts/rts hardware handshake */
    unsigned serial_dtr_dts      : 1;       /* support dtr/dts hardware handshake */
    unsigned serial_rts_on       : 1;       /* start with RTS on */
    unsigned serial_dtr_on       : 1;       /* start with DTR on */

    /* [i_a] which reminds me: TERMINAL.C, SYSLOG.C, UARTPORT.C, UART.C, MOUSE.C, (DEBUG.C):
     * they _all_ initialize the uart some way or another. And most are just copy-pasted from each other.
     * Shouldn't just someone, some day, change this code so we have a 'global serial pool' from which to
     * pick our port?
     *
     * My idea is this: let the user define a nice gloal array with UART ports; each with preferred
     * baud rates, handshake methods, bits-per-byte, parity, stopbits, the goods. And each piece of code
     * is handed a handle into that global pool to pick their port. Everyone using the same serial stuff,
     * but for different ports. And everyone can be happy then: the console, the debug logger, the PPP/SLIP
     * connection; everyone. And it's much more friendly to the user as [s]he won't have to dig through all
     * the very slightly different code sections listed above.
     * It also would make my syslogging-to-serial a lot easier as the initializing business is centralized.
     * Just a thought on a late night when baudrates just didn't match up so very well...
     * One copy-paste too many I guess.
     */
    RTIP_BOOLEAN serial_is_initialized;

    /* openlog() options   */
    char *prefix_string;
    int options;
    int facility;

    /* setlogmask() settings   */
    int priority_mask;

    /* special: re.establish connection after logging N lines   */
    int msg_maxcount;
    int msg_downcounter;

    /* statistics: total lines logged   */
    dword lines_logged;

    /* local data: buffer to construct UDP message across multiple calls   */
    unsigned char *data;
    int data_size;
    int data_fill;
    int header_length;  /* length of syslog header */

    byte server_ip_addr[IP_ALEN];
    int retry_decode_downcounter;

#if (CFG_ADD_SEQUENCE_NUMBER2SYSLOG != 0)
    unsigned long int sequence_number;
#endif

    /* syslog may transfer control to any IP task. Make sure no logging of those gets syslogged or you'll   */
    /* have multitasked recursive syslogging.                                                               */
    /*                                                                                                      */
    /* this list is a list of taskindexes (ks_get_task_index()), terminated by index -1.                    */
#define TASKINDEXES_ARRSIZE    CFG_NUM_TASK_CONTROL_BLOCKS /* IF tasks, Interrupt Tasks, Timer Task, Sentinel */
    int taskindexes[TASKINDEXES_ARRSIZE];
};

XNSYSLOG_CODE xd_sl_prioritynames_arr[] =
{
    { "alert",    LOG_ALERT },
    { "crit",     LOG_CRIT },
    { "debug",    LOG_DEBUG },
    { "emerg",    LOG_EMERG },
    { "err",      LOG_ERR },
    { "error",    LOG_ERR },        /* DEPRECATED */
    { "info",     LOG_INFO },
    { "notice",   LOG_NOTICE },
    { "panic",    LOG_EMERG },      /* DEPRECATED */
    { "warn",     LOG_WARNING },    /* DEPRECATED */
    { "warning",  LOG_WARNING },
    { NULL,       -1 },
};
XNSYSLOG_CODE *xd_sl_prioritynames = xd_sl_prioritynames_arr;


XNSYSLOG_CODE xd_sl_facilitynames_arr[] =
{
    { "auth",     LOG_AUTH },
    { "authpriv", LOG_AUTHPRIV },
    { "cron",     LOG_CRON },
    { "daemon",   LOG_DAEMON },
    { "ftp",      LOG_FTP },
    { "kern",     LOG_KERN },
    { "lpr",      LOG_LPR },
    { "mail",     LOG_MAIL },
    { "news",     LOG_NEWS },
    { "security", LOG_AUTH },       /* DEPRECATED */
    { "syslog",   LOG_SYSLOG },
    { "user",     LOG_USER },
    { "uucp",     LOG_UUCP },
    { "local0",   LOG_LOCAL0 },
    { "local1",   LOG_LOCAL1 },
    { "local2",   LOG_LOCAL2 },
    { "local3",   LOG_LOCAL3 },
    { "local4",   LOG_LOCAL4 },
    { "local5",   LOG_LOCAL5 },
    { "local6",   LOG_LOCAL6 },
    { "local7",   LOG_LOCAL7 },
    { NULL,       -1 },
};
XNSYSLOG_CODE *xd_sl_facilitynames = xd_sl_facilitynames_arr;

static struct xd_syslog_node xd_syslog_channel =
{
-1,                       /*int servernode;   socket handle; -1 if not opened!  */
0,                        /*char *servername; syslog server name; resolvable through the DNS  */
XD_SYSLOG_SERVERPORT,     /*int serverport;   default: XD_SYSLOG_SERVERPORT(514)  */

#if (INCLUDE_VFS)
-1,                       /*int filehandle;   file handle; -1 if not opened!  */
#elif INCLUDE_ERTFS_API
-1,                       /*PCFD filehandle; */
#elif (INCLUDE_DOS_FS)
-1,                       /*int filehandle; */
#endif

0,                        /*char *filename;   syslog log filename  */
0,                        /*int retry_open_downcounter; */
1,                        /*int fileline_maxcount; */
0,                        /*int fileline_downcounter; */
FALSE,                    /*RTIP_BOOLEAN localfilesys_is_initialized; */

-1,                       /*int serial_comm_port;  1..4 for IBM PC compatibles; others may differ; -1 is 'undefined'  */
0,                        /*unsigned int serial_base_io_address;    I/O bus type addressing: i86 CPUs et al  */
NULL,                     /*byte * serial_base_address;         memory mapped I/O type addressing: MC68K CPUs et al  */
0,                        /*int serial_irq;                    IRQ for given serial port...  */
0,                        /*unsigned long int serial_baudrate; 75 ... 115200  */
8,                        /*unsigned serial_bits_per_byte: 4;  5..8  */
0,                        /*unsigned serial_parity       : 2;  O,E,N  */
1,                        /*unsigned serial_stopbits     : 2;  1, 1.5, 2  */
!0,                       /*unsigned serial_xon_xoff     : 1;  support xon/xoff  */
0,                        /*unsigned serial_cts_rts      : 1;  support cts/rts hardware handshake  */
0,                        /*unsigned serial_dtr_dts      : 1;  support dtr/dts hardware handshake  */
0,                        /*unsigned serial_rts_on       : 1;  start with RTS on  */
0,                        /*unsigned serial_dtr_on       : 1;  start with DTR on  */

FALSE,                    /*RTIP_BOOLEAN serial_is_initialized; */

0,                        /*char *prefix_string; */
LOG_CONS | LOG_ODELAY,
                                                    /*int options;   */
-1,                       /*int facility; */

LOG_UPTO(LOG_DEBUG),      /*int priority_mask; */

0,                        /*int msg_maxcount; */
0,                        /*int msg_downcounter; */

0,                        /*dword lines_logged; */

NULL,                     /*unsigned char *data; */
0,                        /*int data_size; */
0,                        /*int data_fill; */
0,                        /*int header_length;  length of syslog header  */

{0,0,0,0},                /*byte server_ip_addr[IP_ALEN]; */
0,                        /*int retry_decode_downcounter; */

#if (CFG_ADD_SEQUENCE_NUMBER2SYSLOG != 0)
0,                        /*unsigned long int sequence_number; */
#endif

{ -1, -1 },               /*int taskindexes[TASKINDEXES_ARRSIZE]; */
};





/*
Name
     syslog, vsyslog, openlog, closelog, setlogmask - control system log

Synopsis
     void
     syslog(int priority, const char *message, ...);

     void
     vsyslog(int priority, const char *message, va_list args);

     void
     openlog(const char *ident, int logopt, int facility);

     void
     closelog(void);

     int
     setlogmask(int maskpri);

Description
     The syslog() function writes message to the system message logger.  The
     message is then written to the system console, log files, logged-in
     users, or forwarded to other machines as appropriate. (See syslogd(8).)

     The message is identical to a printf(3) format string, except that '%m'
     is replaced by the current error message. (As denoted by the global vari-
     able errno; see strerror(3).)  A single trailing newline is allowed, any
     other non-printable characters are encoded into a visible characters with
     strvisx(3).

     The vsyslog() function is an alternate form in which the arguments have
     already been captured using the variable-length argument facilities of
     varargs(3).

     The message is tagged with priority. Priorities are encoded as a facility
     and a level. The facility describes the part of the system generating the
     message.  The level is selected from the following ordered (high to low)
     list:

     LOG_EMERG     A panic condition.  This is normally broadcast to all
                   users.

     LOG_ALERT     A condition that should be corrected immediately, such as a
                   corrupted system database.

     LOG_CRIT      Critical conditions, e.g., hard device errors.

     LOG_ERR       Errors.

     LOG_WARNING   Warning messages.

     LOG_NOTICE    Conditions that are not error conditions, but should possi-
                   bly be handled specially.

     LOG_INFO      Informational messages.

     LOG_DEBUG     Messages that contain information normally of use only when
                   debugging a program.


     The openlog() function provides for more specialized processing of the
     messages sent by syslog() and vsyslog().  The parameter ident is a string
     that will be prepended to every message.  If openlog() is never called,
     or if ident is NULL, the program name is used.  The logopt argument is a
     bit field specifying logging options, which is formed by OR'ing one or
     more of the following values:

     LOG_CONS      If syslog() cannot pass the message to syslogd it will at-
                   tempt to write the message to the console
                   (''/dev/console.'')

     LOG_NDELAY    Open the connection to syslogd immediately.  Normally the
                   open is delayed until the first message is logged.  Useful
                   for programs that need to manage the order in which file
                   descriptors are allocated.

     LOG_PERROR    Write the message to standard error output as well to the
                   system log.

     LOG_PID       Log the process id with each message: useful for identify-
                   ing instantiations of daemons.

     The facility parameter encodes a default facility to be assigned to all
     messages that do not have an explicit facility encoded:

     LOG_AUTH      The authorization system: su(1),  getty(8),  login(8),
                   etc.

     LOG_AUTHPRIV  The same as LOG_AUTH, but logged to a file readable only by
                   selected individuals.

     LOG_CRON      The clock daemon.

     LOG_DAEMON    System daemons, such as routed(8),  that are not provided
                   for explicitly by other facilities.

     LOG_FTP       The Internet File Transfer Protocol daemon.

     LOG_KERN      Messages generated by the kernel.  These cannot be generat-
                   ed by any user processes.

     LOG_LPR       The line printer spooling system: lpr(1),  lpc(8),  lpd(8),
                    etc.

     LOG_MAIL      The mail system.

     LOG_NEWS      The network news system.

     LOG_SYSLOG    Messages generated internally by syslogd(8).

     LOG_USER      Messages generated by random user processes.  This is the
                   default facility identifier if none is specified.

     LOG_UUCP      The uucp system.

     LOG_LOCAL0    Reserved for local use.  Similarly for LOG_LOCAL1 through
                   LOG_LOCAL7.

     The closelog() function can be used to close the log file.

     The setlogmask() function sets the log priority mask to maskpri and re-
     turns the previous mask.  Calls to syslog() with a priority not set in
     maskpri are rejected.  The mask for an individual priority pri is calcu-
     lated by the macro LOG_MASK(pri); the mask for all priorities up to and
     including toppri is given by the macro LOG_UPTO(toppri);. The default al-
     lows all priorities to be logged.

Return Values
     The routines closelog(), openlog(), syslog() and vsyslog() return no val-
     ue.

     The routine setlogmask() always returns the previous log mask level.

Examples
           syslog(LOG_ALERT, "who: internal error 23");

           openlog("ftpd", LOG_PID | LOG_NDELAY, LOG_FTP);
           setlogmask(LOG_UPTO(LOG_ERR));
           syslog(LOG_INFO, "Connection from host %d", CallingHost);

           syslog(LOG_INFO|LOG_LOCAL2, "foobar error: %m");

See Also
     logger(1),  syslogd(8)

History
     These functions appeared in 4.2BSD.

Bugs
     The syslog(), vsyslog(), openlog(), closelog(), and setlogmask() func-
     tions may not be safely called concurrently from multiple threads, e.g.,
     the interfaces described by pthreads(3).

*/

static int convert_ctrl_codes(char *dst, const char *src, int src_len)
{
    int c;
    char *dst_base = dst;

    while ((src_len-- > 0) && (c = *src++))
    {
        if (tc_iscntrl(c))
        {
            switch (c)
            {
            case '\n':
                *dst++ = ' ';
                break;

            case '\t':
                *dst++ = '\t';
                break;

            case 015:  /* '\r' */
                if ((src_len > 0) && (*src == '\n')) break;   /* forget about '\r' if it's a CRLF sequence... */
            default:
                *dst++ = '^';
                *dst++ = (char)(c ^ 0100);
                break;

            case 0177:
                *dst++ = '^';
                *dst++ = '?';
                break;

            case '\b':
                *dst++ = '\\';
                *dst++ = 'b';
                break;

            case 007: /* '\a' */
                *dst++ = '\\';
                *dst++ = 'a';
                break;

            case '\v':
                *dst++ = '\\';
                *dst++ = 'v';
                break;

            case '\f':
                *dst++ = '\\';
                *dst++ = 'f';
                break;
            }
        }
        else if (!tc_isprint(c))
        {
            /* what to do with these??? dump as HEX char '\xNN'   */
            *dst++ = '\\';
            *dst++ = 'x';
            *dst++ = tc_hmap[c >> 4];
            *dst++ = tc_hmap[c & 0x0f];
        }
        else
        {
            *dst++ = (char)c;
        }
    }

    return (int)(dst - dst_base);
}

static int count_dstsize_due_to_ctrl_codes(const char *src, int src_len)
{
    int c;
    int count = 0;

    while ((src_len-- > 0) && (c = *src++))
    {
        if (tc_iscntrl(c))
        {
            switch (c)
            {
            case '\n':
                break;

            case '\t':
                break;

            case 015:  /* '\r' */
                if ((src_len > 0) && (*src == '\n')) continue;   /* forget about '\r' if it's a CRLF sequence... */
            case 0177:
            case '\b':
            case 007: /* '\a' */
            case '\v':
            case '\f':
            default:
                count++;
                break;
            }
        }
        else if (!tc_isprint(c))
        {
            /* what to do with these??? dump as HEX char '\xNN'   */
            count += 4;
            continue;
        }
        count++;
    }

    return count;
}



/* *****************************************************    */
/* COMM PORT ROUTINES                                       */
/* *****************************************************    */
/* Communication initialization. Call from main application */
/* initialization function.                                 */

#if (CFG_SYSLOG_THROUGH_RS232 )

static void ioDLY(void);


void syslog_serial_comm_init(void)
{
    byte baud_rate_divisor;
    byte lcr_val;

    switch (xd_syslog_channel.serial_comm_port)
    {
    case 1:
        xd_syslog_channel.serial_base_io_address = 0x3F8;   /* I/O address for COM1 */
        xd_syslog_channel.serial_irq             =     4;   /* IRQ for COM1 */
        break;

    case 2:
        xd_syslog_channel.serial_base_io_address = 0x2F8;   /* I/O address for COM2 */
        xd_syslog_channel.serial_irq             =     3;   /* IRQ for COM2 */
        break;

    case 3:
        xd_syslog_channel.serial_base_io_address = 0x3E8;   /* I/O address for COM3 */
        xd_syslog_channel.serial_irq             =     4;   /* IRQ for COM3 */
        break;

    case 4:
        xd_syslog_channel.serial_base_io_address = 0x2E8;   /* I/O address for COM4 */
        xd_syslog_channel.serial_irq             =     3;   /* IRQ for COM4 */
        break;

    default:
        xd_syslog_channel.serial_base_io_address = 0x3F8;   /* I/O address for COM1 */
        xd_syslog_channel.serial_irq             =     4;   /* IRQ for COM1 */
        SYSLOG_DIRECTDEBUG_LINE(("syslog_serial_comm_init: illegal comm port - set to 1"));
        break;
    }

    if ((xd_syslog_channel.serial_baudrate == 0)
            /* validate range: 300 .. 115200   */
            || (xd_syslog_channel.serial_baudrate < 300UL)
            || (xd_syslog_channel.serial_baudrate > 115200UL))
    {
        /* default baudrate = 19200   */
        SYSLOG_DIRECTDEBUG_LINE(("syslog_serial_comm_init: baud rate set to default: 19200 baud"));
        xd_syslog_channel.serial_baudrate = 19200UL;
    }

    baud_rate_divisor = (byte)((unsigned long int)(115200UL / xd_syslog_channel.serial_baudrate));

    /*Initialize UART   */
    ks_disable();

    /* Set communication rate and parameters   */
    OUTBYTE(xd_syslog_channel.serial_base_io_address + LCR, 0x80);   /*divisor access: Set DLAB */
    ioDLY();
    OUTBYTE(xd_syslog_channel.serial_base_io_address + BRDL, baud_rate_divisor);
    ioDLY();
    OUTBYTE(xd_syslog_channel.serial_base_io_address + BRDH, baud_rate_divisor >> 8);
    ioDLY();

    lcr_val = /* bits per byte */ 8 - 5;
/*  lcr_val |= ((StopBit == 1) ? 0x00 : 0x04);   */
    lcr_val |= NO_PARITY;

    OUTBYTE(xd_syslog_channel.serial_base_io_address + LCR, lcr_val);
    ioDLY();

    /* Clear existing interrupts   */
    INBYTE(xd_syslog_channel.serial_base_io_address + LSR);
    ioDLY();
    INBYTE(xd_syslog_channel.serial_base_io_address + MSR);
    ioDLY();
    INBYTE(xd_syslog_channel.serial_base_io_address + RBR);
    ioDLY();

    /* Disable all interrupts   */
    OUTBYTE(xd_syslog_channel.serial_base_io_address + IER, 0);
    ioDLY();

    /*Unmask comm interrupt in PIC                                        */
/*  OUTBYTE(IMREG, INBYTE(IMREG) & ~(1 << xd_syslog_channel.serial_irq)); */
/*  ioDLY();                                                              */

    /*Enable interrupt signal and DTR                              */
/*  OUTBYTE(xd_syslog_channel.serial_base_io_address + MCR, 0x0b); */
/*  ioDLY();                                                       */

    ks_enable();


    xd_syslog_channel.serial_is_initialized = TRUE;
}


void syslog_serial_send(PFBYTE msg, int len)
{

    while (len-- > 0)
    {
        unsigned char l;
        unsigned char ctl;

        /* Check for ^q   */
        l = (unsigned char)INBYTE(xd_syslog_channel.serial_base_io_address + LSR);
        if (l & 0x01)
        {
            ctl = (unsigned char)INBYTE(xd_syslog_channel.serial_base_io_address + RBR);
            if (ctl == 19) /* ^s */
            {
                do
                {
                    l = (unsigned char)INBYTE(xd_syslog_channel.serial_base_io_address + LSR);
                    if (l & 0x01)
                    {
                        ctl = (unsigned char)INBYTE(xd_syslog_channel.serial_base_io_address + RBR);
                    }
                } while (ctl != 17);  /* ^q */
            }
        }

        /* Wait for transmitter empty   */
        do
        {
            l = (unsigned char)INBYTE(xd_syslog_channel.serial_base_io_address + LSR);
        } while (!(l & 0x40));

        OUTBYTE(xd_syslog_channel.serial_base_io_address + THR, *msg++);
        ioDLY();
    }

}

static void ioDLY(void)
{
}

#else

void syslog_serial_comm_init(void)
{
    xd_syslog_channel.serial_is_initialized = TRUE;
    return;
}

void syslog_serial_send(PFBYTE msg, int len)
{
    return;
}

#endif




/* written_length = write(data, data_size, 'i/o info pointer'   */
int xn_write2syslog(struct tc_printf_io *io, KS_CONSTANT char *msg, int msglen)
{
    int ret;

    if (in_irq > 0)
    {
        /* ah! you shouldn't call syslog from an ISR!   */
        return -1;
    }

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
        return set_errno(ENOTINITIALIZED);
#endif

    /* see below: if this is recursive logging, then return immediately!   */
    io->channel.syslog.user = get_system_user();         /* get_system_user() will make sure ks_get_task_index() won't fail either! */
    io->channel.syslog.taskindex = ks_get_task_index();


/*  SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:entry before semaphore, user = %p, task = %d, section = %d", (void *)io->channel.syslog.user, (int)io->channel.syslog.taskindex, (int)io->propagate.ivalue));   */

    if (io->channel.syslog.user->write2syslog_active)
    {
        /* 'magic word': syslog call from inside syslog: drop 'em                                           */
/*    SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:recursive logging!!! (section: %d)", (int)io->propagate.ivalue)); */

        return msglen;
    }

    SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:wait for semaphore, msg = [%.*s], section = %d", (int)(msg ? msglen : 12), (msg ? msg : "\nFLUSH MARK\n"), (int)io->propagate.ivalue));
    OS_CLAIM_SYSLOG(WRITE2SYSLOG_CLAIM_SYSLOG);
    SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:got semaphore! section = %d", (int)io->propagate.ivalue));

    /*
     * make damn sure we can't get recursive syslog calls, due to our calling various other routines to write to
     * remote, VF(file), serial, terminal, etc.
     *
     * Now to make sure we're _not_ getting stuck up there inside xn_write2syslog(), waiting for our semaphore to
     * clear, which won't happen cause we're inside __xn_write2syslog() already(!), we'll check for this 'magic word'
     * _outside_ the semaphore-protected critical section! The 'magic word' is a flag which is stored inside RTIP
     * _per_ _task_. (get_system_user() references os_get_user() which in turn references other ks_xxx routines, which
     * will take care of identifying the currently active task and will return a reference to the 'struct sytem_user'
     * datablock which has been linked to the task.
     * Thus, by using get_system_user(), we'll assure ourselves __xn_write2syslog() is both multithreading safe
     * _and_ capable of withstanding 'recursive' logging, i.e. syslogging which originates from routines called
     * by __xn_write2syslog() to perform some operation (like sending a UDP message to a remote site).
     *
     * To make sure our 'recursive logging' protection code is working fine, the flag should ONLY be changed from inside
     * the 'syslog semaphore protected' areas! Checking the flag is allowed anywhere.
     *
     * Is that safe?? Assume task1 is in __xn_write2syslog() and task2 is outside __xn_write2syslog().
     * Case 1: task1 arrives at xn_write2syslog() (due to recursive logging!) and checks the flag BEFORE waiting for the
     * sempahore: this is OK; the flag will be set, so xn_write2syslog() will skip the syslog line.
     * Case 2: task2 arrives at xn_write2syslog() (due to multi-threading) and checks the flag. However, _this_ flag will be FALSE
     * as it is the flag for 'task2' instead of 'task1'. So task2 enters __xn_write2syslog() and performs some actions.
     *
     * Note that the 'critical section protection' by the semaphore will force task2 to wait until task1 has finished
     * with __xn_write2syslog(). This is considered harmless as UDP messages transmitted from task1 or 2 are queued before
     * being processed by task3. And the UDP transmission section is clearly marked for the diagnostic routines that use
     * this syslog facility, so recursive UDP traffic generation is very unlikely.
     */
    SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:flag within syslog, user = %p", (void *)io->channel.syslog.user));
    io->channel.syslog.user->write2syslog_active |= 0x01;

    ret = __xn_write2syslog(io, msg, msglen);

    io->channel.syslog.user->write2syslog_active &= ~0x01;
    SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:flag outside syslog, user = %p", (void *)io->channel.syslog.user));

    SYSLOG_DIRECTDEBUG_LINE(("xn_wr2sys:release semaphore"));
    OS_RELEASE_SYSLOG();

    return ret;
}



int __xn_write2syslog(struct tc_printf_io *io, KS_CONSTANT char *msg, int msglen)
{
    int patched_msglen;
    const char *next_part = NULL;
    int next_part_len = 0;
    int tx;
    int total_msglen = 0;
    int channel_options;

    /* check if we should skip this message (due to masked priority)   */
    if (!(LOG_MASK(io->channel.syslog.facility_and_priority) & xd_syslog_channel.priority_mask))
    {
        return msglen;
    }

    channel_options = xd_syslog_channel.options;
    channel_options &= io->channel.syslog.option_and_mask;
    channel_options |= io->channel.syslog.option_or_mask;

    /*
     * check if we're syslogging something from within any IP task. In that case, make sure LOG_REMOTE is temporarily disabled as that setting
     * will cause lockup.
     */
    {
        int i;

        for (i = 0; xd_syslog_channel.taskindexes[i] != -1; i++)
        {
            if (io->channel.syslog.taskindex == xd_syslog_channel.taskindexes[i])
            {
                SYSLOG_DIRECTDEBUG_LINE(("wr2sys: IP task logging detected: remote logging disabled for this line"));
                channel_options &= ~LOG_REMOTE;
            }
        }
    }

    SYSLOG_DIRECTDEBUG_LINE(("wr2sys:msglen=%d, msg=%.*s", msglen, msglen, msg));

    for (;;)
    {
        /* *collect/buffer* data in a packet until we hit '\n' or msglen == 0   */
#define DEFAULT_SYSLOG_PREFIX_STRING   "mar 12 12:34:56 ??pid?? "

        if (!(channel_options & LOG_MULTILINE))
        {
            /* track down LF for break!   */
            next_part = (const char *)tc_memchr(msg, '\n', msglen);
            if (next_part)
            {
                next_part++;
                next_part_len = (int)(msg + msglen - next_part);
                msglen -= next_part_len;
            }
            else
            {
                next_part_len = 0;
            }
        }
        SYSLOG_DIRECTDEBUG_LINE(("wr2sys: chopped: msglen=%d, msg=%.*s", msglen, msglen, msg));

        patched_msglen = count_dstsize_due_to_ctrl_codes(msg, msglen);
        if (!xd_syslog_channel.data)
        {
            int prefix_strlen = ( xd_syslog_channel.prefix_string
                                ? tc_strlen(xd_syslog_channel.prefix_string)
                                : sizeof(DEFAULT_SYSLOG_PREFIX_STRING))+12+3 /* approx. */
#if (CFG_ADD_SEQUENCE_NUMBER2SYSLOG != 0)
                                    +6+10
#endif
                                ;

            xd_syslog_channel.data = (unsigned char *)ks_malloc((1, xd_syslog_channel.data_size = (patched_msglen + prefix_strlen + 1)), SYSLOG_BUFFER_MALLOC);
            if (!xd_syslog_channel.data)
            {
                /* failure; abort write   */
                break;
            }
            xd_syslog_channel.data_fill = 0;
            xd_syslog_channel.header_length = 0;
        }

        if (msglen > 0)
        {
            /* construct syslog header at start of message   */
            if (xd_syslog_channel.data_fill == 0)
            {
                /* don't allow users to log kernel messages   */
                if ((io->channel.syslog.facility_and_priority & LOG_FACMASK) == LOG_KERN)
                {
                    io->channel.syslog.facility_and_priority &= ~LOG_FACMASK;
                    io->channel.syslog.facility_and_priority |= LOG_USER;
                }

#if (CFG_ADD_SEQUENCE_NUMBER2SYSLOG != 0)
                if (channel_options & LOG_REMOTE)
                {
                    /*
                     * only show 'SEQ:xxx' and increment counter if message is sent to remote host.
                     * After all, 'SEQ:xxx' is only intended to monitor 'UDP packet loss'!
                    */
                    tc_snprintf((char *)xd_syslog_channel.data, xd_syslog_channel.data_size, "<%u> %s"
                                "[SEQ:%010lu]"
                               ,(unsigned int)io->channel.syslog.facility_and_priority
                               ,(xd_syslog_channel.prefix_string ? xd_syslog_channel.prefix_string : DEFAULT_SYSLOG_PREFIX_STRING)
                               ,xd_syslog_channel.sequence_number++
                               );
                }
                else
#endif
                {
                    tc_snprintf((char *)xd_syslog_channel.data, xd_syslog_channel.data_size, "<%u> %s"
                               ,(unsigned int)io->channel.syslog.facility_and_priority
                               ,(xd_syslog_channel.prefix_string ? xd_syslog_channel.prefix_string : DEFAULT_SYSLOG_PREFIX_STRING)
                               );
                }
                xd_syslog_channel.header_length = xd_syslog_channel.data_fill = tc_strlen((const char *)xd_syslog_channel.data);
            }

            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: data[%d] = [%.*s]", xd_syslog_channel.data_fill, (int)xd_syslog_channel.data_fill, xd_syslog_channel.data));

            if (xd_syslog_channel.data_fill + patched_msglen >= xd_syslog_channel.data_size)
            {
                /*
                 * Do _not_ use realloc() type call here, as we _must_ keep the data intact if the call fails. realloc() type calls do not
                 * provide this guarantee.
                 */
                void *p = ks_malloc(1, xd_syslog_channel.data_fill + patched_msglen + 1, SYSLOG_BUFFER_MALLOC);

                if (!p)
                {
                    /* failure! Do not yell/report this, just clip the line and dump it...   */
                    msglen = 0;
                }
                else
                {
                    tc_movebytes(p, xd_syslog_channel.data, xd_syslog_channel.data_fill);
                    ks_free(xd_syslog_channel.data);
                    xd_syslog_channel.data = (unsigned char *)p;
                    xd_syslog_channel.data_size = xd_syslog_channel.data_fill + patched_msglen + 1;
                }
            }
        }

        /* [i_a] 'cool code' ... 'tx' is a 'C' style boolean and thus by definition either a 0 or 1 integer value. Which is nice...   */
        tx = (msglen == 0) || ((msglen > 0) && (msg[msglen - 1] == '\n'));

        if (msglen > 0)
        {
            xd_syslog_channel.data_fill += convert_ctrl_codes((char *)(xd_syslog_channel.data + xd_syslog_channel.data_fill), msg, msglen - tx);
        }
        total_msglen += patched_msglen;

        SYSLOG_DIRECTDEBUG_LINE(("wr2sys: tx=%d:channel_options=%04X, msglen=%d, msg=%.*s", tx, (int)channel_options, xd_syslog_channel.data_fill, xd_syslog_channel.data_fill, xd_syslog_channel.data));

        if (tx)
        {
            struct sockaddr_in sin;
            int failed = 0;
            /*
             * 'should_write_line': Write the msg if it contains relevant data (more than just the 'header') or if the call explicitly
             * passed a 'zero-length' msg-string.
             * The latter situation occurs if the user calls xn_syslog() with an empty string, which is a valid method to add 'empty log lines'.
             * Now who would like to do that, you may wonder... It happens.
             */
            RTIP_BOOLEAN should_write_line = ((xd_syslog_channel.data_fill > xd_syslog_channel.header_length) || (msg && !msglen) /* empty line, NOT the flush signal! */);

            xd_syslog_channel.lines_logged++;

            /* write terminating NUL at end of message   */
            xd_syslog_channel.data[xd_syslog_channel.data_fill] = 0;

            if (channel_options & LOG_REMOTE)
            {
                int old_node = xd_syslog_channel.servernode;

                failed |= LOG_REMOTE;
                xnsl_lock_blockinglist(TRUE);

                /* log to remote server!   */

                /* if N lines have been logged, re-connect.                                               */
                /* WARNING: if reconnect fails (DNS failure or alike), we should KEEP the old connection. */
                /*          Just on the odd chance that THAT one still operates correctly...              */
                if ((xd_syslog_channel.msg_maxcount > 0)
                        && (xd_syslog_channel.servernode >= 0)
                        && xd_syslog_channel.servername
                        && (--xd_syslog_channel.msg_downcounter < 0)
                     )
                {                                             
                    xd_syslog_channel.servernode = -1; /* simulate a 'closed' connection */
                    xd_syslog_channel.retry_decode_downcounter = 0;
                }

                while ((should_write_line || !!(channel_options & LOG_NDELAY))
                             /* do we have something to write? or should we open connection immediately anyway?   */
                             && xd_syslog_channel.servername
                             && (xd_syslog_channel.servernode < 0)
                             && (--xd_syslog_channel.retry_decode_downcounter < 0))
                {
                    /* make sure we don't resolve the faulty(?) servername every moment we want to write a message!   */
                    xd_syslog_channel.retry_decode_downcounter = CFG_SYSLOG_RETRY_CONNECT_DELAY_COUNT;
                    xd_syslog_channel.msg_downcounter = xd_syslog_channel.msg_maxcount;

                    /* Allocate a socket   */
                    if ((xd_syslog_channel.servernode = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                    {
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys:socket() failed. %s",xn_geterror_string(xn_getlasterror())));
                        break; /* Can't Allocate Socket --> goto failed_resolve */
                    }

                    /* Bind my ip address to port XD_SYSLOG_SERVERPORT(514)   */
                    sin.sin_family = AF_INET;
                    if (!xn_name2ip_addr(xd_syslog_channel.server_ip_addr, xd_syslog_channel.servername))
                    {
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys:nam2ip_addr() failed. %s",xn_geterror_string(xn_getlasterror())));
                        closesocket(xd_syslog_channel.servernode);
                        xd_syslog_channel.servernode = -1;
                        break; /* failed to decode IP number/name --> goto failed_resolve */
                    }
                    tc_movebytes((PFBYTE)&sin.sin_addr, xd_syslog_channel.server_ip_addr, IP_ALEN);
                    sin.sin_port = (word)(hs2net((word)(xd_syslog_channel.serverport ? xd_syslog_channel.serverport : XD_SYSLOG_SERVERPORT)));
                    if (connect(xd_syslog_channel.servernode, (PSOCKADDR)&sin, sizeof(sin)) < 0)
                    {
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys:connect() failed. %s",xn_geterror_string(xn_getlasterror())));
                        closesocket(xd_syslog_channel.servernode);
                        xd_syslog_channel.servernode = -1;
                        break;     /* Bind Failed  --> goto failed_resolve */
                    }

                    /* set the data socket to non-blocking UDP traffic!   */
                    if (!set_non_blocking_mode(xd_syslog_channel.servernode))
                    {
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys:set_non_blocking_mode() failed. %s",xn_geterror_string(xn_getlasterror())));
                        closesocket(xd_syslog_channel.servernode);
                        xd_syslog_channel.servernode = -1;
                        break;     /* set_non_blocking_mode failed  --> goto failed_resolve */
                    }
                    break;
                }
/* failed_resolve: ;   */

                /* check if old handle must be closed/kept...   */
                if ((xd_syslog_channel.servernode != old_node) && (old_node >= 0))
                {
                    if (xd_syslog_channel.servernode >= 0)
                    {
                        /* kill the old connection! we've got a new one now.   */
                        closesocket(old_node);
                    }
                    else
                    {
                        /* whoops! keep old handle intact!   */
                        xd_syslog_channel.servernode = old_node;
                    }
                }

                SYSLOG_DIRECTDEBUG_LINE(("wr2sys:write message? should_write_line: %d, node: %d",(int)should_write_line, (int)xd_syslog_channel.servernode));
                if (should_write_line)
                {
                    if (xd_syslog_channel.servernode >= 0)
                    {
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys: sendto=%d:msglen=%d, msg=%.*s", xd_syslog_channel.servernode, xd_syslog_channel.data_fill, xd_syslog_channel.data_fill, xd_syslog_channel.data));

                        msglen = sendto(xd_syslog_channel.servernode, (PFCCHAR)xd_syslog_channel.data, xd_syslog_channel.data_fill, 0 /* flags */, (PSOCKADDR)NULL, 0);
                        if (msglen < 0)
                        {
                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: sendto:msglen:%d,%s,msg=%.*s",(int)msglen, xn_geterror_string(xn_getlasterror()), xd_syslog_channel.data_fill, xd_syslog_channel.data));

                            closesocket(xd_syslog_channel.servernode);
                            xd_syslog_channel.servernode = -1;
                            xd_syslog_channel.retry_decode_downcounter = 0;  /* reconnect immediately! */
                        }
                        else
                        {
                            /* write is successfull.   */
                            failed &= ~LOG_REMOTE;

                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: sendto:success=%s",xn_geterror_string(xn_getlasterror())));
                        }
                    }
                }
                else
                {
                    /* zero length is always OK   */
                    failed &= ~LOG_REMOTE;
                }

                xnsl_lock_blockinglist(FALSE);
            }

            SYSLOG_DIRECTDEBUG_LINE(("wr2sys:after log_remote: fialed = %04X", failed));

            if (channel_options & LOG_LOCALFILE)
            {
                /* log to local logfile!   */
                failed |= LOG_LOCALFILE;
                xnsl_lock_blockinglist(TRUE);

#if (INCLUDE_VFS)

                while (xd_syslog_channel.filename) /* 'while' so we can break out on error :-) without using 'goto'. */
                {
#if (DEBUG_SYSLOG)
                    int ll;
#endif

                    SYSLOG_DIRECTDEBUG_LINE(("wr2sys: filename: %s, filehandle: %d, retry:downcounter: %d,"
                                             " lines:downcounter: %d, init: %d"
                                            ,xd_syslog_channel.filename
                                            ,(int)xd_syslog_channel.filehandle
                                            ,(int)xd_syslog_channel.retry_open_downcounter
                                            ,(int)xd_syslog_channel.fileline_downcounter
                                            ,(int)xd_syslog_channel.localfilesys_is_initialized
                                           ));

                    /* if N lines have been logged, close logfile and reopen it.   */
                    if ((xd_syslog_channel.fileline_maxcount > 0)
                            && (xd_syslog_channel.filehandle >= 0)
                            && (--xd_syslog_channel.fileline_downcounter < 0)
                         )
                    {
                        vf_close(xd_syslog_channel.filehandle);
                        xd_syslog_channel.filehandle = -1;
                        xd_syslog_channel.retry_open_downcounter = 0;
                    }

                    if ((should_write_line || !!(channel_options & LOG_NDELAY))
                        /* do we have something to write? or should we open file immediately anyway?   */
                        && (xd_syslog_channel.filehandle < 0)
                        && (--xd_syslog_channel.retry_open_downcounter < 0))
                    {
                        xd_syslog_channel.retry_open_downcounter = CFG_SYSLOG_RETRY_FINIT_DELAY_COUNT - 1;
                        xd_syslog_channel.fileline_downcounter = xd_syslog_channel.fileline_maxcount - 1;

                        if (!xd_syslog_channel.localfilesys_is_initialized)
                        {
                            if (vf_init() != 0)
                            {
                                /* oops! Virtual file system Init Failure! Don't try opening the logfile...   */
                                break;     /* goto failed_fileopen; */
                            }

                            /* kill old logfile; this is a new session starting!   */
                            vf_delete(xd_syslog_channel.filename);

                            xd_syslog_channel.localfilesys_is_initialized = !0;
                        }

                        xd_syslog_channel.filehandle = vf_open(xd_syslog_channel.filename, VO_RDWR | VO_APPEND | VO_BINARY, VS_IWRITE | VS_IREAD);
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys: vf_open(%s, %s) -> %d, errno: %d"
                                                ,"APPEND"
                                                ,xd_syslog_channel.filename
                                                ,(int)xd_syslog_channel.filehandle
                                                ,(int)xn_getlasterror()
                                               ));     
                        if (xd_syslog_channel.filehandle < 0)
                        {                      
                            xd_syslog_channel.filehandle = vf_open(xd_syslog_channel.filename, VO_WRONLY | VO_CREAT | VO_TRUNC | VO_BINARY, VS_IWRITE | VS_IREAD);
                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: vf_open(%s, %s) -> %d, errno: %d"
                                                    ,"CREATE"              
                                                    ,xd_syslog_channel.filename
                                                    ,(int)xd_syslog_channel.filehandle
                                                    ,(int)xn_getlasterror()
                                                   ));                                     
                        }                                               
                        else
                        {                      
                            /* RTFiles32 doesn't support VO_APPEND so we'd better make sure!   */
                            vf_lseek(xd_syslog_channel.filehandle, 0, VSEEK_END);
                        }
                    }

                    if (xd_syslog_channel.filehandle < 0)
                        break;     /* goto failed_fileopen; */

#if (DEBUG_SYSLOG)
                    ll = 0;
#endif
                    if (should_write_line)
                    {
                        /* make sure vf_write(..., len > 0) for each call.   */
                        if (((xd_syslog_channel.data_fill > xd_syslog_channel.header_length)
                             && ((xd_syslog_channel.data_fill - xd_syslog_channel.header_length)
                                 != (
#if (DEBUG_SYSLOG)
                                     ll =
#endif
                                        vf_write(xd_syslog_channel.filehandle, xd_syslog_channel.data + xd_syslog_channel.header_length, (word)(xd_syslog_channel.data_fill - xd_syslog_channel.header_length))))
                            )
                            || (vf_write(xd_syslog_channel.filehandle, (PFBYTE)"\r\n", (word)2) != 2))
                        {
                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: vf_write -> %d + CRLF, errno: %d"
                                                     ,(int)ll
                                                     ,(int)xn_getlasterror()
                                                     ));
                            /* write failure! flush & close.   */
                            vf_close(xd_syslog_channel.filehandle);
                            xd_syslog_channel.filehandle = -1;
                            xd_syslog_channel.retry_open_downcounter = 0;  /* make sure file is reopened immediately! */
                            break;     /* goto failed_fileopen; */
                        }
                    }

                    /* zero length or successfull write   */
                    failed &= ~LOG_LOCALFILE;
                    break;
                }
/* failed_fileopen: ;   */


#elif INCLUDE_ERTFS_API


                while (xd_syslog_channel.filename) /* 'while' so we can break out on error :-) without using 'goto'. */
                {
#if (DEBUG_SYSLOG)
                    int ll;
#endif

                    SYSLOG_DIRECTDEBUG_LINE(("wr2sys: filename: %s, filehandle: %d, downcounter: %d, init: %d"
                                             ,xd_syslog_channel.filename
                                             ,(int)xd_syslog_channel.filehandle
                                             ,(int)xd_syslog_channel.retry_open_downcounter
                                             ,(int)xd_syslog_channel.localfilesys_is_initialized
                                             ));

                    /* if N lines have been logged, close logfile and reopen it.   */
                    if ((xd_syslog_channel.fileline_maxcount > 0)
                            && (xd_syslog_channel.filehandle >= 0)
                            && (--xd_syslog_channel.fileline_downcounter < 0)
                         )
                    {
                        po_close(xd_syslog_channel.filehandle);
                        xd_syslog_channel.filehandle = -1;
                        xd_syslog_channel.retry_open_downcounter = 0;
                    }

                    if ((should_write_line || !!(channel_options & LOG_NDELAY))
                            /* do we have something to write? or should we open file immediately anyway?   */
                            && (xd_syslog_channel.filehandle < 0)
                            && (--xd_syslog_channel.retry_open_downcounter < 0))
                    {
                        xd_syslog_channel.retry_open_downcounter = CFG_SYSLOG_RETRY_FINIT_DELAY_COUNT - 1;
                        xd_syslog_channel.fileline_downcounter = xd_syslog_channel.fileline_maxcount - 1;

                        if (!xd_syslog_channel.localfilesys_is_initialized)
                        {
                            if (po_init() != 0)
                            {
                                /* oops! Virtual file system Init Failure! Don't try opening the logfile...   */
                                break;     /* goto failed_fileopen; */
                            }

                            /* kill old logfile; this is a new session starting!   */
                            pc_unlink(xd_syslog_channel.filename);

                            xd_syslog_channel.localfilesys_is_initialized = !0;
                        }

                        xd_syslog_channel.filehandle = po_open(xd_syslog_channel.filename, PO_RDWR | PO_APPEND | PO_BINARY, PS_IWRITE | PS_IREAD);
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys: po_open(PO_APPEND) -> %d, errno: %d"
                                                ,(int)xd_syslog_channel.filehandle
                                                ,(int)xn_getlasterror()
                                               ));
                        if (xd_syslog_channel.filehandle < 0)
                        {
                            xd_syslog_channel.filehandle = po_open(xd_syslog_channel.filename, PO_WRONLY | PO_CREAT | PO_BINARY, PS_IWRITE | PS_IREAD);
                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: po_open(PO_CREAT) -> %d, errno: %d"
                                                    ,(int)xd_syslog_channel.filehandle
                                                    ,(int)xn_getlasterror()
                                                   ));
                        }
                    }

                    if (xd_syslog_channel.filehandle < 0)
                        break;     /* goto failed_fileopen; */

#if (DEBUG_SYSLOG)
                    ll = 0;
#endif
                    if (should_write_line)
                    {
                        /* make sure vf_write(..., len > 0) for each call.   */
                        if (((xd_syslog_channel.data_fill > xd_syslog_channel.header_length)
                             && ((xd_syslog_channel.data_fill - xd_syslog_channel.header_length)
                                 != (
#if (DEBUG_SYSLOG)
                                     ll =
#endif
                                        po_write(xd_syslog_channel.filehandle, xd_syslog_channel.data + xd_syslog_channel.header_length, xd_syslog_channel.data_fill - xd_syslog_channel.header_length)))
                            )
                            || (po_write(xd_syslog_channel.filehandle, "\r\n", 2) != 2))
                        {
                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: po_write -> %d + CRLF, errno: %d"
                                                     ,(int)ll
                                                     ,(int)xn_getlasterror()
                                                     ));
                            /* write failure! flush & close.   */
                            po_close(xd_syslog_channel.filehandle);
                            xd_syslog_channel.filehandle = -1;
                            xd_syslog_channel.retry_open_downcounter = 0;  /* make sure file is reopened immediately! */
                            break;     /* goto failed_fileopen; */
                        }
                    }

                    /* zero length or successfull write   */
                    failed &= ~LOG_LOCALFILE;
                    break;
                }
/* failed_fileopen: ;   */


#elif (INCLUDE_DOS_FS)


                while (xd_syslog_channel.filename) /* 'while' so we can break out on error :-) without using 'goto'. */
                {
#if (DEBUG_SYSLOG)
                    int ll;
#endif

                    SYSLOG_DIRECTDEBUG_LINE(("wr2sys: filename: %s, filehandle: %d, downcounter: %d, init: %d"
                                             ,xd_syslog_channel.filename
                                             ,(int)xd_syslog_channel.filehandle
                                             ,(int)xd_syslog_channel.retry_open_downcounter
                                             ,(int)xd_syslog_channel.localfilesys_is_initialized
                                             ));

                    /* if N lines have been logged, close logfile and reopen it.   */
                    if ((xd_syslog_channel.fileline_maxcount > 0)
                            && (xd_syslog_channel.filehandle >= 0)
                            && (--xd_syslog_channel.fileline_downcounter < 0)
                         )
                    {
                        close(xd_syslog_channel.filehandle);
                        xd_syslog_channel.filehandle = -1;
                        xd_syslog_channel.retry_open_downcounter = 0;
                    }

                    if ((should_write_line || !!(channel_options & LOG_NDELAY))
                            /* do we have something to write? or should we open file immediately anyway?   */
                            && (xd_syslog_channel.filehandle < 0)
                            && (--xd_syslog_channel.retry_open_downcounter < 0))
                    {
                        xd_syslog_channel.retry_open_downcounter = CFG_SYSLOG_RETRY_FINIT_DELAY_COUNT - 1;
                        xd_syslog_channel.fileline_downcounter = xd_syslog_channel.fileline_maxcount - 1;

                        if (!xd_syslog_channel.localfilesys_is_initialized)
                        {
                            /* kill old logfile; this is a new session starting!   */
                            unlink(xd_syslog_channel.filename);

                            xd_syslog_channel.localfilesys_is_initialized = !0;
                        }

                        xd_syslog_channel.filehandle = open(xd_syslog_channel.filename, O_WRONLY | O_APPEND | O_CREAT | O_BINARY, S_IWRITE | S_IREAD);
                        SYSLOG_DIRECTDEBUG_LINE(("wr2sys: open -> %d, errno: %d"
                                                                     ,(int)xd_syslog_channel.filehandle
                                                                     ,(int)xn_getlasterror()
                                                                     ));
                    }

                    if (xd_syslog_channel.filehandle < 0)
                        break;     /* goto failed_fileopen; */

#if (DEBUG_SYSLOG)
                    ll = 0;
#endif
                    if (should_write_line)
                    {
                        /* make sure vf_write(..., len > 0) for each call.   */
                        if (((xd_syslog_channel.data_fill > xd_syslog_channel.header_length)
                                 && ((xd_syslog_channel.data_fill - xd_syslog_channel.header_length)
                                         != (
#if (DEBUG_SYSLOG)
                                                 ll =
#endif
                                                            write(xd_syslog_channel.filehandle, xd_syslog_channel.data + xd_syslog_channel.header_length, xd_syslog_channel.data_fill - xd_syslog_channel.header_length)))
                                )
                                || (write(xd_syslog_channel.filehandle, "\r\n", 2) != 2))
                        {
                            SYSLOG_DIRECTDEBUG_LINE(("wr2sys: write -> %d + CRLF, errno: %d"
                                                                         ,(int)ll
                                                                         ,(int)errno()
                                                                         ));
                            /* write failure! flush & close.   */
                            close(xd_syslog_channel.filehandle);
                            xd_syslog_channel.filehandle = -1;
                            xd_syslog_channel.retry_open_downcounter = 0;  /* make sure file is reopened immediately! */
                            break;     /* goto failed_fileopen; */
                        }
                    }

                    /* zero length or successfull write   */
                    failed &= ~LOG_LOCALFILE;
                    break;
                }
/* failed_fileopen: ;   */

#else

#if (DEBUG_SYSLOG) && 0
#error LOG_LOCALFILE not supported on systems without VFS or ERTFS or DOS_FS.
                /* [i_a] currently unsupported!   */
#endif


#endif

                xnsl_lock_blockinglist(FALSE);
            }

            SYSLOG_DIRECTDEBUG_LINE(("wr2sys:after log_localfile: fialed = %04X", failed));

            if ((channel_options & LOG_SERIALPORT) && should_write_line)
            {
                /* log to serial port!   */
                failed |= LOG_SERIALPORT;

                SYSLOG_DIRECTDEBUG_LINE(("wr2sys:log_serialport: init: %d", (int)xd_syslog_channel.serial_is_initialized));

                if (!xd_syslog_channel.serial_is_initialized)
                {
                    syslog_serial_comm_init();
                    /* xd_syslog_channel.serial_is_initialized will have been set if init OK   */
                }

                if (xd_syslog_channel.serial_is_initialized)
                {
                    if (xd_syslog_channel.data_fill > xd_syslog_channel.header_length)
                    {
                        syslog_serial_send(xd_syslog_channel.data + xd_syslog_channel.header_length, xd_syslog_channel.data_fill - xd_syslog_channel.header_length);
                    }
                    syslog_serial_send((PFBYTE)"\r\n", 2);

                    failed &= ~LOG_SERIALPORT;
                }
            }

            SYSLOG_DIRECTDEBUG_LINE(("wr2sys:after log_serialport: fialed = %04X", failed));

            if ((channel_options & LOG_PERROR) && should_write_line)
            {
                /* log to STDERR   */

                /* failed |= LOG_PERROR;   */

#if !(CONSOLE_DEVICE==CONDEV_SYSLOG)        /* prevent recursive loop */
                tm_printf("%s\n", xd_syslog_channel.data + xd_syslog_channel.header_length);
#endif

                /* failed &= ~LOG_PERROR;   */
            }

            if (channel_options & LOG_DEVNULL)
            {
                /* log to /dev/nul (bit bucket) + make sure 'failed' = FALSE   */
                failed = 0;
            }

            if (!(channel_options & LOG_ALL_OUTPUTS))
            {
                /* not a single output channel has been selected: fail!   */
                failed |= ~LOG_ALL_OUTPUTS;
            }

            SYSLOG_DIRECTDEBUG_LINE(("wr2sys:after 'any output channel check': fialed = %04X", failed));

            if (((channel_options & LOG_CONS) || failed) && should_write_line)
            {
                /* log to CONSOLE *if* syslog / file-I/O failed...                                                           */
                /*                                                                                                           */
                /* NOTE: failed is also 'TRUE' is msglen was 0 and message has already been flushed during the previous call */
                /*       bacause of our 'extremely smart heuristics' that watch for 'newline' ("\n") at the end of a         */
                /*       message particle. (See 'tx = ...' statement above)                                                  */
                /*       But _that_ situation doesn't arrive here as .data_fill will be too small!                           */

#if !(CONSOLE_DEVICE==CONDEV_SYSLOG)        /* prevent recursive loop */
                tm_puts((PFCCHAR)(xd_syslog_channel.data + xd_syslog_channel.header_length));
#endif
                /* msglen = -1;   */
            }

            /* clear buffer for next message!   */
            xd_syslog_channel.data_fill = xd_syslog_channel.header_length = 0;
        }

        if (next_part && next_part_len)
        {
            msg = next_part;
            msglen = next_part_len;
            continue;
        }

        return total_msglen;
    }

    /* failure!   */

    return -1;
}


void xn_vsyslog(int priority_and_facility, const char *msg, va_list args)
{
    struct tc_printf_io io = {0};

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return;
    }
#endif

    OS_CLAIM_SYSLOG(VSYSLOG_CLAIM_SYSLOG);

    io.channel.syslog.user = get_system_user();         /* get_system_user() will make sure ks_get_task_index() won't fail either! */
    io.channel.syslog.taskindex = ks_get_task_index();

    io.channel.syslog.user->write2syslog_active |= 0x01;

    /* keep 'facility' if none specified...   */
    if ((priority_and_facility & LOG_FACMASK) == 0)
    {
        priority_and_facility &= ~LOG_FACMASK;
        priority_and_facility |= (xd_syslog_channel.facility & LOG_FACMASK);
    }

    /* don't allow users to log kernel messages   */
    if ((priority_and_facility & LOG_FACMASK) == LOG_KERN)
    {
        priority_and_facility &= ~LOG_FACMASK;
        priority_and_facility |= LOG_USER;
    }

    io.write = &__xn_write2syslog;
    io.channel.syslog.facility_and_priority = priority_and_facility;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_vzprintf(&io, msg, args);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);

    io.channel.syslog.user->write2syslog_active &= ~0x01;

    OS_RELEASE_SYSLOG();
}






void xn_syslog(int priority, const char *message, ...)
{
    va_list ap;

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return;
    }
#endif

    va_start(ap, message);
    xn_vsyslog(priority, message, ap);
    va_end(ap);

#if (DEBUG_SYSLOG)
    va_start(ap, message);
    VDIRECTDEBUG_LINE(message, ap);
    va_end(ap);
#endif
}


void xn_openlog
    (const char *ident
    ,int logopt                          /* 0: keep default */
    ,int facility                        /* -1: keep default, 0: LOG_USER */
    ,const char *remote_servername       /* 0: keep default */
    ,int remote_serverport               /* -1 or 0: keep default */
    ,int reconnect_after_count_msgs      /* -1: keep default */
    ,const char *local_logfilename       /* 0: keep default */
    ,int close_logfile_after_count_msgs  /* -1: keep default */
    ,int serial_comm_port                /* -1: keep default */
    ,long int serial_baud_rate
    )
{
    struct tc_printf_io io = {0};

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return;
    }
#endif

    OS_CLAIM_SYSLOG(OPENLOG_CLAIM_SYSLOG);

    io.channel.syslog.user = get_system_user();         /* get_system_user() will make sure ks_get_task_index() won't fail either! */
    io.channel.syslog.taskindex = ks_get_task_index();

    io.channel.syslog.user->write2syslog_active |= 0x01;
    
    if (ident)
    {
        if (xd_syslog_channel.prefix_string)
            xn_free(xd_syslog_channel.prefix_string);
        xd_syslog_channel.prefix_string = (*ident ? xn_strdup(ident, SYSLOG_BUFFER_MALLOC) : NULL);
    }
    
    if (remote_servername)
    {
        if (xd_syslog_channel.servername)
            xn_free(xd_syslog_channel.servername);
        xd_syslog_channel.servername = (remote_servername && *remote_servername ? xn_strdup(remote_servername, SYSLOG_BUFFER_MALLOC) : NULL);
    }
    if (remote_serverport >= 0)
    {
        xd_syslog_channel.serverport = (remote_serverport <= 0 ? XD_SYSLOG_SERVERPORT : remote_serverport);
    }
    
    if (local_logfilename)
    {
        if (xd_syslog_channel.filename)
            xn_free(xd_syslog_channel.filename);
        xd_syslog_channel.filename = (local_logfilename && *local_logfilename ? xn_strdup(local_logfilename, SYSLOG_BUFFER_MALLOC) : NULL);
    }
    
    if (logopt)
        xd_syslog_channel.options = logopt;

    if (facility >= 0)
        xd_syslog_channel.facility = facility & LOG_FACMASK;
    if (!xd_syslog_channel.facility)
        xd_syslog_channel.facility = LOG_USER;
    if (reconnect_after_count_msgs >= 0)
        xd_syslog_channel.msg_maxcount = reconnect_after_count_msgs;
    if (close_logfile_after_count_msgs >= 0)
        xd_syslog_channel.fileline_maxcount = close_logfile_after_count_msgs;
    if (serial_comm_port >= 0)
    {
        xd_syslog_channel.options |= LOG_SERIALPORT;
        xd_syslog_channel.serial_comm_port = serial_comm_port;
        xd_syslog_channel.serial_baudrate = serial_baud_rate;
    }

    if (xd_syslog_channel.servername 
        && *xd_syslog_channel.servername 
        && xd_syslog_channel.serverport)
    {
        xd_syslog_channel.options |= LOG_REMOTE;
    }
    if (xd_syslog_channel.filename
        && *xd_syslog_channel.filename)
    {
        xd_syslog_channel.options |= LOG_LOCALFILE;
        xd_syslog_channel.retry_open_downcounter = 0;

#if (INCLUDE_VFS)
        if (xd_syslog_channel.filehandle >= 0)
        {
            vf_close(xd_syslog_channel.filehandle);
            xd_syslog_channel.filehandle = -1;
        }
#elif INCLUDE_ERTFS_API
        if (xd_syslog_channel.filehandle >= 0)
        {
            po_close(xd_syslog_channel.filehandle);
            xd_syslog_channel.filehandle = -1;
        }
#elif (INCLUDE_DOS_FS)
        if (xd_syslog_channel.filehandle >= 0)
        {
            close(xd_syslog_channel.filehandle);
            xd_syslog_channel.filehandle = -1;
        }
#endif
    }
    if ((xd_syslog_channel.options & LOG_NDELAY) != LOG_NDELAY)
    {
        xd_syslog_channel.options |= LOG_ODELAY;
    }

    /* trigger open/connect code right now!   */
    io.write = &__xn_write2syslog;
    (*io.write)(&io, NULL, 0);

    io.channel.syslog.user->write2syslog_active &= ~0x01;

    OS_RELEASE_SYSLOG();
}

void xn_closelog(void)
{
    struct tc_printf_io io = {0};

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return;
    }
#endif

    OS_CLAIM_SYSLOG(CLOSELOG_CLAIM_SYSLOG);

    io.channel.syslog.user = get_system_user();         /* get_system_user() will make sure ks_get_task_index() won't fail either! */
    io.channel.syslog.taskindex = ks_get_task_index();

    io.channel.syslog.user->write2syslog_active |= 0x01;

    io.write = &__xn_write2syslog;
    (*io.write)(&io, NULL, 0);

    /* plus some very special code to make sure connections, files, etc. are closed (and reopened) too!   */
    if (xd_syslog_channel.servernode >= 0)
    {
        closesocket(xd_syslog_channel.servernode);
        xd_syslog_channel.servernode = -1;
        xd_syslog_channel.retry_decode_downcounter = 0;  /* reconnect immediately! */
    }

#if (INCLUDE_VFS)
    if (xd_syslog_channel.filehandle >= 0)
    {
        vf_close(xd_syslog_channel.filehandle);
        xd_syslog_channel.filehandle = -1;
    }
#elif (INCLUDE_ERTFS_API)
    if (xd_syslog_channel.filehandle >= 0)
    {
        po_close(xd_syslog_channel.filehandle);
        xd_syslog_channel.filehandle = -1;
    }
#elif (INCLUDE_DOS_FS)
    if (xd_syslog_channel.filehandle >= 0)
    {
        close(xd_syslog_channel.filehandle);
        xd_syslog_channel.filehandle = -1;
    }
#endif

    io.channel.syslog.user->write2syslog_active &= ~0x01;

    OS_RELEASE_SYSLOG();
}

int xn_setlogmask(int maskpri)
{
    int old;

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
        return set_errno(ENOTINITIALIZED);
#endif

    OS_CLAIM_SYSLOG(SETLOGMASK_CLAIM_SYSLOG);

    old = xd_syslog_channel.priority_mask;
    xd_syslog_channel.priority_mask = maskpri;

    OS_RELEASE_SYSLOG();

    return old;
}


void xnsl_add_taskindex_to_blockinglist(int taskindex)
{
    int i;

    SYSLOG_DIRECTDEBUG_LINE(("xnsl: add task %d", (int)taskindex));

    OS_CLAIM_SYSLOG(ADDTASKIDX_CLAIM_SYSLOG);

    for (i = 0; i < TASKINDEXES_ARRSIZE - 1; i++)
    {
        if (xd_syslog_channel.taskindexes[i] == taskindex)
        {
            OS_RELEASE_SYSLOG();
            return; /* allready there. */
        }
        if (xd_syslog_channel.taskindexes[i] == -1)
        {
            /* add index   */
            xd_syslog_channel.taskindexes[i++] = taskindex;
            xd_syslog_channel.taskindexes[i] = -1;

            OS_RELEASE_SYSLOG();

            SYSLOG_DIRECTDEBUG_LINE(("xnsl: added task at [i:%d]<%d, task %d<%d, user: %p", 
                (int)i-1, (int)TASKINDEXES_ARRSIZE, 
                (int)xd_syslog_channel.taskindexes[i-1], 
                (int)CFG_NUM_TASK_CONTROL_BLOCKS, 
                (void *)os_get_user_by_index(xd_syslog_channel.taskindexes[i-1])));
            return;
        }
    }

    OS_RELEASE_SYSLOG();

    /* shouldn't get here!   */
    SYSLOG_DIRECTDEBUG_LINE(("xnsl: Possibly Fatal: out of elements in the syslog.taskindexes array!"));

#if (DEBUG_SYSLOG)
    /* boom! (NULL address access)   */
    ((char *)0)[0] = 0;
#endif


    return;
}


void xnsl_lock_blockinglist(RTIP_BOOLEAN flag)
{
    int i;

    if (flag)
    {
        /* lock   */
        for (i = 0; xd_syslog_channel.taskindexes[i] != -1; i++)
        {
            SYSLOG_DIRECTDEBUG_LINE(("xnsl_lock: lock [i:%d]<%d, task %d<%d, user: %p", 
                (int)i, 
                (int)TASKINDEXES_ARRSIZE, 
                (int)xd_syslog_channel.taskindexes[i], 
                (int)CFG_NUM_TASK_CONTROL_BLOCKS, 
                (void *)os_get_user_by_index(xd_syslog_channel.taskindexes[i])));
            os_get_user_by_index(xd_syslog_channel.taskindexes[i])->write2syslog_active |= 0x02;
        }
    }
    else
    {
        /* release   */
        for (i = 0; xd_syslog_channel.taskindexes[i] != -1; i++)
        {
            SYSLOG_DIRECTDEBUG_LINE(("xnsl_lock: UNlock [i:%d]<%d, task %d<%d, user: %p", 
                (int)i, 
                (int)TASKINDEXES_ARRSIZE, 
                (int)xd_syslog_channel.taskindexes[i], 
                (int)CFG_NUM_TASK_CONTROL_BLOCKS, 
                (void *)os_get_user_by_index(xd_syslog_channel.taskindexes[i])));
            os_get_user_by_index(xd_syslog_channel.taskindexes[i])->write2syslog_active &= ~0x02;
        }
    }

    return;
}


#endif /* INCLUDE_SYSLOG */



