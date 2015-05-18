/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange Controller                           */
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
/* CLASS NAME       : AlarmSourceState                                      */
/*                                                                          */
/* FILE NAME        : AlarmSourceState.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 2007-09-28                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmSourceState.            */
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
#include "AlarmSourceState.h"
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
    StateStringId mAlarmSourceStatesStringIds[] = {
      {ALARM_NOT_USED,            SID_NOT_USED            },
      {ALARM_HIGH_LEVEL,          SID_HIGH_LEVEL          },
      {ALARM_ALARM_LEVEL,         SID_SYS_ALARM_LEVEL     },
      {ALARM_OVERFLOW,            SID_OVERFLOW            },
      {ALARM_ALL_PUMPS_IN_ALARM,  SID_ALL_PUMPS_IN_ALARM  },
      {ALARM_PUMP_1_IN_ALARM,     SID_PUMP_1_IN_ALARM     },
      {ALARM_PUMP_2_IN_ALARM,     SID_PUMP_2_IN_ALARM     },
      {ALARM_PUMP_3_IN_ALARM,     SID_PUMP_3_IN_ALARM     },
      {ALARM_PUMP_4_IN_ALARM,     SID_PUMP_4_IN_ALARM     },
      {ALARM_PUMP_5_IN_ALARM,     SID_PUMP_5_IN_ALARM     },
      {ALARM_PUMP_6_IN_ALARM,     SID_PUMP_6_IN_ALARM     },
      {ALARM_GENI_ERROR_PUMP_1,   SID_GENI_ERROR_PUMP_1   },
      {ALARM_GENI_ERROR_PUMP_2,   SID_GENI_ERROR_PUMP_2   },
      {ALARM_GENI_ERROR_PUMP_3,   SID_GENI_ERROR_PUMP_3   },
      {ALARM_GENI_ERROR_PUMP_4,   SID_GENI_ERROR_PUMP_4   },
      {ALARM_GENI_ERROR_PUMP_5,   SID_GENI_ERROR_PUMP_5   },
      {ALARM_GENI_ERROR_PUMP_6,   SID_GENI_ERROR_PUMP_6   },
      {ALARM_GENI_ERROR_ANY_PUMP, SID_GENI_ERROR_ANY_PUMP },
      {ALARM_ANY_PUMP_IN_ALARM,   SID_ANY_PUMP_IN_ALARM   },
      {ALARM_USER_IO_1,           SID_USERDEFINED_FUNCTION_1 },
      {ALARM_USER_IO_2,           SID_USERDEFINED_FUNCTION_2 },
      {ALARM_USER_IO_3,           SID_USERDEFINED_FUNCTION_3 },
      {ALARM_USER_IO_4,           SID_USERDEFINED_FUNCTION_4 },
      {ALARM_USER_IO_5,           SID_USERDEFINED_FUNCTION_5 },
      {ALARM_USER_IO_6,           SID_USERDEFINED_FUNCTION_6 },
      {ALARM_USER_IO_7,           SID_USERDEFINED_FUNCTION_7 },
      {ALARM_USER_IO_8,           SID_USERDEFINED_FUNCTION_8 }
    };

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmSourceState::AlarmSourceState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mAlarmSourceStatesStringIds;
      mStringIdCount = sizeof( mAlarmSourceStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmSourceState::~AlarmSourceState()
    {
    }

  } // namespace display
} // namespace mpc


