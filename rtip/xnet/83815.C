/*                                                                              */
/* 83815.c                                                                      */
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
/*      This device driver controls the National Semiconductor DP83815          */
/*      ethernet controller, a PCI 10/100 base T device with integrated PHY.    */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*       Data Structure Definition:                                             */

#include "sock.h"
#include "rtip.h"
#include "pci.h"

#if (INCLUDE_N83815)

#if (RTIP_VERSION < 30)
#define ETHERSIZE CFG_ETHERSIZE
#endif

#if (defined (RTKBCPP)) /* for power pack we allocate this */
#if (!INCLUDE_MALLOC_DCU_INIT)
#error DPMI must use MALLOC PACKET_INIT for this card
#endif
#endif

/* If you need to set your own MAC address define SET_MAC_ADDR to 1 and    */
/* fill in the mac_addr array                                              */
#undef SET_MAC_ADDR
#define SET_MAC_ADDR 0

#define SWAPIF32(X) (X)
#define SWAPIF16(X) (X)

/* macros to read and write to MAC/BIU registers   */
#define N83815_OUTWORD(ADDR, VAL) OUTWORD((ADDR), (VAL))     /* local functions are wrappers */
#define N83815_INWORD(ADDR) INWORD((ADDR))                   /* so that byte swapping may be */
#define N83815_OUTDWORD(ADDR, VAL) OUTDWORD((ADDR), (VAL))   /* done if go to a big endian   */
#define N83815_INDWORD(ADDR) INDWORD((ADDR))                 /* processor                    */
#define N83815_OUTBYTE(ADDR, VAL) OUTBYTE((ADDR), (VAL))
#define N83815_INBYTE(ADDR) INBYTE((ADDR))

#define RTPCI_V_ID_NS                 0x100B    /* PCI Vendor ID for National Semiconductor */
#define RTPCI_D_ID_83815              0x0020    /* Device ID for DP83815 assigned by NSC  */

/* When reading from the ring buffer alloc a new dcu and copy if <= this          */
/* size. Otherwise submit the DCU from the ring buffer                            */
#define RX_COPY_BREAK   120

/* Operational registers mapped into PCI memory space or I/O space   */
/* Require 256 bytes                                                 */

/* MAC/BIU Registers   */

#define DP_CR               0x00 /* Command Register */
#define DP_CFG              0x04 /* Configuration Register */
#define DP_MEAR             0x08 /* EEPROM Access Register */
#define DP_PTSCR            0x0C /* PCI Test Control Register */
#define DP_ISR              0x10 /* Intr Status Register (RO) */
#define DP_IMR              0x14 /* Intr Mask Register */
#define DP_IER              0x18 /* Intr Enable Register */
#define DP_TXDP             0x20 /* Tx Descriptor Pointer Register */
#define DP_TXCFG            0x24 /* Tx Configuration Register */
#define DP_RXDP             0x30 /* Rx Descriptor Pointer Register */
#define DP_RXCFG            0x34 /* Rx Configuration Register */
#define DP_CCSR             0x3C /* CLKRUN Control/Status Register */
#define DP_WCSR             0x40 /* Wake on LAN Control/Status Register */
#define DP_PCR              0x44 /* Pause Control/Status Register */
#define DP_RFCR             0x48 /* Rx Filter/Match Control Register */
#define DP_RFDR             0x4C /* Rx Filter/Match Data Register */
#define DP_BRAR             0x50 /* Boot ROM Address */
#define DP_BRDR             0x54 /* Boot ROM Data */
#define DP_SRR              0x58 /* Silicon Revision Register (RO) */
#define DP_MIBC             0x5C /* MIB Control Registor */
#define DP_MIB              0x60 /* MIB Data Register Base (RO) */

/* MIB Registers   */

#define DP_MIB_RX_PKT_ERR   0x60 /* Pkts recvd with errors */
#define DP_MIB_RX_FCS_ERR   0x64 /* Pkts recvd with frame check seq errs */
#define DP_MIB_RX_MISS_PKT  0x68 /* Pkts missed due to FIFO  overruns*/
#define DP_MIB_RX_FA_ERR    0x6C /* Pkts recvd with frame alignment errs */
#define DP_MIB_RX_SYM_ERR   0x70 /* Pkts recvd with symbol errs */
#define DP_MIB_RX_LONG_FRM  0x74 /* Pkts > 1518 bytes */
#define DP_MIB_TXSQE_ERR    0x78 /* Loss of coll. heartbeat on Tx */

/* Internal Phy Registers   */

#define DP_BMCR             0x80 /* Basic Mode Control Register */
#define DP_BMSR             0x84 /* Basic Mode Status Register (RO) */
#define DP_PHYIDR1          0x88 /* PHY Identifier Register #1 (RO) */
#define DP_PHYIDR2          0x8C /* PHY Identifier Register #2 (RO) */
#define DP_ANAR             0x90 /* Auto-Nego Advertisment Reg */
#define DP_ANLPAR           0x94 /* Auto-Nego Link Partner Ability Reg */
#define DP_ANER             0x98 /* Auto-Negotiation Expansion Reg */
#define DP_ANNPTR           0x9C /* Auto-Negotiation Next Page TX */
#define DP_PHYSTS           0xC0 /* PHY Status Register (RO) */
#define DP_FCSCR            0xD0 /* False Carrier Sense Counter Reg */
#define DP_RECR             0xD4 /* Recv Error Counter Register */
#define DP_PHYCR            0xE4 /* PHY Control Register */
#define DP_10BTSCR          0xE8 /* 10Base-TStatus/Control Reg */

/*
 * Command Register Bit Masks (DP_CR)
 *
 * This register is used for issuing commands to DP83815. A global software
 * reset along with individual reset and enable/disable switches for
 * transmitter and receiver are provided here.
 */

#define DP_CR_TXE           0x00000001 /* Transmit Enable */
#define DP_CR_TXD           0x00000002 /* Transmit Disable */
#define DP_CR_RXE           0x00000004 /* Receiver Enable */
#define DP_CR_RXD           0x00000008 /* Receiver Disable */
#define DP_CR_TXR           0x00000010 /* Transmit Reset */
#define DP_CR_RXR           0x00000020 /* Receiver Reset */
#define DP_CR_SWI           0x00000080 /* Software Interrupt */
#define DP_CR_RST           0x00000100 /* Reset */

/*
 * Configuration and Media Status Register Bit Masks (DP_CFG)
 *
 * This register allows configuration of a variety of device and phy options,
 * and provides phy status information.
 */

#define DP_CFG_BEM              0x00000001 /* Big Endian Mode (BM xfers) */
#define DP_CFG_BROM_DIS         0x00000004 /* Disable Boot ROM interface */
#define DP_CFG_PESEL            0x00000008 /* Parity Err Det (BM xfer) */
#define DP_CFG_EXD              0x00000010 /* Excessv Deferl Tmr disbl */
#define DP_CFG_POW              0x00000020 /* Prog Out of Window Timer */
#define DP_CFG_SB               0x00000040 /* Single Back-off */
#define DP_CFG_REQALG           0x00000080 /* PCI Bus Request Algorithm */
#define DP_CFG_EUPHCOMP         0x00000100 /* DP83810 Descriptor Compat */
#define DP_CFG_PHY_DIS          0x00000200 /* Disable internal Phy */
#define DP_CFG_PHY_RST          0x00000400 /* Reset internal Phy */
#define DP_CFG_ANEG_SEL         0x0000E000 /* Auto-nego Sel - Mask */
#define DP_CFG_ANEG_SEL_10_HD   0x00000000 /*  Force 10Mb Half duplex */
#define DP_CFG_ANEG_SEL_100_HD  0x00004000 /*  Force 100Mb Half duplex */
#define DP_CFG_ANEG_SEL_10_FD   0x00008000 /*  Force 10Mb Full duplex */
#define DP_CFG_ANEG_SEL_100_FD  0x0000C000 /*  Force 100Mb Full duplex */
#define DP_CFG_ANEG_SEL_10_XD   0x00002000 /*  Nego 10Mb Half/Full dplx */
#define DP_CFG_ANEG_SEL_ALL_HD  0x00006000 /*  Nego 10/100 Half duplex */
#define DP_CFG_ANEG_SEL_100_XD  0x0000A000 /*  Nego 100 Half/Full duplex */
#define DP_CFG_ANEG_SEL_ALL_XD  0x0000E000 /*  Nego 10/100 Half/Full dplx*/
#define DP_CFG_PAUSE_ADV        0x00010000 /* Strap for pause capable */
#define DP_CFG_PINT_ACEN        0x00020000 /* Phy Intr Auto Clr Enable */
#define DP_CFG_PHY_CFG          0x00FC0000 /* Phy Configuration */
#define DP_CFG_ANEG_DN          0x08000000 /* Auto-negotiation Done */
#define DP_CFG_POL              0x10000000 /* 10Mb Polarity Indication */
#define DP_CFG_FDUP             0x20000000 /* Full Duplex */
#define DP_CFG_SPEED100         0x40000000 /* Speed 100Mb */
#define DP_CFG_LNKSTS           0x80000000 /* Link status */


/*
 * EEPROM Access Register Bit Masks (DP_MEAR)
 *
 * Provides an interface for software access to the NMC9306 style EEPROM.  The
 * default values given assume that the EEDO line has a pullup resistor to
 * VDD.
 */

#define DP_MEAR_EEDI            0x00000001 /* EEPROM data in */
#define DP_MEAR_EEDO            0x00000002 /* EEPROM data out */
#define DP_MEAR_EECLK           0x00000004 /* EEPROM Serial Clock */
#define DP_MEAR_EESEL           0x00000008 /* EEPROM Chip Select */

/* PCI Test Control Register Bit Masks (DP_PTSCR)   */

#define DP_PTSCR_EEBIST_FAIL    0x00000001 /* EE BIST Fail Indication */
#define DP_PTSCR_EEBIST_EN      0x00000002 /* Enable  EEPROM BIST */
#define DP_PTSCR_EELOAD_EN      0x00000004 /* Enable EEPROM Load */
#define DP_PTSCR_RBIST_RXFFAIL  0x00000008 /* RX Filter RAM BIST Fail */
#define DP_PTSCR_RBIST_TXFAIL   0x00000010 /* TX FiFO Fail */
#define DP_PTSCR_RBIST_RXFAIL   0x00000020 /* RX FIFO BIST Fail */
#define DP_PTSCR_RBIST_DONE     0x00000040 /* SRAM BIST Done*/
#define DP_PTSCR_RBIST_EN       0x00000080 /* SRAM BIST Enable */
#define DP_PTSCR_RBIST_MODE     0x00000100 /* SRAM BIST Mode */
#define DP_PTSCR_RBIST_CLKD     0x00000200 /* SRAM BIST Clock */
#define DP_PTSCR_RBIST_RST      0x00000400 /* SRAM BIST Reset */
#define DP_PTSCR_RESVD          0x00001000 /* Reserved -- Must be 1 */


/*
 * Interrupt Status Register Bit Masks (DP_ISR)
 *
 * Indicates the source of an interrupt when the INTA pin goes active.
 * Enabling the corresponding bits in the IMR allows bits in this reg to produce
 * an interrupt. ISR reflects all pending iterrupts regardless of the status
 * of the corresponding mask bit in the IMR. Reading the ISR clears all interrupts.
 * Writing to the ISR has no effect.
 */

#define DP_ISR_RXOK             0x00000001 /* Rx OK */
#define DP_ISR_RXDESC           0x00000002 /* Rx Descriptor */
#define DP_ISR_RXERR            0x00000004 /* Rx packet Error */
#define DP_ISR_RXEARLY          0x00000008 /* Rx Early Threshold */
#define DP_ISR_RXIDLE           0x00000010 /* Rx Idle */
#define DP_ISR_RXORN            0x00000020 /* Rx Overrun */
#define DP_ISR_TXOK             0x00000040 /* Tx Packet OK */
#define DP_ISR_TXDESC           0x00000080 /* Tx Descriptor */
#define DP_ISR_TXERR            0x00000100 /* Tx Packet Error */
#define DP_ISR_TXIDLE           0x00000200 /* Tx Idle */
#define DP_ISR_TXURN            0x00000400 /* Tx Underrun */
#define DP_ISR_MIB              0x00000800 /* MIB Service */
#define DP_ISR_SWI              0x00001000 /* Software Interrupt */
#define DP_ISR_PME              0x00002000 /* Power Management Event */
#define DP_ISR_PHY              0x00004000 /* Phy Interrupt */
#define DP_ISR_HIBERR           0x00008000 /* High Bits error set */
#define DP_ISR_RXSOVR           0x00010000 /* Rx Status FIFO Overrun */
#define DP_ISR_RTABT            0x00100000 /* Received Target Abort */
#define DP_ISR_RMABT            0x00200000 /* Received Master Abort */
#define DP_ISR_SSERR            0x00400000 /* Signaled System Error */
#define DP_ISR_DPERR            0x00800000 /* Detected Parity Error */
#define DP_ISR_RXRCMP           0x01000000 /* Receive Reset Complete */
#define DP_ISR_TXRCMP           0x02000000 /* Transmit Reset Complete */

/*
 * Interrupt Mask Register Bit Masks (DP_IMR)
 *
 * Interrupts are enabled by setting the appropriate bit-mask.
 */

#define DP_IMR_ALLOFF            0x00000000 /* Mask all interrupts off */
#define DP_IMR_RXOK              0x00000001 /* Rx ok */
#define DP_IMR_RXDESC            0x00000002 /* Rx Descriptor */
#define DP_IMR_RXERR             0x00000004 /* Rx packet Error */
#define DP_IMR_RXEARLY           0x00000008 /* Rx Early Threshold */
#define DP_IMR_RXIDLE            0x00000010 /* Rx Idle */
#define DP_IMR_RXORN             0x00000020 /* Rx Overrun */
#define DP_IMR_TXOK              0x00000040 /* Tx Packet Ok */
#define DP_IMR_TXDESC            0x00000080 /* Tx Descriptor */
#define DP_IMR_TXERR             0x00000100 /* Tx Packet Error */
#define DP_IMR_TXIDLE            0x00000200 /* Tx Idle */
#define DP_IMR_TXURN             0x00000400 /* Tx Underrun */
#define DP_IMR_MIB               0x00000800 /* MIB Service */
#define DP_IMR_SWI               0x00001000 /* Software Interrupt */
#define DP_IMR_PME               0x00002000 /* Power Management Event */
#define DP_IMR_PHY               0x00004000 /* Phy Interrupt */
#define DP_IMR_HIERR             0x00008000 /* High Bits error set */
#define DP_IMR_RXSOVR            0x00010000 /* Rx Status FIFO Overrun */
#define DP_IMR_RTABT             0x00100000 /* Received Target Abort */
#define DP_IMR_RMABT             0x00200000 /* Received Master Abort */
#define DP_IMR_SSERR             0x00400000 /* Signaled System Error */
#define DP_IMR_DPERR             0x00800000 /* Detected Parity Error */
#define DP_IMR_RXRCMP            0x01000000 /* Receive Reset Complete */
#define DP_IMR_TXRCMP            0x02000000 /* Transmit Reset Complete */

/*
 * Interrupt Enable Register Bit Masks (DP_IER)
 *
 * Enable or disable DP chip interrupts
 */

#define DP_IER_IE               0x00000001 /* Interrupt Enable */
#define DP_IER_ID               0x00000000 /* Interrupt Disable */

/* Transmit descriptor Pointer Register Bit Mask (DP_TXDP)   */

#define DP_TXDP_MSK             0xFFFFFFFC /* Transmit Descriptor Ptr */

/* Transmit Configuration Register Bit Masks (DP_TXCFG)   */
#define DP_TXCFG_DRTH_VAL       0x00000030 /* Tx Drain Threshold value*/
#define DP_TXCFG_FLTH_VAL       0x00001000 /* Tx Fill Threshold value*/
#define DP_TXCFG_DRTH           0x0000003F /* Tx Drain Threshold mask*/
#define DP_TXCFG_FLTH           0x00003F00 /* Tx Fill Threshold mask*/
#define DP_TXCFG_MXDMA          0x00700000 /* Max DMA Burst Size */
#define DP_TXCFG_MXDMA_1        0x00100000 /* 1 32-bit word */
#define DP_TXCFG_MXDMA_2        0x00200000 /* 2 32-bit words */
#define DP_TXCFG_MXDMA_4        0x00300000 /* 4 32-bit words */
#define DP_TXCFG_MXDMA_8        0x00400000 /* 8 32-bit words */
#define DP_TXCFG_MXDMA_16       0x00500000 /* 16 32-bit words */
#define DP_TXCFG_MXDMA_32       0x00600000 /* 32 32-bit words */
#define DP_TXCFG_MXDMA_64       0x00700000 /* 64 32-bit words */
#define DP_TXCFG_MXDMA_128      0x00000000 /* 128 32-bit words */
#define DP_TXCFG_IFG            0x0C000000 /* Interframe gap Time */
#define DP_TXCFG_ATP            0x10000000 /* Automatic Transmit Pad */
#define DP_TXCFG_MLB            0x20000000 /* MAC Loopback */
#define DP_TXCFG_HBI            0x40000000 /* HeartBeat Ignore */
#define DP_TXCFG_CSI            0x80000000 /* Carrier Sense Ignore */

#define DP_TXCFG_DRTH_SET(X)    ((X) & DP_TXCFG_DRTH)
#define DP_TXCFG_FLTH_SET(X)    (((X) << 8) & DP_TXCFG_FLTH)

/* Receive Descriptor Pointer Register Bit Mask (DP_RXDP)   */

#define DP_RXDP_MSK             0xFFFFFFFC /* Receive Descriptor Ptr */

/* Receive Configuration Register Bit Masks (DP_RXCFG)   */
#define DP_RXCFG_DRTH_VAL       0x00000008 /* Rx Drain Threshold value */
#define DP_RXCFG_DRTH           0x0000003E /* Rx Drain Threshold mask */
#define DP_RXCFG_MXDMA          0x00700000 /* Max DMA Burst size */
#define DP_RXCFG_MXDMA_1        0x00100000 /* 1 32-bit words */
#define DP_RXCFG_MXDMA_2        0x00200000 /* 2 32-bit words */
#define DP_RXCFG_MXDMA_4        0x00300000 /* 4 32-bit words */
#define DP_RXCFG_MXDMA_8        0x00400000 /* 8 32-bit words */
#define DP_RXCFG_MXDMA_16       0x00500000 /* 16 32-bit words */
#define DP_RXCFG_MXDMA_32       0x00600000 /* 32 32-bit words */
#define DP_RXCFG_MXDMA_64       0x00700000 /* 64 32-bit words */
#define DP_RXCFG_MXDMA_128      0x00000000 /* 128 32-bit words */
#define DP_RXCFG_ALP            0x08000000 /* Accept Long Packets */
#define DP_RXCFG_ATX            0x10000000 /* Accept Transmit Packets */
#define DP_RXCFG_ARP            0x40000000 /* Accept Runt Packets */
#define DP_RXCFG_AEP            0x80000000 /* Accept Errored Packets */

#define DP_RXCFG_DRTH_SET(X)    ((X) & DP_RXCFG_DRTH)


/*
 * Wake Command/Status Register Bit Masks (DP_WCSR)
 *
 * It is used to configure/control and monitor DP83815 Wake On LAN Logic
 * The Wake On LAN logic is used to monitor the incoming packet stream while
 * in a low-power state, and provide a wake event to the system if desired
 * packet type, contents, or Link change are detected.
 */

#define DP_WCSR_WKPHY           0x00000001 /* Wake on Phy Interrupt */
#define DP_WCSR_WKUCP           0x00000002 /* Wake on Unicast */
#define DP_WCSR_WKMCP           0x00000004 /* Wake on Multicast */
#define DP_WCSR_WKBCP           0x00000008 /* Wake on Broadcast */
#define DP_WCSR_WKARP           0x00000010 /* Wake on ARP */
#define DP_WCSR_WKPAT0          0x00000020 /* Wake on Pattern 0 match */
#define DP_WCSR_WKPAT1          0x00000040 /* Wake on Pattern 1 match */
#define DP_WCSR_WKPAT2          0x00000080 /* Wake on Pattern 2 match */
#define DP_WCSR_WKPAT3          0x00000100 /* Wake on Pattern 3 match */
#define DP_WCSR_WKMAG           0x00000200 /* Wake on Magic Packet */
#define DP_WCSR_PHYINT          0x00400000 /* Phy Interrupt */
#define DP_WCSR_UCASTR          0x00800000 /* Unicast Received */
#define DP_WCSR_MCASTR          0x01000000 /* Multicast Received */
#define DP_WCSR_BCASTR          0x02000000 /* Broadcast Received */
#define DP_WCSR_ARPR            0x04000000 /* ARP Received */
#define DP_WCSR_PATM0           0x08000000 /* Pattern 0 match */
#define DP_WCSR_PATM1           0x10000000 /* Pattern 1 match */
#define DP_WCSR_PATM2           0x20000000 /* Pattern 2 match */
#define DP_WCSR_PATM3           0x40000000 /* Pattern 3 match */
#define DP_WCSR_MPR             0x80000000 /* Magic Packet Received */


/*
 * Receive Filter/Match Control Register Bit Masks (DP_RFCR)
 *
 * It is used to control and configure the DP83815 Receive Filter Control logic
 * The RFC logic is used to configure destination address filtering of incoming
 * packets.
 */

#define DP_RFCR_RFADDR          0x000003FF /* Rx Filter Extended RegAdd */
#define DP_RFCR_RFADDR_PMATCH1  0x00000000 /* Perfect Match octets 1-0 */
#define DP_RFCR_RFADDR_PMATCH2  0x00000002 /* Perfect Match octets 3-2 */
#define DP_RFCR_RFADDR_PMATCH3  0x00000004 /* Perfect Match octets 5-4 */
#define DP_RFCR_RFADDR_PCOUNT1  0x00000006 /* Pattern Count 1-0 */
#define DP_RFCR_RFADDR_PCOUNT2  0x00000008 /* Pattern Count 3-2 */
#define DP_RFCR_RFADDR_SOPAS1   0x0000000A /* SecureOn Password 1-0 */
#define DP_RFCR_RFADDR_SOPAS2   0x0000000C /* SecureOn Password 3-2 */
#define DP_RFCR_RFADDR_SOPAS3   0x0000000E /* SecureOn Password 5-4 */
#define DP_RFCR_RFADDR_FMEM_LO  0x00000200 /* Rx filter memory start */
#define DP_RFCR_RFADDR_FMEM_HI  0x000003FE /* Rx filter memory end */
#define DP_RFCR_ULM             0x00080000 /* U/L bit Mask */
#define DP_RFCR_UHEN            0x00100000 /* Unicast Hash Enable */
#define DP_RFCR_MHEN            0x00200000 /* Multicast Hash Enable */
#define DP_RFCR_AARP            0x00400000 /* Accept ARP Packets */
#define DP_RFCR_APAT            0x07800000 /* Accept On Pattern Match */
#define DP_RFCR_APM             0x08000000 /* Accept on Perfect match */
#define DP_RFCR_AAU             0x10000000 /* Accept All Unicast */
#define DP_RFCR_AAM             0x20000000 /* Accept All Multicast */
#define DP_RFCR_AAB             0x40000000 /* Accept All Broadcast */
#define DP_RFCR_RFEN            0x80000000 /* Rx Filter Enable */

/*
 * Receive Filter/Match Data Register Bit Masks (DP_RFDR)
 *
 * This register is used to read and write internal receive filter registers,
 * the pattern buffer memory and the hash table memory.
 */

#define DP_RFDR_RFDATA          0x0000FFFF /* Receive Filter data */
#define DP_RFDR_BMASK           0x00030000 /* Byte Mask */

/* Boot ROM Address Register Bit Masks (DP_BRAR)   */

#define DP_BRAR_ADDR            0x0000FFFF /* Boot ROM Address */
#define DP_BRAR_AUTOINC         0x80000000 /* Auto-Increment */

/* Boot ROM Data Register Bit Masks (DP_BRDR)   */

#define DP_BRDR_DATA            0xFFFFFFFF /* Boot ROM Data */

/* Silicon Revision Register Bit Masks (DP_SRR)   */


/* Different in Data sheets I have   */

#define DP_SRR_MIN              0x000000FF /* Minor Revision Level */
#define DP_SRR_MAJ              0x0000FF00 /* Major Revision Level */
#define DP_SRR_MAJ_SHF          8               /* Shift bits */


/*
 * Management Information Base Control Register Bit Masks (DP_MIBC)
 *
 * It is used to control access to the statistics block and the warning bits
 * and to control the collection of management info statistics.
 */

#define DP_MIBC_WRN             0x00000001 /* Warning Tst Indicator (RO) */
#define DP_MIBC_FRZ             0x00000002 /* Freeze All Counters */
#define DP_MIBC_ACLR            0x00000004 /* Clear all Counters */
#define DP_MIBC_MIBS            0x00000008 /* MIB Counter Strobe (TEST) */

/* BMCR - (Internal Phy) Basic Mode Control Register   */

#define DP_BMCR_COL_TST         0x0080 /* Collision Test */
#define DP_BMCR_HDX             0x0000 /* Half duplex mode */
#define DP_BMCR_FDX             0x0100 /* Full duplex mode */
#define DP_BMCR_ANEG_RES        0x0200 /* Restart Auto negotiation */
#define DP_BMCR_ISOLATE         0x0400 /* Isolate */
#define DP_BMCR_PWRDWN          0x0800 /* Power Down */
#define DP_BMCR_ANEG_EN         0x1000 /* Auto Negotiation Enable */
#define DP_BMCR_SPD_100         0x2000 /* Speed Select 100Mbps */
#define DP_BMCR_SPD_10          0x0000 /* Speed Select 10Mbps */
#define DP_BMCR_LOOP            0x4000 /* Loopback */
#define DP_BMCR_RESET           0xB100 /* Reset */

/* BMSR - (Internal Phy) Basic Mode Status Register   */

#define DP_BMSR_XREG_ABLE       0x0001 /* Extended Register Capability */
#define DP_BMSR_JABR_DET        0x0002 /* Jabber Detected */
#define DP_BMSR_LNK_VALID       0x0004 /* Valid Link Status */
#define DP_BMSR_AN_ABLE         0x0008 /* Auto-Neg Ability */
#define DP_BMSR_REM_FLT         0x0010 /* Remote Fault Detected */
#define DP_BMSR_AN_DONE         0x0020 /* Auto Nego Complete */
#define DP_BMSR_PRS_ABLE        0x0040 /* Preamble Supr Capable */
#define DP_BMSR_10_HD_ABLE      0x0800 /* 10BASE-T Half Duplex Capable */
#define DP_BMSR_10_FD_ABLE      0x1000 /* 10BASE-T Full Duplex Capable */
#define DP_BMSR_100_HD_ABLE     0x2000 /* 100BASE-TX Half Duplex Capable */
#define DP_BMSR_100_FD_ABLE     0x4000 /* 100BASE-TX Full Duplex Capable */
#define DP_BMSR_100T4_ABLE      0x8000 /* 100BASE -T4 Capable */

/* PHY Identifier Register #1   */

#define DP_PHYIDR1_OUI_MSB      0xFFFF /* OUI Most significant Bits */

/* PHY Identifier Register #2   */

#define DP_PHYIDR2_MDL_REV      0x000F /* Model Revision number */
#define DP_PHYIDR2_VNDR_MDL     0x03F0 /* Vendor Model Number */
#define DP_PHYIDR2_OUI_LSB      0xFC00 /* OUI Least Significant Bits */

/*
 * Auto-Negotiation Advertisement Register
 *
 * Contains the advertised abilities of this device as they will be transmitted
 * to its link partner during Auto-Negotiation.
 */

#define DP_ANAR_SEL             0x001F /* Protocol Selection Bits */
#define DP_ANAR_10T             0x0020 /* 10BASE-T Support */
#define DP_ANAR_10_FD           0x0040 /* 10BASE-T Full Duplex Support */
#define DP_ANAR_TX              0x0080 /* 100BASE-TX Support */
#define DP_ANAR_TX_FD           0x0100 /* 100BASE-TX Full Duplex Support */
#define DP_ANAR_T4              0x0200 /* 100BASE-T4 Support */
#define DP_ANAR_PAUSE           0x0400 /* Pause */
#define DP_ANAR_RF              0x2000 /* Remote Fault */
#define DP_ANAR_NP              0x8000 /* Next Page Indication */


/*
 * Auto-Negotiation Link Partner Ability Register
 *
 * Contains the advertised abilities of the Link Partner as received during
 * Auto Negotiation. The content changes after the successful autonegotiation
 * if Next-Pages are supported.
 */

#define DP_ANLPAR_SEL           0x001F /* Protocol Selection Bits */
#define DP_ANLPAR_10T           0x0020 /* 10BASE-T Support */
#define DP_ANLPAR_10_FD         0x0040 /* 10BASE-T Full Duplex */
#define DP_ANLPAR_TX            0x0080 /* 100BASE-TX Support */
#define DP_ANLPAR_TX_FD         0x0100 /* 100BASE-TX Full Duplex */
#define DP_ANLPAR_T4            0x0200 /* 100BASE-T4 Support */
#define DP_ANLPAR_RF            0x2000 /* Remote Fault */
#define DP_ANLPAR_ACK           0x4000 /* Acknowledge */
#define DP_ANLPAR_NP            0x8000 /* Next Page Indication */

/*
 * Auto-Negotiation Expansion Register
 *
 * contains additional Local device and Link Partner status info
 */

#define DP_ANER_LP_AN_ABLE      0x0001 /* Link Partner Auto Neg Able */
#define DP_ANER_PAGE_RX         0x0002 /* Link Code Word Page Recvd */
#define DP_ANER_NP_ABLE         0x0004 /* Next Page Able */
#define DP_ANER_LP_NP_ABLE      0x0008 /* Link Partner NextPage Able */
#define DP_ANER_PDF             0x0010 /* Parallel Detection Fault */

/*
 * Auto-Negotiation Next Page Transmit Register
 *
 * contains the next page Info sent by this device to its Link Partner
 * during Auto-Negotiation
 */

#define DP_ANNPTR_CODE          0x07FF /* Code Field */
#define DP_ANNPTR_TOG_TX        0x0800 /* Toggle */
#define DP_ANNPTR_ACK2          0x1000 /* Acknowledge2 */
#define DP_ANNPTR_MP            0x2000 /* Message Page */
#define DP_ANNPTR_NP            0x8000 /* Next Page Indication */

/*
 * PHY Status Register
 *
 * provides a single location within the register set for quick access to
 * commonly accessed information
 */

#define DP_PHYSTS_LNK_VALID     0x0001 /* Valid Link */
#define DP_PHYSTS_SPEED_10      0x0002 /* 10 Mbps Mode */
#define DP_PHYSTS_FDX           0x0004 /* Full Duplex Mode */
#define DP_PHYSTS_LOOP          0x0008 /* Loopback Enabled */
#define DP_PHYSTS_ANEG_DONE     0x0010 /* Auto-Neg Complete */
#define DP_PHYSTS_JABBER        0x0020 /* Jabbler Detected */
#define DP_PHYSTS_REM_FAULT     0x0040 /* Remote Fault Detected */
#define DP_PHYSTS_MII_INTR      0x0080 /* MII Interrupt Pending */
#define DP_PHYSTS_LCWP_RX       0x0100 /* Link Code Word Page Rx'd */
#define DP_PHYSTS_DSCRMBL_LCK   0x0200 /* 100TX Descrambler Lock */
#define DP_PHYSTS_SIG_DET       0x0400 /* 100TX Uncond Signal Detect */
#define DP_PHYSTS_FCSL          0x0800 /* False Carrier Sense Latch */
#define DP_PHYSTS_POL_INV       0x1000 /* Polarity status */
#define DP_PHYSTS_RX_ERR_LATCH  0x2000 /* Received error latch */

/*
 * False carrier Sense Counter Register
 *
 * provides info required to implement the "FalseCarriers" attribute within
 * the MAJ managed object class of Clause 30 of the IEEE 802.3u specification.
 */

#define DP_FCSCR_FCSCNT         0x00FF /* False Carrier Event Counter */

/*
 * Receiver Error Counter Register
 *
 * provides info required to implement the "SymbolErrorDuringCarrier" attribute
 * within  the PHY managed object class of Clause 30 of the IEEE 802.3u
 * specification.
 */

#define DP_RECR_RXERCNT         0x00FF /* RX_ER Counter*/

/* 100Mb/s PCS Configuration and Status Register   */

#define DP_PCSR_NRZI_BYP        0x0004 /* NRZI Bypass Enable */
#define DP_PCSR_FRC_100_OK      0x0020 /* Force 100Mb/s Good Link */
#define DP_PCSR_SD_OPT          0x0100 /* Signal Detect Option */
#define DP_PCSR_SD_F_B          0x0200 /* Signal Detect Force */
#define DP_PCSR_TQ_EN           0x0400 /* 100Mbs True Quite Mode En */
#define DP_PCSR_FREE_CLK        0x0800 /* Receive Clock */
#define DP_PCSR_BYP_4B5B        0x1000 /* Bypass 4B/5B Encoding */

/* PHY Control Register   */

#define DP_PHYCR_PHYADDR        0x001F /* PHY Address */
#define DP_PHYCR_LED_CFG        0x0060 /* LED Configuration */
#define DP_PHYCR_LED_CFG_10_HI  0x0000 /* Speed10 HIGH */
#define DP_PHYCR_LED_CFG_10     0x0020 /* Speed10 selected */
#define DP_PHYCR_LED_CFG_DPLXHI 0x0040 /* DPLX active HIGH */
#define DP_PHYCR_LED_CFG_DPLX   0x0060 /* DPLX selected */
#define DP_PHYCR_PAUSE_PASS     0x0080 /* Pause Compare Pass */
#define DP_PHYCR_BP_STRETCH     0x0100 /* Bypass LED Stretch*/
#define DP_PHYCR_BIST_START     0x0200 /* BIST Start */
#define DP_PHYCR_BIST_PASS      0x0400 /* BIST Pass */
#define DP_PHYCR_PSR_15         0x0800 /* BIST Sequence Sel PSR15 (PSR9) */

/* 10Base-T Status/Control Register(10BTSCR)   */

#define DP_10BTSCR_JABR_DIS     0x0001 /* Jabber Disable */
#define DP_10BTSCR_HB_DIS       0x0002 /* Heartbeat Disable */
#define DP_10BTSCR_LOW_SQL      0x0004 /* Reduced Sqyelch Enable */   /* ?? this not in my data sheet */
#define DP_10BTSCR_AUTOPOL_DIS  0x0008 /* Auto Polarity Disable */
#define DP_10BTSCR_POL          0x0010 /* 10Mb Polarity Status */
#define DP_10BTSCR_FRC_POL_COR  0x0020 /* Force 10Mb Polarity Correction */
#define DP_10BTSCR_FRC_10       0x0040 /* Force 10Mb Good Link */
#define DP_10BTSCR_LP_DIS       0x0080 /* Normal Link Pulse Disable */
#define DP_10BTSCR_LB10_DIS     0x0100 /* 10Base-T Loopback Disable */



/* Download and Transmission   */
#undef TX_RING_SIZE
#define TX_RING_SIZE    4  /* Must be a power of 2 */
                           /* we don't need many since we send only one at a time       */
#define TX_RING_MASK    (TX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */


/* Descriptor Layout   */

#define DP_DESC_LNK             0x00 /* Link field offset */
#define DP_DESC_CMDSTS          0x04 /* Command & Status offset */
#define DP_DESC_BUFPTR          0x08 /* Buffer pointer offset */



/* The transmit descriptor list is made up of the following structures.
   DP83815 only supports a single fragment per descriptor.
   Descriptors must be 32bit aligned. */
typedef struct txdsc_83815
{
    dword link;   /* 32bit "link" field to next descr in linked list, bits1-0 must be 0 */
                  /* since descriptors must be aligned on 32bit boundaries   */
    dword cmdsts;   /* 32bit Command/Status Field (bit-encoded) */
    dword bufptr;   /* 32bit pointer to the 1st fragment or buffer. For transmit,descriptors */
                  /* the buffer can begin on any byte boundary   */
} TXDSC_83815;
typedef  struct txdsc_83815 KS_FAR *PTXDSC_83815;


/* Reception and Upload   */

#undef RX_RING_SIZE
#define RX_RING_SIZE    16  /* Must be a power of 2 */
#define RX_RING_MASK    (RX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */

/* The receive descriptor list is made up of the following structures.
   DP83815 only supports a single fragment per descriptor.
   Descriptors must be 32bit aligned. */

typedef struct rxdsc_83815 {
    dword link;
    dword cmdsts;
    dword bufptr;
} RXDSC_83815;
typedef  struct rxdsc_83815 KS_FAR *PRXDSC_83815;


/* DP_DESC_CMDSTS - Descriptor Command and Status Definitions   */

#define DP_DESC_CMDSTS_SIZE         0x00000FFF /* Size of data in bytes */
#define DP_DESC_CMDSTS_TX_CCNT      0x000F0000 /* Collision Count */
#define DP_DESC_CMDSTS_TX_EC        0x00100000 /* Excessive Collisions */
#define DP_DESC_CMDSTS_TX_OWC       0x00200000 /* Out of window collns */
#define DP_DESC_CMDSTS_TX_ED        0x00400000 /* Excessive deferrals */
#define DP_DESC_CMDSTS_TX_TD        0x00800000 /* Transmit deferrals */
#define DP_DESC_CMDSTS_TX_CRS       0x01000000 /* Carrier sense lost */
#define DP_DESC_CMDSTS_TX_TFU       0x02000000 /* Tx FIFO underrun */
#define DP_DESC_CMDSTS_TX_TXA       0x04000000 /* Tx abort */
#define DP_DESC_CMDSTS_RX_COL       0x00010000 /* Collision */
#define DP_DESC_CMDSTS_RX_LBP       0x00020000 /* Loopback packet */
#define DP_DESC_CMDSTS_RX_FAE       0x00040000 /* Frame align error */
#define DP_DESC_CMDSTS_RX_CRCE      0x00080000 /* CRC error */
#define DP_DESC_CMDSTS_RX_ISE       0x00100000 /* Invalid symbol error */
#define DP_DESC_CMDSTS_RX_RUNT      0x00200000 /* Runt packet */
#define DP_DESC_CMDSTS_RX_LONG      0x00400000 /* Long packet */
#define DP_DESC_CMDSTS_RX_DEST      0x01800000 /* Destination Class */
#define DP_DESC_CMDSTS_RX_DEST_REJ  0x00000000 /*  Packet Rejected */
#define DP_DESC_CMDSTS_RX_DEST_UNI  0x00800000 /*  Unicast packet */
#define DP_DESC_CMDSTS_RX_DEST_MC   0x01000000 /*  Multicast packet */
#define DP_DESC_CMDSTS_RX_DEST_BC   0x01800000 /*  Broadcast packet */
#define DP_DESC_CMDSTS_RX_RXO       0x02000000 /* Receive overrun */
#define DP_DESC_CMDSTS_RX_RXA       0x04000000 /* Receive aborted */
#define DP_DESC_CMDSTS_OK           0x08000000 /* Packet OK */
#define DP_DESC_CMDSTS_TX_SUPCRC    0x10000000 /* Supress CRC */
#define DP_DESC_CMDSTS_RX_INCCRC    0x10000000 /* Include CRC */
#define DP_DESC_CMDSTS_INTR         0x20000000 /* Interrupt */
#define DP_DESC_CMDSTS_MORE         0x40000000 /* More descriptors */
#define DP_DESC_CMDSTS_OWN          0x80000000 /* Desc owner (consumer) */



/* PCI Device Identification information.   */
struct pci_id_info_83815
{
    const char *dev_name;
    word    vendor_id;
    word    device_id;
};

typedef struct _N83815_softc
{
   PIFACE       iface;
   int          device_index;
   IOADDRESS    ia_iobase;
   int          ia_irq;
   PFBYTE       ptxringbuf; /* Pointer to alloced TX descriptor ring buffer with alignment room */
   PTXDSC_83815 ptxring[TX_RING_SIZE]; /* Pointer to above but 32bit aligned */
   int          last_tx_done;       /* last tx entry with processing complete */
   int          this_tx;            /* next tx to use */
   DCU          tx_dcus[TX_RING_SIZE];       /* keep track of the DCU's we send */
   PFBYTE       prxringbuf; /* Pointer to alloced RX descriptor ring buffer with alignment room */
   PRXDSC_83815 prxring[RX_RING_SIZE]; /* Pointer to above but 32bit aligned */
   int          cur_rx;     /* Index into the Rx buffer of next Rx pkt. */
   int          last_rx;
   DCU          rx_dcus[RX_RING_SIZE];       /* keep track of the DCU's we receive */

   dword int_stat;
   dword int_mask;
   dword  tx_config;      /* Contents of the transmitter configuration register */
   dword  rx_config;      /* Driver/NIC configuration */

   /* Operating Mode:                  */
   /* interrupt statistics counters    */
   dword n_int_call_total; /* total interrupt service calls (may handle multiple ints) */
   dword n_ints_total;     /* total interrupts including errors,rx, and tx */
   dword n_tx_ints_total;  /* total tx interrupts including tx error interrupts */
   dword n_rx_pkts_total;  /* total rx interrupts including rx error interrupts */
   dword n_errors_total;   /* total non rx/tx error interrupts */

   EBS_TIMER timer_info;           /* timer information  */
   dword       cur_ticks;          /* incremented every second */
   dword       last_rx_ticks;      /* saved every time a packet is received */

   struct ether_statistics stats;
} N83815_SOFTC;
typedef struct _N83815_softc KS_FAR *PN83815_SOFTC;

#define off_to_N83815_softc(X)  (X) >= CFG_NUM_N83815 ? (PN83815_SOFTC)0 : N83815softc[(X)]
#define iface_to_N83815_softc(X) (X)->minor_number >= CFG_NUM_N83815 ? (PN83815_SOFTC)0 : N83815softc[(X)->minor_number]

RTIP_BOOLEAN N83815_open(PIFACE pi);
void    N83815_close(PIFACE pi);
int     N83815_xmit(PIFACE pi, DCU msg);
RTIP_BOOLEAN N83815_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN N83815_statistics(PIFACE  pi);
RTIP_BOOLEAN N83815_setmcast(PIFACE pi);

#if (DECLARING_DATA || BUILD_NEW_BINARY)
/* ********************************************************************   */
/* GLOBAL DATA                                                            */
/* ********************************************************************   */

#if (SET_MAC_ADDR)
    byte mac_addr[6] = {0x00,0x12,0x34,0x56,0x78,0x9a};
#endif

struct pci_id_info_83815 pci_tab_83815[2] =
{{ "National Etherlink 10/100 card",RTPCI_V_ID_NS, RTPCI_D_ID_83815},
 {0,},                      /* 0 terminated list. */
};

/* Note. these are now pointers. We allocate them from the DCU
  pool since portions of them are in memory space shared by PCI/HOST */
PN83815_SOFTC N83815softc[CFG_NUM_N83815];

EDEVTABLE KS_FAR n83815_device =
{
    N83815_open, N83815_close, N83815_xmit, N83815_xmit_done,
    NULLP_FUNC, N83815_statistics, N83815_setmcast,
    N83815_DEVICE, "N83815", MINOR_0, ETHER_IFACE,
    SNMP_DEVICE_INFO(CFG_OID_N83815, CFG_SPEED_N83815)
    CFG_ETHER_MAX_MTU, CFG_ETHER_MAX_MSS, 
    CFG_ETHER_MAX_WIN_IN, CFG_ETHER_MAX_WIN_OUT, 
    IOADD(0x200), EN(0), EN(0xa)
};
#endif  /* DECLARING_DATA */

#if (!DECLARING_DATA)   /* exclude rest of file */
/* ********************************************************************   */
/* EXTERNS                                                                */
/* ********************************************************************   */
#if (!BUILD_NEW_BINARY)
#if (SET_MAC_ADDR)
extern byte mac_addr[6];
#endif
extern struct pci_id_info_83815 pci_tab_83815[2];
extern PN83815_SOFTC N83815softc[CFG_NUM_N83815];
extern EDEVTABLE KS_FAR n83815_device;
#endif

/* ********************************************************************   */
int N83815_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr);
static void N83815_interrupt(int deviceno);
static void N83815_pre_interrupt(int deviceno);
void N83815_rcv_ring(PN83815_SOFTC sc);
void N83815_getmcaf(PFBYTE mclist, int lenmclist, word *hash_table);
RTIP_BOOLEAN N83815_open_device(PIFACE pi);

/* ********************************************************************   */
RTIP_BOOLEAN N83815_setmcast(PIFACE pi)      /* __fn__ */
{
PN83815_SOFTC sc;
IOADDRESS io_address;
int i;
word hash_table[32];
dword flags;

    sc = iface_to_N83815_softc(pi);
    if (!sc)
        return(FALSE);
    io_address = sc->ia_iobase;
    /* Set multicast filter on chip.               */
    /* If none needed lenmclist will be zero       */

    /* the driver can only handle 32 addresses; if there are more than      */
    /* 32 then accept all addresses; the IP layer will filter the           */
    /* packets                                                              */
    if (pi->mcast.lenmclist > 32)
    {
        flags = DP_RFCR_AAM;
    }

    else
    {

        N83815_getmcaf((PFBYTE) pi->mcast.mclist, pi->mcast.lenmclist,
                        hash_table);

    /* create hash table in memory (64 bytes addressed on word boundaries   */
        for(i=0; i<32; i++)
        {
            N83815_OUTDWORD(io_address + DP_RFCR, DP_RFCR_RFADDR_FMEM_LO + i*2);
            N83815_OUTDWORD(io_address + DP_RFDR, hash_table[i]);
        }

        flags = DP_RFCR_MHEN;
    }

    N83815_OUTDWORD(io_address + DP_RFCR, 0);
    N83815_OUTDWORD(io_address + DP_RFCR, flags);
    return(TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN N83815_open(PIFACE pi)
{
PN83815_SOFTC sc;
IOADDRESS io_address;
PFBYTE p;
dword l;
int i;

    if (!N83815softc[pi->minor_number])
    {
        p = (PFBYTE) dcu_alloc_core(sizeof(*sc)+4);
        /* make sure on 4 byte boundary first     */
        l = (dword) p;
        while (l & 0x3ul) {l++; p++;};
        N83815softc[pi->minor_number] = (PN83815_SOFTC) p;
    }


    sc = iface_to_N83815_softc(pi);
    if (!sc)
        return(FALSE);
    tc_memset(sc, 0, sizeof(*sc));
    sc->iface = pi;

    /* Find a 83815 type device indexed by minor number. The init code also wakes up the
       device from a power down state */
    sc->device_index = N83815_pci_init(pi, &sc->ia_irq, &sc->ia_iobase);
    if (sc->device_index<0)
        return(FALSE);

    /* save in PI structure   */
    pi->io_address = io_address = sc->ia_iobase;
    pi->irq_val    = sc->ia_irq;

    /* Mask off interrupts and disable   */
    N83815_OUTDWORD(io_address + DP_IER, DP_IER_ID);
    N83815_OUTDWORD(io_address + DP_IMR,DP_IMR_ALLOFF);

    /* Do a soft reset which also resets the tx and rx   */
    N83815_OUTDWORD(io_address + DP_CR, DP_CR_RST);
    i = 0;
    do
    {
        if ( N83815_INDWORD(io_address + DP_CR) & DP_CR_RST )
        {
            ks_yield();
        }
        else
            break;
        i++;
    }
    while (i < 100);
    if ( i == 100)
        return(FALSE);

    /* load config from EEPROM   */
    N83815_OUTDWORD(io_address+DP_PTSCR, DP_PTSCR_EELOAD_EN);
    i = 0;
    do
    {
        if (N83815_INDWORD(io_address+DP_PTSCR) & DP_PTSCR_EELOAD_EN)
            ks_sleep(1);
        else
            break;
        i++;
    } while (i < 100);
    if (i == 100)
        return(FALSE);

    /* Hook the interrupt service routines   */
    ks_disable();
#if (RTIP_VERSION > 24)
    ks_hook_interrupt(sc->ia_irq, (PFVOID) pi,
                      (RTIPINTFN_POINTER)N83815_interrupt,
                      (RTIPINTFN_POINTER) (RTIPINTFN_POINTER)N83815_pre_interrupt,
                      pi->minor_number);
#else
    ks_hook_interrupt(sc->ia_irq, (RTIPINTFN_POINTER)N83815_interrupt, pi->minor_number);
#endif

    ks_enable();

    /* Call lower level device open function to allocate and
       format shared memory structures and to set up the chip and registers.
       These have been put in a routine which may also be called from txreset (xmit_done) */
   return(N83815_open_device(pi));
}


/* ********************************************************************   */
RTIP_BOOLEAN N83815_open_device(PIFACE pi)
{
PN83815_SOFTC sc;
IOADDRESS io_address;
PFBYTE p;
dword l;
int i,j;
byte mac_temp[6];
word *pmac_temp = (word *)mac_temp;
dword current_rfcr;

    sc = iface_to_N83815_softc(pi);
    if (!sc)
        return(FALSE);
    io_address = sc->ia_iobase;

    /* Read the MAC address from the perfect filter registers or    */
    /* set the MAC address depending on configuration selection     */
#if (SET_MAC_ADDR)
    pi->addr.my_hw_addr[0] = mac_temp[0] = mac_addr[0];
    pi->addr.my_hw_addr[1] = mac_temp[1] = mac_addr[1];
    pi->addr.my_hw_addr[2] = mac_temp[2] = mac_addr[2];
    pi->addr.my_hw_addr[3] = mac_temp[3] = mac_addr[3];
    pi->addr.my_hw_addr[4] = mac_temp[4] = mac_addr[4];
    pi->addr.my_hw_addr[5] = mac_temp[5] = mac_addr[5];
    for(i=0; i<3; i++)
    {
        N83815_OUTDWORD(io_address + DP_RFCR, DP_RFCR_RFADDR_PMATCH1 + i*2);
        N83815_OUTWORD(io_address + DP_RFDR, *pmac_temp );
        pmac_temp++;
    }
#else
    for(i=0; i<3; i++)
    {
        N83815_OUTDWORD(io_address + DP_RFCR, DP_RFCR_RFADDR_PMATCH1 + i*2);
        *pmac_temp = N83815_INWORD(io_address + DP_RFDR);
        pmac_temp++;
    }
    pi->addr.my_hw_addr[0] = mac_temp[0];
    pi->addr.my_hw_addr[1] = mac_temp[1];
    pi->addr.my_hw_addr[2] = mac_temp[2];
    pi->addr.my_hw_addr[3] = mac_temp[3];
    pi->addr.my_hw_addr[4] = mac_temp[4];
    pi->addr.my_hw_addr[5] = mac_temp[5];
#endif
    /* Set Up Transmitter   */
    /* set up 32 bit aligned pointers to the tx descriptor ring entries and initialize
       the current and last index. The transmit routine will do most of the work
       when we get a packet to send. */
    sc->ptxringbuf =  dcu_alloc_core(TX_RING_SIZE*12 + 4);  /* desc 12 bytes long + 4 for alignment room*/
    if (sc->ptxringbuf == NULL)    return(FALSE);

    p = sc->ptxringbuf;
    l = (dword) p;
    while (l & 0x3ul) {l++; p++;};   /* insures 4 byte alignment */
    sc->ptxring[0] = (PTXDSC_83815) p;
    for (i=1; i<TX_RING_SIZE; i++)
      sc->ptxring[i] = sc->ptxring[i-1] + 1; /* descriptors are 12 bytes long */
    sc->this_tx = 0;
    sc->last_tx_done = 0;
    /* build the ring structures   */
    for (i=0; i<TX_RING_SIZE; i++)
    {
        if (i+1 < TX_RING_SIZE)
              sc->ptxring[i]->link = SWAPIF32(kvtop((PFBYTE) sc->ptxring[i+1]));
        sc->ptxring[i]->cmdsts = SWAPIF32(0x00000000ul);
        sc->ptxring[i]->bufptr = SWAPIF32(0x00000000ul);
    }
    sc->ptxring[TX_RING_SIZE-1]->link = SWAPIF32(kvtop((PFBYTE) sc->ptxring[0]));
    /* address of first desc in list   */
    N83815_OUTDWORD(io_address + DP_TXDP, SWAPIF32(kvtop((PFBYTE) sc->ptxring[0])));
    /* Transmitter configuration   */
    sc->tx_config = DP_TXCFG_MXDMA_32 |
                    DP_TXCFG_ATP |
                    DP_TXCFG_HBI |
                    DP_TXCFG_DRTH_VAL |
                    DP_TXCFG_FLTH_VAL;

    N83815_OUTDWORD(io_address + DP_TXCFG, SWAPIF32(sc->tx_config));

    /* Set Up Receiver   */
    /* set up 32 bit aligned pointers to the rx descriptor ring entries and initialize
       the current and last index. Preallocate receive buffers. */
    sc->prxringbuf =  dcu_alloc_core(RX_RING_SIZE*12 + 4);
    if (sc->prxringbuf == NULL)
    {
      dcu_free_core(sc->ptxringbuf);
      return(FALSE);
    }
    p = sc->prxringbuf;
    l = (dword) p;
    while (l & 0x3ul) {l++; p++;};
    sc->prxring[0] = (PRXDSC_83815) p;
    for (i=1; i<RX_RING_SIZE; i++)
         sc->prxring[i] = sc->prxring[i-1] + 1; /* descriptors are 12 bytes long */
    sc->cur_rx = 0;
    sc->last_rx = RX_RING_SIZE-1;
    /* build the ring structures   */
    for (i=0; i<RX_RING_SIZE; i++)
    {
        sc->rx_dcus[i] = os_alloc_packet_input(ETHERSIZE + 4, DRIVER_ALLOC);
        /* buffer must be aligned on a 32 bit boundary   */
        p = DCUTODATA(sc->rx_dcus[i]);
        l = (dword)p  & ~0x3ul;
        if (!sc->rx_dcus[i] || (l != (dword)p))
        {
            DEBUG_ERROR("N83815: Failure allocating RX DCUS", 0, 0, 0);
            for (j = 0; j < i; j++)
                os_free_packet(sc->rx_dcus[j]);
            dcu_free_core(sc->ptxringbuf);
            dcu_free_core(sc->prxringbuf);
            return(FALSE);
        }
        if (i+1 < RX_RING_SIZE)
              sc->prxring[i]->link = SWAPIF32(kvtop((PFBYTE) sc->prxring[i+1]));
        sc->prxring[i]->cmdsts = SWAPIF32(ETHERSIZE) + 4; /* PP */
      sc->prxring[i]->bufptr =
                           SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(sc->rx_dcus[i])));
    }
    sc->prxring[RX_RING_SIZE-1]->link = SWAPIF32(kvtop((PFBYTE) sc->prxring[0]));
    /* address of first desc in list   */
    N83815_OUTDWORD(io_address + DP_RXDP, SWAPIF32(kvtop((PFBYTE) sc->prxring[0])));
    /* Receiver configuration   */
    sc->rx_config = DP_RXCFG_MXDMA_32 | DP_RXCFG_DRTH_VAL;

    /* Set the mode to accept multicast, broadcast, perfect match, and unicast.   */

    if (!N83815_setmcast(pi))  /* multicast done in this routine */
    {
        DEBUG_ERROR("N83815: Failure setting Multicast filters", 0, 0, 0);
        for (j = 0; j < i; j++)
            os_free_packet(sc->rx_dcus[j]);
        dcu_free_core(sc->ptxringbuf);
        dcu_free_core(sc->prxringbuf);
        return(FALSE);
    }

    current_rfcr = N83815_INDWORD(io_address + DP_RFCR);
    N83815_OUTDWORD(io_address + DP_RFCR, (current_rfcr |
                                           DP_RFCR_AAB  | /* all broadcast packets */
                                     DP_RFCR_APM  | /* perfect match packets */
                             DP_RFCR_AAU  | /* unicast packets */
                                     DP_RFCR_RFEN));

    /* Enable rx/tx, mask interrupts, and enable interrupts   */
    N83815_OUTDWORD(io_address + DP_CR, DP_CR_RXE | DP_CR_TXE);

    sc->int_mask =  (DP_IMR_RXORN |
                     DP_IMR_RXOK  |
                     DP_IMR_RXERR |
                   DP_IMR_TXURN |
                   DP_IMR_TXOK  |
                     DP_IMR_TXERR |
                   DP_IMR_SSERR);
    N83815_OUTDWORD(io_address + DP_IMR, sc->int_mask);

    N83815_OUTDWORD(io_address + DP_IER, DP_IER_IE);
    return(TRUE);
}

/* ********************************************************************   */
RTIP_BOOLEAN N83815_statistics(PIFACE pi)                       /*__fn__*/
{
PETHER_STATS p;
PN83815_SOFTC sc;

    sc = iface_to_N83815_softc(pi);
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
void N83815_close(PIFACE pi)                     /*__fn__*/
{
PN83815_SOFTC sc;
IOADDRESS io_address;
    int i;

    sc = iface_to_N83815_softc(pi);
    if (!sc)
        return;
    io_address = sc->ia_iobase;

    /* Mask off interrupts and disable   */
    N83815_OUTDWORD(io_address + DP_IER, DP_IER_ID);
    N83815_OUTDWORD(io_address + DP_IMR,DP_IMR_ALLOFF);

    /* Do a soft reset which also resets(and disables) the tx and rx   */
    N83815_OUTDWORD(io_address + DP_CR, DP_CR_RST);

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
RTIP_BOOLEAN N83815_xmit_done(PIFACE pi, DCU msg, RTIP_BOOLEAN success)
{
PN83815_SOFTC sc;

    sc = iface_to_N83815_softc(pi);
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
      /* We're in serious troble here. Reset and reinitialize the chip   */
      /* Also record statistics                                          */
      sc->stats.errors_out++;
      sc->stats.tx_other_errors++;

      /* call the device close routine which will free resources and shut down the chip   */
      N83815_close(pi);
      return(N83815_open_device(pi));
    }
}

/* ********************************************************************   */
/* Transmit Routine.                                                      */
int N83815_xmit(PIFACE pi, DCU msg)    /*__fn__*/
{
IOADDRESS io_address;
PN83815_SOFTC sc;
int   length;
int this_tx;

    sc = iface_to_N83815_softc(pi);
    if (!sc)
        return(ENUMDEVICE);
    io_address = sc->ia_iobase;

    length = DCUTOPACKET(msg)->length;
    if (length < ETHER_NOCHK_MIN_LEN)
    length = ETHER_NOCHK_MIN_LEN;
    if (length > ETHERSIZE)
    {
      DEBUG_ERROR("N83815:xmit - length is too large", NOVAR, 0, 0);
      length = ETHERSIZE;         /* what a terriable hack! */
    }

    this_tx = sc->this_tx++;
    sc->this_tx &= TX_RING_MASK;  /* Wrap to zero if must */

/* check that we havn't wrapped over an incomplete xmit buffer   */
    if (sc->this_tx == sc->last_tx_done)
    {
        DEBUG_ERROR("TX Descriptor not owned ", NOVAR , 0, 0);
        return(EOUTPUTFULL);
    }

    sc->tx_dcus[this_tx] = msg;

/* set packet length and mark it as last fragment   */
    sc->ptxring[this_tx]->bufptr = SWAPIF32(kvtop((PFBYTE)DCUTODATA(msg)));
    sc->ptxring[this_tx]->cmdsts = SWAPIF32(((dword) length) | DP_DESC_CMDSTS_OWN);
    N83815_OUTDWORD(io_address + DP_CR, DP_CR_RXE | DP_CR_TXE);

    return(0);
}

/* ********************************************************************   */
void N83815_pre_interrupt(int deviceno)
{
PN83815_SOFTC sc;

    sc = off_to_N83815_softc(deviceno);
    if (!sc)
    {
        DEBUG_ERROR("N83815_pre_interrupt: Bad deviceno ", EBS_INT1,
            deviceno, 0);
        return;
    }

    /* The isr will be masked on again when the strategy routine called   */
    /* from the interrupt task returns                                    */
    DRIVER_MASK_ISR_OFF(sc->ia_irq);
}


/* ********************************************************************   */
void N83815_interrupt(int deviceno)
{
IOADDRESS io_address;
PN83815_SOFTC sc;
dword status;
dword cmdsts;
int last_tx_done;
int i;

    sc = off_to_N83815_softc(deviceno);
    if (!sc)
    {
        DEBUG_ERROR("N83815: Bad arg to isr", NOVAR , 0, 0);
        return;
    }
    sc->n_int_call_total += 1;

    io_address = sc->ia_iobase;
    status = N83815_INDWORD(io_address + DP_ISR);
    sc->int_stat = status; /* for debugging */
    /* Make sure we have a handler for each of the masked interrupt sources   */
    status &= sc->int_mask;

/* DEBUG_ERROR("N83815: INT status == ", DINT2 , (dword)sc->int_stat, (dword)status);   */


    for (i = 0; i < 10; i++) /* just in case we get stuck in here */
    {
      if (!status)
            break;

      sc->n_ints_total += 1;

      if (status & (DP_IMR_RXOK | DP_IMR_RXERR))     /* Packet received */
      {
        status &= ~(DP_IMR_RXOK | DP_IMR_RXERR);
        N83815_rcv_ring(sc);
      }

      if (status & DP_IMR_TXOK)     /* Successful Packet xmit */
      {
        sc->n_tx_ints_total += 1;
        status &= ~DP_IMR_TXOK;
        last_tx_done = sc->last_tx_done;
        while (last_tx_done != sc->this_tx)
        {
          if (sc->ptxring[last_tx_done]->cmdsts & DP_DESC_CMDSTS_OK)
                 ks_invoke_output(sc->iface, 1);
          last_tx_done++;
          last_tx_done &= TX_RING_MASK;  /* Wrap to zero if must */
          sc->last_tx_done = last_tx_done;
        }
      }
      if (status & DP_IMR_TXERR)     /* Un-successful Packet xmit */
      {
        sc->n_tx_ints_total += 1;
        status &= ~DP_IMR_TXERR;
        last_tx_done = sc->last_tx_done;
        while (last_tx_done != sc->this_tx)
        {
          cmdsts = sc->ptxring[last_tx_done]->cmdsts;
          if (!(cmdsts & DP_DESC_CMDSTS_TX_TXA))
                 ks_invoke_output(sc->iface, 1);

         DEBUG_ERROR("N83815 XMIT ERROR == ", EBS_INT1, (dword)cmdsts, 0);
            sc->stats.errors_out++;
            if (cmdsts & DP_DESC_CMDSTS_TX_EC)
                sc->stats.collision_errors++;
            else if (cmdsts & DP_DESC_CMDSTS_TX_OWC)
                sc->stats.owc_collision++;
            else if (cmdsts & DP_DESC_CMDSTS_TX_ED)
                sc->stats.tx_delayed++;
            else if (cmdsts & DP_DESC_CMDSTS_TX_TFU)
                sc->stats.tx_fifo_errors++;
            else if (cmdsts & DP_DESC_CMDSTS_TX_CRS)
                sc->stats.tx_carrier_errors++;
            else
                sc->stats.tx_other_errors++;

          last_tx_done++;
          last_tx_done &= TX_RING_MASK;  /* Wrap to zero if must */
          sc->last_tx_done = last_tx_done;
        }
      }
/* These are tx/rx errors indicated outside the link descriptor. It is assumed     */
/* that they resulted in a packet not being sent or received, therefore the        */
/* link descriptor status word is not updated. If not, some of the errors could    */
/* be double counted.                                                              */
      if (status & DP_ISR_SSERR)     /* System error(PCI error). We should probably reset!! */
      {
        sc->n_errors_total += 1;
        status &= ~DP_ISR_SSERR;
        DEBUG_ERROR("N83815: DP_ISR_SSERR error", NOVAR , 0, 0);
      }
      if (status & DP_ISR_RXORN)     /* Receive data FIFO overrun  */
      {
        sc->n_errors_total += 1;
        status &= ~DP_ISR_RXORN;
        sc->stats.rx_fifo_errors++;
        sc->stats.errors_in++;
        DEBUG_ERROR("N83815: RX DP_ISR_RXORN", NOVAR , 0, 0);
      }
      if (status & DP_IMR_TXURN)     /* Transmit data FIFO underrun*/
      {
        sc->n_errors_total += 1;
        status &= ~DP_IMR_TXURN;
        sc->stats.tx_fifo_errors++;
        sc->stats.errors_out++;
        DEBUG_ERROR("N83815: TX DP_IMR_TXURN", NOVAR , 0, 0);
      }
    }
    if (i == 10)
    {
      DEBUG_ERROR("N83815: >10 loops in ISR", NOVAR , 0, 0);
    }
/*DEBUG_ERROR("N83815: n_tx_ints , n_rcv_pkts", DINT2 , sc->n_tx_ints_total, sc->n_rx_pkts_total);        */
/*DEBUG_ERROR("N83815: n_ints ", DINT1 , sc->n_ints_total, 0);                                            */

    DRIVER_MASK_ISR_ON(sc->ia_irq);
    return;
}
/* ********************************************************************     */
/* Check status and receive data from the ring buffer (called from ISR).    */
/* We'll handle as many packets as are available while we are in here       */
void N83815_rcv_ring(PN83815_SOFTC sc)
{
int cur_rx, last_rx;
int loop_count = 0;
dword cmdsts;
int length;
DCU  msg, invoke_msg;

    cur_rx  = sc->cur_rx;
    last_rx = sc->last_rx;

    while ((cmdsts = SWAPIF32(sc->prxring[cur_rx]->cmdsts)) & DP_DESC_CMDSTS_OWN)
    {
        loop_count += 1;
        sc->n_rx_pkts_total += 1;
        /* Check if we know packet is ok. Otherwise we go on to check for other errors   */
        /* will be set if MORE BIT is set in the IP header - HERE                        */
        /* NOT set because MORE BIT is SET                                               */
        if (cmdsts & DP_DESC_CMDSTS_OK)
        {
            length = (int)(cmdsts & (dword) 0xfff);
            invoke_msg = 0;
            if (length <= RX_COPY_BREAK ) /* copy off if short packet */
            {
               invoke_msg = os_alloc_packet_input(length, DRIVER_ALLOC);
                if (invoke_msg)
                {
                  tc_movebytes((PFBYTE) DCUTODATA(invoke_msg),
                               (PFBYTE) DCUTODATA(sc->rx_dcus[cur_rx]),length);
                }
            }
            else /* process long packet with a new  DCU to the ring */
            {
                msg = os_alloc_packet_input(ETHERSIZE+4, DRIVER_ALLOC); /* PP */
                if (msg)
                {
                    /* Put the new one in the ring and invoke the old       */
                    invoke_msg = sc->rx_dcus[cur_rx];
                    sc->rx_dcus[cur_rx] = msg;
                    sc->prxring[cur_rx]->bufptr =
                           SWAPIF32(kvtop((PFBYTE)(PFBYTE)DCUTODATA(msg)));
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
            else /* trouble!! one of the alloc's failed. Record error if any use. */
            {
                DEBUG_ERROR("N83815 RCV ALLOC FAIL L == ", EBS_INT1, length, 0);
                sc->stats.rx_other_errors++;
                sc->stats.errors_in++;
            }
        }
        else /* input packet has some error*/
        {
            DEBUG_ERROR("I83815 RCV ERROR == ", DINT1, (dword)cmdsts, 0);
            if (cmdsts & DP_DESC_CMDSTS_RX_CRCE)
                sc->stats.rx_crc_errors++;
            else if (cmdsts & DP_DESC_CMDSTS_RX_FAE)
                sc->stats.rx_frame_errors++;
            else if (cmdsts & DP_DESC_CMDSTS_RX_LONG)
                sc->stats.rx_frame_errors++;
            else if (cmdsts & (DP_DESC_CMDSTS_RX_RXO | DP_DESC_CMDSTS_RX_RXA))
                sc->stats.rx_fifo_errors++;
            else if (cmdsts & DP_DESC_CMDSTS_RX_RUNT)
                sc->stats.rx_frame_errors++;
            else
                sc->stats.rx_other_errors++;

            sc->stats.errors_in++;
        }
        /* clear the status ownership bit to put this descriptor back in service. We
           should also reset the packet length */
/*DEBUG_ERROR("cmdst before ", DINT1, (dword)sc->prxring[cur_rx]->cmdsts, 0);   */
        sc->prxring[cur_rx]->cmdsts = (dword) SWAPIF32(ETHERSIZE) + 4; /* PP */
/*DEBUG_ERROR("ether ", DINT1, (dword)SWAPIF32(ETHERSIZE), 0);               */
/*DEBUG_ERROR("cmdst after ", DINT1, (dword)sc->prxring[cur_rx]->cmdsts, 0); */

        sc->last_rx_ticks = sc->cur_ticks;  /* pulse keepalive */
        last_rx  = cur_rx;
        cur_rx = (cur_rx + 1) & RX_RING_MASK;   /* Add & wrap to 0 */
    }        /* End of while loop */
    sc->cur_rx = cur_rx;
    sc->last_rx = last_rx;

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
 int N83815_pci_init(PIFACE pi, int *pirq, IOADDRESS *pbase_addr)
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
      while (pci_tab_83815[device_index].dev_name != 0)
      {
        if ( rtpci_find_device(pci_tab_83815[device_index].device_id,
                              pci_tab_83815[device_index].vendor_id,
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
          if (rtpci_read_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE, &byte_read)
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
                 default_irq = CFG_N83815_PCI_IRQ;

              if (rtpci_write_byte(BusNum, DevFncNum, RTPCI_REG_INT_LINE,default_irq)
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
              DEBUG_ERROR("N83815: Writing default I/O base value", 0, 0, 0);
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
                 d_word.two_words.wordl = CFG_N83815_PCI_IOBASE;

              if (rtpci_write_word(BusNum, DevFncNum, RTPCI_REG_IOBASE, d_word.two_words.wordl)
                                   == RTPCI_K_SUCCESSFUL)
                  *pbase_addr = (IOADDRESS) d_word.two_words.wordl;
              else
                  return(-1);  /* I/O BASE Register Write Failed      */
              }
            else
              *pbase_addr = (word) (d_word.two_words.wordl & RTPCI_M_IOBASE_L);

            if (rtpci_read_word(BusNum,DevFncNum,RTPCI_REG_IOBASE+2,&d_word.two_words.wordh)
                                == RTPCI_K_SUCCESSFUL)
            {
               DEBUG_ERROR("N83815_pci_init. wordh=", EBS_INT1, d_word.two_words.wordh, 0);
               DEBUG_ERROR("N83815_pci_init. base_addr=", EBS_INT1, *pbase_addr, 0);
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
      } /* while (pci_tab_83815[device_index].dev_name != 0) */
      return(-1);
    }
    else  /* if (rtpci_bios_present()) */
      return(-1);      /* No PCI BIOS present.    */
}


/* ********************************************************************   */
/* calc values for multicast                                              */
void N83815_getmcaf(PFBYTE mclist, int lenmclist, word *hash_table)
{
int bytesmclist;
byte c;
PFBYTE cp;
dword crc;
int i, len, offset;
int high,addr;

    bytesmclist = lenmclist * ETH_ALEN;

    /*
     * Set up multicast address filter by passing all multicast addresses
     * through a crc generator, and then using the 9 least significant bits
     * as an index into the 512 bit logical address filter.  The upper 4 bits
     * select the word, while the lower 5 bits select the bit within
     * the word.
     */

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
             /* Just want the 9 least significant bits.     */
        crc &= 0x1ff;
             /* Turn on the corresponding bit in the filter.     */
        if ( (crc & 0x01f) > 0x0f)    /* see if high order word or low order */
            high = 1;
            else
            high = 0;
        addr = (int)((((crc & 0x1e0) >> 5) * 2) + high);  /* which word in hash table should be modified 0 to 31 */
        if (high)
            *(hash_table+addr) = (word)(1 << ((crc & 0x01f) - 16));
        else
            *(hash_table+addr) = (word)(1 <<  (crc & 0x01f));
    }
}

/* ********************************************************************   */
int xn_bind_n83815(int minor_number)
{
    return(xn_device_table_add(n83815_device.device_id,
                        minor_number,
                        n83815_device.iface_type,
                        n83815_device.device_name,
                        SNMP_DEVICE_INFO(n83815_device.media_mib,
                                         n83815_device.speed)
                        (DEV_OPEN)n83815_device.open,
                        (DEV_CLOSE)n83815_device.close,
                        (DEV_XMIT)n83815_device.xmit,
                        (DEV_XMIT_DONE)n83815_device.xmit_done,
                        (DEV_PROCESS_INTERRUPTS)n83815_device.proc_interrupts,
                        (DEV_STATS)n83815_device.statistics,
                        (DEV_SETMCAST)n83815_device.setmcast));
}
#endif      /* DECLARING_DATA */
#endif

