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
/* CLASS NAME       : AnalogInputFunctionState                              */
/*                                                                          */
/* FILE NAME        : AnalogInputFunctionState.cpp                          */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AnalogInputFunctionState.    */
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
#include <AnaInOnIoCtrl.h>
#include "AnalogInputFunctionState.h"


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
    StateStringId mAnalogInputFunctionStatesStringIds[] =
    {
      { MEASURED_VALUE_NOT_SELECTED,              SID_AI_NOT_USED_DISABLED                  },
      { MEASURED_VALUE_FLOW,                      SID_AI_FLOW                               },
      { MEASURED_VALUE_LEVEL_ULTRA_SOUND,         SID_AI_LEVEL_ULTRA_SONIC                  },
      { MEASURED_VALUE_LEVEL_PRESSURE,            SID_AI_LEVEL_PRESSURE                     },
      { MEASURED_VALUE_OUTLET_PRESSURE,           SID_AI_PRESSURE_SENSOR_OUTLET_PIPELINE    },
      { MEASURED_VALUE_MOTOR_CURRENT_PUMP_1,      SID_AI_MOTOR_CURRENT_PUMP_1               },
      { MEASURED_VALUE_MOTOR_CURRENT_PUMP_2,      SID_AI_MOTOR_CURRENT_PUMP_2               },
      { MEASURED_VALUE_MOTOR_CURRENT_PUMP_3,      SID_MOTOR_CURRENT_PUMP_3                  },
      { MEASURED_VALUE_MOTOR_CURRENT_PUMP_4,      SID_MOTOR_CURRENT_PUMP_4                  },
      { MEASURED_VALUE_MOTOR_CURRENT_PUMP_5,      SID_MOTOR_CURRENT_PUMP_5                  },
      { MEASURED_VALUE_MOTOR_CURRENT_PUMP_6,      SID_MOTOR_CURRENT_PUMP_6                  },
      { MEASURED_VALUE_WATER_IN_OIL_PUMP_1,       SID_AI_WATER_IN_OIL_PUMP_1                },
      { MEASURED_VALUE_WATER_IN_OIL_PUMP_2,       SID_AI_WATER_IN_OIL_PUMP_2                },
      { MEASURED_VALUE_WATER_IN_OIL_PUMP_3,       SID_WATER_IN_OIL_PUMP_3                   },
      { MEASURED_VALUE_WATER_IN_OIL_PUMP_4,       SID_WATER_IN_OIL_PUMP_4                   },
      { MEASURED_VALUE_WATER_IN_OIL_PUMP_5,       SID_WATER_IN_OIL_PUMP_5                   },
      { MEASURED_VALUE_WATER_IN_OIL_PUMP_6,       SID_WATER_IN_OIL_PUMP_6                   },
      { MEASURED_VALUE_INSULATION_RESISTANCE,     SID_AI_INSULATION_RESISTANCE              },
      { MEASURED_VALUE_POWER,                     SID_AI_POWER                              },
      { MEASURED_VALUE_POWER_PUMP_1,              SID_AI_POWER_PUMP_1                       },
      { MEASURED_VALUE_POWER_PUMP_2,              SID_AI_POWER_PUMP_2                       },
      { MEASURED_VALUE_POWER_PUMP_3,              SID_AI_POWER_PUMP_3                       },
      { MEASURED_VALUE_POWER_PUMP_4,              SID_AI_POWER_PUMP_4                       },
      { MEASURED_VALUE_POWER_PUMP_5,              SID_AI_POWER_PUMP_5                       },
      { MEASURED_VALUE_POWER_PUMP_6,              SID_AI_POWER_PUMP_6                       },

      { MEASURED_VALUE_USER_DEFINED_SOURCE_1,     SID_AI_OPTIONAL_SENSOR_1                  },
      { MEASURED_VALUE_USER_DEFINED_SOURCE_2,     SID_AI_OPTIONAL_SENSOR_2,                 },
      { MEASURED_VALUE_USER_DEFINED_SOURCE_3,     SID_AI_OPTIONAL_SENSOR_3,                 }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AnalogInputFunctionState::AnalogInputFunctionState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mAnalogInputFunctionStatesStringIds;
      mStringIdCount = sizeof( mAnalogInputFunctionStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AnalogInputFunctionState::~AnalogInputFunctionState()
    {
    }

  } // namespace display
} // namespace mpc




