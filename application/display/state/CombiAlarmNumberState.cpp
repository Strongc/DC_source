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
/* CLASS NAME       : CombiAlarmNumberState                                 */
/*                                                                          */
/* FILE NAME        : CombiAlarmNumberState.cpp                             */
/*                                                                          */
/* CREATED DATE     : 2007-10-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a CombiAlarmNumberState.       */
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
#include "CombiAlarmNumberState.h"

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
    const StateStringId mCombiAlarmNumberStatesStringIds[] =
    {
      { 1,   SID_COMBI_ALARM_1_SOURCE_1   },
      { 2,   SID_COMBI_ALARM_1_SOURCE_2   },
      { 3,   SID_COMBI_ALARM_2_SOURCE_1   },
      { 4,   SID_COMBI_ALARM_2_SOURCE_2   },
      { 5,   SID_COMBI_ALARM_3_SOURCE_1   },
      { 6,   SID_COMBI_ALARM_3_SOURCE_2   },
      { 7,   SID_COMBI_ALARM_4_SOURCE_1   },
      { 8,   SID_COMBI_ALARM_4_SOURCE_2   },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    CombiAlarmNumberState::CombiAlarmNumberState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mCombiAlarmNumberStatesStringIds;
      mStringIdCount = sizeof( mCombiAlarmNumberStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    CombiAlarmNumberState::~CombiAlarmNumberState()
    {
    }
    
    void CombiAlarmNumberState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




