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
/* CLASS NAME       : DigitalOutputConfNumberState                          */
/*                                                                          */
/* FILE NAME        : DigitalOutputConfNumberState.cpp                      */
/*                                                                          */
/* CREATED DATE     : 2005-06-22                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DigitalOutputConfNumberState.*/
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
#include "DigitalOutputConfNumberState.h"
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
    StateStringId mDigitalOutputConfNumberStatesStringIds[] =
    {
      { 0,   SID_NOT_USED       },// "not configured"
      { 1,   SID_DO1_CU351      },
      { 2,   SID_DO2_CU351      },
      { 3,   SID_DO1_IO351_41   },
      { 4,   SID_DO2_IO351_41   },
      { 5,   SID_DO3_IO351_41   },
      { 6,   SID_DO4_IO351_41   },
      { 7,   SID_DO5_IO351_41   },
      { 8,   SID_DO6_IO351_41   },
      { 9,   SID_DO7_IO351_41   },
      { 10,  SID_DO1_IO351_42   },
      { 11,  SID_DO2_IO351_42   },
      { 12,  SID_DO3_IO351_42   },
      { 13,  SID_DO4_IO351_42   },
      { 14,  SID_DO5_IO351_42   },
      { 15,  SID_DO6_IO351_42   },
      { 16,  SID_DO7_IO351_42   },
      { 17,  SID_DO1_IO351_43   },
      { 18,  SID_DO2_IO351_43   },
      { 19,  SID_DO3_IO351_43   },
      { 20,  SID_DO4_IO351_43   },
      { 21,  SID_DO5_IO351_43   },
      { 22,  SID_DO6_IO351_43   },
      { 23,  SID_DO7_IO351_43   },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DigitalOutputConfNumberState::DigitalOutputConfNumberState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mDigitalOutputConfNumberStatesStringIds;
      mStringIdCount = sizeof( mDigitalOutputConfNumberStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DigitalOutputConfNumberState::~DigitalOutputConfNumberState()
    {
    }

  } // namespace display
} // namespace mpc




