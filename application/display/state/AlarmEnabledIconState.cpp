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
/* CLASS NAME       : AlarmEnabledIconState                                 */
/*                                                                          */
/* FILE NAME        : AlarmEnabledIconState.cpp                             */
/*                                                                          */
/* CREATED DATE     : 2012-02-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmEnabledIconState.       */
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
#include <AlarmEnabledIconState.h>
#include <AlarmConfig.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusNone;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusAlarm;

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    StateIconId mAlarmEnabledIconStateIds[] = {
      {0,   &bmStatusNone},
      {1,   &bmStatusAlarm}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmEnabledIconState::AlarmEnabledIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mAlarmEnabledIconStateIds;
      mIconIdCount = sizeof(mAlarmEnabledIconStateIds) / sizeof(StateIconId);
      mSubjectAsInt.SetMinValue(0);
      mSubjectAsInt.SetMaxValue(1);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmEnabledIconState::~AlarmEnabledIconState()
    {
    }

    /*****************************************************************************
    * Function - GetSubject
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Subject* AlarmEnabledIconState::GetSubject()
    {
      AlarmConfig* pDP = dynamic_cast<AlarmConfig*> (ObserverText::GetSubject());
      if (pDP != NULL)
      {
        mSubjectAsInt.SetAsInt(pDP->GetAlarmEnabled());
      }
      return &mSubjectAsInt;
    }

  } // namespace display
} // namespace mpc


