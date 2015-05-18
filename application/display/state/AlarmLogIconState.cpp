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
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <AlarmLogIconState.h>
#include "DataPoint.h"

#include <AlarmInLog.h>
#include <AppTypeDefs.h>
#include <GUI.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusWarning;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusAlarm;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusNone;

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mAlarmLogStatesIconIds[] = {
      {ALARM_STATE_WARNING, &bmStatusWarning},
      {ALARM_STATE_ALARM,   &bmStatusAlarm},
      {ALARM_STATE_READY,   &bmStatusNone}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmLogIconState::AlarmLogIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mAlarmLogStatesIconIds;
      mIconIdCount = sizeof( mAlarmLogStatesIconIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmLogIconState::~AlarmLogIconState()
    {
    }


  } // namespace display
} // namespace mpc


