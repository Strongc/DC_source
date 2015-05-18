/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
*                                                                    *
*       USB device stack for embedded applications                   *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : USB_HW_Dummy.c
Purpose : USB driver template
--------  END-OF-HEADER  ---------------------------------------------
*/
#ifndef _USB_HW_DRIVER_H_
#define _USB_HW_DRIVER_H_

#include "hw_res.h"
#include "typedef.h" 

#define TCS_USB_SER_PORT 1

#define RXEP0   0x00
#define TXEP1	0x81
#define RXEP2	0x02
#define TXEP3	0x83
#define RXEP4	0x04
#define TXEP5	0x85
#define RXEP6	0x06
#define ERR_EPN 0xFF


#define EPN0    0x00
#define EPN1	0x01
#define EPN2	0x02
#define EPN3	0x03
#define EPN4	0x04
#define EPN5	0x05
#define EPN6	0x06


#define	USBFU_BAR_VALUE				0xA8400000
#define	USBHU_BAR_VALUE				0xA8200000


/* USB Function Operation Registers */
#define USBF_GMODE					( 0x00 + USBFU_BAR_VALUE )
#define USBF_VERSION				( 0x04 + USBFU_BAR_VALUE )
#define USBF_GSTAT1					( 0x10 + USBFU_BAR_VALUE )
#define USBF_INTMSK1				( 0x14 + USBFU_BAR_VALUE )
#define USBF_GSTAT2					( 0x18 + USBFU_BAR_VALUE )
#define USBF_INTMSK2				( 0x1C + USBFU_BAR_VALUE )
#define USBF_EP0_CONT				( 0x20 + USBFU_BAR_VALUE )
#define USBF_EP1_CONT				( 0x24 + USBFU_BAR_VALUE )
#define USBF_EP2_CONT				( 0x28 + USBFU_BAR_VALUE )
#define USBF_EP3_CONT				( 0x2C + USBFU_BAR_VALUE )
#define USBF_EP4_CONT				( 0x30 + USBFU_BAR_VALUE )
#define USBF_EP5_CONT				( 0x34 + USBFU_BAR_VALUE )
#define USBF_EP6_CONT				( 0x38 + USBFU_BAR_VALUE )
#define USBF_CMD					( 0x40 + USBFU_BAR_VALUE )
#define USBF_CMD_ADDRESS			( 0x44 + USBFU_BAR_VALUE )
#define USBF_TxEP_STAT				( 0x48 + USBFU_BAR_VALUE )
#define USBF_RxP0_INFO				( 0x50 + USBFU_BAR_VALUE )
#define USBF_RxP0_ADDRESS			( 0x54 + USBFU_BAR_VALUE )
#define USBF_RxP1_INFO				( 0x58 + USBFU_BAR_VALUE )
#define USBF_RxP1_ADDRESS			( 0x5C + USBFU_BAR_VALUE )
#define USBF_RxP2_INFO				( 0x60 + USBFU_BAR_VALUE )
#define USBF_RxP2_ADDRESS			( 0x64 + USBFU_BAR_VALUE )
#define USBF_TxMail_START_ADDR		( 0x70 + USBFU_BAR_VALUE )
#define USBF_TxMail_BTM_ADDR		( 0x74 + USBFU_BAR_VALUE )
#define USBF_TxMail_RD_ADDR			( 0x78 + USBFU_BAR_VALUE )
#define USBF_TxMail_WR_ADDR			( 0x7C + USBFU_BAR_VALUE )
#define USBF_RxMail_START_ADDR		( 0x80 + USBFU_BAR_VALUE )
#define USBF_RxMail_BTM_ADDR		( 0x84 + USBFU_BAR_VALUE )
#define USBF_RxMail_RD_ADDR			( 0x88 + USBFU_BAR_VALUE )
#define USBF_RxMail_WR_ADDR			( 0x8C + USBFU_BAR_VALUE )
#define USBF_EP0_TxDATA_NAK			( 0x100 + USBFU_BAR_VALUE )
#define USBF_EP0_TxDATA_STL			( 0x104 + USBFU_BAR_VALUE )
#define USBF_EP0_TxHANDSHK_TIMEOUT	( 0x110 + USBFU_BAR_VALUE )
#define USBF_EP0_RxDATA_TIMEOUT		( 0x120 + USBFU_BAR_VALUE )
#define USBF_EP0_RxDATA_CRC_ERR		( 0x128 + USBFU_BAR_VALUE )
#define USBF_EP0_RxDATA_BIT_ERR		( 0x12C + USBFU_BAR_VALUE )
#define	USBF_EP0_RxHANDSHK_TIMEOUT	( 0x130 + USBFU_BAR_VALUE )
#define USBF_EP0_RxDATA_NAK			( 0x134 + USBFU_BAR_VALUE )
#define USBF_EP0_RxDATA_STL			( 0x138 + USBFU_BAR_VALUE )
#define USBF_EP2_DATA_TIMEOUT		( 0x160 + USBFU_BAR_VALUE )
#define USBF_EP2_DATA_CRC_ERR		( 0x164 + USBFU_BAR_VALUE )
#define USBF_EP2_DATA_BITSTUFF_ERR	( 0x168 + USBFU_BAR_VALUE )
#define USBF_EP3_DATA_NAK			( 0x180 + USBFU_BAR_VALUE )
#define USBF_EP3_DATA_STL			( 0x184 + USBFU_BAR_VALUE )
#define USBF_EP3_HANDSHK_TIMEOUT	( 0x190 + USBFU_BAR_VALUE )
#define	USBF_EP4_DATA_TIMEOUT		( 0x1a0 + USBFU_BAR_VALUE )
#define USBF_EP4_DATA_CRC_ERR		( 0x1a4 + USBFU_BAR_VALUE )
#define USBF_EP4_DATA_BITSTUFF_ERR	( 0x1a8 + USBFU_BAR_VALUE )
#define	USBF_EP4_DATA_DTOGGLE_ERR	( 0x1ac + USBFU_BAR_VALUE )
#define USBF_EP4_HANDSHK_TIMEOUT	( 0x1b0 + USBFU_BAR_VALUE )
#define	USBF_EP4_HANDSHK_NAK		( 0x1b4 + USBFU_BAR_VALUE )
#define USBF_EP4_HANDSHK_STL		( 0x1b8 + USBFU_BAR_VALUE )
#define	USBF_EP5_DATA_NAK			( 0x1c0 + USBFU_BAR_VALUE )
#define USBF_EP5_DATA_STL			( 0x1c4 + USBFU_BAR_VALUE )
#define USBF_EP5_HANDSHK_TIMEOUT	( 0x1d0 + USBFU_BAR_VALUE )
#define	USBF_EP6_DATA_TIMEOUT		( 0x1e0 + USBFU_BAR_VALUE )
#define USBF_EP6_DATA_CRC_ERR		( 0x1e4 + USBFU_BAR_VALUE )
#define USBF_EP6_DATA_BITSTUFF_ERR	( 0x1e8 + USBFU_BAR_VALUE )
#define	USBF_EP6_DATA_DTOGGLE_ERR	( 0x1ec + USBFU_BAR_VALUE )
#define USBF_EP6_HANDSHK_TIMEOUT	( 0x1f0 + USBFU_BAR_VALUE )
#define	USBF_EP6_HANDSHK_NAK		( 0x1f4 + USBFU_BAR_VALUE )
#define USBF_EP6_HANDSHK_STL		( 0x1f8 + USBFU_BAR_VALUE )


#define GENI_USBF_GMODE					(*(volatile unsigned long int *)(USBF_GMODE)  )
#define GENI_USBF_VERSION				(*(volatile unsigned long int *)(USBF_VERSION)  )
#define GENI_USBF_GSTAT1				(*(volatile unsigned long int *)(USBF_GSTAT1)  )
#define GENI_USBF_INTMSK1				(*(volatile unsigned long int *)(USBF_INTMSK1)  )
#define GENI_USBF_GSTAT2				(*(volatile unsigned long int *)(USBF_GSTAT2)  )
#define GENI_USBF_INTMSK2				(*(volatile unsigned long int *)(USBF_INTMSK2)  )
#define GENI_USBF_EP0_CONT				(*(volatile unsigned long int *)(USBF_EP0_CONT)  )
#define GENI_USBF_EP1_CONT				(*(volatile unsigned long int *)(USBF_EP1_CONT)  )
#define GENI_USBF_EP2_CONT				(*(volatile unsigned long int *)(USBF_EP2_CONT)  )
#define GENI_USBF_EP3_CONT				(*(volatile unsigned long int *)(USBF_EP3_CONT)  )
#define GENI_USBF_EP4_CONT				(*(volatile unsigned long int *)(USBF_EP4_CONT)  )
#define GENI_USBF_EP5_CONT				(*(volatile unsigned long int *)(USBF_EP5_CONT)  )
#define GENI_USBF_EP6_CONT				(*(volatile unsigned long int *)(USBF_EP6_CONT)  )
#define GENI_USBF_CMD					(*(volatile unsigned long int *)(USBF_CMD)  )
#define GENI_USBF_CMD_ADDRESS			(*(volatile unsigned long int *)(USBF_CMD_ADDRESS)  )
#define GENI_USBF_TxEP_STAT				(*(volatile unsigned long int *)(USBF_TxEP_STAT)  )
#define GENI_USBF_RxP0_INFO				(*(volatile unsigned long int *)(USBF_RxP0_INFO)  )
#define GENI_USBF_RxP0_ADDRESS			(*(volatile unsigned long int *)(USBF_RxP0_ADDRESS)  )
#define GENI_USBF_RxP1_INFO				(*(volatile unsigned long int *)(USBF_RxP1_INFO)  )
#define GENI_USBF_RxP1_ADDRESS			(*(volatile unsigned long int *)(USBF_RxP1_ADDRESS)  )
#define GENI_USBF_RxP2_INFO				(*(volatile unsigned long int *)(USBF_RxP2_INFO)  )
#define GENI_USBF_RxP2_ADDRESS			(*(volatile unsigned long int *)(USBF_RxP2_ADDRESS)  )
#define GENI_USBF_TxMail_START_ADDR		(*(volatile unsigned long int *)(USBF_TxMail_START_ADDR)  )
#define GENI_USBF_TxMail_BTM_ADDR		(*(volatile unsigned long int *)(USBF_TxMail_BTM_ADDR)  )
#define GENI_USBF_TxMail_RD_ADDR		(*(volatile unsigned long int *)(USBF_TxMail_RD_ADDR)  )
#define GENI_USBF_TxMail_WR_ADDR		(*(volatile unsigned long int *)(USBF_TxMail_WR_ADDR)  )
#define GENI_USBF_RxMail_START_ADDR		(*(volatile unsigned long int *)(USBF_RxMail_START_ADDR)  )
#define GENI_USBF_RxMail_BTM_ADDR		(*(volatile unsigned long int *)(USBF_RxMail_BTM_ADDR)  )
#define GENI_USBF_RxMail_RD_ADDR		(*(volatile unsigned long int *)(USBF_RxMail_RD_ADDR)  )
#define GENI_USBF_RxMail_WR_ADDR		(*(volatile unsigned long int *)(USBF_RxMail_WR_ADDR)  )
#define GENI_USBF_EP0_TxDATA_NAK		(*(volatile unsigned long int *)(USBF_EP0_TxDATA_NAK)  )
#define GENI_USBF_EP0_TxDATA_STL		(*(volatile unsigned long int *)(USBF_EP0_TxDATA_STL)  )
#define GENI_USBF_EP0_TxHANDSHK_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP0_TxHANDSHK_TIMEOUT)  )
#define GENI_USBF_EP0_RxDATA_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP0_RxDATA_TIMEOUT)  )
#define GENI_USBF_EP0_RxDATA_CRC_ERR	(*(volatile unsigned long int *)(USBF_EP0_RxDATA_CRC_ERR)  )
#define GENI_USBF_EP0_RxDATA_BIT_ERR	(*(volatile unsigned long int *)(USBF_EP0_RxDATA_BIT_ERR)  )
#define GENI_USBF_EP0_RxHANDSHK_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP0_RxHANDSHK_TIMEOUT)  )
#define GENI_USBF_EP0_RxDATA_NAK		(*(volatile unsigned long int *)(USBF_EP0_RxDATA_NAK)  )
#define GENI_USBF_EP0_RxDATA_STL		(*(volatile unsigned long int *)(USBF_EP0_RxDATA_STL)  )
#define GENI_USBF_EP2_DATA_TIMEOUT		(*(volatile unsigned long int *)(USBF_EP2_DATA_TIMEOUT)  )
#define GENI_USBF_EP2_DATA_CRC_ERR		(*(volatile unsigned long int *)(USBF_EP2_DATA_CRC_ERR)  )
#define GENI_USBF_EP2_DATA_BITSTUFF_ERR	(*(volatile unsigned long int *)(USBF_EP2_DATA_BITSTUFF_ERR)  )
#define GENI_USBF_EP3_DATA_NAK			(*(volatile unsigned long int *)(USBF_EP3_DATA_NAK)  )
#define GENI_USBF_EP3_DATA_STL			(*(volatile unsigned long int *)(USBF_EP3_DATA_STL)  )
#define GENI_USBF_EP3_HANDSHK_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP3_HANDSHK_TIMEOUT)  )
#define GENI_USBF_EP4_DATA_TIMEOUT		(*(volatile unsigned long int *)(USBF_EP4_DATA_TIMEOUT)  )
#define GENI_USBF_EP4_DATA_CRC_ERR		(*(volatile unsigned long int *)(USBF_EP4_DATA_CRC_ERR)  )
#define GENI_USBF_EP4_DATA_BITSTUFF_ERR	(*(volatile unsigned long int *)(USBF_EP4_DATA_BITSTUFF_ERR)  )
#define GENI_USBF_EP4_DATA_DTOGGLE_ERR	(*(volatile unsigned long int *)(USBF_EP4_DATA_DTOGGLE_ERR)  )
#define GENI_USBF_EP4_HANDSHK_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP4_HANDSHK_TIMEOUT)  )
#define GENI_USBF_EP4_HANDSHK_NAK		(*(volatile unsigned long int *)(USBF_EP4_HANDSHK_NAK)  )
#define GENI_USBF_EP4_HANDSHK_STL		(*(volatile unsigned long int *)(USBF_EP4_HANDSHK_STL)  )
#define GENI_USBF_EP5_DATA_NAK			(*(volatile unsigned long int *)(USBF_EP5_DATA_NAK)  )
#define GENI_USBF_EP5_DATA_STL			(*(volatile unsigned long int *)(USBF_EP5_DATA_STL)  )
#define GENI_USBF_EP5_HANDSHK_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP5_HANDSHK_TIMEOUT)  )
#define GENI_USBF_EP6_DATA_TIMEOUT		(*(volatile unsigned long int *)(USBF_EP6_DATA_TIMEOUT)  )
#define GENI_USBF_EP6_DATA_CRC_ERR		(*(volatile unsigned long int *)(USBF_EP6_DATA_CRC_ERR)  )
#define GENI_USBF_EP6_DATA_BITSTUFF_ERR	(*(volatile unsigned long int *)(USBF_EP6_DATA_BITSTUFF_ERR)  )
#define GENI_USBF_EP6_DATA_DTOGGLE_ERR	(*(volatile unsigned long int *)(USBF_EP6_DATA_DTOGGLE_ERR)  )
#define GENI_USBF_EP6_HANDSHK_TIMEOUT	(*(volatile unsigned long int *)(USBF_EP6_HANDSHK_TIMEOUT)  )
#define GENI_USBF_EP6_HANDSHK_NAK		(*(volatile unsigned long int *)(USBF_EP6_HANDSHK_NAK)  )
#define GENI_EP6_HANDSHK_STL			(*(volatile unsigned long int *)(USBF_EP6_HANDSHK_STL)  )

/* Default Register Value */
#define DefaultUSBF_GMODE			0x00001800
#define DefaultUSBF_VERSION			0x02000000

/* Clear STALL on EP3, EP4, EP5 and EP6 ****/
#define STALL_CLEAR 0xfffbffff

/*----------------------------------------------------------------------*/
/* Number of BufferDescriptors in RxBufferDirectory						*/
/*----------------------------------------------------------------------*/
#define SN_DESCNUM_IN_DIR	1

/*----------------------------------------------------------------------*/
/* Size of RxBuffer	(Byte)												*/
/*----------------------------------------------------------------------*/
//#define SN_RX_BUFSIZE	0x600	/* 1536byte */
#define SN_RX_BUFSIZE	0x40	/* 64byte */

/*----------------------------------------------------------------------*/
/* Number of BuffDirectory in the Pool									*/
/*----------------------------------------------------------------------*/
#define SN_DIRNUM_POOL0	20//20
#define SN_DIRNUM_POOL1	20//100
#define SN_DIRNUM_POOL2	20//256

#define USB_ERR_TIMEOUT  (U32)4000 //XFK: what is the timeout in ms?
#define USB_TX_WAIT_TIME (U32)6 //XFK: value 6 is found by trial and error.

/*----------------------------------------------------------------------*/
/* Number of MailBoxes													*/
/*----------------------------------------------------------------------*/
#define SN_TX_MLBNUM 16
#define SN_RX_MLBNUM (SN_DIRNUM_POOL0 + SN_DIRNUM_POOL1 + SN_DIRNUM_POOL2)

/*----------------------------------------------------------------------*/
/* Constant Nunber to check TxModeFlag and RxModeFlag					*/
/*----------------------------------------------------------------------*/
#define	CTL_RXMODE	0x00000000
#define INT_RXMODE	0x00000001
#define ISO_RXMODE	0x00000002
#define BULK_RXMODE	0x00000004

#define	CTL_TXMODE	0x00000000
#define	ISO_TXMODE	0x00000001
#define	BULK_TXMODE	0x00000002
#define	INT_TXMODE	0x00000003

/*----------------------------------------------------------------------*/
/* Constant Nunber to check TxModeFlag and RxModeFlag					*/
/*----------------------------------------------------------------------*/
#define EXTRA_32BYTE	32

/*----------------------------------------------------------------------*/
/* error status of TX command issue                 					*/
/*----------------------------------------------------------------------*/
#define	SN_TX_SUCCESS		0
#define	SN_TEPSR_BUSY		1
#define	SN_TXMODE_ERROR		2
#define	SN_TXEPN_DESABLE	3

/*----------------------------------------------------------------------*/
/* Number of Que and Size of Que										*/
/*----------------------------------------------------------------------*/
#define	SN_MAXMSGS	100
#define	SA_MAXMSGS	100

#define	SN_QSIZE	6
#define	SA_QSIZE	4

/*---------------------------------------------------------------------------*/
/* U_GMR - USB General Mode Register (00H)                          		 */
/*---------------------------------------------------------------------------*/
#define	ADD_CLEAR	0xff80ffff


/*********************************************************************
**	SNOWMAN Status Value  
**								U_GSR1 - USB General Status Register 1 (10H R)
**								U_GSR2 - USB General Status Register 2 (18H R)
*********************************************************************/
/*	U_GSR1	*/
#define	GSR2		0x80000000
#define TMF			0x00800000
#define	RMF			0x00400000
#define	RPE2		0x00200000
#define	RPE1		0x00100000
#define	RPE0		0x00080000
#define	RPA2		0x00040000
#define	RPA1		0x00020000
#define	RPA0		0x00010000
#define	DER			0x00000400
#define	EP2FO		0x00000200
#define	EP1FU		0x00000100
#define EP6RF		0x00000080
#define EP5TF		0x00000040
#define	EP4RF		0x00000020
#define	EP3TF		0x00000010
#define	EP2RF		0x00000008
#define	EP1TF		0x00000004
#define	EP0RF		0x00000002
#define	EP0TF		0x00000001

/*	U_GSR2	*/
#define	FW		    0x00200000
#define	IFN		    0x00100000
#define	IEA		    0x00080000
#define	URSM		0x00040000
#define	URST		0x00020000
#define	USPD		0x00010000
#define	EP2OS		0x00000080
#define	EP2ED		0x00000040
#define	EP2ND		0x00000020
#define	EP1NT		0x00000010
#define	EP1ET		0x00000008
#define	EP1ND		0x00000004
#define ES			0x00000002
#define	SL			0x00000001


#define	GSR2_ERROR	0x003F00FF
#define TX_FINISH	0x00000055
#define RX_FINISH	0x000000AA
#define	GSR1_ERROR	0x80FF0700

#define GSR1_ALL_INT	0x80FF07FF
#define GSR2_ALL_INT	0x001A00FF

#define GSR1_ALL_ERR	0x00FF078C
#define GSR2_ALL_ERR	0x001800FF


/* PCI related erros. */
#define SERREN 0x0100
#define PEREN  0x0040

#define DPE  0x8000
#define SSE  0x4000
#define RMA  0x2000
#define RTA  0x1000
#define DPR  0x0100

#define IBERIN 0x02000000
#define IBERSE 0x01000000
#define IBERCH 0x00800000
#define AERIN  0x00200000
#define DTIMEN 0x00100000
#define PERIN  0x00080000
#define RTYEN  0x00040000
#define MAIN   0x00020000
#define TAIN   0x00010000
#define AERSE	0x00002000
#define DTIMSE	 0x00001000
#define PERSE	 0x00000800
#define RTYSE	 0x00000400
#define MASE	 0x00000200
#define TASE	 0x00000100
#define DTIMCH	 0x00000010
#define PERCH	 0x00000008
#define RTYCH	 0x00000004
#define MACH	 0x00000002
#define TACH	 0x00000001

#define PCIIF_IERR   0x0001
#define CPUIF_PCIERR 0x0008
#define PCIIF_SERR   0x0100

#define MPCIIF_IERR  0x0001
#define MCPUIF_PCIERR 0x0008
#define MPCIIF_SERR   0x0100




#define MAXP        0x00000040


/*---------------------------------------------------------------------------*/
/* U_EP0CR - USB EndPoint 0 Controll Register (20H)                          */
/*---------------------------------------------------------------------------*/
#define EP0EN		0x80000000	/* EP0 Enable                                */
#define ISS			0x00100000	/* IN Send Stall                             */
#define INAK		0x00080000	/* IN NAK                                    */
#define OSS			0x00040000	/* OUT Send Stall                            */
#define NHSK0		0x00020000	/* No Handshake is performed                 */
#define ONAK		0x00010000	/* OUT NAK                                   */
#define MAXP0		0x00000040
/*#define MAXP0		0x00000008*/	/* MAX Packet size                       */

/*---------------------------------------------------------------------------*/
/* U_EP1CR - USB EndPoint 1 Controll Register (24H)                          */
/*---------------------------------------------------------------------------*/
#define EP1EN		0x80000000	/* EP1 Enable                                */
#define TM1			0x00080000	/* Tx Mode (NZLP)                            */
#define MAXP1		0x00000040	/* MAX Packet size                            */
/*#define MAXP1		0x000003FF*/

/*---------------------------------------------------------------------------*/
/* U_EP2CR - USB EndPoint 2 Controll Register (28H)                          */
/*---------------------------------------------------------------------------*/
#define EP2EN		0x80000000	/* EP2 Enable                                */
#define RM2_SEP		0x00180000	/* Rx Mode - Separate Mode (11)              */
#define RM2_ASM		0x00100000	/* Rx Mode - Assemble Mode (10)              */
#define RM2_NOR		0x00080000	/* Rx Mode - Normal Mode   (01)              */
#define MAXP2		0x00000040	/* MAX Packet size                           */
/*#define MAXP2		0x000003FF*/

/*---------------------------------------------------------------------------*/
/* U_EP3CR - USB EndPoint 3 Controll Register (2cH)                          */
/*---------------------------------------------------------------------------*/
#define EP3EN		0x80000000	/* EP3 Enable                                */
#define TM3			0x00080000	/* Tx Mode (NZLP)                            */
#define SS3			0x00040000	/* Send Stall                                */
#define NAK3		0x00010000	/* NAK packet is sent                        */
#define MAXP3		0x00000040	/* MAX Packet size                           */

/*---------------------------------------------------------------------------*/
/* U_EP4CR - USB EndPoint 4 Controll Register (30H)                          */
/*---------------------------------------------------------------------------*/
#define EP4EN		0x80000000	/* EP4 Enable                                */
#define RM4_SEP		0x00180000	/* Rx Mode - Separate Mode (11)              */
#define RM4_ASM		0x00100000	/* Rx Mode - Assemble Mode (10)              */
#define RM4_NOR		0x00080000	/* Rx Mode - Normal Mode   (01)              */
#define SS4			0x00040000	/* Send Stall                                */
#define NHSK4		0x00020000	/* No Handshake is performed                 */
#define NAK4		0x00010000	/* NAK Handshake is performed                */
#define MAXP4		0x00000040	/* MAX Packet size                           */

/*---------------------------------------------------------------------------*/
/* U_EP5CR - USB EndPoint 5 Controll Register (34H)                          */
/*---------------------------------------------------------------------------*/
#define EP5EN		0x80000000	/* EP5 Enable                                */
#define FM			0x00080000	/* Feedback Mode                             */
#define SS5			0x00040000	/* Send Stall                                */
#define NAK5		0x00010000	/* NAK Handshake is performed                */
#define MAXP5		0x00000040	/* MAX Packet size                           */

/*---------------------------------------------------------------------------*/
/* U_EP6CR - USB EndPoint 6 Controll Register (38H)                          */
/*---------------------------------------------------------------------------*/
#define EP6EN		0x80000000	/* EP6 Enable                                */
#define SS6			0x00040000	/* Send Stall                                */
#define NHSK6		0x00020000	/* No Handshake is performed                 */
#define NAK6		0x00010000	/* NAK Handshake is performed                */
#define MAXP6		0x00000040	/* MAX Packet size                           */

/*--------------------------------------------------------------------------*/
/* U_CMR - USB Command Register (40H)										*/
/*--------------------------------------------------------------------------*/
#define CMR_BUSY		0x80000000	/* Busy									*/
#define CMD_SEND_EP0	0x00000000	/* Datasending at EndPoint0 (000)		*/
#define CMD_SEND_EP1	0x01000000	/* Datasending at EndPoint1 (001)		*/
#define CMD_SEND_EP3	0x02000000	/* Datasending at EndPoint3 (010)		*/
#define CMD_SEND_EP5	0x03000000	/* Datasending at EndPoint5 (011)		*/
#define CMD_ADD_POOL0	0x04000000	/* Add BufferDir to Pool0 (100)			*/
#define CMD_ADD_POOL1	0x05000000	/* Add BufferDir to Pool1 (101)			*/
#define CMD_ADD_POOL2	0x06000000	/* Add BufferDir to Pool2 (110)			*/

/*----------------------------------------------------------------------*/
/* U_TEPSR - USB Tx EndPoint Status Register (48H)													*/
/*----------------------------------------------------------------------*/
#define	TX_FULL_BIT		0x00000002

/************************************************
** SNOWMAN Transmit/Receive Descriptor
************************************************/
#define LASTBIT			0x80000000	/* Last Descriptor							*/
#define DLBIT			0x40000000	/* Data Buffer / Link Pointer				*/
#define POOLNUM_0		0x00010000
#define POOLNUM_1		0x00020000
#define POOLNUM_2		0x00040000

typedef struct 
{
	unsigned long		Word[2];
} Buffer_Desc;

/************************************************
**	Tx Indication
************************************************/
#define	IBUS_ERROR			0x00000400
#define	EP1_BUFF_UNDERRUN	0x00000200
#define NZLP_MODE			0x00000100
#define EPN0_IN_TXINDI		0x00000000
#define EPN1_IN_TXINDI		0x00000002
#define EPN3_IN_TXINDI		0x00000004
#define EPN5_IN_TXINDI		0x00000006
#define TX_EPN				0x00000007
#define TX_VALID_ERRORS     0x00000400

typedef struct 
{
	unsigned long		Word[1];
} Tx_Indication;

/************************************************
** Rx Indication
************************************************/
#define EPN0_IN_RXINDI			0x20000000
#define EPN2_IN_RXINDI			0x60000000
#define EPN4_IN_RXINDI			0xA0000000
#define EPN6_IN_RXINDI			0xE0000000
#define	DATA_CORRUPTION_EP2		0x02000000
#define	IBUS_ERROR_IN_RXINDI	0x01000000
#define	USB_SETUP_PACKET_IND	0x00800000
#define BUFFER_OVERRUN_EP2		0x00400000
#define CRC_ERROR_EP2			0x00100000
#define BIT_STUFFING_ERROR_EP2	0x00080000
#define OVER65535				0x00040000
#define RX_NORMAL_MODE1			0x00000000
#define RX_NORMAL_MODE2			0x00010000
#define RX_ASSEMBLE_MODE		0x00020000
#define RX_SEPARATE_MODE		0x00030000

#define EPN_MASK				0xE0000000

typedef struct 
{
	unsigned long		Word[2];
} Rx_Indication;

/************************************************
** Device Request
************************************************/
/* bmRequestType value */
#define DEVICE				0x00
#define INTERFACE			0x01
#define ENDPOINT			0x02

/* wValue value */
#define DEVICE_DESC	       		0x0100
#define CONFIGURATION_DESC		0x0200
#define STRING_DESC		       	0x0300
#define INTERFACE_DESC			0x0400
#define ENDPOINT_DESC			0x0500
#define STRING_DESC_INDEX05     	0x0305
#define STRING_DESC_INDEX02     	0x0302
#define REPORT_DESC			0x2200

#define GET_STATUS				0x00
#define CLEAR_FEATURE		0x01
#define SET_FEATURE			0x03
#define SET_ADDRESS			0x05
#define GET_DESCRIPTOR		0x06
#define SET_DESCRIPTOR		0x07
#define GET_CONFIGURATION	0x08
#define SET_CONFIGURATION	0x09
#define GET_INTERFACE		0x0a
#define SET_INTERFACE		0x0b
#define SYNCH_FRAME			0x0c

#define DATA_SEND			0x80

#define REMOTE_WAKEUP		0x02
#define SELF_POWERED		0x01
#define ENDPOINT_STALL		0x01
#define EP_STALL_BIT		0x00040000	/* control register STALL Bit		*/
#define EP_STALL_BIT_RX0	0x00100000	/* control register STALL Bit(EP0Rx)*/
#define REQUEST_SIZE		0x00000008	/* Device Request Size				*/

#define DEVICE_TYPE				0x01
#define CONFIGURATION_TYPE		0x02
#define STRING_TYPE				0x03
#define INTERFACE_TYPE			0x04
#define ENDPOINT_TYPE			0x05


/* USB INTERFACE LAYER DEFINEMENTS */
EXTERN void InitUsbDriverStack(void);
EXTERN void UsbIrqHandler(ULONG,ULONG);
EXTERN unsigned long PCItoCPU( unsigned long addr);
EXTERN unsigned long CPUtoPCI( unsigned long addr);
#endif
