/*                                                                              */
/* DNS.C - domain name service                                                  */
/*                                                                              */
/*   EBS - RTIP                                                                 */
/*                                                                              */
/*   Copyright Peter Van Oudenaren , 1995                                       */
/*   All rights reserved.                                                       */
/*   This code may not be redistributed in source or linkable object form       */
/*   without the consent of its author.                                         */
/*                                                                              */
/*    Module description:                                                       */
/*        This file contains DNS code.                                          */
/*        There should be no need to change values in this file. Configuration  */
/*        values are in xnconf.h                                                */
/*                                                                              */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DNS

         
#include "sock.h"
#include "rtipext.h"

#if (INCLUDE_DNS)

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_DNS_RESULT 0

/* ********************************************************************   */
#define HOSTBYNAME  0        /* Means the request is from gethostbyname */
#define HOSTBYADDR  1        /* Means the request is from gethostbyaddr */

/* ********************************************************************   */
void init_hostentext(PFHOSTENT p);
void add_host_cache_entry(PFHOSTENT the_entry, PFHOSTENT *p_host, PFCHAR psz_name, RTIP_BOOLEAN sem_claimed);
 
/* ********************************************************************   */
/* INITIALIZATION                                                         */
/* ********************************************************************   */

/* *************************************************************************   */
/* init_hostentext() - initializes a hostentext structure                      */
/*                                                                             */
/*   Returns nothing.                                                          */

void init_hostentext(PFHOSTENT p)
{
    p->h_length = 0;
    p->h_addrtype = AF_INET;        /* assume internet addresses  */
    p->h_name = p->sz_name;
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
    p->h_addr = (PFCHAR)&(p->ip_addr.s_un.s_addr);
#else
    p->h_addr = (PFCHAR)&p->ip_addr.s_addr;
#endif
    p->h_aliases[0] = p->alias;
    p->h_ttl = 0;
    p->sz_name[0] = '\0';
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
    p->ip_addr.s_un.s_addr = 0L;
#else
    p->ip_addr.s_addr = 0L;
#endif
}

/* ********************************************************************   */
/* init_dns() -  Initialize DNS runtime environment                       */
/*                                                                        */
/*   Performs the following:                                              */
/*     Initializes host_cache, h_table, and server_ip_table to NULL.      */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void init_dns(void)
{
int loop_cnt;

    /* reset local domain name                               */
    /* tc_memset(my_domain_name, 0, sizeof(my_domain_name)); */
    my_domain_name[0] = '\0';

    cnt_dns = 0;
#if (!INCLUDE_DNS_CACHE)
    host_cache_index = -1;                   /* index to host cache */
#endif

    OS_CLAIM_TABLE(DNS_INIT_TABLE_CLAIM)

    for (loop_cnt=0; loop_cnt < CFG_MAX_HT_ENT; loop_cnt++)
        init_hostentext((PFHOSTENT)&h_table[loop_cnt]);

    for (loop_cnt=0; loop_cnt < CFG_MAX_HC_ENT; loop_cnt++)
        init_hostentext((PFHOSTENT)&host_cache[loop_cnt]);

    for (loop_cnt=0; loop_cnt < CFG_MAX_DNS_SRV; loop_cnt++)
      server_ip_table[loop_cnt] = 0L;

    OS_RELEASE_TABLE()

}

/* ********************************************************************   */
/* INTERNAL DNS ROUTINES - DNS SERVER                                     */
/* ********************************************************************   */

/* *********************************************************************   */
/* packdom() - compress string to packed domain name                       */
/*                                                                         */
/*   Compresses a text string into a packed domain name, suitable for      */
/*   the name server.  A packed domain name consists of a series           */
/*   of length, data entries followed by a 0 length, i.e.                  */
/*   the address 199.0.65.2 would be converted to:                         */
/*   03 31 39 39 01 30 02 36 35 01 32 00                                   */
/*                                                                         */
/*   Returns length of packed string                                       */
/*                                                                         */

static int packdom(PFCHAR dst, PFCHAR src)
{
PFCHAR p;
PFCHAR q,savedst;
long i;         /* wwb - change from int to long for large memory model */

    p = src;
    savedst = dst;
    do 
    {                           /* copy whole string */
        *dst = 0;
        q = dst+1;

        /*  copy the next label along, char by char until it meets a period or   */
        /*  end of string.                                                       */
        while (*p && (*p != '.')) 
            *q++ = *p++;

        /* check if length so far is to large   */
        i = p - src;        
        if (i > 0x3f)
            return(-1);

        *dst = (unsigned char)i;
        *q = 0;
        if (*p) 
        {                   /* update pointers */
            src = ++p;
            dst = q;
        }
     
    } while (*p);
    q++;
    return((int)(q-savedst));           /* length of packed string */
}

/* *********************************************************************   */
/* unpackdom() - Decompresses a packed domain name from Resource Record    */
/*                                                                         */
/*   Decompresses a packed domain name received from another host          */
/*                                                                         */
/*   Returns the number of bytes which should be skipped over at src.      */
/*   This number reflects how many unnecessary low ascii characters        */
/*   are stuck at the beginning of the string.                             */
/*   Includes NULL terminator in length count.                             */
/*                                                                         */

static int unpackdom(PFCHAR dst, PFCHAR src, PFCHAR buf)
{
int i,j;
long retval = 0;            /* need long for large memory model */
PFCHAR savesrc;

    /* Format of Resource record is:   */
    /*     NAME                        */
    /*     TYPE                        */
    /*     CLASS                       */
    /*     TTL    (32 bits)            */
    /*     RDLENGTH                    */
    /*     RDATA                       */
    savesrc = src;
    while (*src) 
    {
        j = *src;
        /* the domain name can be in 2 formats:                       */
        /* - if the 2 most sig bits are on, then the format is the    */
        /*   next 14 bits are an OFFSET from the start of the message */
        /*   which points to the name                                 */
        /* - if both bits are not set, then the formats is a sequence */
        /*   of labels, i.e. len,label,len,label,...,0 OR             */
        /*   a seqence of labes ending with a pointer                 */
        while ((j & 0xC0) == 0xC0) 
        {
            if (!retval)
                retval = src-savesrc+2;
            src++;
            src = (PFCHAR)&buf[(j & 0x3f)*256+*src];        /* pointer dereference */
            j = *src;
        }
        src++;
        for(i=0; i < (j & 0x3f) ; i++)
            *dst++ = *src++;
        *dst++ = '.';
    }
    *(--dst) = 0;           /* add terminator */
    src++;                  /* account for terminator on src */
    if (!retval)
        retval = src-savesrc;
    return((int)retval);
}


/* *********************************************************************        */
/* ddextract() - Extracts a host name or an IP address from a response message. */
/*                                                                              */
/*   Extracts host name if addr = FALSE or IP address if addr = TRUE            */
/*   from a response message.                                                   */
/*   Always extracts time-to-live from response message and put the             */
/*   response in the host cache setting p_host to the newly added               */
/*   host cache entry.                                                          */
/*   Used by gethostbyaddr() and gethostbyname().                               */
/*                                                                              */
/*   Returns error code from response message (0 on success)                    */
/*                                                                              */

int ddextract(PFUSEEK qp, PFHOSTENT *p_host, PFCHAR psz_name, 
              PFBYTE space, RTIP_BOOLEAN addr)
{
dword     ttl;
word      j,nans,rcode;
int       i;
PFRRPART  rrp;
PFBYTE    p;
DCU       msg;
PFHOSTENT p_theentry;

    /* format of response (qp) is:                                 */
    /*    HEADER (12 bytes)                                        */
    /*      ID                                                     */
    /*      FLAGS                                                  */
    /*      QDCOUNT                                                */
    /*      ANCOUNT                                                */
    /*      NSCOUNT                                                */
    /*      ARCOUNT                                                */
    /*    QUESTION (length of QNAME + 4)                           */
    /*      QNAME                                                  */
    /*      QTYPE                                                  */
    /*      QCLASS                                                 */
    /*    RESOURCE RECORDS (length of NAME + 10 + length of RDATA) */
    /*      NAME                                                   */
    /*      TYPE                                                   */
    /*      CLASS                                                  */
    /*      TTL                                                    */
    /*      RDLENGTH                                               */
    /*      RDATA                                                  */
    /*                                                             */
    nans = net2hs(qp->h.ancount);                 /* number of answers  */
    rcode = (word)(DRCODE & net2hs(qp->h.flags)); /* return code for this */
    if (rcode != DNS_RSP_NO_ERR)                  /* message */
    {
        return((int)rcode);
    }
        
    /* response flag is set  and at least one answer       */
/*    if (nans >= 0 && (net2hs(qp->h.flags) & DQR))        */
    if ((signed short)nans >= 0 )
    { 
        if (nans == 0)
            return(DNS_RSP_NAME_ERR);

        /* set p to where QUESTION starts   */
        p = (PFBYTE)qp->x;               
        if (qp->h.qdcount > 0)
        {
            /* unpack QNAME (returns length of QNAME)   */
            i = unpackdom((PFCHAR)space, (PFCHAR)p, (PFCHAR)qp);

            /* spec defines NAME then QTYPE+QCLASS=4 bytes    */
            p += i+4;
        }

        /* look at RESOURCE RECORDS                                              */
        /* at this point, there may be several answers.  We will take the first  */
        /* one which has an IP number.  There may be other types of answers that */
        /* we want to support later.                                             */
        while (nans > 0)            /* look at each answer  */
        {
            nans--;

            /* unpack NAME in Resource Record      */
            i = unpackdom((PFCHAR)space, (PFCHAR)p, (PFCHAR)qp);
            p += i;                     /* account for string  */
            rrp = (PFRRPART)p;          /* resource record here  */

            /*  check things which might not align on 68000 chip    */
            /*  one byte at a time                                  */
            if ( (!*p && *(p+1) == DTYPEA && !*(p+2) && 
                  *(p+3) == DIN && addr) ||
                 (!*p && *(p+1) == DTYPEPTR && !*(p+2) && 
                  *(p+3) == DIN && !addr) )
            {       /* correct type and class */
                msg = os_alloc_packet(sizeof(struct hostentext), DNS_HOST2_ALLOC);
                if (!msg)
                {
                    return(set_errno(ENOPKTS));
                }
                p_theentry = (PFHOSTENT)DCUTODATA(msg);
        
                init_hostentext(p_theentry);

                /* save time-to-live   */
                ttl = WARRAY_2_LONG((unsigned short *)&(rrp->rttl));    /* __st__ w32 fix from Peter's code */
                p_theentry->h_ttl = ttl; 

                /* for gethostbyname fill in IP address, i.e. h_addr field   */
                /* NOTE: gethostbyaddr will fill in h_addr field             */
                if (addr)
                {
                    tc_mv4((PFBYTE)p_theentry->h_addr, (PFBYTE)rrp->rdata, IP_ALEN);
                                                        /* save IP #   */
                }
                else
                {
                    unpackdom((PFCHAR)psz_name, (PFCHAR)rrp->rdata, 
                              (PFCHAR)qp);
                }
                p_theentry->h_length++;

#if (DEBUG_DNS_RESULT)
                DEBUG_ERROR("--QDCOUNT, ANCOUNT = ", EBS_INT2, 
                    net2hs(qp->h.qdcount), net2hs(qp->h.ancount));
                DEBUG_ERROR("  QNAME : ", STR1, &(qp->x[1]), 0);
                DEBUG_ERROR("  space :", STR1, space, 0);
                DEBUG_ERROR("  psz_name :", STR1, psz_name, 0);
                DEBUG_ERROR("  ttl = ", DINT1, ttl, 0);
#endif      /* DEBUG_DNS_RESULT */

                /* add the entry to the host cache                       */
                /* upon return p_host will point to the entry just added */
                add_host_cache_entry(p_theentry, p_host, psz_name, FALSE);

                os_free_packet(msg);

                return(0);                      /* successful return  */
            }

            /* 68000 alignment   */
            tc_movebytes((PFBYTE)&j, (PFBYTE)&rrp->rdlength, 2);
            p += 10+net2hs(j);                  /* length of rest of RR  */
        }
    }
    return(-1);                     /* generic failed to parse  */
}


/* ************************************************************************   */
/* __get_addr_from_dns() -  formats DNS request and sends it                  */
/*                                                                            */
/*   Request the DNS name or IP address from a DNS server                     */
/*                                                                            */
/*   NOTE:                                                                    */
/*   dns_str for gethostbyaddr() = qstring = global = IP address              */
/*   dns_str for gethostbyname() = param from app   = dns name                */
/*                                                                            */
/*   Returns 0 on success, -1 upon failure, or error code                     */
/*   (DNS_RSP_FMT_ERR for example).                                           */
/*                                                                            */

int __get_addr_from_dns(PFCHAR psz_name, PFHOSTENT *p_host,
                        PSOCKADDR_IN p_srvr_addr,
                        int i_addr_len, int time_out, int querytype,
                        RTIP_BOOLEAN *response_flag)
{
struct  sockaddr_in soc_temp_addr;
struct timeval  wait_time;
struct fd_set   f_read;
int             retval;
int             i_temp_addr_len;
int             i_sock;                     /* Socket handle. */
struct sockaddr_in cli_addr;            /* client address; */
word            i_request_val;
int             i;
PFBYTE          p;
PFBYTE          p_save;
long            i_len;  /* wwb - change from int to long for large memory model */
struct useek    question;
DCU             smsg;
PFBYTE          space;

    retval = -1;    /* Default is fail */

    i_request_val = (word) ks_get_ticks();

    /* Create and initialize client socket.   */
    if ((i_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        DEBUG_ERROR("__get_addr_from_dns: out of sockets", NOVAR, 0, 0);
        return -1;
    }

    /* the sendto() call will do the binding   */
    tc_memset((PFBYTE)&cli_addr, 0, sizeof(struct sockaddr_in)-1);

    cli_addr.sin_family = AF_INET;
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
    cli_addr.sin_addr = INADDR_ANY;
#else
    cli_addr.sin_addr.s_addr = INADDR_ANY;
#endif

    cli_addr.sin_port = hs2net(0);

    if (bind(i_sock, (PCSOCKADDR)&cli_addr, sizeof(cli_addr)) < 0)
        goto done;

    /* Initialize query structure   */
    question.h.flags = net2hs(DRD);
    question.h.qdcount = net2hs(1);
    question.h.ancount = 0;
    question.h.nscount = 0;
    question.h.arcount = 0;

    p_save = (PFBYTE)question.x;

    i = packdom((PFCHAR)question.x, psz_name);

    /*                                                                     */
    /*  load the fields of the question structure a character at a time so */
    /*  that 68000 machines won't choke.                                   */
    /*                                                                     */
    p = (PFBYTE)&question.x[i];
    *p++=0;                 /* high byte of qtype */
    switch(querytype)
    {   
        case HOSTBYNAME:
            *p++ = DTYPEA;          /* number is<256, so we know high byte=0 */
        break;
    
        case HOSTBYADDR:
            *p++ = DTYPEPTR;
    }
    *p++ = 0;               /* high byte of qclass */
    *p++ = DIN;             /* qtype is<256 */
    question.h.ident = net2hs(i_request_val);
    i_len = sizeof(struct dhead) + (p-p_save);

    /* Format a DNS query, and send it   */
    if (sendto((int)i_sock, (PFCCHAR)&question, (int)i_len, 0,
               (PCSOCKADDR)p_srvr_addr,i_addr_len) > 0)
    {
        question.h.ident = 0;
        /* Wait for a response (add timeout support here - wwb done).    */
        FD_ZERO((PFDSET)&f_read);
        FD_SET(i_sock, (PFDSET)&f_read);
        wait_time.tv_sec = time_out;
        wait_time.tv_usec = 0;
#ifdef USE_NORTEL_SELECT
        if (select(i_sock+1, (PFDSET)&f_read, 0, 0, (PCTIMEVAL)&wait_time) > 0)
#else
        if (select(1, (PFDSET)&f_read, 0, 0, (PCTIMEVAL)&wait_time) > 0)
#endif
        {
            soc_temp_addr = *p_srvr_addr;
            i_temp_addr_len = i_addr_len;
        
            i_len = recvfrom((int)i_sock, (PFCHAR)&question, DOMSIZE,0,
                             (PSOCKADDR)&soc_temp_addr,
                             (PFINT)&i_temp_addr_len);
            if (i_len < 0)      /* tbd - check valid length */
            {
                DEBUG_ERROR("__get_addr_from_dns: recvfrom returned length", 
                    EBS_INT1, i_len, 0);
                return(-1);
            }

            if (net2hs(question.h.ident) == i_request_val)
            {
                *response_flag = TRUE;

                /* we need a buffer at least as large as CFG_DNS_NAME_LEN, so   */
                /* we allocate a DCU to use the data area                       */
                /* space alloc = min(CFG_DNS_NAME_LEN, 0x3f)                    */
                smsg = os_alloc_packet(MAX_PACKETSIZE, DNS_SPACE_ALLOC);
                if (!smsg)
                {
                    DEBUG_ERROR("__get_addr_from_dns: out of DCUs", NOVAR, 0, 0);
                    return(set_errno(ENOPKTS));
                }
                space = (PFBYTE)DCUTODATA(smsg);

                /* for HOSTBYADDR, ddextractname will write the DNS name   */
                /* into psz_name                                           */
                if (querytype == HOSTBYNAME) 
                    retval = ddextract((PFUSEEK)&question, p_host, 
                                       psz_name, space, TRUE);
                else
                {
                    retval = ddextract((PFUSEEK)&question, p_host, 
                                       psz_name, space, FALSE);
                }
                os_free_packet(smsg);
            }
        }
    }

done:
    closesocket((int)i_sock);
    return(retval);
}



/* *************************************************************************   */
/* _get_addr_from_dns() - called by get_addr_from_dns                          */
/*                                                                             */
/*  Returns 0 for success, -1 for failure.                                     */
/*                                                                             */

int _get_addr_from_dns(PFCHAR psz_host_name, PFHOSTENT *p_host, 
                       PSOCKADDR_IN ps_addr, int querytype, RTIP_BOOLEAN *response_flag)
{
int i;
int i_result = -1;
int delay = CFG_MIN_DNS_DELAY;

   for (i=0; i <= CFG_DNS_RETRIES; i++)
   {
        *response_flag = FALSE;
        i_result = __get_addr_from_dns(psz_host_name, p_host, ps_addr,
                                       sizeof(struct sockaddr_in),
                                       delay, querytype,
                                       response_flag);
        if ( (i_result == DNS_RSP_NO_ERR)   ||
             (i_result == DNS_RSP_NAME_ERR) ||
             (i_result == -1 && *response_flag) )   /* if response received */
                                                    /* but error   */
        {
            DEBUG_ASSERT(i_result == DNS_RSP_NO_ERR,
                "_get_addr_from_dns: result = ", EBS_INT2, 
                i_result, *response_flag);
            break;
        }


        DEBUG_LOG("_get_addr_from_dns: RETRY Loop", LEVEL_3, NOVAR, 0, 0);

        delay <<= 1;  
        if (delay > CFG_MAX_DNS_DELAY)
            delay >>= 1;
   }
   return i_result;
}

/* *************************************************************************   */
/* get_addr_from_dns() - gets an IP address from server                        */
/*                                                                             */
/*   Returns 0 for success, -1 for failure.                                    */
/*                                                                             */
/*                                                                             */

int get_addr_from_dns(PFCHAR psz_host_name, PFHOSTENT *p_host, int querytype)
{
int i;
struct sockaddr_in si_addr;
int i_result = -1;
RTIP_BOOLEAN response_flag;
RTIP_BOOLEAN server_found;
    response_flag = FALSE;
    server_found = FALSE;

    /* Use DNS server address here, also loop thru addresses if multiple   */
    /* servers supported.                                                  */
    /* we break out only when there's a response and no error              */
    for (i=0; i_result != DNS_RSP_NO_ERR && i < CFG_MAX_DNS_SRV; i++)
    {
        if (server_ip_table[i])
        {            
            server_found = TRUE;
            tc_memset((PFBYTE)&si_addr, 0, sizeof(struct sockaddr_in)-1);
            si_addr.sin_family = AF_INET;               
            si_addr.sin_port = hs2net(DNS_PORT);                
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
            si_addr.sin_addr = server_ip_table[i];              
#else
            si_addr.sin_addr.s_addr = server_ip_table[i];               
#endif
            i_result = _get_addr_from_dns(psz_host_name, p_host, 
                                          /* (PSOCKADDR_IN) */ &si_addr,
                                          querytype, &response_flag);
        }
    }

    /* set errno based upon the results from the last server   */
    if (i_result != DNS_RSP_NO_ERR)
    {
        if (i_result == DNS_RSP_NAME_ERR)
            return(set_errno(ENODATA));

        else if (i_result == DNS_RSP_SRV_FAIL)
            return(set_errno(ETRYAGAIN));

        /* if one of the following errors were returned                 */
        /*      DNS_RSP_FMT_ERR - unable to interpret inquiry (internal */
        /*                        DNS error - should never happen)      */
        /*      DNS_RSP_SRV_FAIL- unsuccessful due to problem with      */
        /*                        name server                           */
        /*      DNS_RSP_REFUSED - server refused due to policy reasons  */
        else if (i_result > 0)  
            return(set_errno(ENO_RECOVERY));

        else if (!server_found)
            return(set_errno(ENOSERVERS));

        /* if < 0 then socket call failed, assume no response   */
        /* tbd - maybe another socket call failed               */
        /* NOTE: if response then another error could have been */
        /*       detected, but errno already set                */
        else if (!response_flag)
            return(set_errno(ENORESPONSE));
        else 
        {
             return (-1);
        }
    }

    return(i_result);
} 


/* ********************************************************************   */
/* INTERNAL DNS ROUTINES - HOST TABLE                                     */
/* *******************************************************************   */

/* *************************************************************************   */
/* get_addr_from_host_table() - searches the host table for an address.        */
/*                                                                             */
/*   Searches the host table for an address and if found, sets p_host to       */
/*   point to the matching entry.                                              */
/*                                                                             */
/*   Returns 0 for success, -1 for failure.                                    */
/*                                                                             */

int get_addr_from_host_table(PFCHAR str, PFHOSTENT *p_host,
                             int querytype)
{
int i_result = -1;
int i;

    OS_CLAIM_TABLE(DNS_HOST1_TABLE_CLAIM)

    for (i=0; i < CFG_MAX_HT_ENT; i++)
    {
        if (h_table[i].sz_name[0] == '\0')
            continue;

        if ( ((querytype == HOSTBYNAME) && 
              !tc_strcmp(h_table[i].sz_name, str)) ||
             ((querytype == HOSTBYADDR) && 
              tc_comparen((PFBYTE)&(h_table[i].ip_addr.s_un.s_un_b.s_b1), 
                          (PFBYTE)str, IP_ALEN)) )
        {
            *p_host = (PFHOSTENT)&h_table[i];
            i_result = 0;
            break;
        }
    }
    OS_RELEASE_TABLE()

    return i_result;
}

/* ********************************************************************   */
/* INTERNAL DNS ROUTINES - HOST CACHE                                     */
/* ********************************************************************   */

#if (INCLUDE_DNS_CACHE)
/* **************************************************************************   */
/* get_addr_from_host_cache() - searches cache for address                      */
/*                                                                              */
/*   Searches the host cache table for an address and if found, sets p_host to  */
/*   point to the matching entry.                                               */
/* /                                                                            */
/* Returns 0 for success, -1 for failure                                        */
/*                                                                              */

int get_addr_from_host_cache(PFCHAR str, PFHOSTENT *p_host, 
                             int querytype)
{
int i_result = -1;
int i;

    OS_CLAIM_TABLE(DNS_HOST2_TABLE_CLAIM)

    for (i=0; i < CFG_MAX_HC_ENT; i++)
    {
        if (host_cache[i].sz_name[0] == '\0')
            continue;

        if ( ((querytype == HOSTBYNAME) && 
              !tc_strcmp(host_cache[i].sz_name, str)) ||
             ((querytype == HOSTBYADDR) && 
              tc_comparen((PFBYTE)&(host_cache[i].ip_addr.s_un.s_un_b.s_b1), 
                          (PFBYTE)str, IP_ALEN)) )
        {
            DEBUG_LOG("cache entry found", LEVEL_3, NOVAR, 0, 0);

            /* found an entry, return it   */
            *p_host = (PFHOSTENT)&host_cache[i];
            i_result = 0;
            break;
        }
    }

    OS_RELEASE_TABLE()

    return i_result;
}
#endif  /* INCLUDE_DNS_CACHE */

/* **********************************************************************   */
/* add_host_cache_entry() - adds an entry to the global host cache.         */
/*                                                                          */
/*   Adds an entry to the host cache based upon the_entry and psz_name      */
/*   at the next entry.  The host cache has a ttl and entries are           */
/*   freed by the timer task.                                               */
/*                                                                          */
/*   Sets p_host to entry just added.                                       */
/*                                                                          */
/*   Returns nothing                                                        */
/*                                                                          */

void add_host_cache_entry(PFHOSTENT the_entry, PFHOSTENT *p_host, PFCHAR psz_name, RTIP_BOOLEAN sem_claimed)
{
#if (INCLUDE_DNS_CACHE)
int   host_cache_index;
dword small_ttl;
int   ttl_index;
RTIP_BOOLEAN found;
#endif

    if (!sem_claimed)
    {
        OS_CLAIM_TABLE(DNS_HOST3_TABLE_CLAIM)
    }

#if (INCLUDE_DNS_CACHE)
    small_ttl = 0;
    ttl_index = -1;
    found = FALSE;
    for (host_cache_index=0; host_cache_index < CFG_MAX_HC_ENT; host_cache_index++)
    {
        if (host_cache[host_cache_index].sz_name[0] == 0)   /*If this entry is empty... */
        {
            /* found a free entry   */
            found = TRUE;
            break;
        }
        if ( (small_ttl == 0) || 
             (small_ttl > host_cache[host_cache_index].h_ttl) )
        {
            small_ttl = host_cache[host_cache_index].h_ttl;
            ttl_index = host_cache_index;
        }
    }
    if (!found)
    {
        if (ttl_index == -1)        /* should never happen */
        {
            host_cache_index = 0;
            DEBUG_ERROR("add_host_cache_index: ttl index = -1", NOVAR, 0, 0);
        }
        else
            host_cache_index = ttl_index;
    }
#else
    /* host_cache_index will be the offset of the entry just added    */
    host_cache_index++;
    if (host_cache_index >= CFG_MAX_HC_ENT) 
    {
        host_cache_index = 0;
    }
#endif

    init_hostentext((PFHOSTENT)&host_cache[host_cache_index]);

    tc_strcpy(host_cache[host_cache_index].sz_name, psz_name);
    host_cache[host_cache_index].h_addrtype = the_entry->h_addrtype;
    host_cache[host_cache_index].h_length =   IP_ALEN;
    tc_mv4(host_cache[host_cache_index].h_addr, the_entry->h_addr, IP_ALEN);
    host_cache[host_cache_index].h_ttl = the_entry->h_ttl;

    /* set pointer to entry just added to return to caller   */
    *p_host = (PFHOSTENT)&host_cache[host_cache_index];

    if (!sem_claimed)
    {
        OS_RELEASE_TABLE()
    }
}

#if (INCLUDE_DNS_CACHE)
/* **********************************************************************   */
/* dns_timer() - Host cache table time to live maintenance routine          */
/*                                                                          */
/*   Decrements time to live entries for all valid routing table entries    */
/*   until the time to live is infinite.  This routine is called every      */
/*   second.                                                                */
/*                                                                          */
/*   Returns nothing                                                        */
/*                                                                          */

void dns_timer(void)
{
int loop_cnt;

    for (loop_cnt=0; loop_cnt < CFG_MAX_HC_ENT; loop_cnt++)
    {
        if (host_cache[loop_cnt].h_ttl == 0)
            continue;

        if (host_cache[loop_cnt].h_ttl == 1)
        {
            host_cache[loop_cnt].sz_name[0] = 0;    /*If this entry is empty... */
        }
        host_cache[loop_cnt].h_ttl--;
    }
}
#endif /* INCLUDE_DNS_CACHE */

/* ********************************************************************   */
/* INTERNAL DNS ROUTINES - GET_HOST_BY                                    */
/* ********************************************************************   */

/* dns_str for gethostbyaddr() = qstring = global = IP address   */
/* dns_str for gethostbyname() = param from app   = dns name     */
PFHOSTENT gethostby(PFCHAR dns_str, PFCHAR hostent_str, int querytype)
{         
PFHOSTENT return_val; 
PFHOSTENT p_h_table_host;   /* set to point to entry in host table or */
                                /* host cache table   */

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return 0;
    }
#endif

    /* initialize it just in case it's not updated by    */
    /* get_addr_from_host_table                          */
    p_h_table_host = 0;

    return_val = 0;

    if (get_addr_from_host_table(hostent_str, &p_h_table_host, querytype) == 0)
    {
        return_val = p_h_table_host;
    }
    else    
    {
#if (INCLUDE_DNS_CACHE)
        if (get_addr_from_host_cache(hostent_str, &p_h_table_host, querytype) == 0)
        {
            return_val = p_h_table_host;
        }
        else
#endif  /* INCLUDE_DNS_CACHE */
        {   
            /* get the address from the DNS server                         */
            /* NOTE: after the address is found, add_host_cache_entry()    */
            /*       will be called to add the entry to the host cache and */
            /*       p_h_table_host will be set to point to this entry     */
            if (get_addr_from_dns(dns_str, &p_h_table_host, querytype) == 0)
            {   
                return_val = p_h_table_host;
            }
        }
    }

   return return_val;
}


/* ********************************************************************   */
/* API ROUTINES                                                           */
/* ********************************************************************   */

/* ********************************************************************        */
/* xn_add_host_table_entry() - Add a host entry to the host table              */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "socket.h"                                                       */
/*                                                                             */
/*   int xn_add_host_table_entry(name, ipaddr)                                 */
/*      PFCHAR name     -   The name of the host                               */
/*      dword ipaddr    -   The IP address of the host                         */
/*                                                                             */
/* Description:                                                                */
/*    Adds an entry to the global host table, if there is space.               */
/*                                                                             */
/*    For more details see RTIP Reference Manual.                              */
/*                                                                             */
/* Returns:                                                                    */
/*    -1 if the host table is full, 0 on success                               */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   RTIP Reference's Manual.                                                  */
/*                                                                             */

int xn_add_host_table_entry(PFCHAR name, dword ipaddr)
{
int loop_cnt;

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
      return set_errno(ENOTINITIALIZED);
#endif
   
    OS_CLAIM_TABLE(DNS_ADD_TABLE_CLAIM)

    for (loop_cnt=0; loop_cnt < CFG_MAX_HT_ENT; loop_cnt++)
    {
        if (h_table[loop_cnt].sz_name[0] == 0)              /*If this entry is empty... */
        {       
            tc_strcpy(h_table[loop_cnt].sz_name, name);
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
            h_table[loop_cnt].ip_addr.s_un.s_addr = ipaddr;
#else
            h_table[loop_cnt].ip_addr.s_addr = ipaddr;
#endif
            h_table[loop_cnt].h_length = IP_ALEN;
            OS_RELEASE_TABLE()
            return 0;                                       /* SUCCESS */
        }
    }

    OS_RELEASE_TABLE();
    return set_errno(EHTABLEFULL);                      /* FAILURE     */
}

/* ********************************************************************        */
/* xn_delete_host_table_entry() - Delete a host entry from the host table      */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "socket.h"                                                       */
/*                                                                             */
/*   int xn_delete_host_table_entry(name)                                      */
/*     PFCHAR name      The name of the host to delete from table              */
/*                                                                             */
/* Description:                                                                */
/*    Deletes an entry from the global host table.                             */
/*                                                                             */
/*   For more details see RTIP Reference Manual.                               */
/*                                                                             */
/* Returns:                                                                    */
/*    -1 if the host was not found, 0 on success                               */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   RTIP Reference's Manual.                                                  */
/*                                                                             */

int xn_delete_host_table_entry(PFCHAR name)
{
int loop_cnt;
int ret_val;

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
       return set_errno(ENOTINITIALIZED);
#endif

    OS_CLAIM_TABLE(DNS_DEL_TABLE_CLAIM)

    ret_val = -1;
    for (loop_cnt=0; loop_cnt < CFG_MAX_HT_ENT; loop_cnt++)
    {
        if (h_table[loop_cnt].sz_name[0] == '\0')
            continue;
        if (!tc_strcmp(h_table[loop_cnt].sz_name, name))
        {
            h_table[loop_cnt].sz_name[0] = '\0';
            ret_val = 0;
            break;
        }
    }

    OS_RELEASE_TABLE();

    if (ret_val == -1)
        set_errno(EENTRYNOTFOUND);

    return(ret_val);
}   

#if (INCLUDE_DNS_CACHE)
/* *************************************************************************   */
/* xn_clear_host_cache() - zeroes out the internal cache                       */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "socket.h"                                                       */
/*                                                                             */
/*   xn_clear_host_cache(void)                                                 */
/*                                                                             */
/* Description:                                                                */
/*                                                                             */
/* Returns:                                                                    */
/*   Nothing.                                                                  */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   RTIP Reference's Manual.                                                  */
/*                                                                             */

int xn_clear_host_cache(void)
{
int loop_cnt;

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
      return set_errno(ENOTINITIALIZED);
#endif

    for (loop_cnt=0; loop_cnt < CFG_MAX_HC_ENT; loop_cnt++)
    {
         init_hostentext((PFHOSTENT)&host_cache[loop_cnt]);
    }

    return 0;
}
#endif  /* INCLUDE_DNS_CACHE */


/* ********************************************************************       */
/* xn_add_dns_server() - add a DNS server's IP                                */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "socket.h"                                                      */
/*                                                                            */
/*   int xn_add_dns_server(addr)                                              */
/*       PFCBYTE addr        -  DNS server IP addresses in network byte order */
/*                                                                            */
/* Description:                                                               */
/*   Adds addr to the global DNS server list.  See xn_set_server_list()       */
/*                                                                            */
/* Returns:                                                                   */
/*   Returns 0 if successful                                                  */

int xn_add_dns_server(PFCBYTE addr)
{
int i;
struct in_addr in;

    tc_movebytes(&in, addr, IP_ALEN);
    OS_CLAIM_TABLE(DNS_INIT_TABLE_CLAIM)
    for (i=0; i < CFG_MAX_DNS_SRV; i++) 
    {
        if (server_ip_table[i] == 0) 
        {
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
            server_ip_table[i] = in.s_un.s_addr;
#else
            server_ip_table[i] = in.s_addr;
#endif
            OS_RELEASE_TABLE()
            cnt_dns++;
            return 0;
        }
    }
    OS_RELEASE_TABLE()
    return(set_errno(ETOOMANYSERVERS));
}

/* ********************************************************************        */
/* xn_set_server_list() - Defines the global list of DNS Servers               */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "socket.h"                                                       */
/*                                                                             */
/*   int xn_set_server_list(list,num_elements)                                 */
/*       dword list[]        -      List of 32-bit server IP addresses         */
/*       int num_elements    -      Number of elemenets to copy                */
/*                                                                             */
/*                                                                             */
/* Description:                                                                */
/*   This function copies the contents of the list[] parameter into the        */
/*   global server list.  List[] need not be preserved after this function     */
/*   is called.                                                                */
/*                                                                             */
/*   For more details see RTIP Reference Manual.                               */
/*                                                                             */
/* Returns:                                                                    */
/*   -1 if num_elements was larger than CFG_MAX_DNS_SRV, 0 on success          */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by */
/*   this function.  For error values returned by this function see            */
/*   RTIP Reference's Manual.                                                  */
/*                                                                             */

int xn_set_server_list(dword list[], int num_elements)
{
int i;

#if (INCLUDE_ERROR_CHECKING)
    if (!rtip_initialized)
      return(set_errno(ENOTINITIALIZED));
#endif

    /* if an attempt was made to set too many servers   */
    if (num_elements > CFG_MAX_DNS_SRV) 
        return(set_errno(ETOOMANYSERVERS));

    for (i=0; i < CFG_MAX_DNS_SRV; i++)
    {
        if (i < num_elements)
            server_ip_table[i] = list[i];   /* set the server address */
        else
            server_ip_table[i] = 0L;    /* zero out unset local servers */
    }
    cnt_dns = num_elements;
    return(0);
} 


/* **************************************************************************   */
/* gethostbyname() -  Takes a name string, gives an address                     */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "socket.h"                                                        */
/*                                                                              */
/*   PFHOSTENT gethostbyname(psz_host_name)                                     */
/*      PFCHAR psz_host_name    -   Pointer to name string                      */
/*                                                                              */
/* Description:                                                                 */
/*   LIMITATIONS:  (tbd)                                                        */
/*   1) Only retrieves a single IP address (not good for multi-homed hosts).    */
/*   2) Timer is not implemented.                                               */
/*   3) Should support local address cache                                      */
/*   4) Should return correct error code ie: dns server no reachable, etc.      */
/*   5) Set request # from timer (or something else fairly unique).             */
/*   6) Should allocate and set host name (in hostent structure).               */
/*   7) Does not support default domain (probably should not).                  */
/*                                                                              */
/* Returns:                                                                     */
/*   Pointer to hostentext structure which contains, among other things,        */
/*   the IP address.                                                            */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by  */
/*   this function.  For error values returned by this function see             */
/*   RTIP Reference's Manual.                                                   */
/*   Possible values for this function are:                                     */
/*                                                                              */
/*     ENOTINITIALIZED  -   init_dns has not been called                        */
/*     ENORESPONSE      -   there was no reponse from the name server           */
/*                                                                              */
int xc_strlen(PFCCHAR string)   /*__fn__*/
{
int len=0;

   while (string[len] != 0)
    len++;
   return len;
}

PFHOSTENT gethostbyname(PFCHAR name)
{         
PFHOSTENT p_host;
byte address[IP_ALEN];
/* buffer to hold the full host name if an abbreviated name is given   */
char namebuff[CFG_DNS_NAME_LEN+1];
DCU msg;
PFHOSTENT p_theentry;
PFCHAR dm;
int    dm_len;

    if (tc_strlen((PFCHAR)name) > CFG_DNS_NAME_LEN)
    {
        set_errno(ENAME_TOO_LONG);
        return((PFHOSTENT)0);
    }

    /* if the name is a IP address in string format then return   */
    /* the IP address converted to binary address                 */
    if (ip_str_to_bin(address, name))
    {
        msg = os_alloc_packet(sizeof(struct hostentext), DNS_HOST3_ALLOC);
        if (!msg)
        {
            set_errno(ENOPKTS);
            return((PFHOSTENT)0);
        }
        p_theentry = (PFHOSTENT)DCUTODATA(msg);

        init_hostentext(p_theentry);

        tc_mv4(p_theentry->h_addr, address, IP_ALEN);
        p_theentry->h_ttl = 0;

        /* add the entry to the host cache;                          */
        /* after the call, p_host will point to the entry just added */
        add_host_cache_entry(p_theentry, &p_host, name, FALSE);

        os_free_packet(msg);
    }
    else
    {
        p_host = gethostby(name, name, HOSTBYNAME);
        if (p_host)
            tc_strcpy((PFCHAR)p_host->sz_name, name);
        else
        {
            /* Assume this is not a FQDN, unless it is postfixed with a dot '.' 
             * Attempt resolve() with default domain name appended.
             */
            if (name[0] && name[tc_strlen(name)-1] != '.')
            {
                /* We could've used tc_snprintf() more easily, but then it wouldn't work
                 * without our tc_snprintf() implementation
                 */
                dm = xn_get_domain_name();
                dm_len = 0;
                if (dm)
                    dm_len = tc_strlen(dm);
                if (tc_strlen(name)+dm_len >= CFG_DNS_NAME_LEN) 
                {
                    set_errno(ENAME_TOO_LONG);
                    return((PFHOSTENT)0);
                }
                else 
                {
                    if (dm)
                        tc_sprintf(namebuff, "%s.%s", name, dm);
                    else
                        tc_strcpy(namebuff, name);
                }
                
                /* FQDN constructed. Resolve.   */
                p_host = gethostby(namebuff, namebuff, HOSTBYNAME);
                if (p_host)
                    tc_strcpy((PFCHAR)p_host->sz_name, namebuff);
            }
        }
    }

    return(p_host);
}


/* **************************************************************************   */
/* gethostbyaddr() -  Takes an IP address, gives a name string                  */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "socket.h"                                                        */
/*                                                                              */
/*   PFHOSTENT gethostbyaddr(addr, len, type)                                   */
/*      PFCHAR addr -   Pointer to IP address                                   */
/*      int len     -   Not implemented.                                        */
/*      int type    -   Not implemented.                                        */
/*                                                                              */
/* Description:                                                                 */
/*                                                                              */
/* Returns:                                                                     */
/*   Pointer to hostentext structure which contains, among other things,        */
/*   the name string.                                                           */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set by  */
/*   this function.  For error values returned by this function see             */
/*   RTIP Reference's Manual.                                                   */
/*   Possible values for this function are:                                     */
/*                                                                              */
/*     ENOTINITIALIZED  -   init_dns has not been called                        */
/*     ENORESPONSE          -   there was no reponse from the name server       */
/*                                                                              */

PFHOSTENT gethostbyaddr(PFCHAR addr, int len, int type)
{
PFHOSTENT p_host;
DCU qmsg;
PFCHAR qstring;
char dns_addr[IP_ALEN];

    ARGSUSED_INT(len);
    ARGSUSED_INT(type);

    /* we need a buffer at least as large as CFG_DNS_NAME_LEN, so   */
    /* we allocate a DCU to use the data area                       */
    qmsg = os_alloc_packet(CFG_DNS_NAME_LEN, DNS_STRING_ALLOC);
    if (!qmsg)
    {
        set_errno(ENOPKTS);
        return((PFHOSTENT)0);
    }

    /* flip bytes for request   */
    dns_addr[0] = addr[3];
    dns_addr[1] = addr[2];
    dns_addr[2] = addr[1];
    dns_addr[3] = addr[0];

    qstring = (PFCHAR)(DCUTODATA(qmsg));
    ip_bin_to_str(qstring, (PFCBYTE)dns_addr);
    tc_strcat(qstring, ".IN-ADDR.ARPA");

    p_host = gethostby((PFCHAR)qstring, addr, HOSTBYADDR);
    if (p_host)
        tc_mv4((PFBYTE)(p_host->h_addr), (PFBYTE)addr, IP_ALEN);

    os_free_packet(qmsg);

    return(p_host);

}


#endif  /* INCLUDE_DNS */

/* *************************************************************************      */
/*          LOCAL DOMAIN NAME ROUTINES                                            */
/* *************************************************************************      */
/* xn_set_domain_name - record system's default domain name                       */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   int xn_set_domain_name(name)                                                 */
/*       PFCCHAR name        -  Local domain name                                 */
/*                                                                                */
/* Description:                                                                   */
/*   Sets the domain name for this host                                           */
/*                                                                                */
/* Returns:                                                                       */
/*   Returns 0 if successful, -1 if failure                                       */
/*                                                                                */

int xn_set_domain_name(PFCCHAR name)
{
    if (tc_strlen(name) < sizeof(my_domain_name)) 
    {
        tc_strcpy(my_domain_name, name);
        return 0;
    } 
    else 
    {
        return(set_errno(EFAULT));
    }
}

/* *************************************************************************      */
/* xn_has_dns - determines if DNS server has been set up                          */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   int xn_has_dns(void)                                                         */
/*                                                                                */
/* Description:                                                                   */
/*   Determines if any DNS servers have been set up.  This is useful              */
/*   for PPP since PPP might setup DNS servers.                                   */
/*                                                                                */
/* Returns:                                                                       */
/*   Returns 1 if a DNS server has been setup or 0 if none have.                  */
/*                                                                                */

int xn_has_dns(void)
{
    return(my_domain_name[0] && cnt_dns > 0);
}

/* *************************************************************************      */
/* xn_get_domain_name - return system's default domain name                       */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   PFCHAR xn_get_domain_name(void)                                              */
/*                                                                                */
/* Description:                                                                   */
/*   Returns local domain name.                                                   */
/*                                                                                */
/* Returns:                                                                       */
/*   Returns address of local domain name or 0 if a domain name                   */
/*   has not been setup.                                                          */
/*                                                                                */

PFCHAR xn_get_domain_name(void)
{
    if (my_domain_name[0])
        return my_domain_name;
    else
        return 0;
}





