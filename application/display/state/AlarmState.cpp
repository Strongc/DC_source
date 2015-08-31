/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : AlarmState                                            */
/*                                                                          */
/* FILE NAME        : AlarmState.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 2007-10-22                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a AlarmState.                  */
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
#include "DataPoint.h"
#include "StringDataPoint.h"
#include "AlarmState.h"
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
    StateStringId mAlarmStatesStringIds[] = {
      {AC_SYS_ALARM_FLOAT_SWITCH,               SID_SYS_ALARM_FLOAT_SWITCH},
      {AC_SYS_ALARM_LEVEL_SENSOR,               SID_SYS_ALARM_LEVEL_SENSOR},
      {AC_SYS_ALARM_FLOW_METER,                 SID_SYS_ALARM_FLOW_METER},
      {AC_SYS_ALARM_POWER_METER,                SID_SYS_ALARM_POWER_METER},
      {AC_SYS_ALARM_HIGH_LEVEL,                 SID_SYS_ALARM_HIGH_LEVEL},
      {AC_SYS_ALARM_LEVEL,                      SID_SYS_ALARM_LEVEL},
      {AC_SYS_ALARM_DRY_RUNNING,                SID_SYS_ALARM_DRY_RUNNING},
      {AC_SYS_ALARM_CONFLICTING_LEVELS,         SID_SYS_ALARM_CONFLICTING_LEVELS},
      {AC_SYS_ALARM_HW_FAULT,                   SID_SYS_ALARM_HARDWARE_FAULT},
      {AC_SYS_ALARM_EXTERNAL_FAULT,             SID_SYS_ALARM_EXTERNAL_FAULT},
      {AC_SYS_ALARM_UPS,                        SID_SYS_ALARM_UPS},
      {AC_SYS_ALARM_COM_ERROR_IO351,            SID_SYS_ALARM_COM_ERROR_IO351},
      {AC_SYS_ALARM_MAINS_VOLTAGE,              SID_SYS_ALARM_MAINS_VOLTAGE},
      {AC_SYS_ALARM_CIU_CARD_FAILURE,           SID_SYS_ALARM_CIU_CARD_FAILURE},
      {AC_SYS_ALARM_SCADA_CALLBACK,             SID_SYS_ALARM_SCADA_CALLBACK_ERROR},
      {AC_SYS_ALARM_FORCED_RELAY_OUTPUT_ENABLED, SID_SYS_ALARM_FORCED_RELAY_OUTPUT_ENABLED},
      {AC_SYS_ALARM_OVERFLOW,                   SID_SYS_ALARM_OVERFLOW},
      {AC_SYS_ALARM_ETHERNET_NO_ADDRESS,        SID_SYS_ALARM_ETHERNET_FAULT},
      {AC_SYS_ALARM_ETHERNET_MISUSED,           SID_SYS_ALARM_ETHERNET_DISABLED_DUE_TO_MISUSE},
      {AC_SYS_ALARM_SIM_CARD,                   SID_SYS_ALARM_SIM_CARD_FAULT},
      {AC_SYS_ALARM_USER_DEFINED_SENSOR_1,      SID_SYS_ALARM_USER_DEFINED_SENSOR_1},
      {AC_SYS_ALARM_USER_DEFINED_SENSOR_2,      SID_SYS_ALARM_USER_DEFINED_SENSOR_2},
      {AC_SYS_ALARM_USER_DEFINED_SENSOR_3,      SID_SYS_ALARM_USER_DEFINED_SENSOR_3},
      {AC_SYS_ALARM_PRESSURE_SENSOR_OUTLET_PIPELINE, SID_SYS_ALARM_PRESSURE_SENSOR_OUTLET_PIPELINE},
      {AC_SYS_ALARM_GAS_DETECTOR,               SID_DI_GAS_DETECTOR},
      {AC_SYS_ALARM_WATER_ON_PIT_FLOOR,         SID_DI_WATER_ON_PIT_FLOOR},
      {AC_SYS_ALARM_EXTRA_FAULT_1,              SID_NONE}, //uses stringdatapoint
      {AC_SYS_ALARM_EXTRA_FAULT_2,              SID_NONE}, //uses stringdatapoint
      {AC_SYS_ALARM_EXTRA_FAULT_3,              SID_NONE}, //uses stringdatapoint
      {AC_SYS_ALARM_EXTRA_FAULT_4,              SID_NONE}, //uses stringdatapoint
      {AC_SYS_ALARM_DDA_FAULT,                  SID_DDA_FAULT},
      {AC_SYS_ALARM_DOSING_PUMP,                SID_ANALOG_DOSING_PUMP},
      {AC_PUMP_ALARM_ON_OFF_AUTO_SWITCH,        SID_PUMP_ALARM_ON_OFF_AUTO_SWITCH},
      {AC_PUMP_ALARM_MOTOR_PROTECTION_TRIPPED,  SID_PUMP_ALARM_MOTOR_PROTECTION_TRIPPED},
      {AC_PUMP_ALARM_COMMON_PHASE_ERROR,        SID_PUMP_ALARM_COMMON_PHASE_ERROR},
      {AC_PUMP_ALARM_IO111_WARNING,             SID_PUMP_ALARM_IO111_WARNING},
      {AC_PUMP_ALARM_IO111_ERROR,               SID_PUMP_ALARM_IO111_ALARM},
      {AC_PUMP_ALARM_OVERTEMP_PTC_IO351,        SID_PUMP_ALARM_OVERTEMP_PTC_IO351},
      {AC_PUMP_ALARM_WATER_IN_OIL,              SID_PUMP_ALARM_WATER_IN_OIL},
      {AC_PUMP_ALARM_GENIBUS_COM_ERROR_IO111,   SID_PUMP_ALARM_GENIBUS_COM_ERROR_IO111},
      {AC_PUMP_ALARM_MAX_STARTS_HOUR,           SID_PUMP_ALARM_MAX_STARTS_HOUR},
      {AC_PUMP_ALARM_LOW_FLOW,                  SID_PUMP_ALARM_LOW_FLOW},
      {AC_PUMP_ALARM_TIME_FOR_SERVICE,          SID_PUMP_ALARM_TIME_FOR_SERVICE},
      {AC_PUMP_ALARM_WATER_IN_OIL_SENSOR,       SID_PUMP_ALARM_WATER_IN_OIL_SENSOR},
      {AC_PUMP_ALARM_MOTOR_CURRENT_OVERLOAD,    SID_PUMP_ALARM_OVERLOAD},
      {AC_PUMP_ALARM_MOTOR_CURRENT_UNDERLOAD,   SID_PUMP_ALARM_UNDERLOAD},
      {AC_PUMP_ALARM_LATEST_RUNTIME,            SID_PUMP_ALARM_LATEST_RUNTIME},
      {AC_PUMP_ALARM_CONTACTOR,                 SID_PUMP_ALARM_CONTACTOR},
      {AC_PUMP_ALARM_AMPEREMETER_FAULT,         SID_PUMP_ALARM_AMPEREMETER_FAULT},
      {AC_PUMP_ALARM_TORGUE,                    SID_PUMP_ALARM_TORGUE},
      {AC_PUMP_ALARM_VFD_NOT_READY,             SID_PUMP_ALARM_VFD_NOT_READY},
      {AC_PUMP_ALARM_CUE_ALARM,                 SID_PUMP_ALARM_CUE_ALARM},
      {AC_PUMP_ALARM_CUE_WARNING,               SID_PUMP_ALARM_CUE_WARNING},
      {AC_PUMP_ALARM_MP204_ALARM,               SID_PUMP_ALARM_MP204_ALARM},
      {AC_PUMP_ALARM_MP204_WARNING,             SID_PUMP_ALARM_MP204_WARNING},
      {AC_PUMP_ALARM_POWER_METER,               SID_PUMP_ALARM_POWER_METER},
      {AC_PUMP_ALARM_BLOCKED,                   SID_PUMP_ALARM_BLOCKED},
      {AC_PUMP_ALARM_MOISTURE_PTC_IO351,        SID_PUMP_ALARM_MOISTURE_IO351B},

      {AC_MIX_ALARM_MIXER_CONTACTOR,            SID_MIX_ALARM_MIXER_CONTACTOR},
      {AC_MIX_ALARM_MIXER_SERVICE_TIME,         SID_MIX_ALARM_MIXER_SERVICE_TIME},
      {AC_MIX_ALARM_MIXER_STARTS_HOUR,          SID_MIX_ALARM_MIXER_STARTS_HOUR},

      {AC_COMBI_ALARM_1,                        SID_COMBI_ALARM_1},
      {AC_COMBI_ALARM_2,                        SID_COMBI_ALARM_2},
      {AC_COMBI_ALARM_3,                        SID_COMBI_ALARM_3},
      {AC_COMBI_ALARM_4,                        SID_COMBI_ALARM_4}};

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    AlarmState::AlarmState(Component* pParent) : State(pParent)
    {
      mpStateStringIds = mAlarmStatesStringIds;
      mStringIdCount = sizeof( mAlarmStatesStringIds ) / sizeof(StateStringId);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    AlarmState::~AlarmState()
    {
    }

    /*****************************************************************************
    * Function...: ConnectToSubjects
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void AlarmState::ConnectToSubjects(void)
    {
      State::ConnectToSubjects();

      mDpNameOfExtraFault[0].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_1));
      mDpNameOfExtraFault[1].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_2));
      mDpNameOfExtraFault[2].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_3));
      mDpNameOfExtraFault[3].Attach(::GetSubject(SUBJECT_ID_DIG_IN_FUNC_NAME_EXTRA_FAULT_4));

      for (int i = 0; i < NO_OF_EXTRA_FAULTS; i++)
      {
        mDpNameOfExtraFault[i].Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function - GetStateAsString
    * DESCRIPTION:
    * 
    *****************************************************************************/
    const char* AlarmState::GetStateAsString(int state)
    {
      if (!mDpNameOfExtraFault[0].IsValid())
      {
        ConnectToSubjects();
      }

      if (state >= AC_SYS_ALARM_EXTRA_FAULT_1 && state <= AC_SYS_ALARM_EXTRA_FAULT_4)
      {
        int extra = state - (int)AC_SYS_ALARM_EXTRA_FAULT_1;
        
        return mDpNameOfExtraFault[extra]->GetValue();
      }
      else
      {
        return Languages::GetInstance()->GetString(GetStateStringId(state));
      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * returns display number used by slip point control to set dynamic menu title
    *****************************************************************************/    
    void AlarmState::GetStateDisplayNumber(int state, int pumpGroupSubmenu, char* displayNumber)
    {
      int submenu = 0;
      int subsubmenu_offset = 0;

      if(state >= AC_FIRST_SYSTEM_ALARM && state <= AC_LAST_SYSTEM_ALARM)
      {
        submenu = 1;// Settings->Alarms->System->Config
        subsubmenu_offset = 1 - AC_FIRST_SYSTEM_ALARM;
      }
      else if(state >= AC_FIRST_PUMP_ALARM && state <= AC_LAST_PUMP_ALARM)
      {
        submenu = 2; // Settings->Alarms->Pumps->Config
        subsubmenu_offset = 1 - AC_FIRST_PUMP_ALARM;
      }
      else if(state >= AC_FIRST_MIXER_ALARM && state <= AC_LAST_MIXER_ALARM)
      {
        submenu = 3;// Settings->Alarms->Mixer->Config
        subsubmenu_offset = 1 - AC_FIRST_MIXER_ALARM;
      }
      else if(state >= AC_FIRST_COMBI_ALARM && state <= AC_LAST_COMBI_ALARM)
      {
        submenu = 4;// Settings->Alarms->Combi->Config
        switch(state)
        {
        case AC_COMBI_ALARM_1:
          subsubmenu_offset = 3;
          break;
        case AC_COMBI_ALARM_2:
          subsubmenu_offset = 5;
          break;
        case AC_COMBI_ALARM_3:
          subsubmenu_offset = 7;
          break;
        case AC_COMBI_ALARM_4:
          subsubmenu_offset = 9;
          break;
        default:
          subsubmenu_offset = 0;
        }
        subsubmenu_offset -= AC_FIRST_COMBI_ALARM;
      }
      else 
      {
        FatalErrorOccured("Alarm state OOR error");
      }

       

      if (pumpGroupSubmenu > 0)
      {// Settings->Alarms->Pump->group->Config
        sprintf(displayNumber, "4.5.%i.%i.%i", submenu, pumpGroupSubmenu, (subsubmenu_offset + state));
      }
      else
      {// Settings->Alarms->Pump->Config
        sprintf(displayNumber, "4.5.%i.%i", submenu, (subsubmenu_offset + state));
      }
    }


  } // namespace display
} // namespace mpc


