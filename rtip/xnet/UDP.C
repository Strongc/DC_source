/*                                                                      */
/* UDP.C - UDP and RAW functions                                        */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/*  Module description:                                                 */
/*  This module provides services needed to implement the UDP and RAW   */
/*  services of the API and input packet layers.                        */
/*                                                                      */

/* Functions in this module:                                                   */
/* udp_sock_alloc   - Called by socket. Allocate a port structure and          */
/*                    preinitialize it as much as possible.                    */
/* udp_connect      - Called by xn_connect to set IP address and port number   */
/*                    port should communicate with                             */
/* udp_bind         - Called by xn_bind assign a well known port number to     */
/*                    a UDP port structure.                                    */
/* udp_close        - Unlink a udp port from an interface. Free all resources. */
/* tc_udp_interpret - Interpret an incoming udp packet. Checksum and direct    */
/*                    the packet to the appropriate port.                      */
/* tc_raw_interpret - Interpret an incoming raw packet.                        */
/* udp_pk_send      - Calculate UDP and IP checksums. Send the packet.         */

#define DIAG_SECTION_KERNEL DIAG_SECTION_UDP

#include "sock.h"
#include "rtip.h"
#include "rtipext.h"

/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_RAW 0
#define DISPLAY_NO_MATCH 0  /* display packets which have no associated */
                            /* socket   */
#define DISPLAY_MATCH 0     /* display packets which have associated */
                            /* socket only if DISPLAY_NO_MATCH is also set   */
#define DEBUG_BIND  0

/* ********************************************************************   */
#if (!INCLUDE_UDP && !INCLUDE_RAW)
int tc_udp_error(int rtip_errno)            /*__fn__*/
{
    if (rtip_errno > 0)
        set_errno(rtip_errno);
    return(-1);
}

int tc_udp_error_zero(int rtip_errno)   /*__fn__*/
{
    if (rtip_errno > 0)
        set_errno(rtip_errno);
    return(0);
}

int tc_udp_error_errno(int rtip_errno)  /*__fn__*/
{
    if (rtip_errno > 0)
        set_errno(rtip_errno);
    return(rtip_errno);
}

#else

/* ********************************************************************   */
RTIP_BOOLEAN    bind_port_anyaddr(PUDPPORT pport);
void    add_udpport_list(PUDPPORT port, int list_type);
void    delete_udpport_list(PUDPPORT port);
#if (INCLUDE_RAW)
void    add_rawport_list(PUDPPORT port, int list_type);
void    delete_rawport_list(PUDPPORT port);
#endif
#if (INCLUDE_UDP_PORT_CACHE && GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0)
void check_udp_port_cache(PUDPPORT port, RTIP_BOOLEAN clear);
#endif

/* ********************************************************************      */
/* tc_udp_sock_alloc() - allocates and initializes a UDP port structure      */
/*                                                                           */
/*  Inputs                                                                   */
/*      type        - type of socket (SOCK_DGRAM or SOCK_RAW)                */
/*      protocol    - protocol (used by SOCK_RAW only)                       */
/*                                                                           */
/*   Allocate a udp port structure and initialize generic information in     */
/*   the IP template field for this port. This information will be inhereted */
/*   by bind/connect and send/recv.                                          */
/*                                                                           */
/*   Returns 0 if successful or -1 if an error was detected.                 */
/*   Sets errno if an error is detected.                                     */
/*                                                                           */

PUDPPORT tc_udp_sock_alloc(int type, int protocol)       /*__fn__*/
{
PUDPPORT pu_port;
PUDPPORT uport;
int      i;
RTIP_BOOLEAN  in_use;

    /* Get a free port structure   */
    pu_port = os_alloc_udpport();

    if (!pu_port)
    {
        set_errno(EMFILE);
        return((PUDPPORT)0);
    }

    OS_CLAIM_UDP(SOCK_CLAIM_UDP)

#if (RTIP_VERSION < 26)
    /* Load up fixed values in the port structures udp template   */
    pu_port->eth_connection.eth_type = EIP_68K;
#endif

    /* Ip defaults   */
    if (type == SOCK_DGRAM)
        pu_port->ip_connection.ip_proto = PROTUDP;      /* UDP is the one */
    else
    {
        if (protocol > 0x00ff)
        {
            OS_RELEASE_UDP();
            os_free_udpport(pu_port);
            set_errno(EINVAL);
            return((PUDPPORT)0);
        }
        pu_port->ip_connection.ip_proto = (byte)protocol;    
    }

    /* default is blocking, perform checksum, no limit on input   */
    /* queue,                                                     */
    pu_port->ap.options = default_udp_options;

#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
    pu_port->ap.ip_options = 0;
    pu_port->ap.ip_option_len = 0;
#endif

    pu_port->no_udp_que = 0;

    if (type == SOCK_DGRAM)
    {
        pu_port->ap.port_type = UDPPORTTYPE;         

        /* add port to socket list                                        */
        /* NOTE: must be done after interface is set up in port structure */
        add_udpport_list(pu_port, SOCKET_LIST);  

        /* **************************************************     */
        /* Find an "random" port number within range specified by */
        /* configuration parameters                               */
        /* set initial udp port number   */
        if (!udp_port_number)
        {
            udp_port_number = CFG_UDP_PORT_MIN;
            udp_port_number = (word)
                (udp_port_number +                               
                (ks_get_ticks() % (CFG_UDP_PORT_MAX - CFG_UDP_PORT_MIN)));
        }

        for (;;)
        {
            /* get next port number within range specified by configurartion   */
            /* parameters                                                      */
            udp_port_number = (word)(udp_port_number + 1);
            if ( (udp_port_number < CFG_UDP_PORT_MIN) ||
                (udp_port_number > CFG_UDP_PORT_MAX) )
                udp_port_number = CFG_UDP_PORT_MIN;

            /* assign the port number to the socket   */
            /* NOTE: Will be reset if bind is called  */
            /* NOTE: save in host byte order          */
            pu_port->udp_connection.udp_source = hs2net(udp_port_number);

            /* check if this port number is already in use   */
            in_use = FALSE;
            for (i=FIRST_UDP_PORT; i < TOTAL_UDP_PORTS; i++)
            {
                uport = (PUDPPORT)(alloced_ports[i]);
                if (!uport || !IS_UDP_PORT(uport))
                    continue;

                if ( (uport->udp_connection.udp_source == 
                      pu_port->udp_connection.udp_source) &&
                     (uport != pu_port) )
                    in_use = TRUE;
            }       /* end of for loop thru sockets */
            if (in_use)
                continue;
            break;  
        }

#if (INCLUDE_UDP_PORT_CACHE && (GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0))
        /* Go check that this has a unique udp port number and if so   */
        /* allow udp port caching to speed up udp_interpret.           */
        check_udp_port_cache(pu_port, FALSE);
#endif
    }
    else if (type == SOCK_RAW) 
    {
        pu_port->ap.port_type = RAWPORTTYPE;               
        pu_port->protocol = protocol;

        /* add port to raw socket list                                    */
        /* NOTE: must be done after interface is set up in port structure */
        add_udpport_list(pu_port, SOCKET_LIST);  
    }
    else
    {
        DEBUG_ERROR("tc_udp_sock_alloc - illegal type", NOVAR, 0, 0);
    }
    
    OS_RELEASE_UDP()
    return(pu_port);
}

/* ********************************************************************   */
/* tc_udp_connect() - establish a UDP/RAW connection                      */
/*                                                                        */
/*  Establish a connection to a remote host.                              */
/*                                                                        */
/*  Inputs                                                                */
/*      port        - from socket                                         */
/*      to          - ip address of destination. 4 unsigned chars.        */
/*      to_port     - port number at the dest (ie 23 ==s telnet)          */
/*                    in network byte order                               */
/*                                                                        */
/*  called by connect()                                                   */
/*                                                                        */
/*  Returns 0 if successful or -1 if an error was detected.               */
/*  Sets errno if an error is detected.                                   */
/*                                                                        */

int tc_udp_connect(PUDPPORT pport, PFBYTE to, word to_port)  /*__fn__*/
{

    /* if port not bound, bind it based upon destination IP address   */
    if (!bind_port_anyaddr(pport))
        return(-1);     /* NOTE: bind_port_anyaddr will set errno if error */

    OS_CLAIM_UDP(CONNECT_CLAIM_UDP)

    /* check if it is already connected   */
    if (!tc_cmp4((pport->ip_connection.ip_dest), ip_nulladdr, 4)) 
    {
        OS_RELEASE_UDP()
        return(set_errno(EISCONN));
    }

    tc_mv4((pport->ip_connection.ip_dest), to, 4);

    if (pport->ap.port_type == UDPPORTTYPE) 
        pport->udp_connection.udp_dest = to_port;

    DEBUG_LOG("tc_udp_connect: ", LEVEL_3, IPADDR, pport->ip_connection.ip_dest, 0);

    /* clear any packets in udp exchange in case any came in after bind   */
    /* but before connect has completed since there is no guarentee the   */
    /* packet is for this port and it is ugly to check it when receive is */
    /* done, therefore,                                                   */
    /* - a port must be bound and if it is going to be connected,         */
    /*   connect must be done before pkt comes in                         */
    os_clear_udpapp_exchg(pport);

    /* move port from socket list to active list           */
    /* NOTE: this will work even if already on active list */
    delete_udpport_list(pport);
    add_udpport_list(pport, ACTIVE_LIST);  

#if (INCLUDE_POSTMESSAGE)
    PostMessage(pport->ap.hwnd, pport->ap.ctrl.index, FD_CONNECT, 0);
#endif

    OS_RELEASE_UDP()

    return(0);
}

/* ********************************************************************      */
/* void tc_udp_bind() - bind a UDP port                                      */
/*                                                                           */
/*  Inputs                                                                   */
/*      pu_port     - from socket                                            */
/*      ip_addr     - local ip address to bind to. 4 unsigned chars.         */
/*      to_port     - local port number to bind to - in network byte order   */
/*                                                                           */
/*   Copy the port number and relevant info from the iface structure into    */
/*   the port structure. This establishes a local interface and port number. */
/*                                                                           */
/*   called by xn_bind()                                                     */
/*                                                                           */
/*   Returns 0 if successful or -1 if an error was detected.                 */
/*   Sets errno if an error is detected.                                     */
/*                                                                           */

int tc_udp_bind(PUDPPORT pu_port, PFCBYTE ip_addr, word port_number)       /*__fn__*/
{
PUDPPORT uport;
int i;

    OS_CLAIM_UDP(BIND_CLAIM_UDP)

    /* if port already bound, return error   */
    if (pu_port->ap.port_flags & PORT_BOUND)
    {
        OS_RELEASE_UDP()
        return(set_errno(EINVAL));
    }

    /* check if address is in use by another socket   */
    if (!(pu_port->ap.options & SO_REUSEADDR))
    {
        for (i=FIRST_UDP_PORT; i < TOTAL_UDP_PORTS; i++)
        {
            uport = (PUDPPORT)(alloced_ports[i]);
            if (!uport || !IS_UDP_PORT(uport))
                continue;

            if (uport->ap.port_flags & PORT_BOUND)
            {
                if (uport->udp_connection.udp_source == port_number)
                {
                    if (tc_cmp4(ip_addr, ip_nulladdr, IP_ALEN)  ||
                        (uport->ap.port_flags & PORT_WILD)      ||
                        tc_cmp4(ip_addr, uport->ip_connection.ip_src, 
                                IP_ALEN) )
                    {
                        OS_RELEASE_UDP()
                        return(set_errno(EADDRINUSE));
                    }
                }
            }
        }
    }

    /* set wild IP address flag                                         */
    /* NOTE: do not need to clear port wild flag if not binding to wild */
    /*       since that was default set up and can only bind once       */
    if (tc_cmp4(ip_addr, ip_nulladdr, IP_ALEN))
        pu_port->ap.port_flags |= PORT_WILD;

    /* Set port number                                                 */
    /* NOTE: if port_number is 0, use default set by tc_udp_sock_alloc */
    if (pu_port->ap.port_type == UDPPORTTYPE && port_number)
    {
#if (INCLUDE_UDP_PORT_CACHE && (GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0))
        /* If anybody is sharing our port number let them be cacheable    */
        /* if they are the only one                                       */
        check_udp_port_cache(pu_port, TRUE);
#endif
        pu_port->udp_connection.udp_source = port_number;
#if (INCLUDE_UDP_PORT_CACHE && (GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0))
        /* Go check that this has a unique udp port number and if so   */
        /* allow udp caching.                                          */
        check_udp_port_cache(pu_port, FALSE);
#endif
    }

    DEBUG_LOG("bind UDP port to ", LEVEL_3, DINT1, 
        pu_port->udp_connection.udp_source, 0);

    /* set ip address; IP address could be wild   */
    tc_mv4(pu_port->ip_connection.ip_src, ip_addr, IP_ALEN);

    /* bind done, set flag so know it is bound   */
    pu_port->ap.port_flags |= PORT_BOUND;

    /* move port from socket list to active list           */
    /* NOTE: this will work even if already on active list */
    delete_udpport_list(pu_port);
    add_udpport_list(pu_port, ACTIVE_LIST);  

#if (INCLUDE_POSTMESSAGE)
    PostMessage(pu_port->ap.hwnd, pu_port->ap.ctrl.index, FD_WRITE, 0);
#endif

    OS_RELEASE_UDP()

    return(0);
}

/* ********************************************************************   */
/* if port not bound, bind it based upon destination IP address           */
/* i.e. set ip source and port source in UDP template                     */
RTIP_BOOLEAN    bind_port_anyaddr(PUDPPORT pport)   /*__fn__*/
{
    if (!(pport->ap.port_flags & PORT_BOUND))   /* if not bound already */
    {
        /* use the port number assigned by socket() and IP address 0, i.e.   */
        /* bind to wild IP which will force send to put in local IP address  */
        /* associated with outgoing interface                                */
#if (DEBUG_BIND)
        DEBUG_ERROR("bind anyaddr to port ", EBS_INT1, 
            pport->udp_connection.udp_source, 0);
#endif
        if (tc_udp_bind(pport, ip_nulladdr, pport->udp_connection.udp_source) < 0)
            return(FALSE);   /* NOTE: bind will set errno if error */
    }
#if (DEBUG_BIND)
    else
    {
        DEBUG_ERROR("bind anyaddr - already bound", NOVAR, 0, 0);
    }
#endif

    return(TRUE);
}


/* ********************************************************************   */
/* tc_udp_read() - read data from a socket                                */
/*                                                                        */
/*   Reads input data from socket and stores it in buffer.  buflen is     */
/*   the maximum amount of data to read.                                  */
/*                                                                        */
/*   Returns amount of data read or -1 if error is detected               */
/*                                                                        */

int tc_udp_read(int socket, PANYPORT pport, PFCHAR buffer, int buflen, 
                PSOCKADDR_IN addr_in, int flags, word wait_count)
{
DCU      msg;
RTIP_BOOLEAN  return_ip_hdr;
int      prot_hlen;
int      ip_hlen;
int      src_port;  
int      len;

    if (addr_in)   /* if recvfrom */
    {
        if ( !(pport->port_flags & PORT_BOUND) )
        {
            DEBUG_ERROR("tc_udp_read: recvfrom but NOT BOUND", DINT1, pport, 0);
            return(set_errno(EINVAL));
        }
    }
    else           /* if recv */
    {
        /* check if it is connected   */
        if (tc_cmp4(((PUDPPORT)pport)->ip_connection.ip_dest,ip_nulladdr, 4)) 
        {
            return(set_errno(ENOTCONN));
        }
    }

    /* if no block and packet is not available, return error   */
    if ( !(pport->options & IO_BLOCK_OPT) &&
            !os_udp_pkt_avail((PUDPPORT)pport) )
        return(set_errno(EWOULDBLOCK));              

    /* get UDP input packet (can block in none available)      */
    /* NOTE: if packet is fragmented and the UDP header is not */
    /*       in one chunck it was joined when it was input to  */
    /*       a point where at least the whole UDP header is in */
    /*       the first fragment (see ipfjoin())                */
    if (flags & MSG_PEEK)
        msg = os_rcvx_udpapp_exchg((PUDPPORT)pport, wait_count, FALSE);
    else
        msg = os_rcvx_udpapp_exchg((PUDPPORT)pport, wait_count, TRUE);

    if (msg)
    {
#if (DEBUG_RAW)
        if (pport->port_type == RAWPORTTYPE) 
        {
            DEBUG_ERROR("recvfrom - raw exchange, msg = ", DINT1, msg, 0);
        }
#endif

        /* extract othersides address from incoming packet   */
        if (addr_in)
        {
            /* Get address connected to                                */
            /* NOTE: Going through an intermediate varible (src_port)  */
            /*       solves byte order problem                         */
            tc_udp_pk_peer_address(msg, (PFBYTE)(&addr_in->sin_addr), 
                                    (PFINT)&src_port);
            addr_in->sin_port = (word)src_port;
        }

        /* setup for UDP   */
        prot_hlen = UDP_HLEN_BYTES;
        return_ip_hdr = FALSE;
        ip_hlen = 0;        /* don't copy any of IP hdr */

#if (INCLUDE_RAW)
        if (pport->port_type == RAWPORTTYPE) 
        {
            prot_hlen = 0;
            return_ip_hdr = TRUE;
            ip_hlen = IP_HLEN_DCU(msg);   /* copy all of IP hdr */
        }
#endif

        /* take minimum of buffer length and amount data in pkt;   */
        /* get total size including fragments                      */
        len = xn_pkt_data_size(msg, TRUE) + ip_hlen;
        if (len > buflen)
            len = buflen;

#if (INCLUDE_FRAG)
        if (DCUTOPACKET(msg)->frag_next)    /* if fragmented */
        {
            len = ipf_extract_data(msg, (PFBYTE)buffer, buflen, prot_hlen,
                                    return_ip_hdr);
        }
        else
        {
#endif
            if (pport->port_type == UDPPORTTYPE) 
                tc_movebytes((PFBYTE)buffer, 
                             xn_pkt_data_pointer(socket, msg, INPUT_PKT),
                             len);
#if (INCLUDE_RAW)
            else
            {
                /* raw includes IP header   */
                tc_movebytes((PFBYTE)buffer, 
                                (PFBYTE)(&DCUTOIPPKT(msg)->ip_verlen), 
                                len);
            }
#endif

#if (INCLUDE_FRAG)
        }
#endif
        if (!(flags & MSG_PEEK))
            os_free_packet(msg);    /* Free the input packet */

#if (INCLUDE_POSTMESSAGE)
        /* if more data is available   */
        if (os_udp_pkt_avail((PUDPPORT)pport))
        {
            PostMessage(pport->hwnd, pport->ctrl.index, FD_READ, 0);
        }
#endif

        return(len);
    }                               /* if msg */
    else
    {
        DEBUG_ERROR("os_rcvx_udpapp_exchg() timed out", NOVAR, 0, 0);
        return(set_errno(ETIMEDOUT));
    }

//XPC    return(set_errno(ENOTCONN));    /* should only happen if socket closed or aborted   */
}

/* ********************************************************************   */
/* tc_udp_write() - writes data to a socket                               */
/*                                                                        */
/*   Writes output data from buffer to socket.  buflen is                 */
/*   the amount of data to write.  Called for sockets API.                */
/*                                                                        */
/*   Returns 0 if successful or -1 if error is detected                   */
/*                                                                        */
int tc_udp_write(int socket, PANYPORT pport, PFBYTE buffer, int buflen, 
                 PCSOCKADDR to, int tolen, word wait_count)
{
#if (INCLUDE_UDP || INCLUDE_RAW)
DCU      msg;
#if (INCLUDE_FRAG)
PUDPPKT  pu;
#endif
int      n_this;
int      n_max;
word     prot_hlen;
RTIP_BOOLEAN  set_prot_hdr;
int      pkt_len;
int      dcu_flags;
int      status;
byte     send_to_addr[4];   /* could be actual ip dest or gateway ip dest */
word     to_port;
PFBYTE   to_addr;
PIFACE   pi;
#endif
int      ret_val;
PSOCKADDR_IN addr_in;
RTIP_BOOLEAN set_morefrag;

    if (pport->port_type == UDPPORTTYPE) 
    {
        prot_hlen = UDP_HLEN_BYTES;
        set_prot_hdr = TRUE;
    }
    else
    {
        prot_hlen = 0;
        set_prot_hdr = FALSE;
    }

    if (!buflen)
        return(0);

    addr_in = (PSOCKADDR_IN)to;

    if (addr_in)    /* if sendto */
    {
        if (tolen < (sizeof(struct sockaddr_in) - SIN_ZERO_SIZE)) 
            return(set_errno(EFAULT));

        to_addr = (PFBYTE)(&addr_in->sin_addr);
        to_port = addr_in->sin_port;
    }
    else            /* if send */
    {
        /* send must be connected for send (not necessary for sendto)   */
        if (tc_cmp4(((PUDPPORT)pport)->ip_connection.ip_dest, ip_nulladdr, 4)) 
            return(set_errno(ENOTCONN));

        to_addr = ((PUDPPORT)pport)->ip_connection.ip_dest;
        to_port = ((PUDPPORT)pport)->udp_connection.udp_dest;
    }

    /* ******                                                           */
    /* if blocking mode, let netwrite block RTIP_INF or if non-blocking */
    /* don't let netwrite block waiting for packet to be sent           */
    /* if wait_count, wait until packet is sent;                        */
    /* always free the packet                                           */
    if (pport->options & IO_BLOCK_OPT) 
    {
        dcu_flags = PKT_FLAG_SIGNAL;
    }
    else
    {
        dcu_flags = NO_DCU_FLAGS;
    }

    /* ******                              */
    /* get interface to send packet out on */
    ret_val = tc_udp_get_iface((PUDPPORT)pport, to_addr, (PIFACE KS_FAR *)&pi, 
                               send_to_addr);
    if (ret_val)
        return(ret_val);

    /* ******   */
    n_max = (word)xn_pkt_data_max(socket, to_addr); 

    /* determine amount of data and total packet size                      */
    /* if no frag: Xfer a whole packet's worth or based on mtu, whichever  */
    /* is less                                                             */
    /* if frag: this is size of first                                      */
    n_this = buflen;
    set_morefrag = FALSE;
    if (n_max < buflen)
    {
        set_morefrag = TRUE;
        n_this = n_max & ~7;        /* frag packets must be multiple of 8 */
                                    /* to accommodate ip_fragoff field   */
    }
    pkt_len = IP_TLEN_BYTES + pport->ip_option_len + prot_hlen + n_this;

#if (INCLUDE_NO_DCU_BLOCK)
    /* ******                                                      */
    /* set up to block in os_alloc_packet if no free packets which */
    /* satisfy size are free                                       */
    pport->ctrl.block_ticks = ks_ticks_p_sec(); /* tbd: from setsockopt */
#endif

    /* ******                 */
    /* allocate output packet */
    msg = ip_alloc_packet(pi, pport, pkt_len, 0, SENDTO_ALLOC);
    if (!msg)
        return(set_errno(ENOPKTS));

#if (INCLUDE_NO_DCU_BLOCK)
    /* ******                                                          */
    /* set up to NOT block in os_alloc_packet if no free packets which */
    /* satisfy size are free                                           */
    pport->ctrl.block_ticks = 0;
#endif

    if (set_morefrag)
        DCUTOIPPKT(msg)->ip_fragoff = IP_MOREFRAG_68K;  /* set frag bit on  */
                                                        /* all but last frag   */

    /* ******                                                      */
    /* set up headers in first frag (ether, IP and UDP)            */
    /* NOTE: for raw will write over the UDP header with data and  */
    /*       tc_udp_set_header does not setup length fields anyway */
    if (tc_udp_set_header((PUDPPORT)pport, msg, to_addr, to_port, 
                          pi, set_prot_hdr, n_this, buflen))
    {
        os_free_packet(msg);
        return(-1);
    }

#if (INCLUDE_FRAG)
    /* ******                                          */
    /* Xfer a whole packet's worth possibly fragmented */
    n_this = buflen;
    if (n_this > n_max)
    {
        /* create the fragmented packet   */
        msg = ipf_create_pkt(pi, pport, msg, buflen, prot_hlen);
        if (!msg)
            return(-1);

        /* set up length fields    */
        pu = DCUTOUDPPKT(msg);
        pu->udp_len = hs2net((word)(buflen+prot_hlen));

        /* fill in the data into the UDP packet   */
        msg = ipf_fill_pkt(pi, msg, buffer, buflen, prot_hlen);
        if (!msg)
            return(-1);

        /* fill in the rest of the info and send out the fragments   */
        status = ipf_send_udp_pkt(pi, send_to_addr, (PUDPPORT)pport, 
                                  msg, (word)buflen, wait_count,
                                  dcu_flags);
    }
    else
    {
#endif
        /* Move the bytes in that we are sending   */
#if (INCLUDE_TOKEN || INCLUDE_802_2)
        tc_movebytes(xn_pkt_data_pointer_iface(pi->ctrl.index, socket, msg, 
                                               OUTPUT_PKT), 
                     buffer, n_this);
#else
        tc_movebytes(xn_pkt_data_pointer(socket, msg, OUTPUT_PKT), 
                     buffer, n_this);
#endif

        status = tc_udp_pk_send(pi, send_to_addr, (PUDPPORT)pport, msg, 
                                (word)n_this, (word)n_this, wait_count,
                                dcu_flags);
#if (INCLUDE_FRAG)
    }
#endif

    /* ******   */
    if (status)
        n_this = -1;

    return(n_this);
}


/* ********************************************************************       */
/* udp_close() - close a UDP/RAW port                                         */
/*                                                                            */
/*  Inputs                                                                    */
/*      pu_port        - socket to close                                      */
/*                                                                            */
/*   Close a UDP port. Release owned resources and unlink it from the list of */
/*   active udp ports. Send the port strructure back to the UDPPORT resource  */
/*   exchange so it may be reused by tc_udp_sock_alloc()                      */
/*                                                                            */
/*   Returns nothing                                                          */
/*                                                                            */

void udp_close(PUDPPORT pu_port)     /*__fn__*/
{
PFBYTE p;

    /* If any messages are queued, free them   */
    os_clear_udpapp_exchg(pu_port);

    /* wake up recv with error   */
    pu_port->ap.ctrl.rtip_errno = ENOTCONN;    /* pass to set signal routine */
    os_set_read_signal((PANYPORT)pu_port, FALSE);

    /* wake up arp with error   */
    os_set_sent_signal((PANYPORT)pu_port, FALSE);

    /* Take pu_port out of linked list of in use ports, i.e.                      */
    /*   Points previous port's next field or the list head at the current port's */
    /*   next field.                                                              */
    delete_udpport_list(pu_port);

#if (INCLUDE_UDP_PORT_CACHE && (GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0))
    /* Go see if closing this port will create a situation such that   */
    /* another socket has a unique port number. This will turn on      */
    /* cacheing for that port                                          */
    check_udp_port_cache(pu_port, TRUE);
    if (cached_udp_port == pu_port)
        cached_udp_port = 0;
#endif

    /* To be clean we zero out info beyond the preinitialized stuff in the front   */
    p = (PFBYTE)pu_port;
    p += sizeof(struct _anyport);
    tc_memset(p, 0, sizeof(UDPPORT)-sizeof(struct _anyport));
#if (INCLUDE_ARP)
    /* get rid of any references to this port in the arp cache   */
    tc_arp_closeport((PANYPORT)pu_port);
#endif
    /* Release the port structure   */
    pu_port->ap.list_type = FREE_LIST; 
    os_free_udpport(pu_port);
}

/* ********************************************************************   */
/* tc_udp_close() - close a UDP/RAW port                                  */
/*                                                                        */
/*  Inputs                                                                */
/*      pu_port        - socket to close                                  */
/*                                                                        */
/*   Close a UDP port. Claims UDP semaphore while close done.             */
/*                                                                        */
/*   Called by xn_close() and tc_interface_close(). Claim/release done by */
/*   the caller                                                           */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void    tc_udp_close(PUDPPORT pu_port)     /*__fn__*/
{
    OS_CLAIM_UDP(CLOSE_CLAIM_UDP)

    /* close the socket   */
    udp_close(pu_port);

    OS_RELEASE_UDP()
}

/* ********************************************************************   */
/* INTERPRET (PROCESS INPUT PKTS)                                         */
/* ********************************************************************   */

#if (INCLUDE_UDP)

#if (INCLUDE_UDP_IFACE_CACHE && INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE)

/* ********************************************************************   */
/* clear_udp_iface_cache()  - The routing table has changed so clear udp  */
/*                            route cacheing                              */
/*                                                                        */
/*  Inputs                                                                */
/*      None                                                              */
/*                                                                        */
/*  This routine is called by the routing table code to clear routes      */
/*  cached in udp socket structures. Every time the routing table changes */
/*  all routes are cleared. This is only called if                        */
/*  INCLUDE_SYNCHRONIZE_UDP_IFACE_CACHE is 1.                             */
/*                                                                        */
/*  Returns nothing                                                       */
/*                                                                        */

void clear_udp_iface_cache(void)
{
PUDPPORT uport;
int i;

    OS_CLAIM_UDP(CLEAR_CACHE_CLAIM_UDP)
    for (i=FIRST_UDP_PORT; i < TOTAL_UDP_PORTS; i++)
    {
        uport = (PUDPPORT)(alloced_ports[i]);
        if (!uport || !IS_UDP_PORT(uport))
            continue;

        uport->ap.port_cache_flags &= ~UDP_IFACE_CACHE_VALID;
    }
    OS_RELEASE_UDP()
}

#endif


#if (INCLUDE_UDP_PORT_CACHE && (GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0))

/* ********************************************************************      */
/* check_udp_port_cache()  - Manage the udp port cache                       */
/*                                                                           */
/*  Inputs                                                                   */
/*      port          - Port with port number assigned to it                 */
/*      clear         - If true the port number is going out of service      */
/*                                                                           */
/*    If clear is false then searches all udp ports to determine if the port */
/*    number assigned to port is unique. If so it sets the port's flags to   */
/*    cacheable.                                                             */
/*                                                                           */
/*    If clear is true then port will be coming out of service so search     */
/*    all udp ports to determine if one and only one other port shares the   */
/*    same port number. If so it sets that port's flags to cacheable.        */
/*                                                                           */
/*                                                                           */
/*    Returns nothing                                                        */
/*                                                                           */

void check_udp_port_cache(PUDPPORT port, RTIP_BOOLEAN clear)
{
PUDPPORT uport;
PUDPPORT match_port;
int match_port_count;
int i;
word port_number;

    match_port = 0;
    match_port_count = 0;

    /* Start with caching turned on meaningless with clear   */
    port->ap.port_cache_flags |= UDP_PORT_CACHEABLE;
    port_number = port->udp_connection.udp_source;

    for (i=FIRST_UDP_PORT; i < TOTAL_UDP_PORTS; i++)
    {
        uport = (PUDPPORT)(alloced_ports[i]);
        if (!uport || !IS_UDP_PORT(uport))
            continue;

        if (uport != port && uport->udp_connection.udp_source == port_number)
        {
            /* If the port numbers match then cacheing is not allowed for   */
            /* this port number.                                            */
            /* If doing clear then we count the number of matches. If it is */
            /* exactly one we will enable caching for that port             */
            match_port = port;
            match_port_count++;
            port->ap.port_cache_flags  &= ~UDP_PORT_CACHEABLE;
            uport->ap.port_cache_flags &= ~UDP_PORT_CACHEABLE;
        }
    }
    if (clear)
    {
        if (match_port_count == 1)
            match_port->ap.port_cache_flags |= UDP_PORT_CACHEABLE;
    }
}
#endif



/* ********************************************************************    */
/* udp_find_port()  - find a matching port                                 */
/*                                                                         */
/*  Inputs                                                                 */
/*      port          - sets this to matching socket                       */
/*      listener_port - sets this to matching socket which is listening    */
/*      pu            - input packet                                       */
/*                                                                         */
/*    Searches the active list for a port that the UDP input packet is     */
/*    destined for.  A port must match the IP address and the port         */
/*    number in order to be a match.  If an exact match exists, (i.e.      */
/*    both source and destination IP addresses and port numbers match),    */
/*    the port structure is returned in port parameter.  The last listener */
/*    found, i.e. packet destined for us (IP address and port) but the     */
/*    socket is not connected, is returned in the listener_port parameter. */
/*                                                                         */
/*    Returns nothing                                                      */
/*                                                                         */
void udp_find_port(PIFACE pi, PUDPPORT *port, PUDPPORT *listener_port, DCU msg)
{
PUDPPKT port_pu;    /* Default UDP packet info in our port structure */
PIPPKT  port_pip;
int     out_port_match, his_ip_match;
PUDPPKT pu;
PIPPKT  pip;

    pu  = DCUTOUDPPKT(msg);
    pip = DCUTOIPPKT(msg);

    /* Scan the list of active udp ports on the interface    */
    *port = (PUDPPORT)(root_udp_lists[ACTIVE_LIST]);
    (*listener_port) = (PUDPPORT)0;

    while (*port)
    {
        /* Default UDP packet info in our port structure   */
        port_pu  = (PUDPPKT) &((*port)->udp_connection);
        port_pip = (PIPPKT) &((*port)->ip_connection);

        DEBUG_LOG("    port src port = ", LEVEL_3, EBS_INT1, port_pu->udp_source, 0);
        DEBUG_LOG("    port dest port = ", LEVEL_3, EBS_INT1, port_pu->udp_dest, 0);
        DEBUG_LOG("    msg src port = ", LEVEL_3, EBS_INT1, pu->udp_source, 0);
        DEBUG_LOG("    msg dest port = ", LEVEL_3, EBS_INT1, pu->udp_dest, 0);

        DEBUG_LOG("  port ip_dest: ", LEVEL_3, IPADDR, port_pip->ip_dest, 0);
        DEBUG_LOG("  port ip_src: ", LEVEL_3, IPADDR, port_pip->ip_src, 0);
        DEBUG_LOG("  msg ip_dest: ", LEVEL_3, IPADDR, pip->ip_dest, 0);
        DEBUG_LOG("  msg ip_src: ", LEVEL_3, IPADDR, pip->ip_src, 0);

        /* ******                                                               */
        /* Note: source and dest ports are stored in network order so we        */
        /* compare directly. We match if the sending port is the "dest" port    */
        /* of the connection, the recieving port is the "source" port of the    */
        /* connection, and the sending ip address matches the "dest" ip address */
        /* of the connection                                                    */
        /* See if his destination port is my port number                        */
        if (pu->udp_dest != port_pu->udp_source)
        {      
            /* If not, this port is definately not involved   */
            *port = (PUDPPORT)
                os_list_next_entry_off(root_udp_lists[ACTIVE_LIST], 
                                       (POS_LIST)(*port), ZERO_OFFSET);
            continue; 
        }

        /* ******                                                             */
        /* See if his destination ip addr is my ip addr, or loop back         */
        /* or broadcast or we bound with wild IP address                      */
        /* NOTE: multicast and broadcast must be bound to multicast/broadcast */
        /*       address or to wild IP                                        */
        if ( (pip->ip_dest[0] != IP_LOOP_ADDR) &&
             (!tc_cmp4(pip->ip_dest, port_pip->ip_src, IP_ALEN)) &&
             !((*port)->ap.port_flags & PORT_WILD) )
        {      
            DEBUG_LOG("udp_find_port - his dest does not match", LEVEL_3, NOVAR, 0, 0);

            /* If not, this port is definately not involved   */
            *port = (PUDPPORT)
                os_list_next_entry_off(root_udp_lists[ACTIVE_LIST], 
                                       (POS_LIST)(*port), ZERO_OFFSET);
            continue; 
        }

        /* ******                                             */
        /* See if his port number is my destination (connect) */
        out_port_match = (pu->udp_source==port_pu->udp_dest);

        /* See if his IP address is the current dest ip address for the socket    */
        /* (connect)                                                              */
        his_ip_match = tc_cmp4(pip->ip_src, port_pip->ip_dest, IP_ALEN);

        /* if his IP address is LOOP BACK it matches but only if   */
        /* previously connected - tbd                              */
        if ( (pip->ip_dest[0] == IP_LOOP_ADDR) &&
             (port_pu->udp_dest != 0) && 
             !tc_cmp4(ip_nulladdr, port_pip->ip_dest, IP_ALEN) )
            his_ip_match = TRUE;

        /* If his IP doesn't match see if we are connected to broadcast   */
        /* than we won't match above but we should accept it              */
        if (!his_ip_match)
            his_ip_match = tc_cmp4(ip_broadaddr, port_pip->ip_dest, IP_ALEN);

        /* If his IP doesn't match see if we are connected to net-directed   */
        /* broadcast than we won't match above but we should accept it       */
        if ( !his_ip_match && is_net_broadcast(pi, port_pip->ip_dest))
            his_ip_match = TRUE;

        /* If his port dest is my port Plus his port & IP are registered with   */
        /* this Socket; Then this is an already established connection and      */
        /* we found a match                                                     */
        if (out_port_match && his_ip_match)
            return;

        /* ******                                                            */
        /* If we get to here we know that he is sending to my IP address and */
        /* my udp port address, but I don't have his IP or port address in   */
        /* my template structure. If these values are zero in my template    */
        /* structure (meaning we've never connected),                        */
        /* we remember this socket as a listener. If we don't find           */
        /* any connected sockets we'll use a listener port.                  */
        if (port_pu->udp_dest == 0 && 
            tc_cmp4(ip_nulladdr, port_pip->ip_dest, IP_ALEN))
                (*listener_port) = *port;

        *port = (PUDPPORT)os_list_next_entry_off(root_udp_lists[ACTIVE_LIST], 
                                             (POS_LIST)(*port), ZERO_OFFSET);
    }
#if (DISPLAY_NO_MATCH)
    if ( (*port == 0) & (*listener_port == 0) )
    {
        DEBUG_ERROR("ERROR(rtip) : udp_find_port() - no port found - drop pkt", 
            NOVAR, 0, 0);
        DEBUG_ERROR("    msg src port = ", DINT1, net2hs(pu->udp_source), 0);
        DEBUG_ERROR("    msg dest port = ", DINT1, net2hs(pu->udp_dest), 0);
        DEBUG_ERROR("  msg ip_dest = ", IPADDR, pip->ip_dest, 0);
        DEBUG_ERROR("  msg ip_src = ", IPADDR, pip->ip_src, 0);
        DEBUG_ERROR("****ALL PORTS****", NOVAR, 0, 0);
        DEBUG_ERROR("PORTS", PORTS_UDP, 0, 0);
    }
#if (DISPLAY_MATCH)
    else
    {
        DEBUG_ERROR("UDP: found socket: port, listener_port", EBS_INT2,
            (*port)->ap.ctrl.index, (*listener_port)->ap.ctrl.index);
    }
#endif
#endif
}

/* ********************************************************************        */
/* tc_udp_interpret()     - Dispatch an incoming udp packet                    */
/*                                                                             */
/*   Inputs:                                                                   */
/*     PIFACE  pi  - Interface the packet came in on                           */
/*     DCU     msg - message structure containing the data packet              */
/*                                                                             */
/*   Called by the ip layer when a UDP packet arrives. Performs the UDP        */
/*   checksum  and then searches all user ports for either a listener or       */
/*   a connected port. If one is found the message is enqueued on the port's   */
/*   input queue.                                                              */
/*                                                                             */
/*   If the checksum is bad or no one is listening for this packet the message */
/*   is released.                                                              */
/*                                                                             */

void tc_udp_interpret(PIFACE pi, DCU msg)            /*__fn__*/
{
PUDPPKT  pu;         /* The incoming UDP packet */
PUDPPORT port;
PUDPPORT listener_port;

#if (!INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi);
#endif

    /* Get the packet from the message and perform checksum   */
    DEBUG_LOG("\ntc_udp_interpret - rcvd UDP pck", LEVEL_3, NOVAR, 0, 0);
    UPDATE_INFO(pi, rcv_udp_packets, 1)
    INCR_SNMP(UdpInDatagrams)

    pu = DCUTOUDPPKT(msg);

    OS_CLAIM_UDP(INTERPRET_CLAIM_UDP)

#if (INCLUDE_UDP_PORT_CACHE)
    port = 0;
#if (INCLUDE_MEASURE_PERFORMANCE)
    if (!do_sock_udp_cache)
        cached_udp_port = 0;
#endif
    if (cached_udp_port)
    {
        if (
            (pu->udp_dest == cached_udp_port->udp_connection.udp_source) &&
            (cached_udp_port->ap.port_flags & PORT_WILD || 
            tc_cmp4((DCUTOIPPKT(msg))->ip_dest,
            cached_udp_port->ip_connection.ip_src, IP_ALEN))
            )
            port = cached_udp_port;
    }
    if (!port)
    {
        /* **************************************************          */
        /* now try to find a destination on our machine for the packet */
        udp_find_port(pi, &port, &listener_port, msg);
        if (!port)
            port = listener_port;

        if (port)
#if (GUARANTEE_UNIQUE_UDP_PORT_NUMBERS == 0)
            if (port->ap.port_cache_flags & UDP_PORT_CACHEABLE)
#endif      
                cached_udp_port = port;
    }
#else
    /* **************************************************          */
    /* now try to find a destination on our machine for the packet */
    udp_find_port(pi, &port, &listener_port, msg);
    if (!port)
        port = listener_port;
#endif

    /* **************************************************   */
    /* If we found a connected port send to it              */
    if (port)
    {   
        /* **************************************************   */
        /* Checksum if enabled                                  */
        if ( (port->ap.options & SO_UDPCKSUM_IN) && pu->udp_chk && 
            tc_udp_chksum(msg) )
        {
            UPDATE_INFO(pi, udp_cksum, 1)
            INCR_SNMP(UdpInErrors)
            tc_icmp_send(msg, ICMP_T_PARAM_PROBLEM, 
                         ICMP_C_PTR_VALID, ICMP_P_UDPCHKSUM);
            DEBUG_ERROR("ERROR(rtip) : tc_udp_interpret() - checksum error,len = ", 
                EBS_INT1, IP_HLEN_DCU(msg), 0);

            /* Discard   */
            OS_RELEASE_UDP()
            os_free_packet(msg);            /* Free the input packet */
            return;
        }
        /* Check queue depth if the maximum udp queue value is set   */
        if ( ((port->ap.options & SO_MAX_UDP_QUE) &&
              (port->no_udp_que >= port->max_udp_que)) || 
             (port->ap.port_flags & READ_SHUTDOWN) )
        {
                DEBUG_ERROR("tc_udp_interpret - drop UDP pkt - que full",
                    NOVAR, 0, 0);
                UPDATE_INFO(pi, udp_que_full, 1)
                INCR_SNMP(UdpInErrors)
                OS_RELEASE_UDP()
                os_free_packet(msg);            /* Free the input packet */
                return;
        }

        DEBUG_LOG("tc_udp_interpret() - port found send to udp exchange, sock = ",
            LEVEL_3, EBS_INT1, port->ap.ctrl.index, 0);

#if (INCLUDE_POSTMESSAGE)
        PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_READ, 0);
#endif
        os_sndx_udpapp_exchg(port, msg);
    }

    else
    {
        tc_icmp_send(msg, ICMP_T_DEST_UNREACH, ICMP_C_PORTUR, 0);
        UPDATE_INFO(pi, udp_dropped, 1)
        INCR_SNMP(UdpNoPorts)
        os_free_packet(msg);            /* Free the input packet */
    }
    OS_RELEASE_UDP()
}
#endif      /* end of INCLUDE_UDP */


#if (INCLUDE_RAW)
/* ********************************************************************    */
/* raw_find_port()  - find a matching port                                 */
/*                                                                         */
/*  Inputs                                                                 */
/*      port   - socket                                                    */
/*      pu     - input packet                                              */
/*                                                                         */
/*    Searches the active list for a port that the RAW input packet is     */
/*    destined for.  A port must match the IP address and the port         */
/*    number in order to be a match.  If an exact match exists, (i.e.      */
/*    both source and destination IP addresses and port numbers match),    */
/*    the port structure is returned in port parameter.  The last listener */
/*    found, i.e. packet destined for us (IP address and port) but the     */
/*    socket is not connected, is returned in the listener_port parameter. */
/*                                                                         */
/*    Returns nothing                                                      */
/*                                                                         */
void raw_find_port(PUDPPORT *port, DCU msg)     /*__fn__*/
{
PIPPKT   port_pip;
int      his_ip_match;
PIPPKT   pip;

    pip = DCUTOIPPKT(msg);

    while (*port)
    {
        DEBUG_LOG("RAW interpret: protocol rcvd, port protocol", LEVEL_3, 
            EBS_INT2, pip->ip_proto, (*port)->protocol);

        /* if protocol does not match protocol (from socket call) then   */
        /* try next one                                                  */
        if (pip->ip_proto != (*port)->protocol)
        {
            *port = (PUDPPORT)
                os_list_next_entry_off(root_raw_lists[ACTIVE_LIST], 
                                       (POS_LIST)(*port), ZERO_OFFSET);
            continue; 
        }
            
        /* Default RAW packet info in our port structure   */
        port_pip = (PIPPKT) &((*port)->ip_connection);

        DEBUG_LOG("  port ip_dest = ", LEVEL_3, IPADDR, port_pip->ip_dest, 0);
        DEBUG_LOG("  port ip_src = ", LEVEL_3, IPADDR, port_pip->ip_src, 0);
        DEBUG_LOG("  msg ip_dest = ", LEVEL_3, IPADDR, pip->ip_dest, 0);
        DEBUG_LOG("  msg ip_src = ", LEVEL_3, IPADDR, pip->ip_src, 0);

        /* ******                                                            */
        /* See if his destination ip addr is my ip addr, or loop back        */
        /* or broadcast or we bound with wild IP address                     */
        /* NOTE: multicast and broadcast must be bound to multcast/broadcast */
        /*       address or to wild IP                                       */
        if ( (pip->ip_dest[0] != IP_LOOP_ADDR) &&
             (!tc_cmp4(pip->ip_dest, port_pip->ip_src, IP_ALEN)) &&
             !((*port)->ap.port_flags & PORT_WILD) )
        {      
            DEBUG_LOG("raw_find_port - his dest does not match", LEVEL_3, NOVAR, 0, 0);

            /* If not, this port is definately not involved   */
            *port = (PUDPPORT)
                os_list_next_entry_off(root_raw_lists[ACTIVE_LIST], 
                                       (POS_LIST)(*port), ZERO_OFFSET);
            continue; 
        }

        /* ******                                                               */
        /* See if his IP address is the current dest ip address for the socket  */
        /* (connect)                                                            */
        his_ip_match = tc_cmp4(pip->ip_src, port_pip->ip_dest, IP_ALEN);

        /* if his IP address is LOOP BACK it matches but only if   */
        /* previously connected - tbd                              */
        if ( (pip->ip_dest[0] == IP_LOOP_ADDR) &&
             !tc_cmp4(ip_nulladdr, port_pip->ip_dest,IP_ALEN) )
            his_ip_match = TRUE;

        /* If his IP doesn't match see if we requested a broadcast   */
        /* If so we should accept it (bootp) -                       */
        if (!his_ip_match)
            his_ip_match = tc_cmp4(ip_broadaddr, port_pip->ip_dest,IP_ALEN);

        /* If his port dest is my port Plus his port & IP are registered with   */
        /* this Socket; Then this is an already established connection          */
        if (his_ip_match)
            return;

        /* ******                                                            */
        /* If we get to here we know that he is sending to my IP address and */
        /* my raw port address, but I don't have his IP or port address in   */
        /* my template structure. If these values are zero in my template    */
        /* structure (meaning we've never connected), i.e. this port is      */
        /* a listener.  Return this port since we need to send to all        */
        /* RAW sockets (listeners and connected).                            */
        if (tc_cmp4(ip_nulladdr, port_pip->ip_dest, IP_ALEN))
            return;

        *port = (PUDPPORT)os_list_next_entry_off(root_raw_lists[ACTIVE_LIST], 
                                                 (POS_LIST)(*port), 
                                                 ZERO_OFFSET);
    }

}

/* ********************************************************************         */
/* tc_raw_interpret()     - Dispatch an incoming raw packet                     */
/*                                                                              */
/*   Inputs:                                                                    */
/*     PIFACE  pi  - Interface the packet came in on                            */
/*     DCU     msg - message structure containing the data packet               */
/*                                                                              */
/*   Called by the ip layer when a RAW packet arrives.                          */
/*   Searches all user ports for either a listener or a connected port.         */
/*   If one is found the message is enqueued on the port's input queue.         */
/*                                                                              */
/*   DOES NOT FREE THE PACKET                                                   */
/*                                                                              */
/*   Returns TRUE if found at least one matching RAW socket; else returns FALSE */

RTIP_BOOLEAN tc_raw_interpret(PIFACE pi, DCU msg)            /*__fn__*/
{
PUDPPORT port;
RTIP_BOOLEAN  found_one;
DCU      msg2;
DCU      msg2_head;
DCU      msg_ptr;
DCU      msg2_prev;

#if (!INCLUDE_KEEP_STATS)
    ARGSUSED_PVOID(pi);
#endif

    /* Get the packet from the message and perform checksum   */
    DEBUG_LOG("\ntc_raw_interpret - rcvd RAW pck", LEVEL_3, NOVAR, 0, 0);
    UPDATE_INFO(pi, rcv_raw_packets, 1)

    OS_CLAIM_UDP(INTERPRETR_CLAIM_UDP)

    /* **************************************************          */
    /* now try to find a destination on our machine for the packet */
    found_one = FALSE;
    port = (PUDPPORT)(root_raw_lists[ACTIVE_LIST]);
    while (port)
    {
        /* find the next port which matches   */
        raw_find_port(&port, msg);

        /* **************************************************   */
        /* If we found a connected port send to it              */
        if (port)
        {   
            found_one = TRUE;

            /* **************************************************         */
            /* Check queue depth if the maximum raw queue value is set    */
            if ( ((port->ap.options & SO_MAX_UDP_QUE) &&
                (port->no_udp_que >= port->max_udp_que)) || 
                (port->ap.port_flags & READ_SHUTDOWN) )
            {
                    DEBUG_ERROR("os_sndx_udpapp_exchg - drop RAW pkt - que full",
                        NOVAR, 0, 0);
                    UPDATE_INFO(pi, raw_que_full, 1)

                    /* skip past the port just processed   */
                    port = (PUDPPORT)
                        os_list_next_entry_off(root_raw_lists[ACTIVE_LIST], 
                                               (POS_LIST)port, ZERO_OFFSET);
                    continue;
            }

            DEBUG_LOG("tc_raw_interpret() - port found send to raw exchange", LEVEL_3, NOVAR, 0, 0);

            /* duplicate the packet since might go to multiple RAW sockets   */
            /* as well as to UDP/TCP socket                                  */
            /* NOTE: msg2 will get freed after it is removed from the UDP    */
            /*       exchange (which is on the raw socket); msg will be      */
            /*       freed by either IP/UDP/TCP interpret or if it is        */
            /*       put on an exchange/window it will be freed when         */
            /*       read api call is done                                   */
            msg_ptr = msg;
            msg2_head = (DCU)0;
            msg2_prev = (DCU)0;
            while (msg_ptr)
            {
                msg2 = os_alloc_packet(DCUTOPACKET(msg_ptr)->length, RAW_RCV);
                if (!msg2)
                {
                    if (msg2_head)
                        os_free_packet(msg2_head);
                    msg2_head = (DCU)0;
                    break;
                }
                if (!msg2_head)
                    msg2_head = msg2;

                /* set up pointer to protocol header and IP header   */
                DCUTOPACKET(msg2)->ip_option_len = 
                    DCUTOPACKET(msg_ptr)->ip_option_len;
                DCUSETUPPTRS(msg2, msg_is_802_2(msg_ptr))

                /* copy info extracted from IP header   */
                STRUCT_COPY(msg2->ip_info, msg->ip_info);

                /* copy the input msg to the new DCU   */
                tc_movebytes(DCUTODATA(msg2), DCUTODATA(msg_ptr), 
                    DCUTOPACKET(msg_ptr)->length);
                DCUTOPACKET(msg2)->length = DCUTOPACKET(msg_ptr)->length;

#if (INCLUDE_FRAG)
                /* link the new DCUs together in the frag list     */
                if (msg2_prev)
                    DCUTOPACKET(msg2_prev)->frag_next = msg2;
                DCUTOPACKET(msg2)->frag_next = (DCU)0;
                msg2_prev = msg2;       /* save for next loop */
                /* get next fragment     */
                msg_ptr = DCUTOPACKET(msg_ptr)->frag_next;
#else
                /* get next fragment     */
                msg_ptr = (DCU)0;
#endif
            }       /* end of loop thru frags */
#            if (DEBUG_RAW)
                DEBUG_ERROR("tc_raw_interpret() - port found send to raw exchange: msg = ", 
                    DINT2, msg2, 0);
#            endif

            if (msg2_head)
            {
                os_sndx_udpapp_exchg(port, msg2_head);
            }

            /* skip past the port just processed   */
            port = (PUDPPORT)
                os_list_next_entry_off(root_raw_lists[ACTIVE_LIST], 
                                       (POS_LIST)port, ZERO_OFFSET);
        }   /* end of if packet is destined to current port */
    }       /* end of loop thru ports */
    OS_RELEASE_UDP()
    return(found_one);
}
#endif      /* end of INCLUDE_RAW */

/* ********************************************************************   */
/* OUTPUT ROUTINES                                                        */
/* ********************************************************************   */

/* ********************************************************************   */
int tc_udp_get_iface(PUDPPORT pport, PFBYTE to,                 /*__fn__*/
                      PIFACE KS_FAR *ppi, PFBYTE send_to_addr)  /*__fn__*/
{
#if (!INCLUDE_UDP_IFACE_CACHE)
    ARGSUSED_PVOID(pport)
#endif

#if (INCLUDE_UDP_IFACE_CACHE)
    /* If the routing table cache info is valid for this socket
       then see if we are sending to the same IP address that
       the route was cached for. If so we already have the IFACE
       and the host or router to send the packet to */
    *ppi = 0;
    if ( (pport->ap.port_cache_flags & UDP_IFACE_CACHE_VALID) &&
         tc_cmp4(to, pport->last_sent_ip_addr, IP_ALEN) )
    {
        *ppi = pport->ap.iface;
        tc_mv4(send_to_addr, pport->ap.send_ip_addr, IP_ALEN);
    }
    else
    {
        /* Go search the routing table for the IFACE and 
           host or router to send the packet to. If we find it
           update our caching information */
        *ppi = rt_get_iface(to, (PFBYTE)send_to_addr, (PANYPORT)pport, 
                            (RTIP_BOOLEAN *)0);
        if (*ppi)
        {
            pport->ap.port_cache_flags |= UDP_IFACE_CACHE_VALID;

            /* Save the interface we send to   */
            pport->ap.iface = *ppi;

            /* Save the host we send to   */
            tc_mv4(pport->ap.send_ip_addr, send_to_addr, IP_ALEN);

            /* Save the IP address of the last host we looked up
               in the routing table */
            tc_mv4(pport->last_sent_ip_addr, to, IP_ALEN);
        }
        else
        {
            pport->ap.port_cache_flags &= ~UDP_IFACE_CACHE_VALID;
            DEBUG_LOG("tc_udp_get_iface - rt_get_iface failed", LEVEL_3, NOVAR, 0, 0);
            INCR_SNMP(IpOutNoRoutes)
            OS_RELEASE_UDP()
            return(set_errno(EADDRNOTAVAIL));
        }
    }
#else
    /* get output interface   */
    *ppi = rt_get_iface(to, (PFBYTE)send_to_addr, (PANYPORT)pport, 
                        (RTIP_BOOLEAN *)0);
    if (!*ppi)
    {
        DEBUG_LOG("tc_udp_get_iface - rt_get_iface failed", LEVEL_3, NOVAR, 0, 0);
        INCR_SNMP(IpOutNoRoutes)
        OS_RELEASE_UDP()
        return(set_errno(EADDRNOTAVAIL));
    }
#endif
    return(0);      /* success */
}

/* ********************************************************************        */
/* int tc_udp_set_header() - set up ETHERNET, IP and UDP header                */
/*   Inputs:                                                                   */
/*     PUDPPORT pport    - Initilized port                                     */
/*     DCU      msg      - Message containing data to send                     */
/*     PFBYTE   to       - destination IP address                              */
/*     word     to_port  - udp destination port number                         */
/*     PIFACE   pi       - interface structure                                 */
/*     RTIP_BOOLEAN  set_prot_hdr - set up protocol (UDP) header               */
/*     int      data_len  - length of data in this pkt (used to set up ip_len) */
/*     int      total_data_len - total length of data                          */
/*                                                                             */
/*   Sets up ethernet, IP and UDP headers execpt for length fields and         */
/*   checksums (since the data has not been filled in yet). Also determines    */
/*   the interface and the actual address to send to.                          */
/*                                                                             */
/*   Returns 0 if successful or -1 if an error was detected.                   */
/*   Sets errno if an error is detected.                                       */
/*                                                                             */

int tc_udp_set_header(PUDPPORT pport, DCU msg, PFBYTE to, word to_port, /*__fn__*/
                      PIFACE pi, RTIP_BOOLEAN set_prot_hdr, int data_len,    /*__fn__*/
                      int total_data_len)
{
PUDPPKT pu;
#if (INCLUDE_IPV4)
PIPPKT  pip;
PIPPKT  pip_temp;
#endif
PUDPPKT pu_temp;
word    ip_len;

    DEBUG_LOG("tc_udp_set_header() entered", LEVEL_3, NOVAR, 0, 0);

    /* **************************************************                       */
    /* If send_to,                                                              */
    /*   - bind if necessary                                                    */
    /*   - copy the template to the outgoing msg                                */
    /*   - connect for this msg only i.e. copy the desination ip and port       */
    /*     number into the message  but if it is already connected return error */
    if ((!to_port && (pport->ap.port_type == UDPPORTTYPE)) || 
        !to)
    {
        return(set_errno(EADDRNOTAVAIL));   
    }

    /* if port not bound, bind it based upon destination IP address   */
    if (!(pport->ap.port_flags & PORT_BOUND))   /* if not bound already */
        if (!bind_port_anyaddr(pport))  
            return(-1);     /* NOTE: bind_port_anyaddr will set errno if error */


    /* **************************************************************   */
    /* Claim udp here because the routing table code will claim it      */
    /* if it want's to invalidate our cache entries.                    */
    /* **************************************************************   */
    OS_CLAIM_UDP(HEADER_CLAIM_UDP)

    /* **************************************************   */
    /* calculate IP length                                  */
    ip_len = (word)(IP_HLEN_BYTES + pport->ap.ip_option_len + data_len);
    if (set_prot_hdr)
        ip_len += (word)UDP_HLEN_BYTES;

    /* **************************************************         */
    /* Copy ETHERNET, IP and UDP header fields from the template  */
    /* to the output packet                                       */
    pu = DCUTOUDPPKT(msg);
    pu_temp  = (PUDPPKT)(&((PUDPPORT)pport)->udp_connection);

    /* copy UDP header   */
    if (set_prot_hdr)
    {
        tc_movebytes((PFBYTE)pu, (PFBYTE)pu_temp, sizeof(UDPPKT));
        /* **************************************************   */
        /* set up rest of UDP header                            */
        pu->udp_dest = to_port;    /* keep in network byte order */
    }

    /* **************************************************   */
    /* set up rest of ethernet header                       */
    /* put in our hardware address                          */
    SETUP_LL_HDR(msg, pi, EIP_68K, SEND_802_2(pi, (PANYPORT)pport),
                 ip_len + LLC_SNAP_HLEN_BYTES);

#if (INCLUDE_IPV4)
    /* **************************************************   */
    /* set up IP header                                     */

    /* copy IP headers   */
    pip = DCUTOIPPKT(msg);
    pip_temp  = (PIPPKT)(&((PUDPPORT)pport)->ip_connection);
    tc_movebytes((PFBYTE)pip, (PFBYTE)pip_temp, IP_HLEN_BYTES);

    /* copy dest port and IP to outgoing msg,                                      */
    /* - if send, this is address connected to                                     */
    /* - if sendto, connects the port temporarily, therefore, do not do everything */
    /*   connect would do i.e.                                                     */
    /*     - connect the output msg only (not the template)                        */
    /*     - do not put port on active list                                        */
    /*     - do not clear UDP exchange                                             */
    /* NOTE: when options are supported they should be written in here and         */
    /*       ip_verlen needs to be set accordingly; ip_verlen is fixed now         */
    tc_mv4(pip->ip_dest, to, IP_ALEN);

    /* if bound to wild IP address, put in a legitimate address into the   */
    /* outgoing msg; at this point all we know is the output iface so we   */
    /* will use that one                                                   */
    /* NOTE: if multicast or broadcast, tc_netwrite will set up the source */
    /*       IP address for each interface it is sending to                */
    if ( (pport->ap.port_flags & PORT_WILD) || 
         is_net_broadcast(pi, pip->ip_src) )
        tc_mv4(pip->ip_src, pi->addr.my_ip_addr, IP_ALEN);

    /* setup IP header   */
    setup_ipv4_header(msg, (PANYPORT)pport, ip_len, 0,
                      (RTIP_BOOLEAN)(data_len < total_data_len ? TRUE : FALSE), 0,
                      TRUE);
#endif

    OS_RELEASE_UDP()   /* tc_netwrite blocks so release sem */

    return(0);
}


/* ********************************************************************        */
/* int tc_udp_pk_send() - Send UDP packet                                      */
/*   Inputs:                                                                   */
/*     PIFACE pi           - Interface info                                    */
/*     PFBYTE send_to_addr - IP address to send to (could be gateway)          */
/*     PUDPPORT pport      - Initilized port                                   */
/*     DCU msg             - Message containing data to send                   */
/*     word pkt_len        - Length of the data for this packet fragment       */
/*     word total_len      - Length of the data for all fragments              */
/*     word wait_count     - Fail if not delivered in this many ticks.         */
/*                           if 0 returns success immediately. The             */
/*                           packet will be sent when the output task          */
/*                           gets to it                                        */
/*                                                                             */
/*   Fills in UDP length and UDP checksum info in headers and calls            */
/*   tc_netwrite() to send the packet.  If the packet is fragmented,           */
/*   tc_udp_pk_send() is called for each fragment.                             */
/*                                                                             */
/*   Note: If wait_count is non-zero it is up to the caller to free the packet */
/*                                                                             */
/*   Returns 0 if successful or -1 if an error was detected.                   */
/*   Sets errno if an error is detected.                                       */
/*                                                                             */

int tc_udp_pk_send(PIFACE pi, PFBYTE send_to_addr, PUDPPORT pport,    /*__fn__*/
                   DCU msg, word pkt_len, word total_pkt_len,         /*__fn__*/
                   word wait_count, int dcu_flags)                    /*__fn__*/
{
word    chk;
PUDPPKT pu;
int     ret_val;

    DEBUG_LOG("tc_udp_pk_send() entered", LEVEL_3, NOVAR, 0, 0);

    OS_CLAIM_UDP(WRITE_CLAIM_UDP)

    pu = DCUTOUDPPKT(msg);

    /* **************************************************             */
    /* Load the udp info and set packet length in DCU (used by device */
    /* driver)                                                        */

    /* calculate length of data, IP options, Ethernet and IP headers   */
    /* NOTE: UDP header size might be added below                      */
    TOTAL_IP_HLEN_SIZE(DCUTOPACKET(msg)->length, ((PANYPORT)pport), pi, 0)
    DCUTOPACKET(msg)->length += pkt_len;

    /* write total packet length in the UDP header            */
    /* NOTE: if not frag or first frag, there is a UDP header */
    if (pport->ap.port_type == UDPPORTTYPE)
    {
        /* if not FRAG or the first fragment, then it has a UDP header   */
        if (!is_frag_not_first(msg))
        {
            DCUTOPACKET(msg)->length += UDP_HLEN_BYTES;
            pu->udp_len = hs2net((word)(total_pkt_len+UDP_HLEN_BYTES));
        }

        /* Udp Length is the size of the data plus the udp header length (8)   */
        if (!is_frag(msg))
        {
            /* Calculate the udp checksum   */
            pu->udp_chk = 0;
            if (pport->ap.options & SO_UDPCKSUM_OUT)
            {
                chk = tc_udp_chksum(msg);
                if (chk == 0)
                    chk = (word) 0xffff;    /* equivelent to 0 in 1's complement */
                                            /* see RFC 1122 - 4.1.3.4   */
                pu->udp_chk = chk;
            }
        }

        UPDATE_INFO(pi, udp_sends, 1)
        UPDATE_INFO(pi, udp_send_bytes, pkt_len)
        INCR_SNMP(UdpOutDatagrams)

    }
    else    /* RAW */
    {
        UPDATE_INFO(pi, raw_sends, 1)
        UPDATE_INFO(pi, raw_send_bytes, pkt_len)
    }
    INCR_SNMP(IpOutRequests)

    /* **************************************************   */
    OS_RELEASE_UDP()   /* tc_netwrite blocks so release sem */

    /* Queue it up for writing   */
    ret_val = tc_netwrite(pi, (PANYPORT)pport, msg, (PFBYTE)send_to_addr, 
                          dcu_flags, wait_count);

    if (ret_val != 0)
        return(set_errno(ret_val));   /* tc_netwrite returned errno if error */
    return(ret_val);   
}


/* ********************************************************************       */
/* GET ADDRESS                                                                */
/* ********************************************************************       */
/* tc_udp_pk_peer_address() - extracts othersides address from packet         */
/*                                                                            */
/*  Inputs                                                                    */
/*      msg        - input packet                                             */
/*      host_ip    - set to the IP address of the sender of the input packet  */
/*      host_port  - set to the port number of the sender of the input packet */
/*                                                                            */
/*   extract othersides address (IP and port number) from incoming packet     */
/*   and stores them in host_ip and host_port.  For UDP there is an ACTIVE    */
/*   list and a SOCKET list.                                                  */
/*                                                                            */
/*   Returns nothing                                                          */
/*                                                                            */

void tc_udp_pk_peer_address(DCU msg, PFBYTE host_ip, PFINT host_port) /*__fn__*/
{
PUDPPKT pu;
PIPPKT  pip;

    pu  = DCUTOUDPPKT(msg);
    pip = DCUTOIPPKT(msg);

    tc_mv4(host_ip, pip->ip_src, IP_ALEN);
    *host_port =  (int)pu->udp_source;    /* keep in network byte order */
}

/* ********************************************************************   */
/* UDP LIST ROUTINES                                                      */
/* ********************************************************************   */
/* add_udpport_list() - add port to global list                           */
/*                                                                        */
/*  Inputs                                                                */
/*      port        - socket to add to list                               */
/*      list_type   - specifies which list to add the socket to           */
/*                    (SOCKET_LIST, ACTIVE_LIST)                          */
/*                                                                        */
/*   Deletes UDP port from global list based upon list type               */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */
void add_udpport_list(PUDPPORT port, int list_type)   /*__fn__*/
{
    port->ap.list_type = list_type;

#if (INCLUDE_RAW)
    if (port->ap.port_type == RAWPORTTYPE)
    {
        root_raw_lists[list_type] = 
            os_list_add_rear_off(root_raw_lists[list_type], 
                                 (POS_LIST)port, ZERO_OFFSET);
        return;
    }
#endif

    root_udp_lists[list_type] = 
        os_list_add_rear_off(root_udp_lists[list_type], 
                             (POS_LIST)port, ZERO_OFFSET);
}

/* ********************************************************************   */
/* delete_udpport_list() - deletes ports from global list                 */
/*                                                                        */
/*  Inputs                                                                */
/*      port        - socket to add to list                               */
/*      list_type   - specifies which list to add the socket to           */
/*                    (SOCKET_LIST, ACTIVE_LIST)                          */
/*                                                                        */
/*   Deletes UDP port from global list based upon list type stored in the */
/*   port structure.  For UDP there is an ACTIVE list and a SOCKET list.  */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void delete_udpport_list(PUDPPORT port)  /*__fn__*/
{
int list_type;

    list_type = port->ap.list_type;

#if (INCLUDE_RAW)
    if (port->ap.port_type == RAWPORTTYPE)
    {
        root_raw_lists[list_type] = 
            os_list_remove_off(root_raw_lists[list_type], 
                               (POS_LIST)port, ZERO_OFFSET);
        return;
    }
#endif
    root_udp_lists[list_type] = 
        os_list_remove_off(root_udp_lists[list_type], (POS_LIST)port, 
                           ZERO_OFFSET);
}

/* ********************************************************************   */
/* udp_invalidate_sockets() - abort sockets bound to IP address           */
/*                                                                        */
/*    Frees all sockets in system bound to the IP                         */
/*    address ip_addr.  Called by DHCP client.                            */

void udp_invalidate_sockets(PFBYTE ip_addr)
{
PUDPPORT uport; 
int      i;

    for (i=FIRST_UDP_PORT; i < TOTAL_UDP_PORTS; i++)
    {
        uport = (PUDPPORT)(alloced_ports[i]);
        if (!uport || !IS_UDP_PORT(uport))
            continue;

        if ( (uport->ap.port_flags & PORT_BOUND) &&
             tc_cmp4(ip_addr, uport->ip_connection.ip_src, IP_ALEN) )
        {
            uport->ap.ctrl.rtip_errno = ENOTCONN;        /* pass to set signal routine */
            os_set_read_signal((PANYPORT)uport, FALSE);  /* wake up read */
            os_set_sent_signal((PANYPORT)uport, FALSE);  /* wake up arp */

            /* disable sends and receives   */
            shutdown(uport->ap.ctrl.index, 2);
        }
    }
}

#endif      /* INCLUDE_UDP or INCLUDE_RAW */

