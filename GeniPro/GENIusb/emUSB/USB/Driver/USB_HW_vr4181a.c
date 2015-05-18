/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIpro                                         */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      :    USB_HW_vr4181a.c                                   */
/*                                                                          */
/* FILE NAME        :    USB_HW_vr4181a.c                                   */
/*                                                                          */
/* FILE DESCRIPTION :    Low level driver for USB controller of VR4181a     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "Global.h"         /* Type definitions: U8, U16, U32, I8, I16, I32 */
#include "USB_Private.h"    /* Access to USB stacks definements.            */
#include "USB_HW_Driver.h"  /* Access to USB Low level driver variables.    */
#include "USB.h"			/* Access to USB stacks definements.            */
#include "USB_CDC.h"        /* Access to USB CDC component definements.     */
#include "RTOS.h"			/* Access to RTOS Services.     				*/

/****************************************************************************/
/*                                                                          */
/* D E F I N E M N T S                                                      */
/*                                                                          */
/****************************************************************************/
#define FOR_EXTRA_32BYTE_8	8
#define FOR_EXTRA_32BYTE_4	4
#define	TX_BDESC_SIZE		8
#define	TX_BUFFER_SIZE		128
#define POOL0				0
#define POOL2				2

/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
void SetupUsbMailBox(void);
void AddRxDescriptors_release(Buffer_Desc*);
void _Init(void);
void InitUsbDriverStack(void);
void ReinitUsbDriver(void);

/****************************************************************************/
/*                                                                          */
/* S T A T I C   V A R I A B L E S                                          */
/*                                                                          */
/****************************************************************************/
static volatile U16 u16_regval;
volatile U32 int regval2;
volatile U32 gsr1;
volatile U32 gsr2;
volatile U16 u16_pcierror;
volatile U32 u32pcierror;
volatile U32 u32pcierrorAdd;
volatile U32 gmr;
volatile Tx_Indication *TxRead_p;
volatile Tx_Indication *TxWrite_p;
volatile Rx_Indication *RxRead_p;
volatile Rx_Indication *RxWrite_p;

static U32 Usb_tx_Epn_busy_err_cnt = 0;
static U32 Usb_tx_cmd_busy_err_cnt = 0;
static U32 Usb_Rx_cmd_busy_err_cnt = 0;	
U32 Usb_Rx_Buff_Ovrfl_err_cnt = 0;	


/* 0:Rx not called, 1:Block on RX, 2:Block and signaled on Rx, 3: Non block and Signaled */
volatile U8 u8UsbRxTaskState = 0;  
volatile U8 usbError = 0;

/* Pointer to last linkpointer in the receive pool. */
Buffer_Desc * sn_BottomOfList[3];

/* Flags to indicates transfer mode */
int sn_TxModeFlag = (INT_RXMODE | ISO_RXMODE | BULK_RXMODE);
int sn_RxModeFlag =  (INT_RXMODE | ISO_RXMODE | BULK_RXMODE);

/* Buffer information. */
U32 RXBufDescTop[3];
U32 RXBufDescBtm[3];

/* Tx 0-Length Data pointers */
Buffer_Desc	*sn_TxBufferDesc_Length0;
U8	*sn_TxDataBuffer_Length0;

/* Externs */
#ifdef TCS_USB_SER_PORT
extern UCHAR usb_tx_idle_cnt;
extern const UCHAR usb_tx_idle_cnt_reset_value;
extern ULONG usb_tx_idle_active;
#endif

/* RX pool memory */
Buffer_Desc RxPool_0[((SN_DESCNUM_IN_DIR +1)*20) + 4];
Buffer_Desc RxPool_2[((SN_DESCNUM_IN_DIR +1)*20) + 4];

#pragma ghs section bss=".frambuf"

/* Secure the area for Mailbox */
Tx_Indication	sn_ForTxMailBox[ SN_TX_MLBNUM + FOR_EXTRA_32BYTE_8 ];
Tx_Indication	*sn_TxMailBox;
Rx_Indication	sn_ForRxMailBox[ SN_RX_MLBNUM + FOR_EXTRA_32BYTE_4 ];
Rx_Indication	*sn_RxMailBox;

/* For Tx Data */
Buffer_Desc	sn_ForTxBufferDesc[ TX_BDESC_SIZE ];
Buffer_Desc	*sn_TxBufferDesc;
U8	sn_ForTxDataBuffer[ TX_BUFFER_SIZE ];
U8  *sn_TxDataBuffer;

/* General purpose transmit buffer-64 bytes */
U8 CDC_Tx_Buf[255 + 32];
U8 *cdc_tx_buff_p;

/* Transmit buffer for enumeration phase. */
U8 _aOrigDescBuffer[256 + 32];
U8 *p_aOrgDescBuffer;

/* RX buffers */
U8 RxPool_0_Buff[(SN_DESCNUM_IN_DIR * 20)*SN_RX_BUFSIZE + 32];
U8 RxPool_2_Buff[(SN_DESCNUM_IN_DIR * 20)*SN_RX_BUFSIZE + 32];

#pragma ghs section data= default

/****************************************************************************
*
* S T A T I C   C O D E   
*
*****************************************************************************
*/

/****************************************************************************
*   Name        : PCItoCPU                                                  *
*																			*
*   Inputs      : 32 bit Absolute PCI Address                               *
*   Outputs     : None                                                      *
*   Returns     : 32 bit Absolute CPU Address                               *
*                                                                           *
*   Description : Performs Address Convesrion of PCI space to CPU space     * 
*                 based on PCI mapping onto CPU Bus                         *
*****************************************************************************
*/
U32 PCItoCPU(U32 addr)
{
  return (U32)(addr | 0xA0000000);
}

/****************************************************************************
*   Name        : CPUtoPCI                                                  *
*																			*
*   Inputs      : 32 bit Absolute CPU Address                               *
*   Outputs     : None                                                      *
*   Returns     : 32 bit Absolute PCI Address                               *
*                                                                           *
*   Description : Performs Address Conversion of CPU space to PCI space     * 
*                 based on PCI mapping onto CPU bus                         *
*****************************************************************************
*/
U32 CPUtoPCI(U32 addr)
{
  return (U32)(addr & 0x0FFFFFFF);
}

/****************************************************************************
*   Name        : AddRxDescriptors_atInit                                   *
*																			*
*   Inputs      : Pointer to the buffer descriptor to be added to the Pool  *
*   Outputs     : None                                                      *
*   Returns     : None														*
*                                                                           *
*   Description : Calcutaes the number of directories to be added from the  *
*                 start of directory provided as a input and adds the       *
*                 directories into the pool. Directories belongs to EP0     *
*                 added to Pool0 and EP3 are added to Pool2.                *
*                 This function is used only at the time of initialisation	*
*																			*
*****************************************************************************
*/
void AddRxDescriptors_atInit(Buffer_Desc *p_desc_top)
{
  Buffer_Desc *p_desc;
  U32 Command;
  U32 DirNum;

  p_desc = p_desc_top;

  /************************************************************************/
  /* Finding PoolNumber. And deciding the command to be issued.	 		*/
  /************************************************************************/
  if (p_desc->Word[0] & POOLNUM_0) 
  {
    Command = CMD_ADD_POOL0;
    DirNum = SN_DIRNUM_POOL0;
  } 
  else if (p_desc->Word[0] & POOLNUM_1) 
  {
    Command = CMD_ADD_POOL1;
    DirNum = SN_DIRNUM_POOL1;
  }
  else if (p_desc->Word[0] & POOLNUM_2) 
  {
    Command = CMD_ADD_POOL2;
    DirNum = SN_DIRNUM_POOL2;
  }

  /* Baning the interrupts from here.										*/
  OS_IncDI()

    /*************************************************************************/
    /* Issueing the command.                     							 */
    /*************************************************************************/
    while (1)
    {
      if ((GENI_USBF_CMD & CMR_BUSY) == 0) 
      {
        break;
      }
    }
    GENI_USBF_CMD_ADDRESS = CPUtoPCI((U32)p_desc_top) ;
    GENI_USBF_CMD = (Command  |  DirNum);

    /* Baning the interrupt to here.										*/
    OS_DecRI()
      return;
}

/****************************************************************************
*   Name        : AddRxDescriptors_release                                  *
*																			*
*   Inputs      : Pointer to the buffer descriptor to be added to the Pool  *
*   Outputs     : None                                                      *
*   Returns     : None														*
*                                                                           *
*   Description : Calcutaes the number of directories to be added from the  *
*                 start of directory provided as a input and adds the       *
*                 directories into the pool. Directories belongs to EP0     *
*                 added to Pool0 and EP3 are added to Pool2.                *
*                 This function is used only at the time of initialisation	*
*																			*
*****************************************************************************
*/
void AddRxDescriptors_release(Buffer_Desc * p_desc_top)
{
  Buffer_Desc *p_desc;
  U32 Command;
  U32 timeout_delay;

  p_desc = p_desc_top;

  /*----------------------------------------------------------------------*/
  /* Finding PoolNumber. And deciding the command to be issued.	 		*/
  /*----------------------------------------------------------------------*/
  if (p_desc->Word[0] & POOLNUM_0) 
  {
    Command = CMD_ADD_POOL0;
  } 
  else if (p_desc->Word[0] & POOLNUM_1) 
  {
    Command = CMD_ADD_POOL1;
  }
  else if (p_desc->Word[0] & POOLNUM_2) 
  {
    Command = CMD_ADD_POOL2;
  }
  p_desc->Word[0] &= (~LASTBIT);
  p_desc->Word[0] &= 0xffff0000;
  p_desc->Word[0] |= 0x00000040;

  /* Baning the interrupt from here.										*/
  OS_IncDI()

  /*----------------------------------------------------------------------*/
  /* Issueing the command.												*/
  /*----------------------------------------------------------------------*/
  /* Wait till the USB is busy in processing previous command. */

  timeout_delay = USB_ERR_TIMEOUT;	

  while (--timeout_delay) 
  {
    if ((GENI_USBF_CMD  & CMR_BUSY) == 0) 
    {
      break;
    }
  }

  if (timeout_delay)
  {	
    GENI_USBF_CMD_ADDRESS = CPUtoPCI((U32)p_desc_top) ;
    GENI_USBF_CMD = (Command  |  1);
  }
  else
  {
    Usb_Rx_cmd_busy_err_cnt++;
  }

  /* Baning the interrupt to here.										*/
  OS_DecRI()
    return;
}

/****************************************************************************
*   Name        : ConfigRxBuffer                                            *
*																			*
*   Inputs      : Pointer to the buffer descriptor to be added to the Pool  *
*   Outputs     : None                                                      *
*   Returns     : Pointer to beggining of BufferDirectory   				*
*                                                                           *
*   Description : This function constuct the complte Receive buffer and     *
*                 descriptor link list in the RAM. It intialises it to zero *
*                 and all available state.                                  *
*                 This function is used only at the time of initialisation	*
*																			*
*****************************************************************************
*/
Buffer_Desc* ConfigRxBuffer(U32 PoolNum)
{
  Buffer_Desc *p_RxDesc;
  Buffer_Desc *p_top;
  U32	lcount1;
  U32 lcount2;
  U32 DirNumInPool;
  U32 PoolNumOfDesc;
  U32 tmp;	
  U8 *p_RxBuff;

  switch (PoolNum)
  {
  case 0:
    DirNumInPool = SN_DIRNUM_POOL0;
    PoolNumOfDesc = POOLNUM_0;
    break;
  case 1:
    DirNumInPool = SN_DIRNUM_POOL1;
    PoolNumOfDesc = POOLNUM_1;
    break;
  case 2:
    DirNumInPool = SN_DIRNUM_POOL2;
    PoolNumOfDesc = POOLNUM_2;
    break;
  default:
    break;
  }

  if (PoolNum == 0)
  {
    tmp  = (U32)&RxPool_0[0];
  }
  else if (PoolNum == 2)
  {
    tmp  = (U32)&RxPool_2[0];
  }

  tmp = (tmp + 0x10) & 0xfffffff0;
  RXBufDescTop[PoolNum] = tmp;
  RXBufDescBtm[PoolNum] = tmp + (((SN_DESCNUM_IN_DIR + 1) * DirNumInPool) * sizeof(Buffer_Desc));
  p_RxDesc = (Buffer_Desc *)tmp;

  p_top = p_RxDesc;

  if (PoolNum == 0)
  {
    tmp  = (U32)&RxPool_0_Buff[0];
  }
  else if (PoolNum == 2)
  {
    tmp  = (U32)&RxPool_2_Buff[0];
  }

  tmp = (tmp + 0x10) & 0xfffffff0;
  p_RxBuff = (U8 *)tmp;


  /* Setting the BufferDescriptor by ones*/
  for (lcount2=0; lcount2 < DirNumInPool ; lcount2++)
  {
    for (lcount1=0; lcount1 < SN_DESCNUM_IN_DIR ; lcount1++)
    {
      p_RxDesc->Word[0] = (DLBIT | PoolNumOfDesc | SN_RX_BUFSIZE);
      p_RxDesc->Word[1] = CPUtoPCI((U32)p_RxBuff);
      p_RxBuff += SN_RX_BUFSIZE;
      p_RxDesc++;
    }

    p_RxDesc->Word[0] = PoolNumOfDesc;
    p_RxDesc->Word[1] = CPUtoPCI((U32)(p_RxDesc + 1));

    p_RxDesc++;
  }

  /* Back one. And let linkpointer to point Top of pool. */
  p_RxDesc--;
  p_RxDesc->Word[1] = CPUtoPCI((U32)p_top);

  return p_top;
}

/****************************************************************************
*   Name        : UpdateRxMailBox                                           *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routinr is called from ISR on complete USB packet/s  *
*                 is received. It Copies the data from data buffers linked  *
*                 to buffer decriptors. It releases buffer descriptors and  *
*                 also updates the receive mail box read pointer.           *
*																			*
*****************************************************************************
*/
void UpdateRxMailBox(void)
{
  static volatile U32 prev_word1_addr = 0x00000000;
  Rx_Indication * p_MailBox;
  Buffer_Desc * p_desc;
  USB_SETUP_PACKET pSetupPacket;
  volatile U32 word0_indication;
  volatile U32 word1_address;
  volatile U32 Pool2Dir;
  U32 pnum;
  U16 NumBytes;
  U8 EPIndex;    
  U8 *P_RxDataBuffer;
  U8 *RxDataBuffer; 
  U8 IsSZLPReceived = 0x00;

  /* Getting the latest RxIndication */
  p_MailBox = (Rx_Indication *)PCItoCPU(GENI_USBF_RxMail_RD_ADDR);

  /* Storing the Indication data into variable for calculation. */
  word0_indication = p_MailBox->Word[0];
  word1_address	 = PCItoCPU(p_MailBox->Word[1]);

  if (word1_address == prev_word1_addr)
  {
    IsSZLPReceived = 0x01;
  }

  prev_word1_addr = word1_address;

  if (IsSZLPReceived == 0x00)
  {
    p_desc = (Buffer_Desc *)word1_address;

    RxDataBuffer = (U8 *)PCItoCPU(p_desc->Word[1]);

    P_RxDataBuffer = RxDataBuffer;
    NumBytes = (U16) p_desc->Word[0] & 0xFFFF;

    pnum = (word0_indication & 0xe0000000);

    switch (pnum) 
    {
    case EPN0_IN_RXINDI:
      p_desc->Word[0] = (p_desc->Word[0] | (POOLNUM_0));
      EPIndex = 0;
      break;
    case EPN6_IN_RXINDI:
      p_desc->Word[0] = (p_desc->Word[0] | (POOLNUM_0));
      break;
    case EPN2_IN_RXINDI:
      p_desc->Word[0] = (p_desc->Word[0] | (POOLNUM_1));
      break;
    case EPN4_IN_RXINDI:
      p_desc->Word[0] = (p_desc->Word[0] | (POOLNUM_2));
      EPIndex = 1;
      break;
    default:
      break;
    }

    /* If It's not IBUS error then Process the packet. */
    if ((word0_indication & IBUS_ERROR_IN_RXINDI) == 0x00000000)
    {
      /* If it's reception at EP0, go devicerequest operation */
      if ((word0_indication & EPN_MASK) == EPN0_IN_RXINDI) 
      {
        if (p_MailBox->Word[0] &  USB_SETUP_PACKET_IND)
        {
          pSetupPacket.bmRequestType = RxDataBuffer[0];
          pSetupPacket.bRequest = *(U8 *)(RxDataBuffer + 1);
          pSetupPacket.wValueLow =	*(U8 *)(& RxDataBuffer[2]);
          pSetupPacket.wValueHigh =	*(U8 *)(& RxDataBuffer[3]); 	   
          pSetupPacket.wIndexLow =	*(U8 *)(& RxDataBuffer[4]);
          pSetupPacket.wIndexHigh =	*(U8 *)(& RxDataBuffer[5]); 	   
          pSetupPacket.wLengthLow = *(U8 *)(& RxDataBuffer[6]);
          pSetupPacket.wLengthHigh = *(U8 *)(& RxDataBuffer[7]);
          USB__HandleSetup(&pSetupPacket);
        }
        else
        {
          USB__OnRx(EPIndex, P_RxDataBuffer, NumBytes); 
        }

        if (word0_indication & 0x0000ffff) 
        {
          AddRxDescriptors_release((Buffer_Desc *)word1_address);
        }
      }
      /* Received data for non EP0. */
      else 
      {
        USB__OnRx(EPIndex, P_RxDataBuffer, NumBytes); 
        AddRxDescriptors_release((Buffer_Desc *)word1_address);
      }
    }
  }
  else
  {
    IsSZLPReceived = 0;
  }

  /* Wrighting the address of next Mailbox into RMRA Register */
  if (p_MailBox+1 == &sn_RxMailBox[ SN_RX_MLBNUM ]) 
  {
    GENI_USBF_RxMail_RD_ADDR  = CPUtoPCI((U32)&sn_RxMailBox[0]);
  } 
  else 
  {
    GENI_USBF_RxMail_RD_ADDR  = CPUtoPCI((U32) (p_MailBox+1));
  }
}


/****************************************************************************
*   Name        : SetupUsbMailBox                                           *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routinr sets up TX and RX mailboxes and buffer       *
*                 descriptors.												*
*				  It is called only at the time of initialization			*
*																			*
*****************************************************************************
*/
void SetupUsbMailBox(void)
{
  Buffer_Desc * AddressOfTopDir;
  U32 lCount3;
  U32 MailboxAddress;

  /****************************************************************************
  Transfer of Data between USB hardware buffers (RX and TF FIFO) and 
  Descriptor pointeed buffers in Memory (RAM) is done by DMA. DMA supports 
  only uncacheable memory access that is too alligned with 16 bytes bloacks. 
  So All the buffer descriptors and memory buffers need to be 16 bytes 
  aligned.
  ****************************************************************************/
  /* Aligning the pointer of the Mailboxes array to 16 byte. */
  MailboxAddress = (U32) &sn_ForTxMailBox[0];
  MailboxAddress = (MailboxAddress + 0x10) & 0xfffffff0;
  sn_TxMailBox = (Tx_Indication *)MailboxAddress;

  MailboxAddress = (U32) &sn_ForRxMailBox[0];
  MailboxAddress = (MailboxAddress + 0x10) & 0xfffffff0;
  sn_RxMailBox = (Rx_Indication *)MailboxAddress;

  /* Initializing Mailboxes to all zero. */
  for (lCount3=0; lCount3<SN_TX_MLBNUM; lCount3++) 
  {
    sn_TxMailBox[lCount3].Word[0] = 0x0;
  }
  for (lCount3=0; lCount3<SN_RX_MLBNUM; lCount3++) 
  {
    sn_RxMailBox[lCount3].Word[0] = 0x0;
    sn_RxMailBox[lCount3].Word[1] = 0x0;
  }

  /* Registering the Mailbox Addresses to SFRs.(Ref: 13.2.2) */
  GENI_USBF_TxMail_START_ADDR = CPUtoPCI((U32)&sn_TxMailBox[0]);
  GENI_USBF_TxMail_BTM_ADDR   = CPUtoPCI((U32)&sn_TxMailBox[SN_TX_MLBNUM]);
  GENI_USBF_RxMail_START_ADDR = CPUtoPCI((U32)&sn_RxMailBox[0]);
  GENI_USBF_RxMail_BTM_ADDR   = CPUtoPCI((U32)&sn_RxMailBox[SN_RX_MLBNUM]);


  /****************************************************************************
  Preparing the structure of receive buffer and descriptors.(Ref 13.2.4) 	  
  ****************************************************************************/
  /* Structure for Pool0 of memory */
  AddressOfTopDir = ConfigRxBuffer(POOL0); 
  AddRxDescriptors_atInit(AddressOfTopDir);

  /* Structure for Pool2 of memory */
  if (sn_RxModeFlag & BULK_RXMODE)
  {
    AddressOfTopDir = ConfigRxBuffer(POOL2);
    AddRxDescriptors_atInit(AddressOfTopDir);
  }


  /****************************************************************************
  Preparing the structure of Transmit buffer and descriptors.(Ref 13.2.3)
  In case of Transmission the 16 bytes aligned buffer in a uncaheable memory 
  is linked and used at the run time to transmit the data.	  
  ****************************************************************************/
  /* Initiarize Descriptor for Transmission. */
  MailboxAddress = (U32) &sn_ForTxBufferDesc[0];
  MailboxAddress = (MailboxAddress + 0x10) & 0xfffffff0;
  sn_TxBufferDesc = (Buffer_Desc *)MailboxAddress;

  sn_TxBufferDesc[0].Word[0] = 0x0;
  MailboxAddress = (U32) &sn_ForTxDataBuffer[0];
  MailboxAddress = (MailboxAddress + 0x10) & 0xfffffff0;
  sn_TxDataBuffer = (U8 *)MailboxAddress;

  /* Initialize the Descriptor for Tx (0-Length Packet) */
  sn_TxBufferDesc_Length0 =  &sn_TxBufferDesc[2];
  sn_TxDataBuffer_Length0 =  &sn_TxDataBuffer[2];

  /* Prepare Transmit Buffer Directory */
  sn_TxBufferDesc_Length0[0].Word[0] = (U32)(LASTBIT | DLBIT );
  sn_TxBufferDesc_Length0[0].Word[1] = CPUtoPCI((U32)sn_TxDataBuffer_Length0);

  sn_TxBufferDesc_Length0[1].Word[0] = 0;
  sn_TxBufferDesc_Length0[1].Word[1] = 0;

  /* Prepare Payload Data to SDRAM for zero length packet. */
  sn_TxDataBuffer_Length0[0] = 0x0;


  /****************************************************************************
  Initializing Task level Transmit buffer for DMA operations. 
  ****************************************************************************/
  MailboxAddress = (U32)&CDC_Tx_Buf[0];
  MailboxAddress = (MailboxAddress + 0x10) & 0xfffffff0;
  cdc_tx_buff_p = (U8 *)MailboxAddress;

  /****************************************************************************
  Initializing USB stack receive buffer. It is used in the Control transfers 
  operations on EP0 with using DMA. 
  ****************************************************************************/
  MailboxAddress = (U32) &_aOrigDescBuffer[0];
  MailboxAddress = (MailboxAddress + 0x10) & 0xfffffff0;
  p_aOrgDescBuffer = (U8 *)MailboxAddress;
}

/****************************************************************************
*   Name        : SetPciMapForUsb                                           *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine first accesses the USB function space on the *
*                 PCI map in the configuration read/write mode. So it       *
*                 accesses the configuration registers of the USBFU to      *
*                 configures USB Function's BaseAddress. So hereafter all   *
*                 the USB operational registers can be accessed  by using   *
*                 the USBFU_BAR_VALUE base offset.                          *
*				  It is called only at the time of initialization.			*
*																			*
*****************************************************************************
*/
void SetPciMapForUsb(U32 Addr0)
{
  U32 PciRegVal;

  /**************************************************************************
  Accessing the configuration space of the PCI mapped USBFU device.(6.14.3)
  **************************************************************************/
  PciRegVal = (0x80000000 >> 1) | 0x0000001a ;
  GENI_IOPCIU_PCIINIT01 = PciRegVal;

  /* Base Address Register Setting (Configration Reg Access) */
  (*(volatile U32 *)(Addr0 + 0x10)) = USBFU_BAR_VALUE ;

  /* Command Register Setting (Configration Reg Access) */
  (*(volatile U16 *)(Addr0 + 0x04)) = 0x0046 ;

  /* PCI Base offset settings for accessing USB opeartional space. */
  GENI_IOPCIU_PCIINIT01 = (0xA8000000 | 0x00000016);
}

/****************************************************************************
*   Name        : PCI_DevOperAccess                                         *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine first accesses the USB function space on the *
*                 PCI map in the configuration read/write mode. So it       *
*                 accesses the configuration registers of the USBFU to      *
*                 configures USB Function's BaseAddress. So hereafter all   *
*                 the USB operational registers can be accessed  by using   *
*                 the USBFU_BAR_VALUE base offset.                          *
*				  It is called only at the time of initialization.			*
*																			*
*****************************************************************************
*/
void PCI_DevOperAccess(void)
{
  U32 int	pciw0_addr;

  /* PCIWindow0 Base Address.(Extracting bits 21:31- 0xA8000000) */

  pciw0_addr = (GENI_PCIW0 | KSEG1_BASE) & 0xffe00000 ; 

  SetPciMapForUsb(pciw0_addr);
}

/****************************************************************************
*   Name        : InitIOPCIU                                                *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine enables USB module and 48 Mhz clock for USB  *
*                 It resets the PCI module and initialises all the BAR      *
*                 registers. It initialises the PCI map and configures the  *
*                 USB device for opearational space access.                 *
*				  It is called only at the time of initialization.			*
*																			*
*****************************************************************************
*/
void InitIOPCIU(void)
{
  U32 dela = 0x10000;

  /* Set CMODE(1:0)bit to 0x01 to enable USB 48MHz Clock */
  GENI_PINMODED2 &= 0x3FFF;
  GENI_PINMODED2 |= 0x4000;

  /* USB 48MHz Clock Input Enable for USBF unit. */
  GENI_CMUCLKMSK1 |= 0x2000;
  GENI_CMUCLKMSK3 |= 0x00000012;

  /* Disable USB Vbus current by device. */
  GENI_USBSIGCTRL = 0x0000;

  /* Detach USB */
  GENI_GPDATA0 |= 0x0400;

  /* Forcing both Cold and Warm Resets */
  regval2 = (GENI_IOPCIU_PCICTRL_H); 
  regval2 = regval2 | 0xC0000000;  
  GENI_IOPCIU_PCICTRL_H = regval2;

  while (dela > 0)
    dela--;

  /* Clearing the Warm Reset sticky bit */
  regval2 = GENI_IOPCIU_PCICTRL_H;
  regval2 = regval2 & 0xBFFFFFFF;  
  GENI_IOPCIU_PCICTRL_H = regval2; 
  regval2 = GENI_IOPCIU_PCICTRL_H;

  /* Settings for PCI to CPU mapping. */
  GENI_IOPCIU_BAR_SDRAM = 0x00000000;
  GENI_IOPCIU_BAR_INTCS = 0x0A000000;
  GENI_IOPCIU_BAR_ISAW = 0x10000000;
  GENI_IOPCIU_BAR_PCS1 = 0x1A000000;
  GENI_IOPCIU_BAR_PCS0 = 0x1C000000;
  GENI_IOPCIU_BAR_ROMCS = 0x1E000000;
  GENI_PCIW0 |= 0x080000ad;

  /* Configure USBFU operational register space. */
  PCI_DevOperAccess();

  /* Read error status registers to clear the POR values. */
  u16_regval = GENI_IOPCIU_PCICMD;
  u16_regval = GENI_IOPCIU_PCISTS;
  regval2 = GENI_IOPCIU_PCIERR;
  regval2 = GENI_IOPCIU_PCICTRL_L;
  regval2 = GENI_IOPCIU_PCICTRL_H;	
}

/****************************************************************************
*   Name        : InitUsbHardware                                           *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine configures and Initailises interrupts for    *
*                 Usb at the time of initialization.			            *
*																			*
*****************************************************************************
*/
void InitUsbHardware(void)
{
  /* Read GSTATs to clear the USB hardware status. */
  regval2 =  GENI_USBF_GSTAT1;
  regval2 =  GENI_USBF_GSTAT2;

  /* Enable and Assign USBF Interrupts to the core. */
  u16_regval = GENI_MSYSINT1REG;
  GENI_MSYSINT1REG |= 0x2000;
  u16_regval = GENI_MSYSINT1REG;

  u16_regval = GENI_INTASSIGN3REG;
  GENI_INTASSIGN3REG &= 0xF3FF;
  u16_regval = GENI_INTASSIGN3REG;

  /* Enabling All USB interrupts */
  GENI_USBF_INTMSK2 |= GSR2_ALL_INT;

  /* Enabling global and EP0 TX and RX interurpts */
  GENI_USBF_INTMSK1 |= GSR1_ALL_INT;

  /* Read GSTATs to zero the POR values. */
  regval2 =  GENI_USBF_GSTAT1;
  regval2 =  GENI_USBF_GSTAT2;
}

/****************************************************************************
*   Name        : InitUsbDriverStack                                             *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine Initialises USB stack, Initialises PCI and   *
*                 USBFU configures it, configures endpoints to be used for  *
*                 CDC and starts the USB stack. It also enables EP0,EP3,EP4 *
*                 & EP5 at the time of Initialisation.                      *
*																			*
*****************************************************************************
*/
void InitUsbDriverStack(void)
{ 
  USB_CDC_INIT_DATA InitData;
  static U8 out_buffer[256];

  USB_Init();

  InitData.EPOut = USB_AddEP(USB_DIR_OUT, USB_TRANSFER_TYPE_BULK,   0, out_buffer, 256);
  InitData.EPIn  = USB_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_BULK,   0, NULL, 0);
  InitData.EPInt = USB_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT,  255, NULL, 0);
  USB_CDC_Add(&InitData);

  USB_Start();

  /* Setting All the endpoints. */
  GENI_USBF_EP0_CONT = (EP0EN | MAXP0); // Enabling EP0
  GENI_USBF_EP3_CONT |= (EP3EN | MAXP3);   //Enabling Bulk IN
  GENI_USBF_EP4_CONT |= (RM4_NOR| MAXP4);  //Enabling Bulk OUT
  GENI_USBF_EP5_CONT |= (EP5EN  |  MAXP5); //Enabling Int IN
}

/****************************************************************************
*   Name        : ReconnectUsb                                              *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine Reinitialises PCI,USBFU hardware and         *
*                 Mailboxes, Buffer descriptors for USBFU                   *
*                 It is called from ISR when the USB disconnection is       *
*                 identified.                                               *
*																			*
*****************************************************************************
*/
void ReconnectUsb(void)
{
  /* Clears the PCI status and initialise the module. */
  InitIOPCIU();

  /* Initialise the USB hardware. */
  InitUsbHardware();

  /* Initialise USB Mailbox and buffer descriptors. */
  SetupUsbMailBox();


  /* USB Buffer (FU) Enable. After this register setting HOST can detect 
  the USBFU. Also Attach teh USB to the USB PHY. */
  GENI_USBSIGCTRL = 0x000C;
  GENI_GPMODE1 |= 0x0030;
  GENI_GPDATA0 &= 0xFBFF;
}

/****************************************************************************
*   Name        : RestartUsb                                              *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine restarts teh USB after disconenction         *
*                 It is called from ISR when the USB disconnection is       *
*                 identified.                                               *
*																			*
*****************************************************************************
*/
RestartUsb(void)
{
  ReconnectUsb();
}

/****************************************************************************
*   Name        : ReinitUsbDriver                                           *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine reinitailises the USB after disconnection    *
*                 it reconfigures the PCI and USB hardware and Mailboxes &  *
*                 Buffer desciptors and enables EP0,EP3,EP4 & EP5.          *
*                 It is called from ISR when the USB disconnection is       *
*                 identified.                                               *
*																			*
*****************************************************************************
*/
void ReinitUsbDriver(void)
{
  RestartUsb();

  /* Setting All the endpoints. */
  GENI_USBF_EP0_CONT = (EP0EN | MAXP0); // Enabling EP0
  GENI_USBF_EP3_CONT |= (EP3EN | MAXP3);   //Bulk IN
  GENI_USBF_EP4_CONT |= (RM4_NOR| MAXP4); //Bulk OUT
  GENI_USBF_EP5_CONT |= (EP5EN  |  MAXP5); //Int IN
}

/****************************************************************************
*   Name        : UpdateTxMailBox                                           *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine Upadtes the TX mailbox read pointer and      *
*                 handles the transmission erros if any & updates USB stack *
*                 variables on transmission complete.                       *
*                 It is called from ISR when the transmission on USB is done*
*																			*
*****************************************************************************
*/
void UpdateTxMailBox(void)
{
  Tx_Indication *TxMailBox;
  U8 EpNum;
  U8 errTx = 0;

  /* Read the TxMailBox Indication. */
  TxMailBox = (Tx_Indication *)PCItoCPU(GENI_USBF_TxMail_RD_ADDR);

  /*************************************************************************/
  /* Check for Transmission Errors. 										 */
  /*************************************************************************/
  if (TxMailBox->Word[0] & IBUS_ERROR) 
  {
    errTx = TRUE;
  }
  if (TxMailBox->Word[0] & EP1_BUFF_UNDERRUN) 
  {
    errTx = TRUE;
  } 

  if ((USB_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) == USB_STAT_CONFIGURED) 
  {
    // restart Tx idle countdown if no error.
    if (errTx == FALSE)
    {
#ifdef TCS_USB_SER_PORT
      usb_tx_idle_active = FALSE;
      usb_tx_idle_cnt = usb_tx_idle_cnt_reset_value;									 
      usb_tx_idle_active = TRUE;	  
#endif
    }
  }

  /* Update the Mail Box Read Address Pointer. */
  if ((TxMailBox + 1) >= ((Tx_Indication *)PCItoCPU(GENI_USBF_TxMail_BTM_ADDR))) 
  {
    GENI_USBF_TxMail_RD_ADDR = GENI_USBF_TxMail_START_ADDR;
  } 
  else 
  {
    GENI_USBF_TxMail_RD_ADDR = CPUtoPCI((U32)(TxMailBox + 1));
  }

  /* Handle the transmission complete event. */
  if (TxMailBox->Word[0] & TX_EPN)
  {
    EpNum = ((TxMailBox->Word[0] & TX_EPN) >> 1);
    USB__OnTx(EpNum);
  }
  else 
  {
    /* Clear Buffer Directory */
    sn_TxBufferDesc[0].Word[0] = 0x0;
  }
}

/****************************************************************************
*   Name        : UsbIrqHandler                                             *
*																			*
*   Inputs      : USB Global status register values                         *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine handles all enabled USB interrupts including *
*                 followings:                                               *
*                 1. Bus Reset Signal                                       *
*                 2. Bus Suspend Signal                                     *
*                 3. Bus Resume Signal                                      *
*                 4. USB disconnection                                      *
*                 5. USB error detection                                    *
*                 6. USB Data receive Interrupt                             *
*                 7. USB Data transmit complete Interrupt                   *
*                                                                           *
*                 It is called from USBIrqHandler() function.               *
*																			*
*****************************************************************************
*/
void UsbIrqHandler(ULONG ulGstatGsr1, ULONG ulGstatGsr2) 
{
  /* Check if Bus Reset occured */	
  if (ulGstatGsr1 & GSR2)
  {
    if (ulGstatGsr2 & URST)
    {
      USB__OnBusReset();
    }
  }

  /* Handle error interrupts here */
  if ((ulGstatGsr1 & GSR1_ALL_ERR)||((ulGstatGsr1 & GSR2)&&(ulGstatGsr2 & GSR2_ALL_ERR)))
  {
    gsr1 = ulGstatGsr1;
    gsr2 = ulGstatGsr2;

    /* If Disconnection has occured then reinit USB. */
    if (ulGstatGsr2 & SL)
    {
      ReinitUsbDriver();
    }
  }

  /* Get the value of Tx/Rx Mailbox Read/Wright Address Register.         */
  TxRead_p  = (volatile Tx_Indication *)PCItoCPU(GENI_USBF_TxMail_RD_ADDR);
  TxWrite_p = (volatile Tx_Indication *)PCItoCPU(GENI_USBF_TxMail_WR_ADDR);
  RxRead_p  = (volatile Rx_Indication *)PCItoCPU(GENI_USBF_RxMail_RD_ADDR);
  RxWrite_p = (volatile Rx_Indication *)PCItoCPU(GENI_USBF_RxMail_WR_ADDR);

  /************************************************************************/
  /* Going on while GSR1 is set or while readpointer and wrightpointer    */
  /* don't point same address.                                            */
  /************************************************************************/
  while ((ulGstatGsr1 & (TX_FINISH | RX_FINISH | GSR1_ERROR))
    || (TxRead_p != TxWrite_p)
    || (RxRead_p != RxWrite_p))
  {
    /* Tx operation */
    if ((TxRead_p != TxWrite_p))
    {
      UpdateTxMailBox();
    }

    /* Rx operation */
    if ((RxRead_p != RxWrite_p))
    {
      UpdateRxMailBox();
    }
    else
    {
      break;
    }

    /* Error handling for all the errors */
    ulGstatGsr1 = GENI_USBF_GSTAT1;
    ulGstatGsr2 = GENI_USBF_GSTAT2;

    /* Handle error interrupts */
    if ((ulGstatGsr1 & GSR1_ALL_ERR)||((ulGstatGsr1 & GSR2)&&(ulGstatGsr2 & GSR2_ALL_ERR)))
    {
      gsr1 = ulGstatGsr1;
      gsr2 = ulGstatGsr2;

      if (ulGstatGsr2 & SL)
      {
        ReinitUsbDriver();
      }
    }

    /**********************************************************************/
    /* Get the latest value of Tx/Rx Mailbox Read/Wright Address Register */ 
    /* so as to handle the data received or transmitted or during interrup*/
    /* processing.	                                                      */
    /**********************************************************************/
    TxRead_p  = (volatile Tx_Indication *)PCItoCPU(GENI_USBF_TxMail_RD_ADDR);
    TxWrite_p = (volatile Tx_Indication *)PCItoCPU(GENI_USBF_TxMail_WR_ADDR);
    RxRead_p  = (volatile Rx_Indication *)PCItoCPU(GENI_USBF_RxMail_RD_ADDR);
    RxWrite_p = (volatile Rx_Indication *)PCItoCPU(GENI_USBF_RxMail_WR_ADDR);
  }
}


/*********************************************************************
*																	 *
*             S E M I   P U B L I C   C O D E                        *
*																	 *
*            accessed through function pointers                      *
*																	 *
**********************************************************************
*/

/****************************************************************************
*   Name        : _SetClrStallEP                                            *
*																			*
*   Inputs      : EPIndex     - EP index number                             *
*                 OnOff       - Sets/Clears stall condition for an endpoint *
*                               0  -  Clears stall condition                *
*                               1  -  Sets   stall condition                *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine Sets or clears the stall condition for an    *
*                 specified endpoint                                        *
*																			*
*****************************************************************************
*/
static void _SetClrStallEP(U8 EPIndex, int OnOff) 
{
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];

  switch (pEPStat->EPAddr)
  {
  case RXEP4:
    if (!OnOff)
      GENI_USBF_EP4_CONT &= STALL_CLEAR;
    else
      GENI_USBF_EP4_CONT |= SS4;
    break;

  case TXEP3:
    if (!OnOff)
      GENI_USBF_EP3_CONT &= STALL_CLEAR;
    else
      GENI_USBF_EP3_CONT |= SS3;
    break;

  case TXEP5:
    if (!OnOff)
      GENI_USBF_EP5_CONT &= STALL_CLEAR;
    else
      GENI_USBF_EP5_CONT |= SS5;
    break;

  default:
    //invalid EP
    break;
  }
}

/****************************************************************************
*   Name        : _StallEP0                                                 *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : This routine Sets Sets the stall condition for endpoint 0 *
*																			*
*****************************************************************************
*/
static void _StallEP0(void) 
{
  /* Send STALL at handshake stage. */
  GENI_USBF_EP0_CONT |= OSS; 
}

/****************************************************************************
*   Name        : _SetAddress                                               *
*																			*
*   Inputs      : Addr - Address to set                                     *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : It notifies the device controller of new assigned address *
*                 by host this function is called whrenever there is change *
*                 in address required. It actually writes the address into  *
*                 the General Mode reguster (Bits 16 - 22) of USBFU         *
*																			*
*****************************************************************************
*/
static void _SetAddress(U8 Addr) 
{
  U32 address = 0x00000000;

  address |= Addr;

  /* Update Bit 16 - 22 of GMODE register. */
  gmr = GENI_USBF_GMODE;
  gmr &= ADD_CLEAR;
  gmr |= (address << 16);

  GENI_USBF_GMODE = gmr;
}

/****************************************************************************
*   Name        : _Enable                                                   *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : It Enables all available and used endpoints.              *
*																			*
*****************************************************************************
*/
static void _Enable(void) 
{
  GENI_USBF_EP0_CONT |= EP0EN; //Enable Control EP
  GENI_USBF_EP3_CONT |= EP3EN; //Enable Bulk IN EP
  GENI_USBF_EP4_CONT |= EP4EN; //Enable Bulk OUT EP
  GENI_USBF_EP5_CONT |= EP5EN; //Enable Int IN EP    
}

/****************************************************************************
*   Name        : _DisableRxInterruptEP                                     *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : Disables the RX (OUT) endpoint interrupt.                 *
*																			*
*****************************************************************************
*/
static void _DisableRxInterruptEP(U8 EPIndex) 
{
  EP_STAT * pEPStat;

  pEPStat = &USB_aEPStat[EPIndex];

  switch (pEPStat->EPAddr)
  {
  case RXEP0:  //EP0 Out endpoint
    GENI_USBF_INTMSK1 &= ~EP0RF;
    break;
  case RXEP4: //bulk OUT endpoint
    GENI_USBF_INTMSK1 &= ~EP4RF;
    break;
  default:
    //Incorrect end point
    break;
  }
}

/****************************************************************************
*   Name        : _EnableRxInterruptEP                                      *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : Enables the RX (OUT) endpoint interrupt.                  *
*																			*
*****************************************************************************
*/
static void _EnableRxInterruptEP(U8 EPIndex) 
{
  EP_STAT       * pEPStat;
  pEPStat = &USB_aEPStat[EPIndex];

  switch (pEPStat->EPAddr)
  {
  case RXEP0:
    GENI_USBF_INTMSK1 |= EP0RF;
    break;
  case RXEP4: 
    GENI_USBF_INTMSK1 |= EP4RF;
    break;
  default:
    //incorrect end point
    break;
  }
}

/****************************************************************************
*   Name        : _UpdateEP                                                 *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*   Outputs     : None                                                      *
*   Returns     : None                                       				*
*                                                                           *
*   Description : Sets or updates the endpoint's configuration register     *
*                 Set everything except the Enabling. It will be enabled by *
*                 enable routine.                                           *
*																			*
*****************************************************************************
*/
static void _UpdateEP(EP_STAT * pEPStat) 
{

}

/****************************************************************************
*   Name        : _AllocEP                                                  *
*																			*
*   Inputs      : InDir - Specifies whether it is OUT(0) or IN(1) endpoint  *
*                 TransferType - Specifies the desired transfer type.       *
*                        		0 - Control endpoint                        *
*                        		1 - Isochronous endpoint                    *
*                        		2 - Bulk endpoint                           *
*                        		3 - Interrupt endpoint                      *
*   Outputs     : None                                                      *
*   Returns     : Endpoint address (number) that shall be used. It is       *
*                 specific to VR4181 A controller.                          *
*                   4 => Bulk out                                           *
*                   3 => Bulk In                                            *
*                   5 => Int In                				                *
*                                                                           *
*   Description : Allocates an endpoint to be used.                         *
*                 TRetrun value is also used as endpoint address.           *
*																			*
*****************************************************************************
*/
static U8 _AllocEP(U8 InDir, U8 TransferType) 
{
  U8 u8RetVal = ERR_EPN;

  switch (TransferType)
  {
  case USB_TRANSFER_TYPE_BULK:

    if (InDir == USB_DIR_OUT)
    {
      u8RetVal = RXEP4;
    }
    else if (InDir == USB_DIR_IN)
    {
      u8RetVal = TXEP3;
    }
    break;

  case USB_TRANSFER_TYPE_INT:
    if (InDir == USB_DIR_IN)
    {
      u8RetVal = TXEP5;
    }
    else
    {
      u8RetVal = ERR_EPN;//return error $
    }
    break;

  default:
    u8RetVal = ERR_EPN; //return error $
    break; 

  } 
  return u8RetVal;
}

/****************************************************************************
*   Name        : _GetMaxPacketSize                                         *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*   Outputs     : None                                                      *
*   Returns     : Max packet size of the endpoint                           *
*                                                                           *
*   Description : Returns the max packet size the endpoint can handle.      *
*																			*
*****************************************************************************
*/
static unsigned _GetMaxPacketSize(U8 EPIndex) 
{
  /* For all end points Max packet size is 64 bytes */
  return (U16)MAXP;  
}

/****************************************************************************
*   Name        : _Init                                                     *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                                      *
*                                                                           *
*   Description : This routine initialises the PCI and USB hardware.It Sets *
*                 up the mailboxes and Buffer descriptors and default values*
*                 for EP0 and enables the various interrupts needed for USB *
*                 operations.                                               *
*																			*
*****************************************************************************
*/
static void _Init(void)
{
  InitIOPCIU();
  InitUsbHardware();
  SetupUsbMailBox();
}

/****************************************************************************
*   Name        : _Attach                                                   *
*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                                      *
*                                                                           *
*   Description : This routine attaches the device to USB cable.            *
*																			*
*****************************************************************************
*/
static void _Attach(void) 
{
  //NA
}

/****************************************************************************
*   Name        : _StartTx                                                  *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*   Outputs     : None                                                      *
*   Returns     : None                                                      *
*                                                                           *
*   Description : This routine Initiates an IN data transfer                *
*																			*
*****************************************************************************
*/
static void _StartTx(U8 EPIndex) 
{
  EP_STAT * pEPStat;
  U32 command = 0;
  U32 reg = 0;
  U32 packet_size = 64;
  U32 timeout_delay = 0;
  U8 ep_num = 0;
  int szlp = 0;


  pEPStat = &USB_aEPStat[EPIndex];

  switch (pEPStat->EPAddr)
  {
  case RXEP0: //Control IN end point
    reg = GENI_USBF_EP0_CONT;
    szlp = 1;
    command = CMD_SEND_EP0;
    ep_num = 0x00;
    break;

  case TXEP3: //Bulk IN end point
    reg = GENI_USBF_EP3_CONT;
    packet_size = (reg & 0x000000ff);
    if (reg & 0x00080000) szlp = 1;
    command = CMD_SEND_EP3;
    ep_num = 0x02;
    break;

  case TXEP5: //Int IN end point
    reg = GENI_USBF_EP5_CONT;
    packet_size = (reg & 0x000000ff);
    command = CMD_SEND_EP5;
    ep_num = 0x03;
    break;

  default:
    break;
  }

  /* Check if the endpoint is disbled then return. */
  if ((reg >> 31) == 0) 
  {
    /* EndPoint desable: not send Data: */
    return;	
  }

  /* Fill the transmit buffer decriptors. */
  sn_TxBufferDesc[0].Word[0] = (U32)(LASTBIT | DLBIT | pEPStat->NumBytesRem);
  sn_TxBufferDesc[0].Word[1] = CPUtoPCI((U32)pEPStat->pData);
  sn_TxBufferDesc[1].Word[0] = 0;
  sn_TxBufferDesc[1].Word[1] = 0;

  /* Protect the below operations from interrupt. */
  OS_IncDI();

  /* Wait while the USB is busy processing previous command. */
  timeout_delay = USB_ERR_TIMEOUT;

  while (--timeout_delay) 
  {
    if ((GENI_USBF_CMD  & CMR_BUSY) == 0) 
    {
      break;
    }
  }

  if (!timeout_delay)
  {
    Usb_tx_cmd_busy_err_cnt++;
    goto TXCML;
  }

  timeout_delay = USB_ERR_TIMEOUT;

  /* Wait till Transmit endpoint is busy. */
  while (((GENI_USBF_TxEP_STAT  & (TX_FULL_BIT << (ep_num * 8))) != 0) && (--timeout_delay));

  if (!timeout_delay)
  {
    Usb_tx_Epn_busy_err_cnt++;
    goto TXCML;
  }

  /* Issue the command for data transmission over USB */
  GENI_USBF_CMD_ADDRESS = CPUtoPCI((U32) sn_TxBufferDesc);
  GENI_USBF_CMD = (command | pEPStat->NumBytesRem);  

  /* Send Zero length packet if required. */
  if (szlp == 0) 
  {
    if ((pEPStat->NumBytesRem != 0) && ((pEPStat->NumBytesRem % packet_size) == 0)) 
    {
      /* Wait while the USB is busy processing previous command. */
      timeout_delay = USB_ERR_TIMEOUT;
      while (--timeout_delay) 
      {
        if ((GENI_USBF_CMD  & CMR_BUSY) == 0) 
        {
          break;
        }
      }

      if (!timeout_delay)
      {
        Usb_tx_cmd_busy_err_cnt++;
        goto TXCML;
      }

      timeout_delay = USB_ERR_TIMEOUT;

      /* Wait while Transmit endpoint is busy. */
      while (((GENI_USBF_TxEP_STAT  & (TX_FULL_BIT << (ep_num * 8))) != 0) && (--timeout_delay));

      if (timeout_delay)
      {
        /* Issue the command for data transmission over USB */
        GENI_USBF_CMD_ADDRESS = CPUtoPCI((U32)sn_TxBufferDesc_Length0);
        GENI_USBF_CMD = command; 
      }
      else
      {
        Usb_tx_Epn_busy_err_cnt++;
      }
    }
  }
TXCML:
  /* The squeeze permission */
  OS_DecRI();
}

/****************************************************************************
*   Name        : _SendEP                                                   *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*				  p       - Pointer to the data that shall be sent.         *
*                 NumBytes- Number of bytes to send                         *
*																			*
*   Outputs     : None                                                      *
*   Returns     : None                                                      *
*                                                                           *
*   Description : This routine Initiates an IN data transfer                *
*																			*
*****************************************************************************
*/
static void _SendEP(U8 EPIndex, const U8 * p, unsigned NumBytes) 
{
}

/****************************************************************************
*   Name        : _DisableTx                                                *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*																			*
*   Outputs     : None                                                      *
*   Returns     : None                                                      *
*                                                                           *
*   Description : Disable an IN endpoint/interrupt.                         *
*																			*
*****************************************************************************
*/
static void _DisableTx(U8 EPIndex) 
{
  USB_USE_PARA(EPIndex);
}

/****************************************************************************
*   Name        : _IsInHighSpeedMode                                        *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*																			*
*   Outputs     : None                                                      *
*   Returns     : Bit 0 set  - USB device is a high speed capable           *
*    			  Bit 1 set  - USB device is a high speed mode              *
*                                                                           *
*   Description : This function specifies whether device is High Speed      *
*                 capable and if it is in high speed mode                   *
*																			*
*****************************************************************************
*/
static int _IsInHighSpeedMode(void) 
{
  return 0;
}

/****************************************************************************
*   Name        : _ResetEP                                                  *
*																			*
*   Inputs      : EPIndex - EP index number                                 *
*																			*
*   Outputs     : None                                                      *
*   Returns     : Bit 0 set  - USB device is a high speed capable           *
*    			  Bit 1 set  - USB device is a high speed mode              *
*                                                                           *
*   Description : This function Resets the data toggle to DATA0. It is      *
*                 useful after removing a HALT condition on a BULK endpoint *
*	              Refer to Chapter 5.8.5 in the USB Serial Bus Specification*
*                 Rev.2.0.								                    *
*																			*
*****************************************************************************
*/
static void _ResetEP(U8 EPIndex) 
{
  USB_USE_PARA(EPIndex);
}

/****************************************************************************
*   Name        : USB_X_AddDriver                                           *

*																			*
*   Inputs      : None                                                      *
*   Outputs     : None                                                      *
*   Returns     : None                                                      *
*                                                                           *
*   Description : This function Attaches low level USB driver structure to  *
*                 the USB stack.                                            *
*																			*
*****************************************************************************
*/
void USB_X_AddDriver(void)
{
  USB_AddDriver(&USB_Driver_VR_4181A);
}

/*********************************************************************
*                                                                    *
*       P U B L I C   C O N S T                                      *
*                                                                    *
**********************************************************************
*/
const USB_HW_DRIVER USB_Driver_VR_4181A = 
{
  _Init,
  _AllocEP,
  _UpdateEP,
  _Enable,
  _Attach,
  _GetMaxPacketSize,
  _IsInHighSpeedMode,
  _SetAddress,
  _SetClrStallEP,
  _StallEP0,
  _DisableRxInterruptEP,
  _EnableRxInterruptEP,
  _StartTx,
  _SendEP,
  _DisableTx,
  _ResetEP,
  NULL
};

