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
/* FILE NAME        : Mailbox.h                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Contains the embOS mailboxes                    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __MAILBOX_H__
#define __MAILBOX_H__

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

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#ifndef __PC__
    #define MB_IOB_RECEIVE_SIZE  50
    #define MB_IOB_TRANSMIT_SIZE 50
#else
    #define MB_IOB_RECEIVE_SIZE  90 // 10*RXTGM_LEN
    #define MB_IOB_TRANSMIT_SIZE 60 // 10*TXTGM_LEN
#endif


/*****************************************************************************
 *
 *
 *              MAILBOX DECLARATIONS
 *
 *
 *****************************************************************************/
extern OS_MAILBOX MBIobReceive;
extern U8 MBIobReceiveBuf[MB_IOB_RECEIVE_SIZE];

extern OS_MAILBOX MBIobTransmit;
extern U8 MBIobTransmitBuf[MB_IOB_TRANSMIT_SIZE];

#endif
