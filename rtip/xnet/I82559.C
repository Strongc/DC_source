/*                                                                              */
/* i82559.c                                                                     */
/*                                                                              */
/*   EBS - RTIP                                                                 */
/*                                                                              */
/*   Copyright EBSnet, Inc., 1999                                               */
/*   All rights reserved.                                                       */
/*   This code may not be redistributed in source or linkable object form       */
/*   without the consent of its author.                                         */
/*                                                                              */
/*                                                                              */
/*   Module description:                                                        */
/*       This device driver controls the 82558-59 ethernet controllers          */
/*      on the etherexpress pro/100 PCI NIC card                                */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*       Data Structure Definition:                                             */

#define DIAG_SECTION_KERNEL DIAG_SECTION_DRIVER

#include "sock.h"
#include "rtip.h"
#include "pci.h"

#if (INCLUDE_I82559)

#if (RTIP_VERSION < 30)
#define ETHERSIZE CFG_ETHERSIZE
#endif

#define TEST_FCP 1  /* Automatically clear link pulse */

/* ********************************************************************   */
/* DEBUG DEFINES                                                          */
/* ********************************************************************   */

#define DEBUG_I82559 0

/* ********************************************************************   */
/* DEFINES                                                                */
/* ********************************************************************   */
#define RTPCI_D_ID_82562    0x2449 
#define RTPCI_D_ID_82562ET  0x1039

#undef RX_RING_SIZE
#define RX_RING_SIZE    8  /* Must be power of 2 */
#define RX_RING_MASK    (RX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */


#define USE_FLEX_RXBD 1

#undef TX_RING_SIZE
#define TX_RING_SIZE    4  /* Must b power 2 */
                           /* we don't need many since we send only one at a time       */
#define TX_RING_MASK    (TX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */

/* When reading from the ring buffer alloc a new dcu and copy if <= this          */
/* size. Otherwise submit the DCU from the ring buffer                            */
#define RX_COPY_BREAK   120

/* When transmitting, define the number of bytes which should be in the devices              */
/* Tx fifo before transmitting can begin. This value is internally multiplied by 8           */
/* This is the initial value, if transmit underflows are detected, this value will be        */
/* dynamically adjusted up. Valid range is 1 to 0xE0.                                        */
#define TX_THRESHOLD    0x20
#define TX_TBDNUMBER  0x01000000ul  /* do not change, we transmit 1 buffer per frame */
#define TX_EOF        0x00008000ul  /* flag bit to indicate full frame in tx buffer */

/* Control/Status register offsets     */
#define SCR_STATUS      0
#define SCR_COMMAND     2
#define SCR_POINTER     4
#define SCR_PORT        8
#define SCR_FLASH       0xc
#define SCR_EEPROM      0xe
#define SCR_MDI         0x10

/* Status word - upper byte of Status word     */
#define STATUS_CXTNO    0x8000
#define STATUS_FR       0x4000
#define STATUS_CNA      0x2000
#define STATUS_RNR      0x1000
#define STATUS_MDI      0x0800
#define STATUS_SWI      0x0400
#define STATUS_RESERVED 0x0200
#define STATUS_FCP      0x0100 /* 82558/559 only */

/* Command word mask bits - upper byte of Command word     */
#define COMMAND_DONE        0x8000
#define COMMAND_RXDONE      0x4000
#define COMMAND_IDLE        0x2000
#define COMMAND_RXSUSPEND   0x1000
#define COMMAND_EARLYRX     0x0800
#define COMMAND_FLOWCTL     0x0400
#define COMMAND_TRIGINT     0x0200
#define COMMAND_MASKALL     0x0100

/* Command word CU commands - bits 7-5 (23-20 dword)     */
#define COMMAND_CUNOP         0x0000
#define COMMAND_CUSTART       0x0010
#define COMMAND_CURESUME      0x0020
#define COMMAND_CUHPQSTRT     0x0030 /* 82558/559 only */
#define COMMAND_CUSTATSADDR   0x0040
#define COMMAND_CUSTATSSHOW   0x0050
#define COMMAND_CUCMDBASE     0x0060
#define COMMAND_CUSTATSDUMP   0x0070
#define COMMAND_CUSTATICRES   0x00A0 /* 82558/559 only */
#define COMMAND_CUHPQRESUME   0x00B0 /* 82558/559 only */

/* Command word RU commands - bits 3-0 (18-16 dword)     */
#define COMMAND_RXNOP         0x0000
#define COMMAND_RXSTART       0x0001
#define COMMAND_RXRESUME      0x0002
#define COMMAND_RXABORT       0x0004
#define COMMAND_RXADDRLOAD    0x0006
#define COMMAND_RXRESNORS     0x0007

/* Port interface opcodes     */
#define COMMAND_PORT_RESET          0
#define COMMAND_PORT_SELF_TEST      1
#define COMMAND_PORT_PARTIAL_RESET  2
#define COMMAND_PORT_DUMP           3
#define COMMAND_PORT_DUMP_WAKE      7 /* 82559 only */



#define RX_COMPLETE       0x8000
#define RX_OK             0x2000
#define RX_CRC_ERROR      0x0800
#define RX_ALIGN_ERROR    0x0400
#define RX_TOOBIG_ERROR   0x0200
#define RX_DMAOVRN_ERROR  0x0100
#define RX_TOOSHORT_ERROR 0x0080
#define RX_ETH2TYPE       0x0020
#define RX_NOMATCH        0x0004
#define RX_NOIAMATCH      0x0002

#define TX_UNDERRUN     0x1000
#define TXCNOOP            0x0ul
#define TXCSETIA       0x10000ul
#define TXCCFG         0x20000ul
#define TXCMCAST       0x30000ul
#define TXCXMIT        0x40000ul
#define TXCTDR         0x50000ul
#define TXCDUMP        0x60000ul
#define TXCDIAG        0x70000ul
#define TXCSUSP     0x40000000ul
#define TXCRESUME   0xbffffffful  /* ~TXCSUSP */
#define TXCINTR     0x20000000ul
#define TXCFLEX     0x00080000ul

#define STB_TX_GOOD_FRAMES      0
#define STB_COLL16_ERRS     1
#define STB_LATE_COLLS      2
#define STB_UNDERRUNS       3
#define STB_LOST_CARRIER    4
#define STB_DEFERRED            5
#define STB_ONE_COLLS       6
#define STB_MULTI_COLLS     7
#define STB_TOTAL_COLLS     8
#define STB_RX_GOOD_FRAMES      9
#define STB_CRC_ERRS            10
#define STB_ALIGN_ERRS      11
#define STB_RESOURCE_ERRS   12
#define STB_OVERRUN_ERRS    13
#define STB_COLLS_ERRS      14
#define STB_RUNT_ERRS       15
#define STB_DONE_MARKER     16

/* PHY codes and capabilities     */
/* Read from the EEPROM, words 6 and 7 allow for 2 connected phy's. We will only
   deal with the primary phy. The phy address is the low byte of the word, and the
   PHY device ID is in the high byte bits 13-8. The upper bit indicates 10 base T
   only capability (ie-serial vs MII interface). The 82559 has an embedded
   82555 100 base tx PHY (code 0x07). If another PHY device is used, special mii
   requirements may be necessary. Our primary concern is detecting 10 base t only. */

#define TEN_BASET_ONLY 0x8000
#undef PHY_DEFAULT
#define PHY_DEFAULT    0x07 /* Use this phy address when no eeprom is found */

#if (POWERPC)
void I82559_sync();
#define SWAPIF32(X) longswap(X)
#define SWAPIF16(X) wordswap(X)
#define I82559_INWORD(ADDR)    wordswap((*(volatile short *)(ADDR)))
#define I82559_OUTWORD(ADDR,VAL) (*(volatile short *)(ADDR) = wordswap(VAL)); I82559_sync();
#define I82559_INDWORD(ADDR)    longswap((*(volatile long *)(ADDR)))
#define I82559_OUTDWORD(ADDR,VAL) (*(volatile long *)(ADDR) = longswap(VAL)); I82559_sync();
#define I82559_INBYTE(ADDR)    (*(volatile char *)(ADDR))
#define I82559_OUTBYTE(ADDR,VAL) (*(volatile char *)(ADDR) = (VAL)); I82559_sync();
#else
#define SWAPIF32(X) (X)
#define SWAPIF16(X) (X)
#define I82559_OUTWORD(ADDR, VAL) OUTWORD((ADDR), (VAL))
#define I82559_INWORD(ADDR) INWORD((ADDR))
#define I82559_OUTDWORD(ADDR, VAL) OUTDWORD((ADDR), (VAL))
#define I82559_INDWORD(ADDR) INDWORD((ADDR))
#define I82559_OUTBYTE(ADDR, VAL) OUTBYTE((ADDR), (VAL))
#define I82559_INBYTE(ADDR) INBYTE((ADDR))
#endif

#define I82559_WAIT_CMD_DONE() {int wait = 100; do;while(I82559_INBYTE(io_address+SCR_COMMAND) && --wait >= 0);}

#define off_to_i82559_softc(X)  (X) >= CFG_NUM_I82559 ? (PI82559_SOFTC)0 : i82559softc[(X)]
#define iface_to_i82559_softc(X) (X)->minor_number >= CFG_NUM_I82559 ? (PI82559_SOFTC)0 : i82559softc[(X)->minor_number]

/* Define the following:
   SET_NO_EEPROM_ERROR = 0 if you want to return an error when no eeprom is found
   SET_NO_EEPROM_ERROR = 1 if you want to force a MAC/PHY address when no eeprom is found
   SET_MAC_ADDR        = 0 to use the eeprom's MAC address
   SET_MAC_ADDR        = 1 to override the eeprom's MAC address
*/
#define SET_NO_EEPROM_ERROR 1
#undef SET_MAC_ADDR
#define SET_MAC_ADDR 0

/* PCI Device Identification information.     */
struct i82559_pci_id_info
{
    const char *dev_name;
    word    vendor_id;
    word    device_id;
};

typedef struct rcvdsc
{
    dword status;
    dword link;
    dword buffer;
    dword count;
}  RCVDSC;
typedef  struct rcvdsc KS_FAR *PRCVDSC;

#if (USE_FLEX_RXBD)
typedef struct rcvbd
{
    dword count;
    dword link;
    dword buffer;
    dword size;
}  RCVBD;
typedef  struct rcvbd KS_FAR *PRCVBD;
#endif

typedef struct txdsc_i82559
{
    dword status;
    dword link;
    dword pbuffer_address;
    dword count;
    dword buffer_address;
    dword buffer_length;
    dword buffer_address_1;
    dword buffer_length_1;
} TXDSC_i82559;
typedef  struct txdsc_i82559 KS_FAR *PTXDSC_i82559;

typedef struct _i82559_softc
{
    TXDSC_i82559    tx_descs_data[TX_RING_SIZE];
#if (USE_FLEX_RXBD)
    RCVBD   rx_bd_data[RX_RING_SIZE];
    PRCVBD  rx_bd[RX_RING_SIZE];
    RCVDSC  rx_descs_data[RX_RING_SIZE];
    PRCVDSC rx_descs[RX_RING_SIZE];
#else
    PRCVDSC rx_descs[RX_RING_SIZE];
#endif
    PTXDSC_i82559  tx_descs[TX_RING_SIZE];

    DCU     rx_dcus[RX_RING_SIZE];
    DCU     tx_dcus[TX_RING_SIZE];
    dword   in_setup;
    PTXDSC_i82559  plast_tx;       /* Address of last tx or command desc send */

    PFBYTE      context_block_dcu;  /* DCU to be freed on close */
    PIFACE      iface;
    IOADDRESS   ia_iobase;
    int         ia_irq;
#define SETUP_FRAME_SIZE (CFG_MCLISTSIZE*6 + 10)
    byte        i82559_setup[SETUP_FRAME_SIZE+4];
    PFBYTE      pi82559_setup;      /* pointer to above but 4 byte alligned */
    dword       i82559_stats[32];   /* stats buffer for 559 only 17 are used. */
    dword *     pi82559_stats;      /* pointer to above but 4 byte alligned */
    EBS_TIMER   timer_info;         /* timer information  */
    dword       cur_ticks;          /* incremented every second */
    dword       last_rx_ticks;      /* saved every time a packet is received */
    int         rx_bug;             /* 1 if rx hangs and needs resets if no traffic */
    int         cur_rx;             /* next rx ring entry to rcv */
    int         last_rx;            /* last entry placed in the rx ring */
    int         last_tx_done;       /* last tx entry with processing complete */
    int         this_tx;            /* next tx to use */
    word        phy;                /* primary phy from eeprom */
    word        partner;
    int         flow_ctrl;
    dword       tx_threshold;
    dword       tx_control;
    word        advertising;
    struct ether_statistics stats;
} I82559_SOFTC;
typedef struct _i82559_softc KS_FAR *PI82559_SOFTC;

/* ********************************************************************   */
RTIP_BOOLEAN i82559_open(PIFACE pi);
RTIP_BOOLEAN i82559_statistics(PIFACE pi);
RTIP_BOOLEAN i82559_setmcast(PIFACE pi);
void i82559_close(PIFACE pi);
RTIP_BOOLEAN i82559_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
int i82559_xmit(PIFACE pi, DCU msg);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
struct i82559_pci_id_info i82559_pci_tab[] =
{{ "82559 In Business Card",RTPCI_V_ID_INTEL, RTPCI_D_ID_82559IB },
 { "Device ID for 82559ER ",RTPCI_V_ID_INTEL, RTPCI_D_ID_82559ER },
 { "Device ID for 82559   ",RTPCI_V_ID_INTEL, RTPCI_D_ID_82559   },
 { "Device ID for 82562   ",RTPCI_V_ID_INTEL, RTPCI_D_ID_82562   },
 { "Device ID for 82562ET ",RTPCI_V_ID_INTEL, RTPCI_D_ID_82562ET },
 {0,},                      /* 0 terminated list. */
};

#if (SET_MAC_ADDR || SET_NO_EEPROM_ERROR)
    byte i82559_mac_addr[6] = {0x00,0x12,0x34,0x56,0x78,0x9a};
#endif

volatile dword n_ints_total = 0;
volatile dword n_tx_ints_total = 0;
volatile dword n_rx_pkts_total = 0;

/* Note. these are now pointers. We allocate them from the DCU
  pool since portions of them are in memory space shared by PCI/HOST */
/* I82559_SOFTC KS_FAR _i82559softc[CFG_NUM_I82559];     */
PI82559_SOFTC KS_FAR i82559softc[CFG_NUM_I82559];

byte i82558_config_cmd[22] = 
{
    22, 0x08, 0, 1,  0, 0, 0x22, 0x03,  1, /* 1=Use MII  0=Use AUI */
    0, 0x2E, 0,  0x60, 0x08, 0x88,
    0x68, 0, 0x40, 0xf2, 0xBD,      /* 0xBD->0xFD=Force full-duplex */
    0x31, 0x05, };

EDEVTABLE KS_FAR i82559_device =
{
    i82559_open, i82559_close, i82559_xmit, i82559_xmit_done,
    NULLP_FUNC, i82559_statistics, i82559_setmcast,
    I82559_DEVICE, "SMC91C9X", MINOR_0, ETHER_IFACE,
    SNMP_DEVICE_INFO(CFG_OID_I82559, CFG_SPEED_I82559)
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
extern EDEVTABLE KS_FAR i82559_device;
extern struct i82559_pci_id_info i82559_pci_tab[];
#if (SET_MAC_ADDR || SET_NO_EEPROM_ERROR)
    extern byte i82559_mac_addr[6];
#endif

extern volatile dword n_ints_total;
extern volatile dword n_tx_ints_total;
extern volatile dword n_rx_pkts_total;

/* Note. these are now pointers. We allocate them from the DCU
  pool since portions of them are in memory space shared by PCI/HOST */
/* I82559_SOFTC KS_FAR _i82559softc[CFG_NUM_I82559];     */
extern PI82559_SOFTC KS_FAR i82559softc[CFG_NUM_I82559];

extern byte i82558_config_cmd[22];

#endif  /* !BUILD_NEW_BINARY */

/* ********************************************************************   */
RTIP_BOOLEAN i82559_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr);
void i82559_resume(PI82559_SOFTC sc);
int i82559_read_eeprom(IOADDRESS ioaddr, PFWORD pbuf);
RTIP_BOOLEAN i82559_init_rcv_ring(PI82559_SOFTC sc);
static void i82559_timeout (void KS_FAR *vsc);
static void i82559_interrupt(int deviceno);
static void i82559_pre_interrupt(int deviceno);
static void i82559_set_rcv_mode(PI82559_SOFTC sc);
static word i82559_mdio_read(PI82559_SOFTC sc, int offset);
void i82559_rcv_ring(PI82559_SOFTC sc);

/* ********************************************************************   */
RTIP_BOOLEAN i82559_open(PIFACE pi)
{
PFWORD pbuf;
PI82559_SOFTC sc;
IOADDRESS io_address;
PFBYTE p, save_p;
dword l;
int i;

    /* Alloc context block                       */
    /* i82559softc[i] = &_i82559softc[i];        */
    save_p = p = (PFBYTE) dcu_alloc_core(sizeof(*sc)+4);

    /* make sure on 4 byte boundary first       */
    l = (dword) p;
    while (l & 0x3ul) 
    {
        l++; 
        p++;
    };
    i82559softc[pi->minor_number] = (PI82559_SOFTC) p;

    /* This will fail if the above alloc failed     */
    sc = iface_to_i82559_softc(pi);
    if (!sc)
        return(FALSE);

    tc_memset((PFBYTE)sc, 0, sizeof(*sc));
    sc->context_block_dcu = save_p;

    pi->driver_stats.ether_stats = (PETHER_STATS)&(sc->stats);

#if (INCLUDE_RUN_TIME_CONFIG)
#endif

    /* Point to the stats buffer. make sure on 4 byte boundary first       */
    p = (PFBYTE) &sc->i82559_stats[0];
    l = (dword) p;
    while (l & 0x3ul) 
    {
        l++; 
        p++;
    };
    sc->pi82559_stats = (PFDWORD) p;

    /* Point to the setup buffer. make sure on 4 byte boundary first       */
#if (INCLUDE_RUN_TIME_CONFIG)
    p = (PFBYTE) sc->i82559_setup;
#else
    p = (PFBYTE) &sc->i82559_setup[0];
#endif
    l = (dword) p;
    while (l & 0x3ul) 
    {
        l++; 
        p++;
    };
    sc->pi82559_setup = (PFBYTE) p;

    /* set up transmit descriptors   */
    for (i = 0; i < TX_RING_SIZE; i++)
        sc->tx_descs[i] = &sc->tx_descs_data[i];

#if (USE_FLEX_RXBD)
    for (i = 0; i < RX_RING_SIZE; i++)
    {
        sc->rx_descs[i] = &sc->rx_descs_data[i];
        sc->rx_bd[i] = &sc->rx_bd_data[i];
    }
#endif

    sc->plast_tx = 0;
    sc->this_tx = 0;
    sc->last_tx_done = 0;
    sc->iface = pi;
    if (!i82559_pci_init(pi, &sc->ia_irq, &sc->ia_iobase))
    {
        dcu_free_core(sc->context_block_dcu);
        return(FALSE);
    }
    pi->io_address = io_address = sc->ia_iobase;
    pi->irq_val    = sc->ia_irq;

    /* Read in the setrial eeprom setup data. Use the configuration flags
       SET_NO_EEPROM_ERROR to determine our action when no eeprom is detected
       and SET_MAC_ADDR to override the eeprom MAC address */
    pbuf = (PFWORD) dcu_alloc_core(1024);
    i = i82559_read_eeprom(io_address, pbuf);
    if (i == 0) /* We had did not detect an eeprom. Use defaults from this file */
    {
#if (SET_NO_EEPROM_ERROR) /* Use locally defined eeprom values */
       pi->addr.my_hw_addr[0] = (byte)(i82559_mac_addr[0]);
       pi->addr.my_hw_addr[1] = (byte)(i82559_mac_addr[1]);
       pi->addr.my_hw_addr[2] = (byte)(i82559_mac_addr[2]);
       pi->addr.my_hw_addr[3] = (byte)(i82559_mac_addr[3]);
       pi->addr.my_hw_addr[4] = (byte)(i82559_mac_addr[4]);
       pi->addr.my_hw_addr[5] = (byte)(i82559_mac_addr[5]);
       sc->phy = PHY_DEFAULT;
       sc->rx_bug = 0;
#else
       DEBUG_ERROR("No eeprom detected == ", DINT1 , (dword) i, 0);
       dcu_free_core((PFBYTE) pbuf);
       dcu_free_core(sc->context_block_dcu);
       return(FALSE);
#endif
    }
    else if (i < 0) /* We had an eeprom read or checksum error */
    {
       DEBUG_ERROR("read eeprom fails == ", DINT1 , (dword) i, 0);
       dcu_free_core((PFBYTE) pbuf);
       dcu_free_core(sc->context_block_dcu);
       return(FALSE);
    }
    else /* Use eeprom setup data or MAC address from this file */
    {
#if (SET_MAC_ADDR)
       pi->addr.my_hw_addr[0] = (byte)(i82559_mac_addr[0]);
       pi->addr.my_hw_addr[1] = (byte)(i82559_mac_addr[1]);
       pi->addr.my_hw_addr[2] = (byte)(i82559_mac_addr[2]);
       pi->addr.my_hw_addr[3] = (byte)(i82559_mac_addr[3]);
       pi->addr.my_hw_addr[4] = (byte)(i82559_mac_addr[4]);
       pi->addr.my_hw_addr[5] = (byte)(i82559_mac_addr[5]);
#else
       /* Words 0 - 3 contain the ethernet address       */
       pi->addr.my_hw_addr[0] = (byte)(pbuf[0]);
       pi->addr.my_hw_addr[1] = (byte)(pbuf[0]>>8);
       pi->addr.my_hw_addr[2] = (byte)(pbuf[1]);
       pi->addr.my_hw_addr[3] = (byte)(pbuf[1]>>8);
       pi->addr.my_hw_addr[4] = (byte)(pbuf[2]);
       pi->addr.my_hw_addr[5] = (byte)(pbuf[2]>>8);
#endif
       /* Words 6 and 7 contain the phys. Only use primary PHY (word 6)     */
       sc->phy = *(pbuf+6);

       /* word 3 low byte contains an indication of wether the multicast setup
          workaround can be omitted at 100 or 10 Mbps (bit set == omit workaround) */
       sc->rx_bug = (pbuf[3] & 0x03) == 3 ? 0 : 1;
    }
    dcu_free_core((PFBYTE) pbuf);

    /* reset the device. This will cause the device to require a complete 
       reinitialization   */
    ks_disable();
    I82559_OUTDWORD(io_address + SCR_PORT,COMMAND_PORT_RESET);

    /* hook the interrupt service routine       */
#if (RTIP_VERSION > 24)
    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)i82559_interrupt,
                      (RTIPINTFN_POINTER)i82559_pre_interrupt,
                      pi->minor_number);
#else
    ks_hook_interrupt(sc->ia_irq, (RTIPINTFN_POINTER)i82559_interrupt, pi->minor_number);
#endif
    ks_enable();

    if (!i82559_init_rcv_ring(sc))
    {
        DEBUG_ERROR("init_rcv_ring fails == ", DINT1 , (dword) sc, 0);
        dcu_free_core(sc->context_block_dcu);
        return(FALSE);
    }

    /* Call resume to start things off. Does a complete chip setup     */
    i82559_resume(sc);

    i82559_set_rcv_mode(sc);

    if ((sc->phy & TEN_BASET_ONLY) == 0)
        sc->advertising = i82559_mdio_read(sc, 4);

    sc->cur_ticks = sc->last_rx_ticks = 0; /* watchdog for hung receiver */

    /* Set up a timer to run every three seconds       */
    sc->timer_info.func = i82559_timeout;   /* routine to execute every three seconds */
    sc->timer_info.arg = (void KS_FAR *)sc;
    ebs_set_timer(&sc->timer_info, 3, TRUE);
    ebs_start_timer(&sc->timer_info);

    /* No need to wait for the command unit to accept here.       */
    if ((sc->phy & TEN_BASET_ONLY) == 0)
        i82559_mdio_read(sc, 0);

    return(TRUE);
}

/* ********************************************************************   */
void i82559_resume(PI82559_SOFTC sc)
{
IOADDRESS io_address;
int this_tx;
PTXDSC_i82559 pt;
PTXDSC_i82559 ptn;

    io_address = sc->ia_iobase;

/* mask off all interrupts while we work     */
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_MASKALL);

/* reset tx threshhold to initial value     */
    sc->tx_threshold = TX_THRESHOLD;
/* Make up a tx control word (offset 0x0c in TCB) with current tx threshhold and flags     */
    sc->tx_control = ((sc->tx_threshold << 16) | TX_TBDNUMBER | TX_EOF);

    /* Set the device internal RU base register to 0.       */
    I82559_WAIT_CMD_DONE();
    I82559_OUTDWORD((io_address+SCR_POINTER), 0);
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_RXADDRLOAD | COMMAND_MASKALL);
    I82559_WAIT_CMD_DONE();

    /* Set the device internal CU base register to 0.       */
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CUCMDBASE | COMMAND_MASKALL);
    I82559_WAIT_CMD_DONE(); /*  wait_for_cmd_done(ioaddr + SCBCmd); */

    /* Load the address of a block to dump counters.       */
    I82559_OUTDWORD((io_address +SCR_POINTER) , kvtop((PFBYTE)sc->pi82559_stats));
    I82559_OUTWORD((io_address+SCR_COMMAND),COMMAND_CUSTATSADDR | COMMAND_MASKALL);
    *(sc->pi82559_stats+STB_DONE_MARKER) = 0;
    I82559_WAIT_CMD_DONE();

    /* Start the RU by loading the address of the rx descriptors.       */
    I82559_OUTDWORD((io_address +SCR_POINTER) , kvtop((PFBYTE)sc->rx_descs[sc->cur_rx]));
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_RXSTART | COMMAND_MASKALL);
    I82559_WAIT_CMD_DONE();

    /* Do an initial dump of the stats counters     */
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CUSTATSDUMP | COMMAND_MASKALL);

    /* Format and execute an Individual Address Setup command.
       Fill the first command with our physical address.     */
    this_tx = sc->this_tx++;
    sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */
    pt = sc->tx_descs[this_tx];
    ptn = sc->tx_descs[sc->this_tx];
    pt->status              = SWAPIF32(TXCSUSP|TXCSETIA|0xa000);
    pt->link                = SWAPIF32(kvtop((PFBYTE)ptn));
    tc_movebytes((PFBYTE)&pt->pbuffer_address,(PFBYTE)&sc->iface->addr.my_hw_addr[0], 6);
    if (sc->plast_tx)
    {
        sc->plast_tx->status &= SWAPIF32(TXCRESUME);
    }
    sc->plast_tx = pt;
    /* Start the chip's Tx process and unmask interrupts.       */
    I82559_WAIT_CMD_DONE();
    I82559_OUTDWORD((io_address +SCR_POINTER) , kvtop((PFBYTE)sc->tx_descs[sc->last_tx_done]));
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CUSTART | COMMAND_FLOWCTL);
}

/* ********************************************************************   */
static void i82559_timeout (void KS_FAR *vsc)
{
PI82559_SOFTC sc;
word partner;
int flow_ctrl;
int doreload = 0;

    sc = (PI82559_SOFTC) vsc;

    /* We have MII and lost link beat.       */
    if (!(sc->phy & TEN_BASET_ONLY))
    {
        partner = i82559_mdio_read(sc, 5);
        if (partner != sc->partner)
        {
            flow_ctrl = sc->advertising & partner & 0x0400 ? 1 : 0;
            sc->partner = partner;
            if (flow_ctrl != sc->flow_ctrl)
            {
                DEBUG_ERROR("i82559 - will reload cause lost link beat", NOVAR, 0, 0);
                sc->flow_ctrl = flow_ctrl;
                doreload = 1;
            }

            /* Clear sticky bit.       */
            i82559_mdio_read(sc, 1);

            /* If link beat has returned...       */
            if (!(i82559_mdio_read(sc, 1) & 0x0004))
            {
                DEBUG_ERROR("i82559 - lost link beat", NOVAR, 0, 0);
            }
        }
    }

    sc->cur_ticks += 1;
    if (sc->rx_bug && ((sc->cur_ticks - sc->last_rx_ticks) > 4))
    {
        sc->last_rx_ticks = sc->cur_ticks;
        doreload = 1;
        DEBUG_ERROR("i82559 - No pkt > 4 secs reset RCV", NOVAR, 0, 0);
    }

    if (doreload)
        i82559_set_rcv_mode(sc);

    ebs_start_timer(&sc->timer_info);
}


/* ********************************************************************   */
RTIP_BOOLEAN i82559_statistics(PIFACE pi)                       /*__fn__*/
{
PETHER_STATS p;
PI82559_SOFTC sc;

    sc = iface_to_i82559_softc(pi);
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

/* Set or clear the multicast filter for this adaptor.
   This is very ugly with Intel chips -- we usually have to execute an
   entire configuration command, plus process a multicast command.
   This is complicated.  We must put a large configuration command and
   an arbitrarily-sized multicast command in the transmit list.
   To minimize the disruption -- the previous command might have already
   loaded the link -- we convert the current command block, normally a Tx
   command, into a no-op and link it to the new command.
*/
/* promiscuous          */
/*#define RX_MODE 3     */
/* all multicast        */
/*#define RX_MODE 1     */
#define RX_MODE 0   /* Include multicasts in table */

/* ********************************************************************   */
static void i82559_set_rcv_mode(PI82559_SOFTC sc)
{
IOADDRESS io_address;
int this_tx;
PTXDSC_i82559 pt;
PTXDSC_i82559 plast_tx;
PTXDSC_i82559 pset;
int i;
dword iticks;
PFWORD pparms;
PFWORD paddrs;
PFBYTE pdata;
word w;
int new_rx_mode = RX_MODE;

    io_address = sc->ia_iobase;

    ks_disable();
    this_tx = sc->this_tx++;
    sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */
    pt = sc->tx_descs[this_tx];
    plast_tx = sc->plast_tx;
    sc->tx_dcus[this_tx] = 0;       /* We're not xmitting. */
    sc->plast_tx = pt;
    pt->status = SWAPIF32((TXCCFG|TXCSUSP));
    pt->link = SWAPIF32(kvtop((PFBYTE)sc->tx_descs[sc->this_tx]));
    pdata = (PFBYTE)&pt->pbuffer_address;

    /* Construct a full CmdConfig frame.       */
    tc_movebytes((PFBYTE)pdata, (PFBYTE)i82558_config_cmd, 
        sizeof(i82558_config_cmd));

    *(pdata+1) = 0x88;      /* TXFIFO|RXFIFO == 8|8 == 32|32  */
/* Tx/Rx DMA burst length, 0-127, 0 == no preemption, tx==128 -> disabled.         */
/* BUGBUG - We need to look at this when we get the info.                          */
    *(pdata+4) = 0;         /* rxdmacount; */
    *(pdata+5) = 0x00; /* 0x100;         ?? BUGBUG txdmacount(128) + 0x80; == 0x100 */
    *(pdata+15) |= (new_rx_mode & 2) ? 1 : 0; /* - zero */
    *(pdata+19) = 0x80;         /* sc->flow_ctrl(0) ? 0xBD : 0x80; */
/*      *(pdata+19) |= sc->full_duplex(0) ? 0x40 : 0;       */
    *(pdata+21) = (byte)((new_rx_mode & 1) ? 0x0D : 0x05);
    *(pdata+21) = 0x05;
    if (sc->phy & TEN_BASET_ONLY)
    {           /* Use the AUI port instead. */
        *(pdata+15) |= 0x80;
        *(pdata+8) = 0;
    }

    /* Trigger the command unit resume.       */
    I82559_WAIT_CMD_DONE();
    plast_tx->status &= SWAPIF32(TXCRESUME);
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CURESUME | COMMAND_FLOWCTL);
    ks_enable();

    /* Since this is a long frame, we've probably written into the next descriptors. Wait a
       while fot the frame to be read */
    ks_sleep((word)(ks_ticks_p_sec() / 5));

    /* Set up the multicast list       */
    pset = (PTXDSC_i82559) sc->pi82559_setup;
    pparms = (PFWORD) &pset->pbuffer_address;
    paddrs = (PFWORD) &sc->iface->mcast.mclist[0];
    w = (word) (sc->iface->mcast.lenmclist*6);
    *pparms++ = SWAPIF16(w);

    /* Copy the multicast addresses as word from the mulkticast table       */
    for (i = 0; i < sc->iface->mcast.lenmclist*3; i++)
    {
        *pparms++ = *paddrs++;
    }

    ks_disable();
    this_tx = sc->this_tx++;
    sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */
    sc->tx_dcus[this_tx] = 0;       /* We're not xmitting. */
    plast_tx = sc->plast_tx;
    sc->plast_tx = pset;
    sc->in_setup = 1;
#if (1)
    plast_tx->link =kvtop((PFBYTE)sc->tx_descs[this_tx]);
    sc->tx_descs[this_tx]->status = SWAPIF32(TXCNOOP);
    sc->tx_descs[this_tx]->link = SWAPIF32(kvtop((PFBYTE)pset));
#else
    plast_tx->link = kvtop((PFBYTE)pset);
#endif
    pset->status = SWAPIF32((TXCMCAST|TXCSUSP|TXCINTR));
    pset->link =SWAPIF32(kvtop((PFBYTE)sc->tx_descs[sc->this_tx]));

    I82559_WAIT_CMD_DONE();
    plast_tx->status &= SWAPIF32(TXCRESUME);
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CURESUME | COMMAND_FLOWCTL);
    ks_enable();

    /* Wait for the ISR to signify done by clearing swap_desc       */
    iticks = ks_get_ticks();
    while (1)
    {
       dword j;
       ks_sleep(2);
       if (!sc->in_setup)
         return;
       j = ks_get_ticks();
       if (j < iticks)
          iticks = j;
       if (j > (iticks+3*ks_ticks_p_sec()))
          break;
    }
    DEBUG_ERROR("i82558/9 - Setup failed to interrupt", NOVAR, 0, 0);
}

/* ********************************************************************   */
RTIP_BOOLEAN i82559_setmcast(PIFACE pi)      /* __fn__ */
{
PI82559_SOFTC sc;
    sc = iface_to_i82559_softc(pi);
    if (!sc)
        return(FALSE);
    i82559_set_rcv_mode(sc);
    return(TRUE);
}

/* ********************************************************************   */
void i82559_close(PIFACE pi)                     /*__fn__*/
{
PI82559_SOFTC sc;
int i;

    sc = iface_to_i82559_softc(pi);
    if (!sc)
        return;

    /* mask off all interrupts    */
    I82559_OUTWORD(sc->ia_iobase+SCR_COMMAND, COMMAND_MASKALL);

#if (INCLUDE_RUN_TIME_CONFIG)
#endif

    /* free RX ring buffer   */
    for (i = 0; i < RX_RING_SIZE; i++)
    {
        if (sc->rx_dcus[i])
            os_free_packet(sc->rx_dcus[i]);
    }
    dcu_free_core(sc->context_block_dcu);
}

/* ********************************************************************   */
RTIP_BOOLEAN i82559_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PI82559_SOFTC sc;
IOADDRESS io_address;
word status;

    sc = iface_to_i82559_softc(pi);
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
        /* error - record statistics       */
        sc->stats.errors_out++;
        sc->stats.tx_other_errors++;
        io_address = sc->ia_iobase;
        status = I82559_INWORD((io_address+SCR_STATUS));
        if ((status & 0x00C0) != 0x0080
            &&  (status & 0x003C) == 0x0010)
        {
        /* Only the command unit has stopped.       */
            I82559_OUTDWORD((io_address +SCR_POINTER) , kvtop((PFBYTE)sc->tx_descs[sc->last_tx_done]));
            I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CUSTART | COMMAND_FLOWCTL);
        }
        else
        {
            /* Reset the Tx and Rx units.       */
            I82559_OUTDWORD((io_address + SCR_PORT), COMMAND_PORT_RESET);
            ks_sleep(1);  /* delay at least 10 microsecond */
            i82559_resume(sc);
        }
    }
    return(TRUE);
}

/* ********************************************************************   */
int i82559_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
IOADDRESS io_address;
PI82559_SOFTC sc;
int   length;
int this_tx;
PTXDSC_i82559 pt;
PTXDSC_i82559 plast_tx;

    sc = iface_to_i82559_softc(pi);
    if (!sc)
        return(ENUMDEVICE);
    io_address = sc->ia_iobase;

    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_MIN_LEN)
    length = ETHER_MIN_LEN;
    if (length > ETHERSIZE)
    {
        DEBUG_ERROR("xmit - length is too large", NOVAR, 0, 0);
        length = ETHERSIZE;         /* what a terriable hack! */
    }

    plast_tx = sc->plast_tx;
    this_tx = sc->this_tx++;
    sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */
    pt = sc->tx_descs[this_tx];
    sc->tx_dcus[this_tx] = msg;

    ks_disable();
#if (1)
/* Try interrupt after compete too.        */
    pt->status              = SWAPIF32(TXCSUSP|TXCFLEX|TXCXMIT|TXCINTR);
#else
    pt->status              = SWAPIF32(TXCSUSP|TXCFLEX|TXCXMIT);
#endif
    /* Point to the next TX DESC       */
    pt->link                = SWAPIF32(kvtop((PFBYTE)sc->tx_descs[sc->this_tx]));
    pt->pbuffer_address     = SWAPIF32(kvtop((PFBYTE)&pt->buffer_address));
    pt->count               = SWAPIF32(sc->tx_control);
    pt->buffer_address  = SWAPIF32(kvtop((PFBYTE)DCUTODATA(msg)));
    pt->buffer_length   = SWAPIF32(length);
    sc->plast_tx = pt;
    /* Resume processing       */
    plast_tx->status &= SWAPIF32(TXCRESUME);
    ks_enable();

    I82559_WAIT_CMD_DONE();
    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CURESUME | COMMAND_FLOWCTL);

    return(0);
}

/* ********************************************************************   */
/* The interrupt handler does all of the Rx thread work and cleans up
   after the Tx thread. */

/* Somehow this needs to mask off interrupts and then the ISR needs to
   enable them */
void i82559_pre_interrupt(int deviceno)
{
PI82559_SOFTC sc;
/* IOADDRESS io_address;     */

    sc = off_to_i82559_softc(deviceno);

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq);

/*    io_address = sc.ia_iobase;                                       */
/* mask off all interrupts while we work                               */
/*    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_MASKALL);       */
/*    I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_CUNOP);         */
}

/* ********************************************************************   */
void i82559_interrupt(int deviceno)
{
IOADDRESS io_address;
volatile PI82559_SOFTC sc;
word status;
dword tx_status;
int last_tx_done;
int i;

n_ints_total += 1;
    sc = off_to_i82559_softc(deviceno);
    if (!sc)
    {
        DEBUG_ERROR("I82559 Bad arg to isr", NOVAR , 0, 0);
        return;
    }
    io_address = sc->ia_iobase;
    for (i = 0; i < 100; i++)
    {
        /* Get the interrupt reason and acknowledge       */
        status = I82559_INWORD((io_address+SCR_STATUS));
/*DEBUG_ERROR("I82559 io_address,status == ", DINT2 , (dword)io_address ,(dword) status);  */
#if (TEST_FCP)
        I82559_OUTWORD((io_address+SCR_STATUS),(status & 0xfd00));
#else
        I82559_OUTWORD((io_address+SCR_STATUS),(status & 0xfc00));
#endif
        if ((status & 0xfc00) == 0)
            break;
        if (status & 0x4000)     /* Packet received. */
{

/* DEBUG_ERROR("I82559 CALL rcv isr", NOVAR , 0, 0);       */
            i82559_rcv_ring(sc);
}
        if (status & 0x1000)
        {
/*DEBUG_ERROR("I82559 CALL bad rcv", NOVAR , 0, 0);  */
            if ((status & 0x003c) == 0x0028) /* No more Rx buffers. */
                {I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_RXRESNORS | COMMAND_FLOWCTL);}
            else if ((status & 0x003c) == 0x0008)
            { /* No resources (why?!) */
                /* No idea of what went wrong.  Restart the receiver.       */
                DEBUG_ERROR("I82559 Confused", NOVAR , 0, 0);
#if (USE_FLEX_RXBD)
                sc->cur_rx = 0;
                sc->last_rx = RX_RING_SIZE-1;
#endif /* (USE_FLEX_RXBD) */
                I82559_OUTDWORD((io_address +SCR_POINTER) , kvtop((PFBYTE)sc->rx_descs[sc->cur_rx]));
                I82559_OUTWORD((io_address+SCR_COMMAND), COMMAND_RXSTART | COMMAND_FLOWCTL);
            }
            sc->stats.errors_in++;
        }
        /* User interrupt, Command/Tx unit interrupt or CU not active.       */
        if (status & 0xA400)
        {
            n_tx_ints_total += 1;
/*DEBUG_ERROR("I82559 TX", NOVAR , 0, 0);       */
            last_tx_done = sc->last_tx_done;
            while(last_tx_done != sc->this_tx)
            {
                tx_status = SWAPIF32(sc->tx_descs[last_tx_done]->status);
                if (!(tx_status & 0x8000)) /* Check for complete */
                {
                    break;
                }
                if (tx_status & TX_UNDERRUN)
                {
                    if (sc->tx_threshold < 0xE0)
                        sc->tx_threshold += 1;
                    sc->tx_control = ((sc->tx_threshold << 16) | TX_TBDNUMBER | TX_EOF);
                }
                if (sc->tx_dcus[last_tx_done])
                {
/* DEBUG_ERROR("I82559 TX invoke", NOVAR , 0, 0);       */
                    ks_invoke_output(sc->iface, 1);
                }
                /* If this is a setup frame swap the real tx descriptor back in
                    and zero the swap buffer. This will signal completion */
#if (1)
                else if ((tx_status & 0x70000ul)==0)
#else
                else if ((PTXDSC_i82559) sc->pi82559_setup == sc->tx_descs[last_tx_done])
#endif
                {
/*DEBUG_ERROR("I82559 TX setup", NOVAR , 0, 0);       */
                       sc->in_setup = 0;
                }
                else
                {
/*                DEBUG_ERROR("I82559 TX not setup.. not tx ??", NOVAR , 0, 0);     */
                }
                last_tx_done++;
                last_tx_done &= TX_RING_MASK;  /* Wrap to zero if must */
            }
            sc->last_tx_done = last_tx_done;
        }
    }
    if (i == 100)
    {
        DEBUG_ERROR("I82559 >100 loops in ISR", NOVAR , 0, 0);
        /* Clear all interrupts and return       */
        I82559_OUTWORD((io_address+SCR_STATUS), 0xfc00);
    }
/*DEBUG_ERROR("I82559isr n_tx_ints , n_rcv_pkts", DINT2 , n_tx_ints_total, n_rx_pkts_total);         */
/*DEBUG_ERROR("I82559isr n_ints ", DINT1 , n_ints_total, 0);                                         */
    DRIVER_MASK_ISR_ON(sc->ia_irq);
}

/* ********************************************************************   */
static word i82559_mdio_read(PI82559_SOFTC sc, int offset)
{
dword l1,l2,l3;
dword v;
int i;

    l1 = (dword)offset; l1 <<= 16;
    l2 = (dword) (sc->phy & 0x1f); l2 <<= 21;
    l3 = 0x08000000ul|l1|l2;
    I82559_OUTDWORD(sc->ia_iobase + SCR_MDI,l3);
    for (i = 0; i < 1000; i++)
    {
        v =     I82559_INDWORD(sc->ia_iobase + SCR_MDI);
        if (v & 0x10000000ul)
            break;
    }
    if (i == 1000)
    {
        DEBUG_ERROR("i82559_mdio_read failed", NOVAR, 0, 0);
    }
    return (word)(v & 0xffff);
}


/* =================================================================         */
/* =================================================================         */
/* Input ring buffer management code                                         */
/* =================================================================         */
/* =================================================================         */

/* Initialize the receive ring buffer       */
RTIP_BOOLEAN i82559_init_rcv_ring(PI82559_SOFTC sc)
{
int i,j;

#if (!USE_FLEX_RXBD)
PRCVDSC pthis;
PRCVDSC pnext;
PFBYTE p;
#endif

dword data_size;

    data_size = ETHERSIZE;
    /* allocate the ring buffers       */
    for (i = 0; i < RX_RING_SIZE; i++)
    {
#if (USE_FLEX_RXBD)

        /* for the flex memory model, allocate the DCU and format the bd list    */
        sc->rx_dcus[i] = os_alloc_packet_input(ETHERSIZE, DRIVER_ALLOC);
        sc->rx_bd_data[i].count = SWAPIF32(0x00000000ul);
        sc->rx_bd_data[i].link = SWAPIF32(kvtop((PFBYTE)(PFBYTE) &sc->rx_bd_data[i+1]));
        sc->rx_bd_data[i].buffer = SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(sc->rx_dcus[i])));
        sc->rx_bd_data[i].size = SWAPIF32(data_size);
        /* format a parallel rfd list     */
        sc->rx_descs_data[i].status = SWAPIF32(0x00080000ul);
        sc->rx_descs_data[i].link   =
                SWAPIF32(kvtop((PFBYTE)(PFBYTE) &sc->rx_descs_data[i+1]));
        sc->rx_descs_data[i].buffer = SWAPIF32(0xfffffffful);
        sc->rx_descs_data[i].count   = SWAPIF32(0x00000000ul);

#else
        sc->rx_dcus[i] = os_alloc_packet_input(ETHERSIZE+16, DRIVER_ALLOC);
#endif
        if (!sc->rx_dcus[i])
        {
            DEBUG_ERROR("i82559: Failure allocating RX DCUS", 0, 0, 0);
            for (j = 0; j < i; j++)
                os_free_packet(sc->rx_dcus[j]);
            return(FALSE);
        }
#if (!USE_FLEX_RXBD)
        /* Now we have a DCU - point the DCU data field beyond the rx header         */
/* BUGBUG                                                                            */
/* TBD - Do this later      os_reserve_packet_data(sc->rx_dcus[i], 16);              */
#endif
    }
#if (USE_FLEX_RXBD)
    /* terminate the list of bd's and format the RFD     */
    sc->rx_bd_data[RX_RING_SIZE-1].size = SWAPIF32(data_size | 0x00008000ul);
    sc->rx_bd_data[RX_RING_SIZE-1].link =
           SWAPIF32(kvtop((PFBYTE)(PFBYTE) &sc->rx_bd_data[0]));
    sc->rx_descs_data[0].buffer = SWAPIF32(kvtop((PFBYTE)(PFBYTE)&sc->rx_bd_data[0]));
    sc->rx_descs_data[RX_RING_SIZE-1].status = SWAPIF32(0xC0080000ul);
    sc->rx_descs_data[RX_RING_SIZE-1].link   =
            SWAPIF32(kvtop((PFBYTE)(PFBYTE) &sc->rx_descs_data[0]));
    sc->cur_rx = 0;
    sc->last_rx = RX_RING_SIZE-1;

#else
    data_size = ETHERSIZE;
    data_size <<= 16;
    /* link the buffers together       */
    for (i = 0; i < RX_RING_SIZE-1; i++)
    {
/*      pthis = (PRCVDSC) DCUTOALLOCEDDATA(sc->rx_dcus[i]);         */
/*      pnext = (PRCVDSC) DCUTOALLOCEDDATA(sc->rx_dcus[i+1]);       */
        pthis = (PRCVDSC) DCUTODATA(sc->rx_dcus[i]);
        pnext = (PRCVDSC) DCUTODATA(sc->rx_dcus[i+1]);
        pthis->link   = SWAPIF32(kvtop((PFBYTE)(PFBYTE)pnext));
        pthis->status = SWAPIF32(0x00000000ul);
/*      pthis->buffer = SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(sc->rx_dcus[i])));      */
        p = (PFBYTE) DCUTODATA(sc->rx_dcus[i]);
        p += 16;
        pthis->buffer = SWAPIF32(kvtop((PFBYTE)(PFBYTE)p));
        pthis->count  = SWAPIF32(data_size);
        sc->rx_descs[i] = pthis;
    }
    pnext->link   = 0;
    pnext->status = SWAPIF32(0xC0000000ul);
/*  pnext->buffer = SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(sc->rx_dcus[i])));      */
    p = (PFBYTE) DCUTODATA(sc->rx_dcus[i]);
    p += 16;
    pnext->buffer = SWAPIF32(kvtop((PFBYTE)(PFBYTE)p));
    pnext->count  = SWAPIF32(data_size);
    sc->rx_descs[i] = pnext;
    sc->cur_rx = 0;
    sc->last_rx = i;
#endif /* (USE_FLEX_RXBD) */
    return(TRUE);
}

/* ********************************************************************        */
/* Check status and receive data from the ring buffer (called from ISR)        */
void i82559_rcv_ring(PI82559_SOFTC sc)
{
int i, cur_rx, last_rx;
PRCVDSC pthis;
PRCVDSC plast;
#if (USE_FLEX_RXBD)
PRCVBD pthisbd;
PRCVBD plastbd;
#endif
word status;
word length;
DCU  msg, invoke_msg;
dword data_size;

#if (!USE_FLEX_RXBD)
PFBYTE p;
#endif
    cur_rx  = sc->cur_rx;
    last_rx = sc->last_rx;

    for (i = 0; i < RX_RING_SIZE; i++)
    {
        pthis = sc->rx_descs[cur_rx];
#if (USE_FLEX_RXBD)
        pthisbd = sc->rx_bd[cur_rx];
#endif
        status  = (word)(SWAPIF32(pthis->status));
        if (!(status & RX_COMPLETE)) /* quit when we have no more to process */
            break;

        n_rx_pkts_total += 1;

        if (status & RX_OK)
        {
            /* We've got a good packet       */
#if (USE_FLEX_RXBD)
            length = (word) (SWAPIF32(pthisbd->count)) & 0x3fff;
#else
            length = (word) (SWAPIF32(pthis->count)) & 0x3fff;
#endif

/*            DEBUG_ERROR("I82559 RCV    ", EBS_INT1, status, 0);             */
/*            DEBUG_ERROR("I82559 RCV LENGTH   ", EBS_INT1, length, 0);       */
            invoke_msg = 0;
            if (length <= RX_COPY_BREAK )
            {
                invoke_msg = os_alloc_packet_input(length, DRIVER_ALLOC);
                if (invoke_msg)
                {
#if (USE_FLEX_RXBD)
                  tc_movebytes((PFBYTE) DCUTODATA(invoke_msg),
                                     (PFBYTE) pthisbd->buffer, length);
#else
/*                tc_movebytes(DCUTODATA(invoke_msg), 
                        DCUTODATA(sc->rx_dcus[cur_rx]), length);    */
                  p = (PFBYTE) DCUTODATA(sc->rx_dcus[cur_rx]);
                  p += 16;
                  tc_movebytes((PFBYTE) DCUTODATA(invoke_msg), (PFBYTE)p, length);
#endif
                }
            }
            else
            {
#if (USE_FLEX_RXBD)
                msg = os_alloc_packet_input(ETHERSIZE, DRIVER_ALLOC);
#else
                msg = os_alloc_packet_input(ETHERSIZE+16, DRIVER_ALLOC);
#endif
                if (msg)
                {
                    /* Put the new one in the ring and invoke the old       */
                    invoke_msg = sc->rx_dcus[cur_rx];
                    sc->rx_dcus[cur_rx] = msg;
                    pthisbd->buffer =
                        SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(sc->rx_dcus[cur_rx])));
               }
            }
            if (invoke_msg)
            {
               sc->stats.packets_in++;
               sc->stats.bytes_in += (word)(length - sizeof(struct _ether));
                    DCUTOPACKET(invoke_msg)->length = length;
#if (RTIP_VERSION > 24)
                    ks_invoke_input(sc->iface,invoke_msg);
#else
                os_sndx_input_list(sc->iface, invoke_msg);
                    ks_invoke_input(sc->iface);
#endif
            }
            else
            {
                DEBUG_ERROR("I82559 RCV ALLOC FAIL L == ", EBS_INT1, length, 0);
            }
        }
        else /* !RX_OK - We have some sort of error, record the statistic */
        {
            DEBUG_ERROR("I82559 RCV ERROR == ", EBS_INT1, status, 0);
            if (status & RX_CRC_ERROR)
                sc->stats.rx_crc_errors++;
            else if (status & RX_ALIGN_ERROR)
                sc->stats.rx_frame_errors++;
            else if (status & RX_TOOBIG_ERROR)
                sc->stats.rx_frame_errors++;
            else if (status & RX_DMAOVRN_ERROR)
                sc->stats.rx_fifo_errors++;
            else if (status & RX_TOOSHORT_ERROR)
                sc->stats.rx_frame_errors++;
            else
                sc->stats.rx_other_errors++;
            sc->stats.errors_in++;
        }

        /* We have to rearm put the descriptor on the end of the ring now       */
#if (USE_FLEX_RXBD)
        pthis->status = SWAPIF32(0xC0080000ul);     /* end of list */
        data_size = (dword) ETHERSIZE;
        pthisbd->count = SWAPIF32(0x00000000ul);
        pthisbd->size = SWAPIF32(data_size | 0x00008000ul);
        plast = sc->rx_descs[last_rx];
        plast->status &= SWAPIF32(0x3ffffffful); /* Clear last marker */
        plastbd = sc->rx_bd[last_rx];
        plastbd->size = SWAPIF32(data_size);    /* Clear last marker */

#else
        pthis->status = SWAPIF32(0xC0000000ul);     /* end of list */
        pthis->link = 0;
        data_size = (dword) ETHERSIZE;
        data_size <<= 16;
        pthis->count = SWAPIF32(data_size);
        plast = sc->rx_descs[last_rx];
        plast->link = SWAPIF32(kvtop((PFBYTE)pthis));
        plast->status &= SWAPIF32(0x3ffffffful); /* Clear last marker */
#endif
        sc->last_rx_ticks = sc->cur_ticks;  /* pulse keepalive */
        /* end of got packet without errors                         */
        last_rx  = cur_rx;
        cur_rx = (cur_rx + 1) & RX_RING_MASK;   /* Add & wrap to 0 */
        /* End of loop through ring                                  */
    }
    sc->cur_rx = cur_rx;
    sc->last_rx = last_rx;
    /* done       */
}


/* =================================================================         */
/* =================================================================         */
/* eeprom code                                                               */
/* =================================================================         */
/* =================================================================         */

/*  EEPROM control bits.       */
#define WRITE_ZERO  0x4802
#define WRITE_ONE       0x4806
#define WRITE_ZERO_CLOCK    0x4803
#define WRITE_ONE_CLOCK     0x4807

dword do_eeprom_cmd(IOADDRESS io_address, dword cmd, int cmd_len)
{
    dword retval = 0;
    dword ltemp;
    dword stemp;
    int i;

    io_address = io_address + SCR_EEPROM;
    /* reverse the bits       */
    ltemp = cmd;
    cmd = 0;
    for (i = 0; i < cmd_len; i++)
    {
        cmd <<= 1;
        if (ltemp & 0x01)
            cmd |= 1;
        ltemp >>= 1;
    }

    /* Initiate the sequence       */
    I82559_OUTWORD(io_address, WRITE_ZERO_CLOCK);
    /* Send the command serially and read the results       */
    for (i = 0; i < cmd_len; i++)
    {
           if (cmd & 1)
                {I82559_OUTWORD(io_address, WRITE_ONE);}
            else
                {I82559_OUTWORD(io_address, WRITE_ZERO);}
            I82559_INWORD(io_address);              /* just a delay */
           if (cmd & 1)
                {I82559_OUTWORD(io_address, WRITE_ONE_CLOCK);}
            else
                {I82559_OUTWORD(io_address, WRITE_ZERO_CLOCK);}
            I82559_INWORD(io_address);              /* just a delay */
            cmd >>= 1;
            stemp = I82559_INWORD(io_address);
           retval = (retval << 1);
            if (stemp & 0x08)
                retval |= 1;
    }
    /* Terminate the sequence       */
    I82559_OUTWORD(io_address,WRITE_ZERO);
    I82559_OUTWORD(io_address,0x4800);
    return retval;
}

/* ********************************************************************     */
/* Returns the size of the eeprom or 0 if not found, -1 for eeprom error    */
int i82559_read_eeprom(IOADDRESS ioaddr, PFWORD pbuf)
{
    word checksum = 0;
    int i;
    dword read_cmd;
    dword ltemp;
    dword lsize;
    int  size;

        ltemp = 6;
        ltemp <<= 24;
        lsize = do_eeprom_cmd(ioaddr, ltemp, 28);
        if (lsize == 0xfffffff)
         return (0); /* no eeprom found */
        else if ((lsize & 0xffe0000)  == 0xffe0000)
            size = 0x100;
        else
        {
            size = 0x40;
            ltemp = 6;
            ltemp <<= 22;
        }
        read_cmd = ltemp;

        for (i = 0; i < size; i++)
        {
            ltemp = i; ltemp <<= 16;
            ltemp |= read_cmd;
            *pbuf = (word) do_eeprom_cmd(ioaddr, read_cmd | ltemp, 28);
            checksum += *pbuf++;
        }
        if (checksum != 0xBABA)
        {
            DEBUG_ERROR("Invalid EEPROM checksum", NOVAR, 0, 0);
            return(-1);
        }
        return(size);
}


/* =================================================================         */
/* =================================================================         */
/* pci code                                                                  */
/* =================================================================         */
/* =================================================================         */

#define RTPCI_INTEL_REG_IOBASE  0x14
#define RTPCI_INTEL_REG_MEMBASE 0x10

RTIP_BOOLEAN i82559_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr)
{
    byte return_code;
  byte BusNum[CFG_NUM_I82559];
  byte DevFncNum[CFG_NUM_I82559];
    byte default_irq;
    byte byte_read;
    unsigned short word_read;
    int Index;

  int pos_minor_num;
  int status[CFG_NUM_I82559];
  int found = 0;

    int device_index = 0;

    struct four_bytes {
        unsigned short  wordl;
        unsigned short  wordh;
    };
    union {
        struct four_bytes two_words;
        unsigned long   cat_words;
    } d_word;

    if (rtpci_bios_present())
    {
        /*                                                                             */
        /*  Find and initialize the first specified (Vendor, Device)PCI device .       */
        /*  Since auto-negotiation (DANAS (bit 10, BCR32 == 0) and                     */
        /*  ASEL (bit 1, BCR2 == 1)) is the default, it is assumed                     */
        /*  that sw need do nothing to force auto-negotiation to occur.                */
        /*  NOTE:  Add auto negotiation based on 3/1/99 talk with Nortel. VK           */
        /*                                                                             */
        /*  The index in the Find PCI Device indicates the instance of the             */
        /*  device to search for.                                                      */
        /*  The minor device number should indicate the instance of the device,        */
        /*    therefore index is set equal to minor device number.                     */
        /*                                                                             */

      Index = pi->minor_number;

      /* cycle through each FLAVOR of this card __st__   */
      while (i82559_pci_tab[device_index].dev_name != 0)
      {
        /* make an array of all occurences by looking for   */
        /* each possible minor number of this flavor of     */
        /* this card                               __st__   */
        for (pos_minor_num=0;
              (pos_minor_num<CFG_NUM_I82559) && (found<CFG_NUM_I82559);
               pos_minor_num++)
        {
            status[found] = rtpci_find_device(i82559_pci_tab[device_index].device_id,
                                              i82559_pci_tab[device_index].vendor_id,
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
      if (found <= Index)
        return (FALSE);



            /*                                                                    */
            /*  Set the interrupt line based on the value in the                  */
            /*  PCI Interrupt Line Register.                                      */
            /*  Note:  This writes a byte into an int location.  Any issues       */
            /*         here?                                                      */
            /*                                                                    */
            return_code = rtpci_read_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_INT_LINE, &byte_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
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
                        default_irq = CFG_82559_PCI_IRQ;

                    return_code = rtpci_write_byte(BusNum[Index], DevFncNum[Index],
                                        RTPCI_REG_INT_LINE, default_irq);

                    if (return_code == RTPCI_K_SUCCESSFUL)
                        *pirq = default_irq;
                    else
                        return(FALSE);

                }
                else
                    *pirq = (int)byte_read;
            }
            else
                return(FALSE);  /* INTERRUPT LINE Register read failed     */

            /*                                             */
            /*  Set PCI Latency Timer to 32 if < 32.       */
            /*                                             */
            /*                                             */
            return_code = rtpci_read_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_LTNCY_TMR, &byte_read);
            if (return_code != RTPCI_K_SUCCESSFUL)
                return (FALSE);
            if (byte_read < 32)
            {
                byte_read = 32;
                return_code = rtpci_write_byte(BusNum[Index], DevFncNum[Index], RTPCI_REG_LTNCY_TMR, byte_read);
                if (return_code != RTPCI_K_SUCCESSFUL)
                    return (FALSE);
            }
#if (USE_MEM_MAPPED_IO)
            /*  Read the Memory mapped Base Register. Accept any non-zero value, otherwise
                use predefined values */
            return_code = rtpci_read_dword(BusNum[Index], DevFncNum[Index], 
                                           RTPCI_REG_MEMMAPIO,
                                           &d_word.cat_words);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                if (d_word.cat_words == 0)
                {
                    DEBUG_ERROR("i82559: Writing default Mem Map base value", 0, 0, 0);
                    /* if no address is present in the mem map base register,        */
                    /* set the default base address based on the                     */
                    /* user input i/o address (in demo/test programs)                */
                    /* or the default value from xnconf.h.                           */
                    if (ed_mem_address != 0)
                        d_word.cat_words = ed_mem_address;
                    else
                        d_word.cat_words = CFG_82559_PCI_MEMBASE;
                    return_code = rtpci_write_dword(BusNum[Index], DevFncNum[Index], RTPCI_REG_MEMMAPIO,
                                                    d_word.cat_words);
                    if (return_code == RTPCI_K_SUCCESSFUL)
                        *pbase_addr = (IOADDRESS) d_word.cat_words;
                    else
                        return(FALSE);  /* Register Write Failed      */
                }
                else
                   *pbase_addr = (IOADDRESS) d_word.cat_words;
            }
            else
            {
                return(FALSE);  /* Register Read Failed      */
            }
#else
            /*  Read the I/O Base Register or the Memory Mapped I/O                             */
            /*  Register and store the address as the base address of the device.               */
            /*  This is a double word (32-bit register).  I cannot access it as such            */
            /*  on a 16-bit system in REAL MODE.  Hence, I am doing two word reads.             */
            /*  The 5 low bits of the register are not address bits, therefore I am             */
            /*  masking the register with the value 0xFFFF FFE0 (defined in PCI.H).             */
            /*  For PCI devices, bits 0-7 address the max 256 bytes that the device             */
            /*  can request, bits 8&9 MBZ as these bits indicate ISA devices,                   */
            /*  at least one of bits 12-15 must be 1.                                           */

            return_code = rtpci_read_word(BusNum[Index], DevFncNum[Index], 
                                          RTPCI_INTEL_REG_IOBASE, 
                                          &d_word.two_words.wordl);

            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                if (d_word.two_words.wordl == RTPCI_IOBASE_NOVAL)
                {
                    DEBUG_ERROR("i82559: Writing default I/O base value", 0, 0, 0);
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
                        d_word.two_words.wordl = CFG_82559_PCI_IOBASE;
                    return_code = rtpci_write_word(BusNum[Index], DevFncNum[Index],
                                                   RTPCI_INTEL_REG_IOBASE,
                                                   d_word.two_words.wordl);

                    if (return_code == RTPCI_K_SUCCESSFUL)
                        *pbase_addr = (IOADDRESS) d_word.two_words.wordl;
                    else
                        return(FALSE);  /* I/O BASE Register Write Failed      */
                }
            else
                *pbase_addr = d_word.two_words.wordl & RTPCI_M_IOBASE_L;
                return_code = rtpci_read_word(BusNum[Index], DevFncNum[Index], 
                                              RTPCI_INTEL_REG_IOBASE+2, 
                                              &d_word.two_words.wordh);
/*                if (return_code == RTPCI_K_SUCCESSFUL)
                {
                    DEBUG_ERROR("rti82559_pci_init. wordh=", EBS_INT1, d_word.two_words.wordh, 0);
                    DEBUG_ERROR("rti82559_pci_init. base_addr=", EBS_INT1, *pbase_addr, 0);
                }
*/
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /*  IOBASE Register read failed    */
#endif /* #if (USE_MEM_MAPPED_IO) */

            /*                                                           */
            /*  Write PCI Command Register enabling Bus Mastering        */
            /*  (BMEM) and enabling I/O accesses (IOEN).                 */
            /*                                                           */
            return_code = rtpci_read_word(BusNum[Index], DevFncNum[Index], RTPCI_REG_CMD, &word_read);
            if (return_code == RTPCI_K_SUCCESSFUL)
            {
                word_read |= RTPCI_M_CMD_IOEN | RTPCI_M_CMD_BMEM;
                return_code = rtpci_write_word(BusNum[Index], DevFncNum[Index], RTPCI_REG_CMD, word_read);
            }
            if (return_code != RTPCI_K_SUCCESSFUL)
                return(FALSE);          /* COMMAND Register read/write failed      */
            return(TRUE);   /* PCI Device Successfully initialized */
    }
    else /* if (rtpci_bios_present()) */
        return(FALSE);      /* No PCI BIOS present.    */
}

/* ********************************************************************   */
#if (POWERPC)
void I82559_sync()
{
asm(" isync ");
asm(" eieio ");
asm(" sync ");
}
#else
#define I82559_sync()
#endif

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_i82559(int minor_number)
{
    return(xn_device_table_add(I82559_DEVICE,
                        minor_number,
                        i82559_device.iface_type,
                        "I82559",
                        SNMP_DEVICE_INFO(i82559_device.media_mib,
                                         i82559_device.speed)
                        (DEV_OPEN)i82559_device.open,
                        (DEV_CLOSE)i82559_device.close,
                        (DEV_XMIT)i82559_device.xmit,
                        (DEV_XMIT_DONE)i82559_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)i82559_device.proc_interrupts,
                        (DEV_STATS)i82559_device.statistics,
                        (DEV_SETMCAST)i82559_device.setmcast));
}

#endif /* !DECLARING_DATA */
#endif


