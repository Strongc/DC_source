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
/* CLASS NAME       : OverflowMonthState                                    */
/*                                                                          */
/* FILE NAME        : OverflowMonthState.cpp                                */
/*                                                                          */
/* CREATED DATE     : 2008-10-16                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a OverflowMonthState.          */
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
#include "OverflowMonthState.h"

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
    const StateStringId mOverflowMonthStatesStringIds[] =
    {
      { OVERFLOW_MONTH_JAN,   SID_OVERFLOW_JANUARY    },
      { OVERFLOW_MONTH_FEB,   SID_OVERFLOW_FEBRUARY   },
      { OVERFLOW_MONTH_MAR,   SID_OVERFLOW_MARCH      },
      { OVERFLOW_MONTH_APR,   SID_OVERFLOW_APRIL      },
      { OVERFLOW_MONTH_MAY,   SID_OVERFLOW_MAY        },
      { OVERFLOW_MONTH_JUN,   SID_OVERFLOW_JUNE       },
      { OVERFLOW_MONTH_JUL,   SID_OVERFLOW_JULY       },
      { OVERFLOW_MONTH_AUG,   SID_OVERFLOW_AUGUST     },
      { OVERFLOW_MONTH_SEP,   SID_OVERFLOW_SEPTEMBER  },
      { OVERFLOW_MONTH_OCT,   SID_OVERFLOW_OCTOBER    },
      { OVERFLOW_MONTH_NOV,   SID_OVERFLOW_NOVEMBER   },
      { OVERFLOW_MONTH_DEC,   SID_OVERFLOW_DECEMBER   }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OverflowMonthState::OverflowMonthState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mOverflowMonthStatesStringIds;
      mStringIdCount = sizeof( mOverflowMonthStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OverflowMonthState::~OverflowMonthState()
    {
    }
    
    void OverflowMonthState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




