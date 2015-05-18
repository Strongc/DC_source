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
/* CLASS NAME       : PowerDown                                             */
/*                                                                          */
/* FILE NAME        : PowerDown.H                                           */
/*                                                                          */
/* CREATED DATE     : 16-03-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcPowerDown_h
#define mpcPowerDown_h

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

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define POWER_DOWN_EVENT                1
#define CONFIG_CTRL_POWERED_DOWN_EVENT  2
#define DISPLAY_POWERED_DOWN_EVENT      4

#define POWER_DOWN_PRIO 240

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  FUNCTIONS
 *****************************************************************************/
EXTERN BOOL PowerDownIrq(void);
EXTERN void EnablePowerDownIrq(void);
EXTERN void ChkSupplyVoltage(void);
EXTERN void SetAlarmControlPtrForPowerDown(void* pAlarmControl);
EXTERN void SignalEventToPowerDown(U8 event);

#endif
