/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */
/*  tm_printf, tc_snprintf, tc_sprintf, tc_vsnprintf, tc_vsprintf        */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtip.h"

#if (!KS_USE_ASCII_LIB || KS_USE_TC_PRINTF)

#include "terminal.h"
#include <stdarg.h>   /* for va_arg (tc_sprintf) */

/* support dumping of SNMP OIDs and ASN.1 elements.   */
#if (INCLUDE_SNMP)
#include "snmp.h"
#endif

#if (INCLUDE_TC_PRINTF_FLOAT_SUPPORT != 0)
#include <stdlib.h>   /* vsprintf() definition */
#endif


/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#ifndef DEBUG_TC_VZPRINF
#define DEBUG_TC_VZPRINF  0
#endif

/* ********************************************************************   */
/* CONFIGURATION                                                          */
/* ********************************************************************   */
#if !defined(CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX)
#define CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX      16
#endif

#if (CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX < 2) || \
    (CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX > 16)
#define CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX      16
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */

#define vval  _val._val
#define cp    _val._str._cp
#define slen  _val._str._len
#define integ _val._integ

#define OPTSIGN     0x00
#define SPCSIGN     0x01
#define MANSIGN     0x02
#define NEGSIGN     0x03
#define FILL        0x04
#define LEFT        0x08
#undef LONG
#define LONG        0x10
#define UPCASE      0x20
#define TEN         0x00
#define EIGHT       0x40
#define SIXTEEN     0x80
#define UNSIGN      0xC0
#define BASEM       0xC0
#define EFMT       0x100
#define GFMT       0x200
#define FFMT       0x400
#define ALTERN     0x800
#define SHORT     0x1000
#define POINTER   0x2000
#define THOUSANDS 0x4000
#define PREC_SET  0x8000U

/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
KS_GLOBAL_CONSTANT unsigned long int KS_FAR dpowers[10] =
{
    1,
    10,
    100,
    1000,
    10000,
    100000l,
    1000000l,
    10000000l,
    100000000l,
    1000000000l
};

KS_GLOBAL_CONSTANT unsigned long int KS_FAR hexpowers[8] =
{
    1,
    0x10,
    0x100,
    0x1000,
    0x10000l,
    0x100000l,
    0x1000000l,
    0x10000000l
};

KS_GLOBAL_CONSTANT unsigned long int KS_FAR octpowers[11] =
{
    1,
    010,
    0100,
    01000,
    010000,
    0100000,
    01000000l,
    010000000l,
    0100000000l,
    01000000000l,
    010000000000l
/* too large  ,0100000000000ul     */
};

KS_GLOBAL_CONSTANT char KS_FAR *upper_string = "0123456789ABCDEF";
KS_GLOBAL_CONSTANT char KS_FAR *lower_string = "0123456789abcdef";

/* ********************************************************************   */
/* SUPPORT ROUTINES                                                       */
/* ********************************************************************   */
/* callback used to print formatted string on your display                */
int tc_printf_write(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen)      /* __fn__ */
{
    char str[33];
    int len = msglen;

    ARGSUSED_PVOID(io)

    while (len >= sizeof(str))
    {
        tc_movebytes(str, msg, sizeof(str)-1);
        str[sizeof(str)-1] = 0;
        tm_cputs(str);
        len -= sizeof(str)-1;
        msg += sizeof(str)-1;
    }

    if (len > 0)
    {
        tc_movebytes(str, msg, len);
        str[len] = 0;
        tm_cputs(str);
    }

    return msglen;
}

/* ********************************************************************   */
/*
 * NOTE: this software is almost completely rewritten by 
 * Ger 'Insh_Allah' Hobbelt to facilitate 'output function callback', 
 * additional format specifiers and modifiers and several other 
 * formatting characteristic changes, additions and fixes.
 *
 * This software has been based on ideas as implemented in doprnt() by 
 * HiTech software (License to distribute granted to EBS 1996) and the 
 * portable snprintf() implementation by Mark Martinec
 * (www.ijs.si/software/snprintf/)
 *
 * The %p extensions are identical to and extended upon the Apache 
 * Webserver log/printf() %p extensions (%pD, %pA, %pI)
 */

int tc_vzprintf(struct tc_printf_io *io, KS_CONSTANT char * f, va_list ap)      /* __fn__ */
{
    char  cbuf[82];
    char  tmp[2];
    int   prec;
    int   c;
    int   width;
    int   dump_width;
    unsigned  flag;
    union
    {
        unsigned long int _val;
        struct
        {
            KS_CONSTANT char *  _cp;
            unsigned  _len;
        } _str;
    } _val;
    int ret = 0;
    int wr_len = 0;
    char *next;
#define cbuf_end   (cbuf + sizeof(cbuf))
#if (INCLUDE_TC_PRINTF_FLOAT_SUPPORT != 0)
    KS_CONSTANT char *percent_location;
    va_list ap_copy;
#endif
    KS_CONSTANT char KS_FAR *mapping = NULL;

    while (*f)
    {
        c = *f++;
        if (c != '%')
        {
            KS_CONSTANT char * f_end;

            /* fast: scan till f_end '%': write string part   */
            for (f_end = f--; *f_end; f_end++)
            {
                if (*f_end == '%') break;
            }

            wr_len = (*io->write)(io, f, (int)(f_end - f));
            if (wr_len >= 0) ret += wr_len;
            f = f_end;
            continue;
        }
        width = 0;
        dump_width = 0;
        flag = 0;
#if (INCLUDE_TC_PRINTF_FLOAT_SUPPORT != 0)
        percent_location = f - 1;  /* remember position of the '%' */
        ap_copy = ap;
#endif
        for(;;)
        {
            switch(*f)
            {
            case '-':
                flag |= LEFT;
                f++;
                continue;

            case ' ':
                flag |= SPCSIGN;
                f++;
                continue;

            case '+':
                flag |= MANSIGN;
                f++;
                continue;

            case '#':
                flag |= ALTERN;
                f++;
                continue;

            case '0':
                flag |= FILL;
                f++;
                continue;

            case '\'':
                flag |= THOUSANDS;
                f++;
                continue;

            default:
                break;
            }
            break;
        }

        if (tc_isdigit(*f))
        {
            width = 0;
            do
                width = width * 10 + *f++ - '0';
            while(tc_isdigit(*f));
            dump_width = width;
        }
        else if (*f == '*')
        {
            width = va_arg(ap, int);
            dump_width = width;
            if (width < 0)
            {
                width = -width;
                flag |= LEFT;
            }
            f++;
        }

        prec = 0;
        if (*f == '.')
        {
            flag |= PREC_SET;  /* 'precision == 0' is quite different from 'unspecified precision' */
            if (*++f == '*')
            {
                prec = va_arg(ap, int);
                if (prec < 0)
                {
                    prec = 0;
                    flag &= ~PREC_SET;
                }
                f++;
            }
            else
            {
                while(tc_isdigit(*f))
                    prec = prec*10 + *f++ - '0';
            }
        }
        if (flag & MANSIGN)
            flag &= ~SPCSIGN;
        if (flag & SPCSIGN)
            flag &= ~MANSIGN;
        if (flag & LEFT)
            flag &= ~FILL;

        /* modifier first!   */
        switch (c = *f++)
        {
        case 0:
            f--;
            continue; /* !!! nasty, but no 'goto' no more. All optimizing compilers will work better now :-) !!! */

        case 'h':
            flag |= SHORT;
            c = *f++; /* SHORT */
            break;

        case 'l':
            flag |= LONG;
            c = *f++;
            /* don't support 'll' long-long   */
            break;

        case 'O':
            flag |= LONG;
            c = 'o';
            break;

        case 'D':
            flag |= LONG;
            c = 'd';
            break;

        case 'U':
            flag |= LONG;
            c = 'u';
            break;
        
        case 'F':
            /* FAR pointer. We don't particularly care or support this.   */
            c = *f++;
            break;
            
        case 'N':
            /* NEAR pointer. We don't particularly care or support this.   */
            c = *f++;
            break;
            
        default:
            break;
        }

        /* type! 'hack' for i/o/u/x radixes&signs   */
        switch (c)
        {
        case 'u':
        case 'o':
        case 'X':
        case 'x':
        case 'd':
        case 'i':
            switch (c)
            {
            case 'u':
                flag |= UNSIGN;
                break;

            case 'o':
                flag |= EIGHT;
                break;

            case 'X':
                flag |= UPCASE;
            case 'x':
                flag |= SIXTEEN;
                break;
            }

            if (flag & LONG)
                flag &= ~SHORT;
                
            /* format integer!   */
            if ((flag & BASEM) == TEN)
            {
#if (DEBUG_TC_VZPRINF)
                (*io->write)(io, "{TEN}", 5);
#endif                  
                if (flag & LONG)
                    vval = (unsigned long int)va_arg(ap, long int);
                else if (flag & SHORT)
                    vval = (unsigned long int)va_arg(ap, short int);
                else
                    vval = (unsigned long int)va_arg(ap, int);

                if ((long int)vval < 0)
                {
#if (DEBUG_TC_VZPRINF)
                    (*io->write)(io, "{NEG}", 5);
#endif
                    flag |= NEGSIGN;
                    vval = 0 - vval;
                }
            }
            else
            {
#if (DEBUG_TC_VZPRINF)
                (*io->write)(io, "{HEX}", 5);
#endif
                if (flag & LONG)
                    vval = (unsigned long int)va_arg(ap, unsigned long int);
                else if (flag & SHORT)
                    vval = (unsigned long int)va_arg(ap, unsigned short int);
                else
                    vval = (unsigned long int)va_arg(ap, unsigned int);
            }

            if (prec == 0 && vval == 0 && !(flag & PREC_SET))
                prec++;

            switch ((unsigned char)(flag & BASEM))
            {
            default:
            case TEN:
            case UNSIGN:
                for(c = 1 ; c != sizeof(dpowers)/sizeof(dpowers[0]); c++)
                    if (vval < dpowers[c])
                        break;
                break;

            case SIXTEEN:
                for(c = 1 ; c != sizeof(hexpowers)/sizeof(hexpowers[0]); c++)
                    if (vval < hexpowers[c])
                        break;
                break;

            case EIGHT:
                for(c = 1 ; c != sizeof(octpowers)/sizeof(octpowers[0]); c++)
                    if (vval < octpowers[c])
                        break;
                break;
            }
            if (c < prec)
                c = prec;
            else if (prec < c)
                prec = c;
            if (width && (flag & NEGSIGN))
                width--;

            if (width > prec)
                width -= prec;
            else
                width = 0;

            if (vval && (flag & (FILL|BASEM|ALTERN)) == (EIGHT|ALTERN))
            {
                if (width)
                    width--;
            }
            else if ((flag & (BASEM|ALTERN)) == (SIXTEEN|ALTERN))
            {
                if (width > 2)
                    width -= 2;
                else
                    width = 0;
            }

            next = cbuf;
            if (flag & FILL)
            {
                if (flag & MANSIGN)
                {
                    if (flag & SPCSIGN)
                        *next++ = '-';    /* NEGSIGN ==> both MANSIGN */
                                          /*             and SPCSIGN are 'on'!   */
                    else
                        *next++ = '+';
                }
                else if (flag & SPCSIGN)
                {
                    *next++ = ' ';
                }

                if (vval && (flag & (BASEM|ALTERN)) == (EIGHT|ALTERN))
                {
                    *next++ = '0';
                }
                else if ((flag & (BASEM|ALTERN)) == (SIXTEEN|ALTERN))
                {
                    *next++ = '0';
                    if (flag & UPCASE)
                        *next++ = 'X';
                    else
                        *next++ = 'x';
                }

                if (width)
                {
                    do
                    {
                        *next++ = '0';

                        if (next == cbuf_end)
                        {
                            wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                            if (wr_len >= 0) ret += wr_len; else return wr_len;
                            next = cbuf;
                        }
                    } while(--width);
                }
            }
            else
            {
                if (width && !(flag & LEFT))
                {
                    do
                    {
                        *next++ = ' ';

                        if (next == cbuf_end)
                        {
                            wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                            if (wr_len >= 0) ret += wr_len; else return wr_len;
                            next = cbuf;
                        }
                    } while(--width);
                }

                if (flag & MANSIGN)
                {
                    if (flag & SPCSIGN)
                        *next++ = '-';
                    else
                        *next++ = '+';
                }
                else if (flag & SPCSIGN)
                {
                    *next++ = ' ';
                }

                if (vval && (flag & (BASEM|ALTERN)) == (EIGHT|ALTERN))
                {
                    *next++ = '0';
                }
                else if ((flag & (BASEM|ALTERN)) == (SIXTEEN|ALTERN))
                {
                    *next++ = '0';
                    if (flag & UPCASE)
                        *next++ = 'X';
                    else
                        *next++ = 'x';
                }
            }

            while (prec > c)
            {
                *next++ = '0';
                prec--;

                if (next == cbuf_end)
                {
                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                    next = cbuf;
                }
            }

            /* check if complete number will fit in space left in buffer now   */
            if (next >= cbuf + sizeof(cbuf) - 2 - prec)
            {
                wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                if (wr_len >= 0) ret += wr_len; else return wr_len;
                next = cbuf;
            }

            if (flag & UPCASE)
                mapping = upper_string;
            else
                mapping = lower_string;

            while (prec-- > 0)
            {
                switch ((unsigned char)(flag & BASEM))
                {
                default:
                case TEN:
                case UNSIGN:
                    c = (int)((vval / dpowers[prec]) % 10 + '0');
                    break;

                case SIXTEEN:
                    c = mapping[(int)((vval / hexpowers[prec]) & 0xF)];
                    break;

                case EIGHT:
                    c = (int)(((vval / octpowers[prec]) & 07) + '0');
                    break;
                }
                *next++ = (char)c;
            }

            if ((flag & LEFT) && width)
            {
                do
                {
                    *next++ = ' ';

                    if (next == cbuf_end)
                    {
                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                        next = cbuf;
                    }
                } while(--width);
            }

            if (next > cbuf)
            {
                wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                if (wr_len >= 0) ret += wr_len; else return wr_len;
            }
            continue;

        case 'm':
            /* enter string describing the last error. No argument.   */
            {
#if (INCLUDE_ERRNO_STR)
                KS_CONSTANT char *errmsgconst = xn_geterror_string(xn_getlasterror());
                char errmsg[80];

                if (!errmsgconst || !*errmsgconst)
                {
                    tc_strcpy(errmsg, "???");
                }                           
                else
                {
                    tc_strncpy(errmsg, errmsgconst, sizeof(errmsg));
                    errmsg[sizeof(errmsg)-1] = 0;
                }
#else
                char errmsg[6 + 13];
                
                tc_strcpy(errmsg, "ErrNo");
                tc_itoa(xn_getlasterror(), errmsg + tc_strlen(errmsg), 10);
#endif

                wr_len = (*io->write)(io, errmsg, (int)tc_strlen(errmsg));
                if (wr_len >= 0) ret += wr_len; else return wr_len;
            }
            continue;
        
        case 'n':
            /* %n: write back the total number of characters written so far   */
            {
                int *intp = va_arg(ap, int *);
                
                if (intp)
                    *intp = ret;
            }
            continue;   
            
        case 'p':
            /* format: 0xPPPP[:]PPPP --- or should we perform an 'extended feature' a la Apache ap_snprintf()...   */
            /*
             * %pA  takes a struct in_addr *, and prints it as a.b.c.d
             * %pI  takes a struct sockaddr_in * and prints it as a.b.c.d:port
             * %p   takes a void * and outputs it in hex
             * %pD  takes a void * and creates a nice 'hexdump' of the region specified (through the '[dump_]width' specifier!)
             * %pM  takes a void * and dumps a 6-byte MAC address
             * %pX  takes a void * and hexdumps on a single line
             * %pO  takes a POID and outputs a Object ID as a.b.c.d.e.f.g.h.i.j (variable length, decimal numbers)
             * %pV  takes a PSNVAL and outputs a decoded ASN.1 type & value as (<type>) <value>
             *
             * The %p hacks are to force gcc's printf warning code to skip
             * over a pointer argument without complaining.  This does
             * mean that the ANSI-style %p (output a void * in hex format) won't
             * work as expected at all, but that seems to be a fair trade-off
             * for the increased robustness of having printf-warnings work.
             */
            {
                switch (*f)
                {
                default:
                    /* format: 0xPPPP[:]PPPP   */
                    {
                        PFVOID val = va_arg(ap, PFVOID);
                        unsigned char *ptr = (unsigned char *)&val;
                        int i;

                        next = cbuf;

                        if ((flag & ALTERN) == ALTERN)
                        {
                            *next++ = '0';
                            if (flag & UPCASE)
                            {
                                *next++ = 'X';
                            }
                            else
                            {
                                *next++ = 'x';
                            }
                        }
                        *next++ = '0';
                        if (flag & UPCASE)
                        {
                            mapping = upper_string;
                        }
                        else
                        {
                            mapping = upper_string;  /* lower_string */
                        }
#if (KS_LITTLE_ENDIAN)
                        for(i = sizeof(PFVOID) - 1; i >= 0; i--)
#else
                        for(i = 0; i < sizeof(PFVOID); i++)
#endif
                        {
#if (defined(__BORLANDC__) )
                            if ((i == 4) && (sizeof(PFVOID) > 4))
                            {
                                *next++ = ':';
                            }
#endif
                            *next++ = mapping[ptr[i] >> 4];
                            *next++ = mapping[ptr[i] & 0x0f];
                        }

                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                    }
                    continue;

                case 'A':
                    flag |= UPCASE;
                case 'a':
                    /* format: nnn.nnn.nnn.nnn   */
                    {
                        struct in_addr KS_FAR *ptr = va_arg(ap, struct in_addr KS_FAR *);

                        f++; /* skip the additional 'A' */
                        if (!ptr || !ip_bin_to_str(cbuf, &ptr->s_un.s_un_b.s_b1))
                        {
                            /* decoding failed!   */
                            tc_strcpy(cbuf, "???.???.???.???");
                        }

                        wr_len = (*io->write)(io, cbuf, tc_strlen(cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                    }
                    continue;

                case 'I':
                    flag |= UPCASE;
                case 'i':
                    /* format: nnn.nnn.nnn.nnn:nnn   */
                    {
                        struct sockaddr_in KS_FAR *ptr = va_arg(ap, struct sockaddr_in KS_FAR *);

                        f++; /* skip the additional 'I' */
                        if (!ptr || !ip_bin_to_str(cbuf, (PFCBYTE)&ptr->sin_addr))
                        {
                            /* decoding failed!   */
                            tc_strcpy(cbuf, "???.???.???.???:????");
                        }
                        else
                        {
                            next = cbuf + tc_strlen(cbuf);
                            *next++ = ':';
                            tc_itoa(net2hs(ptr->sin_port), next, 10);
                        }

                        wr_len = (*io->write)(io, cbuf, tc_strlen(cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                    }
                    continue;

                case 'M':
                    flag |= UPCASE;
                case 'm':
                    /* MAC address format: nnn.nnn.nnn.nnn.nnn.nnn   */
                    {
                        byte KS_FAR *n = va_arg(ap, byte KS_FAR *);

                        f++; /* skip the additional 'M' */
                        next = cbuf;
                        if (!n)
                        {
                            /* decoding failed!   */
                            tc_strcpy(next, "??.??.??.??.??.??");
                        }
                        else
                        {
#if (CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX != 16)
                            tc_itoa(n[0], next, CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX);
                            next += tc_strlen(next);
                            *next++ = '.';
                            tc_itoa(n[1], next, CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX);
                            next += tc_strlen(next);
                            *next++ = '.';
                            tc_itoa(n[2], next, CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX);
                            next += tc_strlen(next);
                            *next++ = '.';
                            tc_itoa(n[3], next, CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX);
                            next += tc_strlen(next);
                            *next++ = '.';
                            tc_itoa(n[4], next, CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX);
                            next += tc_strlen(next);
                            *next++ = '.';
                            tc_itoa(n[5], next, CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX);
#else      /* CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX */
                            int i;
                            
                            if (flag & UPCASE)
                                mapping = upper_string;
                            else
                                mapping = lower_string;

                            for(i = 0; i < 5; i++)
                            {
                                *next++ = mapping[n[i] >> 4];
                                *next++ = mapping[n[i] & 0x0f];
                                *next++ = '.';
                            }
                            *next++ = mapping[n[i] >> 4];
                            *next++ = mapping[n[i] & 0x0f];
                            *next = 0;
#endif       /* CFG_TC_PRINTF_DUMP_MAC_ADDRESS_RADIX */
                        }
                        next += tc_strlen(next);

                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                    }
                    continue;

                case 'O':
                    flag |= UPCASE;
                case 'o':
#if (INCLUDE_SNMP)
                    /* format: xxx.xxx.xxx.xxx (OID)   */
                    {
                        POID objidp = va_arg(ap, POID);
                        int i;

                        f++; /* skip the additional 'O' */
                        next = cbuf;

                        if (!objidp)
                        {
                            /* decoding failed!   */
                            tc_strcpy(next, "???.OID.???");
                            next += tc_strlen(next);
                        }
                        else
                        {
                            for (i = 0; i < objidp->len; i++)
                            {
                                tc_itoa(objidp->id[i], next, 10);
                                next += tc_strlen(next);
                                if (i != (objidp->len - 1))
                                    *next++ = '.';

                                if (next >= cbuf_end - 13 /* maxlen("xxx.") */)
                                {
                                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                                    next = cbuf;
                                }
                            }
                        }

                        if (next > cbuf)
                        {
                            wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                            if (wr_len >= 0) ret += wr_len;
                        }
                    }
#else
                    f++; /* skip the additional 'O' */
                    next = cbuf;
                    tc_strcpy(next, "[OID not supported]");

                    wr_len = (*io->write)(io, cbuf, (int)tc_strlen(cbuf));
                    if (wr_len >= 0) ret += wr_len; else return wr_len;
#endif
                    continue;

                case 'V':
                    flag |= UPCASE;
                case 'v':
                    /* ASN value (PSNVAL element)   */
#if (INCLUDE_SNMP)
                    {
                        PSNVAL val = va_arg(ap, PSNVAL);
                        int i;

                        f++; /* skip the additional 'V' */
                        next = cbuf;

                        if (flag & UPCASE)
                            mapping = upper_string;
                        else
                            mapping = lower_string;

                        if (!val)
                        {
                            /* decoding failed!   */
                            tc_strcpy(next, "???.OID+VALUE.???");
                        }
                        else
                        {
                            switch(val->sv_type)
                            {
                            case EBS_ASN1_INT :
                                tc_strcpy(next, "(Integer) ");
                                break;

                            case EBS_ASN1_COUNTER :
                                tc_strcpy(next, "(Counter) ");
                                break;

                            case EBS_ASN1_GAUGE :
                                tc_strcpy(next, "(Gauge) ");
                                break;

                            case EBS_ASN1_TIMETICKS :
                                tc_strcpy(next, "(Timeticks) ");
                                break;

#if (EBS_ASN1_INT32 != EBS_ASN1_INT)
                            case EBS_ASN1_INT32:
                                tc_strcpy(next, " (Integer32) ");
                                break;
#endif

#if (EBS_ASN1_COUNTER32 != EBS_ASN1_COUNTER)
                            case EBS_ASN1_COUNTER32:
                                tc_strcpy(next, " (Counter32) ");
                                break;
#endif

#if (EBS_ASN1_GAUGE32 != EBS_ASN1_GAUGE)
                            case EBS_ASN1_GAUGE32:
                                tc_strcpy(next, " (Gauge32) ");
                                break;
#endif

#if (INCLUDE_SNMPV2)
                            case EBS_ASN1_UINT32:
                                tc_strcpy(next, " (UInteger32) ");
                                break;
#endif

#if (INCLUDE_SNMPV2) && (EBS_ASN1_UNSIGNED32 != EBS_ASN1_GAUGE32)
                            case EBS_ASN1_UNSIGNED32:
                                tc_strcpy(next, " (Unsigned32) ");
                                break;
#endif

#if (INCLUDE_SNMPV2) && (EBS_ASN1_COUNTER64 != EBS_ASN1_COUNTER)
                            case EBS_ASN1_COUNTER64:
                                tc_strcpy(next, " (Counter64) ");
                                break;
#endif

                            case EBS_ASN1_OCTSTR :
                                tc_strcpy(next, "(Octet Str) ");
                                break;

                            case EBS_ASN1_NULL :
                                *next = 0;
                                break;

                            case EBS_ASN1_OBJID :
                                tc_strcpy(next, "(Object ID) ");
                                break;

                            case EBS_ASN1_IPADDR :
                                tc_strcpy(next, "(IP Address) ");
                                break;

                            default:
                                tc_strcpy(next, "(???) ");
                                break;
                            }

                            next += tc_strlen(next);
                            if (next > cbuf)
                            {
                                wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                if (wr_len >= 0) ret += wr_len; else return wr_len;
                                next = cbuf;
                            }

                            /* make sure next == cbuf here: see below for 'ASSUME ...'   */
                            switch(val->sv_type)
                            {
                            case EBS_ASN1_INT:
                            case EBS_ASN1_COUNTER:
                            case EBS_ASN1_GAUGE:
                            case EBS_ASN1_TIMETICKS:
#if (EBS_ASN1_INT32 != EBS_ASN1_INT)
                            case EBS_ASN1_INT32:
#endif
#if (EBS_ASN1_COUNTER32 != EBS_ASN1_COUNTER)
                            case EBS_ASN1_COUNTER32:
#endif
#if (EBS_ASN1_GAUGE32 != EBS_ASN1_GAUGE)
                            case EBS_ASN1_GAUGE32:
#endif
#if (INCLUDE_SNMPV2)
                            case EBS_ASN1_UINT32:
#endif
#if (INCLUDE_SNMPV2) && (EBS_ASN1_UNSIGNED32 != EBS_ASN1_GAUGE32)
                            case EBS_ASN1_UNSIGNED32:
#endif
#if (INCLUDE_SNMPV2) && (EBS_ASN1_COUNTER64 != EBS_ASN1_COUNTER)
                            case EBS_ASN1_COUNTER64:
                                /* not supported yet   */
#endif
                                tc_ltoa(val->sv_val.sv_int, next, 10);
                                break;

                            case EBS_ASN1_OCTSTR:
                                for (i = 0; i < val->sv_val.sv_str.sv_len; i++)
                                {
                                    /* need to implement our own isprint   */
                                    if (!tc_isprint(val->sv_val.sv_str.sv_str[i]))
                                        break;
                                }
                                /* if something wasn't printable   */
                                if (i < val->sv_val.sv_str.sv_len)
                                {
                                    for (i = 0; i < val->sv_val.sv_str.sv_len; i++)
                                    {
                                        *next++ = mapping[(val->sv_val.sv_str.sv_str[i] >> 4) & 0x0F];
                                        *next++ = mapping[val->sv_val.sv_str.sv_str[i] & 0x0F];
                                        if (i != val->sv_val.sv_str.sv_len - 1)
                                        {
                                            *next++ = '-';
                                        }

                                        if (next >= cbuf_end - 4)
                                        {
                                            wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                            if (wr_len >= 0) ret += wr_len; else return wr_len;
                                            next = cbuf;
                                        }
                                    }
                                }
                                else
                                {
                                    /* ASSUME: next == cbuf! Nothing to write before dumping sv_str!   */
                                    if (val->sv_val.sv_str.sv_len > 0)
                                    {
                                        wr_len = (*io->write)(io, val->sv_val.sv_str.sv_str, (int)val->sv_val.sv_str.sv_len);
                                        if (wr_len >= 0) ret += wr_len; else return wr_len;
/*                                      next = cbuf;   */
                                    }
                                }
                                *next = 0;
                                break;

                            case EBS_ASN1_NULL :
                                tc_strcpy(next, "Null");
                                break;

                            case EBS_ASN1_OBJID :
                                for (i = 0; i < val->sv_val.sv_oid.len; i++)
                                {
                                    tc_itoa(val->sv_val.sv_oid.id[i], next, 10);
                                    next += tc_strlen(next);
                                    if (i != (val->sv_val.sv_oid.len - 1))
                                        *next++ = '.';

                                    if (next >= cbuf_end - 13 /* maxlen("xxx.") */)
                                    {
                                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                                        next = cbuf;
                                    }
                                }
                                *next = 0;
                                break;

                            case EBS_ASN1_IPADDR :
                                for (i = 0; i < IP_ALEN; i++)
                                {
                                    tc_itoa((byte) val->sv_val.sv_ipaddr[i], next, 10);
                                    next += tc_strlen(next);
                                    if (i != IP_ALEN - 1)
                                        *next++ = '.';

                                    if (next >= cbuf_end - 13 /* maxlen("xxx.") */)
                                    {
                                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                                        next = cbuf;
                                    }
                                }
                                *next = 0;
                                break;

                            default:
                                *next = 0;
                                break;
                            }
                        }

                        next += tc_strlen(next);
                        if (next > cbuf)
                        {
                            wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                            if (wr_len >= 0) ret += wr_len; else return wr_len;
                        }
                    }
#else
                    f++; /* skip the additional 'V' */
                    next = cbuf;
                    tc_strcpy(next, "[ASN.VALUE not supported]");

                    wr_len = (*io->write)(io, cbuf, (int)tc_strlen(cbuf));
                    if (wr_len >= 0) ret += wr_len; else return wr_len;
#endif
                    continue;

                case 'D':
                    flag |= UPCASE;
                case 'd':
                    /* format: offset | xx xx xx xx ... | xxxxxxxxx |\n   */
                    {
                        int i;
                        int j;
                        unsigned char *ptr = (unsigned char *)va_arg(ap, PFVOID);

                        f++; /* skip the additional 'D' */
                        next = cbuf;
                        if (prec <= 1) prec = 16;             /* number of hex bytes per line */

                        if (flag & UPCASE)
                            mapping = upper_string;
                        else
                            mapping = lower_string;

                        if (dump_width <= 0 || !ptr)
                        {
                            /* no data to dump: format now: '0 | ??? ptr=0x****:**** |'   */
                            tc_strcpy(next, "    0 | ??? PTR=");
                            next += tc_strlen(next);

                            /* dump pointer itself instead: user can then check for NULL pointers easily :-)   */
                            {
                                unsigned char *ptrref = (unsigned char *)&ptr;

                                if ((flag & ALTERN) == ALTERN)
                                {
                                    *next++ = '0';
                                    if (flag & UPCASE)
                                        *next++ = 'X';
                                    else
                                        *next++ = 'x';
                                }

#if (KS_LITTLE_ENDIAN)
                                for(i = sizeof(PFVOID) - 1; i >= 0; i--)
#else
                                for(i = 0; i < sizeof(PFVOID); i++)
#endif
                                {
#if (defined(__BORLANDC__) )
                                    if (i == 4)
                                    {
                                        *next++ = ':';
                                    }
#endif
                                    *next++ = mapping[ptrref[i] >> 4];
                                    *next++ = mapping[ptrref[i] & 0x0f];
                                }
                            }

                            *next++ = ' ';
                        }
                        else
                        {
                            /* else: for (...)   */
                            for (j = i = 0; i < dump_width; i++)
                            {
                                if ((i % prec) == 0)
                                {
                                    /* dump line; create new line!   */
                                    if (i != 0)
                                    {
                                        /* terminate line!   */
                                        *next++ = '|';
                                        *next++ = ' ';

                                        /* dump ASCII representation next to HEX bytes!   */
                                        for ( ; j < i; j++)
                                        {
                                            if (tc_isprint(ptr[j]))
                                            {
                                                *next++ = ptr[j];
                                            }
                                            else
                                            {
                                                *next++ = '.';
                                            }

                                            if (next >= cbuf_end - 4)
                                            {
                                                wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                                if (wr_len >= 0) ret += wr_len; else return wr_len;
                                                next = cbuf;
                                            }
                                        }
                                        *next++ = ' ';
                                        *next++ = '|';
                                            *next++ = '\n';
                                    }

                                    if (next > cbuf)
                                    {
                                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                                        next = cbuf;
                                    }

                                    /* dump offset first! Pad to width=5   */
                                    tc_itoa(i, cbuf + 12 /* no risk at overlapping copy! */, 10);
                                    wr_len = tc_strlen(cbuf + 12);

                                    while (wr_len < 5)
                                    {
                                        *next++ = ' ';
                                        wr_len++;
                                    }

                                    tc_strcpy(next, cbuf + 12);
                                    next = next + tc_strlen(next);
                                    *next++ = ' ';
                                    *next++ = '|';
                                    *next++ = ' ';
                                }

                                *next++ = mapping[ptr[i] >> 4];
                                *next++ = mapping[ptr[i] & 0x0F];
                                *next++ = ' ';

                                if (next >= cbuf_end - 4 /* max(2,3,4) */ )
                                {
                                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                                    next = cbuf;
                                }
                            }

                            /* pad hex part to 'prec' width   */
                            i %= prec;
                            if (!i) i = prec;
                            for (; i < prec; i++)
                            {
                                *next++ = ' ';
                                *next++ = ' ';
                                *next++ = ' ';

                                if (next >= cbuf_end - 4 /* max(2,3,4) */ )
                                {
                                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                                    next = cbuf;
                                }
                            }
                            *next++ = '|';
                            *next++ = ' ';

                            /* dump ASCII representation next to HEX bytes!   */
                            for ( ; j < dump_width; j++)
                            {
                                if (tc_isprint(ptr[j]))
                                {
                                    *next++ = ptr[j];
                                }
                                else
                                {
                                    *next++ = '.';
                                }

                                if (next >= cbuf_end - 4)
                                {
                                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                                    next = cbuf;
                                }
                            }
                            /* pad ASCII part too!   */
                            j %= prec;
                            if (!j) j = prec;
                            for ( ; j < prec; j++)
                            {
                                *next++ = ' ';

                                if (next >= cbuf_end - 4 /* max(2,3,4) */ )
                                {
                                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                                    next = cbuf;
                                }
                            }
                            *next++ = ' ';
                        }
                        /* terminate line!   */
                        *next++ = '|';
                        if (*f && (*f != '\n'))
                            *next++ = '\n';

                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                    }
                    continue;

                case 'X':
                    flag |= UPCASE;
                case 'x':
                    /* format: xxxxxxxx, where each 'xx' is a hexadecimal byte value and the stream of 'values' is determined by 'width'   */
                    {
                        int i;
                        unsigned char *ptr = (unsigned char *)va_arg(ap, PFVOID);

                        f++; /* skip the additional 'X' */

                        if (flag & UPCASE)
                            mapping = upper_string;
                        else
                            mapping = lower_string;

                        next = cbuf;

                        if (dump_width <= 0 || !ptr)
                        {
                            /* no data to dump: format now: '??? ptr=0x****:****'   */
                            tc_strcpy(next, "??? PTR=");
                            next += tc_strlen(next);

                            /* dump pointer itself instead: user can then check for NULL pointers easily :-)   */
                            {
                                unsigned char *ptrref = (unsigned char *)&ptr;

                                if ((flag & ALTERN) == ALTERN)
                                {
                                    *next++ = '0';
                                    if (flag & UPCASE)
                                        *next++ = 'X';
                                    else
                                        *next++ = 'x';
                                }
#if (KS_LITTLE_ENDIAN)
                                for(i = sizeof(PFVOID) - 1; i >= 0; i--)
#else
                                for(i = 0; i < sizeof(PFVOID); i++)
#endif
                                {
#if (defined(__BORLANDC__) )
                                    if (i == 4)
                                    {
                                        *next++ = ':';
                                    }
#endif
                                    *next++ = mapping[ptrref[i] >> 4];
                                    *next++ = mapping[ptrref[i] & 0x0f];
                                }
                            }
                        }
                        else
                        {
                            /* else: for (...)   */
                            for (i = 0; i < dump_width; i++)
                            {
                                *next++ = mapping[ptr[i] >> 4];
                                *next++ = mapping[ptr[i] & 0x0F];

                                if (next >= cbuf_end - 2)
                                {
                                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                                    next = cbuf;
                                }
                            }
                        }

                        if (next >= cbuf)
                        {
                            wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                            if (wr_len >= 0) ret += wr_len; else return wr_len;
                        }
                    }
                    continue;
                }
            }
/*          break; // [i_a] should NEVER get here!   */

        case 'c':
            vval = va_arg(ap, int);
            c = (int)(vval >> 8);
            /* support double-byte characters!   */
            if ((flag & LONG) && c && ((unsigned char)c != 0xFF))
            {
                tmp[0] = (char) c;
                tmp[1] = (char) vval;
                slen = 2;
            }
            else
            {
                tmp[0] = (char) vval;
                slen = 1;
            }
            cp = tmp;
            if (0)
            {
        case '%':
        default:     
                tmp[0] = (char) c;
                cp = tmp;
                slen = 1;
                if (0)
                {
#if (INCLUDE_TC_PRINTF_FLOAT_SUPPORT == 0)
        case 'L':
                    switch (*f)
                    {
                    case 'e':
                    case 'E':
                    case 'f':
                    case 'F':
                    case 'g':
                    case 'G':
                        f++; /* LONG DOUBLE FORMAT */
                        break;
                    default:
                        break;
                    }
                    /* fall through   */
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
                    cp = "(non-float printf)";
                    prec = slen = 18;
                    if (0)
#endif
                    {
        case 's':
                        cp = va_arg(ap, char *);
                        if (!cp)
                            cp = "(null)";
                        /* slen = tc_strlen(cp);   */
                        slen = 0;
                        while(cp[slen])
                            slen++;
                    }
                }
            }
            if ((flag & PREC_SET) && (prec < (int)slen))
                slen = prec;
            if (width > (int)slen)
                width -= slen;
            else
                width = 0;

            next = cbuf;
            if (!(flag & LEFT))
            {
                while (width--)
                {
                    *next++ = ' ';

                    if (next == cbuf_end)
                    {
                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                        next = cbuf;
                    }
                }
                if (next > cbuf)
                {
                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                }
            }

            if (slen > 0)
            {
                wr_len = (*io->write)(io, cp, slen);
                if (wr_len >= 0) ret += wr_len; else return wr_len;
            }

            next = cbuf;
            if (flag & LEFT)
            {
                while(width--)
                {
                    *next++ = ' ';

                    if (next == cbuf_end)
                    {
                        wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                        if (wr_len >= 0) ret += wr_len; else return wr_len;
                        next = cbuf;
                    }
                }

                if (next > cbuf)
                {
                    wr_len = (*io->write)(io, cbuf, (int)(next - cbuf));
                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                }
            }
            continue;

#if (INCLUDE_TC_PRINTF_FLOAT_SUPPORT != 0)
        case 'L':
            switch (*f)
            {
            case 'e':
            case 'E':
            case 'f':
            case 'F':
            case 'g':
            case 'G':
                f++; /* LONG DOUBLE FORMAT */
                break;
            default:
                break;
            }
            /* fall through   */
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
            {
                char fmtstr[sizeof("(float printf format too complex)")];
                int fmtlen;
                /* use standard RTL sprintf() to format these elements!   */

                fmtlen = (int)(f - percent_location);
                if (fmtlen >= sizeof(fmtstr)
                        /* will sprintf() output fit? rough 'conservative' estimate:   */
                        || (width + 3 >= sizeof(cbuf))
                        || (prec + 3 + 5 >= sizeof(cbuf))
                     )
                {
                    /* oops... format string doesn't fit. This is crazy. so barf!   */
                    tc_strcpy(cbuf, "(float printf format too complex)");
                }
                else
                {
                    tc_strncpy(fmtstr, percent_location, fmtlen);
                    fmtstr[fmtlen] = 0;

                    cbuf[0] = 0;      /* some vsprintf() versions don't write anything if something fails dramatically inside their bowels... */
                    vsprintf(cbuf, fmtstr, ap_copy);
                }

                if (cbuf[0] != 0)
                {
                    wr_len = (*io->write)(io, cbuf, (int)tc_strlen(cbuf));
                    if (wr_len >= 0) ret += wr_len; else return wr_len;
                }
            }
            continue;
#endif
        }
    }

    /* tmp[0] = 0;                                             */
    /* wr_len = (*io->write)(io, tmp, 1);                      */
    /* if (wr_len >= 0) ret += wr_len; else return wr_len;     */

    return ret;
}

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */

/* ********************************************************************     */
/* tm_vprintf() - Output a formatted string to a terminal using tm_cputs(). */

#undef tm_vprintf
int tm_vprintf(PFCCHAR f, va_list ap)      /* __fn__ */
{
    struct tc_printf_io io = {0};

    io.write = &tc_printf_write;

    return tc_vzprintf(&io, f, ap);
}

/* ********************************************************************   */
int tc_zprintf(struct tc_printf_io *io, KS_CONSTANT char * f, ...)   
{
    va_list ap;
    int ret;

    va_start(ap, f);
    ret = tc_vzprintf(io, f, ap);
    va_end(ap);

    return ret;
}

/* ********************************************************************   */
#undef tm_printf
int  tm_printf(PFCCHAR f, ...)      /* __fn__ */
{
    va_list ap;
    int ret;
    
    va_start(ap,f);
    ret = tm_vprintf(f, ap);
    va_end(ap);
    
    return ret;
}

/* ********************************************************************   */
#undef tc_snprintf
int tc_snprintf(PFCHAR wh, int bufflen, KS_CONSTANT char * f, ...)   
{
    va_list ap;
    int ret;

    va_start(ap, f);
    ret = tc_vsnprintf(wh, bufflen, f, ap);
    va_end(ap);

    return ret;
}

/* ********************************************************************   */
#undef tc_sprintf
int tc_sprintf(PFCHAR wh, KS_CONSTANT char * f, ...)      /* __fn__ */
{
    va_list ap;
    int ret;

    va_start(ap, f);
    ret = tc_vsnprintf(wh, 32767, f, ap);
    va_end(ap);

    return ret;
}

/* ********************************************************************   */
/* callback used to write the formatted printf string particles to the 
   destination buffer */
int tc_snprintf_write(struct tc_printf_io *io, KS_CONSTANT char * msg, int msglen)      /* __fn__ */
{
    if (io->channel.str.bufpos + msglen >= io->channel.str.buffer_size)
    {
        msglen = io->channel.str.buffer_size - io->channel.str.bufpos - 1;
    }
    if (msglen <= 0) return 0;

    tc_movebytes(io->channel.str.output_buffer + io->channel.str.bufpos, msg, msglen);
    io->channel.str.bufpos += msglen;
    io->channel.str.output_buffer[io->channel.str.bufpos] = 0;

    return msglen;
}



/* ********************************************************************   */
#undef tc_vsnprintf
int tc_vsnprintf(PFCHAR wh, int bufflen, KS_CONSTANT char * f, va_list ap) 
{
    struct tc_printf_io io = {0};

    io.write = &tc_snprintf_write;
    io.channel.str.output_buffer = wh;
    io.channel.str.buffer_size = bufflen;
    io.channel.str.bufpos = 0;

    return tc_vzprintf(&io, f, ap);
}

/* ********************************************************************   */
#undef tc_vsprintf
int tc_vsprintf(PFCHAR wh, KS_CONSTANT char * f, va_list ap)
{
    return tc_vsnprintf(wh, 32767, f, ap);
}

#endif /* !KS_USE_ASCII_LIB || KS_USE_TC_PRINTF */


