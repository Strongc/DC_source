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
/* CLASS NAME       : DigitalInputFunctionState                             */
/*                                                                          */
/* FILE NAME        : DigitalInputFunctionState.cpp                         */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a DigitalInputFunctionState.   */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "IIntegerDataPoint.h"

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DataPoint.h"
#include "DigitalInputFunctionState.h"
#include <DiFuncHandler.h>
#include "StateBracket.h"
#include "DataPointText.h"

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
    StateStringId mDigitalInputFunctionStatesStringIds[] =
    {
      { DIGITAL_INPUT_FUNC_NO_FUNCTION                   , SID_DI_NOT_USED                      },
      { DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1                , SID_FLOAT_SWITCH_1                   },
      { DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2                , SID_FLOAT_SWITCH_2                   },
      { DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3                , SID_FLOAT_SWITCH_3                   },
      { DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4                , SID_FLOAT_SWITCH_4                   },
      { DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5                , SID_FLOAT_SWITCH_5                   },
      { DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_1               , SID_DI_AUTO_MAN_PUMP_1               },
      { DIGITAL_INPUT_FUNC_ON_OFF_PUMP_1                 , SID_DI_MAN_START_PUMP_1              },
      { DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2               , SID_DI_AUTO_MAN_PUMP_2               },
      { DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2                 , SID_DI_MAN_START_PUMP_2              },
      { DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3               , SID_AUTOMATIC_MANUAL_PUMP_3          },
      { DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3                 , SID_MANUAL_START_PUMP_3              },
      { DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4               , SID_AUTOMATIC_MANUAL_PUMP_4          },
      { DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4                 , SID_MANUAL_START_PUMP_4              },
      { DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5               , SID_AUTOMATIC_MANUAL_PUMP_5          },
      { DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5                 , SID_MANUAL_START_PUMP_5              },
      { DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6               , SID_AUTOMATIC_MANUAL_PUMP_6          },
      { DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6                 , SID_MANUAL_START_PUMP_6              },
      { DIGITAL_INPUT_FUNC_EXTERNAL_FAULT                , SID_DI_EXTERNAL_FAULT                },
      { DIGITAL_INPUT_FUNC_ALARM_RESET                   , SID_DI_ALARM_RESET                   },
      { DIGITAL_INPUT_FUNC_ALARM_RELAY_RESET             , SID_DI_ALARM_RELAY_RESET             },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_1     , SID_DI_CONTACTOR_FEEDBACK_PUMP_1     },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2     , SID_DI_CONTACTOR_FEEDBACK_PUMP_2     },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3     , SID_CONTACTOR_FEEDBACK_PUMP_3        },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4     , SID_CONTACTOR_FEEDBACK_PUMP_4        },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5     , SID_CONTACTOR_FEEDBACK_PUMP_5        },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6     , SID_CONTACTOR_FEEDBACK_PUMP_6        },
      { DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER      , SID_DI_CONTACTOR_FEEDBACK_MIXER      },
      { DIGITAL_INPUT_FUNC_FAIL_PHASE                    , SID_DI_FAIL_PHASE                    },
      { DIGITAL_INPUT_FUNC_ENERGY_CNT                    , SID_DI_ENERGY_COUNTER                },
      { DIGITAL_INPUT_FUNC_VOLUME_CNT                    , SID_DI_VOLUME_COUNTER                },
      { DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_1       , SID_DI_MOTOR_PROTECTION_PUMP_1       },
      { DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2       , SID_DI_MOTOR_PROTECTION_PUMP_2       },
      { DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3       , SID_MOTOR_PROTECTION_PUMP_3          },
      { DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4       , SID_MOTOR_PROTECTION_PUMP_4          },
      { DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5       , SID_MOTOR_PROTECTION_PUMP_5          },
      { DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6       , SID_MOTOR_PROTECTION_PUMP_6          },
      { DIGITAL_INPUT_FUNC_ANTI_SEIZING                  , SID_ANTISEIZING                      },
      { DIGITAL_INPUT_FUNC_DAILY_EMPTYING                , SID_DAILY_EMPTYING                   },
      { DIGITAL_INPUT_FUNC_FOAM_DRAINING                 , SID_FOAM_DRAINING                    },
      { DIGITAL_INPUT_FUNC_INTERLOCK                     , SID_DI_INTERLOCK                     },
      { DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1              , SID_DI_VFD_READY_PUMP_1              },
      { DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2              , SID_DI_VFD_READY_PUMP_2              },
      { DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3              , SID_DI_VFD_READY_PUMP_3              },
      { DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4              , SID_DI_VFD_READY_PUMP_4              },
      { DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5              , SID_DI_VFD_READY_PUMP_5              },
      { DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6              , SID_DI_VFD_READY_PUMP_6              },
      { DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH               , SID_DI_OVERFLOW_SWITCH               },      
      { DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1              , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2              , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3              , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_GAS_DETECTOR                  , SID_DI_GAS_DETECTOR                  },
      { DIGITAL_INPUT_FUNC_WATER_ON_PIT_FLOOR            , SID_DI_WATER_ON_PIT_FLOOR            },
      { DIGITAL_INPUT_FUNC_SERVICE_MODE                  , SID_DI_SERVICE_MODE                  },
      { DIGITAL_INPUT_FUNC_EXTRA_FAULT_1                 , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_EXTRA_FAULT_2                 , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_EXTRA_FAULT_3                 , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_EXTRA_FAULT_4                 , SID_NONE                             }, // Uses user-defined string
      { DIGITAL_INPUT_FUNC_DOSING_PUMP                   , SID_DOSING_PUMP                      }
      
    };


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DigitalInputFunctionState::DigitalInputFunctionState(Component* pParent) : StateBracket(pParent)
    {
      mpStateStringIds = mDigitalInputFunctionStatesStringIds;
      mStringIdCount = sizeof( mDigitalInputFunctionStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DigitalInputFunctionState::~DigitalInputFunctionState()
    {
    }
    
    /*****************************************************************************
    * Function...: ConnectToSubjects
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void DigitalInputFunctionState::ConnectToSubjects(void)
    {
      StateBracket::ConnectToSubjects();

      mDpNameOfExtraFault[0].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_1));
      mDpNameOfExtraFault[1].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_2));
      mDpNameOfExtraFault[2].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_3));
      mDpNameOfExtraFault[3].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_4));
      mDpNameOfUsdCounter[0].Attach(::GetSubject(SUBJECT_ID_DIGI_INPUT_USD_COUNTER_NAME_IO_1));
      mDpNameOfUsdCounter[1].Attach(::GetSubject(SUBJECT_ID_DIGI_INPUT_USD_COUNTER_NAME_IO_2));
      mDpNameOfUsdCounter[2].Attach(::GetSubject(SUBJECT_ID_DIGI_INPUT_USD_COUNTER_NAME_IO_3));

      for (int i = 0; i < NO_OF_EXTRA_FAULTS; i++)
      {
        mDpNameOfExtraFault[i].Subscribe(this);
      }

      for (int i = 0; i < NO_OF_USD_COUNTERS; i++)
      {
        mDpNameOfUsdCounter[i].Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function - GetStateAsString
    * DESCRIPTION:
    * 
    *****************************************************************************/
    const char* DigitalInputFunctionState::GetStateAsString(int state)
    {
      if (!mDpNameOfExtraFault[0].IsValid())
      {
        ConnectToSubjects();
      }

      if (state >= DIGITAL_INPUT_FUNC_EXTRA_FAULT_1 && state <= DIGITAL_INPUT_FUNC_EXTRA_FAULT_4)
      {
        int extra = state - (int)DIGITAL_INPUT_FUNC_EXTRA_FAULT_1;
        
        return mDpNameOfExtraFault[extra]->GetValue();
      }
      else if(state >= DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 && state <= DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3)
      {
        int extra = state - (int)DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1;
        
        return mDpNameOfUsdCounter[extra]->GetValue();
      }
      else
      {
        return Languages::GetInstance()->GetString(GetStateStringId(state));
      }
    }


  } // namespace display
} // namespace mpc





