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
/* MODULE NAME      :    com_drv_l.c                                        */
/*                                                                          */
/* FILE NAME        :    com_drv_l.c                                        */
/*                                                                          */
/* FILE DESCRIPTION :    Channel driver for PC                              */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "com_drv_if.h"           /* Access to Driver interface functions   */
#include "pc_com_driver.h"


/****************************************************************************/
/*                                                                          */
/* D E F I N I T I O N S                                                    */
/*                                                                          */
/****************************************************************************/
#define CH_NO COM_CH>>6

BIT COM_geni_irq_idle;
BIT COM_soft_timer_started;

static unsigned char rxBuf[256];
static unsigned char txBuf[256];
unsigned int txIndex = 0;
BIT mReadyToTransmit = FALSE;

void InitCOMDriver(void) 
{
#if(COM_IDLE_TYPE == GENI_IRQ_IDLE)
  COM_geni_irq_idle = FALSE;
  COM_soft_timer_started = FALSE;
#endif

  StartComTask();
  
  GeniChannelBusy(CH_NO, FALSE); 
}

void DisableCOMDriver(void) 
{
}

void ResetCOMDriver(void) 
{ 
  if (txIndex > 0)
  {
    SendComData(txBuf, txIndex);
    txIndex = 0;
  }
  // clear busy flag to exit GENI_TRANSMITTING state
  GeniChannelBusy(CH_NO, FALSE);
}

void TransmitCOMByte(UCHAR tx_byte) 
{   
  txBuf[txIndex++] = tx_byte;
  SendTxByte(CH_NO);
}

UCHAR TransmitSetupCOM(void) 
{
  // reset tx buffer
  memset(txBuf, 0, sizeof(txBuf));
  txIndex = 0;
  mReadyToTransmit = TRUE;
  return TRUE;
}


void GeniCOMInIrq(void)
{   
  unsigned int len;
  int i;   

#if(COM_IDLE_TYPE == GENI_IRQ_IDLE)
  COM_soft_timer_started = TRUE;
  COM_geni_irq_idle = FALSE;
#endif

  len = ReceiveComData(rxBuf, sizeof(rxBuf)); // indeholder netop een geni-requests

  if (len > 0)
  {
    // buffer all bytes
    for (i=0; i<len; i++) 
    {        
      SaveRxByte(CH_NO, rxBuf[i]);
    }
  }

  if (mReadyToTransmit)
  {
    //start transmission
    mReadyToTransmit = FALSE;
    SendTxByte(CH_NO);
  }
}

#if(COM_IDLE_TYPE == GENI_IRQ_IDLE)
/****************************************************************************
*     Name      :  GeniCOMIdleTimerIrq                                      *
*     Inputs    :  none                                                     *
*     Outputs   :                                                           *
*     Updates   :  COM_soft_timer_started, channel_busy, COM_geni_irq_idle  *
*     Returns   :                                                           *
*   Description :  this procedure is called every time  the GeniSysIrq is   *
*               :  called                                                   *
*---------------------------------------------------------------------------*/
void GeniCOMIdleTimerIrq(void)
{
  if (COM_soft_timer_started == TRUE)
  {
    if (COM_geni_irq_idle == TRUE)          // we didn't receive any since last time
    {
      COM_soft_timer_started = FALSE;       // Stop the checking
      GeniChannelBusy(CH_NO, FALSE);        // signal that the channel is busy
    }
    COM_geni_irq_idle = TRUE;
  }
}

#endif


