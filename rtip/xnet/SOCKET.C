/*                                                                      */
/* SOCKET.C - SOCKET API functions                                      */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_IP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_RAW    0
#define DEBUG_SELECT 0

/* ********************************************************************   */
/* EXTERNAL FUNCTIONS                                                     */
#if (INCLUDE_UDP)
int tc_udp_read(int socket, PANYPORT pport, PFCHAR buffer, int buflen, 
                PSOCKADDR_IN addr_in, int flags, word wait_count);
int tc_udp_write(int socket, PANYPORT pport, PFBYTE buffer, int buflen, 
                 PCSOCKADDR to, int tolen, word wait_count);
#endif

/* ********************************************************************           */
/* socket() - Allocate a UDP or TCP socket port.                                  */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   int socket(family, type, protocol)                                           */
/*     int family -  address format specification.  Only PF_INET is               */
/*                   supported.                                                   */
/*     int type   - SOCK_STREAM (TCP), SOCK_DGRM (UDP) or SOCK_RAW                */
/*     int protocol - protocol to be used with the socket (used for SOCK_RAW)     */
/*                                                                                */
/* Description:                                                                   */
/*   This function returns an integer which correlates to a port structure which  */
/*   may then later be used as an argument to the other socket functions.         */
/*   The number of ports available at any one time are determined by the          */
/*   constants CFG_NUDPPORTS and CFG_NTCPPORTS in xnconf.h.                       */
/*                                                                                */
/*   For more details see the RTIP Manual.                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   -1 if error or an integer (0 or greater) which specifies the port number.    */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set       */
/*   by this function.  For error values returned by this function see            */
/*   RTIP Reference Manual.                                                       */

int socket(int family, int type, int protocol)  /*__fn__*/
{
PANYPORT pport;

    RTIP_API_ENTER(API_SOCKET)

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    ARGSUSED_INT(protocol);   /* keep compiler happy */

#if (INCLUDE_ERROR_CHECKING)
    if (family != PF_INET)
        return(set_errno(EAFNOSUPPORT));
#else
    ARGSUSED_INT(family);
#endif

#if (INCLUDE_TCP)
    if (type == SOCK_STREAM)
        pport = (PANYPORT)tc_tcp_sock_alloc(FALSE);
    else
#endif

#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (type == SOCK_DGRAM) || (type == SOCK_RAW) )
        pport = (PANYPORT)tc_udp_sock_alloc(type, protocol);
    else
#endif
        return(set_errno(ETNOSUPPORT));

    if (pport)
    {
        pport->ip_ttl = -1;       /* set thru socket option (TCP, UDP and RAW) */
        pport->mcast_ip_ttl = -1; /* set thru socket option (UDP only) */
        return(pport->ctrl.index);
    }

    return(-1);   /* NOTE: errno will be set by tc_tcp_sock_alloc or */
                  /*       tc_udp_sock_alloc   */
}


#if (INCLUDE_TCP)
/* ********************************************************************     */
/* listen() - Accept connections from any remote host (TCP only)            */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int listen(socket, backlog)                                            */
/*     int socket       - Socket returned by socket & bound in              */
/*                        bind                                              */
/*     int backlog      - number of pending connections port is allowed     */
/*                                                                          */
/* Description:                                                             */
/*   This routine waits at the port's  port number (as assigned in bind)    */
/*   for connection requests from a remote host.                            */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   For a TCP port the function returns 0 if port number is valid and      */
/*   -1 if it is not.                                                       */
/*   For a UDP port the function returns -1.                                */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int listen(int socket, int backlog)                 /*__fn__*/
{
#if (INCLUDE_TCP)   
PANYPORT pport;
#endif

    DEBUG_LOG("listen() - socket = ", LEVEL_3, EBS_INT1, socket, 0);
    DEBUG_LOG("listen - socket = ", LEVEL_3, EBS_INT1, socket, 0); 
    
    RTIP_API_ENTER(API_LISTEN)

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        return(set_errno(ENOTSOCK));
    }

    if (pport->port_type == TCPPORTTYPE) 
    {
        /* error if already listening   */
        if ( (((PTCPPORT)pport)->next_slave) ||
             !tc_cmp4(((PTCPPORT)pport)->out_ip_temp.ip_dest, ip_nulladdr, 4) )
        {
            return(set_errno(EISCONN));
        }

        return(tc_tcp_listen((PTCPPORT)pport, backlog, FALSE));
    }

    return(set_errno(EOPNOTSUPPORT));
}
#endif

/* ********************************************************************      */
/* connect() - Establish a connection to a remote host                       */
/*                                                                           */
/* Summary:                                                                  */
/*   #include "socket.h"                                                     */
/*                                                                           */
/*   int connect(socket, addr, addrlen)                                      */
/*     int socket      - Socket returned by socket                           */
/*     PCSOCKADDR addr - address of otherside to connect to                  */
/*     int addrlen     - length of valid info in addr                        */
/*                                                                           */
/* Description:                                                              */
/*   For TCP sockets this routines establishes a TCP virtual socket between  */
/*   the socket and the service provider at the ip address and port number   */
/*   provided in the argument addr.                                          */
/*                                                                           */
/*   For UDP sockets the IP address and port number are copied into the port */
/*   structure and the function returns immediately.                         */
/*                                                                           */
/*   For more details see the RTIP Manual.                                   */
/*                                                                           */
/* Returns:                                                                  */
/*   0 always for UDP. For TCP, 0 if the connection was successfully         */
/*   established or -1 if it was not successful.                             */
/*                                                                           */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set  */
/*   by this function.  For error values returned by this function see       */
/*   RTIP Reference Manual.                                                  */
/*                                                                           */

int connect(int socket, PCSOCKADDR addr, int addrlen) /*__fn__*/
{
PSOCKADDR_IN addr_in;
PANYPORT pport;
int connected;

    
    RTIP_API_ENTER(API_CONNECT)
    
    DEBUG_LOG("connect() - socket = ", LEVEL_3, EBS_INT1, socket, 0);

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

    addr_in = (PSOCKADDR_IN)addr;

#if (INCLUDE_ERROR_CHECKING)
    /* check if an address to connect to was passed in   */
    if (!addr_in 
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
        || !addr_in->sin_addr 
#else
        || !addr_in->sin_addr.s_addr 
#endif
        || !addr_in->sin_port)
        return(set_errno(EDESTADDREQ));

    if (addrlen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE)) 
        return(set_errno(EFAULT));
#else
    ARGSUSED_INT(addrlen);
#endif

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        connected = tc_tcp_connect((PTCPPORT)pport, (PFBYTE)(&addr_in->sin_addr), addr_in->sin_port);
        return(connected);
    }
#endif /* INCLUDE_TCP         */
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        return(tc_udp_connect((PUDPPORT)pport, (PFBYTE)(&addr_in->sin_addr), 
                              addr_in->sin_port));
    }
#endif          
    return(set_errno(EOPNOTSUPPORT));
}

/* ********************************************************************     */
/* bind() -  Bind a socket to a local port number and IP address            */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int bind(socket, addr, addrlen)                                        */
/*     int socket      - Socket returned by socket                          */
/*     PCSOCKADDR addr - IP and port address of myself (port not used for   */
/*                       RAW)                                               */
/*     int addrlen     - length of valid info in addr                       */
/*                                                                          */
/* Description:                                                             */
/*   This routine attaches a specific input port number and IP address      */
/*   to a socket. It is called before accept is called to accept an         */
/*   incoming connection on the port. Server applications will call         */
/*   bind to establish a "well known port" for clients to call on.          */
/*   Bind does not in itself establish any connections.                     */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if bind was successful otherwise -1                                  */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int bind(int socket, PCSOCKADDR addr, int addrlen) /*__fn__*/
{
PSOCKADDR_IN addr_in;
PANYPORT pport;
    
    RTIP_API_ENTER(API_BIND)

    DEBUG_LOG("bind() - socket = ", LEVEL_3, EBS_INT1, socket, 0);

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

    addr_in = (PSOCKADDR_IN)addr;

#if (INCLUDE_ERROR_CHECKING)
    if ( !addr_in || (addrlen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE)) )
        return(set_errno(EFAULT));
#else
    ARGSUSED_INT(addrlen);
#endif

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        return(tc_tcp_bind((PTCPPORT) pport, (PFBYTE)(&addr_in->sin_addr), 
                           addr_in->sin_port, FALSE));
    }
            
    else 
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
          (pport->port_type == RAWPORTTYPE) )
    {
        return(tc_udp_bind((PUDPPORT) pport, (PFBYTE)(&addr_in->sin_addr), 
                           addr_in->sin_port));
    }
    else
#endif
        return(set_errno(EOPNOTSUPPORT));

}   
    
#if (INCLUDE_TCP)
/* ********************************************************************       */
/* accept() - Accept a connection from any remote host (TCP only)             */
/*                                                                            */
/* Summary:                                                                   */
/*   #include "socket.h"                                                      */
/*                                                                            */
/*   int accept(socket, addr, addrlen)                                        */
/*     int socket     - Socket returned by socket & bound in                  */
/*                      bind & set to LISTEN in listen                        */
/*     PSOCKADDR addr - address of otherside connected to                     */
/*     PFINT addrlen  - pointer to length of valid info in addr               */
/*                                                                            */
/* Description:                                                               */
/*   This routine waits at the port's port number and IP address (as assigned */
/*   in bind) for a connection request from a remote host.                    */
/*   For a TCP port the function returns when the virtual socket has been     */
/*   established.                                                             */
/*                                                                            */
/*   This function is for TCP only.                                           */
/*                                                                            */
/*   For more details see the RTIP Manual.                                    */
/*                                                                            */
/* Returns:                                                                   */
/*   For TCP returns port number of established connection or -1 if timeout   */
/*   or other error occurs.                                                   */
/*   For UDP returns -1.                                                      */
/*                                                                            */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set   */
/*   by this function.  For error values returned by this function see        */
/*   RTIP Reference Manual.                                                   */
/*                                                                            */

int accept(int socket, PSOCKADDR addr, PFINT addrlen)   /*__fn__*/
{
PANYPORT pport;         /* master socket */
#if (INCLUDE_TCP)
PSOCKADDR_IN addr_in;
PTCPPORT tcp_port;
int  src_port;  
#endif

    RTIP_API_ENTER(API_ACCEPT)

    DEBUG_LOG("accept() - socket = ", LEVEL_3, EBS_INT1, socket, 0);

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

    if (pport->port_type == TCPPORTTYPE) 
    {
#if (INCLUDE_ERROR_CHECKING)
        /* check validity of addr and addrlen   */
        if (!addr || *addrlen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE))
            return(set_errno(EFAULT));
#endif

        tcp_port = tc_tcp_accept((PTCPPORT)pport);

        if (tcp_port)
        {
            addr_in = (PSOCKADDR_IN)addr;

            /* Get address connected to                                 */
            /* NOTE: Going through an intermediate variable (src_port)  */
            /*       solves byte order problem                          */
            tc_tcp_pk_peer_address((PTCPPORT)tcp_port, (PFBYTE)(&addr_in->sin_addr), 
                                   (PFINT)&src_port);
            addr_in->sin_port = (word) src_port;

            *addrlen = sizeof(struct sockaddr_in);   
            return(tcp_port->ap.ctrl.index);   /* return socket number */
        }
        else
            return(-1);   /* errno set by tc_tcp_accept */
    }

    return(set_errno(EOPNOTSUPPORT));
}
#endif
        
/* ********************************************************************         */
/* getsockopt() - Extract an option associated with an UDP or TCP socket port.  */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "socket.h"                                                        */
/*                                                                              */
/*   int getsockopt (socket, level, option_name, option_value, optionlen)       */
/*     int socket                - socket returned by socket                    */
/*     int level                 - protocol level (SOL_SOCKET, IPPROTO_IP)      */
/*     int option_name           - options                                      */
/*     PFCHAR option_value       - buffer which contains values for option      */
/*     PFINT optionlen           - length of option_value                       */
/*                                                                              */
/* Description:                                                                 */
/*   Allows application extact the following options associated with a socket:  */
/*      See setsockopt() for options supported                                  */
/*                                                                              */
/*   For more details see the RTIP Manual.                                      */
/*                                                                              */
/* Returns:                                                                     */
/*   0 if option set successfully.                                              */
/*   -1 if an error occurred.                                                   */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set     */
/*   by this function.  For error values returned by this function see          */
/*   RTIP Reference Manual.                                                     */
/*                                                                              */

int getsockopt(int socket, int level, int option_name,     /*__fn__*/
               PFCHAR option_value, PFINT optionlen)    /*__fn__*/
{
PANYPORT pport = 0;
#if (INCLUDE_TCP)
PLINGER  plngr;
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
int iface_no;
PIFACE pi;
PINADDR pin_addr;
#endif

    RTIP_API_ENTER(API_GETSOCKOPT)

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

#if (INCLUDE_ERROR_CHECKING)
    if ( ((unsigned short)level != SOL_SOCKET) &&
         ((unsigned short)level != IPPROTO_IP) ) 
        return(set_errno(EINVAL));
#else
    ARGSUSED_INT(level);
#endif

#if (INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW)
    if (option_name == SO_TOS)
    {
        if (*optionlen != sizeof(byte))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        if (pport->options & SOO_TOS)
            *((PFBYTE)option_value) = pport->tos;
        else
            *((PFBYTE)option_value) = 0;
        return(0);
    }

    if ( (option_name == SO_RCV_TIMEO) || (option_name == SO_SEND_TIMEO) )
    {
        if (*optionlen != sizeof(struct timeval))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        if (pport->options & SOO_RCV_TIMEO)
        {
            STRUCT_COPY(option_value, pport->recv_timeval);
        }
        else if (pport->options & SOO_SEND_TIMEO)
        {
            STRUCT_COPY(option_value, pport->send_timeval);
        }
        else
            *((PFBYTE)option_value) = 0;
        return(0);
    }

#endif  /* INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW */

    /* ******   */
#if (INCLUDE_TCP)  
    if (pport->port_type == TCPPORTTYPE)
    {
        switch (option_name)
        {
            case SO_TCP_NO_COPY:
            case SO_REUSEADDR:
            case SO_REUSESOCK:
            case SO_KEEPALIVE:
            case SO_NAGLE:
            case SO_DELAYED_ACK:
            case SO_TCP_STREAM:
#if (INCLUDE_TCP_TIMESTAMP)
            case (int)SO_TCP_TIMESTAMP:
#endif
#if (INCLUDE_SSL)
            case SO_SECURE_SOCKET:
#endif
                if (*optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                if (pport->options & option_name)
                    *((PFINT)option_value) = 1;
                else
                    *((PFINT)option_value) = 0;
                return(0);

            case SO_LINGER:
                if (*optionlen != sizeof(struct linger))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                plngr = (PLINGER)option_value;
                plngr->l_linger = pport->linger_secs;  

                if (pport->options & SO_LINGER)
                    plngr->l_onoff = TRUE;
                else
                    plngr->l_onoff = FALSE;
                return(0);

            case SO_SELECT_SIZE:
                *(PFINT)option_value = ((PTCPPORT)pport)->select_size;
                return(0);
        }
    }
    else
#endif

    /* ******   */
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        switch (option_name)
        {
            case SO_UDPCKSUM_OUT:
            case SO_UDPCKSUM_IN:
            case SO_REUSEADDR:
                if (pport->port_type == UDPPORTTYPE) 
                {
                    if (*optionlen != sizeof(int))
                        return(set_errno(EFAULT));

                    if (!option_value)
                        return(set_errno(EFAULT));

                    if (pport->options & option_name)
                        *((PFINT)option_value) = 1;
                    else
                        *((PFINT)option_value) = 0;
                    return(0);
                }
                break;

            case SO_MAX_UDP_QUE:
                if (*optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                if (pport->options & option_name)
                    *((PFINT)option_value) = ((PUDPPORT)pport)->max_udp_que;
                else
                    *((PFINT)option_value) = -1;
                return(0);

            case IP_MULTICAST_LOOP:
                if (*optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                if (pport->options & SO_MCAST_LOOP)
                    *((PFINT)option_value) = 1;
                else
                    *((PFINT)option_value) = 0;
                return(0);

            case IP_MULTICAST_TTL:
                if (*optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                *((PFINT)option_value) = pport->ip_ttl;
                    return(0);

            case IP_MULTICAST_IF:
                if (*optionlen != sizeof(struct in_addr))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

              
                iface_no = default_mcast_iface;
                pi = tc_ino2_iface(iface_no, DONT_SET_ERRNO);
                if (pi)
                {       
                    pin_addr = (PINADDR)option_value;
                    /* save address in network byte order   */
                    pin_addr->s_un.s_un_b.s_b1 = pi->addr.my_ip_addr[0];
                    pin_addr->s_un.s_un_b.s_b2 = pi->addr.my_ip_addr[1];
                    pin_addr->s_un.s_un_b.s_b3 = pi->addr.my_ip_addr[2];
                    pin_addr->s_un.s_un_b.s_b4 = pi->addr.my_ip_addr[3];

                    return(0);
                }
                return(set_errno(EBADIFACE));
        }       /* end of switch */
    }
#endif

    /* ******   */
#if (INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) ||
         (pport->port_type == TCPPORTTYPE) )
    {
        if (option_name == SO_IP_TTL)
        {
            if (*optionlen != sizeof(int))
                return(set_errno(EFAULT));

            if (!option_value)
                return(set_errno(EFAULT));

            *((PFINT)option_value) = pport->ip_ttl;
            return(0);
        }

        else
            return(set_errno(ENOPROTOOPT));
    }
#endif

    /* ******   */
    return(set_errno(EOPNOTSUPPORT));
}

/* ********************************************************************            */
/* setsockopt() - Change an option associated with an UDP or TCP socket port.      */
/*                                                                                 */
/* Summary:                                                                        */
/*   #include "socket.h"                                                           */
/*                                                                                 */
/*   int setsockopt (socket, level, option_name, option_value, optionlen)          */
/*     int socket                - socket returned by socket                       */
/*     int level                 - protocol level (SOL_SOCKET, IPPROTO_IP)         */
/*     int option_name           - options                                         */
/*     PFCCHAR option_value      - buffer which contains values for option         */
/*     int optionlen             - length of option_value                          */
/*                                                                                 */
/* Description:                                                                    */
/*   Allows application set the following options associated with a socket:        */
/*      TCP ONLY:                                                                  */
/*          SO_KEEPALIVE    - sending keepalive packets (default is FALSE)         */
/*          SO_LINGER       - linger on close                                      */
/*          SO_TCP_NO_COPY  - packet vs window mode                                */
/*          SO_REUSEADDR    - allow local address reuse                            */
/*          SO_NAGLE        - NAGLE algoritm                                       */
/*          SO_DELAYED_ACK  - delay sending ACK                                    */
/*          SO_STREAM       - do not send unless can send MSS number of bytes      */
/*          SO_TCP_STREAM   - send only full size packets (i.e. mss)               */
/*          SO_TCP_TIMESTAMP- send TIMESTAMP TCP option in SYNC msg                */
/*          SO_SELECT_SIZE  - select wakeup when specified room in                 */
/*                            window is available                                  */
/*          SO_SECURE_SOCKET  - Use secure socket to transmit data                 */
/*                            - (INCLUDE_SSL must be set)                          */
/*          SO_RCV_TIMEO      - set receive timeout                                */
/*          SO_SEND_TIMEO     - Set send timeout                                   */
/*                                                                                 */
/*      UDP ONLY:                                                                  */
/*          SO_UDPCKSUM_IN  - enable/disable UDP checksum for input                */
/*          SO_UDPCKSUM_OUT - enable/disable UDP checksum for output               */
/*          SO_MAX_UDP_QUE  - number of packets allowed to queue on a UDP          */
/*                            socket                                               */
/*          IP_ADD_MEMBERSHIP  - join a multicast group                            */
/*          IP_DROP_MEMBERSHIP - leave a multicast group                           */
/*          IP_MULTICAST_LOOP  - enable/disable loopback of multicast datagrams    */
/*          IP_MULTICAST_TTL   - set IP TTL of outgoing multicast datagrams        */
/*          IP_MULTICAST_IF    - set the default interface for multicast datagrams */
/*                                                                                 */
/*      TCP and UDP:                                                               */
/*          SO_802_2        - Send 802.2 packets                                   */
/*          SO_IP_TTL       - time to live                                         */
/*          SO_TOS          - Type of Service (sent in IP header)                  */
/*          SO_RCV_TIMEO    - set receive timeout                                  */
/*          SO_SEND_TIMEO   - Set send timeout                                     */
/*                                                                                 */
/*   For more details see the RTIP Manual.                                         */
/*                                                                                 */
/* Returns:                                                                        */
/*   0 if option set successfully.                                                 */
/*   -1 if an error occurred.                                                      */
/*                                                                                 */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set        */
/*   by this function.  For error values returned by this function see             */
/*   RTIP Reference Manual.                                                        */
/*                                                                                 */

int setsockopt(int socket, int level, int option_name,      /*__fn__*/
               PFCCHAR option_value, int optionlen)         /*__fn__*/
{
PANYPORT pport = 0;
#if (INCLUDE_TCP)
PLINGER  plngr;
#endif
#if (INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW)
PTIMEVAL tval;
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
#if (INCLUDE_IGMP || INCLUDE_IGMP_V2 )
PIPMREQ  pip_mreq;
dword    mg_address;
PFBYTE   p;
byte     mclist[4*IP_ALEN];
byte     ip_mcast_addr[IP_ALEN];
#endif
byte     interf_addr[IP_ALEN];
PINADDR  pin_addr;
dword    interface_address;
int      use_default_mcast;
int      iface_no;
#endif      /* UDP || RAW */

    RTIP_API_ENTER(API_SETSOCKOPT)

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

#if (INCLUDE_ERROR_CHECKING)
    if ( ((unsigned short)level != SOL_SOCKET) &&
         ((unsigned short)level != IPPROTO_IP) ) 
        return(set_errno(EINVAL));
#else
    ARGSUSED_INT(level);
#endif

#if (INCLUDE_TCP)
    if (option_name == SO_FREE_WITH_INPUT)
    {
        if (optionlen != sizeof(int))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        if (*((PFINT)option_value) == 0)
        {
            pport->options &= ~SOO_FREE_WITH_INPUT;
        }
        else
        {
            pport->options |= SOO_FREE_WITH_INPUT;
        }
        return(0);
    }
#if (INCLUDE_SSL)       /* __st__ 2002.08.02 */
    if (option_name == SO_SECURE_SOCKET)
    {
        /* turn option on  if secure context pointer (option_value) has been initialized   */
        /* turn option off if secure context pointer (option_value) has been set to NULL   */
        if (!optionlen)
        {
            pport->options &= ~SOO_SECURE_SOCKET;
        }
        else
        {
            pport->options |= SOO_SECURE_SOCKET;

            /* pass the secure context to the port struct becuase   */
            /* the context should only be initialized once          */
            /* should be freed at the application layer             */
            /* not every time you close a socket                    */
            ((PTCPPORT)pport)->sctx = (void *)option_value;
        }
        return(0);
    }
#endif /* INCLUDE_SSL */
#endif /* INLCUDE_TCP */

#if (INCLUDE_802_2)
    /* tbd: getsockopt(), merge with other code   */
    if (option_name == SO_802_2)
    {
        if (optionlen != sizeof(int))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        if (*((PFINT)option_value) == 0)
        {
            pport->options &= ~SOO_802_2;
        }
        else
        {
            pport->options |= SOO_802_2;
        }
        return(0);
    }
#endif

#if (INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW)
    if (option_name == SO_TOS)
    {
        if (optionlen != sizeof(byte))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        pport->tos = *((PFBYTE)option_value);

        if (*((PFBYTE)option_value) == 0)
            pport->options &= ~SOO_TOS;
        else
            pport->options |= SOO_TOS;

        return(0);
    }

    if (option_name == SO_ERROR)
    {
        if (optionlen != sizeof(byte))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        /* get and clear errno   */
        *((PFBYTE)option_value) = xn_getlasterror();
        set_errno(0);
    }

    if ( (option_name == SO_RCV_TIMEO) || (option_name == SO_SEND_TIMEO) )
    {
        if (optionlen != sizeof(struct timeval))
            return(set_errno(EFAULT));

        if (!option_value)
            return(set_errno(EFAULT));

        tval = (PTIMEVAL)option_value;
        if (option_name == SO_RCV_TIMEO)
        {
            pport->options |= SOO_RCV_TIMEO;
            STRUCT_COPY(pport->recv_timeval, *tval);
        }
        else if (option_name == SO_SEND_TIMEO)
        {
            pport->options |= SOO_SEND_TIMEO;
            STRUCT_COPY(pport->send_timeval, *tval);
        }
        else
            *((PFBYTE)option_value) = 0;
        return(0);
    }

#endif  /* INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW */

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        /* if keepalive requested, set flag so timeout knows to send   */
        /* keepalives                                                  */
        /* NOTE: will always respond to keepalives even if option not  */
        /*       specified                                             */
        switch (option_name)
        {
            case SO_TCP_NO_COPY:
                /* cannot change once established   */
                if ( ((PTCPPORT)pport)->state != TCP_S_ALLOCED )
                    return(set_errno(EOPNOTSUPPORT));  /* tbd - errno? */
#if (!INCLUDE_TCP_NO_COPY)
                /* if COPY is only option, can't turn on NO_COPY   */
                if (*((PFINT)option_value) == 1)
                    return(set_errno(EOPNOTSUPPORT));  /* tbd - errno? */
#endif      /* !INCLUDE_TCP_NO_COPY */
#if (!INCLUDE_TCP_COPY)
                /* if NO_COPY is only option, can't turn on COPY   */
                if (*((PFINT)option_value) == 0)
                    return(set_errno(EOPNOTSUPPORT));  /* tbd - errno? */
#endif      /* !INCLUDE_TCP_COPY */

            case SO_REUSEADDR:
            case SO_REUSESOCK:
            case SO_KEEPALIVE:
            case SO_NAGLE:
            case SO_DELAYED_ACK:
            case SO_TCP_STREAM:
#if (INCLUDE_TCP_TIMESTAMP)
            case (int)SO_TCP_TIMESTAMP:
#endif
                if (optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                if (*((PFINT)option_value) == 0)
                {
                    pport->options &= ~option_name;

                    if (option_name == SO_TCP_STREAM)
                    {
                        /* send any data which has not been sent    */
                        tc_tcpsend(pport->iface, (PTCPPORT)pport, NO_DCU_FLAGS, 
                                   NORMAL_DATA_ONLY, NO_TCP_FLAGS); 
                    }
                }
                else
                {
                    /* set to default values   */
                    pport->options |= option_name;

                    if (option_name == SO_KEEPALIVE)
                    {
                        ((PTCPPORT)pport)->ka_interval = CFG_KA_INTERVAL;
                        ((PTCPPORT)pport)->ka_retry    = CFG_KA_RETRY;
                        ((PTCPPORT)pport)->ka_tmo      = CFG_KA_TMO;
                    }
                }
                return(0);

            case SO_LINGER:
                if (optionlen != sizeof(struct linger))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                plngr = (PLINGER)option_value;
                /* graceful close = l_onoff=0, linger_secs=0   */
                pport->linger_secs = plngr->l_linger;

                if (plngr->l_onoff)
                {
                    pport->options |= SO_LINGER;
                }
                else
                {
                    pport->options &= ~SO_LINGER;
                }
                return(0);

            case SO_SELECT_SIZE:
                ((PTCPPORT)pport)->select_size = (word)(*(PFINT)option_value);
                return(0);
        }
     }
    else
#endif

#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        switch (option_name)
        {
            case SO_UDPCKSUM_OUT:
            case SO_UDPCKSUM_IN:
            case SO_REUSEADDR:
                if (pport->port_type == UDPPORTTYPE) 
                {
                    if (optionlen != sizeof(int))
                        return(set_errno(EFAULT));

                    if (!option_value)
                        return(set_errno(EFAULT));

                    if (*((PFINT)option_value) == 0)
                        pport->options &= ~option_name;
                    else
                        pport->options |= option_name;
                    return(0);
                }
                break;

            case SO_MAX_UDP_QUE:
                if (optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                if (*((PFINT)option_value) > 0)
                {
                    pport->options |= SO_MAX_UDP_QUE;
                    ((PUDPPORT)pport)->max_udp_que = *((PFINT)option_value);
                }
                else
                {
                    pport->options &= ~SO_MAX_UDP_QUE;
                }
                return(0);

#if (INCLUDE_IGMP || INCLUDE_IGMP_V2)
            case IP_ADD_MEMBERSHIP:
            case IP_DROP_MEMBERSHIP:
                if (optionlen != sizeof(struct ip_mreq))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                pip_mreq = (PIPMREQ)option_value;
                
                /* addresses should come in as network byte order, therefore   */
                /* convert the group address to host order to check            */
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
                mg_address = net2hl(pip_mreq->imr_multiaddr.s_un.s_addr);
#else
                mg_address = net2hl(pip_mreq->imr_multiaddr.s_addr);
#endif
                if ( (mg_address < MIN_VALID_MCADDR) ||  /* out of range for  */
                     (mg_address > MAX_VALID_MCADDR))    /* multicast */
                    return(set_errno(EFAULT));

#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
                interface_address = net2hl(pip_mreq->imr_interface.s_un.s_addr);
#else
                interface_address = net2hl(pip_mreq->imr_interface.s_addr);
#endif
          
                /* convert the addresses to 4 byte arrays   */
                ip_mcast_addr[0] = pip_mreq->imr_multiaddr.s_un.s_un_b.s_b1;
                ip_mcast_addr[1] = pip_mreq->imr_multiaddr.s_un.s_un_b.s_b2;
                ip_mcast_addr[2] = pip_mreq->imr_multiaddr.s_un.s_un_b.s_b3;
                ip_mcast_addr[3] = pip_mreq->imr_multiaddr.s_un.s_un_b.s_b4;

                interf_addr[0] = pip_mreq->imr_interface.s_un.s_un_b.s_b1;
                interf_addr[1] = pip_mreq->imr_interface.s_un.s_un_b.s_b2;
                interf_addr[2] = pip_mreq->imr_interface.s_un.s_un_b.s_b3;
                interf_addr[3] = pip_mreq->imr_interface.s_un.s_un_b.s_b4;

                /* set up the multicast list   */
                p = (PFBYTE)mclist;
                SETMCASTADDR(p, 0, ip_mcast_addr[0], ip_mcast_addr[1],
                             ip_mcast_addr[2], ip_mcast_addr[3])
          
                /* get interface number   */
                if (interface_address == INADDR_ANY)
                    /* this means use the default multicast interface   */
                    iface_no = default_mcast_iface;
                else
                    iface_no = get_local_ifaceno(interf_addr);

                if (iface_no == -1)
                {
                    DEBUG_ERROR("setsockopt: invalid interface or no default_mcast_iface", 
                        NOVAR, 0, 0);
                    return( set_errno( EFAULT ) );
                }

                /* now actually join or leave the group   */
                if (option_name == IP_ADD_MEMBERSHIP)
                {
                    if (!tc_interface_mcast(iface_no, p, 1, ADD_ENTRY) )
                        return -1;

                    /* 0 means not gw, RT_USEIFACEMETRIC means use default   */
                    /* for metric defined for the interface                  */
                    if (xn_rt_add(p, ip_ffaddr, 0, RT_USEIFACEMETRIC, 
                                  iface_no, RT_INF))
                    {
                        DEBUG_ERROR("setsockopt: xn_rt_add failed", NOVAR, 0, 0);
                        return(-1);
                    }
                }
                else
                    if (!tc_interface_mcast(iface_no, p, 1, DELETE_ENTRY))
                       return -1;
                return(0);
#endif      /* IGMP || IGMP_V2 */
        
            case IP_MULTICAST_LOOP:
                if (optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                if (*((PFINT)option_value) == 0)
                    pport->options &= ~SO_MCAST_LOOP;
                else if (*((PFINT)option_value) == 1)
                    pport->options |= SO_MCAST_LOOP;
                else 
                    return(set_errno(EFAULT));  /* value must be 0 or 1 */
                return(0);

            case IP_MULTICAST_TTL:
                if (optionlen != sizeof(int))
                    return(set_errno(EFAULT));

                /* check valid parameter and range   */
                if ( !option_value || (*((PFINT)option_value) < 0) || 
                     (*((PFINT)option_value) > 0xff) )
                    return(set_errno(EFAULT));

                pport->ip_ttl = *((PFINT)option_value);
                return(0);

            case IP_MULTICAST_IF:
                if (optionlen != sizeof(struct in_addr))
                    return(set_errno(EFAULT));

                if (!option_value)
                    return(set_errno(EFAULT));

                pin_addr = (PINADDR)option_value;

                /* addresses should come in as network byte order, therefore   */
                /* convert the interface address to host order                 */
#if !CFG_UNIX_COMPLIANT_SOCK_STRUCTS           
                interface_address = net2hl(pin_addr->s_un.s_addr);
#else
                interface_address = net2hl(pin_addr->s_addr);
#endif

                if (interface_address == INADDR_ANY)  /*don't use default multicast interface */
                {
                    use_default_mcast = 0;
                    return( xn_interface_opt(-1, IO_DEFAULT_MCAST,
                                             (PFCCHAR)&use_default_mcast,
                                             sizeof(int)) );
                }

                else
                {
                    /* convert the addresses to 4 byte arrays   */
                    interf_addr[0] = pin_addr->s_un.s_un_b.s_b1;
                    interf_addr[1] = pin_addr->s_un.s_un_b.s_b2;
                    interf_addr[2] = pin_addr->s_un.s_un_b.s_b3;
                    interf_addr[3] = pin_addr->s_un.s_un_b.s_b4;

                    /* get interface number   */
                    iface_no = get_local_ifaceno(interf_addr);

                    use_default_mcast = 1;
                    return( xn_interface_opt(iface_no, IO_DEFAULT_MCAST,
                                             (PFCCHAR)&use_default_mcast,
                                             sizeof(int)));

                }
        }   /* end of switch */
    }
#endif

    /* ******   */
#if (INCLUDE_TCP || INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) ||
         (pport->port_type == TCPPORTTYPE) )
    {
        if (option_name == SO_IP_TTL)
        {
            if (optionlen != sizeof(int))
                return(set_errno(EFAULT));

            /* check valid parameter and range   */
            if ( !option_value || (*((PFINT)option_value) < 0) || 
                 (*((PFINT)option_value) > 0xff) )
                return(set_errno(EFAULT));

            pport->ip_ttl = *((PFINT)option_value);
            return(0);
        }

        else
            return(set_errno(ENOPROTOOPT));
    }
#endif

    return(set_errno(EOPNOTSUPPORT));
}


/* ********************************************************************     */
/* ioctlsocket() - turns blocking on or off                                 */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int ioctlsocket(socket, cmd, argp)                                     */
/*     int socket                    - socket number returned by socket()   */
/*     long cmd                      - command to be performed              */
/*                                     (FIONBIO, FIONREAD, FIONWRITE)       */
/*     unsigned long KS_FAR *argp  - argument for the command               */
/*                                                                          */
/* Description:                                                             */
/*   FIONBIO: if the value argp points to is 1, the socket is set           */
/*   to non-blocking mode.  If it is 0, the socket is set to blocking       */
/*   mode which is the default.                                             */
/*                                                                          */
/*   FIONREAD: the number of bytes available to read is stored in           */
/*   memory at the address specified in argp.  For TCP, it is the           */
/*   number of bytes in the input window.  For UDP and RAW, it is           */
/*   the number of bytes in the first packet on the UDP input               */
/*   exchange.                                                              */
/*                                                                          */
/*   FIONWRITE: the number of bytes available to write (without             */
/*   blocking) is stored in memory at the address specified in argp.        */
/*   For TCP, it is the output window size minus the number of bytes        */
/*   queued.  For UDP, it is the maximum size of a send, i.e. max           */
/*   fragment size if fragments are enabled or maximum amount of data       */
/*   which will fit in one packet if fragmentation is disabled.             */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns 0 if successful -1 if error.                                   */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int ioctlsocket(int socket, long cmd, unsigned long KS_FAR *argp)  /*__fn__*/
{
PANYPORT pport;
#if (!INCLUDE_FRAG)
int      size;
#endif

    RTIP_API_ENTER(API_IOCTLSOCKET)

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

#if (INCLUDE_ERROR_CHECKING)
    if ( (pport->port_type != TCPPORTTYPE) &&
         (pport->port_type != UDPPORTTYPE) &&
         (pport->port_type != RAWPORTTYPE) )
        return(set_errno(EINVAL));
#endif

    switch (cmd)
    {
    case FIONBIO:
        if (*argp)
            pport->options &= ~IO_BLOCK_OPT;   /* enable non-blocking mode */
        else
        {
            pport->options |= IO_BLOCK_OPT;    /* disable non-blocking mode */
        }

        return(0);

    case FIONREAD:
#if (INCLUDE_TCP)
        if (pport->port_type == TCPPORTTYPE) 
        {
            *argp = ((PTCPPORT)pport)->in.contain;
            return(0);
        }
        else
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
        if ( (pport->port_type == UDPPORTTYPE) ||
              (pport->port_type == RAWPORTTYPE) )
        {
            *argp = os_udp_first_pkt_size((PUDPPORT)pport);
            return(0);
        }
#endif
        return(set_errno(ETNOSUPPORT));

    case FIONWRITE:
#if (INCLUDE_TCP)
        if (pport->port_type == TCPPORTTYPE) 
        {
            *argp = ((PTCPPORT)pport)->out.window_size - 
                    ((PTCPPORT)pport)->out.contain;
            return(0);
        }
        else
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
        {
            if ( (pport->port_type == UDPPORTTYPE) ||
                 (pport->port_type == RAWPORTTYPE) )
            {
#if (INCLUDE_FRAG)
                *argp = 0x1ffful*8ul;  /* + (dword)pi->addr.mtu;   */
                                                    /* 1fff = max offset in    */
                                                    /* incoming fragment       */
                if (*argp > max_frag_size)          /* limit by what DHCP */
                    *argp = max_frag_size;          /* says size is */
#else
                TOTAL_IP_HLEN_SIZE(size, pport, (PIFACE)0, (DCU)0)
                *argp = CFG_MAX_PACKETSIZE - size;
                if (pport->port_type == UDPPORTTYPE)
                    *argp -= UDP_HLEN_BYTES;
#endif
                return(0);
            }
        }
#endif      /* UDP or RAW */
        return(set_errno(ETNOSUPPORT));
    }
    return(set_errno(EINVAL));
}

#if (INCLUDE_SELECT)
/* ********************************************************************   */
/* SELECT                                                                 */
/* ********************************************************************   */

/* ********************************************************************    */
/* delete_fd() - deletes an entry from a select list                       */
/*                                                                         */
/* Summary:                                                                */
/*   #include "socket.h"                                                   */
/*                                                                         */
/*   void delete_fd(int entry, PFDSET fd)                                  */
/*     int entry - offset in fd of entry to be deleted from fd list        */
/*     PFDSET fd -  list of sockets for select()                           */
/*                                                                         */
/* Description:                                                            */
/*                                                                         */
/*   Deletes the entry at offset entry from fd which is a list of sockets. */
/*   The entries after the deleted entry are moved up one entry in the     */
/*   list.                                                                 */
/*                                                                         */
/* Returns:                                                                */
/*   Nothing                                                               */
/*                                                                         */

#ifndef USE_NORTEL_SELECT
void delete_fd(int entry, PFDSET fd)           /*__fn__*/
{
int j;

    for (j=entry; j < fd->num_sockets-1; j++)
        fd->sockets[j] = fd->sockets[j+1];
    fd->sockets[fd->num_sockets-1] = -1;
    fd->num_sockets--;
}
#endif  /* USE_NORTEL_SELECT */

/* ********************************************************************                */
/* poll_read_ready() - Determines if entries are ready for reading                     */
/*                                                                                     */
/* Summary:                                                                            */
/*   int poll_read_ready(fread, delete, poll_error)                                    */
/*     PFDSET fread             - list of sockets to be checked if ready for reading   */
/*     RTIP_BOOLEAN delete      - flag which specifies whether unready items should be */
/*                                deleted or not                                       */
/*     RTIP_BOOLEAN KS_FAR *poll_error  - returns TRUE if error detected                      */
/*     PANYPORT root_port       - root port                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Scan fread list of sockets and returns number sockets ready for reading */
/*                                                                           */
/*   If delete is TRUE, the whole list is scanned and entries which are      */
/*   not ready are deleted.                                                  */
/*   If delete is FALSE, no entries are deleted and a 1 is returned when     */
/*   the first ready entry is found                                          */
/*                                                                           */
/* Returns:                                                                  */
/*   If delete is TRUE, returns number of ready entries.  If delete is       */
/*   FALSE, returns 1 if there is any ready entries and 0 if there           */
/*   are none.  If an error is detected, 0 is returned and poll_error is set */
/*   to TRUE.                                                                */
/*   If delete is TRUE, returns number of ready entries.  If delete is       */
/*   FALSE, returns 1 if there is any ready entries and 0 if there           */
/*   are none.  If an error is detected, 0 is returned and poll_error is set */
/*   to TRUE.                                                                */
/*                                                                           */
int poll_read_ready(PFDSET fread, RTIP_BOOLEAN delete_entry,          /*__fn__*/
                    RTIP_BOOLEAN KS_FAR *poll_error, PANYPORT root_port)  /*__fn__*/
{
PANYPORT pport;
int num_ready;
int i, j;
int ready;
#if (INCLUDE_UDP || INCLUDE_RAW)
int is_connected;
#endif

    if (!fread)
        return 0;

    num_ready = 0;
#ifdef USE_NORTEL_SELECT
    for (i=0; i <= fread->max_fd; i++)
#else
    for (i=0; i < fread->num_sockets; i++)
#endif
    {
#ifdef USE_NORTEL_SELECT
        if (!FD_ISSET(i, fread))
            continue;
        pport = sock_to_port(i);
#else
        pport = sock_to_port(fread->sockets[i]);
#endif
        if (!pport)
        {
            *poll_error = TRUE;
            set_errno(ENOTSOCK);
            return(0);
        }

        /* unbind select signal   */
        if (delete_entry)
        {
            /* first find the offset signal for this port   */
            for (j=0; j < CFG_NUM_SELECT_P_SOCK; j++)
            {
                if (pport->ctrl.select_root[j] == root_port)
                    break;
            }
            if (j < CFG_NUM_SELECT_P_SOCK)
            {
                /* unbind the socket   */
                pport->ctrl.select_root[j] = (PANYPORT) 0;
                pport->ctrl.select_flags[j] &= ~READ_SELECT;
#if (DEBUG_SELECT)
                DEBUG_ERROR("poll_read_ready: port, index", EBS_INT2,
                    pport->ctrl.index, j);
#endif
            }
            DEBUG_ASSERT(j < CFG_NUM_SELECT_P_SOCK, 
                "poll_read_ready: root port not found", NOVAR, 0, 0);
        }

        ready = FALSE;
#if (INCLUDE_TCP)
        if (pport->port_type == TCPPORTTYPE)
        {
            /* if master port, check if accept() will block; the               */
            /* socket will not block if there is a port on master's slave list */
            /* which is in a connected state                                   */
            if (((PTCPPORT)pport)->tcp_port_type >= MASTER_NO_LISTENS)
            {
                if (tc_find_accept_port((PTCPPORT)pport, NO_TAKE_SLAVE_OFF))  
                    ready = TRUE;
            }

            /* if not master port, check if read will block;              */
            /* the socket will not block if there is data in input window */
            /* or there is an invalid state or CWAIT state (otherside did */
            /* a close, therefore, no more data will come in)             */
            else if ( ((PTCPPORT)pport)->in.contain           ||

                      /* error but not still connecting   */
                      (!tc_is_read_state((PTCPPORT)pport)     &&
                       ((PTCPPORT)pport)->state != TCP_S_SYNR &&
                       ((PTCPPORT)pport)->state != TCP_S_SYNS )  ||

                      (((PTCPPORT)pport)->state == TCP_S_CWAIT) )
            {
                ready = TRUE;
            }
        }
        else 
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
        if ( (pport->port_type == UDPPORTTYPE)  ||
                  (pport->port_type == RAWPORTTYPE) )
        {
            /* we must check the error situation, too, here
             * and make sure it matches with what's done in
             * recvfrom and poll_exception
             */
            is_connected = (RTIP_BOOLEAN)
                (!tc_cmp4(((PUDPPORT)pport)->ip_connection.ip_dest,
                          ip_nulladdr, IP_ALEN));
            ready = ((is_connected && pport->ctrl.read_status) ||
                     os_udp_pkt_avail((PUDPPORT)pport));
        }
        else
#endif
        {
            *poll_error = TRUE;
            set_errno(EOPNOTSUPPORT);
            return(0);
        }
    
        /* if ready, keep in list and up count of ready sockets;    */
        /* if not ready, take out of sockets list                   */
        if (ready)
        {
            if (delete_entry)
                num_ready++;
            else
            {
                return(1);      /* SPR - why return here */
            }
        }
        else
        {
            if (delete_entry)
            {
#ifdef USE_NORTEL_SELECT
                FD_CLR(i, fread);
#else
                delete_fd(i, fread);
                i--;    /* next loop make sure don't skip next socket since */
                        /* it was moved   */
#endif
            }
        }
    }
    return(num_ready);
            
}

/* ********************************************************************   */
/* poll_write_ready() - Determines if entries are ready for writing       */
/*                                                                        */
/* Summary:                                                               */
/*   int poll_write_ready(fwrite, delete, poll_error)                                   */
/*     PFDSET fwrite           - list of sockets to be checked if ready for writing        */
/*     RTIP_BOOLEAN    delete       - flag which specifies whether unready items should be */
/*                               deleted or not                                            */
/*     RTIP_BOOLEAN KS_FAR *poll_error - returns TRUE if error detected                      */
/*     PANYPORT root_port       - root port                                   */
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   Scan fwrite list of sockets and returns number sockets ready for writing */
/*                                                                            */
/*   If delete is TRUE, the whole list is scanned and entries which are       */
/*   not ready are deleted.                                                   */
/*   If delete is FALSE, no entries are deleted and a 1 is returned when      */
/*   the first ready entry is found                                           */
/*                                                                            */
/* Returns:                                                                   */
/*   If delete is TRUE, returns number of ready entries.  If delete is        */
/*   false, returns 1 if there is any ready entries and 0 if there            */
/*   are none. If an error is detected, 0 is returns and poll_error is set    */
/*   to TRUE.                                                                 */
/*                                                                            */

int poll_write_ready(PFDSET fwrite, RTIP_BOOLEAN delete_entry, /*__fn__*/
                     RTIP_BOOLEAN KS_FAR *poll_error, PANYPORT root_port)    /*__fn__*/
{
PANYPORT pport;
int      num_ready;
int      i, j;
RTIP_BOOLEAN  ready;
#if (INCLUDE_TCP)
word     select_size;
#endif

    if (!fwrite)
        return(0);

    num_ready = 0;
#ifdef USE_NORTEL_SELECT
    for (i=0; i <= fwrite->max_fd; i++)
#else
    for (i=0; i < fwrite->num_sockets; i++)
#endif
    {
#ifdef USE_NORTEL_SELECT
        if (!FD_ISSET(i, fwrite))
            continue;
        pport = sock_to_port(i);
#else
        pport = sock_to_port(fwrite->sockets[i]);
#endif
        if (!pport)
        {
            *poll_error = TRUE;
            set_errno(ENOTSOCK);
            return(0);
        }

        /* unbind select signal   */
        if (delete_entry)
        {
            /* first find the offset signal for this port   */
            for (j=0; j < CFG_NUM_SELECT_P_SOCK; j++)
            {
                if (pport->ctrl.select_root[j] == root_port)
                {
                    break;
                }
            }
            if (j < CFG_NUM_SELECT_P_SOCK)
            {
                /* unbind select signal   */
                pport->ctrl.select_root[j] = (PANYPORT) 0;
                pport->ctrl.select_flags[j] &= ~WRITE_SELECT;
            }
#if (DEBUG_SELECT)
            DEBUG_ERROR("poll_write_ready: port, index", EBS_INT2,
                pport->ctrl.index, j);
#endif
            DEBUG_ASSERT(j < CFG_NUM_SELECT_P_SOCK, 
                "poll_write_ready: root port not found", NOVAR, 0, 0);
            DEBUG_ASSERT(j < CFG_NUM_SELECT_P_SOCK,
                "poll_write_ready: root port not found: ", 
                EBS_INT2, j, CFG_NUM_SELECT_P_SOCK);
            DEBUG_ASSERT(j < CFG_NUM_SELECT_P_SOCK,
               "poll_write_ready: select roots: ",
                DINT2, pport->ctrl.select_root[0], 
                pport->ctrl.select_root[1]);
        }

        ready = FALSE;
#if (INCLUDE_TCP)
        if (pport->port_type == TCPPORTTYPE)
        {
            DEBUG_LOG("select: out.contain, is_write_state: ", LEVEL_3, DINT2, 
                ((PTCPPORT)pport)->out.contain, tc_is_write_state((PTCPPORT)pport));

            /* if connection in progress (used for non-blocking sockets   */
            /* only)                                                      */
            if (pport->port_flags & TCP_CONN_IN_PROG)
            {
                if (pport->port_flags & TCP_CONN_EST)
                {
                    /* if second time called by a call to select(), since   */
                    /* the connection has been established, clear the in    */
                    /* progress flag so the next time select() is called    */
                    /* for the socket, it will check if the port is ready   */
                    /* for writing (instead of if connection established)   */
                    if (delete_entry)
                        pport->port_flags &= ~TCP_CONN_IN_PROG;
                    ready = TRUE;
                }
            }

            /* if select_size is set, this overrides the behavior of         */
            /* select() in that it will be ready when select_size free bytes */
            /* are in output window                                          */
            else if (((PTCPPORT)pport)->select_size)
            {
                /* make sure select size is not greater than window   */
                /* size or will never wake up                         */
                select_size = ((PTCPPORT)pport)->select_size;
                if (select_size > ((PTCPPORT)pport)->out.window_size)
                    select_size = ((PTCPPORT)pport)->out.window_size;

                if ( (((PTCPPORT)pport)->out.contain + select_size <= 
                      ((PTCPPORT)pport)->out.window_size) ||
                     !tc_is_write_state((PTCPPORT)pport) )
                    ready = TRUE;
            }

#if (SELECT_TCP_EMPTY)
            /* the socket will not block if the output window is empty   */
            /* or there is an invalid state                              */
            else if ( (!((PTCPPORT)pport)->out.contain) ||
                 !tc_is_write_state((PTCPPORT)pport) )
                ready = TRUE;
#else   
            /* blocking mode: output window must be empty                 */
            /* non-blocking mode: output window must have some room in it */
            /* or there is an invalid state                               */
            else if ( (!((PTCPPORT)pport)->out.contain &&     /* blocking mode */
                       (pport->options & IO_BLOCK_OPT)) ||

                      (((PTCPPORT)pport)->out.contain < 
                       ((PTCPPORT)pport)->out.window_size &&  /* non-blocking mode  */
                       !(pport->options & IO_BLOCK_OPT)) || 

                      !tc_is_write_state((PTCPPORT)pport) )
                ready = TRUE;
#endif
        }
        else 
#endif              /* INCLUDE_TCP */
        {
#if (INCLUDE_UDP || INCLUDE_RAW)
            if ( (pport->port_type == UDPPORTTYPE) ||
                 (pport->port_type == RAWPORTTYPE) )
            {
                ready = TRUE;     /* UDP is always ready for writing */
            }

            else
#endif          /* UDP or RAW */
            {
                *poll_error = TRUE;
                set_errno(EOPNOTSUPPORT);
                return(0);
            }
        }

        /* if ready, keep in list and up count of ready sockets;    */
        /* if not ready, take out of sockets list                   */
        /* if ready, keep in list and up count of ready sockets;    */
        /* if not ready, take out of sockets list                   */
        if (ready)
        {
            if (delete_entry)
                num_ready++;
            else
            {
                return(1);      /* SPR - why do we return here */
            }
        }
        else
        {
            if (delete_entry)
            {
#ifdef USE_NORTEL_SELECT
                FD_CLR(i, fwrite);
#else
                delete_fd(i, fwrite);
                i--;    /* next loop make sure don't skip next socket since */
                        /* it was moved   */
#endif
            }
        }
    }
    return(num_ready);
            
}

/* ********************************************************************   */
/* poll_exception_ready() - Determines if entries had an error            */
/*                                                                        */
/* Summary:                                                               */
/*   int poll_exception(fexception, delete, poll_error)                           */
/*     PFDSET fexception       - list of sockets to be checked for an error                */
/*     RTIP_BOOLEAN    delete       - flag which specifies whether unready items should be */
/*                               deleted or not                                            */
/*     RTIP_BOOLEAN KS_FAR *poll_error - returns TRUE if error detected   */
/*     PANYPORT root_port       - root port                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Scan fexception list of sockets and returns number sockets with errors. */
/*                                                                           */
/*   If delete is TRUE, the whole list is scanned and entries which have     */
/*   a good error status are deleted.                                        */
/*   If delete is FALSE, no entries are deleted and a 1 is returned when     */
/*   the first entry is found with an error status.                          */
/*                                                                           */
/* Returns:                                                                  */
/*   If delete is TRUE, returns number of ready entries.  If delete is       */
/*   false, returns 1 if there is any ready entries and 0 if there           */
/*   are none. If an error is detected, 0 is returns and poll_error is set   */
/*   to TRUE.                                                                */

/*   */

int poll_exception(PFDSET fexception, RTIP_BOOLEAN delete_entry, RTIP_BOOLEAN KS_FAR *poll_error, PANYPORT root_port) /*__fn__*/
{
#if (1)
int i, j;
int ready;
int num_ready;
PANYPORT pport;

    ARGSUSED_INT(delete_entry);   /* keep compiler happy */
    ARGSUSED_PVOID(root_port);    /* keep compiler happy */

    if (!fexception)
        return 0;

    num_ready = 0;
    for (i=0; i < fexception->num_sockets; i++)
    {
        pport = sock_to_port(fexception->sockets[i]);
        /* first find the offset signal for this port   */
        for (j=0; j < CFG_NUM_SELECT_P_SOCK; j++)
        {
            if (pport->ctrl.select_root[j] == root_port)
            {
                break;
            }
        }       /* end of for loop */

        if (j < CFG_NUM_SELECT_P_SOCK)
        {
            pport->ctrl.select_root[j] = (PANYPORT) 0;
            pport->ctrl.select_flags[j] &= ~EXCEPTION_SELECT;
        }
        DEBUG_ASSERT(j < CFG_NUM_SELECT_P_SOCK, 
            "poll_exception_ready: root port not found", NOVAR, 0, 0);

        num_ready = 0;
        for (i=0; i < fexception->num_sockets; i++)
        {
            pport = sock_to_port(fexception->sockets[i]);

            if (pport->port_type == TCPPORTTYPE)
            {
                ready = !tc_is_read_state((PTCPPORT)pport) &&
                        ((PTCPPORT)pport)->state != TCP_S_SYNR  &&
                        ((PTCPPORT)pport)->state != TCP_S_SYNS;
                if (ready)
                    ready = !tc_is_write_state((PTCPPORT)pport);
            }
            else if ( (pport->port_type == UDPPORTTYPE) ||
                      (pport->port_type == RAWPORTTYPE) )
            {
                ready = FALSE;
            }
            else
            {
                ready = TRUE;
                *poll_error = TRUE;
                set_errno(EOPNOTSUPPORT);
            }
    
            if (!ready)
            {
                if (delete_entry)
                {
                    delete_fd(i, fexception);
                    i--;    /* next loop make sure don't skip next socket since */
                            /* it was moved   */
                    num_ready++;
                }
            }
        }   /* for loop */
    }       /* for loop */
    return(num_ready);
#else
    ARGSUSED_INT(delete_entry);   /* keep compiler happy */

    if (fexception)
    {
        *poll_error = TRUE;
        set_errno(EOPNOTSUPPORT);
    }
    return(0);
#endif
}

/* ********************************************************************    */
/* bind_select_signals() - bind ports to first port for select signalling  */
/*                                                                         */
/* Summary:                                                                */
/*                                                                         */
/* Description:                                                            */
/*   Binds all the sockets in the list to the first port found.  Binding   */
/*   is done by saving pointer to first port in port structure so          */
/*   when select signal is set, it sets the select signal in the first     */
/*   port.  This is needed since select can only block on one signal but   */
/*   it is waiting for event in multiple sockets(ports).                   */
/*   First port is the first socket(port) encountered, i.e. if a port is   */
/*   passed in first_port that remains the first port.  If none is passed, */
/*   then the first port encountered in fd becomes the first port and it   */
/*   is passed back to the caller in first_port.                           */
/*                                                                         */
/* Returns:                                                                */
/*   Returns port binding to.                                              */
/*                                                                         */

PANYPORT bind_select_signals(PFDSET fd, PANYPORT first_port, int flags, int *sel_off)
{
PANYPORT port;
int i, j;

    if (!fd)
        return(first_port);

#ifdef USE_NORTEL_SELECT
    for (i=0; i <= fd->max_fd; i++)
#else
    for (i=0; i < fd->num_sockets; i++)
#endif
    {
#ifdef USE_NORTEL_SELECT
        if (!FD_ISSET(i, fd)) continue;
        port = sock_to_port(i);
#else
        port = sock_to_port(fd->sockets[i]);
#endif
        if (port)
        {
            /* if first port then every other port sent to select will bind   */
            /* to same signal                                                 */
            if (!first_port)
            {
                first_port = port;
            }

            /* find a free select_root   */
            for (j=0; j < CFG_NUM_SELECT_P_SOCK; j++)
            {
                if (port->ctrl.select_root[j] == 0)
                {
                    break;
                }
            }
            if (j >= CFG_NUM_SELECT_P_SOCK)
            {
                DEBUG_ERROR("bind_select_signals: need to increase CFG_NUM_SELECT_P_SOCK",
                    EBS_INT1, CFG_NUM_SELECT_P_SOCK, 0);
                continue;
                    /* tbd - should return 0 - failure   */
                    /* and should clean select_root      */
            }

            /* keep offset of select signal for OS_TEST_SELECT_SIGNAL   */
            if (*sel_off == -1) /* not set yet */
                *sel_off = i;

            /* attach the root port to the port. We will signal the   */
            /* root port when select is complete                      */
            port->ctrl.select_root[j] = first_port;

            /* let os layer know which lists it is on   */
            port->ctrl.select_flags[j] |= flags;
        }
    }
    return(first_port);
}

/* ********************************************************************   */
/* convert timeout (sec, usec) to ticks                                   */
word convert_timeval_to_ticks(PCTIMEVAL timeout, RTIP_BOOLEAN *error)
{
dword    msecptic;
dword    tickinsecs;
dword    tickinusecs;
dword    tickinmsecs;
dword    tmeout;
word     wait_count;

    *error = FALSE;
    if (timeout)
    {
        msecptic = (dword) ks_msec_p_tick();
        tickinsecs = timeout->tv_sec * (dword)ks_ticks_p_sec();
        tickinusecs = 0;
        if (msecptic && timeout->tv_usec)
        {
            tickinmsecs = ((timeout->tv_usec+999) / 1000);
            tickinusecs =  ((msecptic-1) + tickinmsecs) / msecptic;
        }
        tmeout = tickinsecs + tickinusecs;

        /* if overflow (i.e. will not fit in word), invalid timeout value   */
        if (tmeout & 0xffff0000ul)
        {
            DEBUG_ERROR("select: timeout is too large", NOVAR, 0, 0);
            *error = TRUE;
            set_errno(EINVAL);
            return(0);
        }

        /* convert to word   */
        wait_count = (word)(tmeout);

        /* if need to wait at least 1 usec then wait at least 1 tick   */
        if ( (wait_count == 0) && timeout->tv_sec && timeout->tv_usec )
            wait_count = 1;
    }
    else
        wait_count = (word)RTIP_INF;

    return(wait_count);
}
 
/* ********************************************************************             */
/* select() - determines if sockets are ready for reading, writing or error         */
/*                                                                                  */
/* Summary:                                                                         */
/*   int select(fd, fread, fwrite, fexception, timeout)                             */
/*     int fd                - number of sockets in all the lists (NI)              */
/*     PFDSET fread          - list of sockets to be checked if ready               */
/*                             for reading                                          */
/*     PFDSET fwrite         - list of sockets to be checked if ready               */
/*                             for writing                                          */
/*     PFDSET fexception     - list of sockets to be checked for an error           */
/*     PCTIMEVAL timeout     - amount of time to wait for a socket to               */
/*                             meet above criteria                                  */
/*                                                                                  */
/* Description:                                                                     */
/*   Scans fread, fwrite and fexception lists of sockets and returns number         */
/*   sockets ready for reading/accept, writing or had an error status respectively. */
/*   It also deletes entries from read list which are not ready for reading/accept, */
/*   from write list which are not ready for writing and from the exception         */
/*   list which have a good error status.                                           */
/*                                                                                  */
/*   For more details see the RTIP Manual.                                          */
/*                                                                                  */
/* Returns:                                                                         */
/*   Number of sockets ready for reading, writing or have an error status           */
/*   (0 if timeout) or -1 if error.                                                 */
/*                                                                                  */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set         */
/*   by this function.  For error values returned by this function see              */
/*   RTIP Reference Manual.                                                         */
/*                                                                                  */

int select(int fd, PFDSET fread, PFDSET fwrite, PFDSET fexception,  /*__fn__*/
           PCTIMEVAL timeout)                                       /*__fn__*/
{
word     wait_count;
int      ret_val;
PANYPORT pport;
RTIP_BOOLEAN  poll_error;
int      sel_off;
RTIP_BOOLEAN error;

    RTIP_API_ENTER(API_SELECT)

    DEBUG_ASSERT_LOG(timeout==0, "select: timeout sec, usec = ", LEVEL_3, DINT2, 
        timeout->tv_sec, timeout->tv_usec);

    ARGSUSED_INT(fd);      /* keep compiler happy */
    pport = 0;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
        return(set_errno(ENOTINITIALIZED));
#endif

    /* convert time to wait to ticks   */
    wait_count = convert_timeval_to_ticks(timeout, &error);
    if (error)
        return(-1);

    OS_CLAIM_TCP(SELECT1_CLAIM_TCP)
    OS_CLAIM_UDP(SELECT1_CLAIM_UDP)

    poll_error = FALSE;
    ret_val = 0;
    if (fread)
    {
        ret_val += poll_read_ready(fread, FALSE, 
                                   (RTIP_BOOLEAN KS_FAR *)&poll_error,
                                   (PANYPORT)0);
    }
    if (fwrite)
    {
        ret_val += poll_write_ready(fwrite, FALSE, 
                                    (RTIP_BOOLEAN KS_FAR *)&poll_error,
                                    (PANYPORT)0);
    }
    if (fexception)
    {
        ret_val += poll_exception(fexception, FALSE, 
                                  (RTIP_BOOLEAN KS_FAR *)&poll_error,
                                  (PANYPORT)0);
    }

    /* if an error was detected while checking lists, return error   */
    if (poll_error)
    {
        OS_RELEASE_UDP()
        OS_RELEASE_TCP()
        DEBUG_ERROR("select - poll error 1", NOVAR, 0, 0);
        return(-1);     /* poll_xx_xx will set errno if error */
    }

    /* if nothing is ready, need to block until at least 1 becomes ready   */
    if (ret_val == 0)
    {
        sel_off = -1;   /* not set yet */

        /* pport is root port, i is offset of select signal within port   */
        if (fread)
            pport = bind_select_signals(fread, (PANYPORT)0, READ_SELECT, &sel_off);
        if (fwrite)
            pport = bind_select_signals(fwrite, (PANYPORT)pport, WRITE_SELECT, &sel_off);
        if (fexception)
            pport = bind_select_signals(fexception, (PANYPORT)pport, EXCEPTION_SELECT, &sel_off);
    
        if (pport)
        {
            OS_BIND_SELECT_SIGNAL(pport, sel_off);
            OS_CLEAR_SELECT_SIGNAL(pport, sel_off);
        }
    }

    OS_RELEASE_UDP()
    OS_RELEASE_TCP()

    /* if none is ready and there is at least one entry in a list, wait    */
    /* for anyone to be ready                                              */
    /* NOTE: if anyone becomes ready it will signal the same select signal */
    /*       that is attached to the first port, therefore, wait on the    */
    /*       first port's select signal                                    */
    if (ret_val <= 0 && pport)
    {
#if (DEBUG_SELECT)
        DEBUG_ERROR("wait select: out.contain, in.contain = ", EBS_INT2,
            ((PTCPPORT)pport)->out.contain, 
            ((PTCPPORT)pport)->in.contain);
        DEBUG_ERROR("wait select: wait_count = ", EBS_INT1, wait_count, 0);
#endif
        if (!OS_TEST_SELECT_SIGNAL(pport, sel_off, wait_count)) 
        {
            /* just used to eliminate a warning generated by an   */
            /* implicit test in the macro.                        */
            DEBUG_LOG("select - test timed out", LEVEL_3, EBS_INT1, wait_count, 0);
#if (DEBUG_SELECT)
            DEBUG_ERROR("select - test timed out", EBS_INT1, wait_count, 0);
#endif
        }   
#if (DEBUG_SELECT)
        DEBUG_ERROR("select done: port->in.contain, port->out.contain = ", 
            EBS_INT2, ((PTCPPORT)pport)->in.contain, 
            ((PTCPPORT)pport)->out.contain);
#endif
    }

    OS_CLAIM_TCP(SELECT2_CLAIM_TCP)
    OS_CLAIM_UDP(SELECT2_CLAIM_UDP)

    /* count number of ready sockets and delete ones which are not ready   */
    poll_error = FALSE;
    ret_val = 0;
    if (fread)
        ret_val += poll_read_ready(fread, TRUE, 
                                   (RTIP_BOOLEAN KS_FAR *)&poll_error,
                                   pport);
    if (fwrite)
        ret_val += poll_write_ready(fwrite, TRUE, 
                                    (RTIP_BOOLEAN KS_FAR *)&poll_error,
                                    pport);
    if (fexception)
        ret_val += poll_exception(fexception, TRUE, 
                                  (RTIP_BOOLEAN KS_FAR *)&poll_error,
                                  pport);

    OS_RELEASE_UDP()
    OS_RELEASE_TCP()

    /* if an error was detected while checking lists, return error   */
    if (poll_error)
    {
        DEBUG_ERROR("select - poll error 2", NOVAR, 0, 0);
        return(-1);     /* poll_xx_xx will set errno if error */
    }

#if (DEBUG_SELECT)
    DEBUG_ERROR("select returning: ", EBS_INT1, ret_val, 0);
#endif
    return(ret_val);
}

#ifndef USE_NORTEL_SELECT
/* ********************************************************************   */
/* proc_fd_clr() - performs FD_CLR macro which deletes entry from fd list */
/*                                                                        */
/* Summary:                                                               */
/*   void proc_fd_clr(socket, fd)                                         */
/*     int socket     - socket to be deleted from fd list                 */
/*     PFDSET fd      - list which contains entry to be deleted           */
/*                                                                        */
/* Description:                                                           */
/*   Deletes socket from fd list.  Moves all entries after socket up one  */
/*   entry.                                                               */
/*                                                                        */
/* Returns:                                                               */
/*   Nothing                                                              */
/*                                                                        */

void proc_fd_clr(int socket, PFDSET fd)       /*__fn__*/
{
int i;

    for (i=0; i < fd->num_sockets; i++)
    {
        if (fd->sockets[i] == socket)
        {
            /* matches, so take out of socket list   */
            delete_fd(i, fd);
            i--;    /* next loop make sure don't skip next socket since */
                    /* it was moved   */
        }       
    }
}

/* ********************************************************************     */
/* proc_fd_isset() - performs FD_ISSET macro which checks if socket in list */
/*                                                                          */
/* Summary:                                                                 */
/*   int proc_fd_isset(socket, fd)                                          */
/*     int socket     - socket to search for                                */
/*     PFDSET fd      - list which should be scanned                        */
/*                                                                          */
/* Description:                                                             */
/*   Scans list fd to determine if socket is in list.                       */
/*                                                                          */
/* Returns:                                                                 */
/*   Nonzero if socket is in list, otherwise, returns 0.                    */
/*                                                                          */

int proc_fd_isset(int socket, PFDSET fd)          /*__fn__*/
{
int i;

    for (i=0; i < fd->num_sockets; i++)
    {
        if (fd->sockets[i] == socket)
            return(1);
    }
    return(0);
}

/* ********************************************************************     */
/* proc_fd_zero() - performs FD_ZERO macro which clears all entries in list */
/*                                                                          */
/* Summary:                                                                 */
/*   void proc_fd_zero(fd)                                                  */
/*     PFDSET fd - list which should be cleared                             */
/*                                                                          */
/* Description:                                                             */
/*   Deletes all sockets from fd list.                                      */
/*                                                                          */
/* Returns:                                                                 */
/*   Nothing                                                                */
/*                                                                          */

void proc_fd_zero(PFDSET fd)              /*__fn__*/
{
int i;

    for (i=0; i < FD_SETSIZE; i++)
        fd->sockets[i] = -1;
    fd->num_sockets = 0;
}
#endif      /* USE_NORTEL_SELECT */
#endif      /* INCLUDE_SELECT */

/* ********************************************************************     */
/* shutdown() - shutdown reads/writes on a socket                           */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int shutdown(socket_no, how)                                           */
/*      int socket   -  Socket returned by socket                           */
/*      int how      -  Type of operation to disallow                       */
/*                                                                          */
/* Description:                                                             */
/*   If how is 0, receives on the socket are disallowed.  If how is 1,      */
/*   sends on the socket are disallowed.  If how is 2, both sends           */
/*   and receives are disallowed.                                           */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if successful or -1 if not successful.                               */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int shutdown(int socket_no, int how)                       /*__fn__*/
{
PANYPORT pport;
int ret_val;
#if (INCLUDE_TCP)
PTCPPORT tport;
RTIP_BOOLEAN  send_fin;
int      new_state = 0;
#endif

    RTIP_API_ENTER(API_SHUTDOWN)
    DEBUG_LOG("shutdown - socket = ", LEVEL_3, EBS_INT1, socket_no, 0); 

    ret_val = 0;

    pport = api_sock_to_port(socket_no);

    if (!pport)
        return(set_errno(ENOTSOCK));

    /* **************************************************   */
#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        OS_CLAIM_TCP(SHUT_CLAIM_TCP)

        tport = (PTCPPORT)pport;

        /* do write shutdown for TCP socket   */
        if ( ((how == 1) || (how == 2)) && tc_is_write_state(tport) )
        {
            send_fin = FALSE;

            if (tport->state == TCP_S_SYNS)
            {
                trans_state(tport, TCP_S_CLOSED, NOT_FROM_INTERPRET, TRUE);

                /* If connect is waiting signal error   */
                wake_up(tport, NOT_FROM_INTERPRET, 
                        WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ENOTCONN);
            }
            /* state transition since send FIN   */
            else if ( (tport->state == TCP_S_EST) ||
                      (tport->state == TCP_S_SYNR) )
            {
                send_fin = TRUE;
                new_state = TCP_S_FW1;   /* EST=>FW1 */
#if (INCLUDE_POSTMESSAGE)
                tport->ap.post_flags |= GRACEFUL_CLOSE;
                DEBUG_ERROR("set GRACEFUL_CLOSE", 
                    EBS_INT1, tport->ap.ctrl.index, 0);
#endif
            }
            else if (tport->state == TCP_S_CWAIT)
            {
                send_fin = TRUE;
                new_state = TCP_S_LAST;  /* CWAIT=>LAST */
            }
            else
            {
                OS_RELEASE_TCP()
                return(set_errno(ENOTCONN));
            }
                
            if (send_fin)
            {
                if ( (tport->out.nxt + tport->out.contain) > 
                      tport->out.nxt_to_send )
                {
                    tport->ap.port_flags |= WAIT_CLOSE_SEG;
                    DEBUG_LOG("shutdown: set WAIT_CLOSE_SEG", LEVEL_3, NOVAR, 0, 0);
                }
                else
                {
                    /* send FIN but do not set api_close_done, therefore, will   */
                    /* not free resources                                        */
                    tport->state = new_state;
                    ret_val = (int)tc_tcpsend(pport->iface, tport, 
                                        NO_DCU_FLAGS, NORMAL_SEND, TCP_F_FIN);
                }
            }
        }

        /* do read shutdown for TCP socket   */
        if ( (how == 0) || (how == 2) )
        {
            if (!tc_is_read_state(tport))
            {
                OS_RELEASE_TCP()
                return(set_errno(ENOTCONN));
            }
            /* queue data in window but do not ack it, therefore, save   */
            /* current ack value (NOTE: will still ack a FIN)            */
            /* NOTE: if already got a FIN (i.e. in CWAIT) then do not    */
            /* turn off acking                                           */
            if (tport->state != TCP_S_CWAIT)
                tport->in.nxt_to_send = tport->in.nxt;
        }
    }

    /* **************************************************   */
    /* if not TCP or UDP, return error                      */
    else
#endif
    {

        if ( (pport->port_type != UDPPORTTYPE) &&
             (pport->port_type != RAWPORTTYPE) )
        {
            return(set_errno(EOPNOTSUPPORT));  
        }
    }

    /* **************************************************   */
    /* set shutdown flag for TCP and UDP                    */
    switch (how)
    {
        case 0:
            /* shut down receives (reads)   */
            pport->port_flags |= READ_SHUTDOWN;
            break;

        case 2:
            /* shut down both receives (reads) and sends (writes)   */
            pport->port_flags |= READ_SHUTDOWN;

            /* NOTE: fall thru   */

        case 1:
            /* shut down sends (writes)   */
            pport->port_flags |= WRITE_SHUTDOWN;
            break;

        default:
#if (INCLUDE_TCP)
            if (pport->port_type == TCPPORTTYPE)
            {
                OS_RELEASE_TCP()
            }
#endif
            return(set_errno(EINVAL));   /* illegal value for how */
    }

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        OS_RELEASE_TCP()
    }
#endif

    return(ret_val);    
}

/* ********************************************************************     */
/* closesocket() - Closes a socket and sever any connections                */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int closesocket(socket)                                                */
/*      int socket   -  Socket returned by socket                           */
/*                                                                          */
/* Description:                                                             */
/*   This routine shuts down a connection and frees all buffers associated  */
/*   with the connection. For UDP, the connection is dropped immediately.   */
/*   For TCP, if the connection is still established a TCP close connection */
/*   handshake is initiated. If the connection was never established the    */
/*   resources are freed immediately.                                       */
/*                                                                          */
/*   Failure to call closesocket() will eventually lead to  exhaustion of   */
/*   resources.                                                             */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   0 if successful or -1 if not successful.                               */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int closesocket(int socket_no)                   /*__fn__*/
{
PANYPORT pport;

    RTIP_API_ENTER(API_CLOSESOCKET)
    DEBUG_LOG("closesocket = ", LEVEL_3, EBS_INT1, socket_no, 0); 

    pport = api_sock_to_port(socket_no);
    if (!pport)
        return(set_errno(ENOTSOCK));

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
        return(tc_tcp_close((PTCPPORT)pport));

    else 
#endif
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        tc_udp_close((PUDPPORT)pport);
        return(0);
    }

    else
#endif
        return(set_errno(EOPNOTSUPPORT));
}
    
    
/* ********************************************************************           */
/* recv()  - Receive data at a socket.                                            */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   int recv(socket, buffer, buflen, flags)                                      */
/*     int socket              - Socket returned by socket                        */
/*     PFCHAR buffer           - Buffer to receive to                             */
/*     int buflen              - Maximum bytes to receive                         */
/*     int flags               - control flags (NI)                               */
/*                                                                                */
/* Description:                                                                   */
/*   This routine will receive up to len bytes from the connection at socket      */
/*   and will copy the data to the user's buffer. It will never return more       */
/*   than len bytes but will return as soon as bytes are available in the buffer. */
/*   This is a simpler interface than xn_rcv_msg() but is of slightly             */
/*   lower performance.                                                           */
/*                                                                                */
/*   Note: This is the prefered interface for TCP. For UDP xn_pkt_recv() is       */
/*   prefered.                                                                    */
/*                                                                                */
/*   Warning !! : For UDP each call to recv() dequeues one packet from the        */
/*   port's input queue. If there is more data in the packet than the caller      */
/*   requested the residual data will be lost.                                    */
/*                                                                                */
/*   For more details see the RTIP Manual.                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   The number of bytes available in the buffer if successful or                 */
/*    -1 on an error (timeout, invalid socket number, etc). 0 is returned         */
/*   if otherside initiated a close and there is no more data in the input        */
/*   window.                                                                      */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set       */
/*   by this function.  For error values returned by this function see            */
/*   RTIP Reference Manual.                                                       */
/*                                                                                */

int recv(int socket, PFCHAR buffer, int buflen, int flags)  /*__fn__*/
{
int dummy_fromlen;

    return(recvfrom(socket, buffer, buflen, flags, (PSOCKADDR)0, 
                    (PFINT)&dummy_fromlen));
}

/* ********************************************************************        */
/* recvfrom()  - Receive data at a socket.                                     */
/*                                                                             */
/* Summary:                                                                    */
/*   #include "socket.h"                                                       */
/*                                                                             */
/*   int recvfrom (socket, buffer, buflen, flags, from, fromlen)               */
/*     int socket          - Socket returned by socket                         */
/*     PFCHAR buffer       - Buffer to receive to                              */
/*     int buflen          - Maximum bytes to receive                          */
/*     PSOCKADDR from      - address of otherside connected to                 */
/*     PFINT fromlen       - pointer to length of valid info in from           */
/*     int flags           - control flags (NI)                                */
/*                                                                             */
/* Description:                                                                */
/*  This routine will receive up to n bytes from the connection at socket and  */
/*  copy the data to the user's buffer. It will never return more              */
/*  than n bytes but will return as soon as bytes are available in the buffer. */
/*  This is a simpler interface than xn_pkt_rcv() but is of slightly           */
/*  lower performance.                                                         */
/*                                                                             */
/*  Note: This is the prefered interface for TCP. For UDP xn_pkt_recv() is     */
/*  prefered .                                                                 */
/*                                                                             */
/*  Warning: For UDP each call to xn_receive() dequeues one packet from the    */
/*  port's input queue. If there is more data in the packet than the caller    */
/*  requested the residual data will be lost.                                  */
/*                                                                             */
/*  For more details see the RTIP Manual.                                      */
/*                                                                             */
/* Returns:                                                                    */
/*   The number of bytes available in the buffer if successful or              */
/*    -1 on an error (timeout, invalid socket number, etc). 0 is returned      */
/*   if otherside initiated a close and there is no more data in the input     */
/*   window.  For RAW sockets, the number of bytes includes the IP header.     */
/*                                                                             */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set    */
/*   by this function.  For error values returned by this function see         */
/*   RTIP Reference Manual.                                                    */
/*                                                                             */

int recvfrom(int socket, PFCHAR buffer, int buflen, int flags, /*__fn__*/
             PSOCKADDR from, PFINT fromlen)                    /*__fn__*/
{
PSOCKADDR_IN addr_in;
PANYPORT pport;
#if (INCLUDE_TCP)
int      src_port;  
#endif
#if (INCLUDE_TCP)
int      len;
#endif
word wait_count;
RTIP_BOOLEAN error;

    RTIP_API_ENTER(API_RECV)
    DEBUG_LOG("recvfrom - socket = ", LEVEL_3, EBS_INT1, socket, 0); 
    
    ARGSUSED_INT(flags);          /* keep compiler happy */

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

    /* if buffer is invalid   */
    if (!buffer)
        return(set_errno(EFAULT));

    /* check if operation is illegal due to receive shutdown   */
    if (pport->port_flags & READ_SHUTDOWN)
        return(set_errno(ESHUTDOWN));

    addr_in = (PSOCKADDR_IN)from;

    if ( addr_in && (*fromlen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE)) )
        return(set_errno(EFAULT));

    /* check if operation is illegal due to write shutdown   */
    wait_count = RTIP_INF;
    if (pport->options & SOO_RCV_TIMEO)
    {
        wait_count = convert_timeval_to_ticks(&(pport->recv_timeval), &error);
        if (error)
            return(-1);     /* errno set by convert_timeval_to_ticks */
    }

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        len = tc_tcp_read((PTCPPORT)pport, (PFBYTE)buffer, buflen, 
                                (DCU KS_FAR *)0, flags, wait_count);

        /* if user requested address connected to   */
        if ( addr_in && fromlen && (len > 0) )
        {
            /* Get address connected to                                */
            /* NOTE: Going through an intermediate varible (src_port)  */
            /*       solves byte order problem                         */
            tc_tcp_pk_peer_address((PTCPPORT)pport, (PFBYTE)(&addr_in->sin_addr), 
                                (PFINT)&src_port);
            addr_in->sin_port = (word)src_port;
            *fromlen = sizeof(struct sockaddr_in);
        }
        return(len);
    }
    else
#endif

#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        /* read data and extract remote IP and port   */
        return(tc_udp_read(socket, pport, buffer, buflen, addr_in, flags,
                           wait_count));
    }
#endif

    /* ******   */
    return(set_errno(EOPNOTSUPPORT));

}
    
/* ********************************************************************   */
/*                                                                        */
/* Summary:                                                               */
/*                                                                        */
/* Description:                                                           */
/*                                                                        */
/* Returns:                                                               */
/*                                                                        */

/* ********************************************************************         */
/* send()  -  Send data to a socket.                                            */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "socket.h"                                                        */
/*                                                                              */
/*   int send(socket, buffer, buf_len, flags)                                   */
/*     int socket             - Socket returned by socket                       */
/*     PFCCHAR buffer         - Buffer to receive to                            */
/*     int buflen             - Maximum bytes to receive                        */
/*     int flags              - control flags (MSG_QUEUE - TCP only)            */
/*                                                                              */
/* Description:                                                                 */
/*   This routine sends n data bytes from buffer to the connection at           */
/*   socket.                                                                    */
/*                                                                              */
/*   For more details see the RTIP Manual.                                      */
/*                                                                              */
/* Returns:                                                                     */
/*   The number of bytes queued for sending or -1 if an error occurs.           */
/*   For UDP Returns between 0 and n inclusive. For TCP returns either n or -1. */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set     */
/*   by this function.  For error values returned by this function see          */
/*   RTIP Reference Manual.                                                     */
/*                                                                              */

/* DEFINED AS A MACRO IN RTIP.H   */

/* ********************************************************************         */
/* sendto()  -  Send data to a socket at specified address.                     */
/*                                                                              */
/* Summary:                                                                     */
/*   #include "socket.h"                                                        */
/*                                                                              */
/*   int  sendto (socket, buffer, buflen, flags, to, tolen)                     */
/*     int socket              - Socket returned by socket                      */
/*     PFCCHAR buffer          - Buffer to receive to                           */
/*     int buflen              - Maximum bytes to receive                       */
/*     PSOCKADDR to            - address of otherside to send to                */
/*     PFINT tolen             - pointer to length of valid info in to          */
/*     int flags               - control flags (MSG_QUEUE - TCP only)           */
/*                                                                              */
/* Description:                                                                 */
/*   For TCP this routine sends n data bytes from buffer to the connection at   */
/*   socket (the destination address is ignored.)  For UDP, if a destination    */
/*   address is passed, this routine sends n data bytes from buffer to the      */
/*   address specified, otherwise, it sends it to the address the socket is     */
/*   connected to.                                                              */
/*                                                                              */
/*   For more details see the RTIP Manual.                                      */
/*                                                                              */
/* Returns:                                                                     */
/*   The number of bytes queued for sending or -1 if an error occurs.           */
/*   For UDP Returns between 0 and n inclusive. For TCP returns either n or -1. */
/*                                                                              */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set     */
/*   by this function.  For error values returned by this function see          */
/*   RTIP Reference Manual.                                                     */
/*                                                                              */

int sendto(int socket, PFCCHAR buffer, int buflen, int flags,  /*__fn__*/
           PCSOCKADDR to, int tolen)                           /*__fn__*/
{
PANYPORT pport;
word wait_count;
RTIP_BOOLEAN error;

    RTIP_API_ENTER(API_SEND)

#if (!INCLUDE_UDP && !INCLUDE_RAW)
    ARGSUSED_INT(tolen);
    ARGSUSED_INT(to);
    ARGSUSED_INT(flags);          /* keep compiler happy */
#elif (!INCLUDE_TCP)
    ARGSUSED_INT(flags);          /* keep compiler happy */
#endif

    DEBUG_LOG("send - socket = ", LEVEL_3, EBS_INT1, socket, 0); 

    pport = api_sock_to_port(socket);
    if (!pport)
    {
        DEBUG_ERROR("send - ENOTSOCK: socket = ", EBS_INT1, socket, 0); 
        return(set_errno(ENOTSOCK));
    }

#if (INCLUDE_ERROR_CHECKING)
    /* if buffer is invalid   */
    if (!buffer)
        return(set_errno(EFAULT));
#endif

    /* check if operation is illegal due to write shutdown   */
    if (pport->port_flags & WRITE_SHUTDOWN)
        return(set_errno(ESHUTDOWN));

    /* check if operation is illegal due to write shutdown   */
    wait_count = RTIP_INF;
    if (pport->options & SOO_SEND_TIMEO)
    {
        wait_count = convert_timeval_to_ticks(&(pport->send_timeval), &error);
        if (error)
            return(-1);     /* errno set by convert_timeval_to_ticks */
    }

    /* ******   */
#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        return(tc_tcp_write((PTCPPORT)pport, (PFBYTE)buffer, buflen, 
                            (DCU)0, flags, wait_count));   /* TCP ignores to */
    }
    else
#endif

    /* ******   */
#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        if (!(pport->options & SOO_SEND_TIMEO))
        {
            if (pport->options & IO_BLOCK_OPT) 
            {
                wait_count = (word)RTIP_INF;
            }
            else
            {
                wait_count = 0;
            }
        }
        return(tc_udp_write(socket, pport, (PFBYTE)buffer, buflen, 
                            to, tolen, wait_count));
    }

    /* ******   */
    else
#endif          /* UDP or RAW */

        return(set_errno(EOPNOTSUPPORT));
}

/* ********************************************************************   */
/*                                                                        */
/* Summary:                                                               */
/*                                                                        */
/* Description:                                                           */
/*                                                                        */
/* Returns:                                                               */
/*                                                                        */

/* ********************************************************************     */
/* getpeername() - returns address of otherside connected to                */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int getpeername(socket, addr, addrlen)                                 */
/*     int socket                - Socket returned by socket & bound in     */
/*                                 bind & set to LISTEN in listen           */
/*     PSOCKADDR addr            - address of otherside connected to        */
/*     PFINT addrlen             - pointer to length of valid info in addr  */
/*                                                                          */
/*                                                                          */
/* Description:                                                             */
/*   Extracts othersides address connected to and stores it in addr.  The   */
/*   length of valid information in addr is stored in addrlen.              */
/*   The socket must be connected.                                          */
/*                                                                          */
/*   IP address and port number are in network bytes order.                 */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns 0 if successful, -1 if error detected.                         */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int getpeername (int socket, PSOCKADDR addr, PFINT addrlen)  /*__fn__*/
{
PANYPORT pport;
PSOCKADDR_IN addr_in;
#if (INCLUDE_TCP)
int src_port;
#endif

    RTIP_API_ENTER(API_GETPEERNAME)

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

#if (INCLUDE_ERROR_CHECKING)
    if (!addr || *addrlen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE))
        return(set_errno(EFAULT));
#endif

    addr_in = (PSOCKADDR_IN)addr;
    addr_in->sin_family = PF_INET;

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        /* check if it is not connected   */
        if (tc_cmp4((((PTCPPORT)pport)->out_ip_temp.ip_dest), ip_nulladdr, 4)) 
            return(set_errno(ENOTCONN));

        /* Get address connected to                                */
        /* NOTE: Going through an intermediate varible (src_port)  */
        /*       solves byte order problem                         */
        tc_tcp_pk_peer_address((PTCPPORT)pport, (PFBYTE)(&addr_in->sin_addr), 
                               (PFINT)&src_port);
        addr_in->sin_port = (word) src_port;
    }

    else 
#endif

#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        /* check if it is not connected   */
        if (tc_cmp4((((PUDPPORT)pport)->ip_connection.ip_dest), 
                    ip_nulladdr, IP_ALEN)) 
            return(set_errno(ENOTCONN));

        tc_mv4((PFBYTE)(&addr_in->sin_addr), 
               (((PUDPPORT)pport)->ip_connection.ip_dest), IP_ALEN);
        addr_in->sin_port = ((PUDPPORT)pport)->udp_connection.udp_dest;
                        /* meaningless for RAW sockets   */
    }

    else
#endif
        return(set_errno(EOPNOTSUPPORT));

    *addrlen = sizeof(struct sockaddr_in);   

    return(0);
}



/* ********************************************************************     */
/* getsockname() - returns address of local host                            */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   int getsockname(socket, addr, addrlen)                                 */
/*     int socket                - Socket returned by socket & bound in     */
/*                                 bind & set to LISTEN in listen           */
/*     PSOCKADDR addr            - address of otherside connected to        */
/*     PFINT addrlen             - pointer to length of valid info in addr  */
/*                                                                          */
/*                                                                          */
/* Description:                                                             */
/*   Extracts local address bound to and stores it in addr.  The            */
/*   length of valid information in addr is stored in addrlen.              */
/*   The socket must be bound or connected.                                 */
/*                                                                          */
/*   For more details see the RTIP Manual.                                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns 0 if successful, -1 if error detected.                         */
/*                                                                          */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set */
/*   by this function.  For error values returned by this function see      */
/*   RTIP Reference Manual.                                                 */
/*                                                                          */

int getsockname(int socket, PSOCKADDR addr, PFINT addrlen)  /*__fn__*/
{
PANYPORT pport;
PSOCKADDR_IN addr_in;
#if (INCLUDE_TCP)
PTCPPORT tcp_port;
#endif
#if (INCLUDE_UDP)
byte src_ip_addr[IP_ALEN];
PUDPPORT uport;
#endif

    RTIP_API_ENTER(API_GETSOCKNAME)

    pport = api_sock_to_port(socket);
    if (!pport)
        return(set_errno(ENOTSOCK));

#if (INCLUDE_ERROR_CHECKING)
    if (!addr || *addrlen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE))
        return(set_errno(EFAULT));
#endif

    addr_in = (PSOCKADDR_IN)addr;
    addr_in->sin_family = PF_INET;

#if (INCLUDE_TCP)
    if (pport->port_type == TCPPORTTYPE)
    {
        tcp_port = (PTCPPORT)pport;

        if (!(tcp_port->ap.port_flags & PORT_BOUND))
            return(set_errno(ENOTCONN));    /* not bound */

        /* Get address bound to   */
        tc_mv4((PFBYTE)(&addr_in->sin_addr), 
               tcp_port->out_ip_temp.ip_src, 4);
        addr_in->sin_port = (word)tcp_port->out_template.tcp_source;    
                                            /* keep in network byte order   */
    }
    else
#endif

#if (INCLUDE_UDP || INCLUDE_RAW)
    if ( (pport->port_type == UDPPORTTYPE) ||
         (pport->port_type == RAWPORTTYPE) )
    {
        uport = (PUDPPORT)pport;

        /* if not bound to IP addr but is connected, get local address based   */
        /* upon remote address and routing table                               */
        if (uport->ap.port_flags & PORT_BOUND)
        {
            tc_mv4((PFBYTE)(&addr_in->sin_addr), 
                   (uport->ip_connection.ip_src), IP_ALEN);
        }

        /* if not bound but is connected, get local address based       */
        /* upon remote address and routing table                        */
        /* NOTE: I don't think this is possible since connect will bind */
        else if (tc_cmp4((uport->ip_connection.ip_dest), ip_nulladdr, 4)) 
        {
            if (!tc_get_src_ip(src_ip_addr, 
                               uport->ip_connection.ip_dest))
            {
                return(set_errno(ENOTCONN));
            }
            tc_mv4((PFBYTE)(&addr_in->sin_addr), src_ip_addr, IP_ALEN);
        }
        else
            return(set_errno(ENOTCONN));    /* not bound or connected */

        /* set port number, even if connected but not bound then return   */
        /* port number assigned by socket                                 */
        addr_in->sin_port = uport->udp_connection.udp_source;
        return(0);

    }
    else
#endif

        return(set_errno(EOPNOTSUPPORT));

    *addrlen = sizeof(struct sockaddr_in);   

    return(0);
}

/* ********************************************************************   */
/* ********************************************************************   */
/* Database routines.                                                     */

/* ********************************************************************     */
/* inet_addr - convert a dotted-decimal format IP address to a long         */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   unsigned long inet_addr(string)                                        */
/*      PFCCHAR string - dotted-decimal IP address (e.g. 205.161.8.1)       */
/*                                                                          */
/* Description:                                                             */
/*   Convert a dotted-decimal format IP address to a long.                  */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns the IP address as a long.  If an error is detected INADDR_NONE */
/*   is returned.                                                           */
/*                                                                          */

unsigned long inet_addr(PFCCHAR string)
{
byte ip_addr[IP_ALEN];
unsigned long addr;

    if (ip_str_to_bin((PFBYTE)ip_addr, string))
        tc_mv4((PFBYTE)&addr, (PFBYTE)ip_addr, IP_ALEN);
    else
        addr = INADDR_NONE;

    return(addr);
}

/* ********************************************************************     */
/* inet_addr - convert an IP address to dotted-decimal format               */
/*                                                                          */
/* Summary:                                                                 */
/*   #include "socket.h"                                                    */
/*                                                                          */
/*   PFCHAR inet_ntoa(in)                                                   */
/*      struct in_addr in - IP address (in sin_addr field)                  */
/*                                                                          */
/* Description:                                                             */
/*   Convert an IP address which is in the sin_addr field of the            */
/*   in parameter to dotted-decimal format.  The in parameter is of         */
/*   type struct in_addr.                                                   */
/*                                                                          */
/*   NOTE: the string is returned in the global string ntoa_str, therefore, */
/*         the user must extract the address from ntoa_str before calling   */
/*         inet_ntoa again.                                                 */
/*                                                                          */
/* Returns:                                                                 */
/*   Returns the IP address in dotted-decimal format or 0 if an error is    */
/*   detected.                                                              */
/*                                                                          */

PFCHAR inet_ntoa(struct in_addr in)
{
    if (ip_bin_to_str(ntoa_str, (PFCBYTE)&(in.s_un.s_un_b)))
        return(ntoa_str);
    else
        return((PFCHAR)0);
}

/* ********************************************************************   */
/* DATABASE                                                               */
/* ********************************************************************   */

#if (INCLUDE_DB)

/* ********************************************************************           */
/* getprotobyname() - get protocol information based upon prototcol name          */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   PFPROTOENT getprotobyname(name)                                              */
/*     PFCHAR name      -   the protocol name                                     */
/*                                                                                */
/* Description:                                                                   */
/*   Returns a pointer to the internal database of protocol information           */
/*   corresponding to the requested protocol name.  The entry returned            */
/*   must not be modified.                                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   Returns a pointer to protoent structure if successful, -1 if error detected. */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set       */
/*   by this function.  For error values returned by this function see            */
/*   RTIP Reference Manual.                                                       */
/*                                                                                */

PFPROTOENT getprotobyname(PFCHAR name)
{
int index;
    
#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return((PFPROTOENT)0);
    }
#endif

    for (index=0; index < NUM_PROTOENT; index++)
    {
        if (tc_strcmp(name, protos[index].p_name) == 0)
        {
            return((PFPROTOENT)&protos[index]);
        }
    }
    set_errno(ENODATA);
    return((PFPROTOENT)0);
}

/* ********************************************************************           */
/* getprotobynumber() - get protocol information based upon prototcol number      */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "socket.h"                                                          */
/*                                                                                */
/*   PFPROTOENT getprotobynumber(proto)                                           */
/*     int proto        -   the protocol number in host byte order                */
/*                                                                                */
/* Description:                                                                   */
/*   Returns a pointer to the internal database of protocol information           */
/*   corresponding to the requested protocol number.  The entry returned          */
/*   must not be modified.                                                        */
/*                                                                                */
/* Returns:                                                                       */
/*   Returns a pointer to protoent structure if successful, -1 if error detected. */
/*                                                                                */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set       */
/*   by this function.  For error values returned by this function see            */
/*   RTIP Reference Manual.                                                       */
/*                                                                                */

PFPROTOENT getprotobynumber(int proto)
{
int index;
    
#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return((PFPROTOENT)0);
    }
#endif

    for (index=0; index < NUM_PROTOENT; index++)
    {
        if (proto == protos[index].p_proto)
        {
            return((PFPROTOENT)&protos[index]);
        }
    }
    set_errno(ENODATA);
    return((PFPROTOENT)0);
}

/* ********************************************************************          */
/* getservbyname() - get service information based upon name and protocol        */
/*                                                                               */
/* Summary:                                                                      */
/*   #include "socket.h"                                                         */
/*                                                                               */
/*   PFSERVENT getservbynamep(name, proto)                                       */
/*     PFCHAR name      -   the service name                                     */
/*     PFCHAR proto     -   protocol name                                        */
/*                                                                               */
/* Description:                                                                  */
/*   Returns a pointer to the internal database of service information           */
/*   corresponding to the requested name and protocol.  If proto is              */
/*   NULL, the first entry which matches name is returned.  The entry returned   */
/*   must not be modified.                                                       */
/*                                                                               */
/* Returns:                                                                      */
/*   Returns a pointer to servent structure if successful, -1 if error detected. */
/*                                                                               */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set      */
/*   by this function.  For error values returned by this function see           */
/*   RTIP Reference Manual.                                                      */
/*                                                                               */

PFSERVENT getservbyname(PFCHAR name, PFCHAR proto)
{
int index;

#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return((PFSERVENT)0);
    }
#endif

    for(index=0; index < NUM_SERVENT; index++)
    {
        if ( (!proto || tc_strcmp(proto, servs[index].s_proto) == 0) &&
             (tc_strcmp(name, servs[index].s_name) == 0) )
        {
            return((PFSERVENT)&servs[index]);
        }
    }

    set_errno(ENODATA);
    return((PFSERVENT)0);

}

/* ********************************************************************          */
/* getservbyport() - get service information based upon port and protocol        */
/*                                                                               */
/* Summary:                                                                      */
/*   #include "socket.h"                                                         */
/*                                                                               */
/*   PFSERVENT getservbyport(port, proto)                                        */
/*     int port         -   the port in network byte order                       */
/*     PFCHAR proto     -   protocol name                                        */
/*                                                                               */
/* Description:                                                                  */
/*   Returns a pointer to the internal database of service information           */
/*   corresponding to the requested port and protocol.  If proto is              */
/*   NULL, the first entry which matches name is returned.  The entry returned   */
/*   must not be modified.                                                       */
/*                                                                               */
/* Returns:                                                                      */
/*   Returns a pointer to servent structure if successful, -1 if error detected. */
/*                                                                               */
/*   Upon error, xn_getlasterror() may be called to retrieve error code set      */
/*   by this function.  For error values returned by this function see           */
/*   RTIP Reference Manual.                                                      */
/*                                                                               */

PFSERVENT getservbyport(int port, PFCHAR proto)
{
int index;
    
#if (INCLUDE_ERROR_CHECKING)
    /* if xn_rtip_init() has not called, return error   */
    if (!rtip_initialized)
    {
        set_errno(ENOTINITIALIZED);
        return((PFSERVENT)0);
    }
#endif

    for (index=0; index < NUM_SERVENT; index++)
    {
        if ( (!proto || tc_strcmp(proto, servs[index].s_proto) == 0) &&
             (port == servs[index].s_port) )
        {
            return((PFSERVENT)&servs[index]);
        }
    }

    set_errno(ENODATA);
    return((PFSERVENT)0);
}
#endif

#if (BUILD_NEW_BINARY)
word ntohs(word x) 
{
    return(net2hs((word)x));
}

word htons(word x) 
{
    return(hs2net((word)x));
}

dword htonl(dword x) 
{
    return(hl2net((dword)x));
}

dword ntohl(dword x) 
{
    return(net2hl((dword)x));
}

#endif

