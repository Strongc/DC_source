/*                                                                      */
/* sock.c - sockets porting layer functions                             */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define SET_SOCKET_NONBLOCK 0

/*************************************************************************
 Header files
*************************************************************************/

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/*************************************************************************
 Static Data
*************************************************************************/

#if(INCLUDE_WINSOCK)
static int winsock_initialized=0;
static struct WSAData wsa_data;
#endif


/*************************************************************************
 net_network_init() - Initialize TCP/IP Networking

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

int net_network_init (void)
{
#if (INCLUDE_WINSOCK)

    if(!winsock_initialized)
    {
        /* we support at least up to version 1.1   */
        if(WSAStartup(0x0101,&wsa_data))
        {
            return(-1);
        }

        winsock_initialized=1;
    }
    
#elif (defined(MT92101))
    return(0);

#elif (INCLUDE_RTIP)

#endif

    return (0);
}


/*************************************************************************
 net_network_close() - Do any TCP/IP related cleanup

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

int net_network_close (void)
{
#if(INCLUDE_WINSOCK)
    if(winsock_initialized)
    {
        winsock_initialized = 0;
        return (WSACleanup());
    }
#elif (INCLUDE_RTIP)
#endif

    return (0);
}



/*************************************************************************
 net_socket() - Allocate a socket

 Returns: SOCKET on success, INVALID_SOCKET otherwise.
*************************************************************************/

SOCKET net_socket (int family, int type, int protocol)
{
SOCKET sd;

    sd = socket(family,type,protocol);
    return (sd);
}


/*************************************************************************
 net_closesocket() - Terminate a socket connection

 sd - socket to close

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

int net_closesocket (SOCKET sd)
{
#if (INCLUDE_SUPPORT_SOCKETS && INCLUDE_SSL)
PTCPPORT pport;
#endif

#if (INCLUDE_SUPPORT_SOCKETS)
    if (!IS_INVALID_SOCKET(sd))
    {
#if (INCLUDE_SSL)
        if (net_is_secure_socket(sd))
        {
            pport = (PTCPPORT)api_sock_to_port(sd);
            if (!pport)
                return (-1);

            /* ssl will close the socket   */
            return (vssl_clssock(sd,pport->ssl));
        }
#endif

#if (defined(MT92101))
        shutdown(sd, 0);
        return(0);
#else
        return (closesocket(sd));
#endif

    }
#endif /* INCLUDE_SUPPORT_SOCKETS */

    return (-1);
}


/*************************************************************************
 net_bind() - Establish a TCP bindion to a particular ip/port

 sd - socket to use for this connection
 ip_addr - 4 byte array containing IP address to bind to
 port - port number to bind to

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

int net_bind (SOCKET sd, PFBYTE ip_addr, word port)
{
#if (INCLUDE_SUPPORT_SOCKETS)
struct sockaddr_in sin;

    sin.sin_family=AF_INET;

#if (defined(MT92101))
    sin.sin_port=port;
#else
    sin.sin_port=htons(port);
#endif

#if (WEBC_WINSOCK || CFG_UNIX_COMPLIANT_SOCK_STRUCTS)
    tc_mv4((PFBYTE)&sin.sin_addr.s_addr, ip_addr, IP_ALEN);
#else
    tc_mv4((PFBYTE)&sin.sin_addr, ip_addr, IP_ALEN);
#endif

    if (bind(sd, (PSOCKADDR)&sin, sizeof(sin)) < 0)
    {
        return (-1);
    }

    /* Success. Ready to send   */
    return (0);
#else /*OS*/ /* added else and moved endif */
    return(ESOCKNOTSUPPORT);
#endif /* INCLUDE_SUPPORT_SOCKETS */
}


/*************************************************************************
 net_connect() - Establish a TCP connection to a particular ip/port

 sd - socket to use for this connection
 ip_addr - 4 byte array containing IP address to connect to
 port - port number to connect to

#if (INCLUDE_SSL)
   If the TCP sockets has the SO_SECURE_SOCKET option the connect
   will be sent and then the SSL connect will be done
#endif

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

/* SHANE: timeout parameter is never used   */

int net_connect (SOCKET sd, PFBYTE ip_addr, word port, int timeout)
{
#if (INCLUDE_SUPPORT_SOCKETS)
struct sockaddr_in sin;

    sin.sin_family=AF_INET;

#if (defined(MT92101))
    sin.sin_port=port;
#else
    sin.sin_port=htons(port);
#endif

#if (WEBC_WINSOCK || CFG_UNIX_COMPLIANT_SOCK_STRUCTS)
    tc_mv4((PFBYTE)&sin.sin_addr.s_addr,ip_addr,IP_ALEN);
#else
    tc_mv4((PFBYTE)&sin.sin_addr,ip_addr,IP_ALEN);
#endif

    return (connect(sd, (PSOCKADDR)&sin, sizeof(sin)));

#else /*OS*/ /* added else and moved endif */
    return(ESOCKNOTSUPPORT);
#endif /* INCLUDE_SUPPORT_SOCKETS */
}

/*************************************************************************
 net_listen() - Listen on TCP socket for a connection

 sd - socket to use for this connection
 backlog - backlog size

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

int net_listen (SOCKET sd, int backlog)
{
#if (INCLUDE_SUPPORT_SOCKETS)
    return(listen(sd, backlog));
#else /*OS*/ /* added else and moved endif */
    return(ESOCKNOTSUPPORT);
#endif /* INCLUDE_SUPPORT_SOCKETS */
}

/*************************************************************************
 net_accept() - Listen on TCP socket for a connection

 sd - socket to use for this connection
 backlog - backlog size

#if (INCLUDE_SSL)
   For TCP over SSL returns port number of established
   connection or -1 if timeout or other error occurs.
#endif

 Returns: 0 on success, -1 otherwise.
*************************************************************************/

int net_accept (SOCKET sock_master, SOCKET *sock_slave)
{
#if (INCLUDE_SUPPORT_SOCKETS)
struct sockaddr_in port; /* port to use. default or result of port command */
int sin_len;

    port.sin_family = AF_INET;
    sin_len = sizeof(port);

    /* Wait for a request from client i.e.                       */
    /* put this task to sleep until a connection is requested by */
    /* a client if socket is in blocking mode                    */
    *sock_slave = accept(sock_master, (PSOCKADDR)&port, &sin_len);
    if (*sock_slave == INVALID_SOCKET)  return(-1);

    return(0);
#else /*OS*/ /* added else and moved endif */
    return(ESOCKNOTSUPPORT);
#endif /* INCLUDE_SUPPORT_SOCKETS */
}

/*************************************************************************
 net_recv() - receive data over a socket

 sd - socket to receive data over
 buffer - buffer to place data into
 size - the size of the buffer (max bytes to read)

 Returns: number of bytes received on success, < 0 on error
*************************************************************************/

int net_recv (SOCKET sd, PFCHAR buffer, int size)
{
#if (INCLUDE_SUPPORT_SOCKETS)
int pkt_len;

#if (INCLUDE_SSL)   /* __st__ 2002.08.20 */
PTCPPORT pport;

    if (net_is_secure_socket(sd))
    {
        pport = (PTCPPORT)api_sock_to_port(sd);
        if (!pport)
            return (-1);

        /* will return -1 if vssl_api has not been initialized yet or vssl_sslread was not set up   */
        pkt_len = vssl_sslread(pport->ssl,
                               buffer,
                               (unsigned int)size,
                               net_is_blocking(sd));
    }
    else
#endif  /* INCLUDE_SSL */
    {
        pkt_len = recv(sd, buffer, size, 0);
    }
    return (pkt_len);

#else /*OS*/ /* added else and moved endif */
    return(ESOCKNOTSUPPORT);
#endif /* INCLUDE_SUPPORT_SOCKETS */
}

/*************************************************************************
 net_send() - send data over a socket

 sd - socket to send data over
 buffer - data to send
 size - the size of the buffer (max bytes to send)

 Returns: number of bytes sent on success, < 0 on error
*************************************************************************/

int net_send (SOCKET sd, PFCCHAR buffer, int size)
{
#if (INCLUDE_SUPPORT_SOCKETS)
int bytes_sent;

#if (INCLUDE_SSL)   /* __st__ 2002.08.20 */
PTCPPORT pport;

    if (net_is_secure_socket(sd))
    {
        pport = (PTCPPORT)api_sock_to_port(sd);
        if (!pport)
            return (-1);

        /* will return -1 if vssl_api has not been initialized yet or vssl_sslwrite was not set up   */
        bytes_sent = vssl_sslwrit(pport->ssl,
                                   buffer,
                                   (unsigned int)size,
                                   net_is_blocking(sd));
    }
    else
#endif  /* INCLUDE_SSL */
    {
        bytes_sent = send(sd, buffer, size, 0);
    }
    return (bytes_sent);
#else
    return(ESOCKNOTSUPPORT);
#endif /* INCLUDE_SUPPORT_SOCKETS */
}

/*************************************************************************
  Note: Use when FIONBIO was used
        FIONREAD, and FIONWRITE do not set
        IO_BLOCK_OPT flag in options
*************************************************************************/

RTIP_BOOLEAN net_is_blocking (SOCKET sd)
{
PANYPORT pport;

    pport = api_sock_to_port(sd);
    if (!pport)
        return (FALSE);

    if (pport->options & IO_BLOCK_OPT)
        return (TRUE);
    return (FALSE);
}

/*************************************************************************
  turn blocking on so receive will wait

  Note: Use for FIONBIO
        If you want to set for FIONREAD, FIONWRITE you
        should call ioctlsocket directly
*************************************************************************/

RTIP_BOOLEAN net_set_blocking (SOCKET sd)
{
unsigned long arg;

    arg = 0;   /* disable non-blocking mode (enable blocking) */
    if (ioctlsocket(sd, FIONBIO, (unsigned long KS_FAR *)&arg) != 0)
        return(FALSE);
    return(TRUE);
}

/*************************************************************************
  turn blocking off so receive will return an error if request
  has not arrived; select should be used to ensure receive only
  called if request has arrived

  Note: Use for FIONBIO
        If you want to set for FIONREAD, FIONWRITE you
        should call ioctlsocket directly
*************************************************************************/

RTIP_BOOLEAN net_set_non_blocking (SOCKET sd)
{
unsigned long arg;

    arg = 1;   /* enable non-blocking mode */
    if (ioctlsocket(sd, FIONBIO, (unsigned long KS_FAR *)&arg) != 0)
        return(FALSE);
    return(TRUE);
}

/*************************************************************************
  SO_LINGER     on
  Linger Block  zero
  Will Block    no
  Data Lost     possibly

  Note: Discards data in output window and sends RESET
*************************************************************************/

RTIP_BOOLEAN net_set_hard_close (SOCKET sd)
{
LINGER ling;

    /* set socket option to linger with 0 timeout - hard close   */
    ling.l_onoff = 1;
    ling.l_linger = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_LINGER, (PFCCHAR)&ling, sizeof(struct linger)))
        return(FALSE);
    return (TRUE);
}

/*************************************************************************
  SO_LINGER     on
  Linger Block  non-zero
  Will Block    yes
  Data Lost     possibly

  Note: After output window empy, sends FIN
        If timeout, sends RESET
*************************************************************************/

RTIP_BOOLEAN net_set_linger (SOCKET sd, int timeout)
{
LINGER ling;

    /* set socket option to linger with 0 timeout - hard close   */
    ling.l_onoff = 1;
    ling.l_linger = timeout;
    if (setsockopt(sd, SOL_SOCKET, SO_LINGER, (PFCCHAR)&ling, sizeof(struct linger)))
        return(FALSE);
    return (TRUE);
}

#if (INCLUDE_SSL)
/*************************************************************************
*************************************************************************/

RTIP_BOOLEAN net_is_secure_socket (SOCKET sd)
{
PANYPORT pport;

    pport = api_sock_to_port(sd);
    if (!pport)
        return (FALSE);

    if (pport->options & SOO_SECURE_SOCKET)
        return (TRUE);
    return (FALSE);
}
#endif

#if (INCLUDE_SSL)
/*************************************************************************
  turn on secure socket option
  sctx must be initialized by vssl_cliinit or vssl_srvinit
*************************************************************************/

RTIP_BOOLEAN net_set_socket_secure (SOCKET sd, void *sctx)
{
PANYPORT pport;

    pport = api_sock_to_port(sd);
    if (!pport)
        return (FALSE);

    if ( setsockopt(sd, SOL_SOCKET, SO_SECURE_SOCKET, (PFCCHAR)sctx, 1) )
        return (FALSE);
    return (TRUE);
}
#endif

#if (INCLUDE_SSL)
/*************************************************************************
  turn off secure socket option
*************************************************************************/

RTIP_BOOLEAN net_set_socket_not_secure (SOCKET sd)
{
PANYPORT pport;

    pport = api_sock_to_port(sd);
    if (!pport)
        return (FALSE);

    if ( setsockopt(sd, SOL_SOCKET, SO_SECURE_SOCKET, (PFCCHAR)NULL, 0) )
        return (FALSE);
    return (TRUE);
}
#endif
