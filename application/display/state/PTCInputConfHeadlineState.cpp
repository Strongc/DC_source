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
/* CLASS NAME       : PTCInputConfHeadlineState                             */
/*                                                                          */
/* FILE NAME        : PTCInputConfHeadlineState.cpp                         */
/*                                                                          */
/* CREATED DATE     : 2012-02-16                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a PTCInputConfHeadlineState.   */
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
#include "PTCInputConfHeadlineState.h"

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
    const StateStringId mPTCInputConfHeadlineStatesStringIds[] =
    {
      { 1,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC1_IO351_41_HEADLINE   },
      { 2,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC2_IO351_41_HEADLINE   },
      { 3,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC3_IO351_41_HEADLINE   },
      { 4,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC4_IO351_41_HEADLINE   },
      { 5,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC5_IO351_41_HEADLINE   },
      { 6,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC6_IO351_41_HEADLINE   },
      { 7,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC1_IO351_42_HEADLINE   },
      { 8,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC2_IO351_42_HEADLINE   },
      { 9,   SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC3_IO351_42_HEADLINE   },
      { 10,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC4_IO351_42_HEADLINE   },
      { 11,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC5_IO351_42_HEADLINE   },
      { 12,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC6_IO351_42_HEADLINE   },
      { 13,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC1_IO351_43_HEADLINE   },
      { 14,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC2_IO351_43_HEADLINE   },
      { 15,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC3_IO351_43_HEADLINE   },
      { 16,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC4_IO351_43_HEADLINE   },
      { 17,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC5_IO351_43_HEADLINE   },
      { 18,  SID_FUNCTION_RELATED_TO_PTC_INPUT_PTC6_IO351_43_HEADLINE   },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    PTCInputConfHeadlineState::PTCInputConfHeadlineState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mPTCInputConfHeadlineStatesStringIds;
      mStringIdCount = sizeof(mPTCInputConfHeadlineStatesStringIds) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    PTCInputConfHeadlineState::~PTCInputConfHeadlineState()
    {
    }
    
    void PTCInputConfHeadlineState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




