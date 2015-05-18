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
/* CLASS NAME       : WarningEnabledIconState                               */
/*                                                                          */
/* FILE NAME        : WarningEnabledIconState.cpp                           */
/*                                                                          */
/* CREATED DATE     : 2012-02-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a WarningEnabledIconState.     */
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
#include <WarningEnabledIconState.h>
#include <AlarmConfig.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusNone;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmStatusWarning;

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    StateIconId mWarningEnabledIconStateIds[] = {
      {0,   &bmStatusNone}, 
      {1,   &bmStatusWarning}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    WarningEnabledIconState::WarningEnabledIconState(Component* pParent) : IconState(pParent)
    {
      mpStateIconIds = mWarningEnabledIconStateIds;
      mIconIdCount = sizeof(mWarningEnabledIconStateIds) / sizeof(StateIconId);
      mSubjectAsInt.SetMinValue(0);
      mSubjectAsInt.SetMaxValue(1);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    WarningEnabledIconState::~WarningEnabledIconState()
    {
    }

    /*****************************************************************************
    * Function - GetSubject
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Subject* WarningEnabledIconState::GetSubject()
    {
      AlarmConfig* pDP = dynamic_cast<AlarmConfig*> (ObserverText::GetSubject());
      if (pDP != NULL)
      {
        mSubjectAsInt.SetAsInt(pDP->GetWarningEnabled());
      }
      return &mSubjectAsInt;
    }


  } // namespace display
} // namespace mpc


