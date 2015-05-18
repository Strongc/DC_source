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
/* CLASS NAME       : SignalState                                           */
/*                                                                          */
/* FILE NAME        : SignalState.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 2008-01-11                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SignalState.                  */
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
#include "SignalState.h"

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
    const StateStringId mSignalStatesStringIds[] =
    {
      { GSM_SIGNAL_LEVEL_UNKNOWN,   SID_DASH           },
      { GSM_SIGNAL_LEVEL_NO_SIGNAL, SID_NO_SIGNAL      },
      { GSM_SIGNAL_LEVEL_0,         SID_SIGNAL_0       },
      { GSM_SIGNAL_LEVEL_1,         SID_SIGNAL_1       },
      { GSM_SIGNAL_LEVEL_2,         SID_SIGNAL_2       },
      { GSM_SIGNAL_LEVEL_3,         SID_SIGNAL_3       },
      { GSM_SIGNAL_LEVEL_4,         SID_SIGNAL_4       },
      { GSM_SIGNAL_LEVEL_5,         SID_SIGNAL_5       },
      
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    SignalState::SignalState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mSignalStatesStringIds;
      mStringIdCount = sizeof( mSignalStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SignalState::~SignalState()
    {
    }
    
    void SignalState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




