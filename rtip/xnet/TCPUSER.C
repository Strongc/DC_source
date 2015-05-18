/*                                                                       */
/* TCPUSER.C - TCP API helper functions functions                        */
/*                                                                       */
/* EBS - RTIP                                                            */
/*                                                                       */
/* Copyright Peter Van Oudenaren , 1993                                  */
/* All rights reserved.                                                  */
/* This code may not be redistributed in source or linkable object form  */
/* without the consent of its author.                                    */
/*                                                                       */
/*  Module description:                                                  */
/*  This module provides a layer between the api code in api.c and the   */
/*  tcp stack implentation in tcp.c. If the TCP protocol is not to be    */
/*  used. (INCLUDE_TCP is 0 int xnconf.h) then the routines will not be  */
/*  compiled.                                                            */
/*                                                                       */

/*  Functions in this module                                              */
/* tc_tcp_sock_alloc    -  Called by socket. Allocates and initializes a  */
/*                         tcp port structure.                            */
/* tc_tcp_bind          -  Called by bind. Bind a local port number to a  */
/*                         tcp port structure.                            */
/* tc_tcp_listen        -  Called by listen. Sets up for connections on a */
/*                         tcp port.                                      */
/* tc_tcp_accept        -  Called by accept. Waits for a connection on a  */
/*                         tcp port.                                      */
/* tc_tcp_connect       -  Called by connect. Initiates a session between */
/*                         a local & remote tcp port.                     */
/* tc_tcp_read          -  Called by read(). Implements the read          */
/*                         system call for tcp.                           */
/* tc_tcp_write         -  Called by write(). Implements the write        */
/*                         system call for tcp.                           */
/* tc_tcp_close         -  Called by close(). Initiates a close           */
/*                         of a session.                                  */

#define DIAG_SECTION_KERNEL DIAG_SECTION_TCP


#include "sock.h"
#include "rtip.h"
#include "rtipext.h"
/* ********************************************************************   */
/* DEBUG AIDS                                                             */
/* ********************************************************************   */
#define DEBUG_DACK 0
#define DEBUG_WEB  0
#define DEBUG_SWS  0

#if (DEBUG_WEB)
extern int listen_fail; 
#endif

/* ********************************************************************     */
/* NOTES:                                                                   */
/*    - master, slave ports are for the end which does the passive open     */
/*      i.e. listen, accept; whereas the side which does the active         */
/*      open, i.e. connect just has one port associated with the            */
/*      connections                                                         */
/*    - each port is either on the free, active, listen or sockets list;    */
/*      it can be on the arp cache list and/on a master's port list as well */
/*    - the arp cache and the free list use the same next entry pointers    */
/*      in the port structure; the active, listen and sockets list use      */
/*      the same next pointer in the port structure; They can share the     */
/*      pointers since these sets of lists are exclusive, i.e. port can     */
/*      only be on one of the lists                                         */
/*                                                                          */
/* ********************************************************************     */

#if (!INCLUDE_TCP)
void tcpuser_c_dummy(void)
{
}
#else
/* ********************************************************************            */
/* tc_tcp_sock_alloc() - allocates and initializes a TCP port structure            */
/*                                                                                 */
/*  Inputs:                                                                        */
/*      RTIP_BOOLEAN sem_claimed - specifies whether TCP semaphore already claimed */
/*                                                                                 */
/*  Allocate a tcp port structure and initialize generic information in the        */
/*  TCP port port structure. This information will be inhereted by bind/connect    */
/*  and send/rcv.                                                                  */
/*                                                                                 */
/*  Fails if no port structures are available, etc                                 */
/*                                                                                 */
/*  Returns port structure allocated.                                              */
/*  Sets errno if an error is detected.                                            */
/*                                                                                 */

PTCPPORT tc_tcp_sock_alloc(RTIP_BOOLEAN sem_claimed)       /*__fn__*/
{
PTCPPORT port;
int      best_time, i;
PTCPPORT curr_port;
PTCPPORT best_port;
RTIP_BOOLEAN  in_use;
PTCPPORT tport;

    /* **************************************************         */
    /* Get a free port structure, zero it and allocate semaphores */
    port = os_alloc_tcpport();

    if (!sem_claimed)
    {
        OS_CLAIM_TCP(SOCK_CLAIM_TCP)  /* Exclusive access to tcp resources */
    }

    /* **************************************************                     */
    /* if none available search sockets in TWAIT and CLOSED state for the     */
    /* socket which has been in one the states the longest of all the sockets */
    /* which can be reused, then free the socket, then try allocation again   */
    /* NOTE: sockets in TWAIT and CLOSED state are in active list             */
    if (!port)
    {
        best_port = (PTCPPORT)0;
        /* the first loop thru look for sockets which have an empty   */
        /* input window; the second loop thru look at sockets         */
        /* even if data in input window                               */
        /* tbd: possibly make reuse sock for data in input window     */
        /*      a socket option                                       */
        for (i = 0; i < 2 && !best_port; i++)
        {
            curr_port = (PTCPPORT)(root_tcp_lists[ACTIVE_LIST]);
            best_port = (PTCPPORT)0;
            best_time = -1;
            while (curr_port)
            {
                DEBUG_LOG("tc_tcp_sock_alloc() - try reclaim - state, options = ", 
                    LEVEL_3, EBS_INT2, curr_port->state, curr_port->ap.options); 

                /* if current port can be used   */
                if ( ((curr_port->state == TCP_S_TWAIT) ||
                    (curr_port->state == TCP_S_CLOSED)) &&
                    ((curr_port->ap.options & SO_REUSESOCK) &&
                    (curr_port->ap.port_flags & API_CLOSE_DONE) &&
                    (i == 1 || curr_port->in.contain == 0) ))
                {
                    /* if it is the longest in TWAIT or CLOSED thus far   */
                    if ((int)curr_port->closetime > best_time)
                    {
                        best_port = curr_port;
                        best_time = (int)curr_port->closetime;
                    }
                }
                curr_port = (PTCPPORT)
                    os_list_next_entry_off(root_tcp_lists[ACTIVE_LIST], 
                                           (POS_LIST)curr_port, ZERO_OFFSET);
            }   /* end of loop thru active list */
        }

        /* see if a port structure was found which can be reuse   */
        if (best_port)
        {
            DEBUG_LOG("tc_tcp_sock_alloc() - close a socket in twait",
                LEVEL_3, NOVAR, 0, 0);

            /* go to CLOSED state and dump the input window   */
            /* NOTE: trans_state will free DCUs in windows    */
            trans_state(best_port, TCP_S_CLOSED, NOT_FROM_INTERPRET, TRUE);

            if (tcp_close(best_port) == PORT_FREED)
                port = os_alloc_tcpport();
        }
    }

    /* **************************************************                     */
    /* if none available even after trying to reclaim a socket in TWAIT state */
    if (!port)
    {
        DEBUG_ERROR("tc_tcp_sock_alloc - out of TCP ports", NOVAR, 0, 0);
        DEBUG_ERROR("PORT LISTS", PORTS_TCP, 0, 0);
        set_errno(EMFILE);
        if (!sem_claimed)
        {
            OS_RELEASE_TCP()
        }
        return((PTCPPORT)0);
    }

    /* **************************************************   */
    /* Allocated but not on the list yet                    */
    trans_state(port, TCP_S_ALLOCED, NOT_FROM_INTERPRET, FALSE);

#if (RTIP_VERSION < 26)
    /* Load up fixed values in the port structure   */
    /* type                                         */
    port->out_eth_temp.eth_type = EIP_68K;
#endif

    /* Ip defaults   */
    port->out_ip_temp.ip_proto = PROTTCP;    /* TCP is the one */
    
    /* **************************************************    */
    /* Find a "random" port number within range specified by */
    /* configuration parameters for the initial port number  */
    /* initialize first port number   */
    if (!tcp_port_number)
    {
        tcp_port_number = CFG_TCP_PORT_MIN;
        tcp_port_number = (word)
            (tcp_port_number +                               
            (ks_get_ticks() % (CFG_TCP_PORT_MAX - CFG_TCP_PORT_MIN)));
    }

    /* get next port number within range specified by configuration   */
    /* parameters                                                     */
    for (;;)
    {
        /* get the next port number   */
        tcp_port_number = (word)(tcp_port_number + 1);
        if ( (tcp_port_number < CFG_TCP_PORT_MIN) ||
             (tcp_port_number > CFG_TCP_PORT_MAX) )
            tcp_port_number = CFG_TCP_PORT_MIN;

        /* assign the port number to the socket   */
        /* NOTE: Will be reset if bind is called  */
        /* NOTE: save in host byte order          */
        port->in.port = tcp_port_number;    

        /* check if this port number is already in use   */
        in_use = FALSE;
        for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
        {
            tport = (PTCPPORT)(alloced_ports[i]);
            if (!tport || !IS_TCP_PORT(tport) || tport->state == TCP_S_FREE)
                continue;

            /* check if already in use   */
            if ( (tport->in.port == tcp_port_number) &&
                 (port != tport) )
            {
                in_use = TRUE;
            }
        }   /* end of for loop thru TCP socket */
        if (in_use)
            continue;
        break;  
    }

    /* **************************************************   */
    /* Set up some control block info                       */

#if (INCLUDE_TCP_RTT)
    port->smooth_mean_dev = CFG_MINRTO >> 1; /* 3 seconds */
#endif
    port->rto     = CFG_MINRTO;              /* smooth_rtt + 2*smooth_mean_dev =  */
                                             /* 6 sec   */

    port->ap.options = default_tcp_options;
    port->tcp_port_type = INVALID_PORT_TYPE;  
                                    /* i.e. not master/slave/connected    */
    /* **************************************************                 */
    /* Set up tcp header info                                             */
    /* NOTE: must be done after calculating port number and windows setup */
    /* NOTE: some values which never change are set in template during    */
    /*       runtime; others which change with each pkt are passed as     */
    /*       parameters to tcpsend; otherwise values are set here         */
    /* NOTE: tcp_seq and tcp_ack will be filled when packet sent          */
    port->out_template.tcp_source  = hs2net(tcp_port_number);  
                                                     /* source "my" port   */
                                                     /* (also set by bind) */
    /* **************************************************                    */
    /* add it to the socket list so it can be freed if interface closes      */
    /* before a connect, listen or accept is done which will put it on       */
    /* another list                                                          */
    /* We don't yet link the port into the interface's list of active ports. */
    /* That is done by connect or accept or into the list of listen ports.   */
    /* That is done by listen.                                               */
    /* NOTE: this cannot be done until the interface in the port structure   */
    /*       is set up                                                       */
    add_tcpport_list(port, SOCKET_LIST);

    /* **************************************************   */
    if (!sem_claimed)
    {
        OS_RELEASE_TCP()                /* Release access to tcp resources */
    }
    return(port);
}


/* ********************************************************************            */
/* tc_tcp_bind() - Bind a socket to a port number and IP address                   */
/*                                                                                 */
/*  Inputs:                                                                        */
/*      PTCPPORT pport   -  Socket returned from socket                            */
/*      word port_number -  TCP local port number (network byte order)             */
/*      PFBYTE ip_addr   -  IP address                                             */
/*      RTIP_BOOLEAN sem_claimed - specifies whether TCP semaphore already claimed */
/*                                                                                 */
/*   This routine is called to bind a socket to a local port number and IP         */
/*   address                                                                       */
/*                                                                                 */
/*  Called by: bind()                                                              */
/*                                                                                 */
/*  Returns 0 if successful or -1 if an error is detected.                         */
/*  Sets errno if an error is detected.                                            */
/*                                                                                 */

int tc_tcp_bind(PTCPPORT port, PFBYTE ip_addr, word port_number, RTIP_BOOLEAN sem_claimed)           /*__fn__*/
{
PTCPPORT tport;
int i;

    if (!sem_claimed)
    {
        OS_CLAIM_TCP(BIND_CLAIM_TCP)    /* Exclusive access to tcp resources */
    }

    if (port->ap.port_flags & PORT_BOUND)
    {
        if (!sem_claimed)
        {
            OS_RELEASE_TCP()                /* Release access to tcp resources */
        }
        return(set_errno(EINVAL));
    }

    /* check if address is in use by another socket   */
    if (!(port->ap.options & SO_REUSEADDR))
    {
        for (i=FIRST_TCP_PORT; i < TOTAL_TCP_PORTS; i++)
        {
            tport = (PTCPPORT)(alloced_ports[i]);
            if ( !tport || !IS_TCP_PORT(tport) || 
                 (tport->state == TCP_S_FREE) ||
                 (tport->state == TCP_S_CLOSED) )
                 continue;

            if (tport->ap.port_flags & PORT_BOUND)
            {
                if (tport->out_template.tcp_source == port_number)
                {
                    if (tc_cmp4(ip_addr, ip_nulladdr, IP_ALEN)  ||
                        (tport->ap.port_flags & PORT_WILD)      ||
                        tc_cmp4(ip_addr, tport->out_ip_temp.ip_src, 
                                IP_ALEN) )
                    {
                        if (!sem_claimed)
                        {
                            OS_RELEASE_TCP()  
                        }
                        DEBUG_ERROR("tc_tcp_bind: EADDRINUSE: socket using state ",
                            EBS_INT1, tport->state, 0);
                        return(set_errno(EADDRINUSE));
                    }
                }
            }
        }
    }

    /* if not wild IP address, set Ether Source Hardware address otherwise   */
    /* it will be set when packets are sent and the output iface is known    */
    /* NOTE: it is not necessary to clear port bound wild flags since the    */
    /*       port can only be bound once and it is initialized to not        */
    /*       bound wild                                                      */
    if (tc_cmp4(ip_addr, ip_nulladdr, 4))
        port->ap.port_flags |= PORT_WILD;

    /* if port is 0, use default set by tc_tcp_alloc_socket   */
    if (port_number)  
    {
        port->in.port                 = net2hs(port_number);
        port->out_template.tcp_source = port_number; 
    }

    /* IP source address - could be wild IP address   */
    tc_mv4(port->out_ip_temp.ip_src, ip_addr, 4);

    /* bind done   */
    port->ap.port_flags |= PORT_BOUND;

    if (!sem_claimed)
    {
        OS_RELEASE_TCP()                /* Release access to tcp resources */
    }

    return(0);
}


/* ********************************************************************            */
/* tc_tcp_listen() - allocate and set up a listener socket for the master socket   */
/*                                                                                 */
/* Inputs                                                                          */
/*      PTCPPORT mport      - from socket                                          */
/*      int backlogsize     - number of connections port will accept               */
/*      RTIP_BOOLEAN sem_claimed - specifies whether TCP semaphore already claimed */
/*                                                                                 */
/* Sets up for a connection at this tcp port.                                      */
/* The local port should have first been set by calling bind                       */
/*                                                                                 */
/* called by listen() and tc_tcp_interpret()                                       */
/*                                                                                 */
/* Returns 0 if successful or -1 if not.  Sets errno if an error was               */
/* detected.                                                                       */
/*                                                                                 */

int tc_tcp_listen(PTCPPORT mport, int backlogsize, RTIP_BOOLEAN sem_claimed)       /*__fn__*/
{
PTCPPORT sport;
PTCPPORT lst_port;

#if (DEBUG_WEB)
    DEBUG_ERROR("listen called - tcp_port_type, backlogsize = ", EBS_INT2,
        mport->tcp_port_type, backlogsize);
#endif
    DEBUG_LOG("listen called - tcp_port_type, backlogsize = ", LEVEL_3, EBS_INT2,
        mport->tcp_port_type, backlogsize);

    /* **************************************************                 */
    /* The following summarizes how LISTEN etc works                      */
    /* **************************************************                 */
    /* LISTEN : perform the following:                                    */
    /*     slave port - allocate slave, set tcp_port_type to SLAVE,       */
    /*                  set pointer to master                             */
    /*                  insert slave in master's slave list               */
    /*     master port - set tcp_port_type to backlogsize-1 (i.e. MASTER) */
    /* **************************************************                 */

    /* **************************************************                    */
    /* ACCEPT : perform the following:                                       */
    /*      wait for read signal                                             */
    /*      search master list for slave in EST state                        */
    /*      if find one                                                      */
    /*          SLAVE port - take off master list                            */
    /*                       set port type to NOT_SLAVE_OR_MASTER            */
    /*          MASTER port - if (tcp_port_type = MASTER_NO_LISTENS)         */
    /*                          (i.e. interpret could not add port since too */
    /*                                many listens in progress)              */
    /*               tc_tcp_listen(1) - add slave to master                  */
    /*                        else                                           */
    /*                           tcp_port_type++                             */
    /* **************************************************                    */

    /* **************************************************                    */
    /* tc_tcp_interpret() : perform the following                            */
    /*  if get sync                                                          */
    /*          - go to SYNR                                                 */
    /*          - move port from listen to active list                       */
    /*          - if tcp_port_type is SLAVE                                  */
    /*                   if master tcp_port_type is 0 (backlog size reached) */
    /*                      set master's tcp_port_type to MASTER_NO_LISTENS  */
    /*                   else                                                */
    /*              tc_tcp_listen(tcp_port_type)                             */
    /* **************************************************                    */
    /* fix any illegal values for backlogsize                                */
    /* NOTE: make listen(s,0) same as listen(s,1)                            */
    /* NOTE: accept() could call with backlog = MASTER_NO_LISTENS            */
    if (backlogsize <= 0) 
        backlogsize = 1;

    if (backlogsize > CFG_TCP_MAX_CONN) 
        backlogsize = CFG_TCP_MAX_CONN;

    /* check if the socket was bound    */
    if ( !(mport->ap.port_flags & PORT_BOUND) )
    {
        return(set_errno(EINVAL));
    }

    /* get new port with same port number as master          */
    /* NOTE: it will have a unique seq number and port index */
    sport = tc_tcp_sock_alloc(sem_claimed);  

    if (!sport)
        return(-1);    /* NOTE: tc_tcp_sock_alloc will set errno */

    /* allocate input and output windows      */
    /* NOTE: must be done before state change */
    setup_both_windows(sport);

    /* bind the slave port with same value as master        */
    /* NOTE: tc_tcp_bind expects port in network byte order */
    sport->ap.options |= SO_REUSEADDR;
    if (tc_tcp_bind(sport, mport->out_ip_temp.ip_src, hs2net(mport->in.port),
                    sem_claimed) < 0)
    {
        DEBUG_ERROR("tc_tcp_listen: bind failed: errno = ", EBS_INT1,
            xn_getlasterror(), 0);
        closesocket(sport->ap.ctrl.index);
        return(-1);
    }

    /* ******                     */
    /* set same options as master */
    sport->ap.options = mport->ap.options;
#if (INCLUDE_IPV4 && INCLUDE_IP_OPTIONS)
    sport->ap.ip_options = mport->ap.ip_options;
    sport->ap.route_option_len = mport->ap.route_option_len;
    sport->ap.ip_option_len = mport->ap.ip_option_len;
#endif
    sport->ap.ip_ttl = mport->ap.ip_ttl;

    sport->ka_interval = mport->ka_interval;
    sport->ka_retry = mport->ka_retry;
    sport->ka_tmo = mport->ka_tmo;

#if (INCLUDE_SSL)
    if(sport->ap.options & SOO_SECURE_SOCKET)
        sport->sctx = mport->sctx;
#endif

    /* ******   */
    if (!sem_claimed)
    {
        OS_CLAIM_TCP(LISTEN_CLAIM_TCP)
    }

    /* set up slave port info   */
    sport->tcp_port_type = SLAVE_PORT;  /* passive opened slave port */
    sport->master_port = mport;         /* point to master port */

    /* set the slave port up for listen state   */
    sport->out.port = 0;                /* accept any outside port # */
    trans_state(sport, TCP_S_LISTEN, NOT_FROM_INTERPRET, FALSE);

    /* Link the slave port into the interface's list of listener ports -    */
    /* put at end of list                                                   */
    /* NOTE: sport is not on any list since called os_alloc_tcpport() and   */
    /*       mport is on socket list which it should stay on                */
    delete_tcpport_list(sport);     /* take it off SOCKET LIST or possibly */
                                    /* MASTER list (if it previously had   */
                                    /* a problem during interpret doing    */
                                    /* a listen())                         */
    add_tcpport_list(sport, LISTEN_LIST);

    /* set up master info i.e. insert slave into masters slave list   */
    /* at end of list                                                 */
    sport->next_slave = (PTCPPORT)0;
    if (!mport->next_slave)
    {
        /* list is empty so it is first one in list   */
        mport->next_slave = sport;
    }
    else
    {   
        /* insert at end of list       */
        lst_port = mport->next_slave;
        while (lst_port->next_slave)        /* while not last slave in list */
            lst_port = lst_port->next_slave;
        lst_port->next_slave = sport;
    }
    mport->tcp_port_type = backlogsize - 1;  /* number slave port that */
                                             /* can be added to listen   */
                                             /* list                     */

    if (!sem_claimed)
    {
        OS_RELEASE_TCP()               /* Release access to tcp resources */
    }

#if (DEBUG_WEB)
    DEBUG_ERROR("listen returned - tcp_port_type, backlogsize = ", EBS_INT2,
        mport->tcp_port_type, backlogsize);
    DEBUG_LOG("interpret: PORT SUMMARY = ", LEVEL_3, PORTS_TCP, 0, 0);  
#endif
    DEBUG_LOG("listen returned - tcp_port_type, backlogsize = ", LEVEL_3, EBS_INT2,
        mport->tcp_port_type, backlogsize);

    return(0);
}


/* ********************************************************************       */
/* tc_find_accept_port() - find slave port in state accept can return         */
/*                                                                            */
/*   Scans the slave ports attached to master port and returns the            */
/*   first port it finds which accept can return.  If take_off_flag is        */
/*   true the slave is taken off the master's slave list.                     */
/*                                                                            */
/*   Returns slave port ready for accept or 0 if no acceptable port is found. */
/*                                                                            */

PTCPPORT tc_find_accept_port(PTCPPORT mport, RTIP_BOOLEAN take_off_flag)  /*__fn__*/
{
PTCPPORT sport;
PTCPPORT prev_port;

    sport = mport->next_slave;   /* get first slave */
    prev_port = mport;

    while (sport)
    {
        DEBUG_LOG("  tc_find_accept_port - check mother list: state,index", LEVEL_3, EBS_INT2, 
            sport->state, sport->ap.ctrl.index);
        if ( (sport->state == TCP_S_EST) || (sport->state == TCP_S_CWAIT) ||
             (sport->state == TCP_S_CLOSED) )
        {
            /* now that sport is accepted take it off the masters list   */
            if (take_off_flag)
            {
                DEBUG_LOG("tc_find_accept_port() - take slave off master list, index =", LEVEL_3, EBS_INT1,
                    sport->ap.ctrl.index, 0);
                prev_port->next_slave = sport->next_slave;
                sport->master_port = (PTCPPORT)0;
                sport->tcp_port_type = NOT_SLAVE_OR_MASTER;
                sport->from_state = 0;      /* don't let port return to  */
                                            /* listen state        */
                                            /* if an error occurrs */
            }

            /* if CLOSED by API this socket is gone so go see if   */
            /* there is another                                    */
            if ( (sport->state != TCP_S_CLOSED) ||
                  !(sport->ap.port_flags & API_CLOSE_DONE) )
                return(sport);
        }

        prev_port = sport;
        sport = sport->next_slave;  
    }
    return(sport);   /* return 0 */
}

/* ********************************************************************   */
/* wait_connection() - wait for an EST connection                         */
/*                                                                        */
/* NOTE: enter with TCP sem not claimed;                                  */
/*       if slave port found exit with it claimed                         */
/*       else if error exit with it not claimed                           */
/*                                                                        */
PTCPPORT wait_connection(PTCPPORT mport)
{
PTCPPORT sport;

    sport = (PTCPPORT)0;
    for (;;)
    {
        /* wait for signal slave is available                                    */
        /* NOTE: there is one signal for each time a slave goes into a connected */
        /*       state (or error) so no clearing of signals should be done       */
        /* if blocking port wait for port to be in connected state               */
        /* Note: port is linked into the interface, read_sem will be set by      */
        /* tc_tcp_interpret(). We wait for the sessin to be established          */
        /* NOTE: for transition to EST state, the master port will be signalled  */
        /*       since accept does not know which of the slave ports to wait     */
        /*       for a signal from                                               */
        DEBUG_LOG("wait_connection() - wait on signal - index =", LEVEL_3, EBS_INT1, 
            mport->ap.ctrl.index, 0);
        if ( (mport->ap.options & IO_BLOCK_OPT) )
        {
            if (!OS_TEST_READ_SIGNAL((PANYPORT)mport, (word)RTIP_INF))
            {
                set_errno(ETIMEDOUT);   /* should never happen */
                break;
            }

            /* if signalled failure; try again                         */
            /* but if another thread call closesocket() then return    */
            if (mport->ap.ctrl.read_status )
            {
                if ( mport->ap.port_flags & API_CLOSE_DONE )
                {
                    set_errno( mport->ap.ctrl.rtip_errno );
                    break;
                }

                /* if signalled failure; try again   */
                continue;
            }

            DEBUG_LOG("wait_connection() - signal received", LEVEL_3, NOVAR, 0, 0);
        }

        OS_CLAIM_TCP(ACCEPT_CLAIM_TCP)

        /* now that master has been signaled, find a port in the EST or CWAIT state   */
        /* NOTE: could be in CWAIT if got a FIN before accept() done                  */
        /* NOTE: could be in CLOSED if error occurred                                 */
        sport = tc_find_accept_port(mport, TAKE_SLAVE_OFF);

        /* if found a connection; return success    */
        /* NOTE: sem is still claimed               */
        if (sport)
            break;

        /* if did not find a slave port in EST,CWAIT or CLOSED state return FAIL   */
        else
        {
            DEBUG_LOG("wait_connection() - no slave port in EST, master = ", LEVEL_3, EBS_INT1,
                    mport->ap.ctrl.index, 0);

            OS_RELEASE_TCP()                /* Release access to tcp resources */

            /* if non-blocking mode, return error EWOULDBLOCK;    */
            /* otherwise the errno value was above                */
            if ( !(mport->ap.options & IO_BLOCK_OPT) )
            {
                set_errno(EWOULDBLOCK);
                break;
            }

            /* if here then got a signal but no socket was available;           */
            /* this error could occur if a reset or another error condition     */
            /* occurred after the port was established; when the error occurred */
            /* the port had not been accepted yet so it returned to the         */
            /* listen state or was aborted if there was already a port          */
            /* in the listen state; the signal was from the transition to       */
            /* established and not from the error condition;                    */
            /* so go block again                                                */
            DEBUG_LOG("wait_connection() - wait_connection - no port found - try again", 
                LEVEL_3, NOVAR, 0, 0);
        }
    }
    return(sport);
}


/* ********************************************************************   */
/* tc_tcp_accept(PTCPPORT port) - wait for a connection                   */
/*                                                                        */
/*  Wait for a connection at this tcp port to become established.         */
/*                                                                        */
/*  Inputs                                                                */
/*      PORT        - from socket                                         */
/*                                                                        */
/*  The local port should have first been set by calling bind and listen  */
/*                                                                        */
/*  called by accept()                                                    */
/*                                                                        */
/*  Returns connected port or 0 if an error is detected.                  */
/*  Sets errno if an error is detected.                                   */
/*                                                                        */

PTCPPORT tc_tcp_accept(PTCPPORT mport)       /*__fn__*/
{
PTCPPORT sport = 0;

#if (INCLUDE_POSTMESSAGE)
    mport->ap.post_flags |= ACCEPT_OR_CONNECT_CALLED;
#endif

    /* **************************************************                    */
    /* ACCEPT : perform the following:                                       */
    /*      wait for read signal                                             */
    /*      search master list for slave in EST state                        */
    /*      if find one                                                      */
    /*          SLAVE port - take off master list                            */
    /*                       set port type to NOT_SLAVE_OR_MASTER            */
    /*          MASTER port - if (tcp_port_type = MASTER_NO_LISTENS)         */
    /*                          (i.e. interpret could not add port since too */
    /*                                many listens in progress)              */
    /*                           tc_tcp_listen(1) - add slave to master      */
    /*                        else                                           */
    /*                           tcp_port_type++                             */
    /* **************************************************                    */

#    if (DEBUG_WEB)
        DEBUG_ERROR("accept enter - port type, list type(master=5) = ", EBS_INT2, 
            mport->tcp_port_type, mport->ap.list_type);
#    endif

    /* if listen() has not been called, return error   */
    if (mport->tcp_port_type < MASTER_NO_LISTENS)
    {
        set_errno(EINVAL);
        return(sport);
    }

    /* wait for an established connection or error                  */
    /* NOTE: if find a slave then wait_connection returns with tcp  */
    /*       sem claimed                                            */
    sport = wait_connection(mport);
    if (!sport)
        return(sport);

    /* if previously could not add new port during tc_tcp_interpret()   */
    /* it can be added now                                              */
    if ( (mport->tcp_port_type == MASTER_NO_LISTENS) ||
         (mport->ap.list_type == MASTER_LIST) )
    {
        if (mport->tcp_port_type == MASTER_NO_LISTENS) 
            mport->tcp_port_type = 1;
        else
            mport->tcp_port_type++;

#        if (DEBUG_WEB)
            DEBUG_ERROR("accept - set to MASTER_NO_LISTENS - call listen - port type = ",
                EBS_INT1, mport->tcp_port_type, 0);
#        endif
        if (tc_tcp_listen(mport, mport->tcp_port_type, TRUE))
        {
            DEBUG_ERROR("accept - tc_tcp_listen() failed - errno, tcp_port_type = ",
                    EBS_INT2, xn_getlasterror(), mport->tcp_port_type);

            /* move from socket list to master list with no backlog   */
            /* listerner list; then when sockets are freed another    */
            /* attempt will be made to allocate a backlog listener    */
            delete_tcpport_list(mport);
            add_tcpport_list(mport, MASTER_LIST);

#if (DEBUG_WEB)
            listen_fail++;
#endif
        }
        else
        {
            fix_master(mport);
        }
#        if (DEBUG_WEB)
            DEBUG_ERROR("accept - after listen - added one - port type = ",
                EBS_INT1, mport->tcp_port_type, 0);
#        endif
    }
    else
    {
        mport->tcp_port_type++;
    }

    OS_RELEASE_TCP()                /* Release access to tcp resources */

#    if (DEBUG_WEB)
        DEBUG_ERROR("after accept - port type = ",EBS_INT1, mport->tcp_port_type, 0);
#    endif

#if (INCLUDE_POSTMESSAGE)
    /* if another connection available to accept   */
    if (sport && tc_find_accept_port(mport, NO_TAKE_SLAVE_OFF))
        PostMessage(mport->ap.hwnd, mport->ap.ctrl.index, FD_ACCEPT, 0);
#endif
    return(sport);  
}



/* ********************************************************************   */
/* tc_tcp_connect() - establish a TCP connection                          */
/*                                                                        */
/*  Establish a connection on this tcp port, i.e. sends SYNC message and  */
/*  waits for TCP layer to complete the connection.                       */
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

int tc_tcp_connect(PTCPPORT port, PFBYTE to, word to_port)  /*__fn__*/
{
byte   ip_src[IP_ALEN];
int    tc_errno;
#if (INCLUDE_POSTMESSAGE)
    port->ap.post_flags |= ACCEPT_OR_CONNECT_CALLED;
#endif

    /* if socket already connected   */
    if (!tc_cmp4((port->out_ip_temp.ip_dest), ip_nulladdr, 4)) 
    {
        if ( !(port->ap.options & IO_BLOCK_OPT) &&
             (port->state != TCP_S_EST) && (port->state != TCP_S_CWAIT) &&
             (port->state != TCP_S_CLOSED) )
        {
            tc_errno = EINPROGRESS;     /* same as EWOULDBLOCK */
            goto write_error_exit;
        }
        tc_errno = EISCONN;
        goto write_error_exit;
    }

    /* allocate input and output windows - tbd maybe don't do this yet   */
    /* NOTE: must be done before state change                            */
    setup_both_windows(port);

    OS_CLAIM_TCP(CONNECT_CLAIM_TCP)  /* Exclusive access to tcp resources */

    /* save interface info for output; initializes tcp connect info   */
    if (!tcpinit(port, to))     /* if could not find an output iface */
    {
        OS_RELEASE_TCP()                /* Release access to tcp resources */
        INCR_SNMP(IpOutNoRoutes)
        tc_errno = EADDRNOTAVAIL;
        goto write_error_exit;
    }

    /* based upon destination IP address, bind the port   */
    /* NOTE: bind expects port in network byte order      */
    if (!(port->ap.port_flags & PORT_BOUND))   /* if not bound already */
    {
        if (!tc_get_src_ip((PFBYTE)ip_src, to))
        {
            OS_RELEASE_TCP()                /* Release access to tcp resources */
            tc_errno = EADDRNOTAVAIL;
            goto write_error_exit;
        }

        /* bind with "unique" port number assigned by socket()   */
        if (tc_tcp_bind(port, (PFBYTE)ip_src, net2hs(port->in.port), 
                           TRUE) < 0)
            return(-1);
    }

    /* set destination IP address in output message   */
    tc_mv4(port->out_ip_temp.ip_dest, to, IP_ALEN);

    /* set destination TCP port in output message   */
    port->out_template.tcp_dest = to_port;   /* for example, telnet=23  */
                                             /* keep in network byte order   */
    port->out.port = hs2net(to_port);        /* service is same as port num */
                                             /* keep in host byte order   */

    /* transition to sync sent TCP state   */
    trans_state(port, TCP_S_SYNS, NOT_FROM_INTERPRET, FALSE);
    INCR_SNMP(TcpActiveOpens)

    /* set port type i.e. it is not a master or a slave port but is a   */
    /* port that did an active open                                     */
    port->tcp_port_type = NOT_SLAVE_OR_MASTER;

    /* Link the port into the interface's list of active ports   */
    delete_tcpport_list(port);  
    add_tcpport_list(port, ACTIVE_LIST);

    /* send initial sync msg which will install maximum segment size    */
    /* which will be sent out in the first                              */
    /* NOTE: state must be set before calling tc_tcpsend                */
    OS_CLEAR_WRITE_SIGNAL((PANYPORT)port);    
    DEBUG_LOG("connect() - ->TCP_S_SYNS - send_sync,clear sincetime", LEVEL_3, NOVAR, 0, 0);
    tc_errno = tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, NORMAL_SEND, 
                       TCP_F_SYN);

    /* let select() know connection is in progress     */
    if (!(port->ap.options & IO_BLOCK_OPT))
        port->ap.port_flags |= TCP_CONN_IN_PROG;

    OS_RELEASE_TCP()                /* Release access to tcp resources */

    /* if send failed, return error   */
    if (tc_errno)
        goto write_error_exit_ip;

    /* **************************************************                      */
    /* if port is blocking, wait for connections to be established             */
    /* wait until ack to sync cmd is received, timeout or error; read+sig will */
    /* be set by tc_tcp_interpret()                                            */
    /* tbd - check for nonblocking - only after select done                    */
    /* NOTE: there is one signal sent when connection established so no        */
    /*       clearing of signal should be done                                 */
    if (port->ap.options & IO_BLOCK_OPT)
    {
        DEBUG_LOG("connect() - wait for read signal - tcp sem released", LEVEL_3, NOVAR, 0, 0);
        if (!OS_TEST_WRITE_SIGNAL((PANYPORT)port, (word)RTIP_INF))
            tc_errno = ETIMEDOUT;   /* should never happen */
        else
            tc_errno = port->ap.ctrl.write_status;
        if (tc_errno)  /* timeout or if signalled failure due to reset or  */
                    /* syn rcvd   */
        {
            if (tc_errno == ENOTCONN)  /* if signalled failure due to reset or syn rcvd */
                tc_errno = ECONNREFUSED;
            goto write_error_exit_ip;
        }

        /* double check state; should never get case where signaled successfully   */
        /* without correct status;                                                 */
        /* NOTE: could be CWAIT if remote has already sent FIN                     */
        DEBUG_LOG("connect() - signal read received - state is", LEVEL_3, EBS_INT2,  
            port->state, port->ap.ctrl.index);
        if ( (port->state == TCP_S_EST) || (port->state == TCP_S_CWAIT) )
            return(0);   /* success */
        else
        {
            /* set to closed so timeout will not keep retrying even if num    */
            /* retries not done                                               */
            trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, TRUE);
            tc_errno = ECONNREFUSED; 
            goto write_error_exit_ip;
        }
    }

    /* port is non-blocking so do not block but                       */
    /* - tell select so if called it can wait for connection          */
    /*   to be established                                            */
    /* - return error with EINPROGRESS;                               */
    /*   NOTE: this is not really an error but this is the way        */
    /*         the BSD requirements say this should be handled;       */
    /*         the connection is still in progress and select should  */
    /*         be used to wait until the connection is established    */
    /*   NOTE: WINSOCK requirements say the errno should be           */
    /*         EWOULDBLOCK                                            */
    /*   NOTE: the connection may have been established at this point */
    /*         and TCP state is TCP_S_EST                             */
    /*         Otherwise, we leave TCP_CONN_IN_PROG on                */
    if (port->state == TCP_S_EST) 
    {
        port->ap.port_flags &= ~TCP_CONN_IN_PROG;
        return 0;
    }
    tc_errno = EINPROGRESS;     /* same as EWOULDBLOCK */
    goto write_error_exit;

write_error_exit_ip:
    tc_mv4(port->out_ip_temp.ip_dest, ip_nulladdr, 4);
write_error_exit:
    return(set_errno(tc_errno));    
}


/* ********************************************************************    */
/* tc_tcp_pk_peer_address() - gets othersides IP and port address          */
/*                                                                         */
/*   Extracts port number and IP address of otherside connected to from    */
/*   the output message template and stores them in host_ip and host_port. */
/*                                                                         */
/*   Returns nothing                                                       */
/*                                                                         */

void tc_tcp_pk_peer_address(PTCPPORT port, PFBYTE host_ip, PFINT host_port)  /*__fn__*/
{
    tc_mv4(host_ip, port->out_ip_temp.ip_dest, IP_ALEN);
    *host_port =  (int)port->out_template.tcp_dest;    /* keep in network byte order */
}

/* ********************************************************************   */
/* tc_proc_dequeue() - dequeue data from window and update window size    */
/*                                                                        */
/*   Dequeues at most n bytes from input window.  Updates input           */
/*   window size and based upon silly window syndrome avoidance           */
/*   algorithm, sends new window size to other side                       */
/*                                                                        */
/*   Returns number of bytes dequeued, or -1 if error.  If error is       */
/*   detected, sets errno.                                                */
/*                                                                        */
int tc_proc_dequeue(PTCPPORT port, PFBYTE buf, int n, DCU KS_FAR *pmsg, int flags)  /*__fn__*/
{
int  howmany;
word lowwater;
int  wind_incr;
int  status;
word mss;

#if (!INCLUDE_PKT_API)
    ARGSUSED_PVOID(pmsg);
#else
    if (pmsg)
    {
        *pmsg = dequeue_pkt((PWINDOW)&port->in, flags);
        howmany = pkt_data_size(*pmsg);
    }
    else
#endif
        howmany = dequeue((PWINDOW)&port->in, buf, n, flags);

    /* if just returning data queued without dequeueing   */
    if (flags & MSG_PEEK)
        return(howmany);

    port->in.size = (word)(port->in.size + howmany);   
                    /* increment our window size by # just dequeued   */

    /* **************************************************                    */
    /* check if need to wake other side with new window size                 */
    /* SWS - receiver only advertises a larger window than it is currently   */
    /*       advertising if the window can be increased by either mss        */
    /*       sent by other side or 1/2 our buffer size, whichever is smaller */
    /* first calculate amount window needs to increase by                    */
#if (DEBUG_SWS)
    DEBUG_ERROR("tc_proc_dequeue - port->in.window_size, port->in.mss = ", 
        LINT2, port->in.window_size, port->in.mss);
#endif
    lowwater = (word)(port->in.window_size>>1); /* 1/2 input window size */
    mss = port->in.mss;                         /* input MSS */
    if (mss < lowwater)
        lowwater = mss;

    /* calculate amount window increased from amount advertising   */
    wind_incr = port->in.size - port->in.ad_size;

    /* check if should up window size advertising, and if so send new   */
    /* window size to other side                                        */
    if (wind_incr >= (int)lowwater) 
    {
        DEBUG_LOG("tc_proc_dequeue: update old: port->in.ad_size = ", LEVEL_3, DINT1,
            port->in.ad_size, 0);
        DEBUG_LOG("tc_proc_dequeue: new: in.ad_size, in.contain = ", LEVEL_3, DINT2,
            port->in.size, port->in.contain);
        port->in.ad_size = port->in.size;

#        if (DEBUG_DACK)
            DEBUG_ERROR("send windowsize update, wind_incr, lowwater =", EBS_INT2, 
                wind_incr, lowwater);
#        endif

        /* socket may be closed but still reading data from the socket;   */
        /* if in that state do not send to the remote host                */
        if ((port->state != TCP_S_CLOSED)   &&
            (port->state != TCP_S_TWAIT)    &&
            (port->state != TCP_S_LAST) )
        {
            status = tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, NORMAL_SEND, 
                                TCP_F_ACK);

            /* ignore any send errors - they need to be ignored since data   */
            /* has been dequeued; if return an error, application will not   */
            /* get the data that was dequeued                                */
            if (status)
            {
                DEBUG_ERROR("tc_proc_dequeue - send failed, errno = ", EBS_INT1,
                    status, 0);
                /* set_errno(status);   */
                /* return(-1);          */
            }
        }
    }

    return(howmany);
}


/* ********************************************************************        */
/* int tc_tcp_read() - reads data from input window and stores in buffer       */
/*                                                                             */
/*  Reads up to n bytes of data from input window and stores in buffer.        */
/*  If no data is available, blocks until data becomes available and fills     */
/*  the buffer with whatever data if available.  If less data is available     */
/*  then requested, it still returns what is available.                        */
/*                                                                             */
/*  After the data has been dequeued from the input window, it sends an        */
/*  updated window size to the otherside as long as it will not cause          */
/*  silly window syndrome.                                                     */
/*                                                                             */
/*  Returns number of bytes read if successful or -1 if an error was detected. */
/*  Sets errno if an error is detected.                                        */
/*                                                                             */

int tc_tcp_read(PTCPPORT port, PFBYTE buf, int n, DCU KS_FAR *pmsg,     /*__fn__*/
                int flags, word wait_count)                             /*__fn__*/
{
int howmany;
int err;

    DEBUG_LOG("tc_tcp_read input window contains bytes =", LEVEL_3, EBS_INT1, port->in.contain, 0);
    OS_CLAIM_TCP(READ1_CLAIM_TCP)   /* Exclusive access to tcp resources */

    /* **************************************************   */
    /* check for legal states for read                      */
    if ( !(port->ap.options & IO_BLOCK_OPT) )
    {
        if ( (port->state == TCP_S_SYNR) ||
             (port->state == TCP_S_SYNS) )
        {
            err = EWOULDBLOCK;
            goto read_error_exit_release;
        }
    }

    if (!tc_is_read_state(port))
    {
        DEBUG_LOG("tc_tcp_read() - illegal state for read, state,index = ", LEVEL_3, EBS_INT2, 
                port->state, port->ap.ctrl.index);
        /* NOTE: clearing of signal needs to be done since                     */
        /*       - there is one signal each time a packet comes in, therefore, */
        /*         there is not a one-to-one correspondance between packets    */
        /*         coming in and read since the read size does not need to     */
        /*         match the packet size;                                      */
        /*       - signal is only blocked upon if there is not data in input   */
        /*         window;                                                     */
        OS_CLEAR_READ_SIGNAL((PANYPORT)port);
        err = ENOTCONN;
        goto read_error_exit_release;
    }

    /* **************************************************                 */
    /* only wait for data if input window is empty and not in CWAIT state */
    /* (CWAIT is entered when fin is received, i.e. no more input pkts)   */
    /* NOTE: the while guards against bogus read signals                  */
    /*                                                                    */
    /* NOTE: if CWAIT than a FIN was received and no more data            */
    /*       will be coming in, therefore, request must be satisfied      */
    /*       with available data; NOTE: tc_tcp_interpret will             */
    /*       signal read when FIN has arrived                             */
    /* NOTE: an abort or receipt of a reset msg could change state        */
    /*       in middle and signal reader                                  */
    while ( (!port->in.contain) && 
            (tc_is_read_state(port) &&
             (port->state != TCP_S_CWAIT)) )
    {
        /* the buffer is empty, wait    */
        OS_RELEASE_TCP()                /* Release access to tcp resources */

        /* if no block return error   */
        if ( !(port->ap.options & IO_BLOCK_OPT) )
        {
            err = EWOULDBLOCK;  
            goto read_error_exit;
        }

        /* wait for data to be available; if MSG_WAITALL (and sockets call),   */
        /* wait until enough data is available to satisfy the request          */
        do 
        {
            /* wait for TCP task to signal data available, error or timeout   */
            /*OS*/ /* DEBUG_ERROR("TCP - go into block", NOVAR, 0, 0); */
            /*OS*/ DEBUG_ERROR("\nTCP - go into block: port=, wait_count=", EBS_INT2, (PANYPORT)port, wait_count);
            if (!OS_TEST_READ_SIGNAL((PANYPORT)port, wait_count))
            {
                DEBUG_ERROR("tc_tcp_read - timeout - in.contain, in.nxt = ",
                    DINT2, port->in.contain, port->in.nxt);
                    DEBUG_LOG("tc_tcp_write - signal timedout", LEVEL_3, NOVAR, 0, 0);
                if (port->ap.options & SOO_RCV_TIMEO)
                {
                    if (howmany > 0)
                        return(howmany);
                    err = ETIMEDOUT;
                    goto read_error_exit;
                }
                err = ETIMEDOUT;
                goto read_error_exit;
            }
            DEBUG_ERROR("\nTCP - out of block", NOVAR, 0, 0);
            if (port->ap.ctrl.read_status)   /* if signalled failure */
            {
                err = port->ap.ctrl.read_status;
                goto read_error_exit;
            }

            DEBUG_LOG("tc_tcp_read() - signaled", LEVEL_3, NOVAR, 0, 0);
        } while ( buf && (flags & MSG_WAITALL) && (port->in.contain < (word)n) );

        OS_CLAIM_TCP(READ2_CLAIM_TCP)  /* Exclusive access to tcp resources */
    }

    /* **************************************************    */
    /* illegal state for READ (xn_abort or reciept           */
    /* of a reset msg could have changed state) return error */
    if (!tc_is_read_state(port))
    {
        DEBUG_LOG("tc_tcp_read - bad read state - return", LEVEL_3, NOVAR, 0, 0);
        err = ENOTCONN; 
        goto read_error_exit_release;
    }

    /* if other side closed (CWAIT) and there is no data                  */
    /* in input, return 0 (end-of_file); if there is data then return the */
    /* data available                                                     */
    /* NOTE: CWAIT will not accept any more data but there still          */
    /*       could be some in input window which has not been             */
    /*       read yet                                                     */
    /* NOTE: in.credit was already set to 0 in CWAIT                      */
    /* NOTE: this code is here since don't want to call tc_proc_dequeue   */
    /*       which will advertize window size                             */
    if (port->state == TCP_S_CWAIT) 
    {
#if (INCLUDE_PKT_API)
        if (pmsg)
        {
            *pmsg = dequeue_pkt((PWINDOW)&port->in, flags);
            howmany = xn_pkt_data_size(*pmsg, FALSE);
        }
        else
#endif
            howmany = dequeue((PWINDOW)&port->in, buf, n, flags);
        DEBUG_LOG("tc_tcp_read(CWAIT) - dequeued bytes = ", LEVEL_3, EBS_INT1, howmany, 0);
        if (!(flags & MSG_PEEK))
        {
            port->in.size = (word)(port->in.size + howmany);   
                        /* increment our window size by # just dequeued   */
        }
        OS_RELEASE_TCP()

#if (INCLUDE_POSTMESSAGE)
        /* after read if data is still available after read (with or   */
        /* without MSG_PEEK), the Post Message                         */
        /* NOTE: dequeue updated in.contain                            */
        if (port->in.contain)
        {
            PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_READ, 0);
        }
        else
        {
            if (port->ap.port_flags & WAIT_IN_WIN_EMPTY)
            {
                PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_CLOSE, 0);
                port->ap.port_flags &= ~WAIT_IN_WIN_EMPTY;
            }
        }
#endif
        return(howmany);            /* possibly eof */
    }

    /* **************************************************                 */
    /* If we get here tcp is claimed and there is data and not in CWAIT,  */
    /* therefore, dequeue it                                              */
    /* NOTE: possiby will set err                                         */
    howmany = tc_proc_dequeue(port, buf, n, pmsg, flags);
    DEBUG_LOG("tc_tcp_read(not CWAIT) - dequeued bytes = ", LEVEL_3, EBS_INT1, howmany, 0);

    /* **************************************************                     */
    /* clear the signal                                                       */
    /* NOTE: there could be more than 1 signal per read if the other side     */
    /*       is sending data in smaller chunks then this side is reading      */
    /*       them; in case clear too many signals (i.e. clears signal for     */
    /*       data in buffer which hasn't been read, the code will never       */
    /*       get stuck in situation where data is in buffer but it doesn't    */
    /*       know it due to cleared signal since loop above checks in.contain */
    /*       and only uses the signal to block in case there is no data in    */
    /*       the buffer                                                       */
    OS_CLEAR_READ_SIGNAL((PANYPORT)port);

    OS_RELEASE_TCP()                /* Release access to tcp resources */

#if (INCLUDE_POSTMESSAGE)
    if (port->in.contain)
    {
        PostMessage(port->ap.hwnd, port->ap.ctrl.index, FD_READ, 0);
    }
#endif
    return(howmany);

read_error_exit_release:
    OS_RELEASE_TCP()                /* Release access to tcp resources */
read_error_exit:
    return(set_errno(err)); 
}

/* ********************************************************************           */
/* int tc_tcp_write(PTCPPORT port, PFBYTE buf, int n, int flags, word wait_count) */
/*                                                                                */
/*   Write data into the output queue specified by port.                          */
/*   BLOCKING MODE:                                                               */
/*   if CFG_TCP_SEND_WAIT_ACK, waits for data to be acked.                        */
/*   if !CFG_TCP_SEND_WAIT_ACK, wait for                                          */
/*   acknowledgement that the data was delivered only if all the                  */
/*   data cannot be queued.                                                       */
/*                                                                                */
/*   NOTE: the data to be queued is either in buf                                 */
/*   (if from socket call) or in msg (if rtip api call), i.e.                     */
/*   either buf or msg has to be 0.                                               */
/*                                                                                */
/*   Note: wait_count determines how long to wait for each packet to be           */
/*         delivered, not the maximum time spent in the routine. The time         */
/*         spent inside the routine can be > wait_count.                          */
/*                                                                                */
/*   Note: always frees the packet (msg) if one is passed; if it enqueues         */
/*         the packet it will not be freed until the data is acked or the         */
/*         socket is closed.  RTIP must always free the packet even in            */
/*         error conditions because some errors are detected before the           */
/*         packet is queued and some are detected after.  Once the packet         */
/*         is queued, it will be freed eventually by RTIP.                        */
/*                                                                                */
/*   Returns number of bytes sent or < - 1 if n bytes were not acked.             */
/*   Sets errno if an error is detected.                                          */
/*                                                                                */

int tc_tcp_write(PTCPPORT port, PFBYTE buf, int n, DCU msg, int flags, 
                 word wait_count) /*__fn__*/
{
int  nqueued = 0;
int  before;
int  nleft;
int  tot_queued;
int  status;
RTIP_BOOLEAN queue_zero;        /* num bytes to queue is 0 */
int  err;
RTIP_BOOLEAN pkt_err;

    /* check for correct state     */
    if ( !(port->ap.options & IO_BLOCK_OPT) )
    {
        if ( (port->state == TCP_S_SYNR) ||
             (port->state == TCP_S_SYNS) )
        {
            err = EWOULDBLOCK;
            goto write_error_exit;
        }
    }

    if (!tc_is_write_state(port))
    {
        err = ENOTCONN;
        goto write_error_exit;
    }

    queue_zero = FALSE;
    if (!n)
        queue_zero = TRUE;

    /* for non-blocking sockets, connect set connection in progress flag;   */
    /* since the api is doing a write, turn the flag off so select() will   */
    /* check if the port is ready for writing instead of connected          */
    port->ap.port_flags &= ~TCP_CONN_IN_PROG;

#if (SEND_TCP_QUE_ALL)
    /* if no block and all the data will not fit in the buffer, do not   */
    /* queue any data and return error                                   */
    if ( !(port->ap.options & IO_BLOCK_OPT) &&
         (n > (port->out.window_size - port->out.contain)) )
    {
        err = EWOULDBLOCK;
        goto write_error_exit;
    }
#endif

    tot_queued = 0;
    nleft = n;

    OS_CLAIM_TCP(WRITE_CLAIM_TCP)  /* Exclusive access to tcp resources */

    /* **************************************************          */
    /* Consume all signals up to this point                        */
    /* NOTE: must clear write signal especially if wait_count is 0 */
    OS_CLEAR_WRITE_SIGNAL((PANYPORT)port);

    /* loop until all data acknowledged                          */
    /* NOTE: queue_zero will only be TRUE for first loop         */
    /* NOTE: if MSG_QUEUED then don't wait                       */
    /* OR if !CFG_TCP_SEND_WAIT_ACK,                             */
    /* loop until all data acknowledged or until all data queued */
    /* NOTE: queue_zero will only be TRUE for first loop         */

    while ( (tot_queued < n) || queue_zero || 
            (CFG_TCP_SEND_WAIT_ACK && 
             (port->out.contain > 0) && !(flags & MSG_QUEUE)) )
    {                                   
        if (!tc_is_write_state(port))
        {
            err = ENOTCONN;
            goto write_error_exit_rel;
        }

        before = port->out.contain;

        /* sockets can do no copy mode which means it just does one    */
        /* copy to the output pkt; also xn_pkt_send is always no       */
        /* copy mode                                                   */
        /* xn_pkt_send will send 0 for buffer since data is already in */
        /* a packet which is queued before calling this routine but    */
        /* send() always sends a buf                                   */
        if (!queue_zero)
        {
            pkt_err = FALSE;

#if (INCLUDE_PKT_API)
            if (!buf)
            {
                /* either queue entire msg in output window or don't   */
                /* queue any of it                                     */
                nqueued = enqueue_pkt_out(port->ap.iface, port, msg);  /* que all or none */
            }
            else
#endif
            {
                /* queue what will fit   */
                nqueued = enqueue_out(port, buf, nleft, &pkt_err);

                /* if ran out of DCUs and haven't queued any return error   */
                /* otherwise if blocking mode block and try again           */
                /* or non-blocking mode just return what is queued          */
                if ( pkt_err && (tot_queued <= 0) && (nqueued <= 0))
                {
                    err = ENOPKTS;
                    goto write_error_exit_rel;
                }
            }

            /* adjust pointers etc for next loop by amount just queued   */
            tot_queued += nqueued;
            nleft -= nqueued;
            buf += nqueued;

            DEBUG_LOG("tc_tcp_write() - enqueued data, tryed to que = ", LEVEL_3, EBS_INT2,  
                nqueued, nleft);
            DEBUG_LOG("tc_tcp_write - after adjustement: tot_queued = ", LEVEL_3, EBS_INT1, 
                tot_queued, 0);
        }

        /* if any data was queued, send it                                 */
        /* NOTE: SYNS and SYNR should only queue the data but not send it  */
        /*       until EST state (NOTE: see tc_tcpsend)                    */
        /* NOTE: for MSG_QUEUE only send it if couldn't queue all the data */
        if ( (!(flags & MSG_QUEUE) || tot_queued < n) && 
             ((nqueued > 0) || queue_zero) )
        {
            if ((port->state == TCP_S_EST) || (port->state == TCP_S_CWAIT)) 
            {
                /* transmit what was put into the buffer   */
                if (port->out.size)  /* if the othersides windowsize is not 0 */
                {
                    if (!before)
                        port->out.push = 1;
                    status = tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, 
                                        NORMAL_DATA_ONLY, TCP_F_ACK); 
                    if (status == ENOPKTS) 
                    {
                        TCP_SETUP_RETRANS(port)
                    }
                    else if (status)
                    {
                        /* send failed   */
                        err = status;
                        goto write_error_exit_rel;
                    }
                }

                /* if otherside has 0 input window size, do not send now; timeout   */
                /* should retry so enable the retry                                 */
                else
                {
                    DEBUG_LOG("tc_tcp_write - otherside window size = 0", LEVEL_3, NOVAR, 0, 0);
                    TCP_SETUP_RETRANS(port)
                }
            }           /* end of if EST or CWAIT */
        }               /* end of if data queued and option MSG_QUEUE */

        /* **************************************************                */
        /* wait until signal set by TCP task when all output bytes           */
        /* are acked, timeout or error                                       */
        /* NOTE: do not wait if all data has been sent or non-blocking, i.e. */
        /*       only wait if need to send more data and blocking            */
        /* NOTE: for MSG_QUEUE only wait if did not queue all the data       */
        /* block on all except last loop: wait only until all queued         */
        /* or                                                                */
        /* block on all loops: wait only until all acked                     */
        if ( (CFG_TCP_SEND_WAIT_ACK && !queue_zero && 
              (!(flags & MSG_QUEUE) || tot_queued < n)) ||
             (!CFG_TCP_SEND_WAIT_ACK && tot_queued < n) )
        {
            if ((port->ap.options & IO_BLOCK_OPT) ||
                (port->ap.options & SOO_SEND_TIMEO))
            {
                DEBUG_LOG("tc_tcp_write - wait for signal", LEVEL_3, NOVAR, 0, 0);

                OS_RELEASE_TCP()                /* Release access to tcp resources */

                if (!OS_TEST_WRITE_SIGNAL((PANYPORT)port, wait_count))
                {
                    DEBUG_LOG("tc_tcp_write - signal timedout", LEVEL_3, NOVAR, 0, 0);
                    if (port->ap.options & SOO_SEND_TIMEO)
                    {
                        if (tot_queued > 0)
                            return(tot_queued);
                        err = ETIMEDOUT;
                        goto write_error_exit;
                    }
                    err = ETIMEDOUT;
                    goto write_error_exit;
                }
                if (port->ap.ctrl.write_status)   /* if signalled failure */
                {
                    err = port->ap.ctrl.write_status;
                    goto write_error_exit;
                }

                OS_CLAIM_TCP(WRITE_CLAIM_TCP)  /* Exclusive access to tcp resources */

                DEBUG_LOG("tc_tcp_write - signal received", LEVEL_3, NOVAR, 0, 0);
            }
            else
            {
#            if (SEND_TCP_QUE_ALL)
                /* no room in window; check done above so probably   */
                /* not necessary here also                           */
                err = EWOULDBLOCK;
                goto write_error_exit_rel;
#            else
                /* tbd: is this right?   */
                if (tot_queued <= 0)
                {
                    err = EWOULDBLOCK;
                    goto write_error_exit_rel;
                }
                break;     /* return what can be queued */
#            endif                     
            }
        }

        /* **************************************************          */
        /* state could change due to reset msg or xn_abort so check it */
        /* again                                                       */
        if (!tc_is_write_state(port))
        {
            DEBUG_LOG("tc_tcp_write - bad write state - return", LEVEL_3, NOVAR, 0, 0);
            err = ENOTCONN;
            goto write_error_exit_rel;
        }

        queue_zero = FALSE;     /* only do loop once if queue_zero */
    }       /* end of while loop queueing/sending data */

    OS_RELEASE_TCP()                /* Release access to tcp resources */

#if (INCLUDE_POSTMESSAGE)
    port->ap.post_flags &= ~EWOULDBLOCK_WRITE;
#endif

    /* return amount queued    */
    return(tot_queued);

write_error_exit_rel:
    OS_RELEASE_TCP()                /* Release access to tcp resources */
write_error_exit:
    if ( msg && (nqueued == 0) )
        os_free_packet(msg);
#if (INCLUDE_POSTMESSAGE)
    if (err == EWOULDBLOCK)
        port->ap.post_flags |= EWOULDBLOCK_WRITE;
#endif
    return(set_errno(err));  
}


/* ********************************************************************   */
/* void tcp_abort(PTCPPORT port) - Abort a tcp connection                 */
/*                                                                        */
/*   Ungracefully closes the port.  tc_tcp_close() will be called by      */
/*   timeout routine to finish the close (i.e. free the resources).       */
/*                                                                        */
/*   Assumes TCP sem claimed when called.                                 */
/*                                                                        */
/*   Returns NOTHING                                                      */
/*                                                                        */
void tcp_abort(PTCPPORT port, RTIP_BOOLEAN tcp_send_reset)             /*__fn__*/
{
    DEBUG_LOG("tcp_abort entered, state = ", LEVEL_3, EBS_INT1, port->state, 0);

    /* if user request to send a reset to the remote host, do it   */
    if ( tcp_send_reset && port->ap.iface && port->ap.iface->open_count &&
         (port->state != TCP_S_CLOSED) && (port->state != TCP_S_TWAIT) )
    {
        tc_tcpsend(port->ap.iface, port, NO_DCU_FLAGS, NORMAL_SEND, 
                   TCP_F_RESET);
        DEBUG_LOG("tcp_abort: send reset", LEVEL_3, NOVAR, 0, 0);
    }

    /* perform abort for all states; all states except ALLOCED will   */
    /* transition to CLOSED; at next tcp timeout tcp_close will       */
    /* cleanup the socket and free it                                 */
    switch (port->state)
    {
        default:
        case TCP_S_CLOSED:
        case TCP_S_SYNR:                    /* Syn rcvd. Wait for ack */
        case TCP_S_EST:                     /* normal data trans. */
        case TCP_S_LAST:                    /* FIN sent waiting for ACK */
        case TCP_S_FW1:                     /* waiting for ACK of FIN  */
        case TCP_S_FW2:                     /* want FIN  */
        case TCP_S_CWAIT:                   /* FIN received */
            port->out.contain = 0;
            /* NOTE: fall thru     */

        case TCP_S_ALLOCED:     
        case TCP_S_LISTEN:                  /* passive open */
        case TCP_S_TWAIT:                   /* closed and waiting */
        case TCP_S_SYNS:                    /* Active open */
        case TCP_S_CLOSING:                 /* want ACK of FIN  */
            /* go to closed and discard data in input windows which   */
            /* includes freeing DCUs in input window                  */
            trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, TRUE);
            break;
    }

    /* **************************************************           */
    /* If anyone is waiting (readers, writers, connect or listen),  */
    /* signal error                                                 */
    wake_up(port, NOT_FROM_INTERPRET, 
            READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ENOTCONN);
}                                   

/* ********************************************************************   */
/* tc_tcp_abort() - Aborts a TCP connection and frees port structure      */
/*                                                                        */
/*   Aborts a TCP connection by setting state to closed and frees         */
/*   port structure.                                                      */
/*                                                                        */
/*   ASSUMPTION: this routine is only called by API (socket or rtip)      */
/*                                                                        */
/*   Returns nothing                                                      */
/*                                                                        */

void tc_tcp_abort(PTCPPORT port, RTIP_BOOLEAN tcp_send_reset)         /*__fn__*/
{
PTCPPORT sport;
PTCPPORT nport;

    DEBUG_LOG("tc_tcp_abort entered, state = ", LEVEL_3, EBS_INT1, port->state, 0);
    /* DEBUG_LOG("PORTS", LEVEL_3, PORTS_TCP, 0, 0);   */


    OS_CLAIM_TCP(ABORT_CLAIM_TCP)   /* Exclusive access to tcp resources */

    /* if closing master port   */
    if (port->tcp_port_type >= MASTER_NO_LISTENS)
    {
        /* loop closing all slave ports   */
        sport = port->next_slave;   /* get first slave */
        while (sport)
        {
            DEBUG_LOG("tc_tcp_abort() - abort port and master indexes =", LEVEL_3, EBS_INT2, 
                sport->ap.ctrl.index, port->ap.ctrl.index);
                
            /* take off front of master list                           */
            /* MUST task off master list since tcp_abort might release */
            /* and reclaim TCP semaphore                               */
            nport = sport->next_slave;  
            port->next_slave = nport;   /* make next port head of list */

            /* set flag so know api abortd port (done so know if can release   */
            /* resources)                                                      */
            /* NOTE: assumes this routine only called by api                   */
            sport->ap.port_flags |= API_CLOSE_DONE;
            tcp_abort(sport, tcp_send_reset);

            sport = nport;

        }

        /* abort the master port; it will be in alloced state   */
        port->ap.port_flags |= API_CLOSE_DONE;
        tcp_abort(port, tcp_send_reset);
    }

    /* not aborting a master port so just abort the port   */
    else
    {
        DEBUG_LOG("tc_tcp_abort() - abort port, index =", LEVEL_3, EBS_INT1,
            port->ap.ctrl.index, 0);
        port->ap.port_flags |= API_CLOSE_DONE;
        tcp_abort(port, tcp_send_reset);
    }

    OS_RELEASE_TCP()                /* Release access to tcp resources */
}

/* ********************************************************************         */
/* tcp_close() - perform close processing on a port                             */
/*                                                                              */
/*   Starts the closing process on port (i.e. sends FIN to other side).         */
/*   It will be called again by timeout routine to finish the close.            */
/*   NOTE: this routine needs to be called with TCP semaphore claimed           */
/*   NOTE: this routine might release and reclaim TCP semaphore                 */
/*                                                                              */
/*   Assumes TCP sem claimed when called.                                       */
/*                                                                              */
/*   Returns -1(PORT_FREED) if port taken off active list, 0(CLOSE_SUCCESS) if  */
/*   it was not and no error was detected or errno if port not taken off        */
/*   active list and an error was detected.                                     */

int tcp_close(PTCPPORT port)                                 /*__fn__*/
{
word new_state;   /* state to transition to */
word wait_count;
int  ret_val;
PIFACE pi;
  
    DEBUG_LOG("tcp_close() entered, state = ", LEVEL_3, EBS_INT1, port->state, 0);

#if (INCLUDE_POSTMESSAGE)
    /* Don't think this code should be here since it is supposed   */
    /* to be after local system initiates gracefule close with     */
    /* shutdown                                                    */
    if (port->state != TCP_S_CLOSED)
    {
        port->ap.post_flags |= GRACEFUL_CLOSE;
        DEBUG_ERROR("set GRACEFUL_CLOSE", EBS_INT1, port->ap.ctrl.index, 0);
    }
#endif

    ret_val = CLOSE_SUCCESS;    /* success but not taken off active list */

    new_state = TCP_S_LAST;     /* assume called from CWAIT */

    /* NOTE: this code only done for xn_close() since abort only calls   */
    /*       in CLOSED or ALLOCED state                                  */
    switch (port->state)
    {
        case TCP_S_ALLOCED:     
        case TCP_S_LISTEN:                 
            /* must wake up another thread that may be in accept()   */
            if( port->tcp_port_type >= MASTER_NO_LISTENS )
            {
                /* If anyone is waiting (readers, writers, connect or listen),     */
                /* signal error                                                    */
                wake_up(port, NOT_FROM_INTERPRET, 
                        READ_WAKEUP_FAIL|WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ENOTSOCK);
            }

            trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, TRUE);
            break;

        case TCP_S_SYNS:
            trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, TRUE);

            /* If connect is waiting signal error   */
            wake_up(port, NOT_FROM_INTERPRET, 
                    WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ENOTCONN);
            break;

         case TCP_S_EST:                    /* must initiate close  */
            /* set state to transition to if all output data has been acked   */
            new_state = TCP_S_FW1;          /* wait for ACK of FIN  */

            /* NOTE: fall thru   */

         case TCP_S_CWAIT:                 /* other side already closed  */
            /* ******                                                       */
            /* GRACEFUL CLOSE WITH BLOCK                                    */
            /* if data in output window and linger option with seconds then */
            /* use select to wait until output window empties;              */
            /* if timeout occurs and output window does not empty, send     */
            /* FIN anyway                                                   */
            if ( (port->out.contain) && 
                 (port->ap.options & SO_LINGER) && (port->ap.linger_secs) )
            {
                /* if non-blocking mode, return error EWOULDBLOCK   */
                if ( !(port->ap.options & IO_BLOCK_OPT) )
                {
                    return(set_errno(EWOULDBLOCK));
                }

                OS_CLEAR_WRITE_SIGNAL((PANYPORT)port);

                /* wait for check_ack to signal output window empty       */
                /* NOTE: this is same signal tc_tcp_write waits for       */
                /*       but only one api call can be done at a           */
                /*       time and if we are closing port anyway           */
                /*       tc_tcp_write will fail due to state and          */
                /*       won't even try to block                          */
                /* NOTE: ignore any timeout or error status since closing */
                /*       port anyway                                      */
                wait_count = (word) (port->ap.linger_secs * ks_ticks_p_sec());

                /* let check_ack() know to only signal when window is   */
                /* empty (not when any data is acknowledged which is    */
                /* what blocking mode write and select are waiting on   */
                port->ap.port_flags |= WAIT_GRACE_CLOSE;

                OS_RELEASE_TCP()

                if (!OS_TEST_WRITE_SIGNAL((PANYPORT)port, wait_count)) 
                {
                    /* This is just here to get rid of a warning caused    */
                    /*   because the above macro does an implicit test and */
                    /*   the original code wasn't using the result of the  */
                    /*   test.                                             */
                    /* NOTE: if timeout still start close                  */
                }   

                OS_CLAIM_TCP(CLOSE1_CLAIM_TCP)
                DEBUG_LOG("tcp_close - linger with wc;after wait:wc,out.contain=", LEVEL_3, EBS_INT2, 
                         wait_count, port->out.contain);

                /* if state has changed, tc_tcp_interpret()/check_close()    */
                /* must have sent the FIN since the output buffer emptied or */
                /* the port must have been closed due to error, therefore,   */
                /* do not fall thru to send the FIN                          */
                if ( (port->state != TCP_S_EST) && 
                     (port->state != TCP_S_CWAIT) )
                    break;      /* break out of switch */
            }

            /* ******                                                  */
            /* HARD CLOSE AND GRACEFUL CLOSE WITH BLOCK FOR CASE WITH  */
            /* DATA STILL IN OUTPUT WINDOW                             */
            if ( (port->out.contain &&
                  (port->ap.options & SO_LINGER) && 
                  (port->ap.linger_secs != 0)) ||       /* graceful or */
                 ((port->ap.options & SO_LINGER) && 
                  (port->ap.linger_secs == 0)) )        /* hard close */
            {
                tcp_abort(port, TRUE);
            }

            /* ******                                                   */
            /* GRACEFUL CLOSE (WITH OR WITHOUT BLOCK) FOR CASE WITH NO  */
            /* DATA IN OUTPUT WINDOW                                    */
            /* send FIN                                                 */
            else if (!port->out.contain) 
            {
                /* ******                                                          */
                /* Wait until all data segmentized                                 */
                /* RFC 793 pg 61 says to queue this request until all              */
                /* sends have been segmentized, i.e. formatted to be sent;         */
                /* this is true if nxt+contain > nxt_to_send;                      */
                /* NOTE: if all has been sent nxt+contain should equal nxt_to_send */
                /* NOTE: this ensures the FIN will have the correct sequence       */
                /*       number                                                    */
                /* RFC also says TCP will reliably deliver all buffers             */
                /* SENT before the connection was CLOSED (see pg 37 of RFP)        */
                if ( (port->out.nxt + port->out.contain) > 
                      port->out.nxt_to_send )
                {
                    port->ap.port_flags |= WAIT_CLOSE_SEG;
                    DEBUG_LOG("tcp_close: set WAIT_CLOSE_SEG", LEVEL_3, NOVAR, 0, 0);
                    return(CLOSE_SUCCESS);
                }
                DEBUG_LOG("do close - nxt, nxt_to_send = ", LEVEL_3, DINT2,
                    port->out.nxt, port->out.nxt_to_send);

                /* reset flag since proceeding with the close   */
                port->ap.port_flags &= ~WAIT_CLOSE_SEG;

                DEBUG_LOG("tcp_close - send FIN: from => to state", LEVEL_3, EBS_INT2, 
                    port->state, new_state);
                if (new_state != TCP_S_TWAIT)
                    port->closetime = 0; /* reset timeout ctr for being in */
                                         /* LAST state before close   */

                trans_state(port, new_state, NOT_FROM_INTERPRET, FALSE);
                DECR_SNMP(TcpCurrEstab)

                /* If write is waiting signal error   */
                wake_up(port, NOT_FROM_INTERPRET, 
                        WRITE_WAKEUP_FAIL|SELECT_WRITE_WAKEUP, ENOTCONN);

                DEBUG_LOG("********** TCP_CLOSE - send FIN", LEVEL_3, NOVAR, 0, 0);
                pi = port->ap.iface;
                ret_val = tc_tcpsend(pi, port, NO_DCU_FLAGS, NORMAL_SEND, 
                                     TCP_F_FIN | TCP_F_ACK);
            }

            /* ******                                                       */
            /* GRACEFUL CLOSE FOR CASE WITH DATA IN OUTPUT WINDOW           */
            /* data in output window and no linger                          */
            /* NOTE: don't start close now but when output window empties;  */
            /*       since API_CLOSE_DONE is set, the close will be started */
            /*       when output window empties                             */
            else
            {
                /* resend data forcing otherside to send an ack    */
                pi = port->ap.iface;
                ret_val = tc_tcpsend(pi, port, NO_DCU_FLAGS, 
                                     NORMAL_DATA, NO_TCP_FLAGS);
                    
                DEBUG_LOG("********** TCP_CLOSE - do not send FIN", LEVEL_3, NOVAR, 0, 0);
                DEBUG_LOG("tcp_close - wait for ack, state = ", LEVEL_3, EBS_INT1, port->state, 0);
            }
            break;

        case TCP_S_TWAIT:                /* time out yet?  */
            /* Put the port into the closed state. The tcp timer will put them on the   */
            /* free resource exchange.                                                  */
            if (port->closetime > CFG_TWAITTIME)
                trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, FALSE);
            break;

        case TCP_S_LAST:                    /* five minute time out  */
            if (port->closetime > CFG_LASTTIME)
                trans_state(port, TCP_S_CLOSED, NOT_FROM_INTERPRET, FALSE);
            break;

        default:
            break;
    }

    /* **************************************************             */
    /* If the port goes to the closed state don't get rid of it       */
    /* until the application does a close (this is signalling         */
    /* mechanism to api that close was done) and the input            */
    /* window is empty (i.e. the application reads all the            */
    /* data or did an abort - see xn_abort)                           */
    /* NOTE: this routine will continually be called every second for */
    /*       a closed socket;  after the input window is empty the    */
    /*       socket will be freed                                     */
    /* NOTE: if there is a MASTER socket waiting for a listener       */
    /*       let it use this one                                      */
    if ( (root_tcp_lists[MASTER_LIST] && (port->ap.options & SO_REUSESOCK)) || 
         (port->in.contain == 0) || (port->ap.options & SOO_FREE_WITH_INPUT) )
    {
        if ( (port->state == TCP_S_CLOSED) &&
             (port->ap.port_flags & API_CLOSE_DONE) )
        {
#if (INCLUDE_ROUTING_TABLE && INCLUDE_RT_LOCK)
            /* free routing table entry port is locking   */
            if (port->ap.prt)
            {
                rt_free(port->ap.prt);
            }
#endif

            /* delete from master list   */
            if (port->tcp_port_type == SLAVE_PORT)
                delete_tcpport_master_list(port);

            delete_tcpport_list(port);  /* NOTE: delete_tcpport_list will figure out */
                                        /*       which list to delete from   */

            DEBUG_LOG("tcp close - free the port", LEVEL_3, NOVAR, 0, 0);
            free_tcpport_avail(port, TRUE);
            ret_val = PORT_FREED;
        }
    }
    return(ret_val);
}

/* ********************************************************************   */
/* tc_tcp_close() - close port (possibly master and its slaves)           */
/*                                                                        */
/*   ASSUMPTION: this routine is only called by API (socket or rtip)      */
/*                                                                        */
/*   Returns 0 if successful -1 if an error occured                       */
/*   Sets errno if an error is detected.                                  */
/*                                                                        */

int tc_tcp_close(PTCPPORT port)         /*__fn__*/
{
PTCPPORT sport;
PTCPPORT nport;
int      rval,ret_val;

    DEBUG_LOG("tc_tcp_close entered, state = ", LEVEL_3, EBS_INT1, port->state, 0);

    OS_CLAIM_TCP(CLOSE2_CLAIM_TCP)      /* Exclusive access to tcp resources */

    ret_val = 0;

    /* if closing master port   */
    if (port->tcp_port_type >= MASTER_NO_LISTENS)
    {
        /* loop closing all slave ports   */
        sport = port->next_slave;   /* get first slave */
        while (sport)
        {
            DEBUG_LOG("tc_tcp_close() - close port and master indexes =", LEVEL_3, EBS_INT2, 
                sport->ap.ctrl.index, port->ap.ctrl.index);
                
            /* set flag so know API closed port (done so know if can release   */
            /* resources)                                                      */
            /* NOTE: assumes this routine only called by api                   */
            sport->ap.port_flags |= API_CLOSE_DONE;

            /* take off front of master list                           */
            /* MUST task off master list since tcp_close might release */
            /* and reclaim TCP semaphore                               */
            nport = sport->next_slave;  
            port->next_slave = nport;   /* make next port head of list */

            rval = tcp_close(sport);

            /* if error returned and this is the first error   */
            /* NOTE: tcp_close will return -1 if freed port    */
            if (!ret_val && rval>0)
            {
                ret_val = -1;
                set_errno(rval);
            }
            sport = nport;
        }

        /* close the master port; it will be in alloced state   */
        port->ap.port_flags |= API_CLOSE_DONE;
        rval = tcp_close(port);

        /* if error returned and this is the first error   */
        /* NOTE: tcp_close will return -1 if freed port    */
        if (!ret_val && rval>0)  
        {
            ret_val = -1;
            set_errno(rval);
        }
    }

    else
    {
        DEBUG_LOG("tc_tcp_close() - close port, index =", LEVEL_3, EBS_INT1,
            port->ap.ctrl.index, 0);
        port->ap.port_flags |= API_CLOSE_DONE;
        rval = tcp_close(port);
        if (!ret_val && rval>0)
        {
            ret_val = -1;
            set_errno(rval);
        }
    }

    OS_RELEASE_TCP()                /* Release access to tcp resources */

    DEBUG_LOG("tc_tcp_close exit", LEVEL_3, NOVAR, 0, 0);
    return(ret_val);
}

/* ********************************************************************   */
/* free_tcpport_avail() - free port; if any master ports don't have any   */
/*                        listeners() try to allocate one                 */
/*                                                                        */
/*   Check if there are any master ports which had a problem doing        */
/*   a listen() during interpret.  If there are, try to do the            */
/*   listen() now that an additional socket is available.                 */
/*                                                                        */
/*   This routine should be called to free a socket if there is a new     */
/*   socket available, i.e. if the caller just allocated a socket then    */
/*   determined there is an error there is no point calling this routine  */
/*   since the master list should be empty in that case                   */
/*                                                                        */
/*   Assumes TCP sem claimed when called.                                 */

void free_tcpport_avail(PTCPPORT port, RTIP_BOOLEAN sem_claimed)    /*__fn__*/
{
PTCPPORT mport;

#if (INCLUDE_ARP)
    /* get rid of any references to this port in the arp cache   */
    tc_arp_closeport((PANYPORT)port);
#endif  /* INCLUDE_ARP */

    free_both_windows(port, TRUE);

    port->ap.list_type = FREE_LIST; 
    os_free_tcpport(port);

    /* check if any MASTER sockets need a slave socket   */
    mport = (PTCPPORT)(root_tcp_lists[MASTER_LIST]);
    while (mport)
    {
        if (tc_tcp_listen(mport, mport->tcp_port_type, sem_claimed))
        {
            /* it failed; try another port   */
            DEBUG_ERROR("free_tcpport_avail - tc_tcp_listen() failed - errno = ",
                EBS_INT1, xn_getlasterror(), 0);
#if (DEBUG_WEB)
            listen_fail++;
#endif
        }
        else
        {
            DEBUG_LOG("free_tcpport_avail - listen success : master list -> list list; port type = ",
                LEVEL_3, EBS_INT1, mport->tcp_port_type, 0);

            /* move master port from MASTER list back to SOCKET list   */
            fix_master(mport);

            /* it succeeded, do only one -                                */
            /* NOTE: listen() will move from MASTER list to LISTEN list;  */
            /*       another reason not do continue without starting over */
            /*       at the beginning of the master list                  */
            break;      /* if succeeds only do one */
        }
        mport = (PTCPPORT)os_list_next_entry_off(root_tcp_lists[MASTER_LIST], 
                                                 (POS_LIST)mport, ZERO_OFFSET);
    }
    DEBUG_LOG("PORT LISTS", LEVEL_3, PORTS_TCP, 0, 0);
}


/* ********************************************************************     */
/* delete_tcpport_master_list() - deletes port from its master's slave list */
/*                                                                          */
/*   Deletes port from its master's slave list.  The slave points to the    */
/*   master port.                                                           */
/*                                                                          */
/*   Returns nothing                                                        */
/*                                                                          */

void delete_tcpport_master_list(PTCPPORT port)  /*__fn__*/
{
PTCPPORT mport;
PTCPPORT sport;
PTCPPORT prev_port;

    mport = port->master_port;   /* get master port  */
    if (mport == (PTCPPORT)0)
    {
        DEBUG_LOG("delete_tcpport_master_list() - port has no master", LEVEL_3, NOVAR, 0, 0);
        return;
    }
    sport = mport->next_slave;   /* get first slave */
    prev_port = mport;
    while (sport)
    {
        DEBUG_LOG("delete_tcpport_master_list() - index =", LEVEL_3, EBS_INT1,
            sport->ap.ctrl.index, 0);
        if (sport == port)    /* if found port to be deleted */
            break;
        prev_port = sport;
        sport = sport->next_slave;  
    }

    /* if did not find a slave port to delete, do nothing else   */
    if (!sport)
    {
        DEBUG_LOG("delete_tcpport_master_list - no slave port found,master index = ", LEVEL_3, EBS_INT1,
                mport->ap.ctrl.index, 0);
        return;
    }

    /* now that slave port is found take it off the masters list   */
    DEBUG_LOG("delete_tcpport_master_list() - take slave off master list, index =", LEVEL_3, EBS_INT1,
        sport->ap.ctrl.index, 0);
    prev_port->next_slave = sport->next_slave;
    sport->master_port = (PTCPPORT)0;
}

/* ********************************************************************   */
/* tc_is_read_state() - determines if acceptable state to do read         */
/*                                                                        */
/*    Determines if port is in a state where it is acceptable to do       */
/*    reads.  The valid states are EST, CWAIT, FW1 and FW2.               */
/*                                                                        */
/*    Returns TRUE if is acceptable to do a read or FALSE if it is not.   */
/*                                                                        */

RTIP_BOOLEAN tc_is_read_state(PTCPPORT port)    /*__fn__*/
{
    if ( (port->state != TCP_S_EST)   &&
         (port->state != TCP_S_CWAIT) &&
         (port->state != TCP_S_FW1)   &&
         (port->state != TCP_S_FW2) )
    {
        /* if the socket is closing we need to wait until the input     */
        /* window empties before freeing the socket, i.e. receipt of    */
        /* a FIN implies PUSH which means the input data needs to be    */
        /* passed to the application layer; instead of copying the      */
        /* data to a buffer associated with the application layer       */
        /* we just keep the socket around until the input buffer        */
        /* empties; we still let the sockets do the state transitions   */
        /* though;                                                      */
        /* NOTE: if an xn_abort or an error occurs where the connection */
        /*       is aborted the input window is freed, therefore,       */
        /*       reads will be illegal and the socket will be returned  */
        /*       to the system                                          */
        if ( port->in.contain && 
             ((port->state == TCP_S_CLOSED)     || 
              (port->state == TCP_S_TWAIT)      ||
              (port->state == TCP_S_LAST)) )
        {
            return(TRUE);
        }

        /*DEBUG_ERROR("is_read_state->FALSE: state, in.contain = ", EBS_INT2,    */
        /*  port->state, port->in.contain);                                      */
        return(FALSE);
    }

    return(TRUE);
}

/* ********************************************************************   */
/* tc_is_write_state() - determines if acceptable state to do write       */
/*                                                                        */
/*    Determines if port is in a state where it is acceptable to do       */
/*    writes.  The valid states are EST, CWAIT, SYNS and SYNR.            */
/*                                                                        */
/*    Returns TRUE if is acceptable to do a write or FALSE if it is not.  */
/*                                                                        */
RTIP_BOOLEAN tc_is_write_state(PTCPPORT port)    /*__fn__*/
{
    /* if close done, could still be in a legal state waiting for     */
    /* output window to empty; even so any writes after close is done */
    /* should fail                                                    */
    if (port->ap.port_flags & API_CLOSE_DONE) 
        return(FALSE);

    /* NOTE: SYNS and SYNR will queue output data but will not send it yet   */
    if ( (port->state != TCP_S_EST) &&
         (port->state != TCP_S_CWAIT) &&
         (port->state != TCP_S_SYNS) &&
         (port->state != TCP_S_SYNR) )
        return(FALSE);
    else
        return(TRUE);
}


#endif

