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
/* CLASS NAME       : VfdOperationModeState                                 */
/*                                                                          */
/* FILE NAME        : VfdOperationModeState.cpp                             */
/*                                                                          */
/* CREATED DATE     : 2009-04-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a VfdOperationModeState.       */
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
#include "VfdOperationModeState.h"

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
    const StateStringId mVfdOperationModeStatesStringIds[] =
    {
      { VFD_OPERATION_MODE_NOT_IN_CONTROL, SID_VFD_STATE_NOT_IN_CTRL  },
      { VFD_OPERATION_MODE_INIT,           SID_VFD_STATE_INIT         },
      { VFD_OPERATION_MODE_STOP,           SID_VFD_STATE_STOP         },
      { VFD_OPERATION_MODE_REV_FLUSH,      SID_VFD_STATE_REV_FLUSH    },
      { VFD_OPERATION_MODE_START_FLUSH,    SID_VFD_STATE_START_FLUSH  },
      { VFD_OPERATION_MODE_NORMAL,         SID_VFD_STATE_NORMAL       },
      { VFD_OPERATION_MODE_RUN_FLUSH,      SID_VFD_STATE_RUN_FLUSH    },
      { VFD_OPERATION_MODE_STOP_FLUSH,     SID_VFD_STATE_STOP_FLUSH   }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    VfdOperationModeState::VfdOperationModeState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mVfdOperationModeStatesStringIds;
      mStringIdCount = sizeof( mVfdOperationModeStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    VfdOperationModeState::~VfdOperationModeState()
    {
    }
    
    void VfdOperationModeState::SetSubjectPointer(int Id, Subject* pSubject)
    {
      State::SetSubjectPointer(Id, pSubject);
    }


  } // namespace display
} // namespace mpc




