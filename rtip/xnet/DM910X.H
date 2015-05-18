/*                                                                             */
/* DM910X.H - DAVICOM DM910X                                                   */
/*            PCI single chip fast ethernet NIC controller                     */
/*                                                                             */
/* EBS - RTIP                                                                  */
/*                                                                             */
/* Copyright EBSNet Inc, 2000                                                  */
/* All rights reserved.                                                        */
/* This code may not be redistributed in source or linkable object form        */
/* without the consent of its author.                                          */
/*                                                                             */
/* Technical contacts:                                                         */
/*         Shane Titus               Tom Van Oudenaren                         */
/*         shane@ebsnetinc.com       tom@ebsnetinc.com                         */
/*                                                                             */

#ifndef __DM910X__
#define __DM910X__ 1

/* ********************************************************************     */
typedef struct descriptor KS_FAR * PDESCRIPTOR;
typedef struct descriptor
{
#define OWN_BIT     0x80000000 /* set means owned by controller */
#define ES_BIT      0x00008000
#define TXJT_BIT    0x00004000
#define LOC_BIT     0x00000800
#define NC_BIT      0x00000400
#define LC_BIT      0x00000200
#define EC_BIT      0x00000100
#define CC_BITS     0x00000078
#define PLE_BIT     (1<<3)
#define AE_BIT      (1<<2)
#define FUE_BIT     0x00000002
#define DF_BIT      0x00000001
    dword       status;
#define CI_BIT  0x80000000 
#define ED_BIT  0x40000000
#define BD_BIT  0x20000000
#define CE_BIT  0x01000000
    dword       ctrl_flags;
    dword       buffer;
    dword       nxt_desc;
} DESCRIPTOR;

#define DC_TX_RING_SIZE 1
#define DC_TX_RING_MASK    (DC_TX_RING_SIZE-1) /* So if x + 1 == SIZE then x & MASK = 0 */
#define DC_RX_RING_SIZE 4
#define DC_RX_RING_MASK    (DC_RX_RING_SIZE-1)

/* ********************************************************************     */
typedef struct davicom_softc
{
    /* descriptors - they need to be on 16 byte boundaries so put first     */
    DESCRIPTOR          tx_desc[DC_TX_RING_SIZE];
    DESCRIPTOR          rx_desc[DC_RX_RING_SIZE];
    DCU                 tx_dcus[DC_TX_RING_SIZE]; 
    DCU                 rx_dcus[DC_RX_RING_SIZE]; 

    PIFACE              iface;
    int                 dav_irq;
    IOADDRESS           base_addr;

    struct ether_statistics stats;

    int                 next_tx;            /* next tx to use */
    int                 this_tx;            /* next tx to complete */
    int                 this_rx;            /* next rx to use */

    int                 OutOfBuffers;
    EBS_TIMER           timer;              /* timer to restart receiver  */
} DAVICOM_SOFTC;
typedef struct davicom_softc KS_FAR *PDAVICOM_SOFTC;

/**************************************************
                Davicom Vendor ID
 **************************************************/
#define DC_VENDOR_ID 0x1282
#define DC_DEVICE_ID 0x9102

/* ********************************************************************      */
/* SROM                                                                      */
/* ********************************************************************      */
#define ETHER_SROM_OFF 10           /* offset of ethernet address */

/* ********************************************************************      */
/* Control and Status Registers (CR)                                         */
/* ********************************************************************      */
#define DC_SCR_CR0          0x00    /* System Control Register */
#define DC_TX_START_CR1     0x08    /* Transmit Descriptor Poll Demand */
#define DC_RX_START_CR2     0x10    /* Receive Descriptor Poll Demand */
#define DC_RX_BASE_ADDR_CR3 0x18    /* Receive Descriptor Base Address  */
#define DC_TX_BASE_ADDR_CR4 0x20    /* Transmit Descriptor Base Address  */
#define DC_STATUS_ISR_CR5   0x28    /* Status Report Register */
#define DC_MODE_CR6         0x30    /* Transmit Descriptor Base Address  */
#define DC_ISR_MASK_CR7     0x38    /* Interrupt Mask Register */
#define DC_STATS_CR8        0x40    /* Statistical Counter Register */
#define DC_EX_MGMT_CR9      0x48    /* External Management Access Register */
#define DC_PHY_STATUS_CR12  0x60    /* PHY Status Regiser */
#define DC_FRAME_ACCESS_CR13 0x68   /* Sample Frame Access Register */
#define DC_FRAME_DATA_CR14  0x70    /* Sample Frame Data Register */
#define DC_WD_JABBER_CR15   0x78    /* Watchdog and Jabber Timer Register */

/* ********************************************************************      */
/* Status bits (DC_SCR_CR0 - System Control Register)                        */
/* ********************************************************************      */
#define DC_SCR_SW_RESET         0x00000001  

/* ********************************************************************      */
/* Interupt bits (DC_STATUS_ISR_CR5)                                         */
/* ********************************************************************      */
#define DC_ISR_TX_DONE          0x00000001
#define DC_ISR_TX_STOPPED       0x00000002
#define DC_ISR_TX_NOBUF         0x00000004
#define DC_ISR_TX_JABBER_TMO    0x00000008
#define DC_ISR_TX_UNDERRUN      0x00000020
#define DC_ISR_RX_DONE          0x00000040
#define DC_ISR_RX_NOBUF         0x00000080
#define DC_ISR_RX_STOPPED       0x00000100
#define DC_ISR_RX_WATCHDOG_TMO  0x00000200
#define DC_ISR_TX_EARLY         0x00000400
#define DC_ISR_BUS_ERR          0x00002000
#define DC_ISR_ABNORMAL         0x00008000
#define DC_ISR_NORMAL           0x00010000
#define DC_ISR_RX_STATE         0x000E0000
#define DC_ISR_TX_STATE         0x00700000
#define DC_ISR_BUS_ERROR        0x03800000


/* ********************************************************************      */
/* Mode Register Bits (DC_MODE_CR6)                                          */
/* ********************************************************************      */
#define DC_MODE_RAX             0x40000000  /* Receive all */
#define DC_MODE_NPFIFO          0x20000000  /* Don't purge RX Fifo if no buffer */
#define DC_MODE_TX_TM           0x00400000  /* xmit threshold */
#define DC_MODE_SFT_XMIT        0x00200000  /* Store and Forward xmit */
#define DC_MODE_1_PKT           0x00010000  /* 1 pkt in transmit fifo */
#define DC_MODE_TX_SC           0x00002000  /* Start/Stop Transmit */
#define DC_MODE_PAM             0x00000080  /* pass all multicast */
#define DC_MODE_RX_RC           0x00000002  /* Start/Stop Receive */

/* ********************************************************************      */
/* Mask Bits (DC_ISR_MASK_CR7 - Interrupt Mask Register)                     */
/* ********************************************************************      */
#define DC_IMR_NISE             0x00010000  /* Normal interrupt enable */
#define DC_IMR_AISE             0x00008000  /* Abnornal interrupt enable */
#define DC_IMR_SBEE             0x00002000  /* System bus error enable */
#define DC_IMR_RXWTE            0x00000200  /* Receive Wathdog Timer Expiered enable */
#define DC_IMR_RXPSE            0x00000100  /* Receive process stopped enable */
#define DC_IMR_RXDUE            0x00000080  /* Receive buffer unavailable */
#define DC_IMR_RXCIE            0x00000040  /* Receive complete interrupt */
#define DC_IMR_TXFUE            0x00000020  /* Transmit fifo underrun enabled */
#define DC_IMR_TXJTE            0x00000008  /* Transit jabber expired enabled */
#define DC_IMR_TXDUE            0x00000004  /* Transmit buffer unavailable enabled */
#define DC_IMR_TXPSE            0x00000002  /* Transmit buffer stopped enable */
#define DC_IMR_TXCIE            0x00000001  /* Transmit complete interrupt enable */

/* ********************************************************************      */
/* SROM (DC_EX_MGMT_CR9 - External Management Access Register)               */
/* ********************************************************************      */
/* CR9_EEPROM_SELECTED (CR9<11>):                                            */
/*    This bit is set to select the EEPROM access for memory interface.      */

/* CR9_DATA_OUT_FROM_EEPROM (CR9<3>):                                         */
/*    This bit is set to reflect the signal status of EEDI pin when EEPROM    */
/*    mode is selected.                                                       */

/* CR9_DATA_IN_TO_EEPROM (CR9<2>):                                             */
/*    This bit is set to generate the output signal to EEDO pin when EEPROM    */
/*    mode is selected.                                                        */

/* CR9_CLOCK_TO_EEPROM (CR9<1>):                                               */
/*    This bit is set to generate the output clock to EECLK pin when EEPROM    */
/*    mode is selected.                                                        */

/* CR9_CHIP_SELECT_TO_EEPROM (CR9<0>):                                         */
/*    This bit is set to generate the output signal to EECS pin when EEPROM    */
/*    mode is selected.                                                        */

#define DC_MGMT_MDIN                0x00080000      /* MII Data In */
#define DC_MGMT_MRW                 0x00040000      /* MII Read/Write mode */
#define DC_MGMT_MDOUT               0x00020000      /* output signal -  */
                                                    /* MDIO pin     */
#define DC_MGMT_MDCLK               0x00010000      /* output clock signal -  */
                                                    /* MDC pin     */

#define DM_SIO_ROMCTL_WRITE         0x00002000
#define DM_SIO_ROMCTL_READ          0x00004000

#define CR9_EEPROM_SELECTED         0x00000800
#define CR9_DATA_OUT_FROM_EEPROM    0x00000008
#define CR9_DATA_IN_TO_EEPROM       0x00000004
#define CR9_CLOCK_TO_EEPROM         0x00000002
#define CR9_CHIP_SELECT_TO_EEPROM   0x00000001

/* ********************************************************************      */
/* READ AND WRITE CONTROL REGISTERS                                          */
/* ********************************************************************      */
#define WRITE_CR(CR, VALUE) OUTDWORD(softc->base_addr+CR, (VALUE))

#define READ_CR(CR)         INDWORD(softc->base_addr+CR)

/* ********************************************************************      */
/* PHY REGISTERS                                                             */
/* Use dm_phy_readwrite_word() to read/write to these registers              */
/* ********************************************************************      */
#define DC_PHY_BMCR     0x00    /* Basic Mode Control Register   */
#define DC_PHY_BMSR     0x01    /* Basic Mode Status Register */
#define DC_PHY_REG1     0x02    /* PHY ID Register 1 */
#define DC_PHY_REG2     0x03    /* PHY ID Regsiter 2 */
#define DC_PHY_ANAR     0x04    /* Auto-Neg Ad Register */
#define DC_PHY_ANLPAR   0x05    /* Auto-Neg Link Parner Ability Register */
#define DC_PHY_ANER     0x06    /* Auto-Neg Expansion Register */
#define DC_PHY_DSCR     0x10    /* Config Register */
#define DC_PHY_DSCSR    0x11    /* Config/Status Register */
#define DC_PHY_10BTCSR  0x12    /* 10BaseT Config Status Register */

/* ********************************************************************      */
/* DC_PHY_BMCR  Register bits - Basic Mode Control Register                  */
#define DC_PHY_BMCR_RESET           0x8000      /* Reset PHY */
#define DC_PHY_BMCR_AUTO_EN         0x1000      /* Auto-negotiate Enable */
#define DC_PHY_BMCR_AUTO_ST         0x0400      /* Start Auto-negotiate */

/* ********************************************************************      */
/* DC_PHY_BMSR Register bits - Basic Mode Status Register                    */
#define DC_PHY_BMSR_AUTO_NEG_DONE   0x0020      /* Auto-negotiate Done */
#define DC_PHY_BMSR_CAN_AUTO_NEG    0x0008      /* able to Auto-negotiate */

/* ********************************************************************      */
/* DC_PHY_DSCSR bits - Config/Status Register                                */
#define DC_PHY_DSCSR_100FDX         0x4000      /* 100T - Full Duplex */
#define DC_PHY_DSCSR_100HDX         0x2000      /* 100T - Half Duplex */
#define DC_PHY_DSCSR_10FDX          0x1000      /* 10T - Full Duplex */
#define DC_PHY_DSCSR_10HDX          0x0800      /* 10T - Half Duplex */

/* ********************************************************************      */
/* DC_PHY_DSCR bits - Config Register                                        */
#define DC_PHY_DSCR_RESET           0x0008

#endif      /* __DM910X__ */


