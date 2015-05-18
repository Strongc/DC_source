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
/* CLASS NAME       : DigitalOutputConfHeadlineState                        */
/*                                                                          */
/* FILE NAME        : DigitalOutputConfHeadlineState.cpp                    */
/*                                                                          */
/* CREATED DATE     : 2005-06-22                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DigitalOutputConfHeadlineState.*/
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
#include "DataPoint.h"
#include "DigitalOutputConfHeadlineState.h"
#include <LanguagesStringTable.h>

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
    StateStringId mDigitalOutputConfHeadlineStatesStringIds[] =
    {
      { 1,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO1_CU351_HEADLINE      },
      { 2,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO2_CU351_HEADLINE      },
      { 3,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO1_IO351_41_HEADLINE   },
      { 4,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO2_IO351_41_HEADLINE   },
      { 5,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO3_IO351_41_HEADLINE   },
      { 6,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO4_IO351_41_HEADLINE   },
      { 7,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO5_IO351_41_HEADLINE   },
      { 8,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO6_IO351_41_HEADLINE   },
      { 9,   SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO7_IO351_41_HEADLINE   },
      { 10,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO1_IO351_42_HEADLINE   },
      { 11,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO2_IO351_42_HEADLINE   },
      { 12,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO3_IO351_42_HEADLINE   },
      { 13,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO4_IO351_42_HEADLINE   },
      { 14,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO5_IO351_42_HEADLINE   },
      { 15,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO6_IO351_42_HEADLINE   },
      { 16,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO7_IO351_42_HEADLINE   },
      { 17,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO1_IO351_43_HEADLINE   },
      { 18,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO2_IO351_43_HEADLINE   },
      { 19,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO3_IO351_43_HEADLINE   },
      { 20,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO4_IO351_43_HEADLINE   },
      { 21,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO5_IO351_43_HEADLINE   },
      { 22,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO6_IO351_43_HEADLINE   },
      { 23,  SID_FUNCTION_RELATED_TO_DIGITAL_OUTPUT_DO7_IO351_43_HEADLINE   },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DigitalOutputConfHeadlineState::DigitalOutputConfHeadlineState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mDigitalOutputConfHeadlineStatesStringIds;
      mStringIdCount = sizeof( mDigitalOutputConfHeadlineStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DigitalOutputConfHeadlineState::~DigitalOutputConfHeadlineState()
    {
    }

  } // namespace display
} // namespace mpc




