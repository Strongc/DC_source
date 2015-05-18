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
/* CLASS NAME       : DigitalInputConfHeadlineState                         */
/*                                                                          */
/* FILE NAME        : DigitalInputConfHeadlineState.cpp                     */
/*                                                                          */
/* CREATED DATE     : 2005-06-22                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DigitalInputConfHeadlineState.*/
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
#include "DigitalInputConfHeadlineState.h"

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
    const StateStringId mDigitalInputConfHeadlineStatesStringIds[] =
    {
      { 1,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI1_CU351_HEADLINE      },
      { 2,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI2_CU351_HEADLINE      },
      { 3,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI3_CU351_HEADLINE      },
      { 4,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI1_IO351_41_HEADLINE   },
      { 5,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI2_IO351_41_HEADLINE   },
      { 6,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI3_IO351_41_HEADLINE   },
      { 7,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI4_IO351_41_HEADLINE   },
      { 8,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI5_IO351_41_HEADLINE   },
      { 9,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI6_IO351_41_HEADLINE   },
      { 10,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI7_IO351_41_HEADLINE   },
      { 11,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI8_IO351_41_HEADLINE   },
      { 12,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI9_IO351_41_HEADLINE   },
      { 13,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI1_IO351_42_HEADLINE   },
      { 14,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI2_IO351_42_HEADLINE   },
      { 15,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI3_IO351_42_HEADLINE   },
      { 16,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI4_IO351_42_HEADLINE  },
      { 17,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI5_IO351_42_HEADLINE  },
      { 18,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI6_IO351_42_HEADLINE  },
      { 19,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI7_IO351_42_HEADLINE  },
      { 20,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI8_IO351_42_HEADLINE  },
      { 21,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI9_IO351_42_HEADLINE  },
      { 22,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI1_IO351_43_HEADLINE   },
      { 23,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI2_IO351_43_HEADLINE   },
      { 24,  SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI3_IO351_43_HEADLINE   },
      { 25,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI4_IO351_43_HEADLINE  },
      { 26,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI5_IO351_43_HEADLINE  },
      { 27,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI6_IO351_43_HEADLINE  },
      { 28,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI7_IO351_43_HEADLINE  },
      { 29,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI8_IO351_43_HEADLINE  },
      { 30,   SID_FUNCTION_RELATED_TO_DIGITAL_INPUT_DI9_IO351_43_HEADLINE  },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DigitalInputConfHeadlineState::DigitalInputConfHeadlineState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mDigitalInputConfHeadlineStatesStringIds;
      mStringIdCount = sizeof( mDigitalInputConfHeadlineStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DigitalInputConfHeadlineState::~DigitalInputConfHeadlineState()
    {
    }
    
    void DigitalInputConfHeadlineState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




