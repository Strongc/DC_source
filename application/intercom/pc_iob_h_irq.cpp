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
/*                                                                          */
/*          NOTE: THIS FILE SUPPORTS __PC__ (simulator)                     */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <IobComDrv.h>
#include <mailbox.h>
#include <iob_h_irq.h>
#include <assert.h>


/*****************************************************************************
  DEFINES
 *****************************************************************************/

static bool mTestUartLoopBack = false;
static bool mTestUartLoopBackReadyToRead = false;
static UART_LOOP_BACK_TEST_TYPE mUartLoopBackStatus = UART_NOT_TO_BE_TESTED;

extern "C" void StartUartLoopBackTest(void)
{
  mTestUartLoopBack = true;
  if (mUartLoopBackStatus == UART_NOT_TO_BE_TESTED)
  {
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
	return true;
}


/*****************************************************************************
 * Function - InitInterComChannel
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" void InitInterComChannel(void)
{
  InitUart0();
}

/*****************************************************************************
 * Function - StartInterComTransmit
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" void StartInterComTransmit(void)
{
 // TxFifoEmpty();
}

/*****************************************************************************
 * Function - StartInterComTransmit
 * DESCRIPTION: See h-file
 *
 *****************************************************************************/
extern "C" int GetTxBufferSize(void)
{
  return MB_IOB_TRANSMIT_SIZE;
}



/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
