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
/* CLASS NAME       : SensorElectricalMinState                              */
/*                                                                          */
/* FILE NAME        : SensorElectricalMaxState.cpp                          */
/*                                                                          */
/* CREATED DATE     : 2005-03-15                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a SensorElectricalMaxState.    */
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
#include "SensorElectricalMinState.h"
#include <AppTypeDefs.h>
#include <AnaInOnIOCtrl.H>

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
    StateStringId mSensorElectricalMinStatesStringIds[] = {
	{SENSOR_ELECTRIC_TYPE_0_20mA, SID_0_mA},
	{SENSOR_ELECTRIC_TYPE_4_20mA,SID_4_mA},
	{SENSOR_ELECTRIC_TYPE_0_10V,SID_0_V}
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    SensorElectricalMinState::SensorElectricalMinState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mSensorElectricalMinStatesStringIds;
      mStringIdCount = sizeof( mSensorElectricalMinStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    SensorElectricalMinState::~SensorElectricalMinState()
    {
    }

  } // namespace display
} // namespace mpc


