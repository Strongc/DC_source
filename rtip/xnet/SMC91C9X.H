/*                                                                      */
/* SMC91C9X.H - defines for smc91c9x device driver                      */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1999                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
#ifndef __SMC91C9x_RTIP_H__
#define __SMC91C9x_RTIP_H__

#include "rtip.h"

int smc91c9x_getLinkStatus(void);						//Added by JLA (IO Technologies)

/****** Interrupt Controller Registers *****  */
#define MR8259A     0x21        /* interrupt controller 1 */
#define IR8259A     0x20
#define MR8259B     0xA1        /* interrupt controller 2 */
#define IR8259B     0xA0

/****** 8259 REGISTER DEFINITIONS *****  */
#define EOI     0x20        /* end of interrupt */
#define RIRR    0x0a        /* read interrupt request register */


#if (__RTIP__)
#define ALWAYS_16BIT 1
#else
#define ALWAYS_16BIT 0
#endif

/* ********************************************************************   */
#define SMC_ETHER_MAX_LEN   (ETHERSIZE+4)   /* includes header */
#define CRC_LEN             4

/* [i_a] 2.2 usecs loop on a pentium III is ??? polls? I don't know so let's have some...   */
#define N_ALLOC_POLLS       256

#define SMC_BIT0    0x1
#define SMC_BIT1    0x2
#define SMC_BIT2    0x4
#define SMC_BIT3    0x8
#define SMC_BIT4    0x10
#define SMC_BIT5    0x20
#define SMC_BIT6    0x40
#define SMC_BIT7    0x80
#define SMC_BIT8    0x100
#define SMC_BIT9    0x200
#define SMC_BITA    0x400
#define SMC_BITB    0x800
#define SMC_BITC    0x1000
#define SMC_BITD    0x2000
#define SMC_BITE    0x4000
#define SMC_BITF    0x8000


/* Bank zero register offsets   */
#define TCR_0_BK0 0     /* Transmit control register */
#define EPH_2_BK0 2     /* Transmit status register   */
#define RCR_4_BK0 4     /* Rcv control register      */
#define CNTR_6_BK0 6    /* Counter register          */
#define MIR_8_BK0 8     /* Memory info register      */
/*OS*/ /* ATTENTION! Memory Configuration Register doesn't exist in 91C111.
  It's address is used for new Receive/PHY Control register.                */
#define MCR_A_BK0 0xa   /* Memory configuration register */

/* Bank one register offsets   */
#define CFG_0_BK1 0     /* Configuration register from/to eeprom */
#define BAR_2_BK1 2     /* Base register from/to eeprom */
#define IAR0_4_BK1 4    /* Ethernet address registers from eeprom */
#define IAR2_6_BK1 6
#define IAR4_8_BK1 8
#define GP_A_BK1 0xa    /* GP. word access to eeprom */
#define CNTRL_C_BK1 0xC /* Control register */

/* Bank two register offsets   */
#define MMU_0_BK2 0     /* MMU Command register */
#define PNR_2_BK2 2     /* Packet number register */
#define ALLOC_3_BK2 3   /* Allocation register */
#define FIFO_4_BK2 4    /* Fifo register */
#define PTR_6_BK2 6     /* Pointer register */
#define DATA_8_BK2 8    /* Data xfer register */
#define DATA_A_BK2 0xa  /* Data xfer register */
#define ISR_C_BK2 0xC   /* Interrupt status */
#define IAR_C_BK2 0xC   /* Interrupt acknowledge */
#define IMR_D_BK2 0xD   /* Interrupt mask */

/* Bank three register offsets   */
#define MC0_0_BK3 0     /* Multicast table */
#define MC2_2_BK3 2
#define MC4_4_BK3 4
#define MC6_6_BK3 6

#define BSR_E_BKALL 0xe
#define BANK0   0
#define BANK1   1
#define BANK2   2
#define BANK3   3

/* Bank zero bit definitions                              */
/* define TCR_0_BK0 0        Transmit control register    */
    #define TXEN_B000    SMC_BIT0    /* Enable xmit */
    #define LOOP_B100    SMC_BIT1    /* Loop after encoder */
    #define FORCOL_B200  SMC_BIT2    /* Force collision */
    #define PAD_B700     SMC_BIT7    /* Force padding */
    #define NCRC_B800    SMC_BIT8    /* Disable crc */
    #define MON_BA00     SMC_BITA    /* Monitor carrier */
    #define FDUP_BB00    SMC_BITB    /* Full duplex. (receive own pkts) */
    #define STSQ_BC00    SMC_BITC    /* Stop on sig qual error */
    #define EPLOOP_BD00  SMC_BITD    /* Loop before encoder */

/* define EPH_2_BK0 2       Transmit status register     */
    #define TXSUC_B002   SMC_BIT0   /* Transmit success */
    #define SNGL_B102    SMC_BIT1   /* Single collision occured */
    #define MULT_B202    SMC_BIT2   /* Multiple collsions occured*/
    #define LXTMULT_B302 SMC_BIT3   /* Last xmitted frame was mcast */
    #define COL16_B402   SMC_BIT4   /* 16 collisions detected */
    #define SQET_B502    SMC_BIT5   /* Sig quality erro detected */
    #define LXTBRD_B602  SMC_BIT6   /* Last xmitted frame was broadcast */
    #define TXDFER_B702  SMC_BIT7   /* Last xmit was deferred */
    #define LATCOL_B902  SMC_BIT9   /* Late collision */
    #define LOSTC_BA02   SMC_BITA   /* Carrier lost */
    #define EXCDEF_BB02  SMC_BITB   /* Excessive deferral */
    #define CTRROL_BC02  SMC_BITC   /* Counter rollover */
    #define RXOVRN_BD02  SMC_BITD   /* Rcv overrun (out of memory) */
    #define LNKOK_BE02   SMC_BITE   /* Link ok. */
    #define TXUNRN_BF02  SMC_BITF   /* xmitter underrun. Shouldn't happen */

/* RCR_4_BK0 4       Rcv control register        */
    #define RXABRT_B004 SMC_BIT0        /* Rcv aborted. Pkt too long */
    #define PRMSC_B104  SMC_BIT1        /* Enable promiscuous */
    #define PRMSC_B204  SMC_BIT2        /* Enable all mcasts */
    #define RXEN_B804   SMC_BIT8        /* Enable rcvs */
    #define STRIP_B904  SMC_BIT9        /* Strip crc */
    #define FILT_BE04   SMC_BITE        /* Fiter carrier */
    #define RST_BF04    SMC_BITF        /* SW reset */

/* define CFG_0_BK1 0       Configuration register from/to eeprom   */
    #define INTSEL0_B1XX 0          /* Select int 0 */
    #define INTSEL1_B1XX SMC_BIT1       /* Select int 1 */
    #define INTSEL2_B1XX SMC_BIT2       /* Select int 2 */
    #define INTSEL3_B1XX SMC_BIT2|SMC_BIT1  /* Select int 3 */
    #define FSTRAM_B310 SMC_BIT3        /* Select fast ram */
    #define SZRAM_B510 SMC_BIT5         /* 1 ==s 32K */
    #define DISLNK_B610 SMC_BIT6        /* 1 ==s Disable link test */
    #define EN16_B710 SMC_BIT7          /* 1 ==s enable 16 bit */
    #define AUI_B810 SMC_BIT8           /* 1 ==s enable AUI */
    #define SSQ_B910 SMC_BIT9           /* 1 ==s lower squelch */
    #define FSTP_BA10 SMC_BITA          /* Full step aui if 1 ?? */
    #define NWAI_BC10 SMC_BITC          /* No wait states if 1 ?? */
/* define CNTRL_C_BK1 0xC    Control register   */
    #define STORE_B01C SMC_BIT0         /* Store to eeprom */
    #define LOAD_B11C SMC_BIT1          /* Load from eeprom */
    #define EEPSEL_B21C SMC_BIT2        /* EEprom select */
    #define TEENABLE_B51C SMC_BIT5      /* Xmit error int enable */
    #define CRENABLE_B61C SMC_BIT6      /* Cntr rollover int enable */
    #define LEENABLE_B71C SMC_BIT7      /* Link error int enable */
    #define UDOE_B81C SMC_BIT8
    #define AUTOR_BB1C SMC_BITB         /* Autorelease */
    #define RCVBAD_BE1C SMC_BITE        /* Set to rcv bad pkts (crc) */
    #define BYPASS_BF1C SMC_BITF        /* Bypass not implemented */


/* #define MMU_0_BK2 0       MMU Command register   */
    #define MMU_AMEM SMC_BIT5
    /* Macro to allocat xmit memory. X is in bytes 1 - 2K   */
    #define ALLOCTX(X)  (byte) (MMU_AMEM|( (((X)+0xff) >> 8) -1))
    #define MMU_RESET SMC_BIT6
    #define MMU_RMRX  (SMC_BIT6|SMC_BIT5)       /* Remove rx from q */
    #define MMU_RMRLRX  SMC_BIT7            /* Remove rx from q and free mem */
    #define MMU_RMSP  (SMC_BIT7|SMC_BIT5)       /* Remove specific packet */
    #define MMU_NQTX  (SMC_BIT7|SMC_BIT6)       /* Enque packet for xmit */
    #define MMU_RSTTX (SMC_BIT7|SMC_BIT6|SMC_BIT5)  /* Reset xmit fifo */

/* #define PTR_6_BK2 6       Pointer register   */
    #define PTR_READ    SMC_BITD
    #define PTR_AUTOINC SMC_BITE
    #define PTR_RCV     SMC_BITF
    /* Normal read from rcv area   */
    #define READ_RCV(X) (PTR_READ|PTR_AUTOINC|PTR_RCV|(X))
    /* Normal read from send area   */
    #define READ_XMIT(X) (PTR_READ|PTR_AUTOINC|(X))
    /* Normal write to send area   */
    #define WRITE_XMIT(X) (PTR_AUTOINC|(X))

/* #define ISR_C_BK2 0xC    Interrupt status   */

    #define ISR_RCVINT  SMC_BIT0
    #define ISR_TXINT   SMC_BIT1
    #define ISR_TXEMT   SMC_BIT2
    #define ISR_ALLOC   SMC_BIT3
    #define ISR_OVR     SMC_BIT4
    #define ISR_EPH     SMC_BIT5

/* #define IAR_C_BK2 0xC    Interrupt acknowledge   */
    #define IAR_TXINT   SMC_BIT1
    #define IAR_TXEMT   SMC_BIT2
    #define IAR_OVR     SMC_BIT4

/* #define IMR_D_BK2 0xD    Interrupt mask   */
    #define  IMR_RCVINT SMC_BIT0
    #define IMR_TXINT   SMC_BIT1
    #define IMR_TXEMT   SMC_BIT2
    #define IMR_ALLOC   SMC_BIT3
    #define IMR_OVR     SMC_BIT4
    #define IMR_EPH     SMC_BIT5

/* Packet receive status bits. In the status word of the received frame   */
#define RCVTOOSHORT     SMC_BITA
#define RCVTOOLNG       SMC_BITB
#define RCVODD          SMC_BITC
#define RCVBADDCRV      SMC_BITD
#define RCVALGNERR      SMC_BITF

/* Packet send status bits. In the status word of the sent frame   */
#define TX_SUC      SMC_BIT0

/* MII management register, bank 3   */

#define MGMT        0x08    /* FEAST Registers */
#define MDO     0x01        /* MII Resgiter bits */
#define MDI     0x02
#define MCLK        0x04
#define MDOE        0x08
#define MALL        0x0F


/* PHY control   */
#define OPWrite     0x01
#define OPRead      0x02

#define PHY_CR  0       /*PHY Registers  */
#define PHY_SR  1
#define PHY_ID1 2
#define PHY_ID2 3


/* PHY Control Register bits   */
#define PHY_CR_Reset    0x8000
#define PHY_CR_LOOP 0x4000 /*activate loopback mode */
#define PHY_CR_Speed    0x2000 /*set speed, 1=100,0=10 */
#define PHY_CR_ANEG 0x1000 /*auto speed (overrides set speed) */
#define PHY_CR_PDOWN    0x0800 /*1=power down */
#define PHY_CR_ISOLATE  0x0400 /*1=isolate phy */
#define PHY_CR_RSTANEG  0x0200 /*1=restart autonegotiate */
#define PHY_CR_Duplex   0x0100 /*1=full duplex */
#define PHY_CR_COLLTST  0x0080 /*1=collision test */

/* PHY propietary registers   */
#define PHY_NATIONAL_PAR        0x19
#define PHY_NATIONAL_PAR_DUPLEX     0x0080
#define PHY_NATIONAL_PAR_SPEED_10   0x0040

#define PHY_TDK_DIAG            0x12
#define PHY_TDK_DIAG_DUPLEX     0x0800
#define PHY_TDK_DIAG_RATE       0x0400

#define PHY_QSI_BASETX          0x1F
#define PHY_QSI_BASETX_OPMODE_MASK  0x001c
#define PHY_QSI_BASETX_OPMODE_10HD  (2<<0x1)
#define PHY_QSI_BASETX_OPMODE_100HD (2<<0x2)
#define PHY_QSI_BASETX_OPMODE_10FD  (2<<0x5)
#define PHY_QSI_BASETX_OPMODE_100FD (2<<0x6)


#define PHY_OUI_QSI     0x006051
#define PHY_OUI_TDK     0x00C039
#define PHY_OUI_MITELSMSC   0x00A087
#define PHY_OUI_NATIONAL    0x080017

#define NWAY_TIMEOUT    10

/* ********************************************************************   */
/* ed_softc: per line info and status                                     */
typedef struct _91_parms
{
    /* configuration   */
    int     board_no;
    int     is_16bit;
    IOADDRESS ia_iobase;
    int     ia_irq;

    /* operating regs   */
    PIFACE  pi;
    RTIP_BOOLEAN wait_alloc;        /* waiting for alloc (vs send complete) */

    /* Internal disgnostics   */
    dword diag_alloc_tries;
    dword diag_alloc_poll_suc;
    dword diag_alloc_isr_fail;
    dword diag_alloc_alloc_reg_fail;

   dword diag_packets_received;
   dword diag_packets_dropped;

   dword err_rcvtoolong;
   dword err_rcvtooshort;
   dword err_rcvbadcrc;
   dword err_rcvalignment;
   dword err_rcvephoverrun;
   dword err_rcvoverrun;
   dword err_txunderrun;


    struct ether_statistics stats;
} _91_PARMS;

typedef struct _91_parms KS_FAR *P91_PARMS;


#endif



