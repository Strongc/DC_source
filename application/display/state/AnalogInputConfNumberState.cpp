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
/* CLASS NAME       : AnalogInputConfNumberState                            */
/*                                                                          */
/* FILE NAME        : AnalogInputConfNumberState.cpp                        */
/*                                                                          */
/* CREATED DATE     : 2005-06-22                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AnalogInputConfNumberState.  */
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
#include "AnalogInputConfNumberState.h"
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
    StateStringId mAnalogInputConfNumberStatesStringIds[] =
    {
      {	1,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI1_CU351_HEADLINE			},
      {	2,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI2_CU351_HEADLINE			},
      {	3,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI3_CU351_HEADLINE			},
      {	4,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI1_IO351_41_HEADLINE		},
      {	5,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI2_IO351_41_HEADLINE		},
      {	6,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI1_IO351_42_HEADLINE		},
      {	7,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI2_IO351_42_HEADLINE   },
      {	8,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI1_IO351_43_HEADLINE		},
      {	9,	 SID_FUNCTION_RELATED_TO_ANALOG_INPUT_AI2_IO351_43_HEADLINE   },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AnalogInputConfNumberState::AnalogInputConfNumberState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mAnalogInputConfNumberStatesStringIds;
      mStringIdCount = sizeof( mAnalogInputConfNumberStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AnalogInputConfNumberState::~AnalogInputConfNumberState()
    {
    }

  } // namespace display
} // namespace mpc




