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
/* CLASS NAME       : PTCInputFunctionState                                */
/*                                                                          */
/* FILE NAME        : PTCInputFunctionState.cpp                            */
/*                                                                          */
/* CREATED DATE     : 2012-15-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a PTCInputFunctionState.       */
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
#include "PTCInputFunctionState.h"

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
    const StateStringId mPTCInputFunctionStatesStringIds[] =
    {
      { PTC_INPUT_FUNC_NO_FUNCTION         , SID_PTC_NOT_USED           },
      { PTC_INPUT_FUNC_PTC_1_PUMP_1        , SID_PTC_1_PUMP_1           },
      { PTC_INPUT_FUNC_PTC_2_PUMP_1        , SID_PTC_2_PUMP_1           },
      { PTC_INPUT_FUNC_MOISTURE_PUMP_1     , SID_PTC_MOISTURE_PUMP_1    },
      { PTC_INPUT_FUNC_PTC_1_PUMP_2        , SID_PTC_1_PUMP_2           },
      { PTC_INPUT_FUNC_PTC_2_PUMP_2        , SID_PTC_2_PUMP_2           },
      { PTC_INPUT_FUNC_MOISTURE_PUMP_2     , SID_PTC_MOISTURE_PUMP_2    },
      { PTC_INPUT_FUNC_PTC_1_PUMP_3        , SID_PTC_1_PUMP_3           },
      { PTC_INPUT_FUNC_PTC_2_PUMP_3        , SID_PTC_2_PUMP_3           },
      { PTC_INPUT_FUNC_MOISTURE_PUMP_3     , SID_PTC_MOISTURE_PUMP_3    },
      { PTC_INPUT_FUNC_PTC_1_PUMP_4        , SID_PTC_1_PUMP_4           },
      { PTC_INPUT_FUNC_PTC_2_PUMP_4        , SID_PTC_2_PUMP_4           },
      { PTC_INPUT_FUNC_MOISTURE_PUMP_4     , SID_PTC_MOISTURE_PUMP_4    },      
      { PTC_INPUT_FUNC_PTC_1_PUMP_5        , SID_PTC_1_PUMP_5           },
      { PTC_INPUT_FUNC_PTC_2_PUMP_5        , SID_PTC_2_PUMP_5           },
      { PTC_INPUT_FUNC_MOISTURE_PUMP_5     , SID_PTC_MOISTURE_PUMP_5    },
      { PTC_INPUT_FUNC_PTC_1_PUMP_6        , SID_PTC_1_PUMP_6           },
      { PTC_INPUT_FUNC_PTC_2_PUMP_6        , SID_PTC_2_PUMP_6           },
      { PTC_INPUT_FUNC_MOISTURE_PUMP_6     , SID_PTC_MOISTURE_PUMP_6    },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    PTCInputFunctionState::PTCInputFunctionState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mPTCInputFunctionStatesStringIds;
      mStringIdCount = sizeof(mPTCInputFunctionStatesStringIds) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    PTCInputFunctionState::~PTCInputFunctionState()
    {
    }
    
    void PTCInputFunctionState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




