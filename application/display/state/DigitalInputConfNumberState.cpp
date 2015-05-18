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
/* CLASS NAME       : DigitalInputConfNumberState                           */
/*                                                                          */
/* FILE NAME        : DigitalInputConfNumberState.cpp                       */
/*                                                                          */
/* CREATED DATE     : 2005-06-22                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DigitalInputConfNumberState. */
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
#include "DigitalInputConfNumberState.h"

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
    const StateStringId mDigitalInputConfNumberStatesStringIds[] =
    {
      { 0,   SID_NOT_USED       },// "not configured"
      { 1,   SID_DI1_CU351      },
      { 2,   SID_DI2_CU351      },
      { 3,   SID_DI3_CU351      },
      { 4,   SID_DI1_IO351_41   },
      { 5,   SID_DI2_IO351_41   },
      { 6,   SID_DI3_IO351_41   },
      { 7,   SID_DI4_IO351_41   },
      { 8,   SID_DI5_IO351_41   },
      { 9,   SID_DI6_IO351_41   },
      { 10,  SID_DI7_IO351_41   },
      { 11,  SID_DI8_IO351_41   },
      { 12,  SID_DI9_IO351_41   },
      { 13,  SID_DI1_IO351_42   },
      { 14,  SID_DI2_IO351_42   },
      { 15,  SID_DI3_IO351_42   },
      { 16,  SID_DI4_IO351_42   },
      { 17,  SID_DI5_IO351_42   },
      { 18,  SID_DI6_IO351_42   },
      { 19,  SID_DI7_IO351_42   },
      { 20,  SID_DI8_IO351_42   },
      { 21,  SID_DI9_IO351_42   },
      { 22,  SID_DI1_IO351_43   },
      { 23,  SID_DI2_IO351_43   },
      { 24,  SID_DI3_IO351_43   },
      { 25,  SID_DI4_IO351_43   },
      { 26,  SID_DI5_IO351_43   },
      { 27,  SID_DI6_IO351_43   },
      { 28,  SID_DI7_IO351_43   },
      { 29,  SID_DI8_IO351_43   },
      { 30,  SID_DI9_IO351_43   },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DigitalInputConfNumberState::DigitalInputConfNumberState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mDigitalInputConfNumberStatesStringIds;
      mStringIdCount = sizeof( mDigitalInputConfNumberStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DigitalInputConfNumberState::~DigitalInputConfNumberState()
    {
    }
    
    void DigitalInputConfNumberState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




