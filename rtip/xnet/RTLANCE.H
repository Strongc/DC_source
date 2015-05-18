/*                                                                           */
/* rtlance.h                                                                 */
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
/*      Fall/Winter 1998   V. Kell   Initial coding                          */
/*  August 1999     TVO     32 bit capabilities                              */
/*  August 1999     TVO     AMD 79C973/975 support                           */
/*                                                                           */

/* *********************************************************************   */
#if (CFG_AMD_32BIT)
#define RTWORD dword
#else
#define RTWORD word
#endif

/* *********************************************************************          */
/*                                                                                */
/*  LANCE CSRs:                                                                   */
/*  There are 4 CSRs accessed through 2 bus addressible ports - RAP & RDP.        */
/*  On the PCnet-ISA, PCnet-ISA+ and PCnet-Fast+ boards, CSR88 holds the chip id. */
/*                                                                                */
/*  Constants for CSR address select written to RAP                               */
/*                                                                                */
#define RTLANCE_K_CSR0  0
#define RTLANCE_K_CSR1  1
#define RTLANCE_K_CSR2  2
#define RTLANCE_K_CSR3  3
#define RTLANCE_K_CSR4  4
#define RTLANCE_K_CSR5  5
#define RTLANCE_K_CSR8  8
#define RTLANCE_K_CSR15 15      /* Mode Register loaded during init or direct write */
#define RTLANCE_K_CSR88 88      /* CSRs 88-89 hold the Chip ID */
#define RTLANCE_K_CSR89 89
#define RTLANCE_K_CSR112 112    /* CSR112 holds the missed frame count */

/* *********************************************************************     */
/*                                                                           */
/*  Offsets of the Lance registers from the base address of the LANCE device */
/*  in I/OSpace.                                                             */
/*                                                                           */
#if (CFG_AMD_32BIT) /* when BCSR18 DWIO = 1 */
#define RTLANCE_K_RDP           0x10
#define RTLANCE_K_RAP           0x14
#define RTLANCE_K_RESET         0x18
#define RTLANCE_K_BDP           0x1C   /* Bus Data Port (for Am79C972) */
#define RTLANCE_K_IDP           0x1C   /* ISA Data Port */
#define RTLANCE_K_TOTAL_SIZE    0x20
#else
#define RTLANCE_K_RDP           0x10
#define RTLANCE_K_RAP           0x12
#define RTLANCE_K_RESET         0x14
#define RTLANCE_K_BDP           0x16   /* Bus Data Port (for Am79C972) */
#define RTLANCE_K_IDP           0x16   /* ISA Data Port */
#define RTLANCE_K_TOTAL_SIZE    0x18
#endif

/* *********************************************************************   */
/*                                                                         */
/*  Offsets for the ISA CSRs                                               */
/*  Note: to access an ISA register, write the CSR # into the              */
/*          RAP, then the data into the IDP.                               */
/*                                                                         */
#define RTLANCE_K_ISACSR0   0
#define RTLANCE_K_ISACSR1   1
#define RTLANCE_K_ISACSR2   2   /* Miscellaneous Configuration */
#define RTLANCE_K_ISACSR3   3
#define RTLANCE_K_ISACSR4   4
#define RTLANCE_K_ISACSR5   5
#define RTLANCE_K_ISACSR6   6
#define RTLANCE_K_ISACSR7   7
#define RTLANCE_K_ISACSR8   8   /* Software Configuration */

#define RTLANCE_M_ISACSR8_DMA 0x07  /* bits 0:2 of ISACSR8 hold read-only image of */
                                    /* DMASEL(2:0) of Plug'n'Play register 0x74   */
/* *********************************************************************          */
/*                                                                                */
/* DMA definitions (for Am79C961)                                                 */
/*                                                                                */
#define RTLANCE_DMA_CH4     4   /* Cascade channel-indicates no DMA channel active */
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
#define RTLANCE_K_BCR2   2
#define RTLANCE_K_BCR4   4
#define RTLANCE_K_BCR5   5
#define RTLANCE_K_BCR6   6
#define RTLANCE_K_BCR7   7
#define RTLANCE_K_BCR9   9
#define RTLANCE_K_BCR18 18
#define RTLANCE_K_BCR20 20
#define RTLANCE_K_BCR32 32
#define RTLANCE_K_BCR33 33
#define RTLANCE_K_BCR34 34

/* *********************************************************************   */
/*                                                                         */
/*  BCR register masks - used for setting PHY speed for the Am79C972       */
/*                                                                         */
#if (CFG_AMD_32BIT)
#define RTLANCE_M_NOAND             0x0000FFFFul
#define RTLANCE_M_NOOR              0x00000000ul
#define RTLANCE_M_DWIO_READMASK     0x000000FFul
#define RTLANCE_M_BCR2_ASEL         0x00000002ul
#define RTLANCE_M_BCR2_ASEL_OFF     0x0000FFFDul
#define RTLANCE_M_BCR9_FDEN         0x00000001ul
#define RTLANCE_M_BCR9_FDEN_OFF     0x0000FFFEul
#define RTLANCE_M_BCR18_DWIO        0x00000080ul
#define RTLANCE_M_BCR20_SSIZE32     0x00000100ul    /*read only */
#define RTLANCE_M_BCR20_SWSTYLE2    0x00000002ul
#define RTLANCE_M_BCR32_MIIPD       0x00004000ul
#define RTLANCE_M_BCR32_APEP        0x00000800ul
#define RTLANCE_M_BCR32_DANAS       0x00000080ul
#define RTLANCE_M_BCR32_DANAS_OFF   0x0000FF7Ful
#define RTLANCE_M_BCR32_XPHYRST     0x00000040ul    /* 1=reset phy */
#define RTLANCE_M_BCR32_XPHYRST_OFF 0x0000FFBFul
#define RTLANCE_M_BCR32_XPHYANE     0x00000020ul    /* 1=auto-neg enable, 0=disable */
#define RTLANCE_M_BCR32_XPHYANE_OFF 0x0000FFDFul
#define RTLANCE_M_BCR32_XPHYFD      0x00000010ul    /* 1=FD, 0=HD */
#define RTLANCE_M_BCR32_XPHYFD_OFF  0x0000FFEFul
#define RTLANCE_M_BCR32_XPHYSP      0x00000008ul    /* 1=100BT, 0=10BT */
#define RTLANCE_M_BCR32_XPHYSP_OFF  0x0000FFF7ul    /* 1=100BT, 0=10BT */
#define RTLANCE_M_BCR32_MIIILP      0x00000002ul    /* MII internal loopback */
#define RTLANCE_M_BCR32_MIIILP_OFF  0x0000FFFDul
#else
#define RTLANCE_M_NOAND         0xFFFF
#define RTLANCE_M_NOOR          0x0000
#define RTLANCE_M_BCR2_ASEL     0x0002
#define RTLANCE_M_BCR2_ASEL_OFF 0xFFFD
#define RTLANCE_M_BCR9_FDEN     0x0001
#define RTLANCE_M_BCR9_FDEN_OFF 0xFFFE
#define RTLANCE_M_BCR32_MIIPD   0x4000
#define RTLANCE_M_BCR32_APEP    0x0800
#define RTLANCE_M_BCR32_DANAS       0x0080
#define RTLANCE_M_BCR32_DANAS_OFF   0xFF7F
#define RTLANCE_M_BCR32_XPHYRST     0x0040  /* 1=reset phy */
#define RTLANCE_M_BCR32_XPHYRST_OFF 0xFFBF
#define RTLANCE_M_BCR32_XPHYANE     0x0020  /* 1=auto-neg enable, 0=disable */
#define RTLANCE_M_BCR32_XPHYANE_OFF 0xFFDF
#define RTLANCE_M_BCR32_XPHYFD      0x0010  /* 1=FD, 0=HD */
#define RTLANCE_M_BCR32_XPHYFD_OFF  0xFFEF
#define RTLANCE_M_BCR32_XPHYSP      0x0008  /* 1=100BT, 0=10BT */
#define RTLANCE_M_BCR32_XPHYSP_OFF  0xFFF7  /* 1=100BT, 0=10BT */
#define RTLANCE_M_BCR32_MIIILP      0x0002  /* MII internal loopback */
#define RTLANCE_M_BCR32_MIIILP_OFF  0xFFFD
#endif

/* *********************************************************************   */
/*                                                                         */
/* MII Management Registers                                                */
/*                                                                         */
#if (CFG_AMD_32BIT)
#define RTLANCE_M_MII_CNTRL     0x00000000ul    /* masks for registers to be or-d into */
#define RTLANCE_M_MII_STAT      0x00000001ul    /* BCR33 to access PHY registers */

#define RTLANCE_M_MII_100BT     0x00002000ul  /* masks to be or-d for BCR34 for MII */
#define RTLANCE_M_MII_FD        0x00000100ul    /* control register values */
#define RTLANCE_M_MII_ANE       0x00001000ul
#define RTLANCE_M_MII_ANER      0x00000200ul
#define RTLANCE_M_MII_NOVAL     0x00000000ul
#define RTLANCE_M_MII_RESET     0x00008000ul
#define RTLANCE_M_MII_LOOP      0x00004000ul
#define RTLANCE_M_MII_NOLOOP    0x0000BFFFul
#else
#define RTLANCE_M_MII_CNTRL 0x0000  /* masks for registers to be or-d into */
#define RTLANCE_M_MII_STAT  0x0001  /* BCR33 to access PHY registers */

#define RTLANCE_M_MII_100BT 0x2000  /* masks to be or-d for BCR34 for MII */
#define RTLANCE_M_MII_FD    0x0100  /* control register values */
#define RTLANCE_M_MII_ANE   0x1000
#define RTLANCE_M_MII_ANER  0x0200
#define RTLANCE_M_MII_NOVAL 0x0000
#define RTLANCE_M_MII_RESET 0x8000
#define RTLANCE_M_MII_LOOP      0x4000
#define RTLANCE_M_MII_NOLOOP    0xBFFF
#endif

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
#if (CFG_AMD_32BIT)
#define RTLANCE_M_CSR0_ZEROS    0x00000000ul
#define RTLANCE_M_CSR0_READMASK 0x0000fffful
#define RTLANCE_M_CSR0_ERR      0x00008000ul    
#define RTLANCE_M_CSR0_BABL     0x00004000ul    
#define RTLANCE_M_CSR0_CERR     0x00002000ul    
#define RTLANCE_M_CSR0_MISS     0x00001000ul    
#define RTLANCE_M_CSR0_MERR     0x00000800ul
#define RTLANCE_M_CSR0_RINT     0x00000400ul
#define RTLANCE_M_CSR0_TINT     0x00000200ul
#define RTLANCE_M_CSR0_IDON     0x00000100ul
#define RTLANCE_M_CSR0_INTR     0x00000080ul    
#define RTLANCE_M_CSR0_INEA     0x00000040ul    
#define RTLANCE_M_CSR0_RXON     0x00000020ul    
#define RTLANCE_M_CSR0_TXON     0x00000010ul    
#define RTLANCE_M_CSR0_TDMD     0x00000008ul
#define RTLANCE_M_CSR0_STOP     0x00000004ul
#define RTLANCE_M_CSR0_STRT     0x00000002ul
#define RTLANCE_M_CSR0_INIT     0x00000001ul

#define RTLANCE_M_CSR3_RESV     0x0000fff8ul
#define RTLANCE_M_CSR3_BSWP     0x00000004ul
#define RTLANCE_M_CSR3_ACON     0x00000002ul
#define RTLANCE_M_CSR3_BCON     0x00000001ul

#define RTLANCE_M_CSR5_SPND     0x00000001ul

#define RTLANCE_M_CSR15_MII     0x00000180ul        /* PORTSEL 11 */
#define RTLANCE_M_CSR15_GPSI    0x00000100ul        /* PORTSEL 10 */
#define RTLANCE_M_CSR15_PORTSEL 0x0000fe7ful        /* PORTSEL 00 */
#else
#define RTLANCE_M_CSR0_ERR  0x8000  
#define RTLANCE_M_CSR0_BABL 0x4000  
#define RTLANCE_M_CSR0_CERR 0x2000  
#define RTLANCE_M_CSR0_MISS 0x1000  
#define RTLANCE_M_CSR0_MERR 0x0800
#define RTLANCE_M_CSR0_RINT 0x0400
#define RTLANCE_M_CSR0_TINT 0x0200
#define RTLANCE_M_CSR0_IDON 0x0100
#define RTLANCE_M_CSR0_INTR 0x0080  
#define RTLANCE_M_CSR0_INEA 0x0040  
#define RTLANCE_M_CSR0_RXON 0x0020  
#define RTLANCE_M_CSR0_TXON 0x0010  
#define RTLANCE_M_CSR0_TDMD 0x0008
#define RTLANCE_M_CSR0_STOP 0x0004
#define RTLANCE_M_CSR0_STRT 0x0002
#define RTLANCE_M_CSR0_INIT 0x0001

#define RTLANCE_M_CSR3_RESV 0xfff8
#define RTLANCE_M_CSR3_BSWP 0x0004
#define RTLANCE_M_CSR3_ACON 0x0002
#define RTLANCE_M_CSR3_BCON 0x0001

#define RTLANCE_M_CSR5_SPND 0x0001

#define RTLANCE_M_CSR15_MII     0x0180      /* PORTSEL 11 */
#define RTLANCE_M_CSR15_GPSI    0x0100      /* PORTSEL 10 */
#define RTLANCE_M_CSR15_PORTSEL 0xfe7f      /* PORTSEL 00 */
#endif

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

#if (CFG_AMD_32BIT)
#define RTLANCE_M_RDR2_RLEN (RTLANCE_LOG_RX_BUF << 4) 
#else
#define RTLANCE_M_RDR2_RLEN (RTLANCE_LOG_RX_BUF << 13)
#define RTLANCE_M_RDR2_RESV 0x1f00
#define RTLANCE_M_RDR2_HI8  0x00ff

typedef struct i_rdr{ 
    word    rdr1;       /* RDRA [15:00]      */
    word    rdr2;       /* RLEN+RDRA[23:16]  */
} RTLANCE_T_RDR;
#endif

#if (CFG_AMD_32BIT)
#define RTLANCE_M_TDR2_TLEN (RTLANCE_LOG_TX_BUF << 4)
#else
#define RTLANCE_M_TDR2_TLEN (RTLANCE_LOG_TX_BUF << 13)
#define RTLANCE_M_TDR2_RESV 0x1f00
#define RTLANCE_M_TDR2_HI8  0x00ff
typedef struct i_tdr{ 
    word    tdr1;       /* TDRA [15:00]      */
    word    tdr2;       /* TLEN+TDRA[23:16]  */
} RTLANCE_T_TDR;

#endif
#define RTLANCE_M_MODE_PROM     0x8000
#define RTLANCE_M_MODE_RESV     0x7f00
#define RTLANCE_M_MODE_EMBA     0x0080
#define RTLANCE_M_MODE_INTL     0x0040
#define RTLANCE_M_MODE_DRTY     0x0020
#define RTLANCE_M_MODE_COLL     0x0010
#define RTLANCE_M_MODE_DTCR     0x0008
#define RTLANCE_M_MODE_LOOP     0x0004
#define RTLANCE_M_MODE_NOLOOP   0xfffb
#define RTLANCE_M_MODE_DTX      0x0002
#define RTLANCE_M_MODE_DRX      0x0001

#define RTLANCE_LADR_WORD_CNT   4

#define INIT_BLOCK_ALIGN    3
#if (CFG_AMD_32BIT)
struct i_blk 
{
#if (KS_LITTLE_ENDIAN) /* 1=little endian (INTEL) */
    word    mode;       /* mode                              */
    byte    rdr2;       /* rlen [7-4]; res [3-0] */
    byte    tdr2;       /* tlen [7-4]; res [3-0] */
#else
    byte    tdr2;       /* tlen [7-4]; res [3-0] */
    byte    rdr2;       /* rlen [7-4]; res [3-0] */
    word    mode;       /* mode                              */
#endif
    byte    padr[6];
                            /* physical address [15:0] bit0 MBZ    */
                            /* physical address [31:16]            */
                            /* physical address [47:32]            */
    byte    res1;
    byte    res2;
    word    ladr[RTLANCE_LADR_WORD_CNT];
                            /* logical address filter [15:0]       */
                            /* logical address filter [31:16]      */
                            /* logical address filter [47:32]      */
                            /* logical address filter [63:48]      */
    dword   rdr1;       /* rdra [31-00] */
    dword   tdr1;       /* tdra [31-00] */
} RTLANCE_S_INIT_BLK;

struct u_blk 
{
    dword   dword0;
    dword   dword1;
    dword   dword2;
    dword   dword3;
    dword   dword4;
    dword   dword5;
    dword   dword6;
} RTLANCE_U_INIT_BLK;

typedef struct u_blk KS_FAR *PUINIT_BLK;

#else
typedef struct i_blk
{
    word    mode;       /* mode */
    byte    padr[6];
                                /* physical address [15:0] bit0 MBZ    */
                                /* physical address [31:16]            */
                                /* physical address [47:32]            */
    word    ladr[RTLANCE_LADR_WORD_CNT];
                                /* logical address filter [15:0]       */
                                /* logical address filter [31:16]      */
                                /* logical address filter [47:32]      */
                                /* logical address filter [63:48]      */
    word    rdr1;       /* receive descriptor ring info - rdra */
    word    rdr2;       /*                          rlen + rdra */
    word    tdr1;       /* transmit descriptor ring info - tdra */
    word    tdr2;       /*                          tlen + tdra */
} RTLANCE_S_INIT_BLK;
#endif

typedef struct i_blk KS_FAR *PINIT_BLK;
#define SIZEOF_INIT_BLOCK sizeof(RTLANCE_S_INIT_BLK)

/* *********************************************************************              */
/*                                                                                    */
/*  Buffer Management:                                                                */
/*  These are the structures used to manage the receive and transmit data             */
/*  buffers.                                                                          */
/*  There are two rings, one for receive and one for transmit, that hold              */
/*  descriptors pointing to the data buffers and containing status info               */
/*  pertaining to the data buffers. The 16 bit and 32 bit modes use different         */
/*  formats.                                                                          */
/*                                                                                    */
/* 16 bit receive/transmit descriptor format:                                         */
/*                                                                                    */
/*  Each descriptor in a ring (in memory) is a 4-word entry.                          */
/*                                                                                    */
/*  RMD0:   LADR(15:00) - low order 16 bits of data buffer address                    */
/*                                                                                    */
/*  RMD1:   HADR( 7:00) - high order 8 bits of data buffer address                    */
/*          ENP (8)     - End of Packet                                               */
/*          STP (9)     - Start of Packet                                             */
/*          BUFF(10)                                                                  */
/*          CRC (11)                                                                  */
/*          OFLO(12)                                                                  */
/*          FRAM(13)                                                                  */
/*          ERR (14)                                                                  */
/*          OWN (15)    - Owned by host=0 (hosts sets after emptying buffer)          */
/*                        Owned by LANCE=1(LANCE clears after filling buffer)         */
/*                                                                                    */
/*  RMD2:   BCNT(11-00) - Buffer Byte Count (written by host only)                    */
/*          ONES(15:12) - Must be ones (written by host only)                         */
/*                                                                                    */
/*  RMD3:   MCNT(11:00) - Message Byte Count(written by chip; cleared by host)        */
/*          RES (15:12) - Reserved (Read as Zeroes; Write as zeroes)                  */
/*                                                                                    */
/*                                                                                    */
/*  TMD0:   LADR(15:00) - low order 16 bits of data buffer address                    */
/*                                                                                    */
/*  TMD1:   HADR( 7:00) - high order 8 bits of data buffer address                    */
/*          ENP (8)     - End of Packet                                               */
/*          STP (9)     - Start of Packet                                             */
/*          DEF (10)                                                                  */
/*          ONE (11)                                                                  */
/*          MORE(12)                                                                  */
/*          ADD_FCS(13)                                                               */
/*          ERR (14)                                                                  */
/*          OWN (15)    - Owned by host=0 (hosts sets after emptying buffer)          */
/*                        Owned by LANCE=1(LANCE clears after filling buffer)         */
/*                                                                                    */
/*  TMD2:   BCNT(11-00) - Buffer Byte Count (written by host only)                    */
/*          ONES(15:12) - Must be ones (written by host only)                         */
/*                                                                                    */
/*  TMD3:   MCNT(09:00) - Message Byte Count(written by chip; cleared by host)        */
/*          RTRY(10)                                                                  */
/*          LCAR(11)                                                                  */
/*          LCOL(12)                                                                  */
/*          RES (13)                                                                  */
/*          UFLO(14)                                                                  */
/*          BUFF(15)                                                                  */
/*                                                                                    */
/* 32 bit receive/transmit descriptor format:                                         */
/*                                                                                    */
/*  Each descriptor in a ring (in memory) is a 4-double word entry.                   */
/*                                                                                    */
/*  RMD0:   RBADR(15:00) - receive data buffer address                                */
/*                                                                                    */
/*  RMD1:  BCNT(11-00)  - Buffer Byte Count                                           */
/*          ONES(15:12) - Must be ones (written by host only)                         */
/*          RES (19:16) - Reserved (Read as Zeroes; Write as zeroes)                  */
/*          BAM/LAFM/PAM (22,21,20) - Built in broadcast/log/phys addr filter matches */
/*          ENP (24)     - End of Packet                                              */
/*          STP (25)     - Start of Packet                                            */
/*          BUFF(26)                                                                  */
/*          CRC (27)                                                                  */
/*          OFLO(28)                                                                  */
/*          FRAM(29)                                                                  */
/*          ERR (30)                                                                  */
/*          OWN (31)    - Owned by host=0 (hosts sets after emptying buffer)          */
/*                        Owned by LANCE=1(LANCE clears after filling buffer)         */
/*                                                                                    */
/*  RMD2:   MCNT(11:00) - Message Byte Count(written by chip; cleared by host)        */
/*          ZEROS(15:12) - Zeros (written by controller only)                         */
/*          RFRTAG (30:16) - Receive frame tag (see CSR7 b14)                         */
/*          RES (31) - Reserved (Read as Zeroes; Write as zeroes)                     */
/*                                                                                    */
/*  RMD3:   USER(31:00) - Reserved for user defined                                   */
/*                                                                                    */
/*  TMD0:   RBADR(15:00) - Transmit data buffer address                               */
/*                                                                                    */
/*  TMD1:  BCNT(11-00)  - Buffer Byte Count (written by host only)                    */
/*          ONES(15:12) - Must be ones (written by host only)                         */
/*          RES (22:16) - Reserved (Read as Zeroes; Write as zeroes)                  */
/*          BBPE (23) - Bus parity error                                              */
/*          ENP (24)     - End of Packet                                              */
/*          STP (25)     - Start of Packet                                            */
/*          DEF (26)                                                                  */
/*        ONE (27)                                                                    */
/*        LINT/MORE(28)                                                               */
/*          ADD_FCS(29)                                                               */
/*          ERR (30)                                                                  */
/*          OWN (31)    - Owned by host=0 (hosts sets after emptying buffer)          */
/*                        Owned by LANCE=1(LANCE clears after filling buffer)         */
/*  TMD2:   TRC(3:0) - transmit retry count                                           */
/*          RES(25:4) - Reserved                                                      */
/*          REYTRYERR (26) - Rtery error                                              */
/*          LCARERR (27) - loss of carrier                                            */
/*        LCOLERR (28) - late collision                                               */
/*        EXDEFERR (29) - excessive deferral                                          */
/*          UFLOERR (30) - Underflow error, see CSR3 b6 for graceful recovery         */
/*          BUFFERR (31)                                                              */
/*                                                                                    */
/*  TMD3:   USER(31:00) - Reserved for user defined                                   */
/*                                                                                    */
/*                                                                                    */
/*  Constants used for ring definitions and buffer management.                        */
/*                                                                                    */
#if (CFG_AMD_32BIT)
#define RTLANCE_M_CPU_OWNED     0x00000000ul
#define RTLANCE_M_LANCE_OWNED   0x80000000ul
#else
#define RTLANCE_M_CPU_OWNED     0x0000
#define RTLANCE_M_LANCE_OWNED   0x8000
#endif

#define RTLANCE_LOG_RX_BUF      4   /*  2**4 = 16 rx buffers */
#define RTLANCE_LOG_TX_BUF      2   /*  2**2 =  4 tx buffers */
#define NUM_RX_DESC (1 << RTLANCE_LOG_RX_BUF)
#define NUM_TX_DESC (1 << RTLANCE_LOG_TX_BUF)
#define SIZEOF_RX_DESC_ARRAY (NUM_RX_DESC * sizeof(RTLANCE_RX_DESC))
#define SIZEOF_TX_DESC_ARRAY (NUM_TX_DESC * sizeof(RTLANCE_TX_DESC))
#if (CFG_AMD_32BIT)
#define RX_TX_DESC_ALIGN        0x0f    /* align descriptors on 16-byte bound */
#else
#define RX_TX_DESC_ALIGN        7   /* align descriptors on quadword bound */
#endif
#define PKT_BUF_SZ              1544/* max packet size - for shared mem */

/* *********************************************************************    */
/*                                                                          */
/* Macros for common functions used for writing buffer management registers */
/*                                                                          */
#if (!CFG_AMD_32BIT)
                                /* get the low 16 bits of address x   */
#define RTLANCE_LOW_16(x)   (word)kvtop(x)
                                /* get the high 8 bits of address x   */
#define RTLANCE_HIGH_8(x)   (byte)(kvtop(x) >> 16)
                                /* convert to 2's complement       */
#endif

#define RTLANCE_TWOS_COMP(x) (word)(~(x) + 1)
                                /* increment pointers into descriptor rings   */
#define RTLANCE_INC_RX_PTR(x)           \
        {                               \
            if (x >= (NUM_RX_DESC - 1)) \
                x = 0;                  \
            else                        \
                x++;                    \
        }

#define RTLANCE_INC_TX_PTR(x)           \
        {                               \
            if (x >= (NUM_TX_DESC - 1)) \
                x = 0;                  \
            else                        \
                x++;                    \
        }

/* *********************************************************************   */
/*                                                                         */
/* Ring Buffer Register Definitions                                        */
/*                                                                         */
#if (CFG_AMD_32BIT)
#define RTLANCE_M_RMD1_OWN  0x80000000ul
#define RTLANCE_M_RMD1_ERR  0x40000000ul
#define RTLANCE_M_RMD1_FRAM 0x20000000ul
#define RTLANCE_M_RMD1_OFLO 0x10000000ul
#define RTLANCE_M_RMD1_CRC  0x08000000ul
#define RTLANCE_M_RMD1_BUFF 0x04000000ul
#define RTLANCE_M_RMD1_STP  0x02000000ul
#define RTLANCE_M_RMD1_ENP  0x01000000ul
#define RTLANCE_M_RMD1_BPE  0x00800000ul

#define RTLANCE_M_RMD1_ONES 0x0000f000ul
#define RTLANCE_M_RMD1_BCNT 0x00000ffful

#define RTLANCE_M_RMD2_RESV 0xfffff000ul
#define RTLANCE_M_RMD2_MCNT 0x00000ffful

#else
#define RTLANCE_M_RMD1_OWN  0x8000
#define RTLANCE_M_RMD1_ERR  0x4000
#define RTLANCE_M_RMD1_FRAM 0x2000
#define RTLANCE_M_RMD1_OFLO 0x1000
#define RTLANCE_M_RMD1_CRC  0x0800
#define RTLANCE_M_RMD1_BUFF 0x0400
#define RTLANCE_M_RMD1_STP  0x0200
#define RTLANCE_M_RMD1_ENP  0x0100
#define RTLANCE_M_RMD1_HADR 0x00ff

#define RTLANCE_M_RMD2_ONES 0xf000
#define RTLANCE_M_RMD2_BCNT 0x0fff

#define RTLANCE_M_RMD3_RESV 0xf000
#define RTLANCE_M_RMD3_MCNT 0x0fff

#endif
typedef struct rx_desc
{
    RTWORD  rmd0;       /* low 16 bits of rx buffer address  */
    RTWORD  rmd1;       /* msb=OWN,errors,STP,ENP,high 8 bits of rx buffer addr */
    RTWORD  rmd2;       /* bits 15-12 MBZ, bits 11-0 buffer cnt(2's comp) */
    RTWORD  rmd3;       /* bits 15-12 RES, bits 11-0 message byte count */
} RTLANCE_RX_DESC;

typedef struct rx_desc KS_FAR *PRX_DESC;

/* *********************************************************************   */
#if (CFG_AMD_32BIT)
#define RTLANCE_M_TMD1_OWN  0x80000000ul
#define RTLANCE_M_TMD1_ERR  0x40000000ul
#define RTLANCE_M_TMD1_FCS  0x20000000ul
#define RTLANCE_M_TMD1_MORE 0x10000000ul
#define RTLANCE_M_TMD1_ONE  0x08000000ul
#define RTLANCE_M_TMD1_DEF  0x04000000ul
#define RTLANCE_M_TMD1_STP  0x02000000ul
#define RTLANCE_M_TMD1_ENP  0x01000000ul
#define RTLANCE_M_TMD1_HADR 0x00FF0000ul

#define RTLANCE_M_TMD1_ONES 0x0000f000ul
#define RTLANCE_M_TMD1_BCNT 0x00000ffful

#define RTLANCE_M_TMD2_BUFF 0x80000000ul
#define RTLANCE_M_TMD2_UFLO 0x40000000ul
#define RTLANCE_M_TMD2_RESV 0x20000000ul
#define RTLANCE_M_TMD2_LCOL 0x10000000ul
#define RTLANCE_M_TMD2_LCAR 0x08000000ul
#define RTLANCE_M_TMD2_RTRY 0x04000000ul
#define RTLANCE_M_TMD2_TDR  0x03fffff0ul
#define RTLANCE_M_TMD2_TRC  0x0000000ful
#else
#define RTLANCE_M_TMD1_OWN  0x8000
#define RTLANCE_M_TMD1_ERR  0x4000
#define RTLANCE_M_TMD1_FCS  0x2000
#define RTLANCE_M_TMD1_MORE 0x1000
#define RTLANCE_M_TMD1_ONE  0x0800
#define RTLANCE_M_TMD1_DEF  0x0400
#define RTLANCE_M_TMD1_STP  0x0200
#define RTLANCE_M_TMD1_ENP  0x0100
#define RTLANCE_M_TMD1_HADR 0x00FF

#define RTLANCE_M_TMD2_ONES 0xf000
#define RTLANCE_M_TMD2_BCNT 0x0fff

#define RTLANCE_M_TMD3_BUFF 0x8000
#define RTLANCE_M_TMD3_UFLO 0x4000
#define RTLANCE_M_TMD3_RESV 0x2000
#define RTLANCE_M_TMD3_LCOL 0x1000
#define RTLANCE_M_TMD3_LCAR 0x0800
#define RTLANCE_M_TMD3_RTRY 0x0400
#define RTLANCE_M_TMD3_TDR  0x03ff
#endif
typedef struct tx_desc  /* SWSTYLE = 2 */
{
    RTWORD  tmd0;       /* low 16 bits of tx buffer address */
    RTWORD  tmd1;       /* 15-OWN,14-ERR,9-STP,8-ENP,7:0-high 8 bits of tx buffer address */
    RTWORD  tmd2;   
    RTWORD  tmd3;
} RTLANCE_TX_DESC;

typedef struct tx_desc KS_FAR *PTX_DESC;

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

#if (CFG_AMD_32BIT)

#if (!KS_LITTLE_ENDIAN) /* 1=little endian (INTEL) */
    #define SWAPIF(x)  net2hl((dword)(x))          
    #define SWAPIFW(x) net2hs(x)          
#else
    #define SWAPIF(x)  x
    #define SWAPIFW(x) x
#endif

#else       /* CFG_AND_32BIT */

#if (!KS_LITTLE_ENDIAN) /* 1=little endian (INTEL) */
    #define SWAPIF(x) net2hs(x)
#else
    #define SWAPIF(x) x
#endif

#endif  /* CFG_AND_32BIT */


/*                                                                         */
/* The macros INWORD and OUTWORD are usually set up in osenv.h. For 32 bit */
/* operation, redefine to utilize 32 bit routines.                         */
/*                                                                         */
#if (CFG_AMD_32BIT)
#if (PPC603)
#define INWORD(ADDR)      (*(volatile long *)(ADDR))
#define OUTWORD(ADDR,VAL) (*(volatile long *)(ADDR) = (VAL)); sync()

#else
#undef IOADDRESS
#undef INWORD
#undef OUTWORD
#define IOADDRESS unsigned char *
#define INWORD(ADDR)    (*(volatile long *)(ADDR))
#define OUTWORD(ADDR,VAL) (*(volatile long *)(ADDR) = (VAL))
#endif

#else       /* CFG_AMD_32BIT */
#if (PPC603)
#define INWORD(ADDR)    (*(volatile short *)(ADDR))
#define OUTWORD(ADDR,VAL) (*(volatile short *)(ADDR) = (VAL)); sync()
#endif
#endif      /* CFG_AMD_32BIT */


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
    word    ladr[RTLANCE_LADR_WORD_CNT];/* logical address filter settings for init block */
    int     cur_rx;                 /* next available rx_ring entry */
    int     cur_tx;                 /* next available tx_ring entry */
    int     dirty_tx;               /* next tx ring entry to be free()ed. */
    PFBYTE  prx_dcu;                /* address of dcu holding rx ring */
    PFBYTE  ptx_dcu;                /* address of dcu holding tx ring */
    PFBYTE  pinit_dcu;              /* address of dcu holding init block */
#if (CFG_LANCE_SHARED_MEM)
    PFBYTE  rx_dcus[NUM_RX_DESC];   /* table of dcu addresses used for rx data */
    PFBYTE  tx_laddr[NUM_TX_DESC];  /* Host logical address of xmit buffers */
#else
    DCU     rx_dcus[NUM_RX_DESC];   /* table of dcu addresses used for rx data */
#endif
    PRX_DESC  prx_ring; /* pointer to the top of the rx ring buffer */
    PTX_DESC  ptx_ring; /* pointer to the top of the tx ring buffer */
    PINIT_BLK pinit_block; /* pointer to the initialization block */
    struct  ether_statistics    stats;  
} RTLANCE_T_PRIV;
typedef RTLANCE_T_PRIV KS_FAR *PRTLANCE_PRIV;

/* *********************************************************************      */
/*                                                                            */
/*  The following data structures and definitions provide a mapping from the  */
/*
**  Device IDs and Vendor IDs for PCI Devices 
*/
#define RTPCI_V_ID_AMDX                0x1022        /* Vendor ID for AMD            */
#define RTPCI_D_ID_AM7997X            0x2000        /* Device ID for               */
/*  chip ID number to the part number and features.  These are used in the   */
/*  routines rtlance_get_chip_version, rtlance_close and rtlance_statistics. */
/*                                                                           */
#define LANCE_MUST_PAD          0x00000001ul
#define LANCE_ENABLE_AUTOSELECT 0x00000002ul
#define LANCE_MUST_REINIT_RING  0x00000004ul
#define LANCE_MUST_UNRESET      0x00000008ul
#define LANCE_HAS_MISSED_FRAME  0x00000010ul
#define LANCE_MUST_SPND         0x00000020ul
#define LANCE_HAS_MII           0x00000040ul

struct lance_chip_type 
{
    dword id_number;
    char *name;
    int flags;
};

enum {OLD_LANCE = 0, PCNET_ISA=1, PCNET_ISAP=2, PCNET_ISAPA=3, PCNET_PCI=4, PCNET_VLB=5, 
      PCNET_PCI_II=6,PCNET_FASTP=7,PCNET_FAST3=8,PCNET_FAST3A=9,LANCE_UNKNOWN=10};

/*                    */
/*  Support routines  */
/*                    */
RTIP_BOOLEAN rtlance_get_chip_version(PRTLANCE_PRIV);
void rtlance_assign_rx_ring(PRTLANCE_PRIV);
void rtlance_assign_tx_ring(PRTLANCE_PRIV);
void rtlance_assign_init_block(PRTLANCE_PRIV);
RTIP_BOOLEAN rtlance_init_rx_entry(PRTLANCE_PRIV, int);
RTIP_BOOLEAN rtlance_init_tx_entry(PRTLANCE_PRIV, int);
void rtlance_build_init_block(PRTLANCE_PRIV);
RTIP_BOOLEAN rtlance_init(PRTLANCE_PRIV);
void rtlance_stop(PRTLANCE_PRIV);
void rtlance_free_dcus(PRTLANCE_PRIV);
void rtlance_reinit(PRTLANCE_PRIV);
void rtlance_reorder_buffers(PRTLANCE_PRIV);
void rtlance_rx_interrupt(PRTLANCE_PRIV);
void rtlance_tx_interrupt(PRTLANCE_PRIV);
void rtlance_errors(dword, PRTLANCE_PRIV);
PRTLANCE_PRIV off_to_priv(int);
PRTLANCE_PRIV iface_to_priv(PIFACE);
void rtlance_setup_dma(PRTLANCE_PRIV);
void rtlance_dma_init(int);
void rtlance_set_mcast_list(PRTLANCE_PRIV, int, void *);
RTIP_BOOLEAN rtlance_device_found(PIFACE, unsigned char, unsigned char);
RTIP_BOOLEAN rtlance_set_intline(PRTLANCE_PRIV, unsigned char, unsigned char);
RTIP_BOOLEAN rtlance_set_iobase(PRTLANCE_PRIV, unsigned char, unsigned char);
RTIP_BOOLEAN rtlance_set_cmd(unsigned char, unsigned char);
void rtlance_set_phy_speed(PRTLANCE_PRIV);
void rtlance_get_phy_speed(PRTLANCE_PRIV);
RTIP_BOOLEAN rtlance_change_speed(PIFACE, int);
RTIP_BOOLEAN rtlance_set_loopback(PIFACE, int);
void rtlance_loopback(PRTLANCE_PRIV);
void rtlance_spnd(PRTLANCE_PRIV);
void rtlance_restart(PRTLANCE_PRIV);
void rtlance_phy_reset(PRTLANCE_PRIV);
dword rtlance_longswap(dword l);          /* change 4 bytes in long word 0,1,2,3 -> 3,2,1,0 */
word rtlance_wordswap(word w);          /* change 2 bytes in word 0,1 -> 1,0 */
word rtlance_dump_csr(PIFACE, word, word);
word rtlance_dump_bcr(PIFACE, word, word);
word rtlance_dump_anr(PRTLANCE_PRIV p_priv, word reg, word suspend);


#if (CFG_AMD_32BIT)
void rtlance_write_MIIreg(PRTLANCE_PRIV, dword, dword);
void rtlance_read_MIIreg(PRTLANCE_PRIV, dword, dword *);
void rtlance_write_BCR32(PRTLANCE_PRIV, dword, dword);
void rtlance_invoke_32bit(PRTLANCE_PRIV);
#else
void rtlance_write_MIIreg(PRTLANCE_PRIV, word, word);
void rtlance_read_MIIreg(PRTLANCE_PRIV, word, word *);
void rtlance_write_BCR32(PRTLANCE_PRIV, word, word);
#endif

RTIP_BOOLEAN rtlance_pci_init(PIFACE, PRTLANCE_PRIV);
void dump_lance(IOADDRESS, word, word);

#if (PPC603)
void sync();
#endif

