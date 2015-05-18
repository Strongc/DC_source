/*                                                                         */
/* rtipstub.c - functions needed for standalone which are provided by RTIP */
/*                                                                         */
/* EBS - RTIP                                                              */
/*                                                                         */
/* Copyright Peter Van Oudenaren , 1993                                    */
/* All rights reserved.                                                    */
/* This code may not be redistributed in source or linkable object form    */
/* without the consent of its author.                                      */
/*                                                                         */

#include "sock.h"

#if (!INCLUDE_RTIP)

KS_GLOBAL_CONSTANT byte KS_FAR ip_nulladdr[IP_ALEN] = {0, 0, 0, 0};

#if (INCLUDE_WEB) || (INCLUDE_SSL)

/* ********************************************************************   */
/* convert a hex string to decimal integer                                */
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

/* ********************************************************************   */
void xn_abort(int socket, RTIP_BOOLEAN send_reset)
{
LINGER ling;

    /* set socket option to linger with 0 timeout - hard close   */
    ling.l_onoff = 1;
    ling.l_linger = 0;
    if (setsockopt(socket, SOL_SOCKET, 
                   SO_LINGER, (PFCCHAR)&ling, 
                   sizeof(struct linger)))
    {
        DEBUG_ERROR("setsockopt failed", NOVAR, 0, 0);
    }
}

/* ********************************************************************   */
int set_errno(int errno)
{
    return(-1);
}

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
#endif      /* WEBS or SSL */


/* ********************************************************************   */
void debug_error(char *str, int type, void * val1, void * val2)
{
    switch (type)
    {
    case STR1:
        printf("%s%s\n", str, (char *)val1);
        break;
    case STR2:
        printf("%s%s %s\n", str, (char *)val1, (char *)val2);
        break;
    case EBS_INT1:
        printf("%s%x\n", str, (int)val1);
        break;
    case EBS_INT2:
        printf("%s%x %x\n", str, (int)val1, (int)val2);
        break;
    case DINT1:
        printf("%s%lx\n", str, (unsigned long)val1);
        break;
    case DINT2:
        printf("%s%lx %lx\n", str, (unsigned long)val1, (unsigned long)val2);
        break;
    case LINT1:
        printf("%s%x\n", str, (long)val1);
        break;
    case LINT2:
        printf("%s%x %x\n", str, (long)val1, (long)val2);
        break;
    default:
        printf("%s\n", str);
        break;
    }   /* end of switch */
}

#endif      /* !INCLUDE_RTIP */

