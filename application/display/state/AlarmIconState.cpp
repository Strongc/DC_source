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
/* CLASS NAME       : AlarmIconState                                        */
/*                                                                          */
/* FILE NAME        : AlarmIconState.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmIconState.              */
/*                                                                          */
/****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <AlarmIconState.h>
#include "DataPoint.h"

#include <AlarmInLog.h>
#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmMenuStatusWarning;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmMenuStatusAlarm;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmMenuStatusNone;
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mAlarmStatesIconIds[] = {
#ifdef USE_TFT_COLOURS
      {ALARM_STATE_WARNING, &bmMenuStatusWarning},
#else
      {ALARM_STATE_WARNING, &bmMenuStatusNone},
#endif
      {ALARM_STATE_ALARM,   &bmMenuStatusAlarm},
      {ALARM_STATE_READY,   &bmMenuStatusNone}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmIconState::AlarmIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mAlarmStatesIconIds;
      mIconIdCount = sizeof( mAlarmStatesIconIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmIconState::~AlarmIconState()
    {
    }


  } // namespace display
} // namespace mpc


