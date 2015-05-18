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
/* CLASS NAME       : AnalogOutputConfHeadlineState                         */
/*                                                                          */
/* FILE NAME        : AnalogOutputConfHeadlineState.cpp                     */
/*                                                                          */
/* CREATED DATE     : 2009-05-19                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AnalogOutputConfHeadlineState.*/
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
#include "AnalogOutputConfHeadlineState.h"

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
    const StateStringId mAnalogOutputConfHeadlineStatesStringIds[] =
    {
      { 1,   SID_FUNCTION_AO1_IO351_41   },
      { 2,   SID_FUNCTION_AO2_IO351_41   },
      { 3,   SID_FUNCTION_AO3_IO351_41   },
      { 4,   SID_FUNCTION_AO1_IO351_42   },
      { 5,   SID_FUNCTION_AO2_IO351_42   },
      { 6,   SID_FUNCTION_AO3_IO351_42   },
      { 7,   SID_FUNCTION_AO1_IO351_43   },
      { 8,   SID_FUNCTION_AO2_IO351_43   },
      { 9,   SID_FUNCTION_AO3_IO351_43   }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AnalogOutputConfHeadlineState::AnalogOutputConfHeadlineState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mAnalogOutputConfHeadlineStatesStringIds;
      mStringIdCount = sizeof( mAnalogOutputConfHeadlineStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AnalogOutputConfHeadlineState::~AnalogOutputConfHeadlineState()
    {
    }
    
    void AnalogOutputConfHeadlineState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




