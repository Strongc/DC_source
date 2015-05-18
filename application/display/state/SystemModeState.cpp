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
/* CLASS NAME       : SystemModeState                                       */
/*                                                                          */
/* FILE NAME        : SystemModeState.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2007-08-20                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SystemModeState.             */
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
#include "SystemModeState.h"
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
    StateStringId mSystemModeStatesStringIds[] = {
    {APPLICATION_MODE_STANDBY,             SID_STANDBY},
    {APPLICATION_MODE_STARTUP_DELAY,       SID_STARTUP_DELAY},
    {APPLICATION_MODE_PUMPING,             SID_PUMPING},
    {APPLICATION_MODE_STOP_DELAY,          SID_STOP_DELAY},
    {APPLICATION_MODE_PUMPING_MAX,         SID_PUMPING_MAX},
    {APPLICATION_MODE_OFF,                 SID_OFF},
    {APPLICATION_MODE_FOAM_DRAINING,       SID_FOAM_DRAINING},
    {APPLICATION_MODE_DAILY_EMPTYING,      SID_DAILY_EMPTYING},
    {APPLICATION_MODE_MAINS_FAILURE,       SID_MAINS_FAILURE},
    {APPLICATION_MODE_MANUAL_CONTROL,      SID_MANUAL_CONTROL},
    {APPLICATION_MODE_ALL_PUMP_ALARMS,     SID_ALL_PUMP_ALARMS},
    {APPLICATION_MODE_ANTISEIZING,         SID_ANTISEIZING},
    {APPLICATION_MODE_LEVEL_SENSOR_ERROR,  SID_LEVEL_SENSOR_ERROR},
    {APPLICATION_MODE_INTERLOCKED,         SID_INTERLOCKED},
    {APPLICATION_MODE_ALL_PUMPS_OUT_OF_OPERATION, SID_OUT_OF_OPERATION}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    SystemModeState::SystemModeState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mSystemModeStatesStringIds;
      mStringIdCount = sizeof( mSystemModeStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SystemModeState::~SystemModeState()
    {
    }

  } // namespace display
} // namespace mpc


