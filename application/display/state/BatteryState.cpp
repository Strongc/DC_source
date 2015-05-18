/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MR                                            */
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
/* CLASS NAME       : BatteryState                                          */
/*                                                                          */
/* FILE NAME        : BatteryState.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2008-04-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a BatteryState.                */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "BatteryState.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    const StateStringId mBatteryStatesStringIds[] =
    {
      { BATTERY_STATE_WRONG_POLARITY,   SID_BAT_WRONG_POLARITY  },
      { BATTERY_STATE_BATTERY_DEFECT,   SID_BAT_BATTERY_DEFECT  },
      { BATTERY_STATE_BATTERY_MISSING,  SID_BAT_BATTERY_MISSING },
      { BATTERY_STATE_LOW_BATTERY,      SID_BAT_LOW_BATTERY     },
      { BATTERY_STATE_SHORT_CIRCUIT,    SID_BAT_SHORT_CIRCUIT   },
      { BATTERY_STATE_OK,               SID_BAT_OK              }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    BatteryState::BatteryState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mBatteryStatesStringIds;
      mStringIdCount = sizeof( mBatteryStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    BatteryState::~BatteryState()
    {
    }
    
    void BatteryState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




