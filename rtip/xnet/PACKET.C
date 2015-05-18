/*                                                                         */
/* PACKET.C - Packet device driver interface                               */
/*                                                                         */
/* EBS - RTIP                                                              */
/*                                                                         */
/* Copyright Peter Van Oudenaren , 1993                                    */
/* All rights reserved.                                                    */
/* This code may not be redistributed in source or linkable object form    */
/* without the consent of its author.                                      */
/*                                                                         */
/* Portions of this code were taken from the ncsa telnet package           */
/*                                                                         */
/* cu-notic.txt         NCSA Telnet version 2.2C     2/3/89                */
/*   Notice:                                                               */
/*        Portions of this file have been modified by                      */
/*        The Educational Resources Center of Clarkson University.         */
/*        All modifications made by Clarkson University are hereby placed  */
/*        in the public domain, provided the following statement remain in */
/*        all source files.                                                */
/*        "Portions Developed by the Educational Resources Center,         */
/*                Clarkson University"                                     */
/*        Bugs and comments to bkc@omnigate.clarkson.edu                   */
/*                                bkc@clgw.bitnet                          */
/*        Brad Clements                                                    */
/*        Educational Resources Center                                     */
/*        Clarkson University                                              */
/*   packet.c - FTP Software Packet Interface for NCSA TELNET              */
/*   Author: Brad Clements  bkc@omnigate.clarkson.edu                      */
/*           Clarskon University                                           */
/*           10/24/88                                                      */

#undef  DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"

#if (INCLUDE_PKT )

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
word packet_vector;                       /* non-zero if we found it */
word ip_handle;
word arp_handle;
word rarp_handle;

byte iptype[2];
byte arptype[2];
byte rarptype[2];

/* Unknown bugs in the WD packet driver cause the driver to stall if there is   */
/* too much traffic. We delay 1000 microseconds before each write to fix this   */
/* pressing the '-' key in the demo reduces this by 100. pressing '+'           */
/* increases it by 100.                                                         */
word KS_FAR packet_delay;

/* set class to ETHERNET   */
word KS_FAR packet_class;

/* Pointer to the interface structure. We look at this inside interrupt service   */
PIFACE pktpi;

/* used to register PKT init fnc   */
INIT_FNCS KS_FAR pkt_fnc;       

#endif  /* DECLARING_DATA|| BUILD_NEW_BINARY */

#if (DECLARING_CONSTANTS|| BUILD_NEW_BINARY)
/* Hardwire the packet multiplexer int to 0x60     */
KS_GLOBAL_CONSTANT word packet_int = 0x60;
#endif  /* DECLARING_CONSTANTS|| BUILD_NEW_BINARY */

#if (!DECLARING_DATA && !DECLARING_CONSTANTS)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern word packet_vector;                       /* non-zero if we found it */
extern word ip_handle;
extern word arp_handle;
extern word rarp_handle;

extern byte iptype[2];
extern byte arptype[2];
extern byte rarptype[2];

/* Unknown bugs in the WD packet driver cause the driver to stall if there is   */
/* too much traffic. We delay 1000 microseconds before each write to fix this   */
/* pressing the '-' key in the demo reduces this by 100. pressing '+'           */
/* increases it by 100.                                                         */
extern word KS_FAR packet_delay;

/* Hardwire the packet multiplexer int to 0x60  class to 1 (ethernet)   */
KS_EXTERN_GLOBAL_CONSTANT word packet_int;
extern word KS_FAR packet_class;

/* Pointer to the interface structure. We look at this inside interrupt service   */
extern PIFACE pktpi;
#endif

/* ********************************************************************   */
#ifndef MK_FP    /* some compiler version do not define MK_FP */
#define MK_FP(seg,ofs) ((void KS_FAR *)(((unsigned long)(seg) << 16) | \
                                      (unsigned)(ofs)))
#endif

#define IC_ANY          0
#define IC_ETHERNET     1
#define IC_SLIP         6
#define IT_ANY          0xFFFF

#ifdef __TURBOC__
#define _REGS REGS
#define _SREGS SREGS
#endif

/* ********************************************************************   */
struct pkt_statistics
{
    dword packets_in;
    dword packets_out;
    dword bytes_in;
    dword bytes_out;
    dword errors_in;
    dword errors_out;
    dword packets_lost;
};

/* ********************************************************************   */
RTIP_CDECL(void KS_FAR pkt_receiver(void);)/* This is in assembler so we can */
                                    /* use the interrupt function   */
                                    /* modifier to access regs      */

static word locate_pkt_vector(word vec);
static word pkt_access_type(word if_class,word if_type,word if_number,PFBYTE type, word typelen,void  (KS_FAR * receiver)());
static word pkt_set_rcv_mode(word handle, word mode);
static void pkt_release_type(word    handle);
static RTIP_BOOLEAN pkt_get_address(word handle,PFBYTE storage,word len);
static word pkt_set_multicast_list(PFBYTE list, word len);
static word pkt_send(PFBYTE packet, word len);

RTIP_CDECL(void KS_FAR pkt_receiver(void);) /* This is in assembler so we can */
                                    /* use the interrupt function   */
                                    /* modifier to access regs      */
void _usleep(word micros);

#ifdef __TURBOC__
RTIP_CDECL(void KS_FAR INTERRUPT pkt_receiver2(word bp, word di, word si, word ds, word es,\
                                 word dx, word cx, word bx, word ax);)
#else
RTIP_CDECL(void KS_FAR INTERRUPT pkt_receiver2(word es, word ds, word di, word si, word bp, \
                                 word sp, word bx, word dx, word cx, word ax);)
#endif

/* ********************************************************************   */
/* init_pkt - initialize the data structures for the packet driver        */
/*          - this function needs to be registered before calling         */
/*            xn_rtip_init                                                */
/* ********************************************************************   */
void init_pkt(void)
{
    packet_vector = 0;
    ip_handle = 0xffff;
    arp_handle = 0xffff;
    rarp_handle = 0xffff;

    iptype[0] = 8;
    iptype[1] = 0;
    arptype[0] = 8;
    arptype[1] = 6;
    rarptype[0] = 0x80;
    rarptype[1] = 0x35;

    packet_delay = 1000;
    packet_class = 1;
    pktpi = 0;
}

/* ********************************************************************   */
/* pkt_setmcast() -                                                       */
/*   Takes an interface structures a contiguous array                     */
/*   of bytes containing N IP multicast addresses and n, the number       */
/*   of addresses (not number of bytes).                                  */
/*   Calls the packet packet driver's set_multicast_list                  */

RTIP_BOOLEAN pkt_setmcast(PIFACE pi)            /* __fn__ */
{
    if (pkt_set_multicast_list(pi->mcast.mclist, (word)pi->mcast.lenmclist))
        return(FALSE);
    else
        return(TRUE);
}


/* ********************************************************************         */
/* open the packet driver interface.                                            */
/*                                                                              */
/* This routine opens a packet device driver and copies the board's             */
/* ethernet address into the interface structure. If the open is successful and */
/* the ethernet address was ascertained it returns TRUE otherwise FALSE.        */
/*                                                                              */
/* The address of this function must be placed into the "devices" table in      */
/* iface.c either at compile time or before a device open is called.            */
/*                                                                              */
/*                                                                              */
/* Non packet drivers should behave the same way.                               */
/*                                                                              */

/* we try and locate the packet driver and open ARP and IP handles.    */
/* also open RARP handle                                               */


RTIP_BOOLEAN pkt_open(PIFACE pi)                                /*__fn_*/
{
    if (ip_handle != 0xffff)
        return(TRUE);

    /* Fail if the packet address is wrong   */
    if (locate_pkt_vector(packet_int))
    {
        DEBUG_ERROR("pketopen - locate_pkt_vector: failed", NOVAR, 0, 0);
        return(FALSE);
    }

    if (packet_class<1 || packet_class>11)  /* 11 is the highest number for */
        packet_class = IC_ETHERNET;         /* if_class at the present time    */

    if ((ip_handle = pkt_access_type(packet_class, IT_ANY, 0, iptype,
                                     sizeof(iptype), pkt_receiver)) == 0xffff)
    {
        arp_handle = ip_handle = rarp_handle = 0xffff;
        DEBUG_ERROR("pketopen: pkt_access_type failed", NOVAR, 0, 0);
        return(FALSE);
    }
    /* Set the rcv mode here. because if more then one handle is open
       set rcv mode will fail */
    /* NOTE: the second parameter is mode where                    */
    /*       4 = pkts to this board, mulitcast and broadcast       */
    /*       3 = pkts to this board and broadcast                  */
    /* NOTE: 4 did not work for 3c509 packet driver but 3 did work */
    if (pkt_set_rcv_mode(ip_handle, CFG_PKT_MODE))  
    {
        DEBUG_ERROR("pketopen: pkt_set_rcv_mode failed", NOVAR, 0, 0);
        return(FALSE);
    }

    if ((arp_handle = pkt_access_type(packet_class, IT_ANY, 0, arptype,
                                      sizeof(arptype), pkt_receiver)) == 0xffff)
    {
        pkt_release_type(ip_handle);
        arp_handle = ip_handle = rarp_handle = 0xffff;
        DEBUG_ERROR("pketopen: pkt_access_type - arptype failed", NOVAR, 0, 0);
        return(FALSE);
    }

    if ((rarp_handle = pkt_access_type(packet_class, IT_ANY, 0, rarptype,
                                       sizeof(rarptype), pkt_receiver)) == 0xffff)
    {
        pkt_release_type(ip_handle);
        pkt_release_type(arp_handle);
        arp_handle = ip_handle = rarp_handle = 0xffff;
        DEBUG_ERROR("pketopen: pkt_access_type - rarptype failed", NOVAR, 0, 0);
        return(FALSE);
    }

    /* Now get the ethernet address        */
    if (!pkt_get_address(ip_handle, pi->addr.my_hw_addr, 6))
    {
        DEBUG_ERROR("pketopen: pkt_get_address failed", NOVAR, 0, 0);
        return(FALSE);
    }

    /* Make the interface structure visible to interrupt service routines   */
    pktpi = pi;

    return(TRUE);
}


/* ********************************************************************    */
/* close the packet driver interface.                                      */
/*                                                                         */
/* This routine is called when the device interface is no longer needed    */
/* it should stop the driver from delivering packets to the upper levels   */
/* and shut off packet delivery to the network.                            */
/*                                                                         */
/* The address of this function must be placed into the "devices" table in */
/* iface.c either at compile time or before a device open is called.       */
/*                                                                         */
/*                                                                         */
/* Non packet drivers should behave the same way.                          */
/*                                                                         */

void pkt_close(PIFACE pi)                                /*__fn__*/
{
   /* The interface structure is not used   */
   pi = pi;

    if (ip_handle != 0xffff)
    {
       pkt_release_type(ip_handle);
       pkt_release_type(arp_handle);
       pkt_release_type(rarp_handle);
    }
    pktpi = (PIFACE) 0;
    arp_handle = ip_handle = rarp_handle = 0xffff;
}

/* ********************************************************************      */
/* Transmit. a packet over the packet driver interface.                      */
/*                                                                           */
/* This routine is called when a packet needs sending. The packet contains a */
/* full ethernet frame to be transmitted. The length of the packet is        */
/* provided.                                                                 */
/*                                                                           */
/* The address of this function must be placed into the "devices" table in   */
/* iface.c either at compile time or before a device open is called.         */
/*                                                                           */
/*                                                                           */
/* Non packet drivers should behave the same way.                            */
/*                                                                           */

int pkt_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{

int ret_val;
KS_INTERRUPT_CONTEXT sp;
PFBYTE packet;
int length;

    /* The interface structure is not used     */
    pi = pi;

    length = DCUTOPACKET(msg)->length;
    if (length < 60)   
        length = 60;   
    packet = DCUTODATA(msg);

    /* Delay 1000 microseconds per call.                                   */
    /* This is done because the packet drivers stall in the write routine  */
    /* if there are too many packets being sent & received.                */
    /* this number was determined empirically.                             */
    if (packet_delay)
        _usleep(packet_delay);

    if (pkt_send(packet,(word)length))
        ret_val = ENETDOWN;
    else
        ret_val = REPORT_XMIT_DONE;     /* send done */

    /* signal IP layer that send is done     */
        /* signal IP layer that send is done */
    pi->xmit_done++;
    OS_SET_IP_SIGNAL(pi);


    return(ret_val);
}


/* ********************************************************************          */
/* Statistic. return statistics about the device interface                       */
/*                                                                               */
/* This routine is called by user code that wishes to inspect driver statistics. */
/* We call this routine in the demo program. It is not absolutely necessary      */
/* to implement such a function (Leave it empty.), but it is a handy debugging   */
/* tool.                                                                         */
/*                                                                               */
/* The address of this function must be placed into the "devices" table in       */
/* iface.c either at compile time or before a device open is called.             */
/*                                                                               */
/*                                                                               */
/* Non packet drivers should behave the same way.                                */
/*                                                                               */

RTIP_BOOLEAN pkt_statistics(PIFACE  pi)                       /*__fn__*/
{
union _REGS regs;
struct _SREGS segregs;
struct pkt_statistics KS_FAR *p;

   if (!packet_vector || ip_handle == 0xffff)
      return(FALSE);
        
    regs.h.ah = 24;
    regs.h.al = 0;
    regs.x.bx = ip_handle;

    _segread(&segregs);

    _int86x(packet_vector,&regs,&regs,&segregs);
    if (regs.x.cflag)
    {
        /* Failed   */
        UPDATE_SET_INFO(pi, interface_packets_in, 0L)
        UPDATE_SET_INFO(pi, interface_packets_out, 0L)
        UPDATE_SET_INFO(pi, interface_bytes_in, 0L)
        UPDATE_SET_INFO(pi, interface_bytes_out, 0L)
        UPDATE_SET_INFO(pi, interface_errors_in, 0L)
        UPDATE_SET_INFO(pi, interface_errors_out, 0L)
        UPDATE_SET_INFO(pi, interface_packets_lost, 0L)
        return(FALSE);
    }
    p = (struct pkt_statistics KS_FAR *) MK_FP(segregs.ds, regs.x.si);

    UPDATE_SET_INFO(pi, interface_packets_in, p->packets_in)
    UPDATE_SET_INFO(pi, interface_packets_out, p->packets_out)
    UPDATE_SET_INFO(pi, interface_bytes_in, p->bytes_in)
    UPDATE_SET_INFO(pi, interface_bytes_out, p->bytes_out)
    UPDATE_SET_INFO(pi, interface_errors_in, p->errors_in)
    UPDATE_SET_INFO(pi, interface_errors_out, p->errors_out)
    UPDATE_SET_INFO(pi, interface_packets_lost, p->packets_lost)
    return(TRUE);
}

/* ********************************************************************   */
word nlost = 0;

#if (DOSNET)
#pragma check_stack( off )
#endif

DCU    saved_msg;

#ifdef __TURBOC__
void KS_FAR INTERRUPT pkt_receiver2(word bp, word di, word si, word ds, word es,
                                 word dx, word cx, word bx, word ax)
#else
void KS_FAR INTERRUPT pkt_receiver2(word es, word ds, word di, word si, word bp, 
                                 word sp, word bx, word dx, word cx, word ax)
#endif
{
PFWORD pw;
DCU    msg;

   /* this receiver function assumes that between the first and second call   */
   /* from the packet driver, the underlying telnet code will not access      */
   /* the buffer.                                                             */

   /* here's an incoming packet from the packet driver, first see if we   */
   /* have enough space for it                                            */

   /* SMXMSP3 - calls ENTER_ISR with parameter 0; it is ok since    */
   /*           packet not supported in protected mode              */
   KS_ENTER_ISR();

    /* get rid of compiler warnings    */
    ds=ds;  si=si;  bp=bp;  bx=bx;  dx=dx;  es=es;  di=di;
#    ifndef __TURBOC__
     sp=sp;
#    endif

    /* ******                                                               */
    /* NOTE: for each incoming packet the two interrupts are generated, the */
    /*       first with ax=0 and the second with ax=1                       */

    /* this part covers first call from packet driver - packet driver is    */
    /* requesting a buffer from the application to copy packet into         */
    if (!ax)   
    {
        /* The packet driver gives us the size in cx and wants a data   */
        /* pointer to copy into in es:di                                */
        msg = os_alloc_packet_input((word)cx, DRIVER_ALLOC);
        if (msg)
        {
            DCUTOPACKET(msg)->length = cx;   /* Get the length from cx */
            pw = (PFWORD) &(DCUTOPACKET(msg)->data[0]);
    
            /* now put the data address in es:di to pass to packet driver   */
            es = (word)FP_SEG(pw);
            di = (word)FP_OFF(pw);
            /* save the packet for queueing   */
            saved_msg = msg;
            KS_EXIT_ISR();

            return;
        }
  
        /* Fall through to here if no buffers available   */
        DEBUG_ERROR("pkt_receiver2() - out of DCUs", NOVAR, 0, 0);
        nlost++;
        es = di = 0;    /* tell packet driver no buffer avail, packet driver */
                        /* won't make the second call   */
                                
    }

    /* ******                                                               */
    /* this part covers the second call from packet driver which has copied */
    /* the data into the packet passed back to it during the first call     */
    else            /* second call from packet driver */
    {
        /* Give the packet to the stack   */
        ks_invoke_input(pktpi,saved_msg);
    }
    KS_EXIT_ISR();
}


#if (DOSNET)
#pragma check_stack( on )
#endif

/* ********************************************************************        */
/* locate_pkt_vector:                                                          */
/* We modified the clarkson driver for the embedded version. We don't hunt     */
/* for the packet vector number but simply verify that the driver is installed */
/* at this address.                                                            */
/*                                                                             */
/*                                                                             */
static word locate_pkt_vector(word vec)                              /*__fn__*/
{
byte KS_FAR *  KS_FAR  * ptr0;
byte  KS_FAR  * ptr;

    if (packet_vector)
        return(0);         /* already found! */

    /* Vector must be right   */
    if ((vec < 0x60) || (vec > 0x7f))
        return(0xffff);

    /* Turn the interrupt vector into a KS_FAR pointer to char   */
    ptr0 = (PFBYTE   KS_FAR  *)MK_FP(0,(vec * 4));
    ptr =  *ptr0;

    /* Add three and look for the packet signature   */
    ptr += 3;

    if (tc_comparen((PFBYTE )ptr, (PFBYTE ) "PKT DRVR",8))
    {
        packet_vector = vec;
        return(0);
    }
    return(0xffff);
}


/* ********************************************************************   */
/* pkt_access_type - Establish the receiver function for                  */
/* class:type:interface_number:packet_id                                  */
/*                                                                        */
/* Returns a handle used controlling the fuction.                         */
/*                                                                        */
static word pkt_set_multicast_list(PFBYTE list, word len) /*__fn__*/
{
union _REGS regs;
struct _SREGS segregs;

   if (!packet_vector)
        return(0xffff);
    regs.h.ah = 22;
    regs.x.cx =  len;
    segregs.es = FP_SEG(list);
    regs.x.di =  FP_OFF(list);
    _int86x(packet_vector, &regs, &regs, &segregs);
    if (regs.x.cflag)
    {
        DEBUG_ERROR("PKT set multicast failed", NOVAR, 0, 0);
        return(0xffff);
    }
    return((word)regs.h.dh);
}


/* ********************************************************************   */
/* pkt_access_type - Establish the receiver function for                  */
/* class:type:interface_number:packet_id                                  */
/*                                                                        */
/* Returns a handle used controlling the fuction.                         */
/*                                                                        */
static word pkt_access_type(word if_class,word if_type,word if_number,PFBYTE type, word typelen,void  (KS_FAR * receiver)()) /*__fn__*/
{
union _REGS regs;
struct _SREGS segregs;

   if (!packet_vector)
        return(0xffff);
    regs.h.ah = 2;
    regs.h.al= (char)if_class;
    regs.x.bx = if_type;
    regs.x.dx = if_number;
    segregs.ds = FP_SEG(type);
    regs.x.si =  FP_OFF(type);
    regs.x.cx =  typelen;
    segregs.es = FP_SEG(receiver);
    regs.x.di =  FP_OFF(receiver);
    _int86x(packet_vector, &regs, &regs, &segregs);
    if (regs.x.cflag)
    {
        DEBUG_ERROR("PKT access type failed", NOVAR, 0, 0);
        return(0xffff);
    }
    return((word)regs.x.ax);
}

/* ********************************************************************   */
/* Set the receive mode                                                   */
/*                                                                        */
/* Handle is the handle of the receiver function                          */
/*                                                                        */
/* Modes                                                                  */
/* 1 == DISABLE                                                           */
/* 2 == RECEIVE ONLY PACKETS ADDRESSED TO THIS BOARD                      */
/* 3 == MODE 2 PLUS BROADCASTS                                            */
/* 4 == MODE 3 PLUS LIMITED MUILTICASTS                                   */
/* 5 == MODE 3 PLUS ALL MULTICASTS                                        */
/*                                                                        */

static word pkt_set_rcv_mode(word handle, word mode)                  /*__fn__*/
{
union _REGS regs;

   if (!packet_vector)
      return(0xffff);
   regs.x.ax = 0x1400 /* 0x2000 */; /* 20 decimal */
   regs.x.bx = handle;
   regs.x.cx = mode;
   _int86(packet_vector,&regs,&regs);
   if (regs.x.cflag) 
   {
        DEBUG_ERROR("PKT set rcv mode failed", NOVAR, 0, 0);
         return(regs.h.dh);
    }
   return(0);
}

/* ********************************************************************      */
/* Tell the packet driver to no longer send packets to the receiver function */
/* for handle.                                                               */
/*                                                                           */
static void pkt_release_type(word handle)                            /*__fn__*/
{
union _REGS regs;
   
   if (!packet_vector)
      return;

   regs.x.ax = 0x0300;
   regs.x.bx = handle;
   _int86(packet_vector,&regs,&regs);
   return;
}

/* ********************************************************************   */
/* pkt_get_address() - returns the ethernet address of the host in        */
/*                     storage. len is the max # bytes                    */
/*                                                                        */

static RTIP_BOOLEAN pkt_get_address(word handle,PFBYTE storage,word len)  /*__fn__*/
{
union _REGS regs;
struct _SREGS segregs;

    if (!packet_vector)
        return(FALSE);

    regs.x.ax = 0x0600;
    regs.x.bx = handle;
    regs.x.di = FP_OFF(storage);
    segregs.es = FP_SEG(storage);
    regs.x.cx = len;
    _int86x(packet_vector,&regs,&regs,&segregs);
    if (regs.x.cflag)
    {
        DEBUG_ERROR("PKT get address failed", NOVAR, 0, 0);
        return(FALSE);
    }

    return(TRUE);
}

/* ********************************************************************   */
/* Send to the packet interface                                           */
/*                                                                        */
static word pkt_send(PFBYTE packet, word len)                 /*__fn__*/
{
union _REGS regs;
struct _SREGS segregs;

    if (!packet_vector)
    {
        DEBUG_ERROR("pkt_send - return error", NOVAR, 0, 0);
        return(0xffff);
    }
    regs.h.ah=4;
    regs.h.al=0;
    regs.x.si = FP_OFF(packet);
    regs.x.cx = len;
    segregs.ds = FP_SEG(packet);
    _int86x(packet_vector, &regs, &regs, &segregs);

    if (regs.x.cflag)
    {
        DEBUG_ERROR("pkt_send: send failed, DH =", EBS_INT1, regs.h.dh, 0);
        return(regs.h.dh);
    }
    return(0);
}

/* ********************************************************************   */
/* Microsecond sleep function. Read the 8253 tick count until
   n microseconds have elapsed */
static word read_8253(void)
{
word val;
byte  low;
byte  high;

    _outp(0x43, 0);
    low  = (byte)_inp(0x40);
    high = (byte)_inp(0x40);
    val = (word) high;
    val <<= 8;
    val |= low;
    return(val);
}

/* ********************************************************************   */
void _usleep(word micros)                                /*__fn__*/
{
word val;
word val2;
word oldval;

    /* In mode 0 (the default AT mode) the clock counts by twos. So we    */
    /* must double the input value. Also this routine actually sleeps     */
    /* (.8 * micros) microseconds. We compensated by calling it with 32   */
    /* instead of 25 above.                                               */
    micros <<= 1;

    /* Get the current tick value.    */
    val = read_8253();      /* Get the timer  */

    /* If we are close to a wrap, let it wrap   */
    while (val < micros){val = read_8253();}

    /* Now loop wile we are between val and val - micros    */
    oldval = val;
    val -= micros;  /* Wait til the clock is current val - delay ticks */
    do {val2 = read_8253();} while( (val2 >= val) && (val2 <= oldval));
}

#endif      /* !DECLARING_DATA && !DECLARING_CONSTANTS */
#endif      /* INCLUDE_PKT */



