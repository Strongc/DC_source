/*                                                                                                   */
/* rtlance.c                                                                                         */
/*                                                                                                   */
/*   EBS - RTIP                                                                                      */
/*                                                                                                   */
/*   Copyright EBSnet, Inc., 1999                                                                    */
/*   All rights reserved.                                                                            */
/*   This code may not be redistributed in source or linkable object form                            */
/*   without the consent of its author.                                                              */
/*                                                                                                   */
/*                                                                                                   */
/*   Module description:                                                                             */
/*      This device driver controls the Am79C90, Am79C961(PCnet-ISA+),                               */
/*      Am79C972(PCnet-Fast+ - PCI-based, 100BaseT),                                                 */
/*      Am79C973/5(PCnet-Fast III - PCI-based, 100BaseT, integrated PHY),                            */
/*          and the TARGET_186ES (an AMD evaluation card with an on-board 186 CPU and                */
/*      an ethernet controller that is like the Am79C961 with a little different                     */
/*      DMA controller).                                                                             */
/*                                                                                                   */
/*   Data Structure Definition:                                                                      */
/*      If the driver is run in shared memory mode, the memory is                                    */
/*      divided up as follows:                                                                       */
/*          array of rx ring descriptors                                                             */
/*          array of tx ring descriptors                                                             */
/*          initialization block                                                                     */
/*          rx packets pointed to by ring descriptors                                                */
/*          tx packets pointed to by ring descriptors                                                */
/*      If the driver is run in bus master mode (the default), a separate                            */
/*          DCU is allocated to hold the rx ring descriptors, the tx ring descriptors,               */
/*          and the init block.  In addition, DCUs are also allocated to                             */
/*          hold the rx packets (DCUs for the tx packets are allocated                               */
/*          dynamically).                                                                            */
/*      The private lance data structure RTLANCE_T_PRIV holds pointers to                            */
/*          the rx ring, tx ring, init block, wherever they are located                              */
/*          (shared memory or in DCUs).  RTLANCE_T_PRIV also holds the addresses                     */
/*          of the DCUs allocated for rx packets in Bus Master Mode (so they                         */
/*          may be freed later), and the addresses of the rx packets in                              */
/*          shared memory.                                                                           */
/*                                                                                                   */
/*  NOTE: This source code is arranged with routines entered through the device table                */
/*        first, in device table order.  Support routines follow after all of the                    */
/*        device table entry routines.                                                               */
/*                                                                                                   */
/*   Revision history:                                                                               */
/*      March 1999  V. Kell  Initial coding                                                          */
/*      June  1999  VK       Re-do byte swapping code for transmits, init block and                  */
/*                           receive packet length (rmd3)                                            */
/*                           NOTE: The combination of PowerPC and CFG_LANCE_SHARED_MEM               */
/*                                 is not necessarily supported.  Code must be revisited             */
/*                                 if support of this combination is ever required.                  */
/*      June  1999  VK       Updated reinit path                                                     */
/*      June  1999  VK       Added ability to set phy speed directly (auto-negotiation disabled)     */
/*      July  1999  VK       write to BCR32:XPHYANE to trigger hub to renegotiate connection.        */
/*                           set BCR32:XPHYRST off so reset only happens once.                       */
/*      July  1999  TVO/VK   Update interrupt service routine to match lance.c.                      */
/*      July  1999  VK       Incorporate Nortel Updates into code                                    */
/*      August 1999     TVO     32 bit capabilities                                                  */
/*      August 1999     TVO     AMD 79C973/975 support                                               */
/*      November 2000   TVO     AMD 79C970 support                                                   */
/*                                                                                                   */
#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"

#if (INCLUDE_RTLANCE)
#include "rtlance.h"


#if (CFG_AMD_PCI)
    #include "pci.h"
#endif

#if (defined (RTKBCPP)) /* for power pack we allocate this */
#if (!INCLUDE_MALLOC_DCU_INIT)
#error DPMI must use MALLOC PACKET_INIT for this card
#endif
#endif

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define DEBUG_LANCE 0
#undef SET_MAC_ADDR
#define SET_MAC_ADDR 0  /* pp */
#define PROGRAM_LEDS 1  /* define to enable LED programming */

/* ********************************************************************   */
RTIP_BOOLEAN rtlance_open(PIFACE pi);
void    rtlance_close(PIFACE pi);
int     rtlance_tx_packet(PIFACE pi, DCU msg);
RTIP_BOOLEAN rtlance_tx_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN rtlance_statistics(PIFACE  pi);
RTIP_BOOLEAN rtlance_setmcast(PIFACE pi);
#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
PRTLANCE_PRIV s_priv_table[CFG_NUM_LANCE];
word mode_table[CFG_NUM_LANCE]={0};
word rtlance_is_open=0;

#if (SET_MAC_ADDR)
    byte mac_addr[6] = {0x00,0x12,0x34,0x56,0x78,0x9a};
#endif

#if (CFG_AMD_PCI)
int KS_FAR rt_set_phy_speed = CFG_DEFAULT_PHY_SPEED;
int KS_FAR rt_actual_phy_speed = CFG_DEFAULT_PHY_SPEED;
int KS_FAR rt_loopback_type = CFG_DEFAULT_LB_MODE;
#endif

struct lance_chip_type rtlance_chip_table[11] =
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
    {0x2420, "PCnet/PCI 79C970",        /* 79C970 or 79C974 PCnet-SCSI, PCI. */
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
    /* Bug: the PCnet/PCI actually uses the PCnet/VLB ID number, so just call   */
    /*  it the PCnet32.                                                         */
    {0x2430, "PCnet32",                 /* 79C965 PCnet for VL bus. */
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
    {0x2621, "PCnet/PCI-II 79C970A",      /* 79C970 PCnet-PCI II. PCI-based, 10 base t/10 base 2 */
        (int)(LANCE_MUST_REINIT_RING + LANCE_HAS_MISSED_FRAME +
              LANCE_MUST_SPND)},
    {0x2624, "PCnet/FAST+ 79C972",      /* 79C972 PCnet-FAST+. PCI-based */
        (int)(LANCE_MUST_REINIT_RING + LANCE_HAS_MISSED_FRAME +
              LANCE_MUST_SPND)},
    {0x2625, "PCnet/FAST III 79C973",       /* 79C973 PCnet-FAST III. PCI-based */
        (int)(LANCE_MUST_REINIT_RING + LANCE_HAS_MISSED_FRAME +
              LANCE_MUST_SPND + LANCE_HAS_MII) },
    {0x2627, "PCnet/FAST III 79C975",       /* 79C975 PCnet-FAST III. PCI-based */
        (int)(LANCE_MUST_REINIT_RING + LANCE_HAS_MISSED_FRAME +
              LANCE_MUST_SPND + LANCE_HAS_MII) },
    {0x0,    "PCnet (unknown)",
        (int)(LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
              LANCE_HAS_MISSED_FRAME)},
};

EDEVTABLE KS_FAR rtlance_device =
{
    rtlance_open, rtlance_close, rtlance_tx_packet, rtlance_tx_done,
    NULLP_FUNC, rtlance_statistics, rtlance_setmcast,
    LANCE_DEVICE, "RTLANCE", MINOR_0, ETHER_IFACE,
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
extern EDEVTABLE KS_FAR rtlance_device;
extern PRTLANCE_PRIV s_priv_table[CFG_NUM_LANCE];
extern word mode_table[CFG_NUM_LANCE];
extern word rtlance_is_open;

#if (SET_MAC_ADDR)
extern byte mac_addr[6];
#endif

#if (CFG_AMD_PCI)
extern int KS_FAR rt_set_phy_speed;
extern int KS_FAR rt_actual_phy_speed;
extern int KS_FAR rt_loopback_type;
#endif

extern struct lance_chip_type rtlance_chip_table[11];
#endif  /* !BUILD_NEW_BINARY */

/* *********************************************************************        */
/*                                                                              */
/*  rtlance_open                                                                */
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
RTIP_BOOLEAN rtlance_open (PIFACE pi)
{
    PRTLANCE_PRIV p_priv;   /* ptr to lance driver private data structure */
    int i;                  /* index into rx and tx ring */
    PFBYTE  p;
    dword   l;

    /* Alloc context block   */
    if (!s_priv_table[pi->minor_number])
    {
        p = (PFBYTE) dcu_alloc_core(sizeof(*p_priv)+4);
        /* make sure on 4 byte boundary first     */
        l = (dword) p;
        while (l & 0x3ul) {l++; p++;};
        s_priv_table[pi->minor_number] = (PRTLANCE_PRIV) p;
    }

    /*                                                                             */
    /* If can't map the minor device number to an area of memory to hold the       */
    /* private data structure for the lance, return an error indicating that there */
    /* are not enough device numbers available.                                    */
    p_priv = iface_to_priv(pi);
    if (!p_priv)
    {
        set_errno(ENUMDEVICE);
        return (FALSE);
    }

    /*                                                                         */
    /* tc_memset is defined in osenv.c.  It sets a specified number of bytes   */
    /* to the value specified.  Here, memory locations beginning at p_priv and */
    /* continuing for sizeof structure RTLANCE_T_PRIV are set to 0.            */
    tc_memset((PFBYTE) p_priv, 0, sizeof(*p_priv));

    /*                                                                           */
    /* mode settings stored in the private structure are defaulted to zeroes.    */
    /* If a "user" has set mode settings for this device, copy those settings    */
    /* into the private structure so that they are used in the init block, as    */
    /* opposed to the defaults.                                                  */
    /*  Note: a user calls rtlance_set_mode to set these values.  Masks for this */
    /*        register are defined in rtlance.h.                                 */
    if (mode_table[pi->minor_number] != 0)
        p_priv->mode_settings = mode_table[pi->minor_number];

    /*                                                                             */
    /* Set up the private data structure so that it points to the global interface */
    /* structure. (address is needed later when returning packets)                 */
    p_priv->iface = pi;

    /*                                                                          */
    /* Point the statistics area of the global data structure at the statistics */
    /* area (counters) in the lance private data structure.                     */
    pi->driver_stats.ether_stats = (PETHER_STATS) &(p_priv->stats);

    /*                                                                                    */
    /* Set I/O space base address and interrupt number for this device to either          */
    /* the values set by the kernel OR to the default values in the device table.         */
    /* In the case of the PCI-based PCnet-FAST+,III,IIIA(Am79C972), the irq and base_addr */
    /* could be set based on the values read from the PCI configuration registers.        */
#if (CFG_AMD_PCI)       /* CFG_AMD_PCI set in xnconf.h=>PCnet-FAST+(Am79C972/73/75). */

    if (!rtlance_pci_init(pi, p_priv))
    {
        DEBUG_ERROR("rtlance_open: PCI register configuration failed", NOVAR, 0, 0);
        set_errno(EPROBEFAIL);      /* ??check for a PCI init failed error code?? */
        return(FALSE);
    }
    /* save in iface structure   */
    pi->io_address = p_priv->base_addr;
    pi->irq_val    = p_priv->irq;



#if (CFG_AMD_32BIT)
    /*  Set AMD controller to Double Word I/O Mode with SSIZE32 = 1   */
    rtlance_invoke_32bit(p_priv);
#endif  /* CFG_AMD_32BIT */


#else   /* !CFG_AMD_PCI */

#if (RTIP_VERSION >= 30)
                    p_priv->base_addr = pi->io_address;
                    p_priv->irq = pi->irq_val;  /* in lance.c this is bound to (word). why? */
#else
                    p_priv->base_addr = ed_io_address;
                    p_priv->irq = ed_irq_val;
#endif

#endif  /* CFG_AMD_PCI */

    /* Stop the LANCE   */
    rtlance_stop(p_priv);

    /* Get the version of the LANCE chip.  Things happen based on the   */
    /* chip type (and the flags set based on the type).                 */
    if (!rtlance_get_chip_version(p_priv))
    {
        set_errno(EPROBEFAIL);
        return(FALSE);
    }

#if (!CFG_AMD_PCI)

    /* Set up a DMA channel if it's needed   */
    rtlance_setup_dma(p_priv);
#endif

    /* Assign memory to the rx and tx descriptor rings and   */
    /* the initialization block                              */
    rtlance_assign_rx_ring(p_priv);
    rtlance_assign_tx_ring(p_priv);
    rtlance_assign_init_block(p_priv);

    /* Initialize the rx and tx descriptors in the ring arrays.    */
    /* Point the indexes into the arrays to the first (0th) entry. */
    for (i=0; i < NUM_RX_DESC; i++)
        if (!(rtlance_init_rx_entry(p_priv, i)))
            return(FALSE);
    p_priv->cur_rx = 0;

    for (i=0; i < NUM_TX_DESC; i++)
        if (!(rtlance_init_tx_entry(p_priv, i)))
            return(FALSE);
    p_priv->cur_tx = p_priv->dirty_tx = 0;

    /* Build the lance initialization block.   */
    rtlance_build_init_block(p_priv);

    /* Start (initialize) the device.   */
    if (!rtlance_init(p_priv))
        return(FALSE);
    else
        rtlance_is_open +=1;

#if (CFG_AMD_PCI)
    /*  If this is an AMD PCI chip, set the PHY speed and loopback mode.   */
    if (rtlance_chip_table[p_priv->chip_version].flags &  LANCE_HAS_MII)
    {
    rtlance_set_phy_speed(p_priv);
    if (rt_loopback_type != NO_LOOP)
        rtlance_loopback(p_priv);
    }
#endif

        return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_close                                                          */
/*                                                                         */
/*  This routine closes the LANCE device.                                  */
/*  Note:  add updates from STL/SMX lance.c close routine.  I made a few   */
/*          changes.                                                       */
/*                                                                         */
/*  Input:  pointer to the lance data structure                            */
/*                                                                         */
/*  Indirect Output: a STOPed LANCE device                                 */
/*                                                                         */
/*  Calls:  rtlance_free_dcus                                              */
/*                                                                         */
/*  Called by:  This routine is a device driver entry point.               */
/*              It is entered via a jump through the device table.         */
/*                                                                         */
void rtlance_close(PIFACE pi)
{
    PRTLANCE_PRIV p_priv;

    p_priv = iface_to_priv(pi);
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
#if (DEBUG_LANCE)
    /* Set up to read CSR0   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    DEBUG_ERROR("rtlance_close: Shutting down ethercard, status was ", EBS_INT1,
           SWAPIF(INWORD((p_priv->base_addr)+RTLANCE_K_RDP)), 0);
#endif

    /* Stop the LANCE here - otherwise it occasionally polls   */
    rtlance_stop(p_priv);

    /* Free all DCUs that were allocated for buffer management       */
    /* and the init block.  This happens only if in Bus Master Mode. */
    /* In Shared Memory Mode, the stored addresses are pointers into */
    /* the shared memory.                                            */
#if (CFG_LANCE_BUS_MASTER)
    rtlance_free_dcus(p_priv);
#endif
    rtlance_is_open -=1;
    return;
}



/* *********************************************************************   */
/*                                                                         */
/*  rtlance_tx_packet                                                      */
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
int rtlance_tx_packet(PIFACE pi, DCU msg)
{
    PRTLANCE_PRIV   p_priv;
    RTLANCE_TX_DESC temp_txdesc;
    int length;
/*  int   tx_index;     pp   */
    dword csr0;


#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_tx_packet: entering", NOVAR, 0, 0);
#endif
    p_priv = iface_to_priv(pi);
    if (!p_priv)
        return(ENUMDEVICE);


    /*  Determine how much data goes in buffer;   */
    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)
        length = ETHER_MIN_LEN;
    else if (length > ETHERSIZE)
    {
        DEBUG_ERROR("rtlance_tx_packet: length is too large", EBS_INT1,
            DCUTODATA(msg), ETHERSIZE+4);
        DEBUG_ERROR("rtlance_tx_packet: pkt = ", PKT, DCUTODATA(msg), ETHERSIZE+4);
        length = ETHERSIZE;    /* truncating the packet.  a hack from lance.c */
    }



#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_tx_packet: p_priv->cur_tx=; length=;", EBS_INT2,
    p_priv->cur_tx, length);
    DEBUG_ERROR("rtlance_tx_packet: pkt = ", PKT, DCUTODATA(msg), 25);
#endif

    /*  Check that CPU "OWNS" the current tx entry; (TMD0 & TMD1[7:0]   */
    /*  hold the buffer address).  The msb of TMD1 holds the            */
    /*  ownership bit.                                                  */
    if (p_priv->ptx_ring[p_priv->cur_tx].tmd1 & SWAPIF(RTLANCE_M_LANCE_OWNED))
    {
#if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_tx_packet: no buffer available", 0, 0, 0);
#endif
        return(FALSE);      /* no buffer available. */
    }

    /*  The mutual exclusion protocol for the "OWN" bit in a msg.    */
    /*  descriptor requires that no device change the state of any   */
    /*  field in any descriptor entry after relinquishing ownership. */
    /*  Therefore, the own bit must be set last.                     */
    /*                                                               */

#if (CFG_AMD_32BIT)

    /*  Transfer data to TX buffer(if needed) and set buffer pointer address   */
#if (CFG_LANCE_SHARED_MEM)          /* Shared Memory Mode */
    /* get the physical address of the tx packet from the ring descriptor.   */
    temp_txdesc.tmd0 = p_priv->ptx_ring[p_priv->cur_tx].tmd0;

    /*move the data from the DCU into the tx packet in shared memory   */
    tc_movebytes(p_priv->tx_laddr[p_priv->cur_tx], DCUTODATA(msg), length);
#else                               /* Bus Master Mode */
    /* point the tx ring descriptor to the data area of the DCU   */
    temp_txdesc.tmd0 = kvtop((PFBYTE)DCUTODATA(msg));
#endif

    /*  Write BCNT in TMD1 as a negative 2's complement #.   */
    temp_txdesc.tmd1 = ((word) -length) & RTLANCE_M_RMD1_BCNT;


    /*  Set STP bit in TMD1; Set ENP bit in TMD1;Set ones         */
    /*  Set OWN bit in TMD1 relinquishing ownership to the LANCE. */
    temp_txdesc.tmd1 = temp_txdesc.tmd1 | (RTLANCE_M_LANCE_OWNED |
                                            RTLANCE_M_TMD1_STP   |
                                            RTLANCE_M_TMD1_ONES |
                                            RTLANCE_M_TMD1_ENP);

    /* Clear TMD2,3 (mostly status returns and user space)   */
    temp_txdesc.tmd2 = 0;
    temp_txdesc.tmd3 = 0;

#else

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
    temp_txdesc.tmd0 = RTLANCE_LOW_16((PFBYTE)DCUTODATA(msg));
    temp_txdesc.tmd1 = RTLANCE_HIGH_8((PFBYTE)DCUTODATA(msg));
#endif

    /*  Set STP bit in TMD1; Set ENP bit in TMD1;                 */
    /*  Set OWN bit in TMD1 relinquishing ownership to the LANCE. */
    temp_txdesc.tmd1 = (RTWORD)(temp_txdesc.tmd1 | (RTLANCE_M_LANCE_OWNED |
                                                    RTLANCE_M_TMD1_STP   |
                                                    RTLANCE_M_TMD1_ENP));

#endif /* (CFG_AMD_32BIT) */

    /* Transfer from the temp descriptor to the actual descriptor, swapping bytes   */
    /* if this is a power pc; tmd1 last because of OWN bit.                         */
    /* tx_index = p_priv->cur_tx;  pp                                               */
    p_priv->ptx_ring[p_priv->cur_tx].tmd2 = SWAPIF(temp_txdesc.tmd2);
    p_priv->ptx_ring[p_priv->cur_tx].tmd3 = SWAPIF(temp_txdesc.tmd3);
    p_priv->ptx_ring[p_priv->cur_tx].tmd0 = SWAPIF(temp_txdesc.tmd0);
ks_disable();
    p_priv->ptx_ring[p_priv->cur_tx].tmd1 = SWAPIF(temp_txdesc.tmd1);

    /*  Increment pointer into tx ring.   */
    RTLANCE_INC_TX_PTR(p_priv->cur_tx);
ks_enable();

    /* Set up to read CSR0   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    csr0=      SWAPIF(INWORD((p_priv->base_addr)+RTLANCE_K_RDP));

    /* Trigger an immediate send.   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(csr0 | RTLANCE_M_CSR0_TDMD));
/*  OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));   */

/*  OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |   */
/*                                        RTLANCE_M_CSR0_TDMD));              */

    /* return 0 because that's what lance.c did, and this rtn is called   */
    /* through the device table.                                          */
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_tx_packet: send- cur_tx, cur_rx = ", EBS_INT2, p_priv->cur_tx, p_priv->cur_rx);
    DEBUG_ERROR("rtlance_tx_packet: returning", NOVAR, 0, 0);
#endif

    return(0);
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_tx_done                                                        */
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
RTIP_BOOLEAN rtlance_tx_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
    PRTLANCE_PRIV p_priv;

    p_priv = iface_to_priv(pi);

    if (!p_priv)
        return(FALSE);

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_tx_done: entering", NOVAR, 0, 0);
#endif

    if (success)
    {
        /* Update total number of successfully transmitted packets.   */
        p_priv->stats.packets_out++;
        p_priv->stats.bytes_out += DCUTOPACKET(msg)->length;
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_tx_done: success=", EBS_INT1, success, 0);
#endif
    }
    else
    {
        /* error - record statistics   */
#if (DEBUG_LANCE)
/*  DEBUG_ERROR("rtlance_tx_done: error=", EBS_INT1, 0, 0);   */
#endif
        p_priv->stats.errors_out++;
        p_priv->stats.tx_other_errors++;
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
        DEBUG_ERROR("rtlance_tx_done: transmit timed out, status, dirty_tx = ; resetting.", EBS_INT2,
               SWAPIF(INWORD((p_priv->base_addr)+RTLANCE_K_RDP) & 0xFFFF), p_priv->dirty_tx);
/*VK    DEBUG_ERROR("rtlance_tx_done: cur_tx, cur_rx = ", EBS_INT2, p_priv->cur_tx, p_priv->cur_rx);   */
        /* re-initialize the LANCE chip                                                                */
        rtlance_reinit(p_priv);
    }
    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/* rtlance_statistics                                                      */
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
RTIP_BOOLEAN rtlance_statistics(PIFACE pi)
{
    PRTLANCE_PRIV p_priv;
#if (INCLUDE_KEEP_STATS)
    PETHER_STATS  p_stats;
#endif
    dword saved_addr;
    KS_INTERRUPT_CONTEXT sp;    /* We'll be push/popping interrupts */

    p_priv = iface_to_priv(pi);
    if (!p_priv)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    /* If CSR112 exists for this version of the LANCE chip, copy its       */
    /* contents to the statistics area (112 holds the missed frame count). */
    if (rtlance_chip_table[p_priv->chip_version].flags & LANCE_HAS_MISSED_FRAME)
    {
        sp = ks_splx();
        saved_addr = XINWORD((p_priv->base_addr)+RTLANCE_K_RAP);
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR112));
        p_priv->stats.rx_other_errors = XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(saved_addr));
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

/* ********************************************************************         */
/* MULTICAST routines.                                                          */
/* ********************************************************************         */
/* Set or clear the multicast filter for this adaptor.                          */
/*   num_addrs == -1        Promiscuous mode, receive all packets               */
/*   num_addrs == 0     Normal mode, clear multicast list                       */
/*   num_addrs > 0      Multicast mode, receive normal and MC packets, and do   */
/*                      best-effort filtering.                                  */
/*                                                                              */
/* NOTE:  These routines were taken from the old lance.c driver and modified to */
/*        fit rtlance.c structures.  They are untested in rtlance.c.  5/99,VK   */
/*                                                                              */
RTIP_BOOLEAN rtlance_setmcast(PIFACE pi)
{
PRTLANCE_PRIV p_priv;

    p_priv = iface_to_priv(pi);
    if (!p_priv)
    {
        set_errno(ENUMDEVICE);
        return(FALSE);
    }

    rtlance_set_mcast_list(p_priv, pi->mcast.lenmclist, pi->mcast.mclist);
    return(TRUE);
}

void rtlance_set_mcast_list(PRTLANCE_PRIV p_priv, int num_addrs, void *addrs)
{
    ARGSUSED_PVOID((PFVOID)addrs);

    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_STOP)); /* Temporarily stop the lance.  */

    if (num_addrs >= 0)
    {
        int multicast_table[4];
        int i;

        /* We don't use the multicast table, but rely on upper-layer filtering.   */
        tc_memset(multicast_table, (num_addrs == 0) ? 0 : -1, sizeof(multicast_table));
        for (i = 0; i < 4; i++)
        {
            OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR8 + i));
            OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(multicast_table[i]));
        }
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR15));
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, 0x0000); /* Unset promiscuous mode */
        mode_table[p_priv->iface->minor_number] = 0x0000;
    }
    else
    {
        DEBUG_ERROR("Promiscuous mode enabled.", NOVAR, 0, 0);
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR15));
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_MODE_PROM)); /* Set promiscuous mode */
        mode_table[p_priv->iface->minor_number] = RTLANCE_M_MODE_PROM;
    }

    rtlance_reinit(p_priv);
}

/* *********************************************************************   */
void rtlance_pre_isr(int deviceno)
{
PRTLANCE_PRIV p_priv;   /* ptr to data structure private to lance driver */

    p_priv = off_to_priv(deviceno);
    if (!p_priv)
        return;

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(p_priv->irq);
}

/* *********************************************************************                 */
/*                                                                                       */
/*  rtlance_isr                                                                          */
/*                                                                                       */
/*  This is the interrupt service routine for the LANCE device.  It determines           */
/*  the reason for the interrupt and transfers control to the appropriate                */
/*  routine based on this reason.                                                        */
/*  The LANCE interrupts the host on completion of its initialization routine,           */
/*  the completion of transmission of a packet, the reception of a packet, a transmitter */
/*  timeout error, a missed packet, and a memory error.                                  */
/*  NOTE: No processing occurs as a result of IDON being set.                            */
/*        The bit is just cleared.  rtlance_open just waits on this bit                  */
/*        being set, but doesn't clear it.                                               */
/*                                                                                       */
/*  Input:  minor device number                                                          */
/*                                                                                       */
/*                                                                                       */
/*  Output: none                                                                         */
/*                                                                                       */
/*                                                                                       */
/*                                                                                       */

void rtlance_isr (int min_dev_num)
{
#if (CFG_AMD_32BIT)
    dword  csr0;    /* temporary storage for CSR0 contents */
    PRTLANCE_PRIV  p_priv;  /* ptr to data structure private to lance driver */
    int loop_cnt=10;
    dword old_ptr;
#else
    word  csr0; /* temporary storage for CSR0 contents */
    PRTLANCE_PRIV   p_priv; /* ptr to data structure private to lance driver */
    int loop_cnt=10;
    word old_ptr;
#endif

#if(PPC603)
    if (!rtlance_is_open)
        return;
#endif

    /* Translate the minor device number into the data structure for this    */
    /* device.  If we can't find either the private lance data structure, or */
    /* the RTIP interface structure, return.                                 */
    p_priv = off_to_priv(min_dev_num);
    if (!(p_priv) || !(p_priv->iface))
        return;

#if (DEBUG_LANCE)
/*  DEBUG_ERROR_INT("rtlance_isr: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("rtlance_isr: entering", NOVAR, 0, 0);
#endif

    /* Check flag to see if we are re-entering the isr.   */
    /* Increment flag to indicate we are in the isr.      */
    if (p_priv->in_isr)
    {
/*      DEBUG_ERROR_INT("rtlance_isr: (Re)entering interrupt handler. in_isr = ", EBS_INT1, p_priv->in_isr, 0);   */
        DEBUG_ERROR("rtlance_isr: (Re)entering interrupt handler. in_isr = ", EBS_INT1, p_priv->in_isr, 0);
        goto ex_it;;
    }
    p_priv->in_isr += 1;

    /* Save the RAP contents.   */
    old_ptr = XINWORD((p_priv->base_addr)+RTLANCE_K_RAP);

    /* Read CSR0 [and write back to clear it - this will clear all   */
    /* "write 1 to clear bits in CSR0]                               */
    OUTWORD ((p_priv->base_addr) + RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));

    while (((csr0 = SWAPIF(INWORD((p_priv->base_addr)+RTLANCE_K_RDP))) & (RTLANCE_M_CSR0_ERR  |
                                                    RTLANCE_M_CSR0_TINT |
                                                    RTLANCE_M_CSR0_RINT |
                                                    RTLANCE_M_CSR0_IDON))
            && (--loop_cnt >= 0))
    {

#if (DEBUG_LANCE)
/*  DEBUG_ERROR_INT("rtlance_isr: csr0=", EBS_INT1, csr0, 0);   */
    DEBUG_ERROR("rtlance_isr: csr0=", EBS_INT1, (csr0 & 0xffff), 0);
#endif

        /* If there's an initialization interrupt, clear it.   */
        if (csr0 & RTLANCE_M_CSR0_IDON)
            OUTWORD ((p_priv->base_addr) + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                                 RTLANCE_M_CSR0_IDON) );
        /* Go to the routine that can service the specific interrupt   */
        if (csr0 & RTLANCE_M_CSR0_TINT)     /* tx interrupt */
        {
            rtlance_tx_interrupt(p_priv);
            OUTWORD ((p_priv->base_addr) + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                                 RTLANCE_M_CSR0_TINT) );
        }
        if (csr0 & RTLANCE_M_CSR0_RINT)     /* rx interrupt */
        {
            rtlance_rx_interrupt(p_priv);
            OUTWORD ((p_priv->base_addr) + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                                 RTLANCE_M_CSR0_RINT) );
        }
        if (csr0 & RTLANCE_M_CSR0_ERR)      /* error bit set */
        {
            rtlance_errors(csr0, p_priv);
            OUTWORD ((p_priv->base_addr) + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                                 RTLANCE_M_CSR0_BABL |
                                                 RTLANCE_M_CSR0_CERR |
                                                 RTLANCE_M_CSR0_MISS |
                                                 RTLANCE_M_CSR0_MERR) );
        }
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_isr: cur_rx=; cur_tx=", EBS_INT2, p_priv->cur_rx, p_priv->cur_tx);
#endif
        /* make sure we're pointing to CSR0   */
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    }
    p_priv->in_isr -= 1;

    /* Restore old pointer   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(old_ptr));

ex_it:
    DRIVER_MASK_ISR_ON(p_priv->irq);
    return;
}

/* *********************************************************************             */
/*                                                                                   */
/*  rtlance_get_chip_version                                                         */
/*                                                                                   */
/*  This routine gets the version of the LANCE chip that's on the board and stores   */
/*  it.  This routine will not work for a plain vanilla LANCE (Am79C90), as it reads */
/*  CSR88 which does not exist on that board, so assume 7990 if CSR88                */
/*  does not exist.                                                                  */
/*  Note: chip_table is defined in rtlance.h                                         */
/*                                                                                   */
/*  Input:  pointer to private lance data structure                                  */
/*                                                                                   */
/*  Output: TRUE, if chip version is valid                                           */
/*          FALSE, if not valid                                                      */
/*                                                                                   */
/*  Calls: none                                                                      */
/*                                                                                   */
/*  Called by: rtlance_open                                                          */
/*                                                                                   */
RTIP_BOOLEAN rtlance_get_chip_version(PRTLANCE_PRIV p_priv)
{
    dword   rap;
    int lance_version;
    dword chip_version;
    dword ltemp;

    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR88));
    rap = XINWORD((p_priv->base_addr)+RTLANCE_K_RAP);
#if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_get_chip_version. RAP = ", EBS_INT1, rap, 0);
#endif
    if (rap != RTLANCE_K_CSR88)
    {
        lance_version = 0;
    }
    else                            /* Good, it's a newer chip. */
    {
        chip_version = (dword) XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
        chip_version &= 0xFFFF;
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(89));
        ltemp = XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
        ltemp &= 0xFFFF;
        chip_version |= ltemp << 16;
#if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_get_chip_version: LANCE chip version is ", EBS_INT1, chip_version, 0);
#endif
        if ((chip_version & 0xfff) != 0x003)
            return(FALSE);
        chip_version = (chip_version >> 12) & 0xffff;
        for (lance_version = 1; rtlance_chip_table[lance_version].id_number;
             lance_version++)
        {
            if (rtlance_chip_table[lance_version].id_number ==  chip_version)
                break;
        }

    }
    /* Store index into chip_table in data structure for later use   */
    p_priv->chip_version = lance_version;
    if( lance_version == LANCE_UNKNOWN) return(FALSE);
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance: get_chip_version: chip name", STR1, rtlance_chip_table[lance_version].name, 0);
#endif
    return(TRUE);
}
/* *********************************************************************   */
/*                                                                         */
/*  rtlance_setup_dma                                                      */
/*                                                                         */
/*  This routine                                                           */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: none                                                           */
/*                                                                         */
/*  Calls: none                                                            */
/*                                                                         */
/*  Called by: rtlance_open                                                */
/*                                                                         */
void rtlance_setup_dma(PRTLANCE_PRIV p_priv)
{
    word isacsr8;

    /* Default the stored dma channel to the cascade channel, ch. 4,   */
    /* which indicates that no DMA channel is active                   */
    /* (Native bus-master, no DMA channel needed).                     */
    p_priv->dma_channel = RTLANCE_DMA_CH4;

    /* If this is an ISA-based device with plug'n'play, read ISACSR8   */
    /* for the active dma channel.                                     */
#if (DEBUG_LANCE)
    DEBUG_ERROR("RTLANCE_SETUP_DMA:  chip_version = ", EBS_INT1, p_priv->chip_version, 0);
#endif

    if ((p_priv->chip_version == PCNET_ISAP)||(p_priv->chip_version == PCNET_ISAPA)) /* A plug-n-play version. */
    {
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_ISACSR8));
        isacsr8 = (word)XINWORD((p_priv->base_addr)+RTLANCE_K_IDP);
        p_priv->dma_channel = (word)(isacsr8 & RTLANCE_M_ISACSR8_DMA);

        /* ISACSR8 holds the irq for this device in bits 4-7.   */
        /* Get the irq from here since this is plug 'n' play.   */
        p_priv->irq = (isacsr8 >> 4) & 0x0F;
#if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_setup_dma: irq is ", EBS_INT1, p_priv->irq, 0);
#endif
    }
    else if (p_priv->chip_version != PCNET_FASTP)
    {
        DEBUG_ERROR("rtlance_setup_dma: Not PCNET_ISAP oops ", NOVAR, 0, 0);
        return;
        /* The DMA channel may be passed in PARAM1.      */
/*      if (lp->mem_start & 0x07)                        */
/*          lp->lance_dma = (int)(lp->mem_start & 0x07); */
    }

    if ((p_priv->dma_channel == 4) && (p_priv->chip_version != PCNET_FASTP))
    {
        DEBUG_ERROR("rtlance_setup_dma: no DMA needed.", NOVAR, 0, 0);
    }

#if (CFG_LANCE_BUS_MASTER)
    else
    {
#if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_setup_dma: bus master mode. dma chnl == ", EBS_INT1,p_priv->dma_channel, 0);
#endif
        rtlance_dma_init(p_priv->dma_channel);
    }
#endif
    return;
}

#if (CFG_LANCE_BUS_MASTER)

/* *********************************************************************   */
void rtlance_dma_init(int channel)
{
#if (CFG_AMD_PCI)
    ARGSUSED_INT(channel)
#else
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
#endif  /* (TARGET_186ES || CFG_AMD_PCI) */
    return;
}


#endif  /* (BUS MASTER) */


/* *********************************************************************       */
/*                                                                             */
/*  rtlance_assign_rx_ring                                                     */
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
/*  Called by: rtlance_open                                                    */
/*                                                                             */
void rtlance_assign_rx_ring(PRTLANCE_PRIV p_priv)
{
    union {
        PFBYTE pbyte;
        PRX_DESC pdesc;
    } dcu_ptr;

    /* If running in Shared Memory Mode, use the memory area defined   */
    /* in xnconf.h.                                                    */
    /* If running in Bus Master Mode, allocate a dcu and use the       */
    /* data area for the ring buffer.                                  */
#if (CFG_LANCE_SHARED_MEM)
    dcu_ptr.pbyte = CFG_LANCE_SH_MEM_ADDR;
#else
    dcu_ptr.pbyte = dcu_alloc_core(SIZEOF_RX_DESC_ARRAY + RX_TX_DESC_ALIGN);
    p_priv->prx_dcu = dcu_ptr.pbyte;
#endif

    /* Align the ring buffer address on a quadword boundary.   */
    while ((dword)dcu_ptr.pbyte & RX_TX_DESC_ALIGN) dcu_ptr.pbyte++;

    /* Store the address in the private structure.   */
    p_priv->prx_ring = dcu_ptr.pdesc;

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_assign_tx_ring                                                 */
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
/*  Called by: rtlance_open                                                */
/*                                                                         */
void rtlance_assign_tx_ring(PRTLANCE_PRIV p_priv)
{
    union {
        PFBYTE pbyte;
        PTX_DESC pdesc;
    } dcu_ptr;

    /* If running in Shared Memory Mode, use the memory area beyond   */
    /* the rx ring at CFG_LANCE_SH_MEM_ADDR defined in xnconf.h.      */
    /* If running in Bus Master Mode, allocate a dcu and use the      */
    /* data area for the ring buffer.                                 */
#if (CFG_LANCE_SHARED_MEM)
    dcu_ptr.pbyte = (PFBYTE) p_priv->prx_ring;
    dcu_ptr.pbyte += (SIZEOF_RX_DESC_ARRAY);
#else
    dcu_ptr.pbyte = dcu_alloc_core(SIZEOF_TX_DESC_ARRAY + RX_TX_DESC_ALIGN);
    p_priv->ptx_dcu = dcu_ptr.pbyte;
#endif

    /* Align the ring buffer address on a quadword boundary.   */
    while ((dword)dcu_ptr.pbyte & RX_TX_DESC_ALIGN) dcu_ptr.pbyte++;

    /* Store the address in the private structure.   */
    p_priv->ptx_ring = dcu_ptr.pdesc;

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_assign_init_block                                              */
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
/*  Called by: rtlance_open                                                */
/*                                                                         */
void rtlance_assign_init_block(PRTLANCE_PRIV p_priv)
{
    union {
        PFBYTE pbyte;
        PINIT_BLK pinit_block;
    } dcu_ptr;

    /* If running in Shared Memory Mode, use the memory area beyond   */
    /* the rx ring in the CFG_LANCE_SH_MEM_ADDR defined in xnconf.h.  */
    /* If running in Bus Master Mode, allocate a dcu and use the      */
    /* data area for the ring buffer.                                 */
#if (CFG_LANCE_SHARED_MEM)
    dcu_ptr.pbyte = (PFBYTE) p_priv->ptx_ring;
    dcu_ptr.pbyte += SIZEOF_TX_DESC_ARRAY;
#else
    dcu_ptr.pbyte = dcu_alloc_core(SIZEOF_INIT_BLOCK + INIT_BLOCK_ALIGN);
    p_priv->pinit_dcu = dcu_ptr.pbyte;
#endif

    /* Align the init block address on a word boundary.   */
    while ((dword)dcu_ptr.pbyte & INIT_BLOCK_ALIGN) dcu_ptr.pbyte++;

    /* Store the address in the private structure.   */
    p_priv->pinit_block = dcu_ptr.pinit_block;

    return;
}

/*                                                                           */
/*  rtlance_init_rx_entry                                                    */
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
/*  Called by: rtlance_open                                                  */
/*                                                                           */
RTIP_BOOLEAN rtlance_init_rx_entry(PRTLANCE_PRIV p_priv, int i)
{
    PFBYTE  addr;
    RTLANCE_RX_DESC temp_rxdesc;

#if (CFG_LANCE_SHARED_MEM)
    /* If shared memory, point to an area beyond the init block   */
    /* to serve as the DCU.                                       */
    addr = (PFBYTE) p_priv->pinit_block;
    addr += SIZEOF_INIT_BLOCK+(PKT_BUF_SZ*i);
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
        DEBUG_ERROR("rtlance_init_rx_entry: out of DCUs", NOVAR, 0, 0);
        return (FALSE);
    }
    /* Store the address of the DCU in the lance structure so that   */
    /* it can be freed later.                                        */
    p_priv->rx_dcus[i] = msg;
    /* Get the address of the data portion of the DCU to place   */
    /* in the receive descriptor.                                */
    addr = (PFBYTE) DCUTODATA(msg);
#endif
#if (CFG_AMD_32BIT)
    /* Set rmd0 address of the DCU data.   */
    temp_rxdesc.rmd0 = kvtop(addr);

    /* Set rmd1 with BCNT   */
    temp_rxdesc.rmd1 = RTLANCE_TWOS_COMP(ETHERSIZE+4) &
                                                    RTLANCE_M_RMD1_BCNT;

    /* Set required ones and the own bit in rmd1 to LANCE owned   */
    temp_rxdesc.rmd1 |= (RTLANCE_M_LANCE_OWNED |
                                RTLANCE_M_RMD1_ONES);

    /* Set rmd3 to zeros as the message count is written by the    */
    /* lance and cleared by the CPU after the message is received. */
    temp_rxdesc.rmd2 = 0;
    temp_rxdesc.rmd3 = 0;

#else
    /* Set rmd0 and the low 8 bits of rmd1 to the   */
    /* address of the DCU data.                     */
    temp_rxdesc.rmd0 = RTLANCE_LOW_16(addr);
    temp_rxdesc.rmd1 = RTLANCE_HIGH_8(addr);

    /* Set the own bit in rmd1 to LANCE owned   */
    temp_rxdesc.rmd1 |= RTLANCE_M_LANCE_OWNED;

    /* Set the buffer byte count in rmd2 (2's complement).   */
    temp_rxdesc.rmd2 = RTLANCE_TWOS_COMP(ETHERSIZE+4);

    /* Set rmd3 to zeros as the message count is written by the    */
    /* lance and cleared by the CPU after the message is received. */
    temp_rxdesc.rmd3 = 0;

#endif /* (CFG_AMD_32BIT) */

    /* Swap the bytes if the host is Big Endian.  LANCE operates Little Endian.   */
    p_priv->prx_ring[i].rmd2 = SWAPIF(temp_rxdesc.rmd2);
    p_priv->prx_ring[i].rmd3 = SWAPIF(temp_rxdesc.rmd3);
    p_priv->prx_ring[i].rmd0 = SWAPIF(temp_rxdesc.rmd0);
    p_priv->prx_ring[i].rmd1 = SWAPIF(temp_rxdesc.rmd1);

    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_init_tx_entry                                                  */
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
/*  Called by: rtlance_open                                                */
/*                                                                         */
RTIP_BOOLEAN rtlance_init_tx_entry(PRTLANCE_PRIV p_priv, int i)
{
    RTLANCE_TX_DESC temp_txdesc;

#if (CFG_LANCE_SHARED_MEM)
    PFBYTE addr;

    /* If shared memory, point to an area beyond the init block   */
    /* and the rx buffers to serve as the tx buffers.             */
    /* ?? Is there a maximum size for shared memory??             */
    /* ?? If so, I should be taking this into account so          */
    /* ?? I'm not stepping on anybody.                            */
    addr = (PFBYTE) p_priv->pinit_block;

    addr += (SIZEOF_INIT_BLOCK+(PKT_BUF_SZ*NUM_RX_DESC)+(PKT_BUF_SZ*i));

    p_priv->tx_laddr[i] = addr; /* Host logical address of xmit buffers */

#if (CFG_AMD_32BIT)
    /* Set tmd0 address of the DCU data.   */
    temp_txdesc.tmd0 = kvtop(addr);
#else
    /* Set tmd0 and the low 8 bits of tmd1 to the   */
    /* address of the DCU data.                     */
    temp_txdesc.tmd0 = RTLANCE_LOW_16(addr);
    temp_txdesc.tmd1 = RTLANCE_HIGH_8(addr);
#endif /* (CFG_AMD_32BIT) */
#else
    /* If not shared memory mode, just zero the addresses   */
    /* as they will be filled in as needed.                 */
    temp_txdesc.tmd0 = 0x0000;
    temp_txdesc.tmd1 = 0x0000;
#endif

    /* Set the own bit in tmd1 to CPU owned => clear the bit   */
    temp_txdesc.tmd1 &= RTLANCE_M_CPU_OWNED;

    /* Swap the bytes if the host is Big Endian.  LANCE operates Little Endian.   */
    /* tmd2 and tmd3 are written in rtlance_tx_packet.                            */
    p_priv->ptx_ring[i].tmd0 = SWAPIF(temp_txdesc.tmd0);
    p_priv->ptx_ring[i].tmd1 = SWAPIF(temp_txdesc.tmd1);

    return(TRUE);
}

/* *********************************************************************            */
/*                                                                                  */
/*  rtlance_build_init_block                                                        */
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
/*  Called by: rtlance_open                                                         */
/*                                                                                  */
void rtlance_build_init_block(PRTLANCE_PRIV p_priv)
{
#if (CFG_AMD_32BIT)

    int i;
    union {
        PUINIT_BLK puinit_block;
        PINIT_BLK pinit_block;
    } blk_ptr;


/*word 0 - mode /rlen/tlen   */
    p_priv->pinit_block->mode = p_priv->mode_settings;
    p_priv->pinit_block->rdr2 = RTLANCE_M_RDR2_RLEN;
    p_priv->pinit_block->tdr2 = RTLANCE_M_TDR2_TLEN;

/*word 1,2 - ethernet physical address                                */
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
            = mac_addr[i];
    }
#else
    for (i=0; i < 6; i++)
    {
        p_priv->pinit_block->padr[i]
            = p_priv->iface->addr.my_hw_addr[i]
            = INBYTE(ADDR_ADD(p_priv->base_addr,i));
    }
#endif
    p_priv->pinit_block->res1 = 0;
    p_priv->pinit_block->res2 = 0;

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_bld_init_blk: p_priv->pinit_block->padr[i]=;", EBS_INT2, i,
            p_priv->pinit_block->padr[i]);
#endif

/*word 3,4 - logical address          */
    /* Set the logical address filter */
    for (i=0; i < RTLANCE_LADR_WORD_CNT; i++)
    {
        p_priv->pinit_block->ladr[i] = p_priv->ladr[i];
    }

/*word 5 - Point to the rx ring   */
    p_priv->pinit_block->rdr1 = kvtop((PFBYTE)p_priv->prx_ring);

/*word 6 - Point to the tx ring   */
    p_priv->pinit_block->tdr1 = kvtop((PFBYTE)p_priv->ptx_ring);

/*byteswap all words   */
    blk_ptr.pinit_block=p_priv->pinit_block;

    blk_ptr.puinit_block->dword0=SWAPIF(blk_ptr.puinit_block->dword0);
    blk_ptr.puinit_block->dword3=SWAPIF(blk_ptr.puinit_block->dword3);
    blk_ptr.puinit_block->dword4=SWAPIF(blk_ptr.puinit_block->dword4);
    blk_ptr.puinit_block->dword5=SWAPIF(blk_ptr.puinit_block->dword5);
    blk_ptr.puinit_block->dword6=SWAPIF(blk_ptr.puinit_block->dword6);

#else
    int i;
    RTLANCE_T_RDR   temp_rdr;
    RTLANCE_T_TDR   temp_tdr;


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
            = mac_addr[i];
    }
#else

    for (i=0; i < 6; i++)
    {
        p_priv->pinit_block->padr[i] =
        p_priv->iface->addr.my_hw_addr[i] = INBYTE((p_priv->base_addr) + i);
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_bld_init_blk: p_priv->pinit_block->padr[i]=;", EBS_INT2, i,
            p_priv->pinit_block->padr[i]);
#endif

    }
#endif

    /* Set the logical address filter   */
    for (i=0; i < RTLANCE_LADR_WORD_CNT; i++)
    {
        p_priv->pinit_block->ladr[i] = SWAPIF(p_priv->ladr[i]);
    }

    /* Point to the rx ring and swap the bytes if Big Endian processor   */
    temp_rdr.rdr1 = RTLANCE_LOW_16((PFBYTE)p_priv->prx_ring);
    temp_rdr.rdr2 = RTLANCE_HIGH_8((PFBYTE)p_priv->prx_ring);
    temp_rdr.rdr2 |= RTLANCE_M_RDR2_RLEN;
    p_priv->pinit_block->rdr1 = SWAPIF(temp_rdr.rdr1);
    p_priv->pinit_block->rdr2 = SWAPIF(temp_rdr.rdr2);

    /* Point to the tx ring and swap the bytes if Big Endian processor   */
    temp_tdr.tdr1 = RTLANCE_LOW_16((PFBYTE)p_priv->ptx_ring);
    temp_tdr.tdr2 = RTLANCE_HIGH_8((PFBYTE)p_priv->ptx_ring);
    temp_tdr.tdr2 |= RTLANCE_M_TDR2_TLEN;
    p_priv->pinit_block->tdr1 = SWAPIF(temp_tdr.tdr1);
    p_priv->pinit_block->tdr2 = SWAPIF(temp_tdr.tdr2);

#endif /* (CFG_AMD_32BIT) */

    return;
}

/* *********************************************************************         */
/*                                                                               */
/*  rtlance_init                                                                 */
/*                                                                               */
/*  This routine initializes the LANCE.  This routine ASSUMES the initialization */
/*  block has been configured prior to its being called, and that rtlance_stop   */
/*  has been called to stop the LANCE so that CSR1, CSR2, and CSR3 can be        */
/*  written.                                                                     */
/*                                                                               */
/*  Input:  Address of Initialization Block                                      */
/*          something to indicate if the driver is to operate in interrupt or    */
/*            polling mode                                                       */
/*                                                                               */
/*  Output: Do I want to return TRUE/FALSE?                                      */
/*                                                                               */
/*  Called by: rtlance_open                                                      */
/*                                                                               */
RTIP_BOOLEAN rtlance_init(PRTLANCE_PRIV p_priv)
{
#if (CFG_AMD_32BIT)
    dword   csr0, csr1, csr2, bdp;
    int     i=0;

#if (CFG_AMD_PCI)
    if (rtlance_chip_table[p_priv->chip_version].flags &  LANCE_HAS_MII)
    {
        /* Reset the PHY   */
        rtlance_write_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, RTLANCE_M_MII_RESET);

        /* Delay at least 100msec while mode changes   */
        ks_sleep((word)(1+(ks_ticks_p_sec()/10)));
    }
#endif
    /* Reset controller and wait 3 seconds for synchronization   */
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_RESET);
/*  ks_sleep((word)(1+(ks_ticks_p_sec()*3)));   PP   */
    ks_sleep(ks_ticks_p_sec());

    /* Load CSR1, CSR2 with address of Initialization Block   */
    csr1 = (dword)kvtop((PFBYTE)p_priv->pinit_block);
    csr2 = (csr1 >> 16) & RTLANCE_M_NOAND;
    csr1 &= RTLANCE_M_NOAND;


    /* Write CSR1, CSR2 - Write_RAP (CSR1), Write_RDP (CSR1)   */
    /*                  - Write_RAP(CSR2), Write_RDP(CSR2)     */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR1));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(csr1));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR2));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(csr2));

    /* Load CSR3 (where do values come from?) I'm letting this default for now   */
    /* (set to 0's when stop bit is written).                                    */
    /* Write CSR3 - Write_RAP(CSR3), Write_RDP(CSR3)                             */

    /* Load CSR0 with INIT bit set.                  */
    /* Write CSR0 - Write_RAP(CSR0), Write_RDP(CSR0) */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INIT));

#else   /* CFG_AMD_32BIT */
    word    csr0, csr1, csr2, bdp;
    int     i=0;

#if (CFG_AMD_PCI)
    /* Reset the PHY   */
    rtlance_write_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, RTLANCE_M_MII_RESET);

    /* Delay at least 100msec while mode changes   */
    ks_sleep((word)(1+(ks_ticks_p_sec()/10)));
#endif

    /* Reset controller and wait 3 seconds for synchronization   */
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_RESET);
    ks_sleep((word)(1+(ks_ticks_p_sec()*3)));

    /* Load CSR1, CSR2 with address of Initialization Block   */
    csr1 = RTLANCE_LOW_16((PFBYTE)p_priv->pinit_block);
    csr2 = RTLANCE_HIGH_8((PFBYTE)p_priv->pinit_block);

    /* Write CSR1, CSR2 - Write_RAP (CSR1), Write_RDP (CSR1)   */
    /*                  - Write_RAP(CSR2), Write_RDP(CSR2)     */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR1));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(csr1));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR2));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(csr2));

    /* Load CSR3 (where do values come from?) I'm letting this default for now   */
    /* (set to 0's when stop bit is written).                                    */
    /* Write CSR3 - Write_RAP(CSR3), Write_RDP(CSR3)                             */

    /* Load CSR0 with INIT bit set.                  */
    /* Write CSR0 - Write_RAP(CSR0), Write_RDP(CSR0) */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INIT));


#endif /* (CFG_AMD_32BIT) */

    /* I am waiting on the initialization to complete, NOT for an   */
    /* interrupt signalling 'idon'.                                 */
    do
    {
        csr0 = XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
        if (csr0 & RTLANCE_M_CSR0_IDON)
            break;
        ks_sleep(0);        /* PP */
    } while (i++ < 100);

    if (i >= 100)   /* PP */
    {
      DEBUG_ERROR("RTLance device failed to reinitialize, csr0: ", EBS_INT1, csr0 & 0xFFFF, 0);
   }


    ks_hook_interrupt(p_priv->irq, p_priv->iface,
                      (RTIPINTFN_POINTER)rtlance_isr,
                      (RTIPINTFN_POINTER) rtlance_pre_isr,
                      p_priv->iface->minor_number);


    /* set no underflow on transmit bit   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp |= 0x800;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

#if(PROGRAM_LEDS)
    /* set enable led program bit   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR2));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp |= 0x1000;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

      /* program LED 0 to indicate ethernet activity   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR4));
        bdp = 0x00B0;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

      /* program LED 1 to indicate full duplex status or collision   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR5));
        bdp = 0x0181;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

      /* program LED 2 to indicate link status   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR6));
        bdp = 0x00C0;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

      /* program LED 3 to indicate 100 mhz operation   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR7));
        bdp = 0x1080;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
    /* reset enable led program bit   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR2));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp &= 0x7FFF;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));


#endif

    /* STRT the device.   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                      RTLANCE_M_CSR0_STRT));
    return(TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_stop                                                           */
/*                                                                         */
/*  This routine stops the LANCE by writing the stop bit in CSR0.          */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Called by: rtlance_open, rtlance_reinit, rtlance_close,                */
/*             rtlance_change_speed                                        */
/*                                                                         */
/*                                                                         */
void rtlance_stop(PRTLANCE_PRIV p_priv)
{
#if (CFG_AMD_32BIT)
    dword   csrtmp;
#else
    word    csrtmp;
#endif

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_stop", NOVAR, 0, 0);
#endif

    /* writing a 0 to the address register first to insure CSR0 is accessed.   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    csrtmp = XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
    csrtmp |=RTLANCE_M_CSR0_STOP;
    /* write CSR0 with STOP bit set   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(csrtmp));

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_spnd                                                           */
/*                                                                         */
/*  This routine puts the LANCE (Am79C972) into SUSPEND mode by writing    */
/*  the spnd bit in CSR5.                                                  */
/*                                                                         */
/*  Input:  pointer to private lance data structure                        */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Called by: rtlance_set_loopback                                        */
/*                                                                         */
/*                                                                         */
void rtlance_spnd(PRTLANCE_PRIV p_priv)
{
    int i;
    word csr5;

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_spnd", NOVAR, 0, 0);
#endif

    /* For Am79C972 only.  Processes all current rx's and tx's, then waits.   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR5));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR5_SPND));
    for (i=0; i<1000; i++)
    {
        csr5 = (word)XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
        if (csr5 & RTLANCE_M_CSR5_SPND)
            break;
    }

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_free_dcus                                                      */
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
/*  Called by: rtlance_close                                               */
/*                                                                         */
void rtlance_free_dcus(PRTLANCE_PRIV p_priv)
{
    int i;

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_free_dcus: entering", NOVAR, 0, 0);
#endif

    /* Free all allocated receive buffers, and zero the table.   */
    for (i=0; i < NUM_RX_DESC; i++)
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
/*  rtlance_restart                                                           */
/*                                                                            */
/*  This routine causes the LANCE to restart.  The init block is not reloaded */
/*  and no change is made in the ring buffers.                                */
/*  It is assumed that the init block has been loaded and that rtlance_spnd   */
/*  was called prior to calling this routine.                                 */
/*                                                                            */
/*  Input: pointer to LANCE data structure                                    */
/*                                                                            */
/*  Output: restarted LANCE device (indirect output)                          */
/*                                                                            */
/*  Called By: rtlance_set_loopback                                           */
/*                                                                            */
void rtlance_restart(PRTLANCE_PRIV p_priv)
{
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_restart: entering", NOVAR, 0, 0);
#endif

    /* Set STRT bit and INEA bit in CSR0   */
    OUTWORD(p_priv->base_addr + RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD(p_priv->base_addr + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                                      RTLANCE_M_CSR0_STRT));
    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_reinit                                                         */
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
void rtlance_reinit(PRTLANCE_PRIV p_priv)
{
#if (CFG_AMD_32BIT)
    dword   bdp,csr0;
#else
    word    bdp,csr0;
#endif
int i=0;
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_reinit: entering", NOVAR, 0, 0);
#endif

    /* Write STOP bit in CSR0 (LANCE must be STOPPED in order to write CSR1&CSR2.   */
    rtlance_stop(p_priv);

    /* Call rtlance_reorder_buffers to rearrange descriptors in tx or rx ring.   */
    rtlance_reorder_buffers(p_priv);

    /* Set INIT bit in CSR0 and wait for reset to complete   */
    OUTWORD(p_priv->base_addr + RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD(p_priv->base_addr + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INIT));
    while (i++ < 100)
    {
        csr0 = XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
        if (csr0 & RTLANCE_M_CSR0_IDON)
            break;
    }

    /* set no underflow on transmit bit   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp |= 0x800;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

    /* start the device   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD(p_priv->base_addr + RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                      RTLANCE_M_CSR0_STRT ));
    return;
}

/* *********************************************************************     */
/*                                                                           */
/*  rtlance_reorder_buffers                                                  */
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
void rtlance_reorder_buffers(PRTLANCE_PRIV p_priv)
{
/*  word temp_ptr;   */
    int i;

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_reorder buffers: entering", NOVAR, 0, 0);
    DEBUG_ERROR("rtlance_reorder buffers: cur_tx, cur_rx", EBS_INT2, p_priv->cur_tx, p_priv->cur_rx);

#endif

    /* for the sake of moving on with debug, use the old stuff for now. 3/15 VK   */
    for (i=0; i<NUM_RX_DESC; i++)
    {
/*          //free the packet, then reload the ring entry   */
/*          if (p_priv->rx_dcus[i])                         */
/*              os_free_packet(p_priv->rx_dcus[i]);         */
/*          p_priv->rx_dcus[i] = 0;                         */
        rtlance_init_rx_entry(p_priv, i);
    }

/*      // Move the current rx descriptor + 1 and all following descriptors,      */
/*      // in order, to the 0 position of the rx ring.                            */
/*      temp_ptr = p_priv->cur_rx;                                                */
/*      RTLANCE_INC_RX_PTR(temp_ptr);                                             */
/*                                                                                */
/*      if (temp_ptr == 0)                                                        */
/*          ; // the next descriptor is at the 0 point, so everything's in place. */
/*      else                                                                      */
/*      {                                                                         */
/*          for (i=0; i<NUM_RX_DESC; i++)                                         */
/*          {                                                                     */
/*      }                                                                         */
/*      // Set the rx ring pointers to point to the 'top' of the rx ring          */
    p_priv->cur_rx = 0;

    /* for the sake of debug, do what lance.c did. 3/15 VK   */
    for (i=0; i<NUM_TX_DESC; i++)
        rtlance_init_tx_entry(p_priv, i);

/*      // Re-order the tx pointers to point to the next buffer to transmit   */
/*      // Set the tx ring pointers to point to the 'top' of the tx ring      */
    p_priv->cur_tx = p_priv->dirty_tx = 0;

    return;
}

/*                                                                   */
/*  rtlance_errors                                                   */
/*                                                                   */
/*  This routine handles errors reported by the LANCE via interrupt. */
/*                                                                   */
/*  Input:  Copy of CSR0 read by rtlance_isr                         */
/*                                                                   */
/*  Output: none                                                     */
/*                                                                   */
/*  Calls:                                                           */
/*                                                                   */
/*  Called by: rtlance_isr (when ERR set in CSR0)                    */
/*                                                                   */
void rtlance_errors(dword csr0, PRTLANCE_PRIV p_priv)
{
#if (DEBUG_LANCE)
/*  DEBUG_ERROR_INT("rtlance_errors: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("rtlance_errors: entering", NOVAR, 0, 0);
#endif

    /* Get error bits set in CSR0 (BABL and/or MISS) and   */
    /* update statistics.                                  */
    if (csr0 & RTLANCE_M_CSR0_BABL)
        p_priv->stats.errors_out++;         /* TX babble */
    if (csr0 & RTLANCE_M_CSR0_MISS)
        p_priv->stats.errors_in++;          /* missed RX packet */

    /* if it's a memory error, stop and restart the LANCE.   */
    /* If MERR                                               */
    /*  {                                                    */
    /*      STOP the LANCE (STP in CSR0);                    */
    /*      Rearrange rx and tx descriptors;                 */
    /*      Initialize the LANCE;                            */
    /*  }                                                    */
    if (csr0 & RTLANCE_M_CSR0_MERR)
        rtlance_reinit(p_priv);
    return;
}

/* *********************************************************************       */
/*                                                                             */
/*  rtlance_rx_interrupt                                                       */
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
/*  Called by: rtlance_isr (when RINT in CSR0 is set)                          */
/*                                                                             */
void rtlance_rx_interrupt(PRTLANCE_PRIV p_priv)
{
    RTIP_BOOLEAN error = FALSE;
    word pkt_len;
#if (CFG_AMD_32BIT)
    dword temp_rmd2;
#else
    RTWORD temp_rmd3;
#endif
    DCU msg = 0;
    DCU new_msg;

#if (DEBUG_LANCE)
/*  DEBUG_ERROR_INT("rtlance_rx_interrupt: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("rtlance_rx_interrupt: entering", NOVAR, 0, 0);
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
        if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_ERR))
        {
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_rx_int: RMD1_ERR - p_priv->cur_rx=", EBS_INT1, p_priv->cur_rx, 0);
#endif
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_FRAM))
                p_priv->stats.rx_frame_errors++;
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_OFLO))
                p_priv->stats.rx_overwrite_errors++;
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_CRC))
                p_priv->stats.rx_crc_errors++;
            if (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_BUFF))
                p_priv->stats.rx_fifo_errors++;
            /* Set the own bit in the current rx ring entry to LANCE OWNED   */
            p_priv->prx_ring[p_priv->cur_rx].rmd1 |= SWAPIF(RTLANCE_M_LANCE_OWNED);
            /* Point to the next entry in the rx ring.   */
            RTLANCE_INC_RX_PTR(p_priv->cur_rx)
        }
        /* Read RMD1 checking for STP and ENP both set.                    */
        /* We allocate maximum size entries for the rx_ring, so STP        */
        /* and ENP should both be set, otherwise, it's an error.  We       */
        /* don't expect any buffer chaining to occur.                      */
        /* Note:  this should cover the jabber packet error condition      */
        /*        as documented in the old lance driver.                   */
        /*        Also, it is assumed that INCLUDE_LANCE is set, therefore */
        /*        ETHERSIZE+4 is 1518 (in xnconf.h).                       */
        else if (!((p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_STP)) &&
                   (p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_ENP))))
        {
            /* Go through the ring giving the buffer back to the lance   */
            /* until ENP is set.                                         */
            do
            {
#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_rx_int: STP && ENP - p_priv->cur_rx=", EBS_INT1, p_priv->cur_rx, 0);
#endif
                p_priv->prx_ring[p_priv->cur_rx].rmd1 |=SWAPIF(RTLANCE_M_LANCE_OWNED);
                RTLANCE_INC_RX_PTR(p_priv->cur_rx)
            } while (!(p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_RMD1_ENP)));
            p_priv->stats.rx_other_errors++;
        }
        /* There are no errors (all handled above),   */
        /* so "send the packet up"                    */
        else
        {
#if (CFG_AMD_32BIT)
            temp_rmd2 = p_priv->prx_ring[p_priv->cur_rx].rmd2;
            pkt_len = (word)((SWAPIF(temp_rmd2) & RTLANCE_M_RMD2_MCNT)-4);
#else
            temp_rmd3 = (RTWORD)(p_priv->prx_ring[p_priv->cur_rx].rmd3);
            pkt_len = (word)((SWAPIF(temp_rmd3) & RTLANCE_M_RMD3_MCNT)-4);
#endif
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
                DEBUG_ERROR("rtlance_rx_int: out of DCUs", NOVAR, 0, 0);
                p_priv->stats.packets_lost++;
                /* Set the own bit in the current rx ring entry to LANCE OWNED   */
                p_priv->prx_ring[p_priv->cur_rx].rmd1 |= SWAPIF(RTLANCE_M_LANCE_OWNED);
            }
            else
            {

#if (DEBUG_LANCE)
    DEBUG_ERROR("rtlance_rx_int: p_priv->cur_rx=; length=;", EBS_INT2,
    p_priv->cur_rx, pkt_len);
/*  DEBUG_ERROR("rtlance_rx_int: pkt = ", PKT, DCUTODATA(msg), 25);   */
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
                rtlance_init_rx_entry(p_priv, p_priv->cur_rx);
            }
            /* Point to the next entry in the rx ring.   */
            RTLANCE_INC_RX_PTR(p_priv->cur_rx)
        }

    } while (!(p_priv->prx_ring[p_priv->cur_rx].rmd1 & SWAPIF(RTLANCE_M_LANCE_OWNED)));

    return;
}

/*                                                              */
/*  rtlance_tx_interrupt                                        */
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
/*  Called by: rtlance_isr (when TINT is set in CSR0)           */
/*                                                              */
void rtlance_tx_interrupt(PRTLANCE_PRIV p_priv)
{
    int test_index;

#if (DEBUG_LANCE)
/*  DEBUG_ERROR_INT("rtlance_tx_interrupt: entering", NOVAR, 0, 0);   */
    DEBUG_ERROR("rtlance_tx_interrupt: entering", NOVAR, 0, 0);
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
             SWAPIF(RTLANCE_M_TMD1_ERR)))

#if (CFG_AMD_32BIT) /* when BCSR18 DWIO = 1 */
        {
            p_priv->stats.errors_out++;
            if (p_priv->ptx_ring[test_index].tmd2 &
                SWAPIF(RTLANCE_M_TMD2_RTRY))
                p_priv->stats.collision_errors++;
            if (p_priv->ptx_ring[test_index].tmd2 &
                SWAPIF(RTLANCE_M_TMD2_LCAR))
                p_priv->stats.tx_carrier_errors++;
            if (p_priv->ptx_ring[test_index].tmd2 &
                SWAPIF(RTLANCE_M_TMD2_LCOL))
                p_priv->stats.owc_collision++;
            if (p_priv->ptx_ring[test_index].tmd2 &
                SWAPIF(RTLANCE_M_TMD2_UFLO))
            {
                p_priv->stats.tx_fifo_errors++;
                /* underflow error indicates that the tx fifo   */
                /* was emptied before ENP was reached.          */
                /* transmitter is turned off, so re-init to get */
                /* things going again.                          */
                rtlance_reinit(p_priv);
            }
        }

#else
        {
            p_priv->stats.errors_out++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(RTLANCE_M_TMD3_RTRY))
                p_priv->stats.collision_errors++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(RTLANCE_M_TMD3_LCAR))
                p_priv->stats.tx_carrier_errors++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(RTLANCE_M_TMD3_LCOL))
                p_priv->stats.owc_collision++;
            if (p_priv->ptx_ring[test_index].tmd3 &
                SWAPIF(RTLANCE_M_TMD3_UFLO))
            {
                p_priv->stats.tx_fifo_errors++;
                /* underflow error indicates that the tx fifo   */
                /* was emptied before ENP was reached.          */
                /* transmitter is turned off, so re-init to get */
                /* things going again.                          */
                rtlance_reinit(p_priv);
            }
        }
#endif /* (CFG_AMD_32BIT) */

        else
        {
            if (p_priv->ptx_ring[test_index].tmd1 &
                  (SWAPIF(RTLANCE_M_TMD1_MORE | RTLANCE_M_TMD1_ONE)))
                p_priv->stats.one_collision++;
            p_priv->stats.packets_out++;
        }
        RTLANCE_INC_TX_PTR(test_index);
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
/*  off_to_priv                                                            */
/*                                                                         */
PRTLANCE_PRIV off_to_priv(int min_dev_num)
{
    if (min_dev_num >= CFG_NUM_LANCE)
    {
        DEBUG_ERROR("off_to_priv(): pi->minor_number, CFG_NUM_LANCE = ",
            EBS_INT2, min_dev_num, CFG_NUM_LANCE);
        return ((PRTLANCE_PRIV) 0);
    }
    return ((PRTLANCE_PRIV) s_priv_table[min_dev_num]);
}

/*                */
/*  iface_to_priv */
/*                */
PRTLANCE_PRIV iface_to_priv(PIFACE pi)
{
    if (pi->minor_number >= CFG_NUM_LANCE)
    {
        DEBUG_ERROR("iface_to_priv() - pi->minor_number, CFG_NUM_LANCE = ",
            EBS_INT2, pi->minor_number, CFG_NUM_LANCE);
        return ((PRTLANCE_PRIV) 0);
    }
    return ((PRTLANCE_PRIV) s_priv_table[pi->minor_number]);
}

/* *********************************************************************      */
/*                                                                            */
/*  APPLICATION PROGRAM INTERFACE ROUTINES.                                   */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*  rtlance_set_mode                                                          */
/*                                                                            */
/*  The following routine is used to set the bits in the Mode                 */
/*  Register of the Lance Initialization Block.  This routine is called by    */
/*  a higher layer to dictate how the Lance is meant to operate.              */
/*  The Lance driver defaults are zeros (not set) for all bits in this        */
/*  register.                                                                 */
/*  It is expected that the caller will then call rtlance_reinit in order for */
/*  the selected mode to take effect.                                         */
/*                                                                            */
/*  Input:  Pointer to the interface structure for this device                */
/*          Value to write into the Mode Register (see rtlance.h)             */
/*                                                                            */
/*  Output: FALSE (0) if the minor device number is out of range,             */
/*          TRUE  (1) otherwise.                                              */
/*             (indirectly, the bit will be set in the global mode structure  */
/*             for this device)                                               */
/*                                                                            */
/*  Ex:  To set promiscuous mode, rtlance_set_mode(pi, RTLANCE_M_MODE_PROM)   */
/*                                                                            */
RTIP_BOOLEAN rtlance_set_mode(PIFACE pi, word value)
{
    PRTLANCE_PRIV   p_priv;
#if (CFG_AMD_32BIT)
    dword bdp;
#else
    word bdp;
#endif

    p_priv = iface_to_priv(pi);
    if (!p_priv)
        return(FALSE);

    mode_table[pi->minor_number] = value;
#if (CFG_AMD_32BIT)
    bdp = (dword)value;
#else
    bdp = value;
#endif
    rtlance_spnd(p_priv);   /* put the device in a suspend state */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR15));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(bdp));
    rtlance_restart(p_priv);    /* get the device going again */
    return (TRUE);
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_load_lance                                                     */
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
void rtlance_load_lance(IOADDRESS ioaddr, word reg_num, word value)
{
    OUTWORD(ioaddr+RTLANCE_K_RAP, SWAPIF(reg_num));
    OUTWORD(ioaddr+RTLANCE_K_RDP, SWAPIF(value));
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
void dump_lance(IOADDRESS ioaddr, word start_reg, word end_reg)
{
#if (CFG_AMD_32BIT)
dword w, i;
#else
word w, i;
#endif

    for (i=start_reg; i<=end_reg; i++)
    {
        OUTWORD(ADDR_ADD(ioaddr,RTLANCE_K_RAP), SWAPIF(i));
        w = XINWORD(ADDR_ADD(ioaddr,RTLANCE_K_RDP));

        DEBUG_ERROR("REG_NUM == , REG_VALUE == ", DINT2, i, w);
    }
    return;
}

/* *********************************************************************   */
#if (CFG_AMD_PCI)
/*                                                                       */
/*  rtlance_change_speed                                                 */
/*                                                                       */
/*  This routine is used by a higher layer to change the speed of        */
/*  the PHY (10BT/100BT/AUTONEGOTIATE) used with the PCI-based Am79C972. */
/*                                                                       */
/*  Input:  Pointer to the interface structure for this device           */
/*          Desired setting of the device (see xnconf.h for values)      */
/*                                                                       */
/*  Output: None                                                         */
/*                                                                       */
/*  Indirect Output:  rt_set_phy_speed (rtipdata.c) set to desired speed */
/*                    reinit of LANCE device                             */
/*                                                                       */
/*  Calls:  rtlance_stop                                                 */
/*          rtlance_set_phy_speed                                        */
/*          rtlance_get_phy_speed                                        */
/*          rtlance_reinit                                               */
/*                                                                       */
RTIP_BOOLEAN rtlance_change_speed(PIFACE pi, int speed)
{
    PRTLANCE_PRIV   p_priv;

    p_priv = iface_to_priv(pi);     /* point to driver structure for this device */
    if (!p_priv)
        return(FALSE);

    if (!(rtlance_chip_table[p_priv->chip_version].flags &  LANCE_HAS_MII))
        return(TRUE);

    if ((speed > PHY_AUTO) || (speed < PHY_10BT_HD))
        return(FALSE);
    if (speed == rt_actual_phy_speed)
        return(TRUE);

    rt_set_phy_speed = speed;       /* reset speed indicator */

    rtlance_stop(p_priv);           /* stop the device */
    rtlance_set_phy_speed(p_priv);  /* change the speed of the device */
    rtlance_get_phy_speed(p_priv);  /* save the current device speed */
    rtlance_reinit(p_priv);         /* get the device going again */

    if (rt_set_phy_speed != PHY_AUTO)
    {
        return ((RTIP_BOOLEAN)(rt_set_phy_speed == rt_actual_phy_speed));
    }
    else
    {
        return(TRUE);
    }
}

/*                                                                              */
/*  rtlance_set_loopback                                                        */
/*                                                                              */
/*  This routine saves the loopback type passed to it in a global location, and */
/*  calls rtlance_loopback to set the MII to internal, external, or no loopback */
/*  based on the parameter passed in.                                           */
/*  A restart of the device is forced to cause loopback to take effect.         */
/*  GPSI loopback is not supported.                                             */
/*  This routine assumes that the device has been opened prior to this call.    */
/*  To set loopback type prior to an initial opening of the device, set         */
/*  rt_loopback_type in rtipdata.c (default is no loop) and re-compile/link,    */
/*  then init(run) rtip.                                                        */
/*                                                                              */
/*  Input:  pi - Pointer to the interface structure for this device             */
/*          loop-type - Indicates internal or external loopback                 */
/*                                                                              */
/*  Calls:  rtlance_loopback                                                    */
/*          rtlance_spnd                                                        */
/*          rtlance_restart                                                     */
/*                                                                              */
/*  Called by: user application                                                 */
/*                                                                              */
RTIP_BOOLEAN rtlance_set_loopback(PIFACE pi, int loop_type)
{
    PRTLANCE_PRIV   p_priv;

    p_priv = iface_to_priv(pi);     /* point to driver structure for this device */
    if (!p_priv)
        return(FALSE);

/*  if (rt_loopback_type == loop_type)   */
/*      return TRUE;                     */

    rt_loopback_type = loop_type;   /* save the loopback type */

    rtlance_spnd(p_priv);           /* put the device in a suspend state */
    rtlance_loopback(p_priv);       /* set the device to the loopback type chosen */
    rtlance_restart(p_priv);        /* get the device going again */

    return(TRUE);
}

#if (CFG_AMD_PCI)

/* *********************************************************************   */
/*                                                                         */
/*  DRIVER and API SUPPORT ROUTINES                                        */
/*                                                                         */
/*  rtlance_set_phy_speed                                                  */
/*                                                                         */
/*  This routine reads the variable rt_set_phy_speed and configures the    */
/*  Am79C972 PHY based on that setting.                                    */
/*                                                                         */
/*  Input:  pointer to the device private structure                        */
/*                                                                         */
/*  Output: none                                                           */
/*                                                                         */
/*  Calls:  OUTWORD, XINWORD, rtlance_write_MIIreg                         */
/*                                                                         */
/*  Called by:  rtlance_open, rtlance_change_speed                         */
/*                                                                         */
void rtlance_set_phy_speed(PRTLANCE_PRIV p_priv)
{
#if (CFG_AMD_32BIT)
    dword bdp, mii_val;
#else
    word bdp, mii_val;
#endif
    RTIP_BOOLEAN    phy_reset = TRUE;
    int lance_version;

    lance_version = p_priv->chip_version;

    if (!(rtlance_chip_table[p_priv->chip_version].flags &  LANCE_HAS_MII)) return;

    /* Stop the LANCE   */
    rtlance_stop(p_priv);

    /* Reset the PHY   */
    rtlance_write_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, RTLANCE_M_MII_RESET);

    /* Delay at least 100msec while mode changes   */
    ks_sleep((word)(1+(ks_ticks_p_sec()/10)));


    if(lance_version == PCNET_FASTP)
    {
    /* Set to auto negotiate or if setting speed directly, disable Network Port Manager.   */
    /* Set BCR2...only valid for 79c972                                                    */
    /* If auto negotiation, set BCR2, bit 0(ASEL) to 1                                     */
    /* If setting speed directly, set ASEL to 0                                            */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR2));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
    if (rt_set_phy_speed == PHY_AUTO)
        bdp |= RTLANCE_M_BCR2_ASEL;
    else
        bdp &= RTLANCE_M_BCR2_ASEL_OFF;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
    }

    /*                                                                   */
    /* Set BCR32                                                         */
    /* If auto negotiation, set BCR32,bit 7(DANAS)to 0 and XPHYANE to 1. */
    /* If setting speed directly, set DANAS to 1 and XPHYANE to 0.       */
    /*                                                                   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR32));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
    if (rt_set_phy_speed == PHY_AUTO)
    {
        bdp |= RTLANCE_M_BCR32_XPHYANE;
        bdp &= RTLANCE_M_BCR32_DANAS_OFF;
    }
    else
    {
        bdp |= (RTLANCE_M_BCR32_DANAS | RTLANCE_M_BCR32_XPHYRST);
        bdp &= RTLANCE_M_BCR32_XPHYANE_OFF;
    }
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

    /* Do a s_reset to cause the Network Port Manager to be disabled or   */
    /* auto-negotiation to occur.                                         */
    /* Reset controller and wait 3 seconds for synchronization            */
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_RESET);
    ks_sleep((word)(1+(ks_ticks_p_sec()*3)));

    /* set no underflow on transmit bit   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp |= 0x800;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

    /* Select the MII: set CSR15, bits 7,8(PORTSEL) to 1,1           */
    /* Before CSR15 can be written, either STOP(CSR0) or SPND (CSR5) */
    /* must be set.                                                  */
    /* The above reset will cause STOP to be set.                    */
    /*                                                               */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR15));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_RDP);
    bdp |= RTLANCE_M_CSR15_MII;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(bdp));

    /* Save the value of CSR15 to be placed in the init block.   */
    mode_table[p_priv->iface->minor_number] = (word)bdp;

    if (rt_set_phy_speed != PHY_AUTO)
    {
        /* Wait for the PHY Reset to complete.   */
        do
        {
            rtlance_read_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, &bdp);
            if (bdp & RTLANCE_M_MII_RESET)
                ;
            else
                phy_reset = FALSE;
        } while (phy_reset);


        /*                                                                  */
        /* If Full Duplex Mode, write BCR9, FDEN(bit 0)-full duplex enable. */
        /*                                                                  */
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR9));
        bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        if (rt_set_phy_speed==PHY_10BT_FD || rt_set_phy_speed==PHY_100BT_FD)
            bdp |= RTLANCE_M_BCR9_FDEN;
        else
            bdp &= RTLANCE_M_BCR9_FDEN_OFF;
        OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

        /*                                                                  */
        /* Read BCR32 and write it back setting speed (XPHYSP) and          */
        /* full/half-duplex (XPHYFD) based on the value of rt_set_phy_speed */
        /* in rtipdata.c.                                                   */
        /*                                                                  */
        OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR32));
        bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp &= RTLANCE_M_BCR32_XPHYRST_OFF;

        if (rt_set_phy_speed==PHY_100BT_HD)
        {                                   /* 100BT half-duplex */
            bdp |= RTLANCE_M_BCR32_XPHYSP;
            bdp &= RTLANCE_M_BCR32_XPHYFD_OFF;
            mii_val = RTLANCE_M_MII_100BT;
        }
        else if (rt_set_phy_speed==PHY_100BT_FD)
        {                                   /* 100BT full-duplex */
            bdp |= RTLANCE_M_BCR32_XPHYSP;
            bdp |= RTLANCE_M_BCR32_XPHYFD;
            mii_val = RTLANCE_M_MII_100BT | RTLANCE_M_MII_FD;
        }
        else if (rt_set_phy_speed==PHY_10BT_FD)
        {                                   /* 10BT full-duplex */
            bdp |= RTLANCE_M_BCR32_XPHYFD;
            bdp &= RTLANCE_M_BCR32_XPHYSP_OFF;
            mii_val = RTLANCE_M_MII_FD;
        }
        else    /* default - 10BT half-duplex */
        {
            bdp &= RTLANCE_M_BCR32_XPHYSP_OFF;
            bdp &= RTLANCE_M_BCR32_XPHYFD_OFF;
            mii_val = RTLANCE_M_MII_NOVAL;
        }
        OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));  /* write BCR32 */

        /* write MII control register to force the PHY to the chosen speed   */
        rtlance_write_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, mii_val);
    }

    /* restart the device before we return.   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_CSR0));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_INEA |
                                      RTLANCE_M_CSR0_STRT));
    return;
}

/* *********************************************************************              */
/*                                                                                    */
/*  rtlance_get_phy_speed                                                             */
/*                                                                                    */
/*  This routine reads the MMI generic control register to determine the actual speed */
/*  at which the PHY is running.  The speed(see values in xnconf.h) is written        */
/*  into the variable rt_actual_phy_speed in rtipdata.c.                              */
/*                                                                                    */
/*  Input:  pointer to the private driver data structure                              */
/*                                                                                    */
/*  Output: none                                                                      */
/*                                                                                    */
/*  Indirect output:  rt_actual_phy_speed                                             */
/*                                                                                    */
void rtlance_get_phy_speed(PRTLANCE_PRIV p_priv)
{
#if (CFG_AMD_32BIT)
    dword anr0;
#else
    word anr0;
#endif

    anr0 = rtlance_dump_anr(p_priv, 0, 0);  /* read phy anr0, no suspend */
    if (anr0 & RTLANCE_M_MII_FD)    /* if full dup... */
    {
        if (anr0 & RTLANCE_M_MII_100BT) 
            rt_actual_phy_speed =PHY_100BT_FD;
        else 
            rt_actual_phy_speed =PHY_10BT_FD;
    }
    else                    /* else half dup... */
    {
        if (anr0 & RTLANCE_M_MII_100BT) 
            rt_actual_phy_speed =PHY_100BT_HD;
        else 
            rt_actual_phy_speed =PHY_10BT_HD;
    }
    if (anr0 & RTLANCE_M_MII_ANE)
      rt_actual_phy_speed =PHY_AUTO;

    return;
}

/* *********************************************************************   */
/*                                                                         */
/*  rtlance_write_MIIreg                                                   */
/*                                                                         */
/*  This routine writes a Media Independent Interface (MII) register with  */
/*  the specified value through BCR33 (address of MII with register masked */
/*  in) and BCR34 (value to be written).                                   */
/*                                                                         */
/*  Input:  Address of driver data structure                               */
/*          Register Number as a mask                                      */
/*          Value to place in register                                     */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Indirect Output:  updated MII register value                           */
/*                                                                         */


#if (CFG_AMD_32BIT)
void rtlance_write_MIIreg(PRTLANCE_PRIV p_priv, dword regmask, dword value)
{
dword bdp,phy_ad;
#else
void rtlance_write_MIIreg(PRTLANCE_PRIV p_priv, word regmask, word value)
{
word bdp,phy_ad;
#endif
int lance_version;

    lance_version = p_priv->chip_version;

    /* Put address of MII register writing in RAP   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR33));
    if(lance_version == PCNET_FASTP)    /*79972 uses external phy at addr 1 */
        phy_ad = 0x0020;    /* addr <<5 */
    else if(lance_version == PCNET_FAST3)   /*79973 uses external phy at addr 0x1E */
        phy_ad = 0x03C0;    /* addr <<5 */
    else if(lance_version == PCNET_FAST3A)  /*79975 uses external phy at addr 0x1E */
        phy_ad = 0x03C0;    /* addr <<5 */
    else
        phy_ad = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
    bdp = phy_ad | regmask;
OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
#    if (DEBUG_LANCE)
        DEBUG_ERROR("write_MII BCR33=", EBS_INT1, bdp, 0);
        DEBUG_ERROR("write_MII BCR34=", EBS_INT1, value, 0);
#    endif


    /* Put data writing to MII register in BCR34   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR34));
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(value));

    return;
}
/* *********************************************************************   */
/*                                                                         */
/*  rtlance_read_MIIreg                                                    */
/*                                                                         */
/*  This routine reads a Media Independent Interface (MII) register with   */
/*  the specified value through BCR33 (address of MII with register masked */
/*  in) and BCR34 (value read).                                            */
/*                                                                         */
/*  Input:  Address of driver data structure                               */
/*          Register Number as a mask                                      */
/*          Address at which to place data read                            */
/*                                                                         */
/*  Output: None                                                           */
/*                                                                         */
/*  Indirect Output:  updated MII register value                           */
/*                                                                         */
#if (CFG_AMD_32BIT)
void rtlance_read_MIIreg(PRTLANCE_PRIV p_priv, dword regmask, dword *value)
{
dword bdp,phy_ad;
#else
void rtlance_read_MIIreg(PRTLANCE_PRIV p_priv, word regmask, word *value)
{
word bdp,phy_ad;
#endif
int lance_version;

    lance_version = p_priv->chip_version;

    /* Put address of MII register reading in RAP   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR33));
    if(lance_version == PCNET_FASTP)    /*79972 uses external phy at addr 1 */
        phy_ad = 0x0020;    /* addr <<5 */
    else if(lance_version == PCNET_FAST3)   /*79973 uses external phy at addr 0x1E */
        phy_ad = 0x03C0;    /* addr <<5 */
    else if(lance_version == PCNET_FAST3A)  /*79975 uses external phy at addr 0x1E */
        phy_ad = 0x03C0;    /* addr <<5 */
    else
        phy_ad = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
    bdp = phy_ad | regmask;

    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
    /* Read value of MII register from BCR34   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR34));
    *value = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);

    return;
}

/* *********************************************************************      */
/*                                                                            */
/*  rtlance_loopback                                                          */
/*                                                                            */
/*  This routine does the register writes required to place the device in the */
/*  loopback mode, or no loop mode, as indicated by the global variable       */
/*  rt_loopback_type (in rtipdata.c).                                         */
/*  rtlance_stop must have been called prior to the calling of this routine   */
/*  as CSR15 cannot be written unless the device is STOPed.                   */
/*  GPSI loopback is not supported.                                           */
/*                                                                            */
/*  Input:  p_priv - Pointer to the private data structure for this device    */
/*                                                                            */
/*  Output: none                                                              */
/*                                                                            */
/*  Indirect Output: writes to BCR2, BCR32, CSR15                             */
/*                                                                            */
/*  Calls:  XINWORD, OUTWORD                                                  */
/*                                                                            */
/*  Called by:  rtlance_open, rtlance_set_loopback                            */
/*                                                                            */
#if (CFG_AMD_32BIT)
void rtlance_loopback(PRTLANCE_PRIV p_priv)
{
    dword bdp;
#else
void rtlance_loopback(PRTLANCE_PRIV p_priv)
{
    word bdp;
#endif
    int lance_version;

    lance_version = p_priv->chip_version;

/* Reset the PHY   */
    rtlance_write_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, RTLANCE_M_MII_RESET);

/* Delay at least 100msec while mode changes   */
    ks_sleep((word)(1+(ks_ticks_p_sec()/10)));

/* Reset controller and wait 3 seconds for synchronization   */
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_RESET);
    ks_sleep((word)(1+(ks_ticks_p_sec()*3)));

    /* set no underflow on transmit bit   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
        bdp |= 0x800;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));

/* Manual port select for EXTERNAL_LOOP, auto for NO_LOOP or INTERNAL_LOOP   */
/* Valid for 79c972 chip only                                                */
    if(lance_version == PCNET_FASTP)
    {
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR2));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
    if (rt_loopback_type == NO_LOOP)
        bdp |= RTLANCE_M_BCR2_ASEL;
    else
        bdp &= RTLANCE_M_BCR2_ASEL_OFF;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
#    if (DEBUG_LANCE)
        DEBUG_ERROR("set loopback: port sel BCR2=", EBS_INT1, bdp, 0);
#    endif
    }


/* *********************************************************************            */
/* Set no underflow for LOOP                                                        */
/*  OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR18));            */
/*  bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);                               */
/*  if ((rt_loopback_type == INTERNAL_LOOP) || (rt_loopback_type == EXTERNAL_LOOP)) */
/*      bdp |= 0x800;                                                               */
/*  else                                                                            */
/*      bdp &= ~0x800;                                                              */
/*  OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));                        */
/*                                                                                  */
/*  #if (DEBUG_LANCE)                                                               */
/*      DEBUG_ERROR("set NOUFLOW BCR18: =", EBS_INT1, bdp, 0);                      */
/*  #endif                                                                          */

/* Disable Autonegotiation, set MII internal loop switch for EXTERNAL_LOOP   */
/* Force 10 mbps and Full Duplex for EXTERNAL_LOOP,INTERNAL_LOOP             */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR32));
    if ((rt_loopback_type == INTERNAL_LOOP) || (rt_loopback_type == EXTERNAL_LOOP))
    {
        bdp =   RTLANCE_M_BCR32_DANAS;
        if (rt_loopback_type == INTERNAL_LOOP)
            bdp |=  RTLANCE_M_BCR32_MIIILP | RTLANCE_M_BCR32_XPHYFD;
    }
    else
        bdp = RTLANCE_M_BCR32_XPHYANE;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
#    if (DEBUG_LANCE)
        DEBUG_ERROR("set DANAS,MIIILP BCR32: =", EBS_INT1, bdp, 0);
#    endif

/* Set PHY gpreg0 for loop/noloop, auto neg, full duplex   */
    if (rt_loopback_type == EXTERNAL_LOOP)
    {
        bdp = RTLANCE_M_MII_LOOP | RTLANCE_M_MII_FD;
        rtlance_write_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, bdp);
    }

#    if (DEBUG_LANCE)
        DEBUG_ERROR("set MII: GPR0=", EBS_INT1, bdp, 0);
#    endif

/* Delay at least 1msec while mode changes   */
    ks_sleep((word)(1+(ks_ticks_p_sec()/1000)));

/* Read back so controller can see the configuration and match   */
    rtlance_read_MIIreg(p_priv, RTLANCE_M_MII_CNTRL, &bdp);

/* Read status so controller can see link status. Read twice to work!   */
    rtlance_read_MIIreg(p_priv, RTLANCE_M_MII_STAT, &bdp);
    rtlance_read_MIIreg(p_priv, RTLANCE_M_MII_STAT, &bdp);

#    if (DEBUG_LANCE)
        DEBUG_ERROR("read MII status: =", EBS_INT1, bdp, 0);
#    endif

/* If returning from setting NO_LOOP or INTERNAL mode,      */
/* the controller is in autodetect. In EXTERNAL_LOOP we are */
/* in 10baset, Full Dup.                                    */
    if(rt_loopback_type == NO_LOOP)
     rt_set_phy_speed=rt_actual_phy_speed= PHY_AUTO;
    else
     rt_set_phy_speed=rt_actual_phy_speed= PHY_10BT_FD;

/* Reinit   */
     rtlance_reinit(p_priv);


    return;
}

/* *********************************************************************   */
/* PCI INITIALIZATION                                                      */
/* *********************************************************************   */
/*                                                                         */
/* *********************************************************************   */
/* rtlance_pci_init                                                        */
/*                                                                         */
/* This routine configures the PCI registers.                              */
/*                                                                         */
/* Input:  pointer to the RTIP interface structure                         */
/*         pointer to the LANCE private data structure                     */
/*                                                                         */
/* OUTPUT: TRUE, if success                                                */
/*         FALSE, if failure in accessing a register                       */
/*                                                                         */

RTIP_BOOLEAN rtlance_pci_init(PIFACE pi, PRTLANCE_PRIV p_priv)
{
    unsigned char return_code;
    unsigned char BusNum;
    unsigned char DevFncNum;
    unsigned char default_irq;
    unsigned char byte_read;
    unsigned short word_read;
    int Index;
    struct four_bytes 
    {
        unsigned short  wordl;
        unsigned short  wordh;
    };
    union 
    {
        struct four_bytes two_words;
        unsigned long   cat_words;
    } d_word;

    if (rtpci_bios_present())
    {
        /*                                                                       */
        /*  Find and initialize the first specified (Vendor, Device)PCI device . */
        /*  Since auto-negotiation (DANAS (bit 10, BCR32 == 0) and               */
        /*  ASEL (bit 1, BCR2 == 1)) is the default, it is assumed               */
        /*  that sw need do nothing to force auto-negotiation to occur.          */
        /*  NOTE:  Add auto negotiation based on 3/1/99 talk with Nortel. VK     */
        /*                                                                       */
        /*  The index in the Find PCI Device indicates the instance of the       */
        /*  device to search for.                                                */
        /*  The minor device number should indicate the instance of the device,  */
        /*    therefore index is set equal to minor device number.               */
        /*                                                                       */
#        if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_pci_init: PCI Index. minor_number=", EBS_INT1, pi->minor_number, 0);
#        endif
        Index = pi->minor_number;
#        if (DEBUG_LANCE)
        DEBUG_ERROR("rtlance_pci_init: PCI Index. Index =", EBS_INT1, Index, 0);
#        endif

        return_code = rtpci_find_device(RTPCI_D_ID_AM79972, RTPCI_V_ID_AMD,
                                        Index, &BusNum, &DevFncNum);
        if (return_code == RTPCI_K_SUCCESSFUL)
        {
            /*                                                              */
            /*  Set the interrupt line based on the value in the            */
            /*  PCI Interrupt Line Register.                                */
            /*  Note:  This writes a byte into an int location.  Any issues */
            /*         here?                                                */
            /*                                                              */
#            if (DEBUG_LANCE)
            DEBUG_ERROR("rtlance_pci_init: PCI Device found", 0, 0, 0);
#            endif
            return_code = rtpci_read_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE, &byte_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
#            if (DEBUG_LANCE)
                DEBUG_ERROR("rtlance_pci_init: Reading IntLine Reg. byteread=", EBS_INT1, byte_read, 0);
#            endif
                if (byte_read == RTPCI_INT_LINE_NOVAL)
                {
                    /* set the default interrupt register based on either the   */
                    /* user input value (for demo programs) or                  */
                    /* the default value set in xnconf.h.                       */
#if (RTIP_VERSION >= 30)
                    if (pi->irq_val != -1)
                        default_irq = (unsigned char)pi->irq_val;

#else
                    if (ed_irq_val != -1)
                        default_irq = ed_irq_val;
#endif
                    else
                        default_irq = CFG_AMD_PCI_IRQ;

#                    if (DEBUG_LANCE)
                        DEBUG_ERROR("rtlance_pci_init: default_irq = ", EBS_INT1, default_irq, 0);
#                    endif
                    return_code = rtpci_write_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE, default_irq);
                    if (return_code == RTPCI_K_SUCCESSFUL)
                        p_priv->irq = default_irq;
                    else
                        return(FALSE);
                }
                else
                    p_priv->irq = byte_read;
            }
            else
                return(FALSE);  /* INTERRUPT LINE Register read failed */

            /*                                                                           */
            /*  Let the PCI Latency Timer Register default.                              */
            /*                                                                           */
            /*                                                                           */
            /*  Read the I/O Base Register or the Memory Mapped I/O                      */
            /*  Register and store the address as the base address of the device.        */
            /*  This is a double word (32-bit register).  I cannot access it as such     */
            /*  on a 16-bit system in REAL MODE.  Hence, I am doing two word reads.      */
            /*  The 5 low bits of the register are not address bits, therefore I am      */
            /*  masking the register with the value 0xFFFF FFE0 (defined in PCI.H).      */
            /*  For PCI devices, bits 0-7 address the max 256 bytes that the device      */
            /*  can request, bits 8&9 MBZ as these bits indicate ISA devices,            */
            /*  at least one of bits 12-15 must be 1.                                    */
            /*  Note that I am assuming that the system BIOS  is assigning the I/O space */
            /*  system resource (see pg 755 of PCI HW & SW Architecture and Design).     */
            /*                                                                           */
            return_code = rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_IOBASE, 
                                          &d_word.two_words.wordl);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                if (d_word.two_words.wordl == RTPCI_IOBASE_NOVAL)
                {
#                if (DEBUG_LANCE)
                    DEBUG_ERROR("rtlance_pci_init: Writing default I/O base value", 0, 0, 0);
#                endif
                    /* if no address is present in the I/O base register,   */
                    /* set the default I/O base address based on the        */
                    /* user input i/o address (in demo/test programs)       */
                    /* or the default value from xnconf.h.                  */
#if (RTIP_VERSION >= 30)
                    if (pi->io_address != 0)
                        d_word.two_words.wordl = pi->io_address;
#else
                    if (ed_io_address != 0)
                        d_word.two_words.wordl = ed_io_address;
                    else
                        d_word.two_words.wordl = CFG_AMD_PCI_IOBASE;
#endif
                    return_code = rtpci_write_word(BusNum, DevFncNum, 
                                                   RTPCI_REG_IOBASE, 
                                                   d_word.two_words.wordl);
                    if (return_code == RTPCI_K_SUCCESSFUL)
                        p_priv->base_addr = 
                            (IOADDRESS)(dword)d_word.two_words.wordl;
                    else
                        return(FALSE);  /* I/O BASE Register Write Failed */
                }
                else
                    p_priv->base_addr = (IOADDRESS)(dword)
                        (d_word.two_words.wordl & RTPCI_M_IOBASE_L);

#                if (DEBUG_LANCE)
                    DEBUG_ERROR("rtlancepci_init: lance_init. wordl=", EBS_INT1, d_word.two_words.wordl, 0);
#                endif

                return_code = rtpci_read_word(BusNum, DevFncNum, 
                                              RTPCI_REG_IOBASE+2, 
                                              &d_word.two_words.wordh);
                if (return_code == RTPCI_K_SUCCESSFUL)
                {
#                if (DEBUG_LANCE)
                    DEBUG_ERROR("rtlance_pci_init. wordh=", EBS_INT1, d_word.two_words.wordh, 0);
                    DEBUG_ERROR("rtlance_pci_init. base_addr=", EBS_INT1, p_priv->base_addr, 0);
#                endif
                }
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /*  IOBASE Register read failed */

            /*                                                    */
            /*  Write PCI Command Register enabling Bus Mastering */
            /*  (BMEM) and enabling I/O accesses (IOEN).          */
            /*                                                    */
            return_code = rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_CMD, 
                                          &word_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                word_read |= RTPCI_M_CMD_IOEN | RTPCI_M_CMD_BMEM;
                return_code = rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_CMD, word_read);
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /* COMMAND Register read/write failed */
        }
        else
            return(FALSE);      /* No PCI Device detected. */
    }
    else
        return(FALSE);      /* No PCI BIOS present. */

    return(TRUE);   /* PCI Device Successfully initialized */
}
#endif /* (CFG_AMD_PCI) */

#if (CFG_AMD_32BIT)
/* *********************************************************************      */
/*                                                                            */
/*  rtlance_invoke_32bit                                                      */
/*                                                                            */
/*  This routine performs a write to the RDP, in this case writing zeros to   */
/*  CSR0, in order to invoke DWord I/O Mode.  It is assumed that RAP points   */
/*  to CSR0.                                                                  */
/*  BCR20:SWSTYLE is set to 2.  (If it is decided to use burst mode transfers */
/*  for ring accesses, SWSTYLE must be set to 3, the ring descriptor entries  */
/*  must be updated in rtlance.c, and the PCI Latency Register must be set.)  */
/*                                                                            */
/*  Input:  none                                                              */
/*                                                                            */
/*  Output: none                                                              */
/*                                                                            */
void rtlance_invoke_32bit(PRTLANCE_PRIV p_priv)
{
    dword   bdp;

    /* Write to RDP CSR0 to invoke DWIO mode   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RDP, SWAPIF(RTLANCE_M_CSR0_ZEROS));

    /* Set BCR20:SWSTYLE to 2   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR20));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);
    bdp &= RTLANCE_M_DWIO_READMASK;
    bdp |= RTLANCE_M_BCR20_SWSTYLE2;
    OUTWORD((p_priv->base_addr)+RTLANCE_K_BDP, SWAPIF(bdp));
    /* Verify that the controller is in 32 bit mode   */
    OUTWORD((p_priv->base_addr)+RTLANCE_K_RAP, SWAPIF(RTLANCE_K_BCR18));
    bdp = XINWORD((p_priv->base_addr)+RTLANCE_K_BDP);

    if ((bdp & 0x80) == 0)
    {
        DEBUG_ERROR("rtlance_open: not in 32-bit mode", NOVAR, 0, 0);
    }
    return;
}
#endif /* (CFG_AMD_32BIT) */
#endif /*CFG_AMD_PCI */

/* *******************************************************************    */
/* Swapping routines used for endian conversions                          */
/* *******************************************************************    */
dword rtlance_longswap(dword l)          /* change 4 bytes in long word 0,1,2,3 -> 3,2,1,0 */
{
    /* return((byte 3 to 0) OR (byte 0 to 3) OR (byte 2 to 1) OR   */
    /*        (byte 1 to 2)                                        */
        l = (l >> 24) | (l << 24) | ((l & 0x00ff0000ul) >> 8) |
                                    ((l & 0x0000ff00ul) << 8);
    return(l);
}

/* *********************************************************************   */
#if (PPC603)
void sync()
{
asm(" isync ");
asm(" eieio ");
asm(" sync ");
}
#else
#define sync()
#endif
/* lance register dump routines   */

#if (CFG_AMD_PCI)

#if (RT_NOT_USED)
/* *********************************************************************   */
word rtlance_dump_csr(PIFACE pi, word reg, word suspend)
{
#if (CFG_AMD_32BIT)
    dword  csrtmp,temp1;
#else
    word  csrtmp,temp1;
#endif

    PRTLANCE_PRIV   p_priv;
    word temp2;

    p_priv = iface_to_priv(pi);
    if (!p_priv)
        return(-1);
    if (suspend)
        rtlance_spnd(p_priv);   /* put the device in a suspend state */

    temp1=reg;
    OUTWORD ((p_priv->base_addr) + RTLANCE_K_RAP, SWAPIF(temp1));
    csrtmp = SWAPIF(INWORD((p_priv->base_addr)+RTLANCE_K_RDP));
    temp2 = csrtmp;


    if (suspend)
        rtlance_restart(p_priv);    /* get the device going again */

    return (temp2);
}
#endif

#if (RT_NOT_USED)
/* *********************************************************************   */
word rtlance_dump_bcr(PIFACE pi, word reg, word suspend)
{
#if (CFG_AMD_32BIT)
    dword  bcrtmp,temp1;
#else
    word  bcrtmp,temp1;
#endif

    PRTLANCE_PRIV   p_priv;
    word temp2;

    p_priv = iface_to_priv(pi);
    if (!p_priv)
        return(-1);
    if (suspend)
        rtlance_spnd(p_priv);   /* put the device in a suspend state */

    temp1=reg;
    OUTWORD ((p_priv->base_addr) + RTLANCE_K_RAP, SWAPIF(temp1));
    bcrtmp = SWAPIF(INWORD((p_priv->base_addr)+RTLANCE_K_BDP));
    temp2 = bcrtmp;


    if (suspend)
        rtlance_restart(p_priv);    /* get the device going again */

    return (temp2);
}
#endif

/* *********************************************************************   */
word rtlance_dump_anr(PRTLANCE_PRIV p_priv, word reg, word suspend)
{
#if (CFG_AMD_32BIT)
    dword  anrtmp,temp1;
#else
    word  anrtmp,temp1;
#endif
    word temp2;

    if (suspend)
        rtlance_spnd(p_priv);   /* put the device in a suspend state */

    temp1 = reg;
    rtlance_read_MIIreg(p_priv, temp1, &anrtmp);

    temp2 = (word)anrtmp;

    if (suspend)
        rtlance_restart(p_priv);    /* get the device going again */

    return (temp2);
}
#endif /* (!CFG_AMD_PCI) */

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_rtlance(int minor_number)
{
    return(xn_device_table_add(rtlance_device.device_id,
                        minor_number,
                        rtlance_device.iface_type,
                        rtlance_device.device_name,
                        SNMP_DEVICE_INFO(rtlance_device.media_mib,
                                         rtlance_device.speed)
                        (DEV_OPEN)rtlance_device.open,
                        (DEV_CLOSE)rtlance_device.close,
                        (DEV_XMIT)rtlance_device.xmit,
                        (DEV_XMIT_DONE)rtlance_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)rtlance_device.proc_interrupts,
                        (DEV_STATS)rtlance_device.statistics,
                        (DEV_SETMCAST)rtlance_device.setmcast));
}

#endif /* !DECLARING_DATA */
#endif /* INCLUDE_LANCE */


