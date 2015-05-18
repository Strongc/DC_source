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
/* FILE NAME        : GeniDrv.cpp                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Contains the GENI (RS232) low level driver      */
/*                          interface.                                      */
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

extern "C"                              // TEST JUA: Scope extern "C" can be removed
{                                       // when new GENIpro version is ready.
#include <rs232_drv_if.h>
}

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 *
 *       Function definitions for the interface of low level Geni drivers
 *
 *       Low level functions
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - TransmitSetupRS232
 * DESCRIPTION: Sets up the driver for transmitting
 *
 *****************************************************************************/
extern "C" UCHAR TransmitSetupRS232(void)
{
  SendTxByte(RS232_CH_INDX);
  IobComDrv::GetInstance()->StartBusTransmit();
  return true;
}

/*****************************************************************************
 * Function - TransmitRS232Byte
 * DESCRIPTION: Send byte to the channel
 *
 *****************************************************************************/
extern "C" void TransmitRS232Byte(UCHAR tx_byte)
{
  if ((IobComDrv::GetInstance()->StoreTxByte(tx_byte)) == true)
  {
    //Request one more byte from GENI
    SendTxByte(RS232_CH_INDX);
  }
}

/*****************************************************************************
 * Function - ResetRS232Driver
 * DESCRIPTION: Reset Genichannel driver low level settings for receiving
 *              the next tgm.
 *
 *****************************************************************************/
extern "C" void ResetRS232Driver(void)
{
  IobComDrv::GetInstance()->ResetBus();
}

/*****************************************************************************
 * Function - InitRS232Driver
 * DESCRIPTION: Sets up the driver for receiving
 *
 *****************************************************************************/
extern "C" void InitRS232Driver(UCHAR setup_param)
{
  // This function has been added a parameter in the GENI 6.02.01 - but as the
  // configuration of the intercom is fixed the parameter is not used.
;
}

/*****************************************************************************
 * Function - DisableRS232Driver
 * DESCRIPTION: Disables all interrupts
 *
 *****************************************************************************/
extern "C" void DisableRS232Driver(void)
{
;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

