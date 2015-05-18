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
/* CLASS NAME       : SIMCardState                                          */
/*                                                                          */
/* FILE NAME        : SIMCardState.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2008-01-11                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SIMCardState.                */
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
#include "SIMCardState.h"

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
    const StateStringId mSIMCardStatesStringIds[] =
    {
      { SIM_CARD_STATUS_READY,          SID_READY             },
      { SIM_CARD_STATUS_PIN_INVALID,    SID_PIN_CODE_INVALID  },
      { SIM_CARD_STATUS_PUK_INVALID,    SID_PUK_CODE_INVALID  },
      { SIM_CARD_STATUS_SERVICE_CENTER_NO_MISSING,    SID_SERVICE_CENTER_NO_MISSING},
      { SIM_CARD_STATUS_SIM_NOT_PRESENT, SID_INSERT_SIM       },
      { SIM_CARD_STATUS_DEFECT,         SID_DEFECT            },
      { SIM_CARD_STATUS_WRONG_TYPE,     SID_INVALID_SIM       },
      { SIM_CARD_STATUS_BUSY,           SID_SIM_BUSY          },
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    SIMCardState::SIMCardState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mSIMCardStatesStringIds;
      mStringIdCount = sizeof( mSIMCardStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SIMCardState::~SIMCardState()
    {
    }
    
    void SIMCardState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




