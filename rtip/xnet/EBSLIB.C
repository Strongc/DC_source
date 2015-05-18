/*                                                                      */
/* ebslib.c - Small subroutines used by other parts of the package      */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

/*  Functions in this module                                                    */
/*     tc_memset         -  memset(). args KS_FAR                               */
/*     tc_comparen       -  compare N bytes. Return true if equal.              */
/*     tc_movebytes      -  copy n bytes. Args KS_FAR                           */
/*     tc_memmove        -  copy n bytes, guaranteeing safe copy even w/overlap */

#define DIAG_SECTION_KERNEL DIAG_SECTION_OS


#include "sock.h"
#include "rtipext.h"

#if (INCLUDE_RTIP)
#include "socket.h"
#elif (INCLUDE_WINSOCK)
#include <winsock.h>
#include "rtipstub.h"
#else
#include "rtip.h"
#include "os.h"
#endif


/* ********************************************************************   */
/* Memory set,compare routines                                            */
/* ********************************************************************   */

/* ********************************************************************   */
/* memset function that takes far pointers regardless of model            */
#ifndef tc_memset
void tc_memset(PFBYTE p, byte b, int n)                      /*__fn__*/
{ while(n--) {*p++=b;} }
#endif

/* ********************************************************************   */
/* compare function that takes far pointers regardless of model           */
#ifndef tc_comparen
RTIP_BOOLEAN tc_comparen(PFBYTE a, PFBYTE b, int n)            /*__fn__*/
{
    while(n--) {if (*a++ != *b++) return(FALSE);} return(TRUE);
}
#endif

/* ********************************************************************   */
/* copy function that takes far pointers regardless of model              */
#ifndef tc_movebytes
void tc_movebytes(PFBYTE to, PFBYTE fr, int n)            /*__fn__*/
{
    while(n--) {*to++=*fr++;}
}
#endif

#ifndef tc_memmove
void tc_memmove(PFVOID to, PFCVOID fr, int n)
{
PFBYTE cfr;
PFBYTE cto;

    cfr = (PFBYTE)fr;
    cto = (PFBYTE)to;
    if (fr > to)
        tc_movebytes(cto, cfr, n);
    else
    {
    int i;
        cfr += n;
        cto += n;
        for (i=0; i<n; i++)
        {
            *(--cto) =* (--cfr);
        }
    }
}
#endif

/* ********************************************************************   */
/* STRING and CHAR UTILITIES                                              */
/* ********************************************************************   */

#ifndef tc_strlen
int tc_strlen(PFCCHAR string)   /*__fn__*/
{
int len=0;

   while (string[len] != 0)
    len++;
   return len;
}
#endif

#ifndef tc_strcpy
PFCHAR tc_strcpy(PFCHAR targ, PFCCHAR src)    /*__fn__*/
{
int loop_cnt=0;

    do
    {
        targ[loop_cnt] = src[loop_cnt];
    } while(src[loop_cnt++]);
    return targ;
}
#endif

#ifndef tc_memchr
PFVOID tc_memchr(PFCBYTE str, int chr, int n)            /*__fn__*/
{
    int i;

    for (i = 0; i < n; i++)
        if (*str++ == (byte)chr)
            return (PFVOID)(str-1);
    return 0;
}
#endif

#ifndef tc_strcat
PFCHAR tc_strcat(PFCHAR targ, PFCCHAR src)    /*__fn__*/
{
int ret_val;

    ret_val = tc_strlen(targ);
    tc_strcpy(targ + ret_val, src);
    return targ;
}
#endif

/* concatonate at most n char from src to end of targ;   */
/* targ is null terminated                               */
#ifndef tc_strncat
PFCHAR tc_strncat(PFCHAR targ, PFCCHAR src, KS_CONSTANT int n)    /*__fn__*/
{
int len_targ;

    len_targ = tc_strlen(targ);
    tc_strncpy(targ+len_targ, src, n);
    targ[len_targ+n] = '\0';
    return(targ);
}
#endif

/* copy at most n chars from src to targ    */
/* NOTE: targ might not be null-terminalted */
#ifndef tc_strncpy
int tc_strncpy(PFCHAR targ, PFCCHAR src, KS_CONSTANT int n)    /*__fn__*/
{
int loop_cnt;

    for (loop_cnt=0; loop_cnt < n; loop_cnt++)
    {
        targ[loop_cnt] = src[loop_cnt];
        if (!src[loop_cnt])
            break;
    }

    return loop_cnt;
}
#endif

/* compares 2 strings; returns 0 if they match   */
#ifndef tc_strcmp
int tc_strcmp(PFCCHAR s1, PFCCHAR s2)        /*__fn__*/
{
int index=0;

    while (s1[index] == s2[index] && s1[index] && s2[index])
    {
        index++;
    }

    if (!s1[index] && !s2[index])
        return 0;
    if (s1[index] < s2[index])
        return -1;
    else
        return 1;

}
#endif

/* compares 2 strings doing a case insensitive compare; returns 0 if they match   */
#ifndef tc_stricmp
int tc_stricmp(PFCCHAR s1, PFCCHAR s2)       /*__fn__*/
{
int index=0;
char c1, c2;

    do
    {
        c1 = s1[index];
        c2 = s2[index];

        /* convert chars to lower case if they are letters   */
        if ( (c1 >= 'A') && (c1 <= 'Z') )
            c1 = (char)('a' + (c1 - 'A'));
        if ( (c2 >= 'A') && (c2 <= 'Z') )
            c2 = (char)('a' + (c2 - 'A'));

        index++;
    } while (c1 == c2 && c1 && c2);

    if (!c1 && !c2)
        return 0;
    if (c1 < c2)
        return -1;
    else
        return 1;

}
#endif

/* compares the first n chars in 2 strings, returns 0 if they match   */
#ifndef tc_strncmp
int tc_strncmp(PFCCHAR s1, PFCCHAR s2, KS_CONSTANT int n)    /*__fn__*/
{
int index;

    for (index=0; index < n; index++)
    {
        if ( (s1[index] != s2[index]) || !s1[index] || !s2[index] )
            return(1);
    }
    return(0);
}
#endif

/* compares n chars of 2 strings doing a case insensitive compare;   */
/* returns 0 if they match                                           */
#ifndef tc_strnicmp
int tc_strnicmp(PFCCHAR s1, PFCCHAR s2, KS_CONSTANT int n)    /*__fn__*/
{
int index;
char c1, c2;

    for (index=0; index < n; index++)
    {
        c1 = s1[index];
        c2 = s2[index];

        /* convert chars to lower case if they are letters   */
        if ( (c1 >= 'A') && (c1 <= 'Z') )
            c1 = (char)('a' + (c1 - 'A'));
        if ( (c2 >= 'A') && (c2 <= 'Z') )
            c2 = (char)('a' + (c2 - 'A'));

        if ( (c1 != c2) || !c1 || !c2 )
            return(1);
    }
    return(0);
}
#endif

/* search string str for the string find_str   */
#ifndef tc_strstr
PFCHAR tc_strstr(PFCCHAR str, PFCCHAR find_str)   /*__fn__*/
{
PFCHAR ptr_str;
int find_len;

    ptr_str = (char *)0;
    find_len = tc_strlen(find_str);

    while (*str)    /* loop until end of string */
    {
        if (!tc_strncmp(str, find_str, find_len))
        {
            ptr_str = (PFCHAR)str;
            break;
        }
        str++;
    }
    return(ptr_str);
}
#endif

/* find character 'find_chr' in string 'str'                       */
/* For this function, find_chr == 0 means find the null-terminator */
#ifndef tc_strchr
PFCHAR tc_strchr(PFCCHAR str, KS_CONSTANT char find_chr)   /*__fn__*/
{
    do
    {
        if (*str == find_chr)
        {
            return((PFCHAR)str);
        }
    } while (*str++);

    return(0);
}
#endif

#ifndef tc_strrchr
/* find character 'find_chr' in string 'str' starting at the end of 'str'   */
PFCHAR tc_strrchr(PFCCHAR str, int find_chr)   /*__fn__*/
{
int i = tc_strlen(str);

    /* loop from eos to beginning of string;   */
    /* must start with \0                      */
    do
    {
        if (str[i] == find_chr)
        {
            return((PFCHAR)(str + i));
        }
    } while(i--);

    return(0);
}
#endif

#if (INCLUDE_POP3 || INCLUDE_MODEM || INCLUDE_WEB_BROWSER || INCLUDE_IMAP || INCLUDE_WEB)  // FKA has added INCLUDE_WEB
/* return first occurance of find_str in str disregarding case   */
PFCHAR tc_stristr(PFCHAR str, PFCCHAR find_str)   /*__fn__*/
{
PFCHAR ptr_str;
int find_len;

    ptr_str = (char *)0;
    find_len = tc_strlen(find_str);

    while (*str)    /* loop until end of string */
    {
        if (!tc_strnicmp(str, find_str, find_len))
        {
            ptr_str = str;
            break;
        }
        str++;
    }
    return(ptr_str);
}
#endif

/* ********************************************************************   */
#if (INCLUDE_SYSLOG || INCLUDE_CRYPT || INCLUDE_SSL || INCLUDE_SSL_MD5 || (INCLUDE_SNMP && INCLUDE_SNMPV3) || INCLUDE_SMB_SRV)
char *xn_strdup(KS_CONSTANT char *str, int ID)
{
char *p;

    if (!str)
        return((char *)0);
    p = (char *)ks_malloc(1, tc_strlen(str) + 1, ID);
    if (p)
        tc_strcpy(p, str);
    return p;
}
#endif /* INCLUDE_SYSLOG || INCLUDE_CRYPT || INCLUDE_SSL_MD5 */

#if (INCLUDE_RTIP)
/* ********************************************************************   */
/* CONVERSION ROUTINES                                                    */
/* ********************************************************************   */
#if (INCLUDE_FTP_SRV || INCLUDE_TFTP_CLI || INCLUDE_NFS_CLI || INCLUDE_NFS_SRV)
#ifndef tc_toupper
int tc_toupper(int c)
{
    if ((char)c >= 'a' && (char)c <= 'z')
        c = (int) ('A' + (char) (c - 'a'));
    return(c);
}
#endif
#endif

#if (INCLUDE_TFTP_SRV)
#ifndef tc_isupper
int tc_isupper(int c)
{
    if (c >= 'A' && c <= 'Z')
        return(1);
    return(0);
}
#endif

#ifndef tc_tolower
int tc_tolower(int c)
{
char cout;

    cout = (char)c;
    if (c >= 'A' && c <= 'Z')
        cout = ((char) ((c-'A')+'a'));
    return((int)cout);
}
#endif  /* tc_tolower */

#endif  /* TFTP SRV */
#endif  /* INCLUDE_RTIP */

#ifndef tc_atoi
int tc_atoi(PFCHAR s)
{
    return((int)tc_atol(s));
}
#endif

#ifndef tc_atol
long tc_atol(PFCHAR s)
{
long n;
RTIP_BOOLEAN neg;

    /* skip over tabs and spaces   */
    while ( (*s == ' ') || (*s == '\t') )
        s++;

    n = 0;
    neg = FALSE;

    if (*s == '-')
    {
        neg = TRUE;
        s++;
    }

    while (*s && (*s >= '0' && *s <= '9'))
    {
        n = n * 10;
        n += (long) (*s - '0');
        s++;
    }

    if (neg)
        n = 0 - n;

    return(n);
}
#endif

/* *****************************************************   */
/* Int to HEX or Decimal converter                         */
#ifndef tc_itoa
PFCHAR tc_itoa(int  num, PFCHAR dest, int  base)                       /*__fn__*/
{
    return (tc_ltoa( (long int) num, dest, base));
}
#endif

/* Long to HEX or Decimal converter     */
#ifndef tc_ltoa
PFCHAR tc_ltoa(long num, PFCHAR dest, int base)                      /*__fn__*/
{
    if (num < 0)
    {
        num = -num;
        *dest = '-';
        tc_ultoa((unsigned long) num, dest+1, base);
    }
    else
    {
        tc_ultoa((unsigned long) num, dest, base);
    }
    return dest;
}
#endif

#ifndef tc_ultoa
PFCHAR tc_ultoa(unsigned long num, PFCHAR dest, int base)      /*__fn__*/
{
char buffer[33]; /* MAXINT can have 32 digits max, base 2 */
register int digit;
PFCHAR olddest = dest;
PFCHAR p;

    p = &(buffer[32]);

    *p = '\0';

    /* Convert num to a string going from dest[31] backwards    */
    /* Nasty little ItoA algorithm                              */
    do
    {
        digit = (int) (num % base);

#if (1)
        *(--p) =
          (char)(digit<10 ? (char)(digit + '0') : (char)((digit-10) + 'a'));
#else
        *(--p) =
          (char)(digit<10 ? (char)(digit + '0') : (char)((digit-10) + 'A'));
#endif
        num /= base;
    }
    while (num);

    /* Now put the converted string at the beginning of the buffer   */
    while((*dest++=*p++)!='\0');

    return (olddest);
}
#endif

#ifndef tc_isdigit
int tc_isdigit(int ch)
{
    return ((char)ch >= '0' && (char)ch <= '9');
}
#endif

#ifndef tc_isprint
int tc_isprint(int ch)
{
    return ((char)ch >= ' ' && (char)ch < 127);
}
#endif

#ifndef tc_isspace
int tc_isspace(int ch)
{
    switch (ch)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return (!0);
    }
    return (0);
}
#endif

#ifndef tc_iscntrl
int tc_iscntrl(int ch)
{
    return (ch > 0 && ch < 32) || (ch == 127);
}
#endif



#if (INCLUDE_WEB) || (INCLUDE_SSL)

/* convert a hex string to decimal integer   */
unsigned int tc_hatoi(PFCHAR s)      /* __fn__ */
{
unsigned int n;
char c;
int  cn;

    n = 0;

    if (*s == '0' && ( (*(s+1) == 'x') ||  (*(s+1) == 'X') ) )
        s += 2;

    while (*s)
    {
        c = *s;
        if (c >= 'a' && c <= 'f')
            c = (char) ((c - 'a') + 'A');

        if (c >= 'A' && c <= 'F')
            cn = (int) ((c - 'A') + 10);
        else if (c >= '0' && c <= '9')
            cn = (int) c - '0';
        else
            break;
        n = (word)(n*0x10);
        n += (word)cn;
        s++;
    }
    return(n);
}

#endif
