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
/* MODULE NAME      :    USB Interface Layer                                */
/*                                                                          */
/* FILE NAME        :    usb_drv_I.c                                        */
/*                                                                          */
/* FILE DESCRIPTION :    USB interface layer (Wrapper) for USB Low level    */
/*                       Driver.                                            */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "typedef.h"
#include "geni_cnf.h"             /* Access to GENIpro configuration        */
#include "MicroP.h"               /* Access to global interrupt             */
#include "common.h"               /* Access to common definitions           */
#include "geni_rtos_if.h"         /* Access to RTOS                         */
#include "usb_drv_if.h"           /* Access to Driver interface functions   */
#include "usb_drv_I.h"            /* Access to itself                       */
#include "RTOS.H"
#include "mips_irq.h"
#include "USB_HW_Driver.h"
#include "USB_CDC.h"
#include "USB.h"

#ifdef TCS_USB_SER_PORT

#define USB_MAX_PACKET_SIZE  64
#define GENI_TGM_MAX_SIZE    255 
#define USB_TASK_STACK_SIZE  5000
#define USBTX_TASK_PRIORITY  205 // Must be equal to or higher than GENI_SYS task priority (= 100)
#define USBRX_TASK_PRIORITY  215
#define USB_TXSETUP_EVENT    1

/****************************************************************************/
/* EmbOS Interfaces                                                         */
/****************************************************************************/
/* Task handle for USB Transmit task */
OS_TASK USBTxTaskHandle;

/* Task handle for USB Receive task */
OS_TASK USBRxTaskHandle;

/* Stack for USB Transmit task */
OS_STACKPTR GF_UINT32 USBTxStack[USB_TASK_STACK_SIZE];   

/* Stack for USB Receive task */
OS_STACKPTR GF_UINT32 USBRxStack[USB_TASK_STACK_SIZE];

/****************************************************************************/
/*                                                                          */
/* D E F I N I T I O N S                                                    */
/*                                                                          */
/****************************************************************************/
#define USB_CH_NO USB_CH>>6

/****************************************************************************/
/*                                                                          */
/* T Y P E   D E F I N I T I O N S                                          */
/*                                                                          */
/****************************************************************************/
/* USB Interface Layer States. */ 
typedef enum
{
	RX_READY, // 0
	TX_START, // 1 
	TX_DONE   // 2
}eUSB_STATE;

/****************************************************************************/
/*  Local Varibales (File Statics)                                          */
/****************************************************************************/
eUSB_STATE usb_int_state = RX_READY;

BIT usb_channel_busy;
BIT USB_soft_timer_started;    // flag used when timer idle is soft
GF_UINT8 usb_tx_index;
GF_UINT8 usb_rxd_buf[1024];

/* Buffer pointer for Transmit buffer. */
extern GF_UINT8 * cdc_tx_buff_p;

/****************************************************************************/
/*  LOCAL PROTOTYPES                                                        */
/****************************************************************************/
void InitUSBDriver(GF_UINT8);
void USBInterfaceTransmitTask(void);
void USBInterfaceReceiveTask(void);
void TransmitUSBByte(GF_UINT8);
GF_UINT8 TransmitSetupUSB(void);
void ResetUSBDriver(void);
void DisableUSBDriver(void);;
void InitIdleTimer(void);
void GeniUSBIdleTimerIrq(void);
void UsbChannelBusy(BIT isBusy);

extern int USB_GetState(void);
extern void USB_OS_Delay(int);

/****************************************************************************
*
* S T A T I C   C O D E   
*
*****************************************************************************
*/

/****************************************************************************
*    Name        : InitUSBDriver                                            *
*                                                                           *
*    Inputs      : Channel Parameters                                       *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine initialises USB driver.It performs following* 
*                 actions.                                                  * 
*                 1. Initialses the USB Transmit and Receive Tasks          *
*                 2. Initialises teh Idle timer                             *
*                 3. Inidicates the Channel idle condition.                 *
*                                                                           *
*****************************************************************************
*/
void InitUSBDriver(GF_UINT8 p_setup_param)
{
  /* Create and Start USB Transmit Task */
  OS_CREATETASK(&USBTxTaskHandle, "USB TX Task", USBInterfaceTransmitTask,  USBTX_TASK_PRIORITY, USBTxStack);

  /* Create and Start USB Recive Task */
  OS_CREATETASK(&USBRxTaskHandle, "USB RX Task", USBInterfaceReceiveTask,  USBRX_TASK_PRIORITY, USBRxStack);

  /* Signal that the channel is NOT busy */
  UsbChannelBusy(FALSE);

  /* Initialise port and timer for timer idle */
#if(USB_IDLE_TYPE == TIMER_IDLE)
  InitIdleTimer();                                          
#endif
}

/****************************************************************************
*    Name        : USBInterfaceTransmitTask                                 *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This is a Transmit Task which handles all the USB        *
*                 Transmissions.                                            * 
*                 1. It pends on the event for start of USB transmission    * 
*                 2. It calls the USB stack functions to Transmit the data  *
*                    on USB PHY and wait (block) till the Transmission      *
*                    completes or Timeout or error event.                   *
*                 3. On completetion of transmission it resets teh USB      *
*                    interface layer states to RX_ENB to receive new geni   *
*                    Request.                                               *
*                                                                           *
*****************************************************************************
*/    
void USBInterfaceTransmitTask(void)
{
  GF_UINT32 telegram_length = 0;
  GF_UINT32 packet_length = 0;
  GF_UINT32 tx_index = 0;

  /* USB Buffer (FU) Enable. After this register setting HOST can detect 
  the USBFU. Also Attach the USB to the USB PHY. */
  GENI_USBSIGCTRL = 0x000C;
  GENI_GPMODE1 |= 0x0030;
  GENI_GPDATA0 &= 0xFBFF;

  while (1)
  {
    /* Wait for transmit start event */
    OS_WaitEvent(USB_TXSETUP_EVENT);                    

    usb_int_state = TX_START;
    usb_tx_index = 0;

    while (usb_int_state != TX_DONE)
    {
      SendTxByte(USB_CH_NO);  
    }

    /* Obtain total length */
    telegram_length = usb_tx_index;

    for (tx_index = 0; tx_index < telegram_length; tx_index += 64)
    {
      packet_length = telegram_length - tx_index;
      packet_length = (packet_length > 64) ? 64 : packet_length;

      usb_tx_idle_active = FALSE;
      usb_tx_idle_cnt = usb_tx_idle_cnt_reset_value * 2; // restart Idle countdown
      usb_tx_idle_active = TRUE;    

      USB_CDC_WriteTimed(&cdc_tx_buff_p[tx_index], packet_length, USB_TX_WAIT_TIME);
    }
  }
}

/****************************************************************************
*    Name        : TransmitUSBByte                                          *
*                                                                           *
*    Inputs      : Data - Byte to Transmit                                  *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine fills the byte into the USB transmit buffer.*
*                                                                           *
*****************************************************************************
*/
void TransmitUSBByte(GF_UINT8 data)
{
  if (usb_tx_index <= GENI_TGM_MAX_SIZE)
  {
    cdc_tx_buff_p[usb_tx_index++] = data;
  }
  else
  {
    usb_int_state = TX_DONE;//set breakpoint here
  }
}

/****************************************************************************
*    Name        : TransmitSetupUSB                                         *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine Initiates the USB transmission by sending   *
*                 event to the USB transmit task.                           *
*                                                                           *
*****************************************************************************
*/
GF_UINT8 TransmitSetupUSB(void)
{
  BIT ret_val = FALSE;
  
  if (usb_channel_busy == FALSE)
  {
    /* For handling mis-synchronization */
    usb_tx_idle_active = FALSE;
    usb_tx_idle_cnt = usb_tx_idle_cnt_reset_value * 10; 
    usb_tx_idle_active = TRUE;

    /* Signal USB Transmit Task to start transmission. */
    OS_SignalEvent(USB_TXSETUP_EVENT, &USBTxTaskHandle);

    ret_val = TRUE;
  }
  else
  {
    ret_val = FALSE;//breakpoint
  }
  UsbChannelBusy(TRUE);

  return ret_val;
}


/****************************************************************************
*    Name        : USBInterfaceReceiveTask                                  *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This is a Receive Task which handles all the USB data    *
*                 Receive.                                                  * 
*                 1. When USB is not ready it goes into teh delayed state   * 
*                 2. When USB is ready it blocks to receive data over USB.  *
*                 3. Task unblocks when data is received or error on USB    *
*                    occurred. When data is received it calls the virtual   *
*                    driver functions to pass the entire received packet to *
*                    the application.                                       *
*                                                                           *
*****************************************************************************
*/
void USBInterfaceReceiveTask(void)
{
  GF_UINT8 num_bytes_received;
  GF_UINT8 indx;

  /* Set the Initial interface layer state to Receive */
  usb_int_state = RX_READY;

  while (1)
  {
    while ((USB_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) 
    {
      ///\Todo 20150723 -> Change from 50ms to 150ms dur to larger Objcet
      USB_OS_Delay(150);
    }

    num_bytes_received = 0;

    /* Blocking call to receive bytes on USB */
    num_bytes_received = USB_CDC_Receive(usb_rxd_buf, GENI_TGM_MAX_SIZE);

    while (usb_int_state != RX_READY)
    {
      USB_OS_Delay(10);
    }   

    /* If any bytes is received then process it.*/
    if (num_bytes_received > 0)
    {
      for (indx = 0; indx < num_bytes_received; indx++)
      {
        SaveRxByte(USB_CH_NO, usb_rxd_buf[indx]);
      }
    }

  }
}          

/****************************************************************************
*    Name        : ResetUSBDriver                                           *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine Initiates the USB transmission by sending   *
*                 event to teh USB transmit task.                           *
*                                                                           *
*****************************************************************************
*/
void ResetUSBDriver(void)
{
  UsbChannelBusy(FALSE);

  usb_int_state = RX_READY;
    
  //XFK: Make a reset with minimum interval of 100 ms. Part of workaround for communication halt issue
  usb_tx_idle_active = FALSE;
  usb_tx_idle_cnt = usb_tx_idle_cnt_reset_value * 100; 
  usb_tx_idle_active = TRUE;
  
}

/****************************************************************************
*    Name        : DisableUSBDriver                                         *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine Shall disable USB driver.                   *
*                                                                           *
*****************************************************************************
*/
void DisableUSBDriver(void)
{
  // NA.
}

/****************************************************************************
*    Name        : InitIdleTimer                                            *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine Initalises the IDLE timer for USB.          *
*                                                                           *
*****************************************************************************
*/
void InitIdleTimer(void)
{
  GENI_IDLE_TIMER_SETUP;
  GENI_INT_PRIO_LOW;
}

/****************************************************************************
*    Name        : UsbTxCompleted                                           *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description : This routine changes the state of USB interface layer.   *
*                                                                           *
*****************************************************************************
*/
void UsbTxCompleted(void)
{
  usb_int_state = TX_DONE;
}

/****************************************************************************
*    Name        : UsbChannelBusy                                           *
*                                                                           *
*    Inputs      : None                                                     *
*    Outputs     : None                                                     *
*    Returns     : None                                                     *
*                                                                           *
*    Description :                                                          *
*                                                                           *
*****************************************************************************
*/
void UsbChannelBusy(BIT isBusy)
{
  usb_channel_busy = isBusy;
  GeniChannelBusy(USB_CH_NO, isBusy);
}

#endif
/******************  END OF FILE *******************************************/


