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
/* CLASS NAME       : ControlSourceState                                    */
/*                                                                          */
/* FILE NAME        : ControlSourceState.cpp                                */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a ControlSourceState.          */
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
#include "ControlSourceState.h"
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
    StateStringId mControlSourceStatesStringIds[] = {
      {CONTROL_SOURCE_HMI,          SID_BY_CU351},
      {CONTROL_SOURCE_SYSTEM,       SID_BY_SYSTEM},
      {CONTROL_SOURCE_DI,           SID_BY_ON_OFF_AUTO_SWITCH},
      {CONTROL_SOURCE_SCADA,        SID_BY_SCADA},
      {CONTROL_SOURCE_SERVICE,      SID_BY_SERVICE},
      
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    ControlSourceState::ControlSourceState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mControlSourceStatesStringIds;
      mStringIdCount = sizeof( mControlSourceStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ControlSourceState::~ControlSourceState()
    {
    }

  } // namespace display
} // namespace mpc

