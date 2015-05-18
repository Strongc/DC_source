/*                                                                                     */
/* lanceisa.c                                                                          */
/*                                                                                     */
/*   EBS - RTIP                                                                        */
/*                                                                                     */
/*   Copyright EBSnet, Inc., 1999                                                      */
/*   All rights reserved.                                                              */
/*   This code may not be redistributed in source or linkable object form              */
/*   without the consent of its author.                                                */
/*                                                                                     */
/*                                                                                     */
/*   Module description:                                                               */
/*      This device driver controls the Am79C90, Am79C961(PCnet-ISA+),                 */
/*          and the TARGET_186ES (an AMD evaluation card with an on-board 186 CPU and  */
/*      an ethernet controller that is like the Am79C961 with a little different       */
/*      DMA controller).                                                               */
/*                                                                                     */
/*   Data Structure Definition:                                                        */
/*      If the driver is run in shared memory mode, the memory is                      */
/*      divided up as follows:                                                         */
/*          array of rx ring descriptors                                               */
/*          array of tx ring descriptors                                               */
/*          initialization block                                                       */
/*          rx packets pointed to by ring descriptors                                  */
/*          tx packets pointed to by ring descriptors                                  */
/*      If the driver is run in bus master mode (the default), a separate              */
/*          DCU is allocated to hold the rx ring descriptors, the tx ring descriptors, */
/*          and the init block.  In addition, DCUs are also allocated to               */
/*          hold the rx packets (DCUs for the tx packets are allocated                 */
/*          dynamically).                                                              */
/*      The private lance data structure LANCE_ISA_T_PRIV holds pointers to            */
/*          the rx ring, tx ring, init block, wherever they are located                */
/*          (shared memory or in DCUs).  LANCE_ISA_T_PRIV also holds the addresses     */
/*          of the DCUs allocated for rx packets in Bus Master Mode (so they           */
/*          may be freed later), and the addresses of the rx packets in                */
/*          shared memory.                                                             */
/*                                                                                     */
/*  NOTE: This source code is arranged with routines entered through the device table  */
/*        first, in device table order.  Support routines follow after all of the      */
/*        device table entry routines.                                                 */
/*                                                                                     */
/*   Revision history:                                                                 */
/*      December 2001   P. Van Oudenaren. From rtlance.c                               */
/*                                                                                     */
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_LANCE_ISA)
#include "lanceisa.h"

#if (defined (RTKBCPP)) /* for power pack we allocate this */
#if (!INCLUDE_MALLOC_DCU_INIT)
#error DPMI must use MALLOC PACKET_INIT for this card
#endif
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define DEBUG_LANCE_ISA 0
#undef SET_MAC_ADDR
#define SET_MAC_ADDR 0

#define PROGRAM_LEDS 1  /* define to enable LED programming */

/* ********************************************************************   */
RTIP_BOOLEAN lance_isa_open(PIFACE pi);
void         lance_isa_close(PIFACE pi);
int          lance_isa_tx_packet(PIFACE pi, DCU msg);
RTIP_BOOLEAN lance_isa_tx_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN lance_isa_statistics(PIFACE  pi);
RTIP_BOOLEAN lance_isa_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
PLANCE_ISA_PRIV lance_isa_s_priv_table[CFG_NUM_LANCE_ISA];
word lance_isa_mode_table[CFG_NUM_LANCE_ISA]={0};
word lance_isa_is_open=0;

#if (SET_MAC_ADDR)
    byte mac_isa_addr[6] = {0x00,0x12,0x34,0x56,0x78,0x9a};
#endif

struct lance_isa_chip_type lance_isa_chip_table[5] =
{
    {0x0000, "LANCE 7990",              /* Ancient lance chip. */
        (int)(LANCE_MUST_PAD + LANCE_MUST_UNRESET)},
    {0x0003, "PCnet/ISA 79C960",        /* 79C960 PCnet/ISA. */
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
    {0x2260, "PCnet/ISA+ 79C961",       /* 79C961 PCnet/ISA+, Plug-n-Play. */
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
    {0x2261, "PCnet/ISA+ 79C961A",      /* 79C961A PCnet/ISA+, Plug-n-Play. */
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
    {0x0,    "PCnet (unknown)",
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
};

EDEVTABLE KS_FAR lance_isa_device =
{
    lance_isa_open, lance_isa_close, lance_isa_tx_packet, lance_isa_tx_done,
    NULLP_FUNC, lance_isa_statistics, lance_isa_setmcast,
    LANCE_ISA_DEVICE, "LANCE_ISA", MINOR_0, ETHER_IFACE,
    SNMP_DEVICE_INFO(CFG_OID_LANCE, CFG_SPEED_LANCE)
    CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
    CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
    IOADD(0x300), EN(0), EN(5)
};
#endif  /* DECLARING_DATA|| BUILD_NEW_BINARY */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
extern EDEVTABLE KS_FAR lance_isa_device;
extern PLANCE_ISA_PRIV lance_isa_s_priv_table[CFG_NUM_LANCE_ISA];
extern word lance_isa_mode_table[CFG_NUM_LANCE_ISA];
extern word lance_isa_is_open;

#if (SET_MAC_ADDR)
extern byte mac_isa_addr[6];
#endif

extern struct lance_isa_chip_type lance_isa_chip_table[5];
#endif  /* !BUILD_NEW_BINARY */

/* *********************************************************************        */
/*                                                                              */
/*  lance_isa_open                                                              */
/*                                                                              */
/*  This routine is called by the kernel to open the lance device.              */
/*  RTIP considers opening the device to include starting the device.           */
/*  Therefore, this routine maps all RTIP data structures required to interface */
/*  with the driver, initializes all registers required to operate the device,  */
/*  and it starts the device.                                                   */
/*                                                                              */
/*  Input:   pointer to an interface structure                                  */
/*                                                                              */
/*  Output:  TRUE, if device is successfully opened                             */
/*           FALSE, if failure                                                  */
/*                                                                              */
/*                                                                              */
RTIP_BOOLEAN lance_isa_open (PIFACE pi)
{
    PLANCE_ISA_PRIV p_priv; /* ptr to lance driver private data structure */
    int i;                  /* index into rx and tx ring */
    PFBYTE  p;
    dword   l;

    /* Alloc context block   */
    if (!lance_isa_s_priv_table[pi->minor_number])
    {
        p = (PFBYTE) dcu_alloc_core(sizeof(*p_priv)+4);
        /* make sure on 4 byte boundary first     */
        l = (dword) p;
        while (l & 0x3ul) {l++; p++;};
        lance_isa_s_priv_table[pi->minor_number] = (PLANCE_ISA_PRIV) p;
    }

    /*                                                                             */
    /* If can't map the minor device number to an area of memory to hold the       */
    /* private data structure for the lance, return an error indicating that there */
    /* are not enough device numbers available.                                    */
    p_priv = iface_isa_to_priv(pi);
    if (!p_priv)
    {
        set_errno(ENUMDEVICE);
        return (FALSE);
    }

    /*                                                                           */
    /* tc_memset is defined in osenv.c.  It sets a specified number of bytes     */
      /* to the value specified.  Here, memory locations beginning at p_priv and */
    /* continuing for sizeof structure LANCE_ISA_T_PRIV are set to 0.            */
    tc_memset((PFBYTE) p_priv, 0, sizeof(*p_priv));

    /*                                                                             */
    /* mode settings stored in the private structure are defaulted to zeroes.      */
    /* If a "user" has set mode settings for this device, copy those settings      */
    /* into the private structure so that they are used in the init block, as      */
    /* opposed to the defaults.                                                    */
    /*  Note: a user calls lance_isa_set_mode to set these values.  Masks for this */
    /*        register are defined in lance_isa.h.                                 */
    if (lance_isa_mode_table[pi->minor_number] != 0)
        p_priv->mode_settings = lance_isa_mode_table[pi->minor_number];

    /*                                                                             */
    /* Set up the private data structure so that it points to the global interface */
    /* structure. (address is needed later when returning packets)                 */
    p_priv->iface = pi;

    /*                                                                          */
    /* Point the statistics area of the global data structure at the statistics */
    /* area (counters) in the lance private data structure.                     */
    pi->driver_stats.ether_stats = (PETHER_STATS) &(p_priv->stats);

    /*                                                                            */
    /* Set I/O space base address and interrupt number for this device to either  */
    /* the values set by the kernel OR to the default values in the device table. */

#if (RTIP_VERSION >= 30)
                    p_priv->base_addr = pi->io_address;
#else
                    p_priv->base_addr = ed_io_address;
#endif

#if (RTIP_VERSION >= 30)

                    p_priv->irq = pi->irq_val;  /* in lance.c this is bound to (word). why? */
#else
                    p_priv->irq = ed_irq_val;
#endif

    /* Stop the LANCE   */
    lance_isa_stop(p_priv);

    /* Get the version of the LANCE chip.  Things happen based on the   */
    /* chip type (and the flags set based on the type).                 */
    if (!lance_isa_get_chip_version(p_priv))
    {
        set_errno(EPROBEFAIL);
        return(FALSE);
    }

    /* Set up a DMA channel if it's needed   */
    lance_isa_setup_dma(p_priv);

    /* Assign memory to the rx and tx descriptor rings and   */
    /* the initialization block                              */
    lance_isa_assign_rx_ring(p_priv);
    lance_isa_assign_tx_ring(p_priv);
    lance_isa_assign_init_block(p_priv);

    /* Initialize the rx and tx descriptors in the ring arrays.    */
    /* Point the indexes into the arrays to the first (0th) entry. */
    for (i=0; i < NUM_ISA_RX_DESC; i++)
        if (!(lance_isa_init_rx_entry(p_priv, i)))
            return(FALSE);
    p_priv->cur_rx = 0;

    for (i=0; i < NUM_ISA_TX_DESC; i++)
        if (!(lance_isa_init_tx_entry(p_priv, i)))
            return(FALSE);
    p_priv->cur_tx = p_priv->dirty_tx = 0;

    /* Build the lance initialization block.   */
    lance_isa_build_init_block(p_priv);

    /* Start (initialize) the device.   */
    if (!lance_isa_init(p_priv))
        return(FALSE);
    else
        lance_isa_is_open +=1;

    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_close                                                        */
/*                                                                         */
/*  This routine closes the LANCE device.                                  */
/*  Note:  add updates from STL/SMX lance.c close routine.  I made a few   */
/*          changes.                                                       */
/*                                                                         */
/*  Input:  pointer to the lance data structure                            */
/*                                                                         */
/*  Indirect Output: a STOPed LANCE device                                 */
/*                                                                         */
/*  Calls:  lance_isa_free_dcus                                            */
/*                                                                         */
/*  Called by:  This routine is a device driver entry point.               */
/*              It is entered via a jump through the device table.         */
/*                                                                         */
void lance_isa_close(PIFACE pi)
{
    PLANCE_ISA_PRIV p_priv;

    p_priv = iface_isa_to_priv(pi);
    if (!p_priv)
    {
        return;
    }

    /* ?? the old driver code seems to think that if the chip_table flag   */
    /*  LANCE_HAD_MISSED_FRAME is set, CSR 112 needs to be written, BUT    */
    /*  the old code does nothing beyond writing 112 to the RAP.           */
    /*  CSR 112 holds a missed frame count.  If it is reasonable to        */
    /*  write this to the statistics area, we should do this.              */
    /*  I've left it out for now.                                          */
#if (DEBUG_LANCE_ISA)
    /* Set up to read CSR0   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    DEBUG_ERROR("lance_isa_close: Shutting down ethercard, status was ", EBS_INT1,
           SWAPIF(INWORD((p_priv->base_addr)+LANCE_ISA_K_RDP)), 0);
#endif

    /* Stop the LANCE here - otherwise it occasionally polls   */
    lance_isa_stop(p_priv);

    /* Free all DCUs that were allocated for buffer management       */
    /* and the init block.  This happens only if in Bus Master Mode. */
    /* In Shared Memory Mode, the stored addresses are pointers into */
    /* the shared memory.                                            */
#if (CFG_LANCE_BUS_MASTER)
    lance_isa_free_dcus(p_priv);
#endif
    lance_isa_is_open -=1;
    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_tx_packet                                                    */
/*                                                                         */
/*  This routine is called by RTIP code when there is a packet to          */
/*  be transmitted.                                                        */
/*                                                                         */
/*  Input:  Pointer to interface structure, DCU                            */
/*                                                                         */
/*  Output:  Data to be transmitted placed in transmit descriptor ring.    */
/*           OWN bit set to allow LANCE access to the data.                */
/*                                                                         */
/*  Calls:                                                                 */
/*                                                                         */
/*  Called by:  RTIP via the device table                                  */
/*                                                                         */
int lance_isa_tx_packet(PIFACE pi, DCU msg)
{
    PLANCE_ISA_PRIV p_priv;
    LANCE_ISA_TX_DESC   temp_txdesc;
    int length;
    int   tx_index;
    dword csr0;


#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_tx_packet: entering", NOVAR, 0, 0);
#endif
    p_priv = iface_isa_to_priv(pi);
    if (!p_priv)
        return(ENUMDEVICE);


    /*  Determine how much data goes in buffer;   */
    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)
        length = ETHER_MIN_LEN;
    else if (length > ETHERSIZE+4)
    {
    DEBUG_ERROR("lance_isa_tx_packet: length is too large", EBS_INT1,
            DCUTODATA(msg), ETHERSIZE+4);
    DEBUG_ERROR("lance_isa_tx_packet: pkt = ", PKT, DCUTODATA(msg), ETHERSIZE+4);
        length = ETHERSIZE+4;    /* truncating the packet.  a hack from lance.c */
    }



#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_tx_packet: p_priv->cur_tx=; length=;", EBS_INT2,
    p_priv->cur_tx, length);
    DEBUG_ERROR("lance_isa_tx_packet: pkt = ", PKT, DCUTODATA(msg), 25);
#endif

    /*  Check that CPU "OWNS" the current tx entry; (TMD0 & TMD1[7:0]   */
    /*  hold the buffer address).  The msb of TMD1 holds the            */
    /*  ownership bit.                                                  */
    if (p_priv->ptx_ring[p_priv->cur_tx].tmd1 & SWAPIF(LANCE_ISA_M_LANCE_OWNED))
        return(FALSE);      /* no buffer available. */

    /*  The mutual exclusion protocol for the "OWN" bit in a msg.    */
    /*  descriptor requires that no device change the state of any   */
    /*  field in any descriptor entry after relinquishing ownership. */
    /*  Therefore, the own bit must be set last.                     */
    /*                                                               */


    /*  Zero TMD3.                                              */
    /*  Write BCNT in TMD2 as a negative 2's complement #.  The */
    /*  high byte of TMD2 must be ones.                         */
    temp_txdesc.tmd3 = 0;
    temp_txdesc.tmd2 = (word) -length;

    /*  Transfer data to TX buffer   */
#if (CFG_LANCE_SHARED_MEM)          /* Shared Memory Mode */
    /*  unpack the physical address of the tx packet from the ring descriptor.   */
    temp_txdesc.tmd0 = p_priv->ptx_ring[p_priv->cur_tx].tmd0;
    temp_txdesc.tmd1 = p_priv->ptx_ring[p_priv->cur_tx].tmd1;

    /*move the data from the DCU into the tx packet in shared memory   */
    tc_movebytes(p_priv->tx_laddr[p_priv->cur_tx], DCUTODATA(msg), length);
#else                               /* Bus Master Mode */
    /* point the tx ring descriptor to the data area of the DCU   */
    temp_txdesc.tmd0 = LANCE_ISA_LOW_16((PFBYTE)DCUTODATA(msg));
    temp_txdesc.tmd1 = LANCE_ISA_HIGH_8((PFBYTE)DCUTODATA(msg));
#endif

    /*  Set STP bit in TMD1; Set ENP bit in TMD1;                 */
    /*  Set OWN bit in TMD1 relinquishing ownership to the LANCE. */
    temp_txdesc.tmd1 = (RTWORD_ISA)(temp_txdesc.tmd1 | (LANCE_ISA_M_LANCE_OWNED |
                                                    LANCE_ISA_M_TMD1_STP   |
                                                    LANCE_ISA_M_TMD1_ENP));
    /* Transfer from the temp descriptor to the actual descriptor, swapping bytes   */
    /* if this is a power pc; tmd1 last because of OWN bit.                         */
    tx_index = p_priv->cur_tx;
    p_priv->ptx_ring[p_priv->cur_tx].tmd2 = SWAPIF(temp_txdesc.tmd2);
    p_priv->ptx_ring[p_priv->cur_tx].tmd3 = SWAPIF(temp_txdesc.tmd3);
    p_priv->ptx_ring[p_priv->cur_tx].tmd0 = SWAPIF(temp_txdesc.tmd0);
    ks_disable();
    p_priv->ptx_ring[p_priv->cur_tx].tmd1 = SWAPIF(temp_txdesc.tmd1);

    /*  Increment pointer into tx ring.   */
    LANCE_ISA_INC_TX_PTR(p_priv->cur_tx);
    ks_enable();

    /* Set up to read CSR0   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    csr0=      SWAPIF(INWORD((p_priv->base_addr)+LANCE_ISA_K_RDP));

    /* Trigger an immediate send.   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(csr0 | LANCE_ISA_M_CSR0_TDMD));
/*  OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));   */

/*  OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |   */
/*                                        LANCE_ISA_M_CSR0_TDMD));                */

    /* return 0 because that's what lance.c did, and this rtn is called   */
    /* through the device table.                                          */
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_tx_packet: send- cur_tx, cur_rx = ", EBS_INT2, p_priv->cur_tx, p_priv->cur_rx);
    DEBUG_ERROR("lance_isa_tx_packet: returning", NOVAR, 0, 0);
#endif

    return(0);
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_tx_done                                                      */
/*                                                                         */
/*  This routine processes a packet successfully transmitted by            */
/*  the LANCE.                                                             */
/*                                                                         */
/*                                                                         */
/*  Input:   none                                                          */
/*                                                                         */
/*  Output:  none                                                          */
/*                                                                         */
/*  Calls:                                                                 */
/*                                                                         */
/*  Called by:  RTIP through the device table                              */
/*                                                                         */
RTIP_BOOLEAN lance_isa_tx_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
    PLANCE_ISA_PRIV p_priv;

    p_priv = iface_isa_to_priv(pi);

    if (!p_priv)
        return(FALSE);

#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_tx_done: entering", NOVAR, 0, 0);
#endif

    if (success)
    {
        /* Update total number of successfully transmitted packets.   */
        p_priv->stats.packets_out++;
        p_priv->stats.bytes_out += DCUTOPACKET(msg)->length;
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_tx_done: success=", EBS_INT1, success, 0);
#endif
    }
    else
    {
        /* error - record statistics   */
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_tx_done: error=", EBS_INT1, 0, 0);
#endif
        p_priv->stats.errors_out++;
        p_priv->stats.tx_other_errors++;
        DEBUG_ERROR("lance_isa_tx_done: transmit timed out, status, dirty_tx = ; resetting.", EBS_INT2,
               SWAPIF(INWORD((p_priv->base_addr)+LANCE_ISA_K_RDP)), p_priv->dirty_tx);
/*VK    DEBUG_ERROR("lance_isa_tx_done: cur_tx, cur_rx = ", EBS_INT2, p_priv->cur_tx, p_priv->cur_rx);   */
        /* re-initialize the LANCE chip                                                                  */
        lance_isa_reinit(p_priv);
    }
    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/* lance_isa_statistics                                                    */
/*                                                                         */
/* This routine is called through the device table to record statistics    */
/* for the LANCE board.                                                    */
/*                                                                         */
/* Input:  Pointer to the LANCE interface structure                        */
/*                                                                         */
/* Output: TRUE - if statistics were able to be gathered                   */
/*         FALSE- if unable to gain access to the device data              */
/*                                                                         */
/* Calls:   ks_splx                                                        */
/*          ks_spl                                                         */
/*          XINWORD                                                        */
/*          OUTWORD                                                        */
/*          UPDATE_SET_INFO                                                */
/*                                                                         */
/* Called by: RTIP through the device table                                */
/*                                                                         */
RTIP_BOOLEAN lance_isa_statistics(PIFACE pi)
{
    PLANCE_ISA_PRIV p_priv;
#if (INCLUDE_KEEP_STATS)
    PETHER_STATS  p_stats;
#endif
    int saved_addr;
    KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */

    p_priv = iface_isa_to_priv(pi);
    if (!p_priv)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    /* If CSR112 exists for this version of the LANCE chip, copy its       */
    /* contents to the statistics area (112 holds the missed frame count). */
    if (lance_isa_chip_table[p_priv->chip_version].flags & LANCE_HAS_MISSED_FRAME)
    {
        sp = ks_splx();
        saved_addr = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RAP);
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR112));
        p_priv->stats.rx_other_errors = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(saved_addr));
        ks_spl(sp);
    }

#if (INCLUDE_KEEP_STATS)
    p_stats = (PETHER_STATS) &(p_priv->stats);
    UPDATE_SET_INFO(pi,interface_packets_in, p_stats->packets_in);
    UPDATE_SET_INFO(pi,interface_packets_out, p_stats->packets_out);
    UPDATE_SET_INFO(pi,interface_bytes_in, p_stats->bytes_in)
    UPDATE_SET_INFO(pi,interface_bytes_out, p_stats->bytes_out);
    UPDATE_SET_INFO(pi,interface_errors_in, p_stats->errors_in);
    UPDATE_SET_INFO(pi,interface_errors_out, p_stats->errors_out);
    UPDATE_SET_INFO(pi,interface_packets_lost, p_stats->packets_lost);
#endif
    return(TRUE);
}

/* ********************************************************************           */
/* MULTICAST routines.                                                            */
/* ********************************************************************           */
/* Set or clear the multicast filter for this adaptor.                            */
/*   num_addrs == -1        Promiscuous mode, receive all packets                 */
/*   num_addrs == 0     Normal mode, clear multicast list                         */
/*   num_addrs > 0      Multicast mode, receive normal and MC packets, and do     */
/*                      best-effort filtering.                                    */
/*                                                                                */
/* NOTE:  These routines were taken from the old lance.c driver and modified to   */
/*        fit lance_isa.c structures.  They are untested in lance_isa.c.  5/99,VK */
/*                                                                                */
RTIP_BOOLEAN lance_isa_setmcast(PIFACE pi)
{
PLANCE_ISA_PRIV p_priv;

    p_priv = iface_isa_to_priv(pi);
    if (!p_priv)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    lance_isa_set_mcast_list(p_priv, pi->mcast.lenmclist, pi->mcast.mclist);
    return(TRUE);
}

void lance_isa_set_mcast_list(PLANCE_ISA_PRIV p_priv, int num_addrs, void *addrs)
{
    ARGSUSED_PVOID((PFVOID)addrs);

    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_STOP)); /* Temporarily stop the lance.  */

    if (num_addrs >= 0)
    {
        int multicast_table[4];
        int i;

        /* We don't use the multicast table, but rely on upper-layer filtering.   */
        tc_memset(multicast_table, (num_addrs == 0) ? 0 : -1, sizeof(multicast_table));
        for (i = 0; i < 4; i++)
        {
            OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR8 + i));
            OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(multicast_table[i]));
        }
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR15));
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, 0x0000); /* Unset promiscuous mode */
        lance_isa_mode_table[p_priv->iface->minor_number] = 0x0000;
    }
    else
    {
        DEBUG_ERROR("Promiscuous mode enabled.", NOVAR, 0, 0);
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR15));
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_MODE_PROM)); /* Set promiscuous mode */
        lance_isa_mode_table[p_priv->iface->minor_number] = LANCE_ISA_M_MODE_PROM;
    }

    lance_isa_reinit(p_priv);
}

/* *********************************************************************   */
void lance_isa_pre_isr(int deviceno)
{
PLANCE_ISA_PRIV p_priv; /* ptr to data structure private to lance driver */

    p_priv = off_isa_to_priv(deviceno);
    if (!p_priv)
        return;

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(p_priv->irq)
}

/* *********************************************************************                 */
/*                                                                                       */
/*  lance_isa_isr                                                                        */
/*                                                                                       */
/*  This is the interrupt service routine for the LANCE device.  It determines           */
/*  the reason for the interrupt and transfers control to the appropriate                */
/*  routine based on this reason.                                                        */
/*  The LANCE interrupts the host on completion of its initialization routine,           */
/*  the completion of transmission of a packet, the reception of a packet, a transmitter */
/*  timeout error, a missed packet, and a memory error.                                  */
/*  NOTE: No processing occurs as a result of IDON being set.                            */
/*        The bit is just cleared.  lance_isa_open just waits on this bit                */
/*        being set, but doesn't clear it.                                               */
/*                                                                                       */
/*  Input:  minor device number                                                          */
/*                                                                                       */
/*                                                                                       */
/*  Output: none                                                                         */
/*                                                                                       */
/*                                                                                       */
/*                                                                                       */

void lance_isa_isr (int min_dev_num)
{
    word  csr0; /* temporary storage for CSR0 contents */
    PLANCE_ISA_PRIV   p_priv;   /* ptr to data structure private to lance driver */
    int loop_cnt=10;
    word old_ptr;


    /* Translate the minor device number into the data structure for this    */
    /* device.  If we can't find either the private lance data structure, or */
    /* the RTIP interface structure, return.                                 */
    p_priv = off_isa_to_priv(min_dev_num);
    if (!(p_priv) || !(p_priv->iface))
        return;
#if (DEBUG_LANCE_ISA)
/*  DEBUG_ERROR_INT("lance_isa_isr: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("lance_isa_isr: entering", NOVAR, 0, 0);
#endif

    /* Check flag to see if we are re-entering the isr.   */
    /* Increment flag to indicate we are in the isr.      */
    if (p_priv->in_isr)
    {
/*      DEBUG_ERROR_INT("lance_isa_isr: (Re)entering interrupt handler. in_isr = ", EBS_INT1, p_priv->in_isr, 0);   */
        DEBUG_ERROR("lance_isa_isr: (Re)entering interrupt handler. in_isr = ", EBS_INT1, p_priv->in_isr, 0);
        goto ex_it;;
    }
    p_priv->in_isr += 1;

    /* Save the RAP contents.   */
    old_ptr = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RAP);

    /* Read CSR0 [and write back to clear it - this will clear all   */
    /* "write 1 to clear bits in CSR0]                               */
    OUTWORD ((p_priv->base_addr) + LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));

    while (((csr0 = SWAPIF(INWORD((p_priv->base_addr)+LANCE_ISA_K_RDP))) & (LANCE_ISA_M_CSR0_ERR  |
                                                    LANCE_ISA_M_CSR0_TINT |
                                                    LANCE_ISA_M_CSR0_RINT |
                                                    LANCE_ISA_M_CSR0_IDON))
            && (--loop_cnt >= 0))
    {

#if (DEBUG_LANCE_ISA)
/*  DEBUG_ERROR_INT("lance_isa_isr: csr0=", EBS_INT1, csr0, 0);   */
    DEBUG_ERROR("lance_isa_isr: csr0=", EBS_INT1, (csr0 & 0xffff), 0);
#endif

        /* If there's an initialization interrupt, clear it.   */
        if (csr0 & LANCE_ISA_M_CSR0_IDON)
            OUTWORD ((p_priv->base_addr) + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                                 LANCE_ISA_M_CSR0_IDON) );
        /* Go to the routine that can service the specific interrupt   */
        if (csr0 & LANCE_ISA_M_CSR0_TINT)       /* tx interrupt */
        {
            lance_isa_tx_interrupt(p_priv);
            OUTWORD ((p_priv->base_addr) + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                                 LANCE_ISA_M_CSR0_TINT) );
        }
        if (csr0 & LANCE_ISA_M_CSR0_RINT)       /* rx interrupt */
        {
            lance_isa_rx_interrupt(p_priv);
            OUTWORD ((p_priv->base_addr) + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                                 LANCE_ISA_M_CSR0_RINT) );
        }
        if (csr0 & LANCE_ISA_M_CSR0_ERR)        /* error bit set */
        {
            lance_isa_errors(csr0, p_priv);
            OUTWORD ((p_priv->base_addr) + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                                 LANCE_ISA_M_CSR0_BABL |
                                                 LANCE_ISA_M_CSR0_CERR |
                                                 LANCE_ISA_M_CSR0_MISS |
                                                 LANCE_ISA_M_CSR0_MERR) );
        }
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_isr: cur_rx=; cur_tx=", EBS_INT2, p_priv->cur_rx, p_priv->cur_tx);
#endif
        /* make sure we're pointing to CSR0   */
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    }
    p_priv->in_isr -= 1;

    /* Restore old pointer   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(old_ptr));

ex_it:
    DRIVER_MASK_ISR_ON(p_priv->irq)
    return;
}

/* *********************************************************************             */
/*                                                                                   */
/*  lance_isa_get_chip_version                                                       */
/*                                                                                   */
/*  This routine gets the version of the LANCE chip that's on the board and stores   */
/*  it.  This routine will not work for a plain vanilla LANCE (Am79C90), as it reads */
/*  CSR88 which does not exist on that board, so assume 7990 if CSR88                */
/*  does not exist.                                                                  */
/*  Note: chip_table is defined in lance_isa.h                                       */
/*                                                                                   */
/*  Input:  pointer to private lance data structure                                  */
/*                                                                                   */
/*  Output: TRUE, if chip version is valid                                           */
/*          FALSE, if not valid                                                      */
/*                                                                                   */
/*  Calls: none                                                                      */
/*                                                                                   */
/*  Called by: lance_isa_open                                                        */
/*                                                                                   */
RTIP_BOOLEAN lance_isa_get_chip_version(PLANCE_ISA_PRIV p_priv)
{
    word    rap;
    int lance_version;
    dword chip_version;
    dword ltemp;

    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR88));
    rap = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RAP);
#if (DEBUG_LANCE_ISA)
        DEBUG_ERROR("lance_isa_get_chip_version. RAP = ", EBS_INT1, rap, 0);
#endif
    if (rap != LANCE_ISA_K_CSR88)
    {
        lance_version = 0;
    }
    else                            /* Good, it's a newer chip. */
    {
        chip_version = (dword) XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
        chip_version &= 0xFFFF;
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(89));
        ltemp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
        ltemp &= 0xFFFF;
        chip_version |= ltemp << 16;
#if (DEBUG_LANCE_ISA)
        DEBUG_ERROR("lance_isa_get_chip_version: LANCE chip version is ", EBS_INT1, chip_version, 0);
#endif
        if ((chip_version & 0xfff) != 0x003)
            return(FALSE);
        chip_version = (chip_version >> 12) & 0xffff;
        for (lance_version = 1; lance_isa_chip_table[lance_version].id_number;
             lance_version++)
        {
            if (lance_isa_chip_table[lance_version].id_number ==  chip_version)
                break;
        }

    }
    /* Store index into chip_table in data structure for later use   */
    p_priv->chip_version = lance_version;
    if( lance_version == LANCE_ISA_UNKNOWN) return(FALSE);
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa: get_chip_version: chip name", STR1, lance_isa_chip_table[lance_version].name, 0);
#endif
    return(TRUE);
}
/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_setup_dma                                                    */
/*                                                                         */
/*  This routine                                                           */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: none                                                           */
/*                                                                         */
/*  Calls: none                                                            */
/*                                                                         */
/*  Called by: lance_isa_open                                              */
/*                                                                         */
void lance_isa_setup_dma(PLANCE_ISA_PRIV p_priv)
{
    word isacsr8;

    /* Default the stored dma channel to the cascade channel, ch. 4,   */
    /* which indicates that no DMA channel is active                   */
    /* (Native bus-master, no DMA channel needed).                     */
    p_priv->dma_channel = LANCE_ISA_DMA_CH4;

    /* If this is an ISA-based device with plug'n'play, read ISACSR8   */
    /* for the active dma channel.                                     */
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("LANCE_ISA_SETUP_DMA:  chip_version = ", EBS_INT1, p_priv->chip_version, 0);
#endif

    if ((p_priv->chip_version == PCNET_ISA_ISAP)||(p_priv->chip_version == PCNET_ISA_ISAPA)) /* A plug-n-play version. */
    {
        OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_ISACSR8));
        isacsr8 = XINWORD((p_priv->base_addr)+LANCE_ISA_K_IDP);
        p_priv->dma_channel = (word)(isacsr8 & LANCE_ISA_M_ISACSR8_DMA);

        /* ISACSR8 holds the irq for this device in bits 4-7.   */
        /* Get the irq from here since this is plug 'n' play.   */
        p_priv->irq = (isacsr8 >> 4) & 0x0F;
#if (DEBUG_LANCE_ISA)
        DEBUG_ERROR("lance_isa_setup_dma: irq is ", EBS_INT1, p_priv->irq, 0);
#endif
    }
    else /* if (p_priv->chip_version != PCNET_ISA_FASTP) */
    {
        DEBUG_ERROR("lance_isa_setup_dma: Not PCNET_ISA_ISAP oops ", NOVAR, 0, 0);
        return;
        /* The DMA channel may be passed in PARAM1.      */
/*      if (lp->mem_start & 0x07)                        */
/*          lp->lance_dma = (int)(lp->mem_start & 0x07); */
    }
    /* PVO - 12-28-2001 - What is this dma == 4, no DMA needed ?   */
    if ((p_priv->dma_channel == 4) /* && (p_priv->chip_version != PCNET_ISA_FASTP) */ )
    {
        DEBUG_ERROR("lance_isa_setup_dma: no DMA needed.", NOVAR, 0, 0);
    }

#if (CFG_LANCE_BUS_MASTER)
    else
    {
#if (DEBUG_LANCE_ISA)
        DEBUG_ERROR("lance_isa_setup_dma: bus master mode. dma chnl == ", EBS_INT1,p_priv->dma_channel, 0);
#endif
        lance_isa_dma_init(p_priv->dma_channel);
    }
#endif
    return;
}

#if (CFG_LANCE_BUS_MASTER)

/* *********************************************************************   */
void lance_isa_dma_init(int channel)
{
byte b;
    b = (byte) channel;
    if (channel <= 4)
    {
        b |= CASCADE_MODE;          /* set single mode bit */
        OUTBYTE(DMA_8MODE_REG,b);   /* write to 8 bit dma mode reg. */
        b &= DMA_CHL_FIELD;         /* set proper dma channel bit */
        OUTBYTE(DMA_8MASK_REG,b);   /* write to 8 bit dma mask reg. */
    }
    else
    {
        b &= DMA_CHL_FIELD;         /* set proper dma channel bit */
        OUTBYTE(DMA_16MASK_REG,b);  /* write to 16 bit dma mask reg */
        b |= CASCADE_MODE;          /* set single mode bit */
        OUTBYTE(DMA_16MODE_REG,b);  /* write to 16 bit dma mode reg. */
    }
    return;
}

#endif  /* (BUS MASTER) */


/* *********************************************************************       */
/*                                                                             */
/*  lance_isa_assign_rx_ring                                                   */
/*                                                                             */
/*  This routine assigns memory to the Receive Descriptor Ring.                */
/*                                                                             */
/*  Input:  pointer to the lance data structure                                */
/*                                                                             */
/*  Indirect Output:  pointer to top of rx ring                                */
/*                    pointer to the dcu allocated (if not shared memory mode) */
/*                                                                             */
/*  Calls:  dcu_alloc_core                                                     */
/*                                                                             */
/*  Called by: lance_isa_open                                                  */
/*                                                                             */
void lance_isa_assign_rx_ring(PLANCE_ISA_PRIV p_priv)
{
    union {
        PFBYTE pbyte;
        PRX_ISA_DESC pdesc;
    } dcu_ptr;

    /* If running in Shared Memory Mode, use the memory area defined   */
    /* in xnconf.h.                                                    */
    /* If running in Bus Master Mode, allocate a dcu and use the       */
    /* data area for the ring buffer.                                  */
#if (CFG_LANCE_SHARED_MEM)
    dcu_ptr.pbyte = CFG_LANCE_SH_MEM_ADDR;
#else
    dcu_ptr.pbyte = dcu_alloc_core(SIZEOF_ISA_RX_DESC_ARRAY + RX_TX_DESC_ALIGN_ISA);
    p_priv->prx_dcu = dcu_ptr.pbyte;
#endif

    /* Align the ring buffer address on a quadword boundary.   */
    while ((dword)dcu_ptr.pbyte & RX_TX_DESC_ALIGN_ISA) dcu_ptr.pbyte++;

    /* Store the address in the private structure.   */
    p_priv->prx_ring = dcu_ptr.pdesc;

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_assign_tx_ring                                               */
/*                                                                         */
/*  This routine assigns memory to the Transmit Descriptor Ring.           */
/*                                                                         */
/*  Input:  pointer to lance data structure                                */
/*                                                                         */
/*  Indirect Output:  Pointer to tx_ring                                   */
/*                    pointer to dcu allocated (if not shared memory mode) */
/*                                                                         */
/*  Calls:  dcu_alloc_core                                                 */
/*                                                                         */
/*  Called by: lance_isa_open                                              */
/*                                                                         */
void lance_isa_assign_tx_ring(PLANCE_ISA_PRIV p_priv)
{
    union {
        PFBYTE pbyte;
        PTX_ISA_DESC pdesc;
    } dcu_ptr;

    /* If running in Shared Memory Mode, use the memory area beyond   */
    /* the rx ring at CFG_LANCE_SH_MEM_ADDR defined in xnconf.h.      */
    /* If running in Bus Master Mode, allocate a dcu and use the      */
    /* data area for the ring buffer.                                 */
#if (CFG_LANCE_SHARED_MEM)
    dcu_ptr.pbyte = (PFBYTE) p_priv->prx_ring;
    dcu_ptr.pbyte += (SIZEOF_ISA_RX_DESC_ARRAY);
#else
    dcu_ptr.pbyte = dcu_alloc_core(SIZEOF_ISA_TX_DESC_ARRAY + RX_TX_DESC_ALIGN_ISA);
    p_priv->ptx_dcu = dcu_ptr.pbyte;
#endif

    /* Align the ring buffer address on a quadword boundary.   */
    while ((dword)dcu_ptr.pbyte & RX_TX_DESC_ALIGN_ISA) dcu_ptr.pbyte++;

    /* Store the address in the private structure.   */
    p_priv->ptx_ring = dcu_ptr.pdesc;

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_assign_init_block                                            */
/*                                                                         */
/*  This routine assigns memory to the Initialization Block.               */
/*                                                                         */
/*  Input:  pointer to lance data structure                                */
/*                                                                         */
/*  Indirect Output:  Pointer to init block                                */
/*                    pointer to dcu allocated (if not shared memory mode) */
/*                                                                         */
/*  Calls:  dcu_alloc_core                                                 */
/*                                                                         */
/*  Called by: lance_isa_open                                              */
/*                                                                         */
void lance_isa_assign_init_block(PLANCE_ISA_PRIV p_priv)
{
    union {
        PFBYTE pbyte;
        PINIT_ISA_BLK pinit_block;
    } dcu_ptr;

    /* If running in Shared Memory Mode, use the memory area beyond   */
    /* the rx ring in the CFG_LANCE_SH_MEM_ADDR defined in xnconf.h.  */
    /* If running in Bus Master Mode, allocate a dcu and use the      */
    /* data area for the ring buffer.                                 */
#if (CFG_LANCE_SHARED_MEM)
    dcu_ptr.pbyte = (PFBYTE) p_priv->ptx_ring;
    dcu_ptr.pbyte += SIZEOF_ISA_TX_DESC_ARRAY;
#else
    dcu_ptr.pbyte = dcu_alloc_core(SIZEOF_ISA_INIT_BLOCK + INIT_BLOCK_ALIGN);
    p_priv->pinit_dcu = dcu_ptr.pbyte;
#endif

    /* Align the init block address on a word boundary.   */
    while ((dword)dcu_ptr.pbyte & INIT_BLOCK_ALIGN) dcu_ptr.pbyte++;

    /* Store the address in the private structure.   */
    p_priv->pinit_block = dcu_ptr.pinit_block;

    return;
}

/*                                                                           */
/*  lance_isa_init_rx_entry                                                  */
/*                                                                           */
/*  This routine initializes all the descriptors in the Receive Ring.        */
/*                                                                           */
/*  Input:  pointer to the lance data structure                              */
/*          index of this entry into the DCU table (rx_dcus) and the rx ring */
/*                                                                           */
/*  Indirect Output:  initialized descriptors                                */
/*                                                                           */
/*  Calls:  os_alloc_packet_input                                            */
/*                                                                           */
/*  Called by: lance_isa_open                                                */
/*                                                                           */
RTIP_BOOLEAN lance_isa_init_rx_entry(PLANCE_ISA_PRIV p_priv, int i)
{
    PFBYTE  addr;
    LANCE_ISA_RX_DESC temp_rxdesc;

#if (CFG_LANCE_SHARED_MEM)
    /* If shared memory, point to an area beyond the init block   */
    /* to serve as the DCU.                                       */
    addr = (PFBYTE) p_priv->pinit_block;
    addr += SIZEOF_ISA_INIT_BLOCK+(PKT_BUF_SZ*i);
    p_priv->rx_dcus[i] = addr;
#else
    DCU     msg;
    /* Get a dcu.   */
    if (p_priv->rx_dcus[i] == 0)
        msg = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC);
    else
        msg = p_priv->rx_dcus[i];
    if (!msg)
    {
        DEBUG_ERROR("lance_isa_init_rx_entry: out of DCUs", NOVAR, 0, 0);
        return (FALSE);
    }
    /* Store the address of the DCU in the lance structure so that   */
    /* it can be freed later.                                        */
    p_priv->rx_dcus[i] = msg;
    /* Get the address of the data portion of the DCU to place   */
    /* in the receive descriptor.                                */
    addr = (PFBYTE) DCUTODATA(msg);
#endif
    /* Set rmd0 and the low 8 bits of rmd1 to the   */
    /* address of the DCU data.                     */
    temp_rxdesc.rmd0 = LANCE_ISA_LOW_16(addr);
    temp_rxdesc.rmd1 = LANCE_ISA_HIGH_8(addr);

    /* Set the own bit in rmd1 to LANCE owned   */
    temp_rxdesc.rmd1 |= LANCE_ISA_M_LANCE_OWNED;

    /* Set the buffer byte count in rmd2 (2's complement).   */
    temp_rxdesc.rmd2 = LANCE_ISA_TWOS_COMP(ETHERSIZE+4);

    /* Set rmd3 to zeros as the message count is written by the    */
    /* lance and cleared by the CPU after the message is received. */
    temp_rxdesc.rmd3 = 0;

    /* Swap the bytes if the host is Big Endian.  LANCE operates Little Endian.   */
    p_priv->prx_ring[i].rmd2 = SWAPIF(temp_rxdesc.rmd2);
    p_priv->prx_ring[i].rmd3 = SWAPIF(temp_rxdesc.rmd3);
    p_priv->prx_ring[i].rmd0 = SWAPIF(temp_rxdesc.rmd0);
    p_priv->prx_ring[i].rmd1 = SWAPIF(temp_rxdesc.rmd1);

    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_init_tx_entry                                                */
/*                                                                         */
/*  This routine initializes all the descriptors in the Transmit Ring.     */
/*                                                                         */
/*  Input:  pointer to the lance data structure                            */
/*          index of this entry in the tx ring                             */
/*                                                                         */
/*  Indirect Output:  initialized descriptors                              */
/*                                                                         */
/*  Calls:  os_alloc_packet_input                                          */
/*                                                                         */
/*  Called by: lance_isa_open                                              */
/*                                                                         */
RTIP_BOOLEAN lance_isa_init_tx_entry(PLANCE_ISA_PRIV p_priv, int i)
{
    LANCE_ISA_TX_DESC   temp_txdesc;

#if (CFG_LANCE_SHARED_MEM)
    PFBYTE addr;

    /* If shared memory, point to an area beyond the init block   */
    /* and the rx buffers to serve as the tx buffers.             */
    /* ?? Is there a maximum size for shared memory??             */
    /* ?? If so, I should be taking this into account so          */
    /* ?? I'm not stepping on anybody.                            */
    addr = (PFBYTE) p_priv->pinit_block;

    addr += (SIZEOF_ISA_INIT_BLOCK+(PKT_BUF_SZ*NUM_ISA_RX_DESC)+(PKT_BUF_SZ*i));

    p_priv->tx_laddr[i] = addr; /* Host logical address of xmit buffers */

    /* Set tmd0 and the low 8 bits of tmd1 to the   */
    /* address of the DCU data.                     */
    temp_txdesc.tmd0 = LANCE_ISA_LOW_16(addr);
    temp_txdesc.tmd1 = LANCE_ISA_HIGH_8(addr);
#else
    /* If not shared memory mode, just zero the addresses   */
    /* as they will be filled in as needed.                 */
    temp_txdesc.tmd0 = 0x0000;
    temp_txdesc.tmd1 = 0x0000;
#endif

    /* Set the own bit in tmd1 to CPU owned => clear the bit   */
    temp_txdesc.tmd1 &= LANCE_ISA_M_CPU_OWNED;

    /* Swap the bytes if the host is Big Endian.  LANCE operates Little Endian.   */
    /* tmd2 and tmd3 are written in lance_isa_tx_packet.                          */
    p_priv->ptx_ring[i].tmd0 = SWAPIF(temp_txdesc.tmd0);
    p_priv->ptx_ring[i].tmd1 = SWAPIF(temp_txdesc.tmd1);

    return(TRUE);
}

/* *********************************************************************            */
/*                                                                                  */
/*  lance_isa_build_init_block                                                      */
/*                                                                                  */
/*  This routine configures the 12 words in the initialization block, the address   */
/*  of which is written into CSR1 & CSR2, when the LANCE device is initialized.     */
/*  The init block consists of:                                                     */
/*          1.  bits to set in Mode Register (if any) - default: 0's.               */
/*          2.  Physical Address of device                                          */
/*      3.  Logical Address filter (for multicast)                                  */
/*          3.  Length & location of RX ring                                        */
/*          4.  Length & location of TX ring                                        */
/*                                                                                  */
/*  Input:  Pointer to lance data structure (which holds the ptr to the init block) */
/*                                                                                  */
/*  Output: Configured Initialization Block.                                        */
/*                                                                                  */
/*  Calls: none                                                                     */
/*                                                                                  */
/*  Called by: lance_isa_open                                                       */
/*                                                                                  */
void lance_isa_build_init_block(PLANCE_ISA_PRIV p_priv)
{
    int i;
    LANCE_ISA_T_RDR temp_rdr;
    LANCE_ISA_T_TDR temp_tdr;


    /* Write the mode.  Get the settings from the data structure.   */
    p_priv->pinit_block->mode = SWAPIF(p_priv->mode_settings);


    /* Write the physical address of the device.                      */
    /* There is a 16 byte station address PROM at the base address.   */
    /* The first 6 bytes are the station address.  Write these to the */
    /* physical address locations in the init block, and into the     */
    /* iface structure for reference by other RTIP routines.          */
#if (SET_MAC_ADDR)
    for (i=0; i < 6; i++)
    {
        p_priv->pinit_block->padr[i]
            = p_priv->iface->addr.my_hw_addr[i]
            = mac_isa_addr[i];
    }
#else

    for (i=0; i < 6; i++)
    {
        p_priv->pinit_block->padr[i] =
        p_priv->iface->addr.my_hw_addr[i] = INBYTE((p_priv->base_addr) + i);
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_bld_init_blk: p_priv->pinit_block->padr[i]=;", EBS_INT2, i,
            p_priv->pinit_block->padr[i]);
#endif

    }
#endif

    /* Set the logical address filter   */
    for (i=0; i < LANCE_ISA_LADR_WORD_CNT; i++)
    {
        p_priv->pinit_block->ladr[i] = SWAPIF(p_priv->ladr[i]);
    }

    /* Point to the rx ring and swap the bytes if Big Endian processor   */
    temp_rdr.rdr1 = LANCE_ISA_LOW_16((PFBYTE)p_priv->prx_ring);
    temp_rdr.rdr2 = LANCE_ISA_HIGH_8((PFBYTE)p_priv->prx_ring);
    temp_rdr.rdr2 |= LANCE_ISA_M_RDR2_RLEN;
    p_priv->pinit_block->rdr1 = SWAPIF(temp_rdr.rdr1);
    p_priv->pinit_block->rdr2 = SWAPIF(temp_rdr.rdr2);

    /* Point to the tx ring and swap the bytes if Big Endian processor   */
    temp_tdr.tdr1 = LANCE_ISA_LOW_16((PFBYTE)p_priv->ptx_ring);
    temp_tdr.tdr2 = LANCE_ISA_HIGH_8((PFBYTE)p_priv->ptx_ring);
    temp_tdr.tdr2 |= LANCE_ISA_M_TDR2_TLEN;
    p_priv->pinit_block->tdr1 = SWAPIF(temp_tdr.tdr1);
    p_priv->pinit_block->tdr2 = SWAPIF(temp_tdr.tdr2);


    return;
}

/* *********************************************************************         */
/*                                                                               */
/*  lance_isa_init                                                               */
/*                                                                               */
/*  This routine initializes the LANCE.  This routine ASSUMES the initialization */
/*  block has been configured prior to its being called, and that lance_isa_stop */
/*  has been called to stop the LANCE so that CSR1, CSR2, and CSR3 can be        */
/*  written.                                                                     */
/*                                                                               */
/*  Input:  Address of Initialization Block                                      */
/*          something to indicate if the driver is to operate in interrupt or    */
/*            polling mode                                                       */
/*                                                                               */
/*  Output: Do I want to return TRUE/FALSE?                                      */
/*                                                                               */
/*  Called by: lance_isa_open                                                    */
/*                                                                               */
RTIP_BOOLEAN lance_isa_init(PLANCE_ISA_PRIV p_priv)
{
    word    csr0, csr1, csr2, bdp;
    int     i=0;

    /* Reset controller and wait 3 seconds for synchronization   */
    bdp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RESET);
    ks_sleep((word)(1+(ks_ticks_p_sec()*3)));

    /* Load CSR1, CSR2 with address of Initialization Block   */
    csr1 = LANCE_ISA_LOW_16((PFBYTE)p_priv->pinit_block);
    csr2 = LANCE_ISA_HIGH_8((PFBYTE)p_priv->pinit_block);

    /* Write CSR1, CSR2 - Write_RAP (CSR1), Write_RDP (CSR1)   */
    /*                  - Write_RAP(CSR2), Write_RDP(CSR2)     */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR1));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(csr1));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR2));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(csr2));

    /* Load CSR3 (where do values come from?) I'm letting this default for now   */
    /* (set to 0's when stop bit is written).                                    */
    /* Write CSR3 - Write_RAP(CSR3), Write_RDP(CSR3)                             */

    /* Load CSR0 with INIT bit set.                  */
    /* Write CSR0 - Write_RAP(CSR0), Write_RDP(CSR0) */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INIT));



    /* I am waiting on the initialization to complete, NOT for an   */
    /* interrupt signalling 'idon'.                                 */
    while (i++ < 100)
    {
        csr0 = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
        if (csr0 & LANCE_ISA_M_CSR0_IDON)
            break;
    }

#if (RTIP_VERSION > 24)
    ks_hook_interrupt(p_priv->irq, p_priv->iface,
                      (RTIPINTFN_POINTER)lance_isa_isr,
                      (RTIPINTFN_POINTER) lance_isa_pre_isr,
                      p_priv->iface->minor_number);
#else
    ks_hook_interrupt(p_priv->irq,
                      (RTIPINTFN_POINTER)lance_isa_isr,
                      p_priv->iface->minor_number);
#endif

    /* set no underflow on transmit bit   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_BDP);
        bdp |= 0x800;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));

#if(PROGRAM_LEDS)
    /* set enable led program bit   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR2));
    bdp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_BDP);
        bdp |= 0x1000;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));

      /* program LED 0 to indicate ethernet activity   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR4));
        bdp = 0x00B0;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));

      /* program LED 1 to indicate full duplex status or collision   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR5));
        bdp = 0x0181;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));

      /* program LED 2 to indicate link status   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR6));
        bdp = 0x00C0;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));

      /* program LED 3 to indicate 100 mhz operation   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR7));
        bdp = 0x1080;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));
    /* reset enable led program bit   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR2));
    bdp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_BDP);
        bdp &= 0x7FFF;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));


#endif

    /* STRT the device.   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                      LANCE_ISA_M_CSR0_STRT));
    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_stop                                                         */
/*                                                                         */
/*  This routine stops the LANCE by writing the stop bit in CSR0.          */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Called by: lance_isa_open, lance_isa_reinit, lance_isa_close,          */
/*             lance_isa_change_speed                                      */
/*                                                                         */
/*                                                                         */
void lance_isa_stop(PLANCE_ISA_PRIV p_priv)
{
    word    csrtmp;

#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_stop", NOVAR, 0, 0);
#endif

    /* writing a 0 to the address register first to insure CSR0 is accessed.   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    csrtmp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
    csrtmp |=LANCE_ISA_M_CSR0_STOP;
    /* write CSR0 with STOP bit set   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(csrtmp));

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_spnd                                                         */
/*                                                                         */
/*  This routine puts the LANCE (Am79C972) into SUSPEND mode by writing    */
/*  the spnd bit in CSR5.                                                  */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Called by: lance_isa_set_loopback                                      */
/*                                                                         */
/*                                                                         */
void lance_isa_spnd(PLANCE_ISA_PRIV p_priv)
{
    int i;
    word csr5;

#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_spnd", NOVAR, 0, 0);
#endif

    /* For Am79C972 only.  Processes all current rx's and tx's, then waits.   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR5));
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR5_SPND));
    for (i=0; i<1000; i++)
    {
        csr5 = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
        if (csr5 & LANCE_ISA_M_CSR5_SPND)
            break;
    }

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_free_dcus                                                    */
/*                                                                         */
/*  This routine frees all the DCUs that were allocated for buffer         */
/*  management and for the initialization block.                           */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Calls: dcu_free_core, os_free_packet                                   */
/*                                                                         */
/*  Called by: lance_isa_close                                             */
/*                                                                         */
void lance_isa_free_dcus(PLANCE_ISA_PRIV p_priv)
{
    int i;

#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_free_dcus: entering", NOVAR, 0, 0);
#endif

    /* Free all allocated receive buffers, and zero the table.   */
    for (i=0; i < NUM_ISA_RX_DESC; i++)
    {
        if (p_priv->rx_dcus[i] != 0)
            os_free_packet(p_priv->rx_dcus[i]);
        p_priv->rx_dcus[i] = 0;
    }

    /* Free the memory used to store the initialization block,   */
    /* and the rx and tx rings, and zero the pointers in the     */
    /* data structure.                                           */
    dcu_free_core((PFBYTE)p_priv->pinit_dcu);
    dcu_free_core((PFBYTE)p_priv->prx_dcu);
    dcu_free_core((PFBYTE)p_priv->ptx_dcu);
    p_priv->pinit_dcu = 0;
    p_priv->prx_dcu = 0;
    p_priv->ptx_dcu = 0;

    return;
}

/* *********************************************************************      */
/*                                                                            */
/*  lance_isa_restart                                                         */
/*                                                                            */
/*  This routine causes the LANCE to restart.  The init block is not reloaded */
/*  and no change is made in the ring buffers.                                */
/*  It is assumed that the init block has been loaded and that lance_isa_spnd */
/*  was called prior to calling this routine.                                 */
/*                                                                            */
/*  Input: pointer to LANCE data structure                                    */
/*                                                                            */
/*  Output: restarted LANCE device (indirect output)                          */
/*                                                                            */
/*  Called By: lance_isa_set_loopback                                         */
/*                                                                            */
void lance_isa_restart(PLANCE_ISA_PRIV p_priv)
{
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_restart: entering", NOVAR, 0, 0);
#endif

    /* Set STRT bit and INEA bit in CSR0   */
    OUTWORD(p_priv->base_addr + LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    OUTWORD(p_priv->base_addr + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                                      LANCE_ISA_M_CSR0_STRT));
    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_reinit                                                       */
/*                                                                         */
/*  This routine causes the LANCE to re-initialize (and restart).          */
/*                                                                         */
/*  Input: pointer to LANCE data structure                                 */
/*                                                                         */
/*  Output: reinitialized LANCE device (indirect output)                   */
/*                                                                         */
/*  NOTE: This is different from regular initialization in that            */
/*          the rx and tx rings are reordered rather than re-              */
/*          initialized.                                                   */
/*                                                                         */
/*                                                                         */
void lance_isa_reinit(PLANCE_ISA_PRIV p_priv)
{
    word    bdp,csr0;
int i=0;
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_reinit: entering", NOVAR, 0, 0);
#endif

    /* Write STOP bit in CSR0 (LANCE must be STOPPED in order to write CSR1&CSR2.   */
    lance_isa_stop(p_priv);

    /* Call lance_isa_reorder_buffers to rearrange descriptors in tx or rx ring.   */
    lance_isa_reorder_buffers(p_priv);

    /* Set INIT bit in CSR0 and wait for reset to complete   */
    OUTWORD(p_priv->base_addr + LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    OUTWORD(p_priv->base_addr + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INIT));
    while (i++ < 100)
    {
        csr0 = XINWORD((p_priv->base_addr)+LANCE_ISA_K_RDP);
        if (csr0 & LANCE_ISA_M_CSR0_IDON)
            break;
    }

    /* set no underflow on transmit bit   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+LANCE_ISA_K_BDP);
        bdp |= 0x800;
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_BDP, SWAPIF(bdp));

    /* start the device   */
    OUTWORD((p_priv->base_addr)+LANCE_ISA_K_RAP, SWAPIF(LANCE_ISA_K_CSR0));
    OUTWORD(p_priv->base_addr + LANCE_ISA_K_RDP, SWAPIF(LANCE_ISA_M_CSR0_INEA |
                                      LANCE_ISA_M_CSR0_STRT ));
    return;
}

/* *********************************************************************     */
/*                                                                           */
/*  lance_isa_reorder_buffers                                                */
/*                                                                           */
/*  This routine is called to service an error condition that requires the   */
/*  descriptor ring to be reordered (as the LANCE device resets its pointers */
/*  to the start of the descriptor ring on error).                           */
/*                                                                           */
/*  Input:  Address of descriptor ring                                       */
/*                                                                           */
/*  Output:  Re-ordered descriptor ring                                      */
/*                                                                           */
/*                                                                           */
void lance_isa_reorder_buffers(PLANCE_ISA_PRIV p_priv)
{
/*  word temp_ptr;   */
    int i;

#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_reorder buffers: entering", NOVAR, 0, 0);
    DEBUG_ERROR("lance_isa_reorder buffers: cur_tx, cur_rx", EBS_INT2, p_priv->cur_tx, p_priv->cur_rx);

#endif

    /* for the sake of moving on with debug, use the old stuff for now. 3/15 VK   */
    for (i=0; i<NUM_ISA_RX_DESC; i++)
    {
/*          //free the packet, then reload the ring entry   */
/*          if (p_priv->rx_dcus[i])                         */
/*              os_free_packet(p_priv->rx_dcus[i]);         */
/*          p_priv->rx_dcus[i] = 0;                         */
        lance_isa_init_rx_entry(p_priv, i);
    }

/*      // Move the current rx descriptor + 1 and all following descriptors,      */
/*      // in order, to the 0 position of the rx ring.                            */
/*      temp_ptr = p_priv->cur_rx;                                                */
/*      LANCE_ISA_INC_RX_PTR(temp_ptr);                                           */
/*                                                                                */
/*      if (temp_ptr == 0)                                                        */
/*          ; // the next descriptor is at the 0 point, so everything's in place. */
/*      else                                                                      */
/*      {                                                                         */
/*          for (i=0; i<NUM_ISA_RX_DESC; i++)                                     */
/*          {                                                                     */
/*      }                                                                         */
/*      // Set the rx ring pointers to point to the 'top' of the rx ring          */
    p_priv->cur_rx = 0;

    /* for the sake of debug, do what lance.c did. 3/15 VK   */
    for (i=0; i<NUM_ISA_TX_DESC; i++)
        lance_isa_init_tx_entry(p_priv, i);

/*      // Re-order the tx pointers to point to the next buffer to transmit   */
/*      // Set the tx ring pointers to point to the 'top' of the tx ring      */
    p_priv->cur_tx = p_priv->dirty_tx = 0;

    return;
}

/*                                                                   */
/*  lance_isa_errors                                                 */
/*                                                                   */
/*  This routine handles errors reported by the LANCE via interrupt. */
/*                                                                   */
/*  Input:  Copy of CSR0 read by lance_isa_isr                       */
/*                                                                   */
/*  Output: none                                                     */
/*                                                                   */
/*  Calls:                                                           */
/*                                                                   */
/*  Called by: lance_isa_isr (when ERR set in CSR0)                  */
/*                                                                   */
void lance_isa_errors(dword csr0, PLANCE_ISA_PRIV p_priv)
{
#if (DEBUG_LANCE_ISA)
/*  DEBUG_ERROR_INT("lance_isa_errors: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("lance_isa_errors: entering", NOVAR, 0, 0);
#endif

    /* Get error bits set in CSR0 (BABL and/or MISS) and   */
    /* update statistics.                                  */
    if (csr0 & LANCE_ISA_M_CSR0_BABL)
        p_priv->stats.errors_out++;         /* TX babble */
    if (csr0 & LANCE_ISA_M_CSR0_MISS)
        p_priv->stats.errors_in++;          /* missed RX packet */

    /* if it's a memory error, stop and restart the LANCE.   */
    /* If MERR                                               */
    /*  {                                                    */
    /*      STOP the LANCE (STP in CSR0);                    */
    /*      Rearrange rx and tx descriptors;                 */
    /*      Initialize the LANCE;                            */
    /*  }                                                    */
    if (csr0 & LANCE_ISA_M_CSR0_MERR)
        lance_isa_reinit(p_priv);
    return;
}

/* *********************************************************************       */
/*                                                                             */
/*  lance_isa_rx_interrupt                                                     */
/*                                                                             */
/*  This routine processes a packet placed in the receive buffer by the LANCE  */
/*  device.                                                                    */
/*  This routine in called when the RINT bit in CSR0 is set.                   */
/*                                                                             */
/*  Note:  On page 25 in Spec it states:                                       */
/*         On packet receipt, look at Indiv/Group bit of destination address.  */
/*         If multicast, search_mcast_addr_list to see if in list (then what?- */
/*                  I assume you would process the packet like a regular rx)   */
/*         else                                                                */
/*            discard packet                                                   */
/*                                                                             */
/*  Input:  Pointer to the lance private data structure                        */
/*                                                                             */
/*                                                                             */
/*  Output: Received data queued to RTIP                                       */
/*                                                                             */
/*  Called by: lance_isa_isr (when RINT in CSR0 is set)                        */
/*                                                                             */
void lance_isa_rx_interrupt(PLANCE_ISA_PRIV p_priv)
{
    RTIP_BOOLEAN error = FALSE;
    word pkt_len;
    RTWORD_ISA temp_rmd3;
    DCU msg = 0;
    DCU new_msg;

#if (DEBUG_LANCE_ISA)
/*  DEBUG_ERROR_INT("lance_isa_rx_interrupt: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("lance_isa_rx_interrupt: entering", NOVAR, 0, 0);
#endif

    /* Step through the rx_ring until the next entry is not        */
    /* owned by the CPU.                                           */
    /* If an initial RX interrupt occurred, there is one packet,   */
    /* at least, that is CPU owned.  Process it, then check if the */
    /* next packet is owned by the CPU.                            */
    do
    {
        /* Read RMD1, checking for an error condition.   */
        /* If an error occurred, increment counters.     */
        if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_ERR))
        {
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_rx_int: RMD1_ERR - p_priv->cur_rx=", EBS_INT1, p_priv->cur_rx, 0);
#endif
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_FRAM))
                p_priv->stats.rx_frame_errors++;
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_OFLO))
                p_priv->stats.rx_overwrite_errors++;
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_CRC))
                p_priv->stats.rx_crc_errors++;
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_BUFF))
                p_priv->stats.rx_fifo_errors++;
            /* Set the own bit in the current rx ring entry to LANCE OWNED   */
            p_priv->prx_ring[p_priv->cur_rx].rmd1 |= SWAPIF(LANCE_ISA_M_LANCE_OWNED);
            /* Point to the next entry in the rx ring.   */
            LANCE_ISA_INC_RX_PTR(p_priv->cur_rx)
        }
        /* Read RMD1 checking for STP and ENP both set.                    */
        /* We allocate maximum size entries for the rx_ring, so STP        */
        /* and ENP should both be set, otherwise, it's an error.  We       */
        /* don't expect any buffer chaining to occur.                      */
        /* Note:  this should cover the jabber packet error condition      */
        /*        as documented in the old lance driver.                   */
        /*        Also, it is assumed that INCLUDE_LANCE is set, therefore */
        /*        ETHERSIZE+4 is 1518 (in xnconf.h).                       */
        else if (!((p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_STP)) &&
                   (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_ENP))))
        {
            /* Go through the ring giving the buffer back to the lance   */
            /* until ENP is set.                                         */
            do
            {
#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_rx_int: STP && ENP - p_priv->cur_rx=", EBS_INT1, p_priv->cur_rx, 0);
#endif
                p_priv->prx_ring[p_priv->cur_rx].rmd1 |=SWAPIF(LANCE_ISA_M_LANCE_OWNED);
                LANCE_ISA_INC_RX_PTR(p_priv->cur_rx)
            } while (!(p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_RMD1_ENP)));
            p_priv->stats.rx_other_errors++;
        }
        /* There are no errors (all handled above),   */
        /* so "send the packet up"                    */
        else
        {
            temp_rmd3 = (RTWORD_ISA)(p_priv->prx_ring[p_priv->cur_rx].rmd3);
            pkt_len = (word)((SWAPIF(temp_rmd3) & LANCE_ISA_M_RMD3_MCNT)-4);
#if (CFG_LANCE_SHARED_MEM)          /* Shared Memory Mode */
            msg = os_alloc_packet_input(pkt_len, DRIVER_ALLOC);
            if (msg)
                tc_movebytes(DCUTODATA(msg), p_priv->rx_dcus[p_priv->cur_rx], pkt_len);
            else
                error = TRUE;
#else                               /* Bus Master Mode */
            new_msg = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC);
            if (!new_msg)
                error = TRUE;
            else
            {
                msg = p_priv->rx_dcus[p_priv->cur_rx];
                p_priv->rx_dcus[p_priv->cur_rx] = new_msg;
            }
#endif

            if (error)
            {
                DEBUG_ERROR("lance_isa_rx_int: out of DCUs", NOVAR, 0, 0);
                p_priv->stats.packets_lost++;
                /* Set the own bit in the current rx ring entry to LANCE OWNED   */
                p_priv->prx_ring[p_priv->cur_rx].rmd1 |= SWAPIF(LANCE_ISA_M_LANCE_OWNED);
            }
            else
            {

#if (DEBUG_LANCE_ISA)
    DEBUG_ERROR("lance_isa_rx_int: p_priv->cur_rx=; length=;", EBS_INT2,
    p_priv->cur_rx, pkt_len);
/*  DEBUG_ERROR("lance_isa_rx_int: pkt = ", PKT, DCUTODATA(msg), 25);   */
#endif
                DCUTOPACKET(msg)->length = pkt_len;
                p_priv->stats.packets_in++;
                p_priv->stats.bytes_in += pkt_len;
                /* Send to input list.  Will get sent to IP exchange.   */
#if (RTIP_VERSION > 24)
                ks_invoke_input(p_priv->iface,msg);
#else
                os_sndx_input_list(p_priv->iface, msg);
                ks_invoke_input(p_priv->iface);
#endif
                /*allocate the new DCU to this rx ring entry   */
                lance_isa_init_rx_entry(p_priv, p_priv->cur_rx);
            }
            /* Point to the next entry in the rx ring.   */
            LANCE_ISA_INC_RX_PTR(p_priv->cur_rx)
        }

    } while (!(p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(LANCE_ISA_M_LANCE_OWNED)));

    return;
}

/*                                                              */
/*  lance_isa_tx_interrupt                                      */
/*                                                              */
/*  This routine processes a packet successfully transmitted by */
/*  the LANCE.                                                  */
/*                                                              */
/*  Input:   Pointer to the lance private data structure        */
/*                                                              */
/*  Output:  none                                               */
/*                                                              */
/*  Calls:                                                      */
/*                                                              */
/*  Called by: lance_isa_isr (when TINT is set in CSR0)         */
/*                                                              */
void lance_isa_tx_interrupt(PLANCE_ISA_PRIV p_priv)
{
    int test_index;

#if (DEBUG_LANCE_ISA)
/*  DEBUG_ERROR_INT("lance_isa_tx_interrupt: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("lance_isa_tx_interrupt: entering", NOVAR, 0, 0);
#endif

    test_index = p_priv->dirty_tx;


    while(test_index != p_priv->cur_tx)
    {
        /* If ERR bit set in TMD1                                                 */
        /* {                                                                      */
        /*      Record statistics if they are being kept;                         */
        /*      If BUFF or UFLO error (transmitter has been turned off and        */
        /*              must be turned back on)                                   */
        /*      {                                                                 */
        /*          Stop the LANCE;                                               */
        /*          Rearrange the descriptors in the ring (so that valid ones are */
        /*              at the top of the ring);                                  */
        /*          Initialize the LANCE;                                         */
        /*      }                                                                 */
        if ((p_priv->ptx_ring[test_index].tmd1 &
             SWAPIF(LANCE_ISA_M_TMD1_ERR)))

        {
            p_priv->stats.errors_out++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(LANCE_ISA_M_TMD3_RTRY))
                p_priv->stats.collision_errors++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(LANCE_ISA_M_TMD3_LCAR))
                p_priv->stats.tx_carrier_errors++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(LANCE_ISA_M_TMD3_LCOL))
                p_priv->stats.owc_collision++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(LANCE_ISA_M_TMD3_UFLO))
            {
                p_priv->stats.tx_fifo_errors++;
                /* underflow error indicates that the tx fifo   */
                /* was emptied before ENP was reached.          */
                /* transmitter is turned off, so re-init to get */
                /* things going again.                          */
                lance_isa_reinit(p_priv);
            }
        }
        else
        {
            if (p_priv->ptx_ring[test_index].tmd1 &
                  (SWAPIF(LANCE_ISA_M_TMD1_MORE | LANCE_ISA_M_TMD1_ONE)))
                p_priv->stats.one_collision++;
            p_priv->stats.packets_out++;
        }
        LANCE_ISA_INC_TX_PTR(test_index)
    }
    /*  Notify the calling process that a transmit interrupt has occurred.   */
    ks_invoke_output(p_priv->iface, 1);

    p_priv->dirty_tx = test_index;
    return;
}



/* *********************************************************************   */
/*                                                                         */
/*  Memory Management Support Routines.                                    */
/*                                                                         */
/*                                                                         */
/*  off_isa_to_priv                                                        */
/*                                                                         */
PLANCE_ISA_PRIV off_isa_to_priv(int min_dev_num)
{
    if (min_dev_num >= CFG_NUM_LANCE_ISA)
    {
        DEBUG_ERROR("off_isa_to_priv(): pi->minor_number, CFG_NUM_LANCE_ISA = ",
            EBS_INT2, min_dev_num, CFG_NUM_LANCE_ISA);
        return ((PLANCE_ISA_PRIV) 0);
    }
    return ((PLANCE_ISA_PRIV) lance_isa_s_priv_table[min_dev_num]);
}

/*                    */
/*  iface_isa_to_priv */
/*                    */
PLANCE_ISA_PRIV iface_isa_to_priv(PIFACE pi)
{
    if (pi->minor_number >= CFG_NUM_LANCE_ISA)
    {
        DEBUG_ERROR("iface_isa_to_priv() - pi->minor_number, CFG_NUM_LANCE_ISA = ",
            EBS_INT2, pi->minor_number, CFG_NUM_LANCE_ISA);
        return ((PLANCE_ISA_PRIV) 0);
    }
    return ((PLANCE_ISA_PRIV) lance_isa_s_priv_table[pi->minor_number]);
}

/* *********************************************************************   */
/*                                                                         */
/*  lance_isa_load_lance                                                   */
/*                                                                         */
/*  Write a specified CSR with the given value.                            */
/*  This routine was taken from lance.c and modified to be general just in */
/*  case a customer somewhere needs it.                                    */
/*  NOTE: untested at this point.                                          */
/*                                                                         */
/*  INPUT:  base address of device                                         */
/*          number of CSR to load                                          */
/*          value to write to CSR                                          */
/*                                                                         */
/*  OUTPUT: none                                                           */
/*                                                                         */
void lance_isa_load_lance(int ioaddr, word reg_num, word value)
{
    OUTWORD(ioaddr+LANCE_ISA_K_RAP, SWAPIF(reg_num));
    OUTWORD(ioaddr+LANCE_ISA_K_RDP, SWAPIF(value));
    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  dump_lance                                                             */
/*                                                                         */
/*  This routine dumps a range of registers.                               */
/*  This routine was taken from lance.c and modified to be general just in */
/*  case a customer somewhere needs it.                                    */
/*  NOTE: untested at this point.                                          */
/*                                                                         */
/*  INPUT:  base address of device                                         */
/*          first CSR to read and display                                  */
/*          last CSR to read and display                                   */
/*                                                                         */

void dump_lance_isa(word ioaddr, word start_reg, word end_reg)
{
word w, i;
word ww,ii;
    for (i=start_reg; i<=end_reg; i++)
    {
        OUTWORD(ioaddr+LANCE_ISA_K_RAP, SWAPIF(i));
        w = XINWORD(ioaddr+LANCE_ISA_K_RDP);
        ww=w;
        ii=i;

        DEBUG_ERROR("REG_NUM == , REG_VALUE == ", EBS_INT2, ii, ww);
    }
    return;
}


#if (!KS_LITTLE_ENDIAN) /* 1=little endian (INTEL) */
/* *******************************************************************    */
/* Swapping routines used for endian conversions                          */
/* *******************************************************************    */
dword lance_isa_longswap(dword l)          /* change 4 bytes in long word 0,1,2,3 -> 3,2,1,0 */
{
    /* return((byte 3 to 0) OR (byte 0 to 3) OR (byte 2 to 1) OR   */
    /*        (byte 1 to 2)                                        */
        l = (l >> 24) | (l << 24) | ((l & 0x00ff0000ul) >> 8) |
                                    ((l & 0x0000ff00ul) << 8);
    return(l);
}

/* *********************************************************************   */
word lance_isa_wordswap(word w)          /* change 2 bytes in word 0,1 -> 1,0 */
{
        w = (word)((w>>8) | (w<<8));
    return(w);
}
#endif /* (!KS_LITTLE_ENDIAN) - 1=little endian (INTEL) */

#define sync()

/* lance register dump routines   */
/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_lance_isa(int minor_number)
{
    return(xn_device_table_add(lance_isa_device.device_id,
                        minor_number,
                        lance_isa_device.iface_type,
                        lance_isa_device.device_name,
                        SNMP_DEVICE_INFO(lance_isa_device.media_mib,
                                         lance_isa_device.speed)
                        (DEV_OPEN)lance_isa_device.open,
                        (DEV_CLOSE)lance_isa_device.close,
                        (DEV_XMIT)lance_isa_device.xmit,
                        (DEV_XMIT_DONE)lance_isa_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)lance_isa_device.proc_interrupts,
                        (DEV_STATS)lance_isa_device.statistics,
                        (DEV_SETMCAST)lance_isa_device.setmcast));
}

#endif /* !DECLARING_DATA */
#endif /* INCLUDE_LANCE_ISA) */

