/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : Mailbox                                               */
/*                                                                          */
/* FILE NAME        : Mailbox.cpp                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Contains the embOS mailboxes                    */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

 /*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <rtos.h>
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Mailbox.h>


/*****************************************************************************
 *
 *
 *              MAILBOX DECLARATIONS
 *
 *
 *****************************************************************************/
OS_MAILBOX MBIobReceive;
U8 MBIobReceiveBuf[MB_IOB_RECEIVE_SIZE];

OS_MAILBOX MBIobTransmit;
U8 MBIobTransmitBuf[MB_IOB_TRANSMIT_SIZE];

