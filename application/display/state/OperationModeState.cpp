/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : OperationModeState                                    */
/*                                                                          */
/* FILE NAME        : OperationModeState.h                                  */
/*                                                                          */
/* CREATED DATE     : 2007-07-20                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
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
#include "OperationModeState.h"
#include <AppTypeDefs.h>

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
    StateStringId mOperationModeStatesStringIds[] = {
      {ACTUAL_OPERATION_MODE_NOT_INSTALLED, SID_NOT_INSTALLED},
      {ACTUAL_OPERATION_MODE_DISABLED, SID_OUT_OF_OPERATION},
      {ACTUAL_OPERATION_MODE_STOPPED, SID_STOPPED},
      {ACTUAL_OPERATION_MODE_STARTED, SID_RUNNING}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    OperationModeState::OperationModeState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mOperationModeStatesStringIds;
      mStringIdCount = sizeof( mOperationModeStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    OperationModeState::~OperationModeState()
    {
    }

  } // namespace display
} // namespace mpc

