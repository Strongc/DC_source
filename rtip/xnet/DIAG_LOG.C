/*                                                                               */
/* DIAG_LOG.C - log/diagnostics/debug/error reporting functions: replace DEBUG.C */
/*                                                                               */
/*                                                                               */
/*   EBS - RTIP                                                                  */
/*                                                                               */
/*   Copyright Peter Van Oudenaren , 1993                                        */
/*   All rights reserved.                                                        */
/*   This code may not be redistributed in source or linkable object form        */
/*   without the consent of its author.                                          */
/*                                                                               */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DIAGNOSTICS


#include    "sock.h"

#if (CFG_USE_DIAG_MSG)

#if (INCLUDE_SNMP)
#include "snmp.h"
#endif

#if !defined(SAY_RADIX)
#define SAY_RADIX 16      /* 16 = integers written in hex */
                                                    /* 10 = integers written in decimal   */
#endif

int diag_debug_syslog_level_limit = LOG_DEBUG;


void  ___rtip_tracedo_say_screen(xd_diag_section_t section, const char *s1, int line, const char *fname)      /* __fn__ */
{
    const char *p = strrchr(fname, '\\');
    if (!p)
        p = strrchr(fname, '/');
    if (!p)
        p = fname;
    else
        p++;

    if (!s1) s1 = "(null)";
#if 01
    DIRECTDEBUG_LINE(("[TRACE]%s - %d:%s", s1, line, p));
#else
    diag_debug_message(section, LOG_EMERG, "[TRACE]%s - %d:%s", s1, line, p);
#endif
}


#if (CFG_USE_DIAG_MSG)

int directdebug_printf_write(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen)
{
    static int debug_line_counter = 0;
    static int last_c_is_NL = 0;
    char str[33];
    int len = msglen;
    KS_CONSTANT char *p;

    /* count # of lines debug-dumped....   */
    if (len > 0)
    {
        for (p = msg; p; )
        {
            if ((int)(msg + len - p) > 0)
                p = tc_memchr(p, '\n', msg + len - p);
            else
                break;

            if (!p) break;
            debug_line_counter++;
            p++;
        }

        /* keep track of NewLine-termination of message[parts]...   */
        last_c_is_NL = (msg[len - 1] == '\n');
    }
    else if (!last_c_is_NL && !msg && !len)
    {
        /* EOL mark: add extra NL if necessary!   */
        msg = "\n";
        msglen = len = 1;

        last_c_is_NL = !0;
        debug_line_counter++;
    }

    /* dump message to console/where-ever   */
    while (len >= sizeof(str))
    {
        tc_movebytes(str, msg, sizeof(str)-1);
        str[sizeof(str)-1] = 0;
#if !(CONSOLE_DEVICE==CONDEV_SYSLOG)        /* prevent recursive loop */
        tm_cputs(str);
#endif
        len -= sizeof(str)-1;
        msg += sizeof(str)-1;
    }

    if (len > 0)
    {
        tc_movebytes(str, msg, len);
        str[len] = 0;
#if !(CONSOLE_DEVICE==CONDEV_SYSLOG)        /* prevent recursive loop */
        tm_cputs(str);
#endif
    }

    if (debug_line_counter > 15)
    {
/*    tm_printf("[press key...]");    */
/*    tm_getch();                     */
        tm_printf("\r");
        debug_line_counter = 0;
    }

    return msglen;
}



/* tm_printf() clone with auto-newline padding and 'pause' option. Once I get tm_getch() *NOT* to crash :-(   */
void vdirectdebug_line_printf(PFCCHAR f, va_list ap)
{
    struct tc_printf_io io = {0};

    io.write = &directdebug_printf_write;

    tc_vzprintf(&io, f, ap);
    (*io.write)(&io, NULL, 0); /* signal end of line... */
}

void directdebug_line_printf(PFCCHAR f, ...)
{
    va_list ap;

    va_start(ap,f);
    vdirectdebug_line_printf(f, ap);
    va_end(ap);
}

#endif /* #if (CFG_USE_DIAG_MSG) */



























extern RTIP_BOOLEAN KS_FAR print_debug_log;
#if (INCLUDE_ROUTING_TABLE)
extern ROUTE KS_FAR rt_table[CFG_RTSIZE];   /* routing table */
extern POS_LIST root_rt_default;
#endif


/* ********************************************************************   */
/* DEBUG ROUTINES - i.e. DEBUG_ERROR and DEBUG_LOG map to these routines  */
/* ********************************************************************   */
void diag_debug_errorlog(xd_diag_section_t section, int level, PFCCHAR string, int type, dword val1, dword val2)
{
#if (INCLUDE_ROUTING_TABLE)
    PROUTE prt;
#endif
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

/*  tc_zprintf(&io, "{syslog=%d,in_irq=%d,%d}", (int)in_xn_syslog, (int)in_irq, (int)31415);   */
    switch (type)
    {
    case NOVAR :
        tc_zprintf(&io, "%s", string);
        break;
    case EBS_INT1  :
        tc_zprintf(&io, "%s%d", string, (int)val1);
        break;
    case EBS_INT2  :
        tc_zprintf(&io, "%s%d,%d", string, (int)val1, (int)val2);
        break;
    case LINT1 :
        tc_zprintf(&io, "%s%ld", string, (long)val1);
        break;
    case LINT2 :
        tc_zprintf(&io, "%s%ld,%ld", string, (long)val1, (long)val2);
        break;
    case DINT1 :
        tc_zprintf(&io, "%s%lu", string, (dword)val1);
        break;
    case DINT2 :
        tc_zprintf(&io, "%s%lu,%lu", string, (dword)val1, (dword)val2);
        break;
    case PTR1 :
        tc_zprintf(&io, "%s%p", string, (PFVOID)val1);
        break;
    case PTR2 :
        tc_zprintf(&io, "%s%p,%p", string, (PFVOID)val1, (PFVOID)val2);
        break;
    case STR1 :
        tc_zprintf(&io, "%s[%s]", string, (PFCHAR)val1);
        break;
    case STR2 :
        tc_zprintf(&io, "%s[%s][%s]", string, (PFCHAR)val1, (PFCHAR)val2);
        break;
#if (INCLUDE_RTIP)
    case IPADDR:
        tc_zprintf(&io, "%s[%pA]", string, (PFBYTE)val1);
        break;
    case ETHERADDR:
        tc_zprintf(&io, "%s = [%pM]", string, (PFBYTE)val1);
        break;
    case PKT:
        tc_zprintf(&io, "%s%s%*pD", string, (val2 > 16 ? "\n" : " "), (int)val2, (PFBYTE)val1);
        break;
#if (INCLUDE_ROUTING_TABLE)
    case RT_ENTRY:
        prt = (PROUTE)(&(rt_table[(int)val1]));
        diag_say_rt_entry(section, level, string, (int)val1, prt);
    case RT:
        diag_say_rt(section, level, string);
        break;
#endif
    case ARPC_ENTRY:
#if (INCLUDE_ARP)
        diag_say_arpcache_entry(section, level, string, (int)val1);
        break;
#endif
    case PORTS_ALL:
        diag_say_ports_tcp(section, level, string);
        diag_say_ports_udp(section, level, string);
        diag_say_ports_raw(section, level, string);
        diag_say_ports_ping(section, level, string);
        break;
    case PORTS_TCP:
        diag_say_ports_tcp(section, level, string);
        break;
    case PORTS_UDP:
        diag_say_ports_udp(section, level, string);
        break;
    case PORTS_RAW:
        diag_say_ports_raw(section, level, string);
        break;
    case PORTS_PING:
        diag_say_ports_ping(section, level, string);
        break;
#endif /* (INCLUDE_RTIP) */
    default:
        tc_zprintf(&io, "ERROR: debug_error() - invalid type for : [%s]", string);
        break;
    }

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_debug_log(xd_diag_section_t section, int level, PFCCHAR string, int type, dword val1, dword val2)
{
    if (!print_debug_log)
        return;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    diag_debug_errorlog(section, level, string, type, val1, val2);
}

#if (CFG_USE_DIAG_MSG)

void diag_debug_message(xd_diag_section_t section, int level, PFCCHAR string, ...)
{
    va_list ap;
    struct tc_printf_io io = {0};

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
    {
        return;
    }
    io.propagate.ivalue = (int)section;
    va_start(ap, string);

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;
    tc_vzprintf(&io, string, ap);
    va_end(ap);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

#endif  /* #if (CFG_USE_DIAG_MSG) */



/* *****************************************************   */
/* FORMAT ROUTINES                                         */
/* *****************************************************   */

void diag_say_hex(xd_diag_section_t section, int level, PFCCHAR comment, PFBYTE p, int len)     /* __fn__ */
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s%s%*pD" /* was: '%*.25pD' */
                        ,(comment ? comment : "")
                        ,(comment ? (len > 16 ? "\n" : " ") : "")
                        ,len
                        ,p
                        );
    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}


void diag_say(xd_diag_section_t section, int level, PFCCHAR comment)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s", comment);
    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_int(xd_diag_section_t section, int level, PFCCHAR comment, int val)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

#if (SAY_RADIX == 16)
    tc_zprintf(&io, "%s %X", comment, val);
#elif (SAY_RADIX == 8)
    tc_zprintf(&io, "%s %o", comment, val);
#elif (SAY_RADIX == 10)
    tc_zprintf(&io, "%s %d", comment, val);
#else
#error this SAY_RADIX value is not supported
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ip_addr(xd_diag_section_t section, int level, PFCCHAR comment, byte KS_FAR *addr)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s%pA", comment, (struct in_addr *)addr);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ether_addr(xd_diag_section_t section, int level, PFCCHAR comment, byte KS_FAR *addr)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s = %pM", comment, addr);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_int2(xd_diag_section_t section, int level, PFCCHAR comment, int val1, int val2)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

#if (SAY_RADIX == 16)
    tc_zprintf(&io, "%s %X %X", comment, val1, val2);
#elif (SAY_RADIX == 8)
    tc_zprintf(&io, "%s %o %o", comment, val1, val2);
#elif (SAY_RADIX == 10)
    tc_zprintf(&io, "%s %d %d", comment, val1, val2);
#else
#error this SAY_RADIX value is not supported
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_lint(xd_diag_section_t section, int level, PFCCHAR comment, long val1)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

#if (SAY_RADIX == 16)
    tc_zprintf(&io, "%s %lX", comment, val1);
#elif (SAY_RADIX == 8)
    tc_zprintf(&io, "%s %o %lo", comment, val1);
#elif (SAY_RADIX == 10)
    tc_zprintf(&io, "%s %d %ld", comment, val1);
#else
#error this SAY_RADIX value is not supported
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_lint2(xd_diag_section_t section, int level, PFCCHAR comment, long val1, long val2)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

#if (SAY_RADIX == 16)
    tc_zprintf(&io, "%s %lX %lX", comment, val1, val2);
#elif (SAY_RADIX == 8)
    tc_zprintf(&io, "%s %lo %lo", comment, val1, val2);
#elif (SAY_RADIX == 10)
    tc_zprintf(&io, "%s %ld %ld", comment, val1, val2);
#else
#error this SAY_RADIX value is not supported
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_dint(xd_diag_section_t section, int level, PFCCHAR comment, dword val1)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

#if (SAY_RADIX == 16)
    tc_zprintf(&io, "%s %lX", comment, val1);
#elif (SAY_RADIX == 8)
    tc_zprintf(&io, "%s %lo", comment, val1);
#elif (SAY_RADIX == 10)
    tc_zprintf(&io, "%s %lu", comment, val1);
#else
#error this SAY_RADIX value is not supported
#endif
}

void diag_say_dint2(xd_diag_section_t section, int level, PFCCHAR comment, dword val1, dword val2)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

#if (SAY_RADIX == 16)
    tc_zprintf(&io, "%s %lX %lX", comment, val1, val2);
#elif (SAY_RADIX == 8)
    tc_zprintf(&io, "%s %lo %lo", comment, val1, val2);
#elif (SAY_RADIX == 10)
    tc_zprintf(&io, "%s %lu %lu", comment, val1, val2);
#else
#error this SAY_RADIX value is not supported
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ptr(xd_diag_section_t section, int level, PFCCHAR comment, PFVOID val1)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s %p", comment, val1);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ptr2(xd_diag_section_t section, int level, PFCCHAR comment, PFVOID val1, PFVOID val2)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s %p %p", comment, val1, val2);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_str(xd_diag_section_t section, int level, PFCCHAR comment, PFCHAR str1)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s %.60s", comment, (str1 ? str1 : "OOPS - str1 is 0"));

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_str2(xd_diag_section_t section, int level, PFCCHAR comment, PFCHAR str1, PFCHAR str2)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s %s %s", comment, (str1 ? str1 : "OOPS - str1 is 0"), (str2 ? str2 : "OOPS - str2 is 0"));

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}


/* ********************************************************************   */
/* PORTS LISTS                                                            */
/* ********************************************************************   */
void diag_say_ports_tcp(xd_diag_section_t section, int level, PFCCHAR comment)
{
#if (INCLUDE_TCP)
    PTCPPORT port;
#endif
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }

#if (INCLUDE_TCP)
    port = (PTCPPORT)root_tcp_lists[MASTER_LIST];
    while (port)
    {
        tc_zprintf(&io, "say_ports - master - state = %d, index = %d\n"
                        "          - master - tcp_port_type = %d, port_flags = %08lX\n" 
                  ,(int)port->state, (int)port->ap.ctrl.index
                  ,(int)port->tcp_port_type, (unsigned long)port->ap.port_flags
                  );
        if (port->ap.port_flags & PORT_BOUND) 
        {
             tc_zprintf(&io, "          - master - bound to %pA\n"
                       ,(byte KS_FAR *)port->out_ip_temp.ip_src
                       );
        }
        tc_zprintf(&io, "          - master - out.port = %d, in.port = %d\n"
                  ,(int)port->out.port, (int)port->in.port
                  );
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[MASTER_LIST], 
                                            (POS_LIST)port, ZERO_OFFSET);
    }

    port = (PTCPPORT)root_tcp_lists[LISTEN_LIST];
    while (port)
    {
        tc_zprintf(&io, "say_ports - listen - state = %d,index = %d\n"
                  ,(int)port->state, (int)port->ap.ctrl.index
                  );
        if (port->ap.port_flags & PORT_BOUND) 
        {
             tc_zprintf(&io, "          - listen - bound to %pA\n"
                       ,(byte KS_FAR *)port->out_ip_temp.ip_src
                       );
        }
        tc_zprintf(&io, "          - listen - out.port = %d, in.port = %d\n" 
                        "          - listen - tcp_port_type = %d, port_flags = %08lX\n"
                        "          - listen - src IP addr = %pA\n"
                  ,(int)port->out.port, (int)port->in.port
                  ,(int)port->tcp_port_type, (unsigned long int)port->ap.port_flags
                  ,(byte KS_FAR *)port->out_ip_temp.ip_src
                  );
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[LISTEN_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);
    }

    port = (PTCPPORT)root_tcp_lists[SOCKET_LIST];
    while (port)
    {
        tc_zprintf(&io, "say_ports - socket - state = %d,index = %d\n" 
                        "          - socket - tcp_port_type = %d, port_flags = %08lX\n"
                  ,(int)port->state, (int)port->ap.ctrl.index
                  ,(int)port->tcp_port_type, (unsigned long int)port->ap.port_flags
                  );
        if (port->ap.port_flags & PORT_BOUND) 
        {
             tc_zprintf(&io, "          - socket - bound to %pA\n"
                       ,(byte KS_FAR *)port->out_ip_temp.ip_src
                       );
        }
        tc_zprintf(&io, "          - socket - out.port = %d, in.port = %d\n"
                  ,(int)port->out.port, (int)port->in.port
                  );
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[SOCKET_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);
    }

    port = (PTCPPORT)root_tcp_lists[ACTIVE_LIST];
    while (port)
    {
        tc_zprintf(&io, "say_ports - active - state = %d,index = %d\n"
                        "          - active - out.port = %d, in.port = %d\n" 
                        "          - active - src IP addr = %pA\n"
                        "          - active - dest IP addr = %pA\n"
                        "          - active - port_flags (0x100=API_CLOSE_DONE) = %#08lX, options(0x200=REUSE) = %#08lX\n"
                        "          - active - in.contain = %d, out.contain = %d\n"
                  ,(int)port->state, (int)port->ap.ctrl.index
                  ,(int)port->out.port, (int)port->in.port
                  ,(byte KS_FAR *)port->out_ip_temp.ip_src
                  ,(byte KS_FAR *)port->out_ip_temp.ip_dest
                  ,(unsigned long int)port->ap.port_flags, (unsigned long int)port->ap.options
                  ,(int)port->in.contain, (int)port->out.contain
                  );
        port = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                                (POS_LIST)port, ZERO_OFFSET);
    }

#endif      /* end of if INCLUDE_TCP */

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ports_udp(xd_diag_section_t section, int level, PFCCHAR comment)
{
#if (INCLUDE_UDP)
PUDPPORT udp_port;
PUDPPKT  udp_pkt;    /* Default UDP packet info in our port structure */
PIPPKT   ip_pkt;     /* Default IP packet info in our port structure */
#endif
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }
#if (INCLUDE_UDP)
    udp_port = (PUDPPORT)root_udp_lists[ACTIVE_LIST];
    while (udp_port)
    {
        udp_pkt = (PUDPPKT) &(udp_port->udp_connection);
        ip_pkt  = (PIPPKT) &(udp_port->ip_connection);

        tc_zprintf(&io, "diag_say_ports - active UDP port index = %d\n"
                                        "               - active UDP - dest port = %d, src port = %d\n"
                                        "               - active UDP - src IP addr = %pA\n"
                                        "               - active UDP - dest IP addr = %pA\n"
                            ,(int)udp_port->ap.ctrl.index
                            ,(int)udp_pkt->udp_dest, (int)udp_pkt->udp_source
                            ,ip_pkt->ip_src
                            ,ip_pkt->ip_dest
                            );
        udp_port = (PUDPPORT)os_list_next_entry_off(root_udp_lists[ACTIVE_LIST],
                                                    (POS_LIST)udp_port, ZERO_OFFSET);
    }

    udp_port = (PUDPPORT)root_udp_lists[SOCKET_LIST];
    while (udp_port)
    {
        udp_pkt = (PUDPPKT) &(udp_port->udp_connection);
        ip_pkt  = (PIPPKT) &(udp_port->ip_connection);

        tc_zprintf(&io, "diag_say_ports - socket UDP port index = %d\n"
                                        "          - socket UDP - dest port = %d, src port = %d\n"
                                        "          - socket UDP - src IP addr = %pA\n"
                                        "          - socket UDP - dest IP addr = %pA\n"
                            ,(int)udp_port->ap.ctrl.index
                            ,(int)udp_pkt->udp_dest, (int)udp_pkt->udp_source
                            ,ip_pkt->ip_src
                            ,ip_pkt->ip_dest
                            );
        udp_port = (PUDPPORT)os_list_next_entry_off(root_udp_lists[SOCKET_LIST],
                                                    (POS_LIST)udp_port, ZERO_OFFSET);
    }
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ports_raw(xd_diag_section_t section, int level, PFCCHAR comment)
{
#if (INCLUDE_RAW)
PUDPPORT raw_port;
PUDPPKT  raw_pkt;    /* Default UDP packet info in our port structure */
PIPPKT   ip_pkt;     /* Default UDP packet info in our port structure */
#endif
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }
#if (INCLUDE_RAW)
    raw_port = (PUDPPORT)root_raw_lists[ACTIVE_LIST];
    while (raw_port)
    {
        raw_pkt = (PUDPPKT) &(raw_port->udp_connection);
        ip_pkt  = (PIPPKT) &(raw_port->ip_connection);

        tc_zprintf(&io, "diag_say_ports - active UDP port index = %d\n"
                                        "               - active UDP - dest port = %d, src port = %d\n"
                                        "               - active UDP - src IP addr = %pA\n"
                                        "               - active UDP - dest IP addr = %pA\n"
                            ,(int)raw_port->ap.ctrl.index
                            ,(int)raw_pkt->udp_dest, (int)raw_pkt->udp_source
                            ,ip_pkt->ip_src
                            ,ip_pkt->ip_dest
                            );
        raw_port = (PUDPPORT)os_list_next_entry_off(root_raw_lists[ACTIVE_LIST],
                                                    (POS_LIST)raw_port, ZERO_OFFSET);
    }

    raw_port = (PUDPPORT)root_raw_lists[SOCKET_LIST];
    while (raw_port)
    {
        raw_pkt = (PUDPPKT) &(raw_port->udp_connection);
        ip_pkt  = (PIPPKT) &(raw_port->ip_connection);

        tc_zprintf(&io, "diag_say_ports - socket UDP port index = %d\n"
                                        "               - socket UDP - dest port = %d, src port = %d\n"
                                        "               - socket UDP - src IP addr = %pA\n"
                                        "               - socket UDP - dest IP addr = %pA\n"
                            ,(int)raw_port->ap.ctrl.index
                            ,(int)raw_pkt->udp_dest, (int)raw_pkt->udp_source
                            ,ip_pkt->ip_src
                            ,ip_pkt->ip_dest
                            );
        raw_port = (PUDPPORT)os_list_next_entry_off(root_raw_lists[SOCKET_LIST],
                                                    (POS_LIST)raw_port, ZERO_OFFSET);
    }
#endif

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_ports_ping(xd_diag_section_t section, int level, PFCCHAR comment)
{
#if (INCLUDE_PING)
PANYPORT port;
PIFACE pi;
int iface_off;
#endif
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }
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
                tc_zprintf(&io, "diag_say_ports - ping - iface = %d, seq = %d\n"
                                    ,(int)iface_off, (int)port->ping_sequence
                                    );
                port = (PANYPORT)os_list_next_entry_off(pi->ctrl.root_ping_ports,
                                                        (POS_LIST)port, ZERO_OFFSET);
            }
        }
    }
#endif    /* end of if INCLUDE_PING */

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

#if (INCLUDE_ROUTING_TABLE)
/* ********************************************************************   */
/* ROUTING TABLE                                                          */
/* ********************************************************************   */
void diag_say_rt_entry(xd_diag_section_t section, int level, PFCCHAR comment, int i, PROUTE prt)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }

    if (i == -1)
    {
        tc_zprintf(&io, "rt entry\n");
    }
    else
    {
        tc_zprintf(&io, "rt entry = %d\n", i);
    }
    tc_zprintf(&io,   "         rt_dest   = %pA\n"
                                        "         rt_mask   = %pA\n"
                                        "         rt_gw     = %pA\n"
                                        "         rt_iface  = %d\n"
                                        "         rt_metric = %lu\n"
#if (INCLUDE_RT_TTL)
                                        "         rt_ttl    = %d\n"
#endif
                                        "         rt_flags  = $%X\n"
#if (INCLUDE_RT_LOCK)
                                        "         rt_usecnt = %d\n"
#endif
                        ,prt->rt_dest
                        ,prt->rt_mask
                        ,prt->rt_gw
                        ,(int)prt->rt_iface
                        ,(dword)prt->rt_metric
#if (INCLUDE_RT_TTL)
                        ,(int)prt->rt_ttl
#endif
                        ,(int)prt->rt_flags
#if (INCLUDE_RT_LOCK)
                        ,(int)prt->rt_usecnt
#endif
                        );

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

void diag_say_rt(xd_diag_section_t section, int level, PFCCHAR comment)
{
    int    i;
    PROUTE prt;
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }

    tc_zprintf(&io, "ROUTING TABLE\n");
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
            diag_say_rt_entry(section, level, "", i, prt);
        }
    }

    tc_zprintf(&io, "DEFAULT GATEWAYS\n");
    prt = (PROUTE)(root_rt_default);
    while (prt)
    {
        diag_say_rt_entry(section, level, "", -1, prt);
        prt = (PROUTE)os_list_next_entry_off(root_rt_default,
                                             (POS_LIST)prt, ROUTE_OFFSET);
    }

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}
#endif    /* INCLUDE_ROUTING_TABLE */

#if (INCLUDE_ARP)
/*****************************************************************   */
/*ARP CACHE                                                          */
/*****************************************************************   */
void diag_say_arpcache_entry(xd_diag_section_t section, int level, PFCCHAR comment, int index)
{
    PARPCACHE p;
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s\n", comment);
    }

    p = tc_arpcache;

    tc_zprintf(&io, "arpcache index = %d, state = $%X\n"
                        ,(int)index, (int)p[index].arpc_state
                        );


    switch (p[index].arpc_state)
    {
    case ARPC_STATE_FREE:
        tc_zprintf(&io, "arpcache index = %d, state = %s\n"
                  ,(int)index, "FREE"
                  );
        break;

    case ARPC_STATE_PENDING:
        tc_zprintf(&io, "arpcache index = %d, state = %s\n"
                  ,(int)index, "PENDING"
                  );
        break;

    case ARPC_STATE_RESOLVED:
        tc_zprintf(&io, "arpcache index = %d, state = %s\n"
                  ,(int)index, "RESOLVED"
                  );
        break;
    }

    if (p[index].arpc_state != ARPC_STATE_FREE)
    {
        if (p[index].pi)
        {
            tc_zprintf(&io, "interface = %d\n"
                      ,(int)p[index].pi->ctrl.index
                      );
        }
        tc_zprintf(&io, "arpcache ttl = %ld, nretries= %ld\n"
                        "arpcache IP addr = %pA"
                  ,(long int)p[index].arpc_ttl, (long int)p[index].arpc_nretries
                  ,(byte KS_FAR *)p[index].arpc_ip_addr
                  );
        if (p[index].arpc_state == ARPC_STATE_RESOLVED)
        {
            tc_zprintf(&io, "arpcache HW addr = %pM"
                      ,(byte KS_FAR *)p[index].arpc_hw_addr
                      ); /*added ethernet address  */
        }
    }

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}
#endif /* INCLUDE_ARP */





#if (INCLUDE_SNMP)
/* ********************************************************************   */
/* SNMP                                                                   */
/* ********************************************************************   */

void snmpform_val(char *buf, PSNVAL val);

/* outputs the fields of obj   */
void diag_asn1_output(xd_diag_section_t section, int level, PFCCHAR comment, struct asn1_object *obj)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%sASN.1 type = %lu, len = %lu, data:\n%*pD", (comment ? comment : ""), (dword)obj->type, (dword)obj->len, (int)obj->len, (void *)obj->value);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

/* --------------------------------------------------------------------
 * snmpprint - print out each binding in the bindings list.
 * --------------------------------------------------------------------
 */
void diag_snmpprint(xd_diag_section_t section, int level, PFCCHAR comment, PSNBENTRY bindl)
{
    /* for each element in bindl, print objid string and value.   */
    for (; bindl != (PSNBENTRY) 0; bindl = bindl->sb_next)
    {
        diag_snmpprint_one(section, level, comment, bindl);
    }
}

void diag_snmpprint_one(xd_diag_section_t section, int level, PFCCHAR comment, PSNBENTRY bindl)
{
    PMIB np;
    struct oid tmpobj;
    word *tp;
    int i;
    POID oip = &bindl->sb_oid;
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    if (comment && *comment)
    {
        tc_zprintf(&io, "%s", comment);
    }

    if ((np = getmib(oip)) == (PMIB)0)
    {
        tc_zprintf(&io, "(%pO) = ", oip);
    }
    else
    {
#if (INCLUDE_MIB_STRING)
        tc_zprintf(&io, "(%s%s", np->mi_prefix, np->mi_name);
#else
        tc_zprintf(&io, "(%pO", &np->mi_objid);
#endif
        if (np->mi_vartype == T_AGGREGATE || np->mi_vartype == T_TABLE)
        {
            tmpobj.len = oip->len - np->mi_objid.len;
            tp = tmpobj.id;
            for (i = np->mi_objid.len; i < oip->len; i++)
                    *tp++ = oip->id[i];
            tc_zprintf(&io, ".%pO) = ", &tmpobj);
        }
        else
        {
            tc_zprintf(&io, ") = ");
        }
    }

    tc_zprintf(&io, "%pV", &bindl->sb_val);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

/* --------------------------------------------------------------------
 * snmpprint_val - print out the value in the snval structure.
 * --------------------------------------------------------------------
 */
void diag_snmpprint_val(xd_diag_section_t section, int level, PFCCHAR comment, PSNVAL val)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s%pV", ( comment ? comment : ""), val);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}

/* --------------------------------------------------------------------
 * snmpprint_objid - print out the value in the objidp structure.
 * --------------------------------------------------------------------
 */
void diag_snmpprint_objid(xd_diag_section_t section, int level, PFCCHAR comment, POID objidp)
{
    struct tc_printf_io io = {0};

    io.propagate.ivalue = (int)section;

    if (LOG_PRI(level) > DEBUG_SYSLOG_LEVEL(level))
        return;

    /* don't allow users to log kernel messages   */
    if ((level & LOG_FACMASK) == LOG_KERN)
    {
        level &= ~LOG_FACMASK;
        level |= LOG_USER;
    }

    io.write = &diag_debug_printf_write;
    io.channel.syslog.facility_and_priority = level;
    io.channel.syslog.option_and_mask = ~0; /* all 1 bits! */
    io.channel.syslog.option_or_mask = 0;

    tc_zprintf(&io, "%s%pO", (comment ? comment : ""), objidp);

    /* flush line!   */
    (*io.write)(&io, NULL, 0);
}


/* --------------------------------------------------------------------
 * snmpform_objname - construct the name corresponding to an objid.
 * --------------------------------------------------------------------
 */
#endif   /* end of if INCLUDE_SNMP */



int diag_map_errlevel2syslog_level(int level)
{
    static const int trans[] =
    {
        LOG_MAKEPRI(LOG_USER, LOG_EMERG),             /* <= 0 */
        LOG_MAKEPRI(LOG_USER, LOG_ERR),               /* LEVEL_1: 1 */
        LOG_MAKEPRI(LOG_USER, LOG_WARNING),           /* LEVEL_2: 2 */
        LOG_MAKEPRI(LOG_USER, LOG_INFO),              /* LEVEL_3: 3 */
        LOG_MAKEPRI(LOG_USER, LOG_DEBUG),             /* LEVEL_4: 4 */
        LOG_MAKEPRI(LOG_USER, LOG_CRIT),              /* >= 5 */
    };

    if (level < 0) return trans[0];
    if (level >= sizeof(trans)/sizeof(trans[0])) return trans[sizeof(trans)/sizeof(trans[0]) - 1];
    return trans[level];
}

#endif  /* (CFG_USE_DIAG_MSG) */

                                                                                                    

