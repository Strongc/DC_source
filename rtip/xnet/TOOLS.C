/*                                                                      */
/* tools.c - Small subroutines used by other parts of the package       */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

/*  Functions in this module                                             */
/*     tc_ip_chksum      -  Perform check sum on an IP packet            */
/*     tc_udp_chksum     -  Perform checksum on a UDP packet             */
/*     tc_tcp_chksum     -  Perform checksum on a TCP packet             */
/*     ipf_icmp_chksum   -  Perform checksum on a fragmented ICMP packet */
/*     net2hs            -  Convert word from net to host order          */
/*     hs2net            -  Convert word from host to net order          */
/*     net2hl            -  Convert long from net to host order          */
/*     hl2net            -  Convert long from host to net order          */

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

#if (INCLUDE_RTIP)
/* ********************************************************************   */

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DISPLAY_UDP_CHKSUM 0

#if (defined(__BORLANDC__))
/* ********************************************************************   */
#define USE_ASM_CHKSUM 1
word asm_tc_ip_chksum(PFWORD pw, word len);
#endif

#ifndef USE_ASM_CHKSUM
#define USE_ASM_CHKSUM 0
#endif

/* ********************************************************************   */
/* determine if need to use different format for assembly code            */
#define BORLAND_31 0    
#if (IX86 && USE_ASM_CHKSUM && defined(__BORLANDC__))
#if (__BORLANDC__ <= 0x0410)
#undef BORLAND_31
#define BORLAND_31 1    /* set if using Borland 3.1 compiler or earlier */
#endif
#endif  /* IX86 && USE_ASM_CHKSUM etc */

/* ********************************************************************   */
/* SOCKET TO PORT                                                         */
/* ********************************************************************   */

/* ********************************************************************   */
/* sock_to_port() - convert socket number to port structure               */
/*                                                                        */
/*   Convert a socket number (returned by socket) to a port structure.    */
/*   Used for TCP, UDP and RAW sockets.                                   */
/*                                                                        */
/*   Returns port structure                                               */
/*                                                                        */

PANYPORT sock_to_port(int socket_index)
{
    if ( (socket_index < 0) || 
         (socket_index >= (TOTAL_PORTS)) )
        return((PANYPORT)0);

    /* internal, map a socket number to a socket structure   */
    return(alloced_ports[socket_index]);
}

#if (INCLUDE_ERROR_CHECKING)
/* ********************************************************************   */
/* api_sock_to_port() - convert socket number to port structure           */
/*                                                                        */
/*   Checks if RTIP has been initialized and converts a socket            */
/*   number (returned by socket) to a port structure.                     */
/*   Used for TCP, UDP and RAW sockets.                                   */
/*   Sets errno if detects an error.                                      */
/*                                                                        */
/*   NOTE: if INCLUDE_ERROR_CHECKING is off, this is a macro defined      */
/*         in rtip.h                                                      */
/*                                                                        */
/*   Returns port structure                                               */
/*                                                                        */
PANYPORT api_sock_to_port(int socket_index)
{
int err = 0;

    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        err = ENOTINITIALIZED;

    if ( (socket_index < 0) || 
         (socket_index >= (TOTAL_PORTS)) )
        err = ENOTSOCK;

    if (err)
    {
        set_errno(err);
        return((PANYPORT)0);
    }

    /* internal, map a socket number to a socket structure   */
    return(alloced_ports[socket_index]);
}
#endif  /* INCLUDE_ERROR_CHECKING */


#if (IX86 && USE_ASM_CHKSUM) 
/* ********************************************************************   */
/* CHECKSUM routines                                                      */
/* ********************************************************************   */
/* real mode version for IX86   */
word asm_tc_ip_chksum(PFWORD pw, word len)
{
word answer;
word len_resid;

    len_resid = len & 7;    /* residual count not divisible by 8 */
    len >>= 3;              /* Count of words divided by 8 */
_asm {
    push es
    push si                    
    push bx
    push cx

    les si, pw               /* ES:SI is the source of the data */

    sub ax, ax               /* Clear checksum */

    mov cx, len              /* loop counter of 8 word chunks */
    cmp cx, 0
    jz  doresid
#if (BORLAND_31)
}                            /* needed for Borland 3.1 compiler only */
#endif
loopto:                      /* add up the check sums. add 8 words per */
                             /* loop to reduce looping   */
#if (BORLAND_31)
_asm {                   /* needed for Borland 3.1 compiler only */
#endif
    mov bx,WORD PTR es:[si]  /* get the word to add */
    add ax, bx               /* add the low word    */
    adc ax, 0                /* add in the carry bit */
    add si, 2                /* point to the next word */
    mov bx,WORD PTR es:[si]  /* checksum word 1 */
    add ax, bx
    adc ax, 0
    add si, 2
    mov bx,WORD PTR es:[si]  /* checksum word 2 */
    add ax, bx
    adc ax, 0
    add si, 2
    mov bx,WORD PTR es:[si]  /* checksum word 3 */
    add ax, bx
    adc ax, 0
    add si, 2
    mov bx,WORD PTR es:[si]  /* checksum word 4 */
    add ax, bx
    adc ax, 0
    add si, 2
    mov bx,WORD PTR es:[si]  /* checksum word 5 */
    add ax, bx
    adc ax, 0
    add si, 2
    mov bx,WORD PTR es:[si]  /* checksum word 6 */
    add ax, bx
    adc ax, 0
    add si, 2
    mov bx,WORD PTR es:[si]  /* checksum word 7  */
    add ax, bx
    adc ax, 0
    add si, 2
    /* do this word_len/8 times   */
    loop loopto
#if (BORLAND_31)
}                            /* needed for Borland 3.1 compiler only */
#endif

    /* Now checksum the rest one word at a time   */
doresid:
#if (BORLAND_31)
_asm {                   /* needed for Borland 3.1 compiler only */
#endif
    mov cx, len_resid        /* word_len % 8 */
    cmp cx, 0                /* make sure there are some to do */
    jz  done           
#if (BORLAND_31)
}                            /* needed for Borland 3.1 compiler only */
#endif

loopto_2:
#if (BORLAND_31)
_asm {                   /* needed for Borland 3.1 compiler only */
#endif
    mov bx,WORD PTR es:[si] 
    add ax, bx
    adc ax, 0
    add si, 2
    /*  do this len_resid times    */
    loop loopto_2
#if (BORLAND_31)
}                            /* needed for Borland 3.1 compiler only */
#endif
done:
#if (BORLAND_31)
_asm {                   /* needed for Borland 3.1 compiler only */
#endif
    pop cx
    pop bx
    pop si
    pop es
    mov answer, ax
    }
    return(answer);
}

#endif /* USE_ASM_CHKSUM */

/* ********************************************************************   */
/* Mostly From Comer & Stevens. Thank you                                 */
/* Ip chksum routine                                                      */
/* len: number of words                                                   */
word tc_ip_chksum(PFWORD pw, int len)          /*__fn__*/
{
#if (USE_ASM_CHKSUM)
    return (word)~asm_tc_ip_chksum(pw,(word)len);
#else
/* Slow version, should be done in assembly   */
dword sum = 0;

    while (len--)
        sum += *pw++;
    sum = (sum>>16)+(sum&0xffff);
    sum +=(sum>>16);
    return (word) (~sum & 0xffff);
#endif
}


word ipf_icmp_chksum(DCU msg, int byte_len)          /*__fn__*/
{
dword sum = 0;
PICMPPKT pic;
PIPPKT   pip;
PFWORD   pw;
int      curr_byte_len;
int      word_len;
int      curr_word_len;

    word_len = (byte_len+1)/2;

    /* NOTE: all fragments except possibly the last have an even number   */
    /*       of bytes                                                     */
    /* NOTE: ICMP checksum includes the ICMP header (in 1st frag)         */
#if (INCLUDE_FRAG)
    while (msg && word_len)
    {
#endif

        pic = DCUTOICMPPKT(msg);
        pip = DCUTOIPPKT(msg);
        pw = (PFWORD)(&(pic->icmp_type));
        curr_byte_len = net2hs(pip->ip_len) - IP_HLEN(pip);
        curr_word_len = (curr_byte_len+1)/2;

        /* if len is odd, pad with 0                                  */
        /* NOTE: only the last fragment can be odd                    */
        /* NOTE: will never overwrite data area of packet since there */
        /*       is a dword (see fence) is in the DCU after the data  */
        /*       area                                                 */
        if (curr_byte_len & 0x01)
        {
            ((PFCHAR)pw)[curr_byte_len] = 0;
        }

        while (curr_word_len-- && word_len)
        {
            sum += *pw++;
            word_len--;
        }

#if (INCLUDE_FRAG)
        msg = DCUTOPACKET(msg)->frag_next;
    }
#endif

    sum = (sum>>16) + (sum&0xffff);
    sum += (sum>>16);
    return (word) (~sum & 0xffff);
}

#if (INCLUDE_FRAG)
/* ********************************************************************   */
/* Mostly From Comer & Stevens. Thank you                                 */
/* Udp chksum routine                                                     */
word    tc_udp_chksum(DCU msg)               /*__fn__*/
{
dword sum;
word tot_len;
word len;
word word_len;
word t;
PFWORD pw;
PUDPPKT pu;
PIPPKT pip;
#if (DISPLAY_UDP_CHKSUM)
    DEBUG_ERROR("UDP CHKSUM: ", PKT, DCUTODATA(msg), DCUTOPACKET(msg)->length);
#endif

    pu  = DCUTOUDPPKT(msg);
    pip = DCUTOIPPKT(msg);

    sum = 0;

    /* Calulate pseudo header values.   */
    /* ip_src & dest                    */
    pw = (PFWORD ) pip->ip_src;
    sum += *pw++; sum += *pw++;   sum += *pw++;  sum += *pw;    
           
    /* udp length + ip protocol                                            */
    /* NOTE: NCSA puts a zero byte in the pseudo header just before the ip */
    /* protocol which is a byte. This emulates that                        */
    t = hs2net((word) pip->ip_proto);
    sum += t;
    sum += pu->udp_len; /* tbd - assumes in 1st frag */

    /* Add up the udp header and the data fields;                      */
    /* point to the beginning of the UDP packet                        */
    /* NOTE: for fragments length can only be odd on the last fragment */
    /* NOTE: starting from beginning of UDP header so will still work  */
    /*       even if UDP header is fragmented                          */
    tot_len = net2hs(pu->udp_len);
    while (msg && tot_len)
    {
        pu = DCUTOUDPPKT(msg);
        pip = DCUTOIPPKT(msg);

        /* point just after IP header; for first fragment this is      */
        /* udp source|udp dest|len|checksum|... data but for all other */
        /* fragments it is data                                        */
        pw = (PFWORD) &pu->udp_source;  
        len = (word) (net2hs(pip->ip_len) - IP_HLEN(pip));

        /* convert len to words; if len is odd pad with zero   */
        word_len = len;
        if (len & 0x01)
            ((PFCHAR ) pw)[word_len++] = 0;
        word_len >>= 1;          

        /* for faster performance add in 32 word chunks instead of 1      */
        /* word chunk in the loop                                         */
        /* NOTE: a lot of CPU time is spent processing checksums; if      */
        /*       performance is an issue, you should look at the assembly */
        /*       code generated here and possibly rewrite in assembly     */
#if (USE_ASM_CHKSUM)
        sum = sum + asm_tc_ip_chksum(pw,word_len);
/*      pw += word_len;   */
#else
        while (word_len >= 32)
        {
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            word_len -= (word) 32;
        }

        /* add the remainder (<32 words) in a loop   */
        while (word_len--)
            sum += *pw++;
#endif /* (USE_ASM_CHKSUM) */

        tot_len = (word)(tot_len - len);
        msg = DCUTOPACKET(msg)->frag_next;
    }       /* end of while loop */

    sum = (sum>>16)+(sum&0xffff);
    sum +=(sum>>16);
    return (word) (~sum & 0xffff);
}

#else
word    tc_udp_chksum(DCU msg)               /*__fn__*/
{
dword sum;
word len;
word t;
PFWORD pw;
PUDPPKT pu;
PIPPKT pip;
    pu = DCUTOUDPPKT(msg);
    pip = DCUTOIPPKT(msg);

    sum = 0;

    /* Calulate pseudo header values.   */
    /* ip_src & dest                    */
    pw = (PFWORD ) pip->ip_src;
    sum += *pw++; sum += *pw++;   sum += *pw++;  sum += *pw++;    
                /* udp length + ip protocol       */

    /* NCSA puts a zero byte in the pseudo header just before the ip   */
    /* protocol which is a byte. This emulates that                    */
    t = hs2net((word) pip->ip_proto);
    sum += t;
    sum += pu->udp_len;

    /* Add up the udp header and the data fields. If len is odd   */
    /* pad with zero                                              */
    pw = (PFWORD ) &pu->udp_source;  /* source|dest|len|checksum|... data */
    len = net2hs(pu->udp_len);

    if (len&0x01)
        ((PFCHAR ) pw)[len++] = 0;
    len >>= 1;          /* convert len to words */

#if (USE_ASM_CHKSUM)
    sum = sum + asm_tc_ip_chksum(pw,len);
#else
    /* NOTE: a lot of CPU time is spent processing checksums; if      */
    /*       performance is an issue, you should look at the assembly */
    /*       code generated here and possibly rewrite in assembly     */
    while (len >= 32)
    {
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
        len -= 32;
    }

    while (len--)
        sum += *pw++;
#endif /*  (USE_ASM_CHKSUM) */
    sum = (sum>>16)+(sum&0xffff);
    sum +=(sum>>16);
    return (word) (~sum & 0xffff);
}
#endif


#if (INCLUDE_TCP)
/* ********************************************************************   */
/* Mostly From Comer & Stevens. Thank you                                 */
/* TCP chksum routine                                                     */
/* NOTE: frag_next field in msg must be set up                            */
word tc_tcp_chksum(DCU msg)               /*__fn__*/
{
dword   sum;
word    len;
PFWORD  pw;
PTCPPKT pt;
PIPPKT  pip;
byte    save_pad;
PFBYTE  save_pw;
word    save_len;
#if (INCLUDE_FRAG)
word total_len;
#endif
    sum = 0;

    pip = DCUTOIPPKT(msg);  

    /* Get total tcp packet length excluding IP section    */
    /* i.e. TCP header and data length                     */
#if (INCLUDE_FRAG)
    /* calculate length of tcp header and tcp data   */
    if (DCUTOPACKET(msg)->frag_next)        /* if packet is fragmented */
        len = ipf_tcp_tlen(msg, -1);
    else
    {
#endif
        len = (word) (net2hs(pip->ip_len) - (word)IP_HLEN(pip));
#if (INCLUDE_FRAG)
    }
#endif

    /* **************************************************   */
    /* Calulate pseudo header values.                       */
    /* ip_src & dest                                        */
    pw = (PFWORD) pip->ip_src;

/*DEBUG_ERROR("BEFORE loop: tcp chksum len, *pw = ", DINT2, len, *pw);   */
    sum += *pw++; sum += *pw++;   sum += *pw++;  sum += *pw;    
            /* length + ip protocol       */

    sum += hs2net((word) pip->ip_proto);

    /* add in the length   */
    sum += hs2net((word)len);

    /* **************************************************   */
#if (INCLUDE_FRAG)
    total_len = (word)len;
    while (msg && total_len)
    {
        /* calculate length for fragment   */
        len = (word) (net2hs(pip->ip_len) - (word)IP_HLEN(pip));
#endif
        pt = DCUTOTCPPKT(msg);  

        /* Add up the TCP header and the data fields.    */
        pw = (PFWORD) &pt->tcp_source;  /* tcp header */
/*DEBUG_ERROR("tcp chksum (in loop) len, *pw = ", DINT2, len, *pw);   */

        /* If len is odd, pad with zero           */
        /* NOTE: done since chksum uses word adds */
        save_len = (word)len;

        /* if no copy mode, packet is part of window so must restore it   */
        /* when done                                                      */
        save_pw = ((PFBYTE)pw) + len;
        save_pad = *save_pw;
        if (len & 0x01)
        {
            ((PFCHAR)pw)[len++] = 0;
        }

        len >>= 1;          /* convert len to words */
#if (USE_ASM_CHKSUM)
        sum = sum + asm_tc_ip_chksum(pw, (word)len);
/*      pw += len;   */
#else
        /* NOTE: a lot of CPU time is spent processing checksums; if      */
        /*       performance is an issue, you should look at the assembly */
        /*       code generated here and possibly rewrite in assembly     */
        while (len >= 32)
        {
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            sum += *pw++;  sum += *pw++;  sum += *pw++;   sum += *pw++;
            len -= (word) 32;
        }
        while (len--)
            sum += *pw++;
#endif /* (USE_ASM_CHKSUM) */

        /* restore any padding which was done above   */
        if (save_len & 0x01)
            *save_pw = save_pad;

#if (INCLUDE_FRAG)
        total_len = (word)(total_len - save_len);
/*DEBUG_ERROR("total_len, save_len (BOTTOM LOOP = ", EBS_INT2, total_len, save_len);   */
        msg = DCUTOPACKET(msg)->frag_next;
        if (msg)
        {
DEBUG_ERROR("got a FRAG", NOVAR, 0, 0);
            pip = DCUTOIPPKT(msg);  
            pw = (PFWORD) pip->ip_src;  /* NO ip hdr so points to prot hdr */
        }

    }
#endif

    /* **************************************************   */
    sum =  (sum>>16)+(sum&0xffff);
    sum += (sum>>16);
    return (word) (~sum & 0xffff);
}
#endif      /* INCLUDE_TCP */

#if (KS_LITTLE_ENDIAN) 
/* ********************************************************************   */
dword net2hl(dword l)          /*__fn__*/
{
#    if (KS_LITTLE_ENDIAN)
        /* return((byte 3 to 0) OR (byte 0 to 3) OR (byte 2 to 1) OR   */
        /*        (byte 1 to 2)                                        */
        l = (l >> 24) | (l << 24) | ((l & 0x00ff0000ul) >> 8) | 
                                    ((l & 0x0000ff00ul) << 8);
#    endif
    return(l);
}

/* ********************************************************************   */
dword hl2net(dword l)          /*__fn__*/
{
#    if (KS_LITTLE_ENDIAN)
        /* return((byte 3 to 0) OR (byte 0 to 3) OR (byte 2 to 1) OR   */
        /*        (byte 1 to 2)                                        */
        l = (l >> 24) | (l << 24) | ((l & 0x00ff0000ul) >> 8) | 
                                    ((l & 0x0000ff00ul) << 8);
#    endif
    return(l);
}
#endif  /* (KS_LITTLE_ENDIAN) */

#if (!INLINE_SUPPORT && KS_LITTLE_ENDIAN) 
/* ********************************************************************   */
/* Net to intel and intel to net conversion routines                      */
/* ********************************************************************   */
/*                                                                        */
/* ********************************************************************   */
word net2hs(word w)          /*__fn__*/
{
#    if (KS_LITTLE_ENDIAN)
        w = (word)((w>>8) | (w<<8));
#    endif
    return(w);
}

/* ********************************************************************   */
word hs2net(word w)          /*__fn__*/
{
#    if (KS_LITTLE_ENDIAN)
        w = (word)((w>>8) | (w<<8));
#    endif
    return(w);
}
#endif      /* !INCLUDE_SUPPORT && KS_LITTLE_ENDIAN */

#endif /* (INCLUDE_RTIP) */

#if (INCLUDE_RTIP)

#endif      /* INCLUDE_RTIP */

/* ********************************************************************   */
/* ERRNO routines                                                         */
/* ********************************************************************   */

#if (!WEBC_WINSOCK)
/* ********************************************************************   */
/* get_system_user() - set structure with user information                */
/*                                                                        */
/*    Gets and returns the user structure for the calling task.           */
/*    If the user table is full, entry 0 is returned (tbd)                */
/*                                                                        */
/*    Returns user table entry for calling task                           */
/*                                                                        */

PSYSTEM_USER get_system_user(void)
{
    return(os_get_user());
}
#endif


/* ********************************************************************   */
/* int set_errno() - set errno for the calling task                       */
/*                                                                        */
/*    Saves errno for the calling task in array based on callers taskid.  */
/*                                                                        */
/*    Returns -1                                                          */
/*                                                                        */
int set_errno(int error)    /*__fn__*/
{
#if (!WEBC_WINSOCK)
PSYSTEM_USER user;

    user = get_system_user();
    if (user)
        user->rtip_errno = error;
#endif

    return(-1);
}
    
/* ********************************************************************   */
/* KEEP COMPILER HAPPY ROUTINES                                           */
/* ********************************************************************   */

#if (defined(__BORLANDC__))
#pragma warn -aus   /* turn off warning "identifier is assigned a value that is never used" */
#endif


#if (KS_ARGSUSED == ARGSUSED_FUNC)       /* defined in osenv.h */
/* Used to keep the compiler happy    */
void argsused_pvoid(PFVOID p)  /*__fn__*/
{
    p = p;  
}

/* Used to keep the compiler happy    */
void argsused_int(int i)       /*__fn__*/
{
    i = i;
}
#endif

#if (defined(__BORLANDC__))
#pragma warn .aus   /* turn warning back on */
#endif


/* ********************************************************************   */
/* STR to BIN etc CONVERSION ROUTINES                                     */
/* ********************************************************************   */

#if (INCLUDE_RTIP)

/* converts a 4-byte IP address to a string   */
RTIP_BOOLEAN ip_bin_to_str(PFCHAR string, PFCBYTE n)                  /*__fn__*/
{
PFCHAR p;

    p = string;

    tc_itoa(n[0], p, 10);
    tc_strcat(p, ".");
    p = string+tc_strlen(string);
    tc_itoa(n[1], p, 10);
    tc_strcat(p, ".");
    p = string+tc_strlen(string);
    tc_itoa(n[2], p, 10);
    tc_strcat(p, ".");
    p = string+tc_strlen(string);
    tc_itoa(n[3], p, 10);
    return(TRUE);
}

/* converts a IP address in string format to a four byte array;      */
/* Returns TRUE if the string is an IP address or FALSE if it is not */
RTIP_BOOLEAN ip_str_to_bin(PFBYTE address, PFCCHAR string)     /*__fn__*/
{
byte adbuf[IP_ALEN];
char strbuf[IP_ALEN];
int i;
int j;
int temp_val;

    for (i = 0; i < IP_ALEN; i++)
    {
        adbuf[i] = 0;
        strbuf[0] = '\0';
        for (j = 0; j < 3; j++)
        {
            if (*string == '\0' || *string == '.' || *string == ' ' )
            {
                strbuf[j] = '\0';
                break;
            }
            else if (*string < '0' || *string > '9')
                return(FALSE);
            strbuf[j] = *string++;
        }
        /* Make sure the buffer is terminated   */
        strbuf[3] = '\0';

        /* Check for too many digits eg: .xxxx.    */
        if (*string != '\0' && *string != '.')
            return(FALSE);

        /* Check for too few digits eg: ..    */
        if (j == 0)
            return(FALSE);

        /* check for < for digits in the address   */
        if (*string == '\0' && i != 3)
            return(FALSE);
        if (*string == '.')
            string++;

        /* Strip leading 0s   */
        for (j = 0; j < 3; j++)
        {
            if (strbuf[j] != '0')
            {
                /* convert the string to digit; store in int so can check   */
                /* range                                                    */
                temp_val = tc_atoi(&strbuf[j]);
                if (temp_val > 0xff)
                    return(FALSE);
                adbuf[i] = (unsigned char)temp_val;
                break;
            }
        }
    }
    tc_mv4(address, adbuf, IP_ALEN);
    return(TRUE);
}

#if (INCLUDE_TCP_TIMESTAMP || INCLUDE_SNMP || INCLUDE_NFS_CLI || INCLUDE_NFS_SRV || INCLUDE_RIP) 
/* ********************************************************************   */
/* DWORD ALIGNMENT ROUTINES                                               */
/* ********************************************************************   */
union lbu 
{
    byte  as_bytes[4];
    dword as_long;
};

/* return dword which is stored in memory at pd   */
/* byte array to long                             */
dword byte_to_long(PFDWORD pd)
{
union lbu l;
PFBYTE p;
    p = (PFBYTE) pd;
    l.as_bytes[0] = *(p+0);
    l.as_bytes[1] = *(p+1);
    l.as_bytes[2] = *(p+2);
    l.as_bytes[3] = *(p+3);
    return(l.as_long);
}

/* store d at pd in memory     */
/* long to byte array          */
void long_to_bytes(PFDWORD pd, dword d)
{
union lbu l;
PFBYTE p;
    p = (PFBYTE) pd;
    l.as_long = d;
    *(p+0) = l.as_bytes[0];
    *(p+1) = l.as_bytes[1];
    *(p+2) = l.as_bytes[2];
    *(p+3) = l.as_bytes[3];
}

#endif /* INCLUDE_TCP_TIMESTAMP || INCLUDE_SNMP || INCLUDE_NFS_CLI || INCLUDE_NFS_SRV || INCLUDE_RIP */
#endif /* INCLUDE_RTIP */

/* ********************************************************************   */
/* SELECT UTILITIES                                                       */
/* used by xn_line utilities, SNMP, TELNET SERVER and BOOTP               */
/* ********************************************************************   */

#if (INCLUDE_RTIP || WEBC_WINSOCK)

/* ********************************************************************   */
/* check if socket is ready for read/accept - wait                        */
/* returns the following:                                                 */
/*      FALSE if neither is ready                                         */
/*      TRUE if socket is ready                                           */
RTIP_BOOLEAN do_read_select(int socket, long wait)      /*__fn__*/
{
fd_set fd_read;
struct timeval to;
int ret_val;
#ifdef USE_NORTEL_SELECT
int nfds = socket + 1;
#endif

    FD_ZERO(&fd_read);
    FD_SET(socket, &fd_read);
    if (wait == RTIP_INF)
    {
#ifdef USE_NORTEL_SELECT
        ret_val = select(nfds, (fd_set KS_FAR *)&fd_read, (fd_set KS_FAR *)0, 
                         (fd_set KS_FAR *)0, (struct timeval KS_FAR *)0);
#else
        ret_val = select(1, (fd_set KS_FAR *)&fd_read, (fd_set KS_FAR *)0, 
                         (fd_set KS_FAR *)0, (struct timeval KS_FAR *)0);
#endif
    }
    else
    {
        to.tv_sec = wait;
        to.tv_usec = 0;
#ifdef USE_NORTEL_SELECT
        ret_val = select(nfds, (fd_set KS_FAR *)&fd_read, (fd_set KS_FAR *)0, 
                         (fd_set KS_FAR *)0, (struct timeval KS_FAR *)&to);
#else
        ret_val = select(1, (fd_set KS_FAR *)&fd_read, (fd_set KS_FAR *)0, 
                         (fd_set KS_FAR *)0, (struct timeval KS_FAR *)&to);
#endif
    }
    if (ret_val < 1)
        return(FALSE);

    if (FD_ISSET(socket, &fd_read))
        return(TRUE);

    DEBUG_ERROR("do_read_select() - oops!", NOVAR, 0, 0);   /* tbd */
    return(FALSE);
}


/* ********************************************************************   */
/* check if socket is ready for write/connect;                            */
/* returns TRUE if it is ready or FALSE if a timeout occurs               */
RTIP_BOOLEAN do_write_select(int socket_no, long wait)      /*__fn__*/
{
fd_set fd_write;
struct timeval to;

    FD_ZERO(&fd_write);
    FD_SET(socket_no, &fd_write);
    to.tv_sec = wait;
    to.tv_usec = 0;

#ifdef USE_NORTEL_SELECT
    if (select(socket_no+1, (PFDSET)0, (PFDSET)&fd_write, (PFDSET)0, 
               (PTIMEVAL)&to) < 0)
#else
    if (select(1, (PFDSET)0, (PFDSET)&fd_write, (PFDSET)0, 
               (PTIMEVAL)&to) < 0)
#endif
    {
        return(FALSE);
    }

    if (FD_ISSET(socket_no, &fd_write))
        return(TRUE);

    DEBUG_ERROR("do_write_select() - timed out", NOVAR, 0, 0);  
    return(FALSE);
}

/* ********************************************************************   */
/* set_blocking_mode - put socket in blocking mode                        */
/*                                                                        */
/* Returns TRUE if successful, FALSE if error                             */
RTIP_BOOLEAN set_blocking_mode(int sock)
{
unsigned long arg;

    /* turn blocking off so receive will return an error if request   */
    /* has not arrived; select should be used to ensure receive only  */
    /* called if request has arrived                                  */
    arg = 0;   /* disable non-blocking mode (enable blocking) */
    if (ioctlsocket(sock, FIONBIO, 
                    (unsigned long KS_FAR *)&arg) != 0) 
    {
        return(FALSE);
    }
    return(TRUE);
}

/* ********************************************************************   */
/* set_non_blocking_mode - put socket in non-blocking mode                */
/*                                                                        */
/* Returns TRUE if successful, FALSE if error                             */
RTIP_BOOLEAN set_non_blocking_mode(int sock)
{
unsigned long arg;

    /* turn blocking off so receive will return an error if request   */
    /* has not arrived; select should be used to ensure receive only  */
    /* called if request has arrived                                  */
    arg = 1;   /* enable non-blocking mode */
    if (ioctlsocket(sock, FIONBIO, 
                    (unsigned long KS_FAR *)&arg) != 0) 
    {
        return(FALSE);
    }
    return(TRUE);
}

#if (INCLUDE_SNMP)
/* calculate time system has been up; used by SNMP   */
dword calc_sys_up_time(void)
{
dword ticks = ks_get_ticks() - sys_up_time;
dword seconds = ticks / ks_ticks_p_sec();

    /* wraps after 1.26 year :-(   */
    return((seconds * 100UL) + 
           ((ticks % ks_ticks_p_sec()) * 100UL) / ks_ticks_p_sec());
}
#endif /* INCLUDE_SNMP */


#endif /* INCLUDE_RTIP || WEBC_WINSOCK */
