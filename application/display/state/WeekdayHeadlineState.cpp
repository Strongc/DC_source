/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : WeekdayHeadlineState                                  */
/*                                                                          */
/* FILE NAME        : WeekdayHeadlineState.cpp                              */
/*                                                                          */
/* CREATED DATE     : 2007-11-28                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a WeekdayHeadlineState.        */
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
#include "DataPoint.h"
#include "WeekdayHeadlineState.h"
#include <AppTypeDefs.h>

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
    StateStringId mWeekdayHeadlineStatesStringIds[] = {
      {WEEKDAY_MONDAY,               SID_MONDAY},
      {WEEKDAY_TUESDAY,              SID_TUESDAY},
      {WEEKDAY_WEDNESDAY,            SID_WEDNESDAY},
      {WEEKDAY_THURSDAY,             SID_THURSDAY},
      {WEEKDAY_FRIDAY,               SID_FRIDAY},
      {WEEKDAY_SATURDAY,             SID_SATURDAY},
      {WEEKDAY_SUNDAY,               SID_SUNDAY}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    WeekdayHeadlineState::WeekdayHeadlineState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mWeekdayHeadlineStatesStringIds;
      mStringIdCount = sizeof( mWeekdayHeadlineStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    WeekdayHeadlineState::~WeekdayHeadlineState()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * protected GetStateStringId made public 
    *****************************************************************************/    
    U16 WeekdayHeadlineState::GetStateStringId(int state)
    {
      return State::GetStateStringId(state);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * returns display number used by slip point control to set dynamic menu title
    *****************************************************************************/    
    void WeekdayHeadlineState::GetStateDisplayNumber(int state, char* displayNumber)
    {
      if(state >= WEEKDAY_MONDAY && state <= WEEKDAY_SUNDAY)
      {
        sprintf(displayNumber, "4.3.4.%i", (state + 1)); // Settings->comm->sms schedule
      }
      else 
      {
        FatalErrorOccured("Alarm state OOR error");
      }      
    }


  } // namespace display
} // namespace mpc


