/*                                                                       */
/*  EBS - RTIP                                                           */
/*                                                                       */
/*  Copyright Peter Van Oudenaren , 1993                                 */
/*  All rights reserved.                                                 */
/*  This code may not be redistributed in source or linkable object form */
/*  without the consent of its author.                                   */
/*                                                                       */

#undef  DIAG_SECTION_KERNEL
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER


#include "sock.h"
#include "rtip.h"

#if (INCLUDE_TCFE574)
#if (INCLUDE_TCFE574_PCMCIA)
#include "pcmcia.h"
#endif


#include "3cfe574.h"
#include "3cfe.h"


#define DEBUG_TCFE574       0
#define DEBUG_TCFE574_TRACE 0

#define TCFE_MACROS         1


#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ---------------------------------------------------------------------    */
/* GLOBAL DATA                                                              */
/* ---------------------------------------------------------------------    */
TCFE574_SOFTC KS_FAR TCFE574softc[CFG_NUM_TCFE574];

#if (INCLUDE_TCFE574_PCMCIA)
EDEVTABLE KS_FAR TCFE574_pcmcia_device =
{
     TCFE574_open, TCFE574_close, TCFE574_xmit, TCFE574_xmit_done,
     NULLP_FUNC, TCFE574_statistics, TCFE574_setmcast,
     TCFE574_PCMCIA_DEVICE, "3Com 3CFE574 PCMCIA", MINOR_0, ETHER_IFACE,
     SNMP_DEVICE_INFO(CFG_OID_NE2000, CFG_SPEED_NE2000)
     MAX_MTU, MAX_MSS, MAX_WIN_IN, MAX_WIN_OUT, EN(0x120), EN(0x0), EN(5)
};
#endif
#endif  /* DECLARING_DATA */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ---------------------------------------------------------------------    */
/* EXTERNS                                                                  */
/* ---------------------------------------------------------------------    */
#if (!BUILD_NEW_BINARY)
extern TCFE574_SOFTC KS_FAR TCFE574softc[CFG_NUM_TCFE574];

#if (INCLUDE_TCFE574_PCMCIA)
extern EDEVTABLE KS_FAR TCFE574_pcmcia_device;
extern int card_device_type[NSOCKET];   /* BSS area used by the pcmcia system  */
#endif  /* INCLUDE_TCFE574_PCMCIA */
#endif  /* !BUILD_NEW_BINARY */

/* ---------------------------------------------------------------------    */
/* Forward references for internal utility functions                        */
/* ---------------------------------------------------------------------    */

/* ---------------------------------------------------------------------    */
/* ---------------------------------------------------------------------    */
#if (INCLUDE_TCFE574_PCMCIA)
int            tcfe_pcmciaInit          ( PIFACE, int *, IOADDRESS * );
#endif
RTIP_BOOLEAN   tcfe_openDevice          ( PIFACE );
/* ---------------------------------------------------------------------    */
/* ---------------------------------------------------------------------    */

/* ---------------------------------------------------------------------    */
/* ---------------------------------------------------------------------    */
unsigned short TCFE_Init                ( IOADDRESS ioBase );
unsigned short TCFE_Enable              ( IOADDRESS ioBase );
unsigned short TCFE_Disable             ( IOADDRESS ioBase );
unsigned short TCFE_Reset               ( IOADDRESS ioBase );
unsigned short TCFE_EnableStatistics    ( IOADDRESS ioBase );
unsigned short TCFE_ResetStatistics     ( IOADDRESS ioBase );
unsigned short TCFE_ReadStatistics      ( IOADDRESS ioBase, PTCFE_STATS stats        );
unsigned short TCFE_GetHWAddress        ( IOADDRESS ioBase, byte *phys_addr          );
int            TCFE_Transmit            ( IOADDRESS ioBase, PFBYTE message,           int length               );
unsigned short TCFE_TransmitComplete    ( IOADDRESS ioBase, PETHER_STATS error_stats );
int            TCFE_Receive             ( IOADDRESS ioBase, PFBYTE message,           int length               );
unsigned short TCFE_ReceiveComplete     ( IOADDRESS ioBase, PIFACE pi,                PETHER_STATS error_stats );
unsigned short TCFE_ReceiveFilter       ( IOADDRESS ioBase );
unsigned short TCFE_SetThresholds       ( IOADDRESS ioBase );
unsigned short TCFE_EnableInterrupts    ( IOADDRESS ioBase );
unsigned short TCFE_DisableInterrupts   ( IOADDRESS ioBase );
unsigned short TCFE_ProcessCommand      ( IOADDRESS ioBase, unsigned short val,       unsigned short wait      );

#if (!TCFE_MACROS)
unsigned short TCFE_SwitchBank          ( IOADDRESS ioBase, unsigned short Bank      );
word           TCFE_ReadWord            ( IOADDRESS ioBase, unsigned short reg       );
void           TCFE_WriteWord           ( IOADDRESS ioBase, unsigned short reg,       word val                 );
byte           TCFE_ReadByte            ( IOADDRESS ioBase, unsigned short reg       );
void           TCFE_WriteByte           ( IOADDRESS ioBase, unsigned short reg,       byte val                 );
word           TCFE_ObtainStatus        ( IOADDRESS ioBase );
#endif
/* ---------------------------------------------------------------------    */
/* ---------------------------------------------------------------------    */


/* ---------------------------------------------------------------------    */
/* DEFINES                                                                  */
/* ---------------------------------------------------------------------    */
#if (RTIP_VERSION < 30)
#define ETHERSIZE           CFG_ETHERSIZE
#endif
#define ETHER_MAX_LEN       ETHERSIZE+4

/* ---------------------------------------------------------------------    */
/* MACROS                                                                   */
/* ---------------------------------------------------------------------    */
#if (TCFE_MACROS)

#define TCFE_SwitchBank( IOBASE, BANK )     TCFE_ProcessCommand(IOBASE, (unsigned short)(CSelectRegisterBank | BANK), TCFE_NO_WAIT )
#define TCFE_ReadWord( IOBASE, REG )        TCFE574_INWORD ( IOBASE + REG )
#define TCFE_WriteWord( IOBASE, REG, VAL )  TCFE574_OUTWORD( (IOBASE + REG), VAL )
#define TCFE_ReadByte( IOBASE, REG )        TCFE574_INBYTE ( IOBASE + REG )
#define TCFE_WriteByte( IOBASE, REG, VAL )  TCFE574_OUTBYTE( (IOBASE + REG), VAL )
#define TCFE_ObtainStatus( IOBASE )         TCFE_ReadWord  ( IOBASE, INTSTATUS_COMMAND )

#endif  /* TCFE_MACROS */

/* ---------------------------------------------------------------------    */
/* TCFE574_open() - open a device                                           */
/*                                                                          */
/* Perform device driver specific processing to enable sending and          */
/* receiving packets.  Also probes for the device.                          */
/*                                                                          */
/* Inputs:                                                                  */
/*   pi - interface structure of the device to open                         */
/*                                                                          */
/* Returns: TRUE if successful, FALSE if failure                            */
/* ---------------------------------------------------------------------    */

RTIP_BOOLEAN TCFE574_open(PIFACE pi)
{
PTCFE574_SOFTC sc;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_open", NOVAR , 0, 0);
#endif

    sc = iface_to_TCFE574_softc(pi);
    if (!sc)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }
    tc_memset((PFBYTE) sc, 0, sizeof(*sc));


    /* --------------------------    */
    /*  Find a 3CFE574BT device      */
    /*  indexed by minor number.     */
    /*  Init the divice and get      */
    /*  it on the bus.               */
    /* --------------------------    */
    if (pi->pdev->device_id == TCFE574_PCMCIA_DEVICE)
    {
        /* --------------------------    */
        /*  set up pointers between      */
        /*  iface and softc              */
        /*  structures                   */
        /* --------------------------    */
        sc->iface = pi;
        pi->driver_stats.ether_stats = (PETHER_STATS)&(sc->stats);

        sc->ia_iobase = pi->io_address;
        sc->ia_irq    = pi->irq_val;

        if( tcfe_pcmciaInit(pi, (int *)&sc->ia_irq, (IOADDRESS *)&sc->ia_iobase) < 0 )
        {
#if (DEBUG_TCFE574)
            DEBUG_ERROR("TCFE574_open: tcfe_pcmciaInit failed.", NOVAR, 0, 0);
#endif
            return(FALSE);
        }
    }
    else
    {
        /* --------------------------    */
        /*  Add aditional card type      */
        /*  if needed here and           */
        /*  initialize (ie PCI).         */
        /* --------------------------    */
        return(FALSE);
    }

    return( tcfe_openDevice(pi) );
}


/* ---------------------------------------------------------------------    */
/* TCFE574_close() - close a device                                         */
/*                                                                          */
/* Perform device driver specific processing to disable sending and         */
/* receiving packets.                                                       */
/*                                                                          */
/* Inputs:                                                                  */
/*   pi - interface structure of the device to close                        */
/*                                                                          */
/* Returns: nothing                                                         */
/* ---------------------------------------------------------------------    */

void TCFE574_close(PIFACE pi)
{
PTCFE574_SOFTC sc;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_close", NOVAR , 0, 0);
#endif

    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
        return;

    DRIVER_MASK_ISR_OFF(sc->ia_irq);

    if( !TCFE_DisableInterrupts( sc->ia_iobase ) )
    {
        DEBUG_ERROR( "TCFE574_close: TCFE_DisableInterrupts failed!", NOVAR, 0, 0 );
    }

    if( !TCFE_Disable( sc->ia_iobase ) )
    {
        DEBUG_ERROR( "TCFE574_close: TCFE574_Disable failed!", NOVAR, 0, 0 );
    }

#if (INCLUDE_TCFE574_PCMCIA)
    if ( (sc->socket_number != -1) &&
         (sc->socket_number > 0)   &&
         (sc->socket_number < NSOCKET) )
    {
        pcmctrl_card_down(sc->socket_number);
    }
#endif

    return;
}


/* ---------------------------------------------------------------------    */
/* INTERRUPT routines                                                       */
/* ---------------------------------------------------------------------    */
void TCFE574_pre_interrupt(int deviceno)
{
PTCFE574_SOFTC sc;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_pre_interrupt", NOVAR , 0, 0);
#endif


    sc = off_to_TCFE574_softc(deviceno);
    if( !sc )
        return;

    /* --------------------------    */
    /*  The isr will be masked on    */
    /*  again when the strategy      */
    /*  routine called from the      */
    /*  interrupt task returns       */
    /* --------------------------    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq)
    return;
}

/* ---------------------------------------------------------------------    */
/* TCFE574_interrupt() - process driver interrupt                           */
/*                                                                          */
/* Processing routine for device driver interrupt.  It should process       */
/* transmit complete interrupts, input interrupts as well as any            */
/* indicating errors.                                                       */
/*                                                                          */
/* If an input packet arrives it is sent to the IP exchange (which          */
/* will signal the IP task to wake up).                                     */
/*                                                                          */
/* If an output xmit completed, it will wake up the IP tasks to             */
/* process the output list.                                                 */
/*                                                                          */
/* Input:                                                                   */
/*   minor_number - minor number of device driver which caused the          */
/*   interrupt                                                              */
/* Returns: nothing                                                         */
/* ---------------------------------------------------------------------    */

void TCFE574_interrupt(int minor_no)
{
PTCFE574_SOFTC sc;
PIFACE         pi;
word           status;


#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_interrupt", NOVAR , 0, 0);
#endif


    sc = off_to_TCFE574_softc(minor_no);
    if (!sc)
    {
        set_errno(ENUMDEVICE);
        return;
    }

    pi = sc->iface;
    if (!pi)
        goto mask_isr_on;

    /* --------------------------    */
    /*           BANK 1              */
    /* --------------------------    */
    if( !TCFE_SwitchBank( sc->ia_iobase, CBank1 ) )
        goto mask_isr_on;

    /* --------------------------    */
    /* Clear the interrupt status    */
    /* --------------------------    */
    status = 0;
    status = TCFE_ObtainStatus( sc->ia_iobase );

    /* --------------------------    */
    /*  Process all the interrupt    */
    /*  cases that are pending.      */
    /* --------------------------    */
    do {

#if (DEBUG_TCFE574)
        DEBUG_ERROR("TCFE574_interrupt: status: ", EBS_INT1, status, 0);
#endif

        if( status & SInterruptLatch )
        {
#if (DEBUG_TCFE574)
            DEBUG_ERROR("TCFE574_interrupt: ==== SInterruptLatch ====", NOVAR, 0, 0);
#endif
            if( !TCFE_ProcessCommand( sc->ia_iobase, CAcknowledgeInterrupt | CInterruptLatchAck, TCFE_WAIT ) )
                goto mask_isr_on;
        }
        else
        {
#if (DEBUG_TCFE574)
            DEBUG_ERROR("TCFE574_interrupt: No further interrupts need be handled.", NOVAR, 0, 0);
#endif
            /* --------------------------    */
            /*  Clear all interrupts         */
            /*  enable and return.           */
            /* --------------------------    */
            TCFE_EnableInterrupts( sc->ia_iobase );
            goto mask_isr_on;
        }

        /* --------------------------    */
        /*  A transmit has completed,    */
        /*  check for error, tell the    */
        /*  OS, count it and ack.        */
        /* --------------------------    */
        if( status & STxComplete )
        {
#if (DEBUG_TCFE574)
            DEBUG_ERROR("TCFE574_interrupt: ==== STxComplete ====", NOVAR, 0, 0);
#endif
            if( !TCFE_TransmitComplete( sc->ia_iobase, (PETHER_STATS) &(sc->stats) ) )
            {
#if (DEBUG_TCFE574)
                DEBUG_ERROR("TCFE574_interrupt: Tx error.", NOVAR, 0, 0);
#endif
                /* --------------------------    */
                /*  Tell the IP task that        */
                /*  the Tx completed             */
                /*  unsuccessfully.              */
                /* --------------------------    */
                pi->xmit_status = ENETDOWN;
            }

            /* --------------------------    */
            /*  Tell the OS that the Tx      */
            /*  completed.                   */
            /* --------------------------    */
            ks_invoke_output(sc->iface);

            /* --------------------------    */
            /*        Ack the Tx.            */
            /* --------------------------    */
            TCFE_WriteByte( sc->ia_iobase, TCFE_TX_STATUS, NextTxStatus );
        }

        /* --------------------------    */
        /*  A receive has completed,     */
        /*  check for error, tell the    */
        /*  OS, count it and ack.        */
        /* --------------------------    */
        if( status & SRxComplete )
        {
#if (DEBUG_TCFE574)
            DEBUG_ERROR("TCFE574_interrupt: ==== SRxComplete ====", NOVAR, 0, 0);
#endif

            if( !TCFE_ReceiveComplete( sc->ia_iobase, sc->iface, (PETHER_STATS) &(sc->stats) ) )
            {
                /* --------------------------    */
                /*          Rx Error.            */
                /* --------------------------    */
#if (DEBUG_TCFE574)
                DEBUG_ERROR("TCFE574_interrupt: Rx Error.", NOVAR, 0, 0);
#endif
                /* --------------------------    */
                /*  If Error, we still           */
                /*  attempt to pass any Rx'ed    */
                /*  messages before the error    */
                /*  on to the ip layer.          */
                /* --------------------------    */
            }

#if (DEBUG_TCFE574)
            DEBUG_ERROR("TCFE574_interrupt: Rx complete.", NOVAR, 0, 0);
#endif

            /* --------------------------    */
            /*        Ack the NIC.           */
            /* Automatically acknowledged    */
            /* after discarding the data.    */
            /* --------------------------    */
        }

        status = TCFE_ObtainStatus( sc->ia_iobase );

    } while ( status != 0 );


mask_isr_on:
    DRIVER_MASK_ISR_ON(sc->ia_irq);
}

/* ---------------------------------------------------------------------   */
/* TCFE574_xmit_done() - process a completed transmit                      */
/*                                                                         */
/* This routine is called as a result of the transmit complete             */
/* interrupt occuring (see ks_invoke_output).                              */
/*                                                                         */
/* Inputs:                                                                 */
/*   pi     - interface structure                                          */
/*   DCU    - packet transmitting                                          */
/*   status - TRUE indicates the xmit completed successfully, FALSE        */
/*            indicates it did not (possible errors include                */
/*            timeout etc)                                                 */
/*                                                                         */
/* Returns: TRUE  if xmit done or error                                    */
/*          FALSE if xmit not done; if it is not done when the             */
/*                next xmit interrupt occurs, TCFE574_xmit_done will       */
/*                be called again                                          */
/*                                                                         */

RTIP_BOOLEAN TCFE574_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PTCFE574_SOFTC sc;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_xmit_done", NOVAR , 0, 0);
#endif


    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
        return(FALSE);

    if( success )
    {
        sc->stats.packets_out++;
        sc->stats.bytes_out += DCUTOPACKET(msg)->length;
    }
    else
    {
        sc->stats.errors_out++;

        /* --------------------------    */
        /*  If not success, then         */
        /*  reset the Tx.                */
        /* --------------------------    */
        if( !TCFE_ProcessCommand( sc->ia_iobase, CTxReset, TCFE_WAIT ) )
            return(FALSE);

        if( !TCFE_ProcessCommand( sc->ia_iobase, CTxEnable, TCFE_NO_WAIT ) )
            return(FALSE);
    }

    /* --------------------------    */
    /*  Check the Tx status and      */
    /*  if the Tx was shutdown do    */
    /*  to the Tx queue being        */
    /*  full, or the if the packet   */
    /*  encountered too many         */
    /*  collisions, or the host      */
    /*  couldn't supply the          */
    /*  packet, or it is simply      */
    /*  taking too long, we will     */
    /*  enable the Tx. The reset     */
    /*  and enble will be done in    */
    /*  the TCFE_TransmitComplete    */
    /*  if needed.                   */
    /* --------------------------    */
    if( !TCFE_TransmitComplete( sc->ia_iobase, (PETHER_STATS) &(sc->stats) ) )
    {
#if (DEBUG_TCFE574)
        DEBUG_ERROR("TCFE574_xmit_done: A Tx error was found, but handled.", NOVAR , 0, 0);
#endif
    }


    return(TRUE);
}

/* ---------------------------------------------------------------------   */
/* TCFE574_xmit() - transmit a packet                                      */
/*                                                                         */
/* Starts transmitting packet msg.  TCFE574_xmit_done will be called       */
/* when transmit complete interrupt occurs.                                */
/*                                                                         */
/* Inputs:                                                                 */
/*   pi  - interface structure of the device to open                       */
/*   msg - packet to transmit where                                        */
/*         DCUTOPACKET(msg)->length - length of packet                     */
/*         DCUTODATA(msg)           - packet                               */
/*                                                                         */
/* Returns: REPORT_XMIT_DONE if xmit is done                               */
/*          0 if the transmit is started but not done                      */
/*          errno if an error occured                                      */
/*                                                                         */

int TCFE574_xmit(PIFACE pi, DCU msg)
{
PTCFE574_SOFTC sc;
int tx_bytes;
int ret_val;
int length;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_xmit", NOVAR , 0, 0);
#endif

    ret_val = 0; /* anticipate success */
    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
    {
        set_errno(ENUMDEVICE);
        return(ENUMDEVICE);
    }

    length = DCUTOPACKET(msg)->length;

    if (length < ETHER_NOCHK_MIN_LEN)
    {
        length = ETHER_NOCHK_MIN_LEN;
    }

    if (length > ETHERSIZE)
    {
        /* --------------------------    */
        /*  This should NEVER happen.    */
        /*  If it does, your bug is      */
        /*  located in one of the        */
        /*  higher layers.               */
        /* --------------------------    */
#if (DEBUG_TCFE574)
        DEBUG_ERROR("TCFE574_xmit: Message length is too long.  Reducing to ", EBS_INT1, ETHERSIZE, 0);
#endif
        length = ETHERSIZE;
    }

    /* --------------------------    */
    /*  Disable interrupts. Use      */
    /*  ks_disable because it is     */
    /*  not called from the ISR.     */
    /*  If it is changed to be       */
    /*  called from the ISR use      */
    /*  ks_splx() so to keep the     */
    /*  nesting of the disable       */
    /*  and enables correct.         */
    /* --------------------------    */
    ks_disable();

    if( !(tx_bytes = TCFE_Transmit(sc->ia_iobase, DCUTODATA(msg), length)) )
    {
        ret_val = REPORT_XMIT_DONE;
    }
#if (DEBUG_TCFE574)
    DEBUG_ERROR("TCFE574_xmit: Transmitted, out of ", EBS_INT2, tx_bytes, length);
#endif

    if( !TCFE_TransmitComplete( sc->ia_iobase, (PETHER_STATS) &(sc->stats) ) )
    {
#if (DEBUG_TCFE574)
        DEBUG_ERROR("TCFE574_interrupt: Tx error.", NOVAR, 0, 0);
#endif
        /* --------------------------    */
        /*  Tell the IP task that        */
        /*  the Tx completed             */
        /*  unsuccessfully.              */
        /* --------------------------    */
        pi->xmit_status = ENETDOWN;
    }

    /* --------------------------    */
    /*  Tell the OS that the Tx      */
    /*  completed.                   */
    /* --------------------------    */
    ks_invoke_output(sc->iface);

    /* --------------------------    */
    /*     Enable interrupts.        */
    /* --------------------------    */
    ks_enable();


    return(ret_val);
}

/* ---------------------------------------------------------------------   */
/* TCFE574_setmcast() - Set up for multicast                               */
/*                                                                         */
/* When setting up, if in the TCFE_ReceiveFilter() the                     */
/* CReceiveMulticastHash bit is set, the hash table setting                */
/* here will be used.                                                      */
/*                                                                         */
/* Returns: TRUE if successful FALSE if error                              */
/*                                                                         */

RTIP_BOOLEAN TCFE574_setmcast(PIFACE pi)
{
PTCFE574_SOFTC sc;
unsigned short table;
unsigned short bit;
byte           i;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_setmcast", NOVAR , 0, 0);
#endif

    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
        return(FALSE);

    /* --------------------------    */
    /*  There are eight registers    */
    /*  that make up the hash        */
    /*  table. Call                  */
    /*  The filter should be a       */
    /*  6bit number from 0 to 63     */
    /*  that represents the hashed   */
    /*  values of the destination    */
    /*  address in the multicast     */
    /*  packet to be accepted.       */
    /* --------------------------    */

    /* Set multicast filter on chip.                  */
    /* If none needed lenmclist will be zero          */
    TCFE574_getmcaf((PFBYTE) pi->mcast.mclist, pi->mcast.lenmclist, (PFBYTE) &sc->rx_mcast[0]);

    /* loop through each bit in the filter table. If set, turn on the corresponding bit     */
    for( i=0; i<64; i++ )
    {
        bit   = ((unsigned short)i % 8) << 1;   /* __st__ resides in bits 1,2,3 */
        table = ((unsigned short)i / 8) << 4;   /* __st__ resides in bits 4,5,6 */

        if( sc->rx_mcast[i] != 0 )
        {
            if( !TCFE_ProcessCommand( sc->ia_iobase, (unsigned short)(CMulticastHashTable | table | bit | CAddressAccept), TCFE_WAIT ) )
                return(FALSE);
        }
        else
        {
            if( !TCFE_ProcessCommand( sc->ia_iobase, (unsigned short)(CMulticastHashTable | table | bit | CAddressPrevent), TCFE_WAIT ) )
                return(FALSE);
        }
    }
    return(TRUE);
}

/* ---------------------------------------------------------------------   */
/*  Taken from our 3Com 3C90X driver.                                      */

void TCFE574_getmcaf(PFBYTE mclist, int lenmclist, PFBYTE af)
{
int bytesmclist;
byte c;
PFBYTE cp;
dword crc;
int i, len, offset;

    bytesmclist = lenmclist * ETH_ALEN;

    /*
     * Set up multicast address filter by passing all multicast addresses
     * through a crc generator, and then using the high order 6 bits as an
     * index into the 64 bit logical address filter.  The high order bit
     * selects the word, while the rest of the bits select the bit within
     * the word.
     */

    /* the driver can only handle 64 addresses; if there are more than      */
    /* 64 than accept all addresses; the IP layer will filter the           */
    /* packets                                                              */
    if (lenmclist > 64)
        for (i = 0; i < 64; i ++) af[i] = 1;
    else
    {
        for (offset = 0; offset < bytesmclist; offset += 6)
        {
            cp = mclist + offset;
            crc = 0xffffffffL;
            for (len = 6; --len >= 0;)
            {
                c = *cp++;
                for (i = 8; --i >= 0;)
                {
                    if (((crc & 0x80000000L) ? 1 : 0) ^ (c & 0x01))
                    {
                        crc <<= 1;
                        crc ^= 0x04c11db6L | 1;
                    } else
                        crc <<= 1;
                    c >>= 1;
                }
            }

            /* Just want the 6 most significant bits.     */
            crc >>= 26;

            /* Turn on the corresponding bit in the filter.     */
            af[crc] = 1;
        }
    }
}

/* ---------------------------------------------------------------------   */
/* TCFE574_statistics() - update statistics                                */
/*                                                                         */
/* Update statistics in interface structure which are kept in              */
/* a structure specific to this device driver                              */
/*                                                                         */
/* Returns: TRUE if successful FALSE if error                              */
/*                                                                         */

RTIP_BOOLEAN TCFE574_statistics(PIFACE pi)
{
    PTCFE574_SOFTC sc;
    PETHER_STATS p;

#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("TCFE574_statistics", NOVAR , 0, 0);
#endif

    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
        return(FALSE);

    p = (PETHER_STATS) &(sc->stats);
    UPDATE_SET_INFO( pi, interface_packets_in,  p->packets_in  )
    UPDATE_SET_INFO( pi, interface_packets_out, p->packets_out )
    UPDATE_SET_INFO( pi, interface_bytes_in,    p->bytes_in    )
    UPDATE_SET_INFO( pi, interface_bytes_out,   p->bytes_out   )
    UPDATE_SET_INFO( pi, interface_errors_out,  p->errors_out  )

    return(TRUE);
}


/* 3cfe.c contains all the low level functions for access to the 3Com
 * card.  It's implemented as a code include so that the driver module
 * is integral but the source files are more easily managed.
 */
#include "3cfe.c"


/* =================================================================              */
/* =================================================================              */
/* Driver utility level routines                                                  */
/* =================================================================              */
/* ********************************************************************           */
/*                                                                                */
/* Return Type: RTIP_BOOLEAN                                                      */
/* Routine:     tcfe_openDevice                                                   */
/* Description: This is the work horse used to open the TCFE574 device            */
/*              NIC.  It reads a few of the internals from the card,              */
/*              then calls the H/W setup routine                                  */
/* Input:       PIFACE pi - the per device data structure used to manage          */
/*              individual instances of the NIC served by this driver.            */
/* Output:      TRUE on successful open/init, else FALSE                          */
/*                                                                                */
/* ********************************************************************           */

RTIP_BOOLEAN tcfe_openDevice(PIFACE pi)
{
word config_low;
word config_high;
unsigned short i;
PTCFE574_SOFTC sc;
unsigned short getMacAddr = 1;


#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("tcfe_openDevice", NOVAR , 0, 0);
#endif


    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
        goto openfail;

    if( !TCFE_Init( sc->ia_iobase ) )
    {
#if (DEBUG_TCFE574)
        DEBUG_ERROR("tcfe_openDevice: TCFE_Init failed with error: ", NOVAR, 0, 0);
#endif
        goto openfail;
    }


    /* --------------------------    */
    /*           BANK 0              */
    /*  Obtain the MAC address       */
    /*  from the EEPROM, if it       */
    /*  wasn't found in the CIS.     */
    /*  (EEPROM / Identification)    */
    /* --------------------------    */
    for( i=0; i<6; i++ )
    {
        if( pi->addr.my_hw_addr[i] != 0 )
        {
            getMacAddr = 0;
            break;
        }
    }
    if( getMacAddr )
        if( !TCFE_GetHWAddress( sc->ia_iobase, (byte *)&(pi->addr.my_hw_addr[0])) )
            goto openfail;


    /* --------------------------    */
    /*           BANK 1              */
    /*  No need to alter this        */
    /*  Bank containing tx and       */
    /*  rx information.              */
    /* --------------------------    */


    /* --------------------------    */
    /*           BANK 2              */
    /* --------------------------    */
    if( !TCFE_SwitchBank( sc->ia_iobase, CBank2 ) )
        goto openfail;

    /* --------------------------    */
    /*  Adding MAC Address to        */
    /*  I/O Bank 2                   */
    /*  (StationAddress).            */
    /*  Then clear the Mask.         */
    /* --------------------------    */
    for (i = STATION_ADDR_LO; i < STATION_ADDR_HI + 0x02; i = i + 0x01)
    {
        /* --------------------------    */
        /* STATION_ADDR_LO   Byte 0,1    */
        /* STATION_ADDR_MID  Byte 2,3    */
        /* STATION_ADDR_HI   Byte 4,5    */
        /* --------------------------    */
        TCFE_WriteByte(sc->ia_iobase, i, pi->addr.my_hw_addr[i]);
    }
    TCFE_WriteWord(sc->ia_iobase, STATION_MASK_LO,  0);
    TCFE_WriteWord(sc->ia_iobase, STATION_MASK_MID, 0);
    TCFE_WriteWord(sc->ia_iobase, STATION_MASK_HI,  0);


    /* --------------------------    */
    /*           BANK 3              */
    /* --------------------------    */
    if( !TCFE_SwitchBank( sc->ia_iobase, CBank3 ) )
        goto openfail;

    config_low  = 0x0000;
    config_high = 0x0000;

    config_low  = TCFE_ReadWord(sc->ia_iobase, INTERNAL_CONFIG0);
    config_high = TCFE_ReadWord(sc->ia_iobase, INTERNAL_CONFIG2);

    TCFE_WriteWord(sc->ia_iobase, INTERNAL_CONFIG0, config_low                  );
    TCFE_WriteWord(sc->ia_iobase, INTERNAL_CONFIG2, XcvrSelect | AutoNegotiation);

    TCFE_WriteWord(sc->ia_iobase, MAC_CONTROL,   FullDuplexEnable);

    TCFE_WriteWord(sc->ia_iobase, RESET_OPTIONS, MiiDevice | PwrCntl);

    /* --------------------------    */
    /*      Reset Rx and Tx          */
    /* --------------------------    */
    if( !TCFE_Reset(sc->ia_iobase) )
    {
#if (DEBUG_TCFE574)
        DEBUG_ERROR("tcfe_openDevice: could not reset the Rx and Tx.", NOVAR, 0, 0);
#endif
        goto openfail;
    }


    /* --------------------------    */
    /*           BANK 4              */
    /* --------------------------    */
    if( !TCFE_SwitchBank( sc->ia_iobase, CBank4 ) )
        goto openfail;

    /* --------------------------    */
    /*  Only want to set other       */
    /*  values here if planning      */
    /*  to test in loop back.        */
    /* --------------------------    */

    /* --------------------------    */
    /*  Extend the range of the      */
    /*  Rcvd and Xmitted from 15     */
    /*  to 20 bits to reduce the     */
    /*  UdateStatistics interrupt.   */
    /* --------------------------    */
    TCFE_WriteWord(sc->ia_iobase, NETWORK_DIAGNOSTIC, UpperBytesEnable);

    /* --------------------------    */
    /*  Reset the Bad Start of       */
    /*  Stream Delimiter.            */
    /* --------------------------    */
    TCFE_ReadByte(sc->ia_iobase, BAD_SSD);


    /* --------------------------    */
    /*           BANK 5              */
    /*  Contains read only values    */
    /*  for Threshold and            */
    /*  Interrupt purposes.          */
    /* --------------------------    */


    /* --------------------------    */
    /*           BANK 6              */
    /* --------------------------    */

    /* --------------------------    */
    /*     Reset Statistics.         */
    /*  This function will jump      */
    /*  to Bank 6 for us.            */
    /* --------------------------    */
    TCFE_ResetStatistics(sc->ia_iobase);


    /* --------------------------    */
    /*           BANK 7              */
    /*  No need to alter these       */
    /*  values used for              */
    /*  MasterAddress/Satus/Len.     */
    /* --------------------------    */


    /* --------------------------    */
    /*      COMMAND REGISTER         */
    /*  1. Select the Bank to be     */
    /*     used for communication.   */
    /*  2. Enable Rx.                */
    /*  3. Enable Tx.                */
    /*  4. Clear each type of        */
    /*     interrupt in the          */
    /*     Interrupt Status reg.     */
    /*  5. Set Interrupt reg.        */
    /*  6. Set Indication reg.       */
    /*  7. Set up the Rx filter.     */
    /*  8. Set the threshold for     */
    /*     early Rx.                 */
    /*  9. Set the threshold on      */
    /*     Rx before starting to     */
    /*     Tx.                       */
    /* 10. Enable statistics.        */
    /* --------------------------    */

    /*  1.   */
    if( !TCFE_SwitchBank( sc->ia_iobase, CBank1 ) )
        goto openfail;

    /*  2.    */
    /*  3.    */
    if( !TCFE_Enable( sc->ia_iobase ) )
        goto openfail;

    /*  4.                           */
    /*  5.                           */
    /*  6.                           */
    /* --------------------------    */
    /*  Hook the interrupt           */
    /*  service routines             */
    /* --------------------------    */
    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)TCFE574_interrupt,
                      (RTIPINTFN_POINTER) (RTIPINTFN_POINTER)TCFE574_pre_interrupt,
                      pi->minor_number);

    if( !TCFE_EnableInterrupts( sc->ia_iobase ) )
        goto openfail;

    /*  7.   */
    if( !TCFE_ReceiveFilter( sc->ia_iobase ) )
        goto openfail;

    /*  8.    */
    /*  9.    */
    if( !TCFE_SetThresholds( sc->ia_iobase ) )
        goto openfail;

    /* 10.   */
    if( !TCFE_EnableStatistics( sc->ia_iobase ) )
        goto openfail;

    /* --------------------------    */
    /*     Enable interrupts.        */
    /* --------------------------    */
    ks_enable();

    return(TRUE);

openfail:
    DEBUG_ERROR("tcfe_openDevice: 3Com 3CFE574 pcmcia open device failed.", NOVAR, 0, 0);
    set_errno(EDEVOPENFAIL);
    return(FALSE);
}


#if (INCLUDE_TCFE574_PCMCIA)
/* ---------------------------------------------------------------------        */
/*                                                                              */
/* Type: int                                                                    */
/* Routine: tcfe_pcmciaInit                                                     */
/* Description:                                                                 */
/* Input: PIFACE pi - the per device data structure used to manage              */
/*        individual instances of the NIC served by this driver.                */
/*        int *pirq - Pointer to the int location to return the irq             */
/*        information.                                                          */
/*        IOADDRESS *ioBase - pointer to the int location to return the         */
/*        base IO address where the NIC will respond.                           */
/* Output:                                                                      */
/*                                                                              */
/* ---------------------------------------------------------------------        */

int tcfe_pcmciaInit(PIFACE pi, int *pirq, IOADDRESS *pbase_addr)
{
PTCFE574_SOFTC sc;

/* --------------------------    */
/*  init to keep compiler        */
/*  happy                        */
/* --------------------------    */
int found = -1;
int socket_number = 0;


#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("tcfe_pcmciaInit", NOVAR , 0, 0);
#endif



    sc = iface_to_TCFE574_softc(pi);
    if( !sc )
        return(-1);


    sc->socket_number = -1;


    found = -1;
    for (socket_number = 0; socket_number < NSOCKET; socket_number++)
    {
        /* --------------------------    */
        /*  Check if a card is           */
        /*  installed and powered up.    */
        /*  PCMCIA_TCFE574_DEVICE can    */
        /*  be found in pcmcia.h.        */
        /* --------------------------    */
        if (pcmcia_card_type(socket_number) == PCMCIA_TCFE574_DEVICE)
            found++;

        if (found == pi->minor_number)
            break;                              /* The card we want */
    }

    if (found != pi->minor_number)
        goto probefail;

    /* --------------------------    */
    /*  determine which device       */
    /*  and initialize               */
    /* --------------------------    */
#if (INCLUDE_TCFE574_PCMCIA)
    if( !card_is_tcfe574(socket_number, *pirq, (int)*pbase_addr, pi->addr.my_hw_addr) )
    {
#if (DEBUG_TCFE574)
        DEBUG_ERROR( "tcfe_pcmciaInit: 3COM 3CFE574 not found or not recognized.", NOVAR, 0, 0);
#endif
        goto probefail;
    }
#endif


probedone:
    sc->socket_number = socket_number;
    return(socket_number);

probefail:
    DEBUG_ERROR("tcfe_pcmciaInit: 3Com 3CFE574 pcmcia probe failed.", NOVAR, 0, 0);
    sc->socket_number = -1;
    set_errno(EPROBEFAIL);
    return(-1);

}
#endif /* INCLUDE_TCFE574_PCMCIA */



/* =================================================================              */
/* =================================================================              */
/* Driver API                                                                     */
/* =================================================================              */
#if (INCLUDE_TCFE574_PCMCIA)
int xn_bind_tcfe574_pcmcia(int minor_number)
{
#if (DEBUG_TCFE574_TRACE)
    DEBUG_ERROR("xn_bind_tcfe574_pcmcia: ", EBS_INT1 , minor_number, 0);
#endif

    return(xn_device_table_add(TCFE574_pcmcia_device.device_id,
                        minor_number,
                        TCFE574_pcmcia_device.iface_type,
                        TCFE574_pcmcia_device.device_name,
                        SNMP_DEVICE_INFO(TCFE574_pcmcia_device.media_mib,
                                         TCFE574_pcmcia_device.speed)
                        (DEV_OPEN)TCFE574_pcmcia_device.open,
                        (DEV_CLOSE)TCFE574_pcmcia_device.close,
                        (DEV_XMIT)TCFE574_pcmcia_device.xmit,
                        (DEV_XMIT_DONE)TCFE574_pcmcia_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)TCFE574_pcmcia_device.proc_interrupts,
                        (DEV_STATS)TCFE574_pcmcia_device.statistics,
                        (DEV_SETMCAST)TCFE574_pcmcia_device.setmcast));
}
#endif      /* INCLUDE_TCFE574_PCMCIA */


#endif      /* !DECLARING_DATA */
#endif      /* INCLUDE_TCFE574 */

