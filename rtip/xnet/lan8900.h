/***********************************************************************   */
/*                                                                         */
/*   MODULE:     lan8900.h                                                 */
/*   DATE:       5/7/96                                                    */
/*   PURPOSE:    Crystal Semiconductor CS8900 driver for pSOS/x86          */
/*   PROGRAMMER: Quentin Stephenson                                        */
/*                                                                         */
/*----------------------------------------------------------------------   */
/*                                                                         */
/*               Copyright 1996, Crystal Semiconductor Corp.               */
/*                      ALL RIGHTS RESERVED                                */
/*                                                                         */
/***********************************************************************   */

#ifndef __LAN8900__
#define __LAN8900__ 1

#include "rtip.h"

#define BSP_CS8900_IRQ        10             /*OS*/  /* added */
#define CS8900A_ETH_ADDR_LO   0x04030200UL   /*OS*/  /* added */
#define CS8900A_ETH_ADDR_HI   0x00000005UL   /*OS*/  /* added */



#define SUPPORTS_CS8920 0                    /*OS*/ /* was 1 */
#define BSP_CS8900_MEM_MODE 0       /* 1 to turn on memory mode */

#define INCLUDE_CS89X0_EEPROM   0            /*OS*/  /* added */


#define TEN_BASE_T 1
#define TEN_BASE_2 0

#define LOW 1
#define HIGH 1
#define BSP_CS8900_DCDC_POL LOW

#define BSP_CS8900_EEPROM       0            /*OS*/ /* was 1 */
#define BSP_CS8900_IO_BASE      (cs->io_address)

#define BSP_CS8900_MEM_MODE     0
#define BSP_CS8900_USE_SA       1       /* used by MEM_MODE */
#define BSP_CS8900_IOCHRDY      1       /* used if BSP_CS8900_EEPROM is 0 */
#define BSP_CS8900_MEDIA_TYPE   TEN_BASE_T  /* used if MEM_MODE and  */
                                            /* if BSP_CS8900_EEPROM is 0   */
#define BSP_CS8900_FDX          1       /* used by TEN_BASE_T */

#define CS_NULL     ((void *)(0))

#define MAXLOOP       0x8888

/* *********************************************************************   */
#define DISPLAY_LAN8900_OUTPUT 0

#if (DISPLAY_LAN8900_OUTPUT)
/* *********************************************************************   */
#define LAN_OUTBYTE(ADD, VAL)   \
    DEBUG_ERROR("OUTBYTE: WRITE TO ADDR, VALUE: ", DINT2, ADD, VAL); \
    OUTBYTE((ADD), (VAL) )
#define LAN_OUTWORD(ADD, VAL)   \
    DEBUG_ERROR("OUTWORD: WRITE TO ADDR, VALUE: ", DINT2, ADD, VAL); \
    OUTWORD((ADD), (VAL) )
#define LAN_OUTDWORD(ADDR, VAL) \
    DEBUG_ERROR("OUTDWORD: WRITE TO ADDR, VALUE: ", DINT2, ADD, VAL); \
    OUTDWORD ((ADDR), (VAL))
#else
#define LAN_OUTBYTE(ADD, VAL)   \
    OUTBYTE((ADD), (VAL) )
#define LAN_OUTWORD(ADD, VAL)   \
    OUTWORD((ADD), (VAL) )
#define LAN_OUTDWORD(ADDR, VAL) \
    OUTDWORD ((ADDR), (VAL))
#endif

/* *********************************************************************   */
#define SIGNATURE_PKTPG_PTR 0x3000
#define EISA_NUM_CRYSTAL    0x630E
#define PROD_ID_MASK        0xE000
#define PROD_ID_CS8900      0x0000
#define PROD_ID_CS8920      0x4000
#define PROD_ID_CS892X      0x6000
#define PROD_REV_MASK       0x1F00

/* IO Port Addresses   */

#define PORT_RXTX_DATA     (BSP_CS8900_IO_BASE+0x00)
#define PORT_RXTX_DATA_1   (BSP_CS8900_IO_BASE+0x02)
#define PORT_TX_CMD        (BSP_CS8900_IO_BASE+0x04)
#define PORT_TX_LENGTH     (BSP_CS8900_IO_BASE+0x06)
#define PORT_ISQ           (BSP_CS8900_IO_BASE+0x08)
#define PORT_PKTPG_PTR     (BSP_CS8900_IO_BASE+0x0A)
#define PORT_PKTPG_DATA    (BSP_CS8900_IO_BASE+0x0C)
#define PORT_PKTPG_DATA_1  (BSP_CS8900_IO_BASE+0x0E)


/* PacketPage Offsets   */

#define PKTPG_EISA_NUM     0x0000
#define PKTPG_PRODUCT_ID   0x0002
#define PKTPG_IO_BASE      0x0020
#define PKTPG_INT_NUM      0x0022
#define PKTPG_INTNUM_20    0x0370   /*  ISA interrupt select - CS8920 */
#define PKTPG_MEM_BASE     0x002C
#define PKTPG_MEME_BASE_20 0x0348
#define PKTPG_EEPROM_CMD   0x0040
#define PKTPG_EEPROM_DATA  0x0042
#define PKTPG_RX_CFG       0x0102
#define PKTPG_RX_CTL       0x0104
#define PKTPG_TX_CFG       0x0106
#define PKTPG_BUF_CFG      0x010A
#define PKTPG_LINE_CTL     0x0112
#define PKTPG_SELF_CTL     0x0114
#define PKTPG_BUS_CTL      0x0116
#define PKTPG_TEST_CTL     0x0118
#define PKTPG_ISQ          0x0120
#define PKTPG_RX_EVENT     0x0124
#define PKTPG_TX_EVENT     0x0128
#define PKTPG_BUF_EVENT    0x012C
#define PKTPG_RX_MISS      0x0130
#define PKTPG_TX_COL       0x0132
#define PKTPG_LINE_ST      0x0134
#define PKTPG_SELF_ST      0x0136
#define PKTPG_BUS_ST       0x0138
#define PKTPG_TX_CMD       0x0144
#define PKTPG_TX_LENGTH    0x0146
#define PKTPG_IND_ADDR     0x0158
#define PKTPG_RX_STATUS    0x0400
#define PKTPG_RX_LENGTH    0x0402
#define PKTPG_RX_FRAME     0x0404
#define PKTPG_TX_FRAME     0x0A00
#define CS8920_NO_INTS 0x0F   /*  Max CS8920 interrupt select # */


/* EEPROM Offsets   */

#define EEPROM_IND_ADDR_H  0x001C
#define EEPROM_IND_ADDR_M  0x001D
#define EEPROM_IND_ADDR_L  0x001E
#define EEPROM_ISA_CFG     0x001F
#define EEPROM_MEM_BASE    0x0020
#define EEPROM_XMIT_CTL    0x0023
#define EEPROM_ADPTR_CFG   0x0024


/* Register Numbers   */

#define REG_NUM_MASK       0x003F
#define REG_NUM_RX_EVENT   0x0004
#define REG_NUM_TX_EVENT   0x0008
#define REG_NUM_BUF_EVENT  0x000C
#define REG_NUM_RX_MISS    0x0010
#define REG_NUM_TX_COL     0x0012


/* Self Control Register   */

#define SELF_CTL_RESET     0x0040
#define SELF_CTL_HC1E      0x2000
#define SELF_CTL_HCB1      0x8000


/* Self Status Register   */

#define SELF_ST_INIT_DONE  0x0080
#define SELF_ST_SI_BUSY    0x0100
#define SELF_ST_EEP_PRES   0x0200
#define SELF_ST_EEP_OK     0x0400
#define SELF_ST_EL_PRES    0x0800


/* EEPROM Command Register   */

#define EEPROM_CMD_READ    0x0200
#define EEPROM_CMD_ELSEL   0x0400


/* Bus Control Register   */

#define BUS_CTL_USE_SA     0x0200
#define BUS_CTL_MEM_MODE   0x0400
#define BUS_CTL_IOCHRDY    0x1000
#define BUS_CTL_INT_ENBL   0x8000


/* Bus Status Register   */

#define BUS_ST_TX_BID_ERR  0x0080
#define BUS_ST_RDY4TXNOW   0x0100

/* @jla added for rdy4tx interrupt  */

/* Buf Event Register   */

#define BUF_EVENT_RDY4TX   0x0100

/* Line Control Register   */

#define LINE_CTL_RX_ON          0x0040
#define LINE_CTL_TX_ON          0x0080
#define LINE_CTL_AUI_ONLY       0x0100
#define LINE_CTL_10BASET        0x0000
#define LINE_CTL_LOWRX_SQUELCH  0x4000


/* Test Control Register   */

#define TEST_CTL_DIS_LT    0x0080
#define TEST_CTL_ENDEC_LP  0x0200
#define TEST_CTL_AUI_LOOP  0x0400
#define TEST_CTL_DIS_BKOFF 0x0800
#define TEST_CTL_FDX       0x4000


/* Receiver Configuration Register   */

#define RX_CFG_SKIP        0x0040
#define RX_CFG_RX_OK_IE    0x0100
#define RX_CFG_CRC_ERR_IE  0x1000
#define RX_CFG_RUNT_IE     0x2000
#define RX_CFG_X_DATA_IE   0x4000


/* Receiver Event Register   */

#define RX_EVENT_RX_OK     0x0100
#define RX_EVENT_IND_ADDR  0x0400
#define RX_EVENT_MCAST     0x0200
#define RX_EVENT_BCAST     0x0800
#define RX_EVENT_CRC_ERR   0x1000
#define RX_EVENT_RUNT      0x2000
#define RX_EVENT_X_DATA    0x4000
#define DEF_RX_ACCEPT (RX_EVENT_IND_ADDR | RX_EVENT_BCAST | RX_EVENT_RX_OK)

/* Receiver Control Register   */

#define RX_CTL_RX_OK_A     0x0100
#define RX_CTL_MCAST_A     0x0200
#define RX_CTL_IND_A       0x0400
#define RX_CTL_BCAST_A     0x0800
#define RX_CTL_CRC_ERR_A   0x1000
#define RX_CTL_RUNT_A      0x2000
#define RX_CTL_X_DATA_A    0x4000


/* Transmit Configuration Register   */

#define TX_CFG_LOSS_CRS_IE 0x0040
#define TX_CFG_SQE_ERR_IE  0x0080
#define TX_CFG_TX_OK_IE    0x0100
#define TX_CFG_OUT_WIN_IE  0x0200
#define TX_CFG_JABBER_IE   0x0400
#define TX_CFG_ANY_COL_ENBL 0x0800
#define TX_CFG_16_COLL_IE  0x8000
#define TX_CFG_ALL_IE      0x8FC0


/* Transmit Event Register   */

#define TX_EVENT_TX_OK     0x0100
#define TX_EVENT_OUT_WIN   0x0200
#define TX_EVENT_JABBER    0x0400
#define TX_EVENT_16_COLL   0x1000
#define RX_MISS_COUNT_OVRFLOW_ENBL 0x2000

/* Transmit Command Register   */

#define TX_CMD_START_5     0x0000
#define TX_CMD_START_381   0x0080
#define TX_CMD_START_1021  0x0040
#define TX_CMD_START_ALL   0x00C0
#define TX_CMD_FORCE       0x0100
#define TX_CMD_ONE_COLL    0x0200
#define TX_CMD_NO_CRC      0x1000
#define TX_CMD_NO_PAD      0x2000


/* Buffer Configuration Register   */

#define BUF_CFG_SW_INT     0x0040
#define BUF_CFG_RDY4TX_IE  0x0100
#define BUF_CFG_TX_UNDR_IE 0x0200


/* ISA Configuration from EEPROM   */

#define ISA_CFG_IRQ_MASK   0x000F
#define ISA_CFG_USE_SA     0x0080
#define ISA_CFG_IOCHRDY    0x0100
#define ISA_CFG_MEM_MODE   0x8000


/* Memory Base from EEPROM   */

#define MEM_BASE_MASK      0xFFF0


/* Adpater Configuration from EEPROM (offset = EEPROM_ADPTR_CFG)   */

#define ADPTR_CFG_MEDIA    0x0060
#define ADPTR_CFG_10BASET  0x0020
#define ADPTR_CFG_AUI      0x0040
#define ADPTR_CFG_10BASE2  0x0060
#define ADPTR_CFG_DCDC_POL 0x0080
/* Transmission Control from EEPROM   */

#define XMIT_CTL_FDX       0x8000

typedef struct cs_parms
{
    PIFACE      pi;
    RTIP_BOOLEAN    wait_alloc;
    word        csHardwareAddr[3];
    int         csIntNumber;
    IOADDRESS   io_address;             /* io_address (if not 0) */
    word        mem_address;            /* memory address (if not 0) */
    word *      cs_pPacketPage;
    int         csInMemoryMode;
    int         chip_type;              /* one of: CS8900, CS8920, CS8920M */
} CS_PARMS;

typedef struct cs_parms KS_FAR * PCS_PARMS;

#endif          /* LAN8900 */
