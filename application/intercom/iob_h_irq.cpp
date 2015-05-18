/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos                                     */
/*               All rights reserved                                        */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/****************************************************************************/
/*                                                                          */
/* FILE NAME        : iob_h_irq.cpp                                         */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : IOB communication interrupt handling.           */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <IobComDrv.h>
#include <IobHwIf.h>
#include <vr4181a.h>
#include <microp.h>
#include <mailbox.h>
#include <iob_h_irq.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define UART_CLOCK_SETUP           (*(U16 *)CMUCLKMSK0)
#define RXD0                       (*(U8 *)SIURB_0)
#define TXD0                       (*(U8 *)SIUTH_0)
#define UART0_BAUDRATE_SETUP_LSB   (*(U8 *)SIUDLL_0)
#define UART0_INT_ENABLE           (*(U8 *)SIUIE_0)
#define UART0_BAUDRATE_SETUP_MSB   (*(U8 *)SIUDLM_0)
#define UART0_INT_INDICATE         (*(U8 *)SIUIID_0)
#define UART0_FIFO_CTRL            (*(U8 *)SIUFC_0)
#define UART0_LINE_CTRL            (*(U8 *)SIULC_0)
#define UART0_MODEM_CTRL           (*(U8 *)SIUMC_0)
#define UART0_LINE_STATUS          (*(U8 *)SIULS_0)
#define UART0_MODEM_STATUS         (*(U8 *)SIUMS_0)
#define UART0_SCRATCH              (*(U8 *)SIUSC_0)
#define UART0_RESET                (*(U8 *)SIURESET_0)
#define UART0_OPERATION_MASK       (*(U8 *)SIUACTMSK_0)
#define UART0_ACTIVITY_TIMER       (*(U8 *)SIUACTTMR_0)

#define FIFO_TRIGGER_LEVEL 8

#define TX_BUFFER_SIZE     12

static BOOL isr_enabled = FALSE;

static bool mTestUartLoopBack = false;
static bool mTestUartLoopBackReadyToRead = false;
static UART_LOOP_BACK_TEST_TYPE mUartLoopBackStatus = UART_NOT_TO_BE_TESTED;


/*****************************************************************************
 *
 *
 *              LOCAL FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function - InitUart0
 * DESCRIPTION: Initialises vr4181 UART0
 *
 *****************************************************************************/
static void InitUart0(void)
{
  //Init clock to uart 0, 1, 2 ... Skal det ske her?
  UART_CLOCK_SETUP = 0x0700;

  //Activity timer is not notified
  UART0_OPERATION_MASK = 0x3D;
  //Use of activity timer prohibited
  UART0_ACTIVITY_TIMER = 0x00;

  //8 data bits, 1 stop bit, no parity, set LCR7 to access divisor latch
  UART0_MODEM_CTRL = 0x00;
  UART0_LINE_CTRL = 0x83;

  //Baudrate = 57600
  UART0_BAUDRATE_SETUP_LSB = 0x14;
  UART0_BAUDRATE_SETUP_MSB = 0x00;

  //Enable receive/transmit FIFO and set trigger level
  UART0_FIFO_CTRL = 0;
  UART0_FIFO_CTRL = 0x01; //first set FCR0
  UART0_FIFO_CTRL = 0x87; //then set trigger level

  //DMA transfer method is FIFO mode
  //UART0_FIFO_CTRL = 0x09;

  //Clear LCR7 to access to RxD, TxD and IE register
  UART0_LINE_CTRL = 0x03;

  //Enable receive data ready and transmit holding register empty interrupt
  UART0_INT_ENABLE = 0x03;
}

/*****************************************************************************
 * Function - RxDataReady
 * DESCRIPTION: ISR when rx trigger level reached.
 *              FIFO_TRIGGER_LEVEL bytes is now in FIFO buffer
 *
 *****************************************************************************/
static void RxDataReady(void)
{
  int fifo_indx = 0;
  char rx_byte;

  /* Empty FIFO */
  while (UART0_LINE_STATUS & 0x01)
  {
    rx_byte = RXD0;
    if ( mTestUartLoopBack == true)
    {
      fifo_indx++;
      if (mTestUartLoopBackReadyToRead == true)
      {
        if (rx_byte != 0x55)
        {
           mUartLoopBackStatus = UART_TEST_ERROR;
        }
        else
        {
           mUartLoopBackStatus = UART_TEST_OK;
        }
      }
      else
      {
       if ( fifo_indx >= FIFO_TRIGGER_LEVEL)
       {
         mTestUartLoopBackReadyToRead = true;
       }
      }
    }
    if ( isr_enabled == TRUE )
    {
      OS_PutMailCond1(&MBIobReceive, &rx_byte);
    }
  }
  /* Signal InterCom Task */
  IobComDrv::GetInstance()->SignalIobDataRxed();
}
/*****************************************************************************
 * Function - TxFifoEmpty
 * DESCRIPTION: ISR when tx FIFO is empty
 *
 *****************************************************************************/
static void TxFifoEmpty()
{
  int mb_empty;
  int i = 0;
  char tx_byte ;

  if ( isr_enabled == TRUE )
  {
    do
    {
      mb_empty = OS_GetMailCond1(&MBIobTransmit, &tx_byte);
      if ( mTestUartLoopBack == false)
      {
        if (!mb_empty)
        {
          TXD0 = tx_byte;
          i++;
        }
        else
        {
          if (i == 0)
          {
            /* Put 2 dummy tgms in FIFO */
            for (i=0; i<12; i++)
            {
              TXD0 = 0x80; // dummy to generate irq!!
            }
          }
        }
      }
      else if( mTestUartLoopBack == true)
      {
        for (i=0; i<12; i++)
        {
          TXD0 = 0x55; // dummy to generate irq!!
        }
      }
    } while (!mb_empty);
  }

  /* Signal InterCom Task */
  IobComDrv::GetInstance()->SignalIobDataTxed();
}

/*****************************************************************************
 * Function - InternalComIrqHandler
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" void StartUartLoopBackTest(void)
{
  mTestUartLoopBack = true;
  if (mUartLoopBackStatus == UART_NOT_TO_BE_TESTED)
  {
    mTestUartLoopBackReadyToRead = false;
    mUartLoopBackStatus = UART_TESTING;
  }
}
/*****************************************************************************
 * Function - InternalComIrqHandler
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" void StopUartLoopBackTest(void)
{
  mTestUartLoopBack = false;
  mUartLoopBackStatus = UART_NOT_TO_BE_TESTED;

}

/*****************************************************************************
 * Function - InternalComIrqHandler
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" UART_LOOP_BACK_TEST_TYPE GetUartLoopBackTestStatus(void)
{
    return mUartLoopBackStatus;
}
/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - InternalComIrqHandler
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" BOOL InternalComIrqHandler(void)
{
  U8 local_siuiid;
  BOOL ret_value = FALSE;

  local_siuiid = (*(U8*)SIUIID_0);

  /* Handle receive irq */
//  if (( local_siuiid & 0x0F) == 4)  // Receive data exist
  if (( local_siuiid & 0x04) == 4)  // Receive data exist
  {
    RxDataReady(); // Call ISR
    ret_value = TRUE;
  }

  /* Handle transmit irq */
  //if (( local_siuiid & 0x0F ) == 2)   // Transmit holding register empty
  if (( local_siuiid & 0x02 ) == 2)   // Transmit holding register empty
  {
    TxFifoEmpty(); // Call ISR
    ret_value = TRUE;
  }
  return ret_value;
}


/*****************************************************************************
 * Function - InitInterComChannel
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" void InitInterComChannel(void)
{
  InitUart0();
  isr_enabled = TRUE;
}

/*****************************************************************************
 * Function - StartInterComTransmit
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" void StartInterComTransmit(void)
{
  TxFifoEmpty();
}

/*****************************************************************************
 * Function - StartInterComTransmit
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" int GetTxBufferSize(void)
{
  return TX_BUFFER_SIZE;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
