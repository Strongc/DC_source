/*                                                                                                     */
/* R8139.c                                                                                             */
/*                                                                                                     */
/*   EBS - RTIP                                                                                        */
/*                                                                                                     */
/*   Copyright EBSnet, Inc., 1999                                                                      */
/*   All rights reserved.                                                                              */
/*   This code may not be redistributed in source or linkable object form                              */
/*   without the consent of its author.                                                                */
/*                                                                                                     */
/*                                                                                                     */
/*   Module description:                                                                               */
/*      This device driver controls the Realtek 8139 ethernet controller,                              */
/*      a PCI 10/100 base T device with integrated PHY.                                                */
/*      Developed and tested with the Accton Cheetah PCI adapter,                                      */
/*      part # EN1207D-TX/WOL and SMC1211TX PCI card.                                                  */
/*                                                                                                     */
/*                                                                                                     */
/*                                                                                                     */
/*                                                                                                     */
/*                                                                                                     */
/*       Data Structure Definition:                                                                    */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"
#include "pci.h"

#if (RTIP_VERSION < 30)
#define ETHERSIZE CFG_ETHERSIZE
#endif

#define DISABLE_TX 0

#if (INCLUDE_R8139)

#define DEBUG_R8139 0
#define DEBUG_R8139_XMIT_QUE 0

/* ********************************************************************   */
#if (PPC603)
void R8139_sync();
#define SWAPIF32(X) longswap(X)
#define SWAPIF16(X) wordswap(X)
#define R8139_INWORD(ADDR)    (*(volatile short *)(ADDR))
#define R8139_OUTWORD(ADDR,VAL) (*(volatile short *)(ADDR) = (VAL)); sync();
#define R8139_INDWORD(ADDR)    (*(volatile long *)(ADDR))
#define R8139_OUTDWORD(ADDR,VAL) (*(volatile long *)(ADDR) = (VAL)); sync();
#define R8139_INBYTE(ADDR)    (*(volatile char *)(ADDR))
#define R8139_OUTBYTE(ADDR,VAL) (*(volatile char *)(ADDR) = (VAL)); sync();
#else
#define SWAPIF32(X) (X)
#define SWAPIF16(X) (X)
#define R8139_OUTWORD(ADDR, VAL) OUTWORD((ADDR), (VAL))
#define R8139_INWORD(ADDR) INWORD((ADDR))
#define R8139_OUTDWORD(ADDR, VAL) OUTDWORD((ADDR), (VAL))
#define R8139_INDWORD(ADDR) INDWORD((ADDR))
#define R8139_OUTBYTE(ADDR, VAL) OUTBYTE((ADDR), (VAL))
#define R8139_INBYTE(ADDR) INBYTE((ADDR))
#endif

/* Offsets to I/O registers.   */
#define IDR0          0x00  /* Ethernet hardware address. 4 byte access only*/
#define MAR0          0x08  /* Multicast filter. 4 byte access only*/

#define TXSD0         0x10  /* Transmit status of descriptor. */
#define TXADD0        0x20  /* Tx data */

#define RBSTART       0x30
#define RxEarlyCnt    0x34
#define RxEarlyStatus 0x36
#define CR_COMMAND    0x37  /* Command Register */
#define RXBUFPTR      0x38
#define RXBUFADDR     0x3A
#define IMR           0x3C
#define ISR           0x3E
#define TCR           0x40
#define RCR           0x44
#define Timer         0x48                  /* A general-purpose counter. */
#define RXMISS        0x4C              /* 24 bits valid, write clears. */
#define Cfg9346       0x50
#define CONFIG0       0x51
#define CONFIG1       0x52
#define FlashReg      0x54
#define GPPinData     0x58
#define GPPinDir      0x59
#define MII_SMI       0x5A
#define HltClk        0x5B
#define MultiIntr     0x5C
#define TxSummary     0x60
#define MII_CR        0x62
#define MII_SR        0x64
#define MII_ANAD      0x66
#define MII_ANPA      0x68
#define NWayExpansion 0x6A
/* Undocumented registers, but required for proper operation.   */
#define FIFOTMS       0x70  /* FIFO Test Mode Select */
#define CSCR          0x74  /* Chip Status and Configuration Register. */
#define PARA78        0x78
#define PARA7c        0x7c  /* Magic transceiver parameter register. */

/* Command Register Bit Assignments   */
#define CR_RESET  0x10
#define CR_RXENA  0x08
#define CR_TXENA  0x04      /* transmit enable */
#define CR_RXBUFE 0x01

/* Interrupt register bits.   */
#undef INT_SYSERR
#define INT_SYSERR  0x8000
#define INT_TIMEOUT 0x4000
#define INT_CABLE   0x2000
#define INT_RXFIFO  0x0040
#define INT_RXUNDER 0x0020
#define INT_RXOVER  0x0010
#define INT_TXERR   0x0008
#define INT_TX      0x0004
#define INT_RXERR   0x0002
#undef INT_RX
#define INT_RX      0x0001
#define INT_ALL     0xE07F
#define INT_ALLRX   0x0073
#define INT_ALLTX   0x000C
#define INT_ALLERR  0xE000


/* Transmit Status register bits.   */
#define TXSTAT_CRS   0x80000000l /* Carrier sense lost */
#define TXSTAT_ABT   0x40000000l /* Transmit Aborted */
#define TXSTAT_OWC   0x20000000l /* Transmit out of window collision */
#define TXSTAT_OK    0x00008000l /* Successful transmit */
#define TXSTAT_UNDER 0x00004000l /* Transmit FIFO underrun */
#define TXSTAT_OWN   0x00002000l /* 1=cpu owns, 0=dma owns */

/* Mode Bits in RCR ... receiver configuration register   */
#define RX_MODE_BROADCAST   0x08
#define RX_MODE_MULTICAST   0x04
#define RX_MODE_UNICAST     0x02
#define RX_MODE_PROMISCUOUS 0x01

/* receive packet status bits   */
#define RXSTAT_MAR  0x8000 /* multicast address received */
#define RXSTAT_PAM  0x4000 /* physical address match */
#define RXSTAT_BAR  0x2000 /* broadcast address received */
#define RXSTAT_ISE  0x0020 /*invalid symbol error (100 bt only) */
#define RXSTAT_RUNT 0x0010 /* packet too short */
#define RXSTAT_LONG 0x0008 /* packet too long */
#define RXSTAT_CRC  0x0004 /* CRC error */
#define RXSTAT_FAE  0x0002 /* frame alignment error */
#define RXSTAT_ROK  0x0001 /* receive OK */

/* PHY mode control register bits for MII_CR  (0x62)  */
#undef PHY_DEFAULT
#define PHY_DEFAULT  1        /* When set, accept defaults, else use settings */
#define PHY_AUTOM    0x1000 /* when bit is set, phy mode is autonegotiate */
#define PHY_SPEED    0x2000 /* when bit is set and not in auto, phy speed=100Mbps */
#define PHY_DUPLEX   0x0100 /* when bit is set and not in auto, phy duplex=full*/
#define PHY_SETTINGS 0x0000   /* 0=10bt, h-duplex */

/* Twister tuning parameters from RealTek.
   Completely undocumented, but required to tune bad links. */
/* PCI Tuning Parameters
   Threshold is bytes transferred to chip before transmission starts. */
#define TX_FIFO_THRESH 256  /* In bytes, rounded down to 32 byte units. */

/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024.   */
#define RX_FIFO_THRESH  4       /* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST    4       /* Maximum PCI burst, '4' is 256 bytes */
#define TX_DMA_BURST    4       /* Calculate as 16<<val. */

#undef TX_RING_SIZE
#define TX_RING_SIZE    4  /* Must b power 2 */
                           /* we don't need many since we send only one at a time       */
#define TX_RING_MASK    (TX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */

/* ********************************************************************   */
/* PCI Device Identification information.                                 */
struct pci_id_info_r8139
{
    const char *dev_name;
    word    vendor_id;
    word    device_id;
};

typedef struct _r8139_softc
{
   PIFACE  iface;
   int device_index;
   IOADDRESS  ia_iobase;
   int     ia_irq;
   word cur_rx;     /* Index into the Rx buffer of next Rx pkt. */
   word mii_cr;
   word mii_sr;
   word mii_anad;
   int last_tx_done;       /* last tx entry with processing complete */
   int this_tx;            /* next tx to use */
#if (INCLUDE_XMIT_QUE)
   int first_tx;            /* first tx to use when xfering multiple packets*/
   int num_tx;              /* number of transmits in block of transfers */
#endif
   DCU tx_dcus[TX_RING_SIZE];       /* device supports 4 tx descriptors */
   PFBYTE pr8139rxbuf;
   dword  tx_flag;

   PFBYTE rx_buf;       /* in memory receive buffer */
   dword  rx_index;      /* index into the receive buffer where the next packet starts */
   dword  rx_config;     /* receive configuration, includes mode */
   dword  rx_mcast[2];   /* receive multicast filter */

   /* Operating Mode:   */

   /* interrupt statistics counters   */
   dword n_int_call_total; /* total interrupt service calls (may handle multiple ints) */
   dword n_ints_total;     /* total interrupts including errors,rx, and tx */
   dword n_tx_ints_total;  /* total tx interrupts including tx error interrupts */
   dword n_rx_pkts_total;  /* total rx interrupts including rx error interrupts */
   dword n_errors_total;   /* total non rx/tx error interrupts */

   EBS_TIMER timer_info;           /* timer information  */
   dword       cur_ticks;          /* incremented every second */
   dword       last_rx_ticks;      /* saved every time a packet is received */

   struct ether_statistics stats;
} R8139_SOFTC;
typedef struct _r8139_softc KS_FAR *PR8139_SOFTC;

/* ********************************************************************   */
#define off_to_r8139_softc(X)  (X) >= CFG_NUM_R8139 ? (PR8139_SOFTC)0 : r8139softc[(X)]
#define iface_to_r8139_softc(X) (X)->minor_number >= CFG_NUM_R8139 ? (PR8139_SOFTC)0 : r8139softc[(X)->minor_number]


/* ********************************************************************   */
int r8139_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr);
void r8139_interrupt(int deviceno);
void r8139_pre_interrupt(int deviceno);
void r8139_rcv_ring(PR8139_SOFTC sc);
void r8139_getmcaf(PFBYTE mclist, int bytesmclist, PFBYTE af);

RTIP_BOOLEAN r8139_open(PIFACE pi);
RTIP_BOOLEAN r8139_statistics(PIFACE pi);
RTIP_BOOLEAN r8139_setmcast(PIFACE pi);
void r8139_close(PIFACE pi);
RTIP_BOOLEAN r8139_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
int r8139_xmit(PIFACE pi, DCU msg);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
/* Note. these are now pointers. We allocate them from the DCU
  pool since portions of them are in memory space shared by PCI/HOST */
/* R8139_SOFTC KS_FAR _r8139softc[CFG_NUM_R8139];   */
PR8139_SOFTC r8139softc[CFG_NUM_R8139];

unsigned long param[4][4]=
{
    {0x0cb39de43ul,0x0cb39ce43ul,0x0fb38de03ul,0x0cb38de43ul},
    {0x0cb39de43ul,0x0cb39ce43ul,0x0cb39ce83ul,0x0cb39ce83ul},
    {0x0cb39de43ul,0x0cb39ce43ul,0x0cb39ce83ul,0x0cb39ce83ul},
    {0x0bb39de43ul,0x0bb39ce43ul,0x0bb39ce83ul,0x0bb39ce83ul}
};


struct pci_id_info_r8139 KS_FAR pci_tab_r8139[] =
{{ "RealTek RTL8129 Fast Ethernet",RTPCI_V_ID_RTK, RTPCI_D_ID_8139},
 { "SMC1211TX EZCard 10/100 (RealTek RTL8139)",RTPCI_V_ID_SMC, RTPCI_D_ID_8139A},
 { "Accton MPX5030 (RealTek RTL8139)",RTPCI_V_ID_ACC, RTPCI_D_ID_8139A},
 { "SiS 900 (RealTek RTL8139) Fast Ethernet",RTPCI_V_ID_SIS, RTPCI_D_ID_8139B},
 { "SiS 7016 (RealTek RTL8139) Fast Ethernet",RTPCI_V_ID_SIS, RTPCI_D_ID_8139C},
 {0,},                      /* 0 terminated list. */
};

#if (!defined (RTKBCPP) )
byte KS_HUGE r8139rxbuf[CFG_R8139_RX_BUFLEN + 16]; /* add 16 for alignment */
#endif

EDEVTABLE KS_FAR r8139_device =
{
    r8139_open, r8139_close, r8139_xmit, r8139_xmit_done,
    NULLP_FUNC, r8139_statistics, r8139_setmcast,
    R8139_DEVICE, "Realtek8139", MINOR_0, ETHER_IFACE,
    SNMP_DEVICE_INFO(CFG_OID_R8139, CFG_SPEED_R8139)
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
extern EDEVTABLE KS_FAR r8139_device;
extern PR8139_SOFTC r8139softc[CFG_NUM_R8139];
extern unsigned long param[4][4];
extern struct pci_id_info_r8139 KS_FAR pci_tab_r8139[];

#if (!defined (RTKBCPP) )
extern byte KS_HUGE r8139rxbuf[CFG_R8139_RX_BUFLEN + 16]; /* add 16 for alignment */
#endif

#endif

/* ********************************************************************   */
void set_up_xmit(PR8139_SOFTC sc)
{
    R8139_OUTDWORD(sc->ia_iobase + TCR,(TX_DMA_BURST<<8)|0x03000000l);
/*    sc->tx_flag = (TX_FIFO_THRESH<<11) & 0x003f0000l;   */
}

/* ********************************************************************   */
RTIP_BOOLEAN r8139_open(PIFACE pi)
{
PR8139_SOFTC sc;
IOADDRESS io_address;
PFBYTE p;
dword l;
int i;

    /* Alloc context block                  */
    /* r8139softc[i] = &_r8139softc[i];     */
    if (!r8139softc[pi->minor_number])
    {
        p = (PFBYTE) dcu_alloc_core(sizeof(*sc)+4);
        /* make sure on 4 byte boundary first     */
        l = (dword) p;
        while (l & 0x3ul) {l++; p++;};
        r8139softc[pi->minor_number] = (PR8139_SOFTC) p;
    }

    sc = iface_to_r8139_softc(pi);
    if (!sc)
        return(FALSE);
    tc_memset(sc, 0, sizeof(*sc));

#if (INCLUDE_XMIT_QUE)
    sc->first_tx = -1;
#endif

#if (defined (RTKBCPP)) /* for power pack we allocate this */
    sc->pr8139rxbuf = (PFBYTE) ks_dpmi_alloc((word)(CFG_R8139_RX_BUFLEN + 16)); /* add 16 for alignment */
    if (!sc->pr8139rxbuf)
    {
        DEBUG_ERROR("r8139 - dpmi alloc failed", NOVAR, 0, 0);
        return(FALSE);
    }
#else
    sc->pr8139rxbuf = (PFBYTE) &r8139rxbuf[0];
#endif
    if (!sc->pr8139rxbuf)
    {
        return(FALSE);
    }
    sc->iface = pi;
    pi->driver_stats.ether_stats = (PETHER_STATS)&(sc->stats);

#if (INCLUDE_XMIT_QUE)
    pi->xmit_que_depth = TX_RING_SIZE;
#endif

    /* Find an8139 type device indexed by minor number   */
    sc->device_index = r8139_pci_init(pi, &sc->ia_irq, &sc->ia_iobase);
    if (sc->device_index<0)
        return(FALSE);
    pi->io_address = io_address = sc->ia_iobase;
    pi->irq_val    = sc->ia_irq;

    /* Reset the chip and hook the interrupt service routine while waiting     */
    ks_disable();
    R8139_OUTBYTE(io_address + CR_COMMAND,CR_RESET);
#if (RTIP_VERSION > 24)
    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)r8139_interrupt,
                      (RTIPINTFN_POINTER)r8139_pre_interrupt,
                      pi->minor_number);
#else
    ks_hook_interrupt(sc->ia_irq, (RTIPINTFN_POINTER)r8139_interrupt, 
                      pi->minor_number);
#endif

    ks_enable();

    /* Check that the chip has finished the reset.   */
    for (i = 1000; i > 0; i--)
      if ((R8139_INBYTE(io_address + CR_COMMAND) & CR_RESET) == 0)
           i=0;

    /* The EEPROM initializes the macaddr,config,msr,phy and tw registers 
       at reset. Read back the MAC address */
    for (i = 0; i < 6; i++)
    {
        pi->addr.my_hw_addr[i] = 0;
        pi->addr.my_hw_addr[i] = R8139_INBYTE(io_address + IDR0 +i);
    }

    /* Enable Tx/Rx before setting tx/rx configs   */
    R8139_OUTBYTE(io_address + CR_COMMAND,CR_RXENA|CR_TXENA);

    /* Initialise the mode to accept broadcast and unicast only.   */
    sc->rx_mcast[0] = 0;   /* receive multicast filter. 0 accepts none */
    sc->rx_mcast[1] = 0;
    R8139_OUTDWORD(io_address + MAR0, sc->rx_mcast[0]);
    R8139_OUTDWORD(io_address + MAR0 + 4, sc->rx_mcast[1]);
    sc->rx_config = (dword)((RX_FIFO_THRESH << 13) |
                            (CFG_R8139_RX_BUFLEN_IDX << 11) |
                            (RX_DMA_BURST<<8) |
                            RX_MODE_BROADCAST |
                            RX_MODE_MULTICAST |
                            RX_MODE_UNICAST);

    R8139_OUTDWORD(io_address + RCR, sc->rx_config);

    sc->tx_flag = (TX_FIFO_THRESH<<11) & 0x003f0000l;
    set_up_xmit(sc);

#if(PHY_DEFAULT)
    /* We'll accept the default speed/duplex and advertisement values to open.
       Pick up current values now for reference. */
    sc->mii_cr = R8139_INWORD(io_address + MII_CR);
    sc->mii_sr = R8139_INWORD(io_address + MII_SR);
    sc->mii_anad = R8139_INWORD(io_address + MII_ANAD);
#else
    /* We'll force the device into some configuration depending on the definition
       of PHY_SETTINGS */
    sc->mii_cr = PHY_SETTINGS;
    R8139_OUTWORD(io_address + MII_CR,sc->mii_cr);
    sc->mii_sr = R8139_INWORD(io_address + MII_SR);
    sc->mii_anad = R8139_INWORD(io_address + MII_ANAD);
#endif

    /* Point to the receive buffer. Make sure on 4 byte boundary first. 
       Buffer is allocated in rtipdata.c                 */
    p = (PFBYTE) sc->pr8139rxbuf;
    l = (dword) p;
    while (l & 0x3ul) 
    {
        l++; 
        p++;
    };
    sc->rx_buf = (PFBYTE) p;
    R8139_OUTDWORD(io_address + RBSTART,kvtop(p));
    R8139_OUTDWORD(io_address + RXMISS,0l);     /*clear missed packet counter */

    /* Enable all known interrupts by setting the interrupt mask.   */
    R8139_OUTWORD(io_address + IMR,INT_ALL);

    /* Set up a timer to run every three seconds       */

    sc->cur_ticks = sc->last_rx_ticks = 0; /* watchdog for hung receiver */
    return(TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN r8139_statistics(PIFACE pi)                       /*__fn__*/
{
PETHER_STATS p;
PR8139_SOFTC sc;

    sc = iface_to_r8139_softc(pi);
    if (!sc)
        return(FALSE);

   p = (PETHER_STATS) &(sc->stats);
   UPDATE_SET_INFO(pi,interface_packets_in, p->packets_in)
   UPDATE_SET_INFO(pi,interface_packets_out, p->packets_out)
   UPDATE_SET_INFO(pi,interface_bytes_in, p->bytes_in)
   UPDATE_SET_INFO(pi,interface_bytes_out, p->bytes_out)
   UPDATE_SET_INFO(pi,interface_errors_in, p->errors_in)
   UPDATE_SET_INFO(pi,interface_errors_out, p->errors_out)
   UPDATE_SET_INFO(pi,interface_packets_lost, p->packets_lost)
   return(TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN r8139_setmcast(PIFACE pi)      /* __fn__ */
{
PR8139_SOFTC sc;
IOADDRESS io_address;
    sc = iface_to_r8139_softc(pi);
    if (!sc)
        return(FALSE);
    io_address = sc->ia_iobase;
    /* Set multicast filter on chip.               */
    /* If none needed lenmclist will be zero       */
    r8139_getmcaf((PFBYTE) pi->mcast.mclist, pi->mcast.lenmclist,
                (PFBYTE) &sc->rx_mcast[0]);
    R8139_OUTDWORD(io_address + MAR0, sc->rx_mcast[0]);
    R8139_OUTDWORD(io_address + MAR0 + 4, sc->rx_mcast[1]);
    return(TRUE);
}

/* ********************************************************************   */
void r8139_close(PIFACE pi)                     /*__fn__*/
{
PR8139_SOFTC sc;
IOADDRESS io_address;

    sc = iface_to_r8139_softc(pi);
    if (!sc)
        return;
    io_address = sc->ia_iobase;

    /* Disable all interrupts by clearing the interrupt mask.   */
    R8139_OUTWORD(io_address + IMR,0);
    /* Disable Tx/Rx   */
    R8139_OUTBYTE(io_address + CR_COMMAND,0);

}


/* ********************************************************************   */
RTIP_BOOLEAN r8139_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PR8139_SOFTC sc;
IOADDRESS io_address;

#if (DEBUG_R8139_XMIT_QUE)
    DEBUG_ERROR("rs189_xmit_done: ", DINT2, msg, 0);
#endif
    sc = iface_to_r8139_softc(pi);
    if (!sc)
        return(FALSE);

    if (success)
    {
        /* Update total number of successfully transmitted packets.       */
        sc->stats.packets_out++;
        sc->stats.bytes_out += DCUTOPACKET(msg)->length;
    }
    else
    {
    PFBYTE p;
    dword l;
    int i;
      /* We're in serious troble here. Reset and reinitialize the chip   */
      /* error - record statistics                                       */
      sc->stats.errors_out++;
      sc->stats.tx_other_errors++;
      io_address = sc->ia_iobase;
      ks_disable();
      R8139_OUTBYTE(io_address + CR_COMMAND,CR_RESET);

      /* Check that the chip has finished the reset.   */
      for (i = 1000; i > 0; i--)
        if ((R8139_INBYTE(io_address + CR_COMMAND) & CR_RESET) == 0)
           i=0;

      /* Enable Tx/Rx before setting tx/rx configs   */
#if (INCLUDE_XMIT_QUE)
      R8139_OUTBYTE(io_address + CR_COMMAND,CR_RXENA);
#else
      R8139_OUTBYTE(io_address + CR_COMMAND,CR_RXENA|CR_TXENA);
#endif
      R8139_OUTDWORD(io_address + MAR0, sc->rx_mcast[0]);
      R8139_OUTDWORD(io_address + MAR0 + 4, sc->rx_mcast[1]);
      R8139_OUTDWORD(io_address + RCR,sc->rx_config);

      R8139_OUTDWORD(io_address + TCR,(TX_DMA_BURST<<8)|0x03000000ul);

      sc->rx_index = 0;
      /* We'll accept the default speed/duplex and advertisement values to re-open.
         Pickurrent values now for reference. */
      sc->mii_cr = R8139_INWORD(io_address + MII_CR);
      sc->mii_sr = R8139_INWORD(io_address + MII_SR);
      sc->mii_anad = R8139_INWORD(io_address + MII_ANAD);

      /* Point to the receive buffer. Make sure on 4 byte boundary first. Buffer is
       allocated in rtipdata.c                 */
      p = (PFBYTE) sc->pr8139rxbuf;
      l = (dword) p;
      while (l & 0x3ul) 
      {
        l++; 
        p++;
      };
      sc->rx_buf = (PFBYTE) p;
      R8139_OUTDWORD(io_address + RBSTART,kvtop(p));
      R8139_OUTDWORD(io_address + RXMISS,0l);

/*set_rx_mode(dev);     may need this                                                      */
/*outb(CmdRxEnb | CmdTxEnb, ioaddr + ChipCmd); was already done unless reset by rx_mode    */

      /* Enable all known interrupts by setting the interrupt mask.   */
      R8139_OUTWORD(io_address + IMR,INT_ALL);
      ks_enable();
    }
    return(TRUE);
}

/* ********************************************************************   */
/* Transmit Routine. The 8139 device requires that the packet data is 32 bit aligned.
   It is assumed here that the DCU's were aligned, no checking will be done here. */
int r8139_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
IOADDRESS io_address;
PR8139_SOFTC sc;
int   length;
int this_tx;
#if (INCLUDE_XMIT_QUE)
int nxt_tx;
#endif
dword status;

    sc = iface_to_r8139_softc(pi);
    if (!sc)
        return(ENUMDEVICE);

    io_address = sc->ia_iobase;

    if (msg)
    {
        length = DCUTOPACKET(msg)->length;
        if (length < ETHER_MIN_LEN)
            length = ETHER_MIN_LEN;
        if (length > ETHERSIZE)
        {
            DEBUG_ERROR("xmit - length is too large", NOVAR, 0, 0);
            length = ETHERSIZE;         /* what a terriable hack! */
        }

#if (INCLUDE_XMIT_QUE)
        /* if we are starting a new block of transfers save the first one   */
        /* disable transmitter                                              */
        if (sc->first_tx == -1)
        {
#if (DISABLE_TX)
            R8139_OUTBYTE(io_address + CR_COMMAND,CR_RXENA);
#endif
            sc->first_tx = this_tx;
        }
        sc->num_tx++;
#endif

        /* get current discriptor and update descriptor pointer   */
        this_tx = sc->this_tx++;
        this_tx &= TX_RING_MASK;  /* Wrap to zero if must */
        sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */

        sc->tx_dcus[this_tx] = msg;

        /* check that we own the descriptor otherwise return error  */
        status = R8139_INDWORD(io_address + TXSD0 + (this_tx<<2));
        if (!(status & TXSTAT_OWN))
        {
            DEBUG_ERROR("TX Descriptor not owned ", DINT2 , this_tx, status);
            return(EOUTPUTFULL);
        }

        /* write the packet address to the descriptor   */
        R8139_OUTDWORD(io_address + TXADD0 +(this_tx<<2), 
                       kvtop((PFBYTE)DCUTODATA(msg)));

        /* write the length to the descriptor   */
        R8139_OUTDWORD(io_address + TXSD0 +(this_tx<<2), 
                       sc->tx_flag | length);


#if (DEBUG_R8139_XMIT_QUE)
        DEBUG_ERROR("rs189_xmit: msg, this_tx", DINT2, msg, this_tx);
#endif

/*    */
#if (INCLUDE_XMIT_QUE)
        /* get next descriptor (already calculated above)   */
        nxt_tx = sc->this_tx; + 1;

        /* see if we can fit any more packets in the ring buffer   */
        status = R8139_INDWORD(io_address + TXSD0 + (nxt_tx<<2));
        if ( (status & TXSTAT_OWN) && (sc->num_tx <= TX_RING_SIZE) )
        {
#if (DEBUG_R8139_XMIT_QUE)
            DEBUG_ERROR("r8139_xmit: we can queue more: nxt_tx", EBS_INT1,
                nxt_tx, 0);
#endif
            return(REPORT_XMIT_MORE);
        }
#if (DEBUG_R8139_XMIT_QUE)
        else
        {
            DEBUG_ERROR("r8139_xmit: we can NOT queue more: nxt_tx", EBS_INT1,
                nxt_tx, 0);
        }
#endif
#endif      /* INCLUDE_XMIT_QUE */
    }       /* if msg */

    /* start xmit   */
#if (DEBUG_R8139_XMIT_QUE)
    DEBUG_ERROR("r8139_xmit: start xmit", NOVAR, 0, 0);
#endif

#if (INCLUDE_XMIT_QUE)
    sc->first_tx = -1;

#    if (DISABLE_TX)
        /* Enable Tx/Rx before setting tx configs   */
        R8139_OUTBYTE(io_address + CR_COMMAND,CR_RXENA|CR_TXENA);
        set_up_xmit(sc);
#    endif
#endif

    return(0);
}

/* ********************************************************************   */
void r8139_pre_interrupt(int deviceno)
{
PR8139_SOFTC sc;

    sc = off_to_r8139_softc(deviceno);

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq)
}

/* ********************************************************************   */
void r8139_interrupt(int deviceno)
{
IOADDRESS io_address;
PR8139_SOFTC sc;
word status;
dword tx_status;
int last_tx_done;
int i;

    sc = off_to_r8139_softc(deviceno);
    if (!sc)
    {
        DEBUG_ERROR("R8139 Bad arg to isr", NOVAR , 0, 0);
        goto ex_it;
    }
    sc->n_int_call_total += 1;

    io_address = sc->ia_iobase;
    for (i = 0; i < 10; i++)
    {
      /* Get the interrupt reason and acknowledge       */
      status = R8139_INWORD((io_address + ISR));
      R8139_OUTWORD((io_address+ISR),(status & INT_ALL));
/* DEBUG_ERROR("R8139 INT status == ", DINT1 , (dword) status, 0);   */

      if ((status & INT_ALL) == 0)
            break;

      sc->n_ints_total += 1;

      if (status & INT_ALLRX)     /* Packet received or RX error */
      {
/* DEBUG_ERROR("R8139 RX", NOVAR , 0, 0);   */
        sc->n_rx_pkts_total += 1;
        if (status & INT_RXFIFO)
        {
            sc->stats.rx_fifo_errors++;
        }
        else if (status & (INT_RXUNDER | INT_RXOVER | INT_RXERR))
        {
           sc->stats.errors_in++;
        }
        r8139_rcv_ring(sc);
      }
      if (status & INT_ALLTX)     /* Packet sent or TX error */
      {
        sc->n_tx_ints_total += 1;
        last_tx_done = sc->last_tx_done;
        while(last_tx_done != sc->this_tx)
        {
          tx_status = R8139_INDWORD(io_address + TXSD0 + (last_tx_done<<2));
/* DEBUG_ERROR("R8139 TX last_tx_done,status == ", DINT2 , (dword)last_tx_done, tx_status);   */
          if ( !(tx_status & (TXSTAT_OK | TXSTAT_UNDER | TXSTAT_ABT)))
                 break; /* It still hasn't been Txed */

          if (tx_status & TXSTAT_OK)
          {
          dword collisions = (tx_status>>24)&0x0000000F;
            if (sc->tx_dcus[last_tx_done])
            {
/*  DEBUG_ERROR("R8139 TX invoke: collisions ==", DINT1 , (dword)collisions, 0);   */
              if(collisions == 1) 
                sc->stats.one_collision++;
              if(collisions > 1)  
                sc->stats.multiple_collisions++;

#              if (INCLUDE_XMIT_QUE)
                 sc->num_tx--;
                 if (sc->num_tx == 0)
                    ks_invoke_output(sc->iface, 1);
#              else
                 ks_invoke_output(sc->iface, 1);
#              endif
            }
          }
          else if (tx_status & TXSTAT_UNDER) /* Transmit FIFO underrun */
          {
            sc->stats.tx_fifo_errors++;
            /* Add 64 to the Tx FIFO threshold.   */
            if (sc->tx_flag <  0x00300000l) sc->tx_flag += 0x00020000l;
          }
          else if (tx_status & TXSTAT_CRS)   /* Carrier sense lost */
            sc->stats.tx_carrier_errors++;
          else if (tx_status & TXSTAT_ABT)   /* Transmit Aborted */
          {
            sc->stats.collision_errors++;
#if (!INCLUDE_XMIT_QUE)
            set_up_xmit(sc);
#endif
          }
          else if (tx_status & TXSTAT_OWC)   /* Transmit out of window collision */
            sc->stats.owc_collision++;

          last_tx_done++;
          last_tx_done &= TX_RING_MASK;  /* Wrap to zero if must */
          sc->last_tx_done = last_tx_done;
        }
      }
      if (status & INT_ALLERR)     /* A non rx or tx type error */
      {
        sc->n_errors_total += 1;
      }


    if (i == 10)
    {
        DEBUG_ERROR("R8139 >10 loops in ISR", NOVAR , 0, 0);
        /* Clear all interrupts and return       */
        R8139_OUTWORD((io_address+ISR),(INT_ALL));
        break;
    }
/*DEBUG_ERROR("R8139 n_tx_ints , n_rcv_pkts", DINT2 , sc->n_tx_ints_total, sc->n_rx_pkts_total);        */
/*DEBUG_ERROR("R8139 n_ints ", DINT1 , sc->n_ints_total, 0);                                            */

    }
ex_it:
    DRIVER_MASK_ISR_ON(sc->ia_irq)

    return;
}

/* ********************************************************************        */
/* Check status and receive data from the ring buffer (called from ISR)        */
void r8139_rcv_ring(PR8139_SOFTC sc)
{
word rx_status;
word rx_length,length;
DCU  invoke_msg;
PFBYTE rx_buf = sc->rx_buf;     /* in memory receive buffer */
dword rx_index = sc->rx_index;  /* index into the receive buffer where the next packet starts */
IOADDRESS io_address = sc->ia_iobase;

/* word rx_buf_ptr = R8139_INWORD(io_address + RXBUFPTR);                       */
/* word rx_buf_addr = R8139_INWORD(io_address + RXBUFADDR);                     */
/* DEBUG_ERROR("R8139 RCV: PTR,ADDR   ", EBS_INT2, rx_buf_ptr, rx_buf_addr);    */

    while(!(R8139_INBYTE(io_address + CR_COMMAND) & CR_RXBUFE))
    {
/* DEBUG_ERROR("R8139 RCV: index   ", EBS_INT1, (dword)rx_index, 0);   */
      rx_status = *(word *)((dword) rx_buf + rx_index);
      rx_length = *(word *)((dword) rx_buf + rx_index + 2); /* packet length not including stat/length */
/* DEBUG_ERROR("R8139 RCV: STATUS,LENGTH   ", EBS_INT2, rx_status, rx_length);   */

      if (rx_status & (RXSTAT_ISE |RXSTAT_FAE))
      {
            sc->stats.rx_frame_errors++;
      }
      else if (rx_status & RXSTAT_CRC)
      {
            sc->stats.rx_crc_errors++;
      }
      else if (rx_status & (RXSTAT_RUNT |RXSTAT_LONG))
      {
            sc->stats.rx_frame_errors++;
      }
      else /* receive a good packet here */
      {
        /* attempt to allocate a DCU to move the data into   */
        invoke_msg = os_alloc_packet_input(rx_length, DRIVER_ALLOC);
        if (!invoke_msg)
        {
            DEBUG_ERROR("r8139_rcv_ring: os_alloc_packet failed",
                NOVAR, 0, 0);
            sc->stats.packets_lost++;
        }
        else
        {
         sc->stats.packets_in++;
          sc->stats.bytes_in += (word)(rx_length - sizeof(struct _ether));
          /* if data wrapped, we need to do a two part copy   */
          if ((rx_index + rx_length + 4) > CFG_R8139_RX_BUFLEN)
          {
            length = (word)(CFG_R8139_RX_BUFLEN - rx_index -4);
            tc_movebytes((PFBYTE) DCUTODATA(invoke_msg),
                         (PFBYTE) (rx_buf + rx_index +4), length);
            tc_movebytes((PFBYTE) (DCUTODATA(invoke_msg) + length),
                         (PFBYTE) rx_buf, rx_length - length);
          }
          else
          {
            tc_movebytes((PFBYTE) DCUTODATA(invoke_msg),
                         (PFBYTE) (rx_buf + rx_index +4), rx_length);
          }

          DCUTOPACKET(invoke_msg)->length = rx_length - 4;
#if (RTIP_VERSION > 24)
          ks_invoke_input(sc->iface,invoke_msg);
#else
          os_sndx_input_list(sc->iface, invoke_msg);
          ks_invoke_input(sc->iface);
#endif
        }
      }
      rx_index += rx_length + 4 + 3; /* increment the buffer index pointer */
      rx_index &= ~3;                /* word align if necessary */
      rx_index = rx_index % CFG_R8139_RX_BUFLEN;  /* Wrap if we must */
      R8139_OUTWORD(io_address + RXBUFPTR, rx_index-16);
    }
    sc->rx_index = rx_index;
    /* done       */
}

/* =================================================================         */
/* =================================================================         */
/* pci code                                                                  */
/* =================================================================         */
/* =================================================================         */

/* Use the PCI device identification table and look for one of the devices. Return
   the table index of the first device if found or -1. When using the minor_number
   to open multiple instances of a device, the devices should have the same
   vendor/device_id signature.
 */
 int r8139_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr)
{
  byte BusNum[CFG_NUM_R8139];
  byte DevFncNum[CFG_NUM_R8139];
  byte default_irq;
  byte byte_read;
  unsigned short word_read;

  int pos_minor_num;
  int status[CFG_NUM_R8139];
  int found = 0;

  int Index = pi->minor_number;
  int device_index = 0;

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
      /* cycle through each FLAVOR of this card __st__   */
      while (pci_tab_r8139[device_index].dev_name != 0)
      {
        /* make an array of all occurences by looking for   */
        /* each possible minor number of this flavor of     */
        /* this card                               __st__   */
        for (pos_minor_num=0;
              (pos_minor_num<CFG_NUM_R8139) && (found<CFG_NUM_R8139);
               pos_minor_num++)
        {
            status[found] = rtpci_find_device(pci_tab_r8139[device_index].device_id,
                                  pci_tab_r8139[device_index].vendor_id,
                                  pos_minor_num, &BusNum[found], &DevFncNum[found]);


            /* we break if status doesn't equal 0 because      __st__   */
            /* our driver layer treats all cards of the same            */
            /* type (but different flavors) as the same (meriting       */
            /* different minor numbers), but the pci bios calls         */
            /* treat cards with different flavors as different cards    */
            /* causing each flavor of this type to start with MINOR_0   */
            if (status[found] == RTPCI_K_SUCCESSFUL)
                found++;
            else
                break;
        }

        device_index++;
      }

      /* if the number of devices found is less than the   */
      /* current minor number needed we FAIL               */
      if (found < Index)
        return (-1);




          /*                                                                    */
          /*  Set the interrupt line based on the value in the                  */
          /*  PCI Interrupt Line Register.                                      */
#if (PPC603) /* Cogent PPC603 Always uses IRQ2, See Init603.c for base address */
          *pirq = 2;
          *pbase_addr = cog603_base_address();
          rtpci_write_dword(BusNum[Index], DevFncNum[Index], RTPCI_REG_IOBASE, 0x01);
          /* set max latency timer to max                                    */
          rtpci_write_byte(BusNum[Index], DevFncNum[Index],  0x3F , 0xFF);
#else
          if(rtpci_read_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_INT_LINE, &byte_read)
                             == RTPCI_K_SUCCESSFUL)
          {
            if (byte_read == RTPCI_INT_LINE_NOVAL)
            {
              /* set the default interrupt register based on either the          */
              /* user input value (for demo programs) or                         */
              /* the default value set in xnconf.h.                              */
#if (RTIP_VERSION >= 30)
              if (pi->irq_val != -1)
                 default_irq = (byte) pi->irq_val;
#else
               if (ed_irq_val != -1)
                 default_irq = (byte) ed_irq_val;
#endif
            else
                 default_irq = CFG_R8139_PCI_IRQ;

               if(rtpci_write_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_INT_LINE,default_irq)
                                   == RTPCI_K_SUCCESSFUL)
                *pirq = default_irq;
              else
                return(-1);
            }
            else
              *pirq = (int)byte_read;
            }
          else
            return(-1);  /* INTERRUPT LINE Register read failed     */

          /*                                             */
          /*  Set PCI Latency Timer to 32 if < 32.       */
          /*                                             */
          /*                                             */
          if (rtpci_read_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_LTNCY_TMR, &byte_read)
                              != RTPCI_K_SUCCESSFUL)
            return (-1);
          if (byte_read < 32)
          {
            byte_read = 32;
            if (rtpci_write_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_LTNCY_TMR, byte_read)
                                 != RTPCI_K_SUCCESSFUL)
             return (-1);
          }

          /*  Read the I/O Base Register or the Memory Mapped I/O                             */
          /*  Register and store the address as the base address of the device.               */
          /*  This is a double word (32-bit register).  I cannot access it as such            */
          /*  on a 16-bit system in REAL MODE.  Hence, I am doing two word reads.             */
          /*  The 5 low bits of the register are not address bits, therefore I am             */
          /*  masking the register with the value 0xFFFF FFE0 (defined in PCI.H).             */
          /*  For PCI devices, bits 0-7 address the max 256 bytes that the device             */
          /*  can request, bits 8&9 MBZ as these bits indicate ISA devices,                   */
          /*  at least one of bits 12-15 must be 1.                                           */
          /*  Note that I am assuming that the system BIOS  is assigning the I/O space        */
          /*  system resource (see pg 755 of PCI HW & SW Architecture and Design).            */
          /*                                                                                  */

          if (rtpci_read_word(BusNum[Index], DevFncNum[Index], RTPCI_REG_IOBASE, &d_word.two_words.wordl)
                              == RTPCI_K_SUCCESSFUL)
          {
            if (d_word.two_words.wordl == RTPCI_IOBASE_NOVAL)
            {
              DEBUG_ERROR("r8139: Writing default I/O base value", 0, 0, 0);
              /* if no address is present in the I/O base register,         */
              /* set the default I/O base address based on the              */
              /* user input i/o address (in demo/test programs)             */
              /* or the default value from xnconf.h.                        */
#if (RTIP_VERSION >= 30)
              if (pi->io_address != 0)
                 d_word.two_words.wordl = pi->io_address;
#else
              if (ed_io_address != 0)
                 d_word.two_words.wordl = ed_io_address;
#endif
              else
                 d_word.two_words.wordl = CFG_R8139_PCI_IOBASE;

              if (rtpci_write_word(BusNum[Index], DevFncNum[Index], RTPCI_REG_IOBASE, d_word.two_words.wordl)
                                   == RTPCI_K_SUCCESSFUL)
                  *pbase_addr = (IOADDRESS) d_word.two_words.wordl;
              else
                  return(-1);  /* I/O BASE Register Write Failed      */
              }
            else
              *pbase_addr = d_word.two_words.wordl & RTPCI_M_IOBASE_L;

            if (rtpci_read_word(BusNum[Index],DevFncNum[Index],RTPCI_REG_IOBASE+2,&d_word.two_words.wordh)
                                == RTPCI_K_SUCCESSFUL)
            {
               DEBUG_ERROR("rt8139_pci_init. wordh=", EBS_INT1, d_word.two_words.wordh, 0);
               DEBUG_ERROR("rt8139_pci_init. base_addr=", EBS_INT1, *pbase_addr, 0);
            }
#endif /* #if (PPC603) */
            /*                                                           */
            /*  Write PCI Command Register enabling Bus Mastering        */
            /*  (BMEM) and enabling I/O accesses (IOEN).                 */
            /*                                                           */
            if (rtpci_read_word(BusNum[Index], DevFncNum[Index], RTPCI_REG_CMD, &word_read)
                                == RTPCI_K_SUCCESSFUL)
            {
              word_read |= RTPCI_M_CMD_IOEN | RTPCI_M_CMD_BMEM;
              if (rtpci_write_word(BusNum[Index], DevFncNum[Index], RTPCI_REG_CMD, word_read)
                                    != RTPCI_K_SUCCESSFUL)
                return(-1);          /* COMMAND Register read/write failed      */
            }
          }
          return(device_index);
    }
    else  /* if (rtpci_bios_present()) */
      return(-1);      /* No PCI BIOS present.    */
}

/* ********************************************************************   */
/* calc values for multicast                                              */
void r8139_getmcaf(PFBYTE mclist, int lenmclist, PFBYTE af)
{
int bytesmclist;
byte c;
PFBYTE cp;
dword crc;
int i, len, offset;
byte bit;
int row, col;

    bytesmclist = lenmclist * ETH_ALEN;

    /*
     * Set up multicast address filter by passing all multicast addresses
     * through a crc generator, and then using the high order 6 bits as an
     * index into the 64 bit logical address filter.  The high order bit
     * selects the word, while the rest of the bits select the bit within
     * the word.
     */

/*  if (ifp->if_flags & IFF_PROMISC) {      */
/*      ifp->if_flags |= IFF_ALLMULTI;      */
/*      af[0] = af[1] = 0xffffffff;         */
/*      return;                             */
/*  }                                       */

    /* the driver can only handle 64 addresses; if there are more than      */
    /* 64 than accept all addresses; the IP layer will filter the           */
    /* packets                                                              */
    if (lenmclist > 64)
        af[0] = af[1] = af[2] = af[3] = af[4] = af[5] = af[6] = af[7] = 0xff;
    else
    {
        af[0] = af[1] = af[2] = af[3] = af[4] = af[5] = af[6] = af[7] = 0;
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

            /* Turn on the corresponding bit in the filter.      */
    /*      af[crc >> 5] |= 1 << ((crc & 0x1f) ^ 24);            */
            row = (int)(crc/8);
            col = (int)(crc % 8);
            bit =  (byte) (1 << col);
            af[row] |= bit;
        }
    }
}


/* ********************************************************************   */
#if (PPC603)
void R8139_sync()
{
asm(" isync ");
asm(" eieio ");
asm(" sync ");
}
#else
#define R8139_sync()
#endif

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_r8139(int minor_number)
{
    return(xn_device_table_add(R8139_DEVICE,
                        minor_number,
                        r8139_device.iface_type,
                        "R8139",
                        SNMP_DEVICE_INFO(r8139_device.media_mib,
                                         r8139_device.speed)
                        (DEV_OPEN)r8139_device.open,
                        (DEV_CLOSE)r8139_device.close,
                        (DEV_XMIT)r8139_device.xmit,
                        (DEV_XMIT_DONE)r8139_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)r8139_device.proc_interrupts,
                        (DEV_STATS)r8139_device.statistics,
                        (DEV_SETMCAST)r8139_device.setmcast));
}

#endif      /* !DECLARING_DATA */
#endif      /* INCLUDE_R8139 */
