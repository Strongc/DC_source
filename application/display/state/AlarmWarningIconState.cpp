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
/* CLASS NAME       : AlarmWarningIconState                                 */
/*                                                                          */
/* FILE NAME        : AlarmWarningIconState.cpp                             */
/*                                                                          */
/* CREATED DATE     : 2009-07-14                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmWarningIconState.       */
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
#include <AlarmWarningIconState.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusWarning;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusAlarm;

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {

    StateIconId mAlarmWarningIconStatesIconIds[] = {
      {ALARM_STATE_WARNING, &bmStatusWarning},
      {ALARM_STATE_ALARM,   &bmStatusAlarm}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmWarningIconState::AlarmWarningIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mAlarmWarningIconStatesIconIds;
      mIconIdCount = sizeof( mAlarmWarningIconStatesIconIds ) / sizeof(StateIconId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmWarningIconState::~AlarmWarningIconState()
    {
    }


  } // namespace display
} // namespace mpc


