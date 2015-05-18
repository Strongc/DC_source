/*                                                                                    */
/* 3C90X.c                                                                      */
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
/*      This device driver controls the 3Com 905c ethernet controller,          */
/*      a PCI 10/100 base T device with integrated PHY.                         */
/*      Developed and tested with the 3Com Etherlink 10/100 adapter,            */
/*      model # 3C905C-TX-M.                                                    */
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

#if (INCLUDE_TC90X)

#if (RTIP_VERSION < 30)
#define ETHERSIZE CFG_ETHERSIZE
#endif

/* ********************************************************************   */
#define DEBUG_TC90X 0

/* ********************************************************************   */
#define SWAPIF32(X) (X)
#define SWAPIF16(X) (X)

#define NUM_INTS_PROC 10

#define TC90X_OUTWORD(ADDR, VAL) OUTWORD((ADDR), (VAL))
#define TC90X_INWORD(ADDR) INWORD((ADDR))
#define TC90X_OUTDWORD(ADDR, VAL) OUTDWORD((ADDR), (VAL))
#define TC90X_INDWORD(ADDR) INDWORD((ADDR))
#define TC90X_OUTBYTE(ADDR, VAL) OUTBYTE((ADDR), (VAL))
#define TC90X_INBYTE(ADDR) INBYTE((ADDR))

#define RTPCI_V_ID_3COM               0x10B7    /* Vendor ID for 3COM */
#define RTPCI_D_ID_905c               0x9200    /* Device ID for 3c905c  */
#define RTPCI_D_ID_3C905BTX           0x9055
#define RTPCI_D_ID_900B               0x9005    /* Device ID for 3c900B Combo  */

/* When reading from the ring buffer alloc a new dcu and copy if <= this          */
/* size. Otherwise submit the DCU from the ring buffer                            */
#define RX_COPY_BREAK   0

/* I/O registers occupy 128 bytes of i/o or mapped memory space.
   The first 16 bytes are a switchable window into one of 8 register banks.
   The width of register accesses must be considered, in general a register must be
   accessed as opperands no wider than the width of the register */

/* Window 0 register offsets   */
#define W0_EepromData       0x0c
#define W0_EepromCommand    0x0a
#define W0_BiosRomData      0x08
#define W0_BiosRomAddr      0x04

/* Window 1 register offsets   */
#define W1_TriggerBits      0x0c
#define W1_SosBits          0x0a
#define W1_WakeOnTimer      0x08
#define W1_SmbRxBytes       0x06
#define W1_SmbDiag          0x05
#define W1_SmbArb           0x04
#define W1_SmbStatus        0x02
#define W1_SmbAddress       0x01
#define W1_SmbFifoData      0x00

/* Window 2 register offsets   */
#define W2_ResetOptions     0x0c
#define W2_StationMaskHi    0x0a
#define W2_StationMaskMid   0x08
#define W2_StationMaskLo    0x06
#define W2_StationAddressHi 0x04
#define W2_StationAddressMid 0x02
#define W2_StationAddressLo 0x00

/* Window 3 register offsets   */
#define W3_TxFree           0x0c
#define W3_RxFree           0x0a
#define W3_MediaOptions     0x08
#define W3_MacControl       0x06
#define W3_MaxPktSize       0x04
#define W3_InternalConfig   0x00

/* Window 4 register offsets   */
#define W4_UpperBytesOk     0x0d
#define W4_BadSSD           0x0c
#define W4_MediaStatus      0x0a
#define W4_PhysicalMgmt     0x08
#define W4_NetworkDiagnostic 0x06
#define W4_FifoDiagnostic   0x04

/* Window 5 register offsets   */
#define W5_IndicationEnable 0x0c
#define W5_InterruptEnable  0x0a
#define W5_TxReclaimThresh  0x09
#define W5_RxFilter         0x08
#define W5_RxEarlyThresh    0x06
#define W5_TxStartThresh    0x00

/* Window 6 register offsets   */
#define W6_BytesXmittedOk   0x0c
#define W6_BytesRcvdOk      0x0a
#define W6_UpperFramesOk    0x09
#define W6_FramesDeferred   0x08
#define W6_FramesRcvdOk     0x07
#define W6_FramesXmittedOk  0x06
#define W6_RxOverruns       0x05
#define W6_LateCollisions   0x04
#define W6_SingleCollisions 0x03
#define W6_MultipleCollisions   0x02
#define W6_SqeErrors        0x01
#define W6_CarrierLost      0x00

/* Window 7 register offsets   */
#define W7_PowerMgmtEvent   0x0c
#define W7_VlanEtherType    0x04
#define W7_VlanMask         0x00

/* NON-Windowed register offsets   */
#define WX_PowerMgmtCtrl    0x7c
#define WX_UpMaxBurst       0x7a
#define WX_DnMaxBurst       0x78
#define WX_DebugControl     0x74
#define WX_DebugData        0x70
#define WX_ConfigData       0x48
#define WX_ConfigAddress    0x44
#define WX_RealTimeCnt      0x40
#define WX_UpBurstThresh    0x3e
#define WX_UpPoll           0x3d
#define WX_UpPriorityThresh 0x3c
#define WX_UpListPtr        0x38
#define WX_Countdown        0x36
#define WX_FreeTimer        0x34
#define WX_UpPktStatus      0x30
#define WX_DnPoll           0x2d
#define WX_DnPriorityThresh 0x2c
#define WX_DnBurstThresh    0x2a
#define WX_DnListPtr        0x24
#define WX_DmaCtrl          0x20
#define WX_IntStatusAuto    0x1e
#define WX_TxStatus         0x1b
#define WX_Timer            0x1a
#define WX_TxPktId          0x18

/* Offset for all windows used for status and command 
   (including switching windows */
#define WX_STAT_CMD  0x0E   /* word byte access only*/

/* Command Register Bit Assignments. The word wide command register is at offset 0x0e in all
   windows. The upper 5 bits are the command code with the lower 11 being the parameter if
   applicable. Commands marked with -*- must read cmdInProgress to ensure completion of
   the command before any further commands are issued*/
/* Reset Commands   */
#define CR_GlobalReset      0x0000  /* -*- Perform an overall reset of NIC. */
#define CR_RxReset          0x2800  /* -*- Reset the receive logic. */
#define CR_TxReset          0x5800  /* -*- Reset the transmit logic.*/

/* Transmit Commands   */
#define CR_DnStall          0x3002  /* -*- Stall the download engine.*/
#define CR_DnUnStall        0x3003  /* Unstall the download engine.*/
#define CR_SetTxReclaimThresh   0xc000  /* Set the value of the TxReclaimThresh register.*/
#define CR_SetTxStartThresh 0x9800  /* Set the value of the TxStartThresh register.*/
#define CR_TxAgain          0x9000  /* Retransmit the last packet in the queue that was
                                       just sent out of the transmit FIFO.*/
#define CR_TxDisable        0x5000  /* Disable packet transmission.*/
#define CR_TxDone           0x3800  /* Used by the external SMBus controller to signal to
                                       the NIC that the data which has been downloaded to
                                       the transmit FIFO is a complete packet.*/
#define CR_TxEnable         0x4800  /* Enable packet transmission.*/
#define CR_TxFifoBisect     0xd800  /* Logically split the transmit FIFO into two 1K FIFOs
                                       to prepare the transmit FIFO to accept keep-alive
                                       frames.*/

/* Receive Commands   */
#define CR_RxDisable        0x1800  /* Disable packet reception.*/
#define CR_RxDiscard        0x4000  /* Used by the external SMBus controller to cause the
                                       top receive packet to be discarded.*/
#define CR_RxEnable         0x2000  /* Enable packet reception.*/
#define CR_SetHashFilterBit 0xcc00  /* Program a particular bit in the hash filter.*/
#define CR_SetRxEarlyThresh 0x8800  /* Set the value of the RxEarlyThresh register.*/
#define CR_SetRxFilter      0x8000  /* Set the value of the RxFilter register.*/
/* Mode Bits in RCR ... receiver configuration register   */
#define CR_Set_UNICAST      0x8001
#define CR_Set_MULTICAST    0x8002
#define CR_Set_BROADCAST    0x8004
#define CR_Set_ALL_CAST     0x8007
#define CR_Set_PROMISCUOUS  0x8008
#define CR_UpStall          0x3000  /* -*- Stall the upload engine.*/
#define CR_UpUnStall        0x3001  /* Unstall the upload engine.*/

/* Interrupt  Commands   */
#define CR_AcknowledgeInterrupt 0x6801  /* Acknowledge active interrupts.*/
#define CR_RequestInterrupt     0x6000  /* Cause the NIC to generate an interrupt.*/
#define CR_SetIndicationEnable  0x7800  /* Set the value of the IndicationEnable register.*/
#define CR_SetInterruptEnable   0x7000  /* Set the value of the InterruptEnable register.*/

/* Other Commands   */
#define CR_DisableDcConverter   0xb800  /* Disable the 10BASE2 DC-DC converter.*/
#define CR_EnableDcConverter    0x1000  /* Enable the 10BASE2 DC-DC converter.*/
#define CR_SelectRegisterWindow 0x0800  /* Change the visible window.*/
#define CR_StatisticsDisable    0xb000  /* Disable collection of statistics.*/
#define CR_StatisticsEnable     0xa800  /* Enable collection of statistics.*/

/* Interrupt status register bits.   */
#define INT_LATCH       0x0001
#undef INT_SYSERR
#define INT_SYSERR      0x0002
#define INT_TX          0x0004
#undef INT_RX
#define INT_RX          0x0010
#define INT_RX_EARLY    0x0020
#define INT_REQUEST     0x0040
#define INT_STATS       0x0080
#define INT_LINK        0x0100
#define INT_DNCOMPLETE  0x0200
#define INT_UPCOMPLETE  0x0400
#define INT_CMDINPROG   0x1000
#define INT_WINMASK     0xE000
#undef INT_ENABLE
#define INT_ENABLE      0x0604  /* or'ed enable bits */

/* Download and Transmission   */
#undef TX_RING_SIZE
#define TX_RING_SIZE    2  /* Must be a power of 2 */
                           /* we don't need many since we send only one at a time       */
#define TX_RING_MASK    (TX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */

/* ********************************************************************   */
/* The transmit descriptor list is made up of the following structures. Although multiple
   data fragments are supported we will always use only one fragment for all packets.
   Descriptors must be 8 byte aligned (type 0 only) */
typedef struct txdsc_3C90X
{
    dword link;
    dword status;
    dword pbuffer_address;
    dword buffer_length;
} TXDSC_3C90X;
typedef  struct txdsc_3C90X KS_FAR *PTXDSC_3C90X;

/* Tx status bits   */
#define TX_CRCDisable   0x00002000
#define TX_TX_INT_ENA   0x00010000
#define TX_ADD_IP_CKSM  0x02000000
#define TX_ADD_TCP_CKSM 0x04000000
#define TX_ADD_UDP_CKSM 0x08000000
#define TX_RNDUP_DEFEAT 0x10000000
#define TX_DPD_EMPTY    0x20000000
#define TX_DN_INT_ENA   0x80000000

/* Bits in TxStatus Register   */
#define TX_txStatusOverflow  0x04
#define TX_maxCollisions     0x08

/* Reception and Upload   */
#undef RX_RING_SIZE
#define RX_RING_SIZE    4  /* Must be a power of 2 */
#define RX_RING_MASK    (RX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */

/* ********************************************************************   */
/* The receive descriptor list is made up of the following structures. Although multiple
   data fragments are supported we will always use only one fragment for all packets.
   Descriptors must be 8 byte aligned (type 0 only) */
typedef struct rxdsc 
{
    dword link;
    dword status;
    dword pbuffer_address;
    dword buffer_length;
} RXDSC;
typedef  struct rxdsc KS_FAR *PRXDSC;

/* Rx status bits   */
#define RX_UP_ERROR     0x00004000
#define RX_UP_COMPLETE  0x00008000
#define RX_OVERRUN_ERR  0x00010000
#define RX_RUNT_ERR     0x00020000
#define RX_ALIGN_ERR    0x00040000
#define RX_CRC_ERR      0x00080000
#define RX_SIZE_ERR     0x00100000
#define RX_IP_CKSM_ERR  0x02000000
#define RX_TCP_CKSM_ERR 0x04000000
#define RX_UDP_CKSM_ERR 0x08000000
#define RX_IP_CKSM_CKD  0x20000000
#define RX_TCP_CKSM_CKD 0x40000000
#define RX_UDP_CKSM_CKD 0x80000000


/* PCI Tuning Parameters
   Threshold is bytes transferred to chip before transmission starts. */
#define TX_FIFO_THRESH 256  /* In bytes, rounded down to 32 byte units. */

/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024.   */
#define RX_FIFO_THRESH  4       /* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST    4       /* Maximum PCI burst, '4' is 256 bytes */
#define TX_DMA_BURST    4       /* Calculate as 16<<val. */


/* ********************************************************************   */
/* PCI Device Identification information.                                 */
struct pci_id_info
{
    const char *dev_name;
    word        vendor_id;
    word        device_id;
};

/* ********************************************************************   */
typedef struct _TC90X_softc
{
   PIFACE     iface;
   int        device_index;
   IOADDRESS  ia_iobase;
   int        ia_irq;
   /* TX descriptor ring with alignment room   */
/*   byte  txring[TX_RING_SIZE*16 + 8];        */

   /* Pointer to alloced TX descriptor ring buffer with alignment room   */
   PFBYTE      ptxringbuf;

   /* Pointer to above but 8 byte aligned   */
   PTXDSC_3C90X ptxring[TX_RING_SIZE];

   /* last tx entry with processing complete   */
   int last_tx_done;

   /* next tx to use   */
   int this_tx;

   /* keep track of the DCU's we send   */
   DCU tx_dcus[TX_RING_SIZE];

   /* RX descriptor ring with alignment room   */
/*   byte  rxring[RX_RING_SIZE*16 + 8];        */

   /* Pointer to alloced RX descriptor ring buffer with alignment room   */
   PFBYTE prxringbuf;

   /* Pointer to above but 8 byte aligned   */
   PRXDSC prxring[RX_RING_SIZE];

   /* Index into the Rx buffer of next Rx pkt.   */
   int cur_rx;
   int last_rx;

   /* keep track of the DCU's we send   */
   DCU rx_dcus[RX_RING_SIZE];

   word int_stat;

   /* receive multicast filter (index to 64 bits)   */
   byte rx_mcast[64];

   /* primary phy number - keep even alignment   */
   byte phy[2];

   /* Contents of the internal configuration register   */
   dword  internal_config;

   /* Driver/NIC configuration   */
   dword  config;

   /* Operating Mode:                  */
   /* interrupt statistics counters    */
   dword n_int_call_total; /* total interrupt service calls (may handle multiple ints) */
   dword n_ints_total;     /* total interrupts including errors,rx, and tx */
   dword n_tx_ints_total;  /* total tx interrupts including tx error interrupts */
   dword n_rx_pkts_total;  /* total rx interrupts including rx error interrupts */
   dword n_errors_total;   /* total non rx/tx error interrupts */

   /* timer information    */
    EBS_TIMER timer_info;
    /* incremented every second   */
    dword       cur_ticks;
    /* saved every time a packet is received   */
    dword       last_rx_ticks;
    int         OutOfBuffers;

    struct ether_statistics stats;
} TC90X_SOFTC;
typedef struct _TC90X_softc KS_FAR *PTC90X_SOFTC;


#define off_to_TC90X_softc(X)  (X) >= CFG_NUM_TC90X ? (PTC90X_SOFTC)0 : &TC90Xsoftc[(X)]
#define iface_to_TC90X_softc(X) (X)->minor_number >= CFG_NUM_TC90X ? (PTC90X_SOFTC)0 : &TC90Xsoftc[(X)->minor_number]

#define SELECT_WIN(win_no) TC90X_OUTWORD(io_address+WX_STAT_CMD, CR_SelectRegisterWindow+win_no)

/* ********************************************************************   */
int TC90X_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr);
void TC90X_timeout (void KS_FAR *vsc);
void TC90X_interrupt(int deviceno);
void TC90X_pre_interrupt(int deviceno);
void TC90X_rcv_ring(PTC90X_SOFTC sc);
void TC90X_getmcaf(PFBYTE mclist, int bytesmclist, PFBYTE af);
RTIP_BOOLEAN TC90X_open_device(PIFACE pi);


/* ********************************************************************   */
RTIP_BOOLEAN TC90X_open(PIFACE pi);
void         TC90X_close(PIFACE pi);
int          TC90X_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN TC90X_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN TC90X_statistics(PIFACE  pi);
RTIP_BOOLEAN TC90X_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */
TC90X_SOFTC KS_FAR TC90Xsoftc[CFG_NUM_TC90X];

struct pci_id_info pci_tab_3c90x[] =
{
    { "3Com Etherlink 10/100 card",RTPCI_V_ID_3COM, RTPCI_D_ID_905c},
    { "Fast Etherlink 10/100 PCI TX NIC", RTPCI_V_ID_3COM, RTPCI_D_ID_3C905BTX},
    { "3Com Etherlink XL card"          , RTPCI_V_ID_3COM, RTPCI_D_ID_900B},
    {0,},                       /* 0 terminated list. */
};

EDEVTABLE KS_FAR tc90x_device =
{
    TC90X_open, TC90X_close, TC90X_xmit, TC90X_xmit_done,
    NULLP_FUNC, TC90X_statistics, TC90X_setmcast,
    TC90X_DEVICE, "3COM90X", MINOR_0, ETHER_IFACE,
    SNMP_DEVICE_INFO(CFG_OID_TC90X, CFG_SPEED_TC90X)
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
extern EDEVTABLE KS_FAR tc90x_device;
extern struct pci_id_info pci_tab_3c90x[];
extern TC90X_SOFTC KS_FAR TC90Xsoftc[CFG_NUM_TC90X];
extern EDEVTABLE KS_FAR TC90X_device;
#endif

/* ********************************************************************   */
RTIP_BOOLEAN TC90X_setmcast(PIFACE pi)      /* __fn__ */
{
PTC90X_SOFTC sc;
IOADDRESS io_address;
byte i;
    sc = iface_to_TC90X_softc(pi);
    if (!sc)
        return(FALSE);
    io_address = sc->ia_iobase;
    /* Set multicast filter on chip.               */
    /* If none needed lenmclist will be zero       */
    TC90X_getmcaf((PFBYTE) pi->mcast.mclist, pi->mcast.lenmclist,
                (PFBYTE) &sc->rx_mcast[0]);

/* loop through each bit in the filter table. If set, turn on the corresponding bit   */
    for(i=0;i<64;i++)
     {
       if (sc->rx_mcast[i] != 0)
           TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_SetHashFilterBit | i );
     }
    return(TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN TC90X_open(PIFACE pi)
{
PTC90X_SOFTC sc;
IOADDRESS io_address;

    sc = iface_to_TC90X_softc(pi);
    if (!sc)
        return(FALSE);
    tc_memset(sc, 0, sizeof(*sc));
    sc->iface = pi;

    /* Find a 90x type device indexed by minor number. The init code also wakes up the
       device from a power down state */
    sc->device_index = TC90X_pci_init(pi, &sc->ia_irq, &sc->ia_iobase);
    if (sc->device_index<0)
        return(FALSE);
    io_address = sc->ia_iobase;

    /* Disable all device interrupts by clearing the indication and enable registers
       and acknowledge any pending bits */
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_SetIndicationEnable | 0x0000);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_SetInterruptEnable | 0x0000);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_AcknowledgeInterrupt | 0x0769);

    /* Hook the interrupt service routines   */
    ks_disable();
#if (RTIP_VERSION > 24)
    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)TC90X_interrupt,
                      (RTIPINTFN_POINTER) (RTIPINTFN_POINTER)TC90X_pre_interrupt,
                      pi->minor_number);
#else
    ks_hook_interrupt(sc->ia_irq, (RTIPINTFN_POINTER)TC90X_interrupt, pi->minor_number);
#endif
    ks_enable();

    /* Set up a timer to run every three seconds       */
    sc->cur_ticks = sc->last_rx_ticks = 0; /* watchdog for hung receiver */
    sc->timer_info.func = TC90X_timeout;   /* routine to execute every three seconds */
    sc->timer_info.arg = (void KS_FAR *)sc;
    ebs_set_timer(&sc->timer_info, 1, TRUE);
    ebs_start_timer(&sc->timer_info);

    /* Call lower level device open function to allocate and
       format shared memory structures and to set up the chip and registers.
       These have been put in a routine which may also be called from txreset (xmit_done) */
    return(TC90X_open_device(pi));
}


/* ********************************************************************   */
RTIP_BOOLEAN TC90X_open_device(PIFACE pi)
{
PTC90X_SOFTC sc;
IOADDRESS io_address;
PFBYTE p;
dword l;
word eeprom[0x21];
int i,j;
dword data_size;

    sc = iface_to_TC90X_softc(pi);
    if (!sc)
        return(FALSE);
    io_address = sc->ia_iobase;

    /* We assume a global reset was done at power up or in the reset routine.
       We'll reset the recvr and xmitr to assure a clean start in case this is
       a re-open operation */
     TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_RxReset);
     while(TC90X_INWORD(io_address + WX_STAT_CMD) & INT_CMDINPROG) i=1;
     TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_TxReset);
     while(TC90X_INWORD(io_address + WX_STAT_CMD) & INT_CMDINPROG) i=1;

    /* Detect which phy address we are using   */
    /*tvotvo - implement phy detect code       */
    sc->phy[0] = 0x18; /* Assume we use the internal phy */

    /* We assume the device has a serial eeprom and that it was automatically read
       in to configure the PCI parameters and certain registers (including the MAC address
       in the station address registers. However, the 3c905c documentation alludes to a
       logic error in byte swapping the station address registers. It is unclear wether we
       should use the value from the eeprom or the registers. To be safe, we'll read the
       eeprom value. If this is an embedded device that has no eeprom, modify this code. */

    SELECT_WIN(0);
    for (i=0; i<0x22; i++)
    {
     TC90X_OUTWORD(io_address + W0_EepromCommand, 0x0080 + i);
     /* Utilize the chips countdown counter to delay 160 micro sec. The counter mode
        and count rate are set by bits in DmaCtrl register which is cleared at reset.
        therfore, count rate = 3.2 micro, mode = count when loaded */
     TC90X_OUTWORD(io_address + WX_Countdown, 0x40ff);
     for (j=0; j<100; j++)
     {
        while (TC90X_INDWORD(io_address + WX_Countdown) & 0xff00) 
            i=i;
        if (!(TC90X_INWORD(io_address + W0_EepromCommand) & 0x8000)) 
            break; /* wait for busy flag to clear */
     }
     if(j>99) 
        return (FALSE); /* the eeprom is not responding */
     eeprom[i] = TC90X_INWORD(io_address + W0_EepromData);
    }

    /* Words 10,11,12 contain the ethernet address       */
    pi->addr.my_hw_addr[0] = (byte)(eeprom[10]);
    pi->addr.my_hw_addr[1] = (byte)(eeprom[10]>>8);
    pi->addr.my_hw_addr[2] = (byte)(eeprom[11]);
    pi->addr.my_hw_addr[3] = (byte)(eeprom[11]>>8);
    pi->addr.my_hw_addr[4] = (byte)(eeprom[12]);
    pi->addr.my_hw_addr[5] = (byte)(eeprom[12]>>8);

    SELECT_WIN(2);
    for (i=0; i<6; i++)
        TC90X_OUTBYTE(io_address + W2_StationAddressLo +i, pi->addr.my_hw_addr[i]);
    TC90X_OUTWORD(io_address + W2_StationMaskLo, 0);
    TC90X_OUTWORD(io_address + W2_StationMaskMid, 0);
    TC90X_OUTWORD(io_address + W2_StationMaskHi, 0);

    /* Read and save the internal configuration register.   */
    SELECT_WIN(3);
    sc->internal_config = TC90X_INDWORD(io_address + W3_InternalConfig);

    /* ----------------------------    */
    /*  Mask off the high order        */
    /*  configuration values and       */
    /*  set the NIC to auto            */
    /*  negotiate its speed.  Set      */
    /*  up the MAC for full duplex.    */
    /*                       __st__    */
    /* ----------------------------    */
    sc->internal_config = sc->internal_config & CR_InternalConfigMask;
    sc->internal_config = sc->internal_config | CR_AutoNegotiation;
    
    TC90X_OUTDWORD(io_address + W3_InternalConfig, sc->internal_config);
    TC90X_OUTWORD( io_address + W3_MacControl,     CR_FullDuplexEnable);
    /* ----------------------------    */
    /* ----------------------------    */

    /* set up 8 byte aligned pointers to the tx descriptor ring entries and initialize
       the current ans last index. The transmit routine will do most of the work
       when we get a packet to send. */
    sc->ptxringbuf =  dcu_alloc_core(TX_RING_SIZE*16 + 8);
    if (sc->ptxringbuf == NULL)    
        return(FALSE);

    p = sc->ptxringbuf;
    l = (dword) p;
    while (l & 0x7ul) 
    {
        l++; 
        p++;
    };
    sc->ptxring[0] = (PTXDSC_3C90X) p;
    sc->ptxring[0]->link = 0;    /* We won't be using linked packets */
    for (i=1; i<TX_RING_SIZE; i++)
    {
      sc->ptxring[i] = sc->ptxring[i-1] + 1; /* descriptors are 16 bytes long */
      sc->ptxring[i]->link = 0;    /* We won't be using linked packets */
    }
    sc->this_tx = 0;
    sc->last_tx_done = 0;

    /* set up 8 byte aligned pointers to the rx descriptor ring entries and initialize
       the current ans last index. Preallocate receive buffers. */
    sc->prxringbuf =  dcu_alloc_core(RX_RING_SIZE*16 + 8);
    if (sc->prxringbuf == NULL)
    {
      dcu_free_core(sc->ptxringbuf);
      return(FALSE);
    }
    p = sc->prxringbuf;
    l = (dword) p;
    while (l & 0x7ul) {l++; p++;};
    sc->prxring[0] = (PRXDSC) p;
    for (i=1; i<RX_RING_SIZE; i++)
         sc->prxring[i] = sc->prxring[i-1] + 1; /* descriptors are 16 bytes long */
    sc->cur_rx = 0;
    sc->last_rx = RX_RING_SIZE-1;

    /* build the ring structures   */
    data_size = SWAPIF32(ETHERSIZE+4 | 0x80000000ul); /* mark as last fragment */
    for (i=0; i<RX_RING_SIZE; i++)
    {
        sc->rx_dcus[i] = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC);
        if (!sc->rx_dcus[i])
        {
            DEBUG_ERROR("3c90x: Failure allocating RX DCUS", 0, 0, 0);
            for (j = 0; j < i; j++)
                os_free_packet(sc->rx_dcus[j]);
            return(FALSE);
        }
        if(i+1 < RX_RING_SIZE)
              sc->prxring[i]->link = SWAPIF32(kvtop((PFBYTE) sc->prxring[i+1]));
        sc->prxring[i]->status = SWAPIF32(0x00000000ul);
        sc->prxring[i]->pbuffer_address =
                           SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(sc->rx_dcus[i])));
        sc->prxring[i]->buffer_length = data_size;
    }
    sc->prxring[RX_RING_SIZE-1]->link = SWAPIF32(kvtop((PFBYTE) sc->prxring[0]));
    TC90X_OUTDWORD(io_address + WX_UpListPtr, kvtop((PFBYTE) sc->prxring[0]));


    /* Initialise the multicast bits then set the mode to accept broadcast,multicast,
       and unicast. */
    for (i=0; i<64; i++) sc->rx_mcast[i] = 0;   /* receive multicast filter. 0 accepts none */
    if(!TC90X_setmcast(pi)) return (FALSE);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_Set_ALL_CAST);

     /* Enable rx/tx and interrupts   */
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_RxEnable);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_TxEnable);

    TC90X_OUTWORD(io_address + WX_STAT_CMD, 
        CR_SetIndicationEnable | INT_ENABLE);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, 
        CR_SetInterruptEnable | INT_ENABLE);

    return(TRUE);
}

/* ********************************************************************   */
static void TC90X_timeout (void KS_FAR *vsc)
{
   PTC90X_SOFTC sc = (PTC90X_SOFTC) vsc;

   if (sc->OutOfBuffers)
   {
      int i;

      sc->OutOfBuffers = 0;

      for (i=0; i<RX_RING_SIZE; i++)
      {
         if (sc->rx_dcus[i] == NULL)
         {
            DCU msg;

            sc->rx_dcus[i] = msg = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC);
            if (msg)
            {
               sc->prxring[i]->pbuffer_address = SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(msg)));
               sc->prxring[i]->status = SWAPIF32(0x00000000ul);
#if DEBUG_TC90X
               DEBUG_ERROR("3C90X_timeout: added new receive DCU", NOVAR, 0, 0);
#endif
            }
            else
            {
               sc->OutOfBuffers = 1;
#if DEBUG_TC90X
               DEBUG_ERROR("3C90X_timeout: out of DCUs", NOVAR, 0, 0);
#endif
            }
         }
      }
      /* unstall the receiver   */
      TC90X_OUTWORD(sc->ia_iobase + WX_STAT_CMD, CR_UpUnStall);
   }
   ebs_start_timer(&sc->timer_info);
}

/* ********************************************************************   */
RTIP_BOOLEAN TC90X_statistics(PIFACE pi)                       /*__fn__*/
{
PETHER_STATS p;
PTC90X_SOFTC sc;

    sc = iface_to_TC90X_softc(pi);
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
void TC90X_close(PIFACE pi)                     /*__fn__*/
{
PTC90X_SOFTC sc;
IOADDRESS io_address;
    int i;

    sc = iface_to_TC90X_softc(pi);
    if (!sc)
        return;
    io_address = sc->ia_iobase;

    /* Disable rx/tx    */
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_RxDisable);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_TxDisable);

    /* Disable all device interrupts by clearing the indication and enable registers
       and acknowledge any pending bits */
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_SetIndicationEnable | 0x0000);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_SetInterruptEnable | 0x0000);
    TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_AcknowledgeInterrupt | 0x0769);

    /* Free all allocated receive buffers, and zero the table.     */
    for (i=0; i<RX_RING_SIZE; i++)
    {
       if (sc->rx_dcus[i]!= 0)
           os_free_packet(sc->rx_dcus[i]);
       sc->rx_dcus[i]= 0;
    }
    dcu_free_core(sc->ptxringbuf);
    dcu_free_core(sc->prxringbuf);
}


/* ********************************************************************   */
RTIP_BOOLEAN TC90X_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PTC90X_SOFTC sc;
IOADDRESS io_address;

    sc = iface_to_TC90X_softc(pi);
    if (!sc)
        return(FALSE);

    if (success)
    {
        /* Update total number of successfully transmitted packets.       */
        sc->stats.packets_out++;
        sc->stats.bytes_out += DCUTOPACKET(msg)->length;
        return(TRUE);
    }
    else
    {
    int i;

      DEBUG_ERROR("TC90X_xmit_done: failure: reset chip",
        NOVAR, 0, 0);
      /* We're in serious troble here. Reset and reinitialize the chip   */
      /* error - record statistics                                       */
      sc->stats.errors_out++;
      sc->stats.tx_other_errors++;
      io_address = sc->ia_iobase;
      ks_disable();

      io_address = sc->ia_iobase;

      /* call the device close routine which will free resources and shut down the chip   */
      TC90X_close(pi);
      /* Do a global reset to put the chip back to power up state   */
      TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_GlobalReset);
      while(TC90X_INWORD(io_address + WX_STAT_CMD) & INT_CMDINPROG) i=1;
      /* call the low level open routine which sets up the chip and buffer resources   */
      ks_enable();
      return(TC90X_open_device(pi));
    }
}

/* ********************************************************************   */
/* Transmit Routine. The 3c90x device has no packet alignment constraints, however the
   descriptors must be 8 byte aligned. This was taken care of in the open routine. */
int TC90X_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
IOADDRESS io_address;
PTC90X_SOFTC sc;
int   length;
int this_tx;

    sc = iface_to_TC90X_softc(pi);
    if (!sc)
        return(ENUMDEVICE);
    io_address = sc->ia_iobase;

    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_NOCHK_MIN_LEN)
        length = ETHER_NOCHK_MIN_LEN;
    if (length > ETHERSIZE)
    {
      DEBUG_ERROR("3c90x:xmit - length is too large", NOVAR, 0, 0);
      length = ETHERSIZE;         /* what a terriable hack! */
    }

    this_tx = sc->this_tx++;
    sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */

    /* check that we haven't wrapped over an incomplete xmit buffer   */
    if (sc->this_tx == sc->last_tx_done)
    {
        DEBUG_ERROR("TX Descriptor not owned ", EBS_INT2,
            sc->this_tx, sc->last_tx_done);
        return(EOUTPUTFULL);
    }

    sc->tx_dcus[this_tx] = msg;

    /* set packet length and mark it as last fragment   */
    sc->ptxring[this_tx]->buffer_length = 
        SWAPIF32(((dword) length) | 0x80000000);
    sc->ptxring[this_tx]->pbuffer_address = 
        SWAPIF32(kvtop((PFBYTE)DCUTODATA(msg)));
    sc->ptxring[this_tx]->status =
         SWAPIF32(TX_RNDUP_DEFEAT | TX_DN_INT_ENA | TX_TX_INT_ENA);
    TC90X_OUTDWORD(io_address + WX_DnListPtr, 
        kvtop((PFBYTE) sc->ptxring[this_tx]));
    return(0);
}

/* ********************************************************************   */
void TC90X_pre_interrupt(int deviceno)
{
PTC90X_SOFTC sc;

    sc = off_to_TC90X_softc(deviceno);
    if (!sc)
        return;

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq);
}

/* ********************************************************************   */
void TC90X_interrupt(int deviceno)
{
IOADDRESS io_address;
PTC90X_SOFTC sc;
word status;
int last_tx_done;
int i;
word tx_status;

    sc = off_to_TC90X_softc(deviceno);
    if (!sc)
    {
        DEBUG_ERROR("3c90x: Bad arg to isr", NOVAR , 0, 0);
        return;     /* can't enable interrupts since don't know irq - yiks */
    }
    sc->n_int_call_total += 1;

    io_address = sc->ia_iobase;
    status = TC90X_INWORD(io_address + WX_STAT_CMD);
    sc->int_stat = status; /* for debugging */

    for (i = 0; i < NUM_INTS_PROC; i++)
    {
/* DEBUG_ERROR("3c90x: INT status == ", DINT1 , (dword) status, 0);   */

      if ((status & INT_LATCH) == 0)
            break;

      sc->n_ints_total += 1;

      if (status & INT_UPCOMPLETE)     /* Packet received or RX error */
      {
        TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_AcknowledgeInterrupt | INT_UPCOMPLETE);
        TC90X_rcv_ring(sc);
      }

      if (status & INT_DNCOMPLETE)     /* Packet downloaded, assume it sent OK */
      {
        sc->n_tx_ints_total += 1;

        last_tx_done = sc->last_tx_done;
        while(last_tx_done != sc->this_tx)
        {
          ks_invoke_output(sc->iface, 1);
          last_tx_done++;
          last_tx_done &= TX_RING_MASK;  /* Wrap to zero if must */
          sc->last_tx_done = last_tx_done;
        }
        TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_AcknowledgeInterrupt | INT_DNCOMPLETE);
      }

      if (status & INT_TX)     /* Should indicate an xmit error  */
      {
        tx_status = TC90X_INBYTE(io_address + WX_TxStatus);
        if(tx_status & TX_maxCollisions)
        {
           TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_TxEnable);
           sc->stats.collision_errors++;
           DEBUG_ERROR("3c90x: TX error - maxCollisions", NOVAR, 0, 0);
        }
        else if(tx_status & TX_txStatusOverflow)
        {
           TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_TxEnable);
           DEBUG_ERROR("3c90x: TX error - txStatusOverflow", NOVAR, 0, 0);
        }
        else 
        {
           DEBUG_ERROR("3c90x: TX error - TX_status == ", DINT1 , (dword) ((status<<16) + tx_status), 0);
        }
        sc->n_errors_total += 1;
        TC90X_OUTBYTE(io_address + WX_TxStatus,0); /* clear with arbitrary write */
      }

      if (i == NUM_INTS_PROC)
      {
        DEBUG_ERROR("3c90x: >10 loops in ISR", NOVAR , 0, 0);
        /* Clear all interrupts and return         */
        TC90X_OUTWORD(io_address + WX_STAT_CMD, CR_AcknowledgeInterrupt | INT_ENABLE);
        break;
      }
/*DEBUG_ERROR("3c90x: n_tx_ints , n_rcv_pkts", DINT2 , sc->n_tx_ints_total, sc->n_rx_pkts_total);           */
/*DEBUG_ERROR("3c90x: n_ints ", DINT1 , sc->n_ints_total, 0);                                               */
      status = TC90X_INWORD(io_address + WX_STAT_CMD);
      sc->int_stat = status; /* for debugging */

    }
    DRIVER_MASK_ISR_ON(sc->ia_irq);
    return;
}

/* ********************************************************************     */
/* Check status and receive data from the ring buffer (called from ISR).    */
void TC90X_rcv_ring(PTC90X_SOFTC sc)
{
int cur_rx, last_rx;
int loop_count = 0;
dword status;
word length;
DCU  msg;

    cur_rx  = sc->cur_rx;
    last_rx = sc->last_rx;

    while ((status = SWAPIF32(sc->prxring[cur_rx]->status)) & RX_UP_COMPLETE)
    {
        if (sc->rx_dcus[cur_rx] == NULL)
           break;
        loop_count += 1;
        sc->n_rx_pkts_total += 1;
/*      DEBUG_ERROR("3c90x status == ", DINT1 , (dword) status, 0);   */

        /* Check if we have some sort of error, record the statistic. We'll reuse the buffer
           since the data is N.G. */
        if (status & RX_UP_ERROR)
        {
/*          DEBUG_ERROR("TC90X RCV ERROR == ", EBS_INT1, status, 0);   */
            if (status & RX_CRC_ERR)
                sc->stats.rx_crc_errors++;
            else if (status & RX_ALIGN_ERR)
                sc->stats.rx_frame_errors++;
            else if (status & RX_SIZE_ERR)
                sc->stats.rx_frame_errors++;
            else if (status & RX_OVERRUN_ERR)
                sc->stats.rx_fifo_errors++;
            else if (status & RX_RUNT_ERR)
                sc->stats.rx_frame_errors++;
            else
                sc->stats.rx_other_errors++;
            sc->stats.errors_in++;
            /* clear the status upcomplete bit to put this descriptor back in service. This
               should also unstall the receiver if we ran out of descriptors */
            sc->prxring[cur_rx]->status = SWAPIF32(0x00000000ul);
        }
        else /* process input packet */
        {
            length = (word)(status & (dword) 0x1fff);
/*          DEBUG_ERROR("3c90x length == ", EBS_INT1 , length, 0);   */
            sc->stats.packets_in++;
            sc->stats.bytes_in += (word)(length - sizeof(struct _ether));
            DCUTOPACKET(sc->rx_dcus[cur_rx])->length = length;
            ks_invoke_input(sc->iface, sc->rx_dcus[cur_rx]);
            sc->rx_dcus[cur_rx] = msg = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC);
            if (msg)
            {
                sc->prxring[cur_rx]->pbuffer_address = SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(msg)));
                /* clear the status upcomplete bit to put this descriptor back in service. This
                   should also unstall the receiver if we ran out of descriptors */
                sc->prxring[cur_rx]->status = SWAPIF32(0x00000000ul);
            }
            else /* trouble!! one of the alloc's failed. Record error if any use. */
            {
                sc->OutOfBuffers = 1;
                sc->stats.rx_other_errors++;
                sc->stats.errors_in++;
                DEBUG_ERROR("3c90x interrupt: out of DCUs ", NOVAR, 0, 0);
            }
        }

        sc->last_rx_ticks = sc->cur_ticks;  /* pulse keepalive */
        last_rx  = cur_rx;
        cur_rx = (cur_rx + 1) & RX_RING_MASK;   /* Add & wrap to 0 */
    }        /* End of while loop */
    sc->cur_rx = cur_rx;
    sc->last_rx = last_rx;
    /* unstall the receiver   */
    TC90X_OUTWORD(sc->ia_iobase + WX_STAT_CMD, CR_UpUnStall);
    /* done         */
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
int TC90X_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr)
{
#if (PPC603) /* Cogent PPC603 */
  byte return_code;
#endif
  byte BusNum;
  byte DevFncNum;
  byte default_irq;
  byte byte_read;
  unsigned short word_read;
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
      while (pci_tab_3c90x[device_index].dev_name != 0)
      {
        if( rtpci_find_device(pci_tab_3c90x[device_index].device_id,
                              pci_tab_3c90x[device_index].vendor_id,
                              Index, &BusNum, &DevFncNum) != RTPCI_K_SUCCESSFUL)
        {
          device_index++;
        }
        else
        {
          /*                                                                    */
          /*  Set the interrupt line based on the value in the                  */
          /*  PCI Interrupt Line Register.                                      */
#if (PPC603) /* Cogent PPC603 Always uses IRQ2, See Init603.c for base address */
          *pirq = 2;
          *pbase_addr = cog603_base_address();
          return_code = rtpci_write_dword(BusNum, DevFncNum, RTPCI_REG_IOBASE, 0x01);
/* set max latency timer to max                                                            */
          return_code = rtpci_write_byte(BusNum, DevFncNum,  0x3F , 0xFF);
#else
          if(rtpci_read_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE, &byte_read)
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
                 default_irq = CFG_TC90X_PCI_IRQ;

              if(rtpci_write_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE,default_irq)
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
          if (rtpci_read_byte(BusNum, DevFncNum, RTPCI_REG_LTNCY_TMR, &byte_read)
                              != RTPCI_K_SUCCESSFUL)
            return (-1);
          if (byte_read < 32)
          {
            byte_read = 32;
            if (rtpci_write_byte(BusNum, DevFncNum, RTPCI_REG_LTNCY_TMR, byte_read)
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

          if (rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_IOBASE, &d_word.two_words.wordl)
                              == RTPCI_K_SUCCESSFUL)
          {
            if (d_word.two_words.wordl == RTPCI_IOBASE_NOVAL)
            {
              DEBUG_ERROR("3c90x: Writing default I/O base value", 0, 0, 0);
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
                 d_word.two_words.wordl = CFG_TC90X_PCI_IOBASE;

              if (rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_IOBASE, d_word.two_words.wordl)
                                   == RTPCI_K_SUCCESSFUL)
                  *pbase_addr = (IOADDRESS) d_word.two_words.wordl;
              else
                  return(-1);  /* I/O BASE Register Write Failed      */
              }
            else
              *pbase_addr = d_word.two_words.wordl & RTPCI_M_IOBASE_L;

            if (rtpci_read_word(BusNum,DevFncNum,RTPCI_REG_IOBASE+2,&d_word.two_words.wordh)
                                == RTPCI_K_SUCCESSFUL)
            {
               DEBUG_ERROR("TC90X_pci_init. wordh=", EBS_INT1, d_word.two_words.wordh, 0);
               DEBUG_ERROR("TC90X_pci_init. base_addr=", EBS_INT1, *pbase_addr, 0);
            }
#endif /* #if (PPC603) */
            /*                                                           */
            /*  Write PCI Command Register enabling Bus Mastering        */
            /*  (BMEM) and enabling I/O accesses (IOEN).                 */
            /*                                                           */
            if (rtpci_read_word(BusNum, DevFncNum, RTPCI_REG_CMD, &word_read)
                                == RTPCI_K_SUCCESSFUL)
            {
              word_read |= RTPCI_M_CMD_IOEN | RTPCI_M_CMD_BMEM;
              if (rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_CMD, word_read)
                                    != RTPCI_K_SUCCESSFUL)
                return(-1);          /* COMMAND Register read/write failed      */
            }
          }

          /* Check if the device is in any of the power down states. If so, wake it up   */
          if (rtpci_read_word(BusNum, DevFncNum, 0xe0, &word_read)
                                == RTPCI_K_SUCCESSFUL)
            {
              word_read &= 0xfffc;
              if (rtpci_write_word(BusNum, DevFncNum, 0xe0, word_read)
                                    != RTPCI_K_SUCCESSFUL)
                return(-1);          /* COMMAND Register read/write failed      */
            }

          return(device_index);
        } /* if rtpci_find_device {device_index++}.. else {  } */
      } /* while (pci_tab_3c90x[device_index].dev_name != 0) */
      return(-1);
    }
    else  /* if (rtpci_bios_present()) */
      return(-1);      /* No PCI BIOS present.    */
}

/* ********************************************************************   */
/* calc values for multicast                                              */
void TC90X_getmcaf(PFBYTE mclist, int lenmclist, PFBYTE af)
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

/* ********************************************************************   */
/* API                                                                    */
/* ********************************************************************   */
int xn_bind_tc90x(int minor_number)
{
    return(xn_device_table_add(tc90x_device.device_id,
                        minor_number,
                        tc90x_device.iface_type,
                        tc90x_device.device_name,
                        SNMP_DEVICE_INFO(tc90x_device.media_mib,
                                         tc90x_device.speed)
                        (DEV_OPEN)tc90x_device.open,
                        (DEV_CLOSE)tc90x_device.close,
                        (DEV_XMIT)tc90x_device.xmit,
                        (DEV_XMIT_DONE)tc90x_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)tc90x_device.proc_interrupts,
                        (DEV_STATS)tc90x_device.statistics,
                        (DEV_SETMCAST)tc90x_device.setmcast));
}
#endif  /* DECLARING_DATA */
#endif

