/*                                                                           */
/* lanceisa.h                                                                */
/*                                                                           */
/*   EBS - RTIP                                                              */
/*                                                                           */
/*   Copyright EBSnet, Inc., 1998                                            */
/*   All rights reserved.                                                    */
/*   This code may not be redistributed in source or linkable object form    */
/*   without the consent of its author.                                      */
/*                                                                           */
/*                                                                           */
/*   Module description:                                                     */
/*      This file contains definitions of registers and data structures used */
/*      by the LANCE device driver (rtlance.c).                              */
/*                                                                           */
/*                                                                           */
/*   Revision history:                                                       */
/*  December 2001       PVO From rtlance.c                                   */
/*                                                                           */

/* *********************************************************************   */
#define RTWORD_ISA word

/* *********************************************************************          */
/*                                                                                */
/*  LANCE CSRs:                                                                   */
/*  There are 4 CSRs accessed through 2 bus addressible ports - RAP & RDP.        */
/*  On the PCnet-ISA, PCnet-ISA+ and PCnet-Fast+ boards, CSR88 holds the chip id. */
/*                                                                                */
/*  Constants for CSR address select written to RAP                               */
/*                                                                                */
#define LANCE_ISA_K_CSR0    0
#define LANCE_ISA_K_CSR1    1
#define LANCE_ISA_K_CSR2    2
#define LANCE_ISA_K_CSR3    3
#define LANCE_ISA_K_CSR4    4
#define LANCE_ISA_K_CSR5    5
#define LANCE_ISA_K_CSR8    8
#define LANCE_ISA_K_CSR15   15      /* Mode Register loaded during init or direct write */
#define LANCE_ISA_K_CSR88 88        /* CSRs 88-89 hold the Chip ID */
#define LANCE_ISA_K_CSR89   89
#define LANCE_ISA_K_CSR112 112  /* CSR112 holds the missed frame count */

/* *********************************************************************     */
/*                                                                           */
/*  Offsets of the Lance registers from the base address of the LANCE device */
/*  in I/OSpace.                                                             */
/*                                                                           */
#define LANCE_ISA_K_RDP         0x10
#define LANCE_ISA_K_RAP         0x12
#define LANCE_ISA_K_RESET           0x14
#define LANCE_ISA_K_BDP         0x16   /* Bus Data Port (for Am79C972) */
#define LANCE_ISA_K_IDP         0x16   /* ISA Data Port */
#define LANCE_ISA_K_TOTAL_SIZE  0x18

/* *********************************************************************   */
/*                                                                         */
/*  Offsets for the ISA CSRs                                               */
/*  Note: to access an ISA register, write the CSR # into the              */
/*          RAP, then the data into the IDP.                               */
/*                                                                         */
#define LANCE_ISA_K_ISACSR0 0
#define LANCE_ISA_K_ISACSR1 1
#define LANCE_ISA_K_ISACSR2 2   /* Miscellaneous Configuration */
#define LANCE_ISA_K_ISACSR3 3
#define LANCE_ISA_K_ISACSR4 4
#define LANCE_ISA_K_ISACSR5 5
#define LANCE_ISA_K_ISACSR6 6
#define LANCE_ISA_K_ISACSR7 7
#define LANCE_ISA_K_ISACSR8 8   /* Software Configuration */

#define LANCE_ISA_M_ISACSR8_DMA 0x07    /* bits 0:2 of ISACSR8 hold read-only image of */
                                    /* DMASEL(2:0) of Plug'n'Play register 0x74   */
/* *********************************************************************          */
/*                                                                                */
/* DMA definitions (for Am79C961)                                                 */
/*                                                                                */
#define LANCE_ISA_DMA_CH4       4   /* Cascade channel-indicates no DMA channel active */
#define DMA_8MASK_REG   0x0A    /*      ; system 2nd dma cntler mask reg */
#define DMA_16MASK_REG  0xD4    /*      ; system 1st dma cntler mask reg */
#define DMA_8MODE_REG   0x0B    /*      ; system 2nd dma cntler mode reg */
#define DMA_16MODE_REG  0xD6    /*      ; system 1st dma cntler mode reg */
#define DMA_CHL_FIELD   0x3     /*      ; dma channel fields bit 1:0 */
#define SET_DMA_MASK    0x4     /*      ; mask reg,bit 2: 0,clear/1,set */
#define SINGLE_MODE     0x40    /*      ; mode reg,bit 7,6: 01 single mode */
#define CASCADE_MODE    0xC0    /*      ; mode reg,bit 7,6: 11 cascade mode */

/* *********************************************************************   */
/*                                                                         */
/*  BCR register offsets - used for configuring PHY speed for Am79C972     */
/*                                                                         */
#define LANCE_ISA_K_BCR2     2
#define LANCE_ISA_K_BCR4     4
#define LANCE_ISA_K_BCR5     5
#define LANCE_ISA_K_BCR6     6
#define LANCE_ISA_K_BCR7     7
#define LANCE_ISA_K_BCR9     9
#define LANCE_ISA_K_BCR18   18
#define LANCE_ISA_K_BCR20   20
#define LANCE_ISA_K_BCR32 32
#define LANCE_ISA_K_BCR33   33
#define LANCE_ISA_K_BCR34 34

/* *********************************************************************   */
/*                                                                         */
/*  BCR register masks - used for setting PHY speed for the Am79C972       */
/*                                                                         */
#define LANCE_ISA_M_NOAND           0xFFFF
#define LANCE_ISA_M_NOOR            0x0000
#define LANCE_ISA_M_BCR2_ASEL       0x0002
#define LANCE_ISA_M_BCR2_ASEL_OFF   0xFFFD
#define LANCE_ISA_M_BCR9_FDEN       0x0001
#define LANCE_ISA_M_BCR9_FDEN_OFF 0xFFFE
#define LANCE_ISA_M_BCR32_MIIPD 0x4000
#define LANCE_ISA_M_BCR32_APEP  0x0800
#define LANCE_ISA_M_BCR32_DANAS     0x0080
#define LANCE_ISA_M_BCR32_DANAS_OFF 0xFF7F
#define LANCE_ISA_M_BCR32_XPHYRST       0x0040  /* 1=reset phy */
#define LANCE_ISA_M_BCR32_XPHYRST_OFF   0xFFBF
#define LANCE_ISA_M_BCR32_XPHYANE       0x0020  /* 1=auto-neg enable, 0=disable */
#define LANCE_ISA_M_BCR32_XPHYANE_OFF   0xFFDF
#define LANCE_ISA_M_BCR32_XPHYFD        0x0010  /* 1=FD, 0=HD */
#define LANCE_ISA_M_BCR32_XPHYFD_OFF    0xFFEF
#define LANCE_ISA_M_BCR32_XPHYSP        0x0008  /* 1=100BT, 0=10BT */
#define LANCE_ISA_M_BCR32_XPHYSP_OFF    0xFFF7  /* 1=100BT, 0=10BT */
#define LANCE_ISA_M_BCR32_MIIILP        0x0002  /* MII internal loopback */
#define LANCE_ISA_M_BCR32_MIIILP_OFF    0xFFFD

/* *********************************************************************       */
/*                                                                             */
/*  CSR0 & CSR3 & CSR15 bit definitions.                                       */
/*  CSR1 & CSR2 hold the address of the initialization block (low order 16     */
/*  bits in CSR1 (note: low order bit must be a zero as address of init block  */
/*  is required to be on a word boundary) and high order 8 bits in CSR2.       */
/*  CSR1, CSR2, & CSR3 are accessible only when the STOP bit in CSR0 is set.   */
/*  CSR15 is accessible only when either the STOP bit or CSR5:SPND bit is set. */
/*                                                                             */
/*  CSR0 - bit 0:  INIT                                                        */
/*       - bit 1:  STRT                                                        */
/*       - bit 2:  STOP                                                        */
/*       - bit 3:  TDMD                                                        */
/*       - bit 4:  TXON                                                        */
/*       - bit 5:  RXON                                                        */
/*       - bit 6:  INEA                                                        */
/*       - bit 7:  INTR                                                        */
/*       - bit 8:  IDON                                                        */
/*       - bit 9:  TINT                                                        */
/*       - bit 10: RINT                                                        */
/*       - bit 11: MERR                                                        */
/*       - bit 12: MISS                                                        */
/*       - bit 13: CERR                                                        */
/*       - bit 14: BABL                                                        */
/*       - bit 15: ERR                                                         */
/*                                                                             */
/*  CSR3 - bit 0: BCON                                                         */
/*       - bit 1: ACON                                                         */
/*       - bit 2: BSWP                                                         */
/*       - bit 3-15: RESERVED                                                  */
/*                                                                             */
/*  CSR15- bit 8:7 PORTSEL[1:0]; 11=>MII selected, 10=>GPSI selected           */
/*                                                                             */
#define LANCE_ISA_M_CSR0_ERR    0x8000  
#define LANCE_ISA_M_CSR0_BABL 0x4000    
#define LANCE_ISA_M_CSR0_CERR   0x2000  
#define LANCE_ISA_M_CSR0_MISS   0x1000  
#define LANCE_ISA_M_CSR0_MERR   0x0800
#define LANCE_ISA_M_CSR0_RINT   0x0400
#define LANCE_ISA_M_CSR0_TINT   0x0200
#define LANCE_ISA_M_CSR0_IDON 0x0100
#define LANCE_ISA_M_CSR0_INTR   0x0080  
#define LANCE_ISA_M_CSR0_INEA 0x0040    
#define LANCE_ISA_M_CSR0_RXON   0x0020  
#define LANCE_ISA_M_CSR0_TXON   0x0010  
#define LANCE_ISA_M_CSR0_TDMD   0x0008
#define LANCE_ISA_M_CSR0_STOP   0x0004
#define LANCE_ISA_M_CSR0_STRT   0x0002
#define LANCE_ISA_M_CSR0_INIT 0x0001

#define LANCE_ISA_M_CSR3_RESV 0xfff8
#define LANCE_ISA_M_CSR3_BSWP 0x0004
#define LANCE_ISA_M_CSR3_ACON 0x0002
#define LANCE_ISA_M_CSR3_BCON 0x0001

#define LANCE_ISA_M_CSR5_SPND   0x0001

#define LANCE_ISA_M_CSR15_MII       0x0180      /* PORTSEL 11 */
#define LANCE_ISA_M_CSR15_GPSI  0x0100      /* PORTSEL 10 */
#define LANCE_ISA_M_CSR15_PORTSEL 0xfe7f        /* PORTSEL 00 */

/* *********************************************************************     */
/*                                                                           */
/*  Initialization Block:                                                    */
/*  The initialization block is read by the LANCE when the INIT bit in CSR0  */
/*  is set.                                                                  */
/*  The initialization block consists of 12 consecutive words:               */
/*     IADR+00:     MODE                                                     */
/*     IADR+02:     PADR (15:00)                                             */
/*     IADR+04:     PADR (31:16)                                             */
/*     IADR+06:     PADR (47:32)                                             */
/*     IADR+08:     LADRF(15:00)                                             */
/*     IADR+10:     LADRF(31:16)                                             */
/*     IADR+12:     LADRF(47:32)                                             */
/*     IADR+14:     LADRF(63:48)                                             */
/*     IADR+16:     RDRA (15:00)                                             */
/*     IADR+18:     RLEN-RDRA(23:16)                                         */
/*     IADR+20:     TDRA (15:00)                                             */
/*     IADR+22:     TLEN-TDRA(23:16)                                         */
/*                                                                           */
/*  The Mode Register allows alteration of the LANCE's operating parameters. */
/*  Normal operation is with the Mode Register clear.                        */
/*  Mode Register bit definitions.                                           */
/*     Mode - bit 0:   DRX                                                   */
/*          - bit 1:   DTX                                                   */
/*          - bit 2:   LOOP                                                  */
/*          - bit 3:   DTCR                                                  */
/*          - bit 4:   COLL                                                  */
/*          - bit 5:   DRTY                                                  */
/*          - bit 6:   INTL                                                  */
/*          - bit 7:   EMBA                                                  */
/*          - bit 9-14: RESERVED                                             */
/*          - bit 15:  PROM                                                  */
/*                                                                           */
/*                                                                           */

#define LANCE_ISA_M_RDR2_RLEN   (LANCE_ISA_LOG_RX_BUF << 13)
#define LANCE_ISA_M_RDR2_RESV 0x1f00
#define LANCE_ISA_M_RDR2_HI8    0x00ff

typedef struct i_isa_rdr{ 
    word    rdr1;       /* RDRA [15:00]      */
    word    rdr2;       /* RLEN+RDRA[23:16]  */
} LANCE_ISA_T_RDR;

#define LANCE_ISA_M_TDR2_TLEN   (LANCE_ISA_LOG_TX_BUF << 13)
#define LANCE_ISA_M_TDR2_RESV 0x1f00
#define LANCE_ISA_M_TDR2_HI8    0x00ff
typedef struct i_isa_tdr{ 
    word    tdr1;       /* TDRA [15:00]      */
    word    tdr2;       /* TLEN+TDRA[23:16]  */
} LANCE_ISA_T_TDR;

#define LANCE_ISA_M_MODE_PROM       0x8000
#define LANCE_ISA_M_MODE_RESV   0x7f00
#define LANCE_ISA_M_MODE_EMBA   0x0080
#define LANCE_ISA_M_MODE_INTL   0x0040
#define LANCE_ISA_M_MODE_DRTY   0x0020
#define LANCE_ISA_M_MODE_COLL   0x0010
#define LANCE_ISA_M_MODE_DTCR   0x0008
#define LANCE_ISA_M_MODE_LOOP   0x0004
#define LANCE_ISA_M_MODE_NOLOOP 0xfffb
#define LANCE_ISA_M_MODE_DTX    0x0002
#define LANCE_ISA_M_MODE_DRX    0x0001

#define LANCE_ISA_LADR_WORD_CNT 4

#define INIT_BLOCK_ALIGN    3
typedef struct i_isa_blk
{
    word    mode;       /* mode */
    byte    padr[6];
                                /* physical address [15:0] bit0 MBZ    */
                                /* physical address [31:16]            */
                                /* physical address [47:32]            */
    word    ladr[LANCE_ISA_LADR_WORD_CNT];
                                /* logical address filter [15:0]       */
                                /* logical address filter [31:16]      */
                                /* logical address filter [47:32]      */
                                /* logical address filter [63:48]      */
    word    rdr1;       /* receive descriptor ring info - rdra */
    word    rdr2;       /*                          rlen + rdra */
    word    tdr1;       /* transmit descriptor ring info - tdra */
    word    tdr2;       /*                          tlen + tdra */
} LANCE_ISA_S_INIT_BLK;

typedef struct i_isa_blk KS_FAR *PINIT_ISA_BLK;
#define SIZEOF_ISA_INIT_BLOCK sizeof(LANCE_ISA_S_INIT_BLK)

/* *********************************************************************       */
/*                                                                             */
/*  Buffer Management:                                                         */
/*  These are the structures used to manage the receive and transmit data      */
/*  buffers.                                                                   */
/*  There are two rings, one for receive and one for transmit, that hold       */
/*  descriptors pointing to the data buffers and containing status info        */
/*  pertaining to the data buffers. The 16 bit and 32 bit modes use different  */
/*  formats.                                                                   */
/*                                                                             */
/* 16 bit receive/transmit descriptor format:                                  */
/*                                                                             */
/*  Each descriptor in a ring (in memory) is a 4-word entry.                   */
/*                                                                             */
/*  RMD0:   LADR(15:00) - low order 16 bits of data buffer address             */
/*                                                                             */
/*  RMD1:   HADR( 7:00) - high order 8 bits of data buffer address             */
/*          ENP (8)     - End of Packet                                        */
/*          STP (9)     - Start of Packet                                      */
/*          BUFF(10)                                                           */
/*          CRC (11)                                                           */
/*          OFLO(12)                                                           */
/*          FRAM(13)                                                           */
/*          ERR (14)                                                           */
/*          OWN (15)    - Owned by host=0 (hosts sets after emptying buffer)   */
/*                        Owned by LANCE=1(LANCE clears after filling buffer)  */
/*                                                                             */
/*  RMD2:   BCNT(11-00) - Buffer Byte Count (written by host only)             */
/*          ONES(15:12) - Must be ones (written by host only)                  */
/*                                                                             */
/*  RMD3:   MCNT(11:00) - Message Byte Count(written by chip; cleared by host) */
/*          RES (15:12) - Reserved (Read as Zeroes; Write as zeroes)           */
/*                                                                             */
/*                                                                             */
/*  TMD0:   LADR(15:00) - low order 16 bits of data buffer address             */
/*                                                                             */
/*  TMD1:   HADR( 7:00) - high order 8 bits of data buffer address             */
/*          ENP (8)     - End of Packet                                        */
/*          STP (9)     - Start of Packet                                      */
/*          DEF (10)                                                           */
/*          ONE (11)                                                           */
/*          MORE(12)                                                           */
/*          ADD_FCS(13)                                                        */
/*          ERR (14)                                                           */
/*          OWN (15)    - Owned by host=0 (hosts sets after emptying buffer)   */
/*                        Owned by LANCE=1(LANCE clears after filling buffer)  */
/*                                                                             */
/*  TMD2:   BCNT(11-00) - Buffer Byte Count (written by host only)             */
/*          ONES(15:12) - Must be ones (written by host only)                  */
/*                                                                             */
/*  TMD3:   MCNT(09:00) - Message Byte Count(written by chip; cleared by host) */
/*          RTRY(10)                                                           */
/*          LCAR(11)                                                           */
/*          LCOL(12)                                                           */
/*          RES (13)                                                           */
/*          UFLO(14)                                                           */
/*          BUFF(15)                                                           */
/*                                                                             */
/*                                                                             */
/*  Constants used for ring definitions and buffer management.                 */
/*                                                                             */
#define LANCE_ISA_M_CPU_OWNED       0x0000
#define LANCE_ISA_M_LANCE_OWNED 0x8000

#define LANCE_ISA_LOG_RX_BUF        4   /*  2**4 = 16 rx buffers */
#define LANCE_ISA_LOG_TX_BUF        2   /*  2**2 =  4 tx buffers */
#define NUM_ISA_RX_DESC (1 << LANCE_ISA_LOG_RX_BUF)
#define NUM_ISA_TX_DESC (1 << LANCE_ISA_LOG_TX_BUF)
#define SIZEOF_ISA_RX_DESC_ARRAY (NUM_ISA_RX_DESC * sizeof(LANCE_ISA_RX_DESC))
#define SIZEOF_ISA_TX_DESC_ARRAY (NUM_ISA_TX_DESC * sizeof(LANCE_ISA_TX_DESC))
#define RX_TX_DESC_ALIGN_ISA        7   /* align descriptors on quadword bound */
#define PKT_BUF_SZ              1544/* max packet size - for shared mem */

/* *********************************************************************    */
/*                                                                          */
/* Macros for common functions used for writing buffer management registers */
/*                                                                          */
                                /* get the low 16 bits of address x         */
#define LANCE_ISA_LOW_16(x) (word)kvtop(x)
                                /* get the high 8 bits of address x   */
#define LANCE_ISA_HIGH_8(x) (byte)(kvtop(x) >> 16)
                                /* convert to 2's complement       */

#define LANCE_ISA_TWOS_COMP(x) (word)(~(x) + 1)
                                /* increment pointers into descriptor rings   */
#define LANCE_ISA_INC_RX_PTR(x)             \
        {                               \
            if (x >= (NUM_ISA_RX_DESC - 1)) \
                x = 0;                  \
            else                        \
                x++;                    \
        }

#define LANCE_ISA_INC_TX_PTR(x)             \
        {                               \
            if (x >= (NUM_ISA_TX_DESC - 1)) \
                x = 0;                  \
            else                        \
                x++;                    \
        }

/* *********************************************************************   */
/*                                                                         */
/* Ring Buffer Register Definitions                                        */
/*                                                                         */
#define LANCE_ISA_M_RMD1_OWN    0x8000
#define LANCE_ISA_M_RMD1_ERR  0x4000
#define LANCE_ISA_M_RMD1_FRAM 0x2000
#define LANCE_ISA_M_RMD1_OFLO 0x1000
#define LANCE_ISA_M_RMD1_CRC  0x0800
#define LANCE_ISA_M_RMD1_BUFF 0x0400
#define LANCE_ISA_M_RMD1_STP  0x0200
#define LANCE_ISA_M_RMD1_ENP  0x0100
#define LANCE_ISA_M_RMD1_HADR 0x00ff

#define LANCE_ISA_M_RMD2_ONES   0xf000
#define LANCE_ISA_M_RMD2_BCNT 0x0fff

#define LANCE_ISA_M_RMD3_RESV 0xf000
#define LANCE_ISA_M_RMD3_MCNT 0x0fff

typedef struct rx_isa_desc
{
    RTWORD_ISA  rmd0;       /* low 16 bits of rx buffer address  */
    RTWORD_ISA  rmd1;       /* msb=OWN,errors,STP,ENP,high 8 bits of rx buffer addr */
    RTWORD_ISA  rmd2;       /* bits 15-12 MBZ, bits 11-0 buffer cnt(2's comp) */
    RTWORD_ISA  rmd3;       /* bits 15-12 RES, bits 11-0 message byte count */
} LANCE_ISA_RX_DESC;

typedef struct rx_isa_desc KS_FAR *PRX_ISA_DESC;

/* *********************************************************************   */
#define LANCE_ISA_M_TMD1_OWN    0x8000
#define LANCE_ISA_M_TMD1_ERR    0x4000
#define LANCE_ISA_M_TMD1_FCS    0x2000
#define LANCE_ISA_M_TMD1_MORE   0x1000
#define LANCE_ISA_M_TMD1_ONE    0x0800
#define LANCE_ISA_M_TMD1_DEF    0x0400
#define LANCE_ISA_M_TMD1_STP    0x0200
#define LANCE_ISA_M_TMD1_ENP    0x0100
#define LANCE_ISA_M_TMD1_HADR   0x00FF

#define LANCE_ISA_M_TMD2_ONES 0xf000
#define LANCE_ISA_M_TMD2_BCNT 0x0fff

#define LANCE_ISA_M_TMD3_BUFF   0x8000
#define LANCE_ISA_M_TMD3_UFLO   0x4000
#define LANCE_ISA_M_TMD3_RESV   0x2000
#define LANCE_ISA_M_TMD3_LCOL   0x1000
#define LANCE_ISA_M_TMD3_LCAR   0x0800
#define LANCE_ISA_M_TMD3_RTRY   0x0400
#define LANCE_ISA_M_TMD3_TDR    0x03ff

typedef struct tx_isa_desc  /* SWSTYLE = 2 */
{
    RTWORD_ISA  tmd0;       /* low 16 bits of tx buffer address */
    RTWORD_ISA  tmd1;       /* 15-OWN,14-ERR,9-STP,8-ENP,7:0-high 8 bits of tx buffer address */
    RTWORD_ISA  tmd2;   
    RTWORD_ISA  tmd3;
} LANCE_ISA_TX_DESC;

typedef struct tx_isa_desc KS_FAR *PTX_ISA_DESC;

/* *********************************************************************   */
/*                                                                         */
/* Minimum size in bytes of an Ethernet packet, including header           */
/*                                                                         */
#define ETHER_MIN_LEN   64

/*                                                                          */
/* The SWAPIF() macro is used to swap bytes in the case where the processor */
/* is Big Endian (the lance device is Little Endian).                       */
/* Use tools.c routines to swap bytes.                                      */
/* in 32 bit mode, swap bytes in dword. Also provide a word swap.           */
/* Use routines defined in tools.c                                          */

#if (!KS_LITTLE_ENDIAN) /* 1=little endian (INTEL) */
    #define SWAPIF(x) lance_isa_wordswap(x)          
#else
    #define SWAPIF(x) x
#endif

/*                                                                         */
/* The macros INWORD and OUTWORD are usually set up in osenv.h. For 32 bit */
/* operation, redefine to utilize 32 bit routines.                         */
/*                                                                         */

/* *********************************************************************    */
/*                                                                          */
/* The XINWORD macro swaps word input based on the above SWAPIF definition. */
/*                                                                          */
#define XINWORD(X)  SWAPIF(INWORD(X));

/*                                                                    */
/* This is the definition of the data structure used by rtlance.c to  */
/* manage rings and lance access in general.                          */
/*                                                                    */
typedef struct 
{
    int     irq;                    /* Interrupt */
    IOADDRESS base_addr;            /* Base I/O address for device */
    PIFACE  iface;                  /* interface structure defined in RTIP.h */
    int     chip_version;           /* ??stored as unsigned char in lance.c. why??? */
    int     in_isr;                 /* flag set in rtlance_isr(interrupt service routine) */
    word    dma_channel;            /* DMA channel for ISA Bus Master device */
    word    mode_settings;          /* mode settings for init block */
    word    ladr[LANCE_ISA_LADR_WORD_CNT];/* logical address filter settings for init block */
    int     cur_rx;                 /* next available rx_ring entry */
    int     cur_tx;                 /* next available tx_ring entry */
    int     dirty_tx;               /* next tx ring entry to be free()ed. */
    PFBYTE  prx_dcu;                /* address of dcu holding rx ring */
    PFBYTE  ptx_dcu;                /* address of dcu holding tx ring */
    PFBYTE  pinit_dcu;              /* address of dcu holding init block */
#if (CFG_LANCE_SHARED_MEM)
    PFBYTE  rx_dcus[NUM_ISA_RX_DESC];   /* table of dcu addresses used for rx data */
    PFBYTE  tx_laddr[NUM_ISA_TX_DESC];  /* Host logical address of xmit buffers */
#else
    DCU     rx_dcus[NUM_ISA_RX_DESC];   /* table of dcu addresses used for rx data */
#endif
    PRX_ISA_DESC  prx_ring; /* pointer to the top of the rx ring buffer */
    PTX_ISA_DESC  ptx_ring; /* pointer to the top of the tx ring buffer */
    PINIT_ISA_BLK pinit_block; /* pointer to the initialization block */
    struct  ether_statistics    stats;  
} LANCE_ISA_T_PRIV;
typedef LANCE_ISA_T_PRIV KS_FAR *PLANCE_ISA_PRIV;

/* *********************************************************************      */
/*                                                                            */
/*  The following data structures and definitions provide a mapping from the  */
/*
**  Device IDs and Vendor IDs for PCI Devices 
*/
#define RTPCI_V_ID_AMDX                0x1022        /* Vendor ID for AMD            */
#define RTPCI_D_ID_AM7997X            0x2000        /* Device ID for               */
/*  chip ID number to the part number and features.  These are used in the         */
/*  routines lance_isa_get_chip_version, lance_isa_close and lance_isa_statistics. */
/*                                                                                 */
#define LANCE_MUST_PAD          0x00000001ul
#define LANCE_ENABLE_AUTOSELECT 0x00000002ul
#define LANCE_MUST_REINIT_RING  0x00000004ul
#define LANCE_MUST_UNRESET      0x00000008ul
#define LANCE_HAS_MISSED_FRAME  0x00000010ul
#define LANCE_MUST_SPND         0x00000020ul
#define LANCE_HAS_MII           0x00000040ul

struct lance_isa_chip_type 
{
    dword id_number;
    char *name;
    int flags;
};

enum {OLD_ISA_LANCE = 0, PCNET_ISA_ISA=1, PCNET_ISA_ISAP=2, PCNET_ISA_ISAPA=3,LANCE_ISA_UNKNOWN=4};

/*                    */
/*  Support routines  */
/*                    */
RTIP_BOOLEAN lance_isa_get_chip_version(PLANCE_ISA_PRIV);
void lance_isa_assign_rx_ring(PLANCE_ISA_PRIV);
void lance_isa_assign_tx_ring(PLANCE_ISA_PRIV);
void lance_isa_assign_init_block(PLANCE_ISA_PRIV);
RTIP_BOOLEAN lance_isa_init_rx_entry(PLANCE_ISA_PRIV, int);
RTIP_BOOLEAN lance_isa_init_tx_entry(PLANCE_ISA_PRIV, int);
void lance_isa_build_init_block(PLANCE_ISA_PRIV);
RTIP_BOOLEAN lance_isa_init(PLANCE_ISA_PRIV);
void lance_isa_stop(PLANCE_ISA_PRIV);
void lance_isa_free_dcus(PLANCE_ISA_PRIV);
void lance_isa_reinit(PLANCE_ISA_PRIV);
void lance_isa_reorder_buffers(PLANCE_ISA_PRIV);
void lance_isa_rx_interrupt(PLANCE_ISA_PRIV);
void lance_isa_tx_interrupt(PLANCE_ISA_PRIV);
void lance_isa_errors(dword, PLANCE_ISA_PRIV);
PLANCE_ISA_PRIV off_isa_to_priv(int);
PLANCE_ISA_PRIV iface_isa_to_priv(PIFACE);
void lance_isa_setup_dma(PLANCE_ISA_PRIV);
void lance_isa_dma_init(int);
void lance_isa_set_mcast_list(PLANCE_ISA_PRIV, int, void *);
RTIP_BOOLEAN lance_isa_device_found(PIFACE, unsigned char, unsigned char);
RTIP_BOOLEAN lance_isa_set_intline(PLANCE_ISA_PRIV, unsigned char, unsigned char);
RTIP_BOOLEAN lance_isa_set_iobase(PLANCE_ISA_PRIV, unsigned char, unsigned char);
RTIP_BOOLEAN lance_isa_set_cmd(unsigned char, unsigned char);
void lance_isa_restart(PLANCE_ISA_PRIV);
dword lance_isa_longswap(dword l);          /* change 4 bytes in long word 0,1,2,3 -> 3,2,1,0 */
word lance_isa_wordswap(word w);          /* change 2 bytes in word 0,1 -> 1,0 */

