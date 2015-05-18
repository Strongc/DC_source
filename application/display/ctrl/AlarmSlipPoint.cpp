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
/* CLASS NAME       : AlarmSlipPoint                                        */
/*                                                                          */
/* FILE NAME        : AlarmSlipPoint.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 05-11-2007                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the alarm DataPoints into common                         */
/* virtual DataPoints for the display to look at...                         */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Factory.h>
#include <DataPoint.h>
#include <UpperStatusLine.h>
#include <DisplayController.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "AlarmSlipPoint.h"


/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_PUMP_ALARM_CONFIGS_ID 60
#define DISPLAY_ALARM_CONFIG_ANALOGE_ID 66
#define DISPLAY_ALARM_CONFIG_DIGITAL_ID 67

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

namespace mpc
{
  namespace display
  {
    namespace ctrl
    {

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
AlarmSlipPoint::AlarmSlipPoint()
{
  mCurrentlyUpdating = false;
  mpAlarmState = new AlarmState();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AlarmSlipPoint::~AlarmSlipPoint()
{
  delete mpAlarmState;
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AlarmSlipPoint::InitSubTask(void)
{
  mpCurrentAlarmNumber->SetAsInt(1);

  mCurrentlyUpdating = true;
  
  UpdateVirtualAlarm();
  UpdateUpperStatusLine();
  
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AlarmSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if (mpCurrentAlarmNumber.IsUpdated() || mpCurrentPumpGroup.IsUpdated())
  {
    UpdateVirtualAlarm();

    UpdateUpperStatusLine();
  }

  if (mpVirtualCategory.IsUpdated())
  {
    //update warning and alarm enabled from category
    UpdateCategory(true);

    mpVirtualWarningEnabled.ResetUpdated();
    mpVirtualAlarmEnabled.ResetUpdated();

    UpdateCurrentAlarm();
  }
  else if (mpVirtualWarningEnabled.IsUpdated()
    || mpVirtualAlarmEnabled.IsUpdated())
  {
    //update category from warning and alarm enabled
    UpdateCategory(false);

    mpVirtualCategory.ResetUpdated();

    UpdateCurrentAlarm();
  }
  else if ( mpVirtualSms1Enabled.IsUpdated()
    || mpVirtualSms2Enabled.IsUpdated()
    || mpVirtualSms3Enabled.IsUpdated()
    || mpVirtualScadaEnabled.IsUpdated()
    || mpVirtualCustomRelayForWarningEnabled.IsUpdated()
    || mpVirtualCustomRelayForAlarmEnabled.IsUpdated()
    || mpVirtualAutoAck.IsUpdated()
    || mpVirtualDelay.IsUpdated()
    || mpVirtualWarningLimitFloat.IsUpdated()
    || mpVirtualWarningLimitInt.IsUpdated()
    || mpVirtualAlarmLimitFloat.IsUpdated()
    || mpVirtualAlarmLimitInt.IsUpdated())
  {
    UpdateCurrentAlarm();
  }

  
  const int index = mpCurrentAlarmNumber->GetAsInt();

  if ((index >= 0) && (index < NO_OF_ALARM_CONFIG))
  {
    int group_no = 0;

    if (mpIsHighEnd->GetAsBool() && index >= AC_FIRST_PUMP_ALARM && index <= AC_LAST_PUMP_ALARM) 
    {
      group_no = mpCurrentPumpGroup->GetAsInt();
    }

    if (mpAlarmDP[index][group_no].IsUpdated())
    {
      UpdateVirtualAlarm();
    }
  }
  else
  {
    FatalErrorOccured("ASP: OOR error"); // index out of range
  }
  

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AlarmSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reacting on updates

  for (int i = 0; i < NO_OF_ALARM_CONFIG; ++i )
  {
    if (mpAlarmDP[i][0].Detach(pSubject))
    {
      return;
    }
  }

  mpCurrentAlarmNumber.Detach(pSubject);
  mpVirtualCategory.Detach(pSubject);
  mpVirtualSms1Enabled.Detach(pSubject);
  mpVirtualSms2Enabled.Detach(pSubject);
  mpVirtualSms3Enabled.Detach(pSubject);
  mpVirtualScadaEnabled.Detach(pSubject);
  mpVirtualWarningEnabled.Detach(pSubject);
  mpVirtualAlarmEnabled.Detach(pSubject);
  mpVirtualCustomRelayForWarningEnabled.Detach(pSubject);
  mpVirtualCustomRelayForAlarmEnabled.Detach(pSubject);
  mpVirtualAutoAck.Detach(pSubject);
  mpVirtualDelay.Detach(pSubject);
  mpVirtualWarningLimitFloat.Detach(pSubject);
  mpVirtualWarningLimitInt.Detach(pSubject);
  mpVirtualAlarmLimitFloat.Detach(pSubject);
  mpVirtualAlarmLimitInt.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AlarmSlipPoint::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    if (mpCurrentAlarmNumber.Update(pSubject)){}
    else if (mpCurrentPumpGroup.Update(pSubject)){}
    else if (mpVirtualCategory.Update(pSubject)){}
    else if (mpVirtualSms1Enabled.Update(pSubject)){}
    else if (mpVirtualSms2Enabled.Update(pSubject)){}
    else if (mpVirtualSms3Enabled.Update(pSubject)){}
    else if (mpVirtualScadaEnabled.Update(pSubject)){}
    else if (mpVirtualWarningEnabled.Update(pSubject)){}
    else if (mpVirtualAlarmEnabled.Update(pSubject)){}
    else if (mpVirtualCustomRelayForWarningEnabled.Update(pSubject)){}
    else if (mpVirtualCustomRelayForAlarmEnabled.Update(pSubject)){}
    else if (mpVirtualAutoAck.Update(pSubject)){}
    else if (mpVirtualDelay.Update(pSubject)){}
    else if (mpVirtualWarningLimitFloat.Update(pSubject)){}
    else if (mpVirtualWarningLimitInt.Update(pSubject)){}
    else if (mpVirtualAlarmLimitFloat.Update(pSubject)){}
    else if (mpVirtualAlarmLimitInt.Update(pSubject)){}
    else
    {

      for (int group_no = FIRST_PUMP_GROUP; group_no <= LAST_PUMP_GROUP; group_no++)
      {
        for ( int i = 0; i < NO_OF_ALARM_CONFIG; ++i )
        {
          if (mpAlarmDP[i][group_no].IsValid() && mpAlarmDP[i][group_no].Update(pSubject))
          {
            break;
          }
        }
      }
    }
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AlarmSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
    case SP_ASP_SYS_ALARM_FLOAT_SWITCH:
      mpAlarmDP[AC_SYS_ALARM_FLOAT_SWITCH][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_LEVEL_SENSOR:
      mpAlarmDP[AC_SYS_ALARM_LEVEL_SENSOR][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_FLOW_METER:
      mpAlarmDP[AC_SYS_ALARM_FLOW_METER][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_POWER_METER:
      mpAlarmDP[AC_SYS_ALARM_POWER_METER][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_HIGH_LEVEL:
      mpAlarmDP[AC_SYS_ALARM_HIGH_LEVEL][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_LEVEL:
      mpAlarmDP[AC_SYS_ALARM_LEVEL][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_DRY_RUNNING:
      mpAlarmDP[AC_SYS_ALARM_DRY_RUNNING][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_CONFLICTING_LEVELS:
      mpAlarmDP[AC_SYS_ALARM_CONFLICTING_LEVELS][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_ETHERNET_NO_ADDRESS:
      mpAlarmDP[AC_SYS_ALARM_ETHERNET_NO_ADDRESS][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_ETHERNET_MISUSED:
      mpAlarmDP[AC_SYS_ALARM_ETHERNET_MISUSED][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_SIM_CARD:
      mpAlarmDP[AC_SYS_ALARM_SIM_CARD][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_UPS:
      mpAlarmDP[AC_SYS_ALARM_UPS][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_HW_FAULT:
      mpAlarmDP[AC_SYS_ALARM_HW_FAULT][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_EXTERNAL_FAULT:
      mpAlarmDP[AC_SYS_ALARM_EXTERNAL_FAULT][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_MAINS_VOLTAGE:
      mpAlarmDP[AC_SYS_ALARM_MAINS_VOLTAGE][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_CIU_CARD_FAILURE:
      mpAlarmDP[AC_SYS_ALARM_CIU_CARD_FAILURE][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_SCADA_CALLBACK:
      mpAlarmDP[AC_SYS_ALARM_SCADA_CALLBACK][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_FORCED_RELAY_OUTPUT_ENABLED:
      mpAlarmDP[AC_SYS_ALARM_FORCED_RELAY_OUTPUT_ENABLED][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_OVERFLOW:
      mpAlarmDP[AC_SYS_ALARM_OVERFLOW][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_COM_ERROR_IO351:
      mpAlarmDP[AC_SYS_ALARM_COM_ERROR_IO351][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_USER_DEFINED_SENSOR_1:
      mpAlarmDP[AC_SYS_ALARM_USER_DEFINED_SENSOR_1][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_USER_DEFINED_SENSOR_2:
      mpAlarmDP[AC_SYS_ALARM_USER_DEFINED_SENSOR_2][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_USER_DEFINED_SENSOR_3:
      mpAlarmDP[AC_SYS_ALARM_USER_DEFINED_SENSOR_3][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_PRESSURE_SENSOR_OUTLET_PIPELINE:
      mpAlarmDP[AC_SYS_ALARM_PRESSURE_SENSOR_OUTLET_PIPELINE][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_GAS_DETECTOR:
      mpAlarmDP[AC_SYS_ALARM_GAS_DETECTOR][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_WATER_ON_PIT_FLOOR:
      mpAlarmDP[AC_SYS_ALARM_WATER_ON_PIT_FLOOR][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_EXTRA_FAULT_1:
      mpAlarmDP[AC_SYS_ALARM_EXTRA_FAULT_1][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_EXTRA_FAULT_2:
      mpAlarmDP[AC_SYS_ALARM_EXTRA_FAULT_2][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_EXTRA_FAULT_3:
      mpAlarmDP[AC_SYS_ALARM_EXTRA_FAULT_3][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_EXTRA_FAULT_4:
      mpAlarmDP[AC_SYS_ALARM_EXTRA_FAULT_4][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_DDA_FAULT:
      mpAlarmDP[AC_SYS_ALARM_DDA_FAULT][0].Attach(pSubject);
      break;
    case SP_ASP_SYS_ALARM_DOSING_PUMP:
      mpAlarmDP[AC_SYS_ALARM_DOSING_PUMP][0].Attach(pSubject);
      break;
      
    case SP_ASP_PG1_ALARM_ON_OFF_AUTO_SWITCH:
      mpAlarmDP[AC_PUMP_ALARM_ON_OFF_AUTO_SWITCH][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_MOTOR_PROTECTION_TRIPPED:
      mpAlarmDP[AC_PUMP_ALARM_MOTOR_PROTECTION_TRIPPED][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_COMMON_PHASE_ERROR:
      mpAlarmDP[AC_PUMP_ALARM_COMMON_PHASE_ERROR][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_IO111_WARNING:
      mpAlarmDP[AC_PUMP_ALARM_IO111_WARNING][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_IO111_ERROR:
      mpAlarmDP[AC_PUMP_ALARM_IO111_ERROR][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_OVERTEMP_PTC_IO351:
      mpAlarmDP[AC_PUMP_ALARM_OVERTEMP_PTC_IO351][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_WATER_IN_OIL:
      mpAlarmDP[AC_PUMP_ALARM_WATER_IN_OIL][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_GENIBUS_COM_ERROR_IO111:
      mpAlarmDP[AC_PUMP_ALARM_GENIBUS_COM_ERROR_IO111][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_MAX_STARTS_HOUR:
      mpAlarmDP[AC_PUMP_ALARM_MAX_STARTS_HOUR][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_LOW_FLOW:
      mpAlarmDP[AC_PUMP_ALARM_LOW_FLOW][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_TIME_FOR_SERVICE:
      mpAlarmDP[AC_PUMP_ALARM_TIME_FOR_SERVICE][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_WATER_IN_OIL_SENSOR:
      mpAlarmDP[AC_PUMP_ALARM_WATER_IN_OIL_SENSOR][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_MOTOR_CURRENT_OVERLOAD:
      mpAlarmDP[AC_PUMP_ALARM_MOTOR_CURRENT_OVERLOAD][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_MOTOR_CURRENT_UNDERLOAD:
      mpAlarmDP[AC_PUMP_ALARM_MOTOR_CURRENT_UNDERLOAD][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_LATEST_RUNTIME:
      mpAlarmDP[AC_PUMP_ALARM_LATEST_RUNTIME][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_CONTACTOR:
      mpAlarmDP[AC_PUMP_ALARM_CONTACTOR][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_AMPEREMETER_FAULT:
      mpAlarmDP[AC_PUMP_ALARM_AMPEREMETER_FAULT][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_TORGUE:
      mpAlarmDP[AC_PUMP_ALARM_TORGUE][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_VFD_NOT_READY:
      mpAlarmDP[AC_PUMP_ALARM_VFD_NOT_READY][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_CUE_ALARM:
      mpAlarmDP[AC_PUMP_ALARM_CUE_ALARM][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_CUE_WARNING:
      mpAlarmDP[AC_PUMP_ALARM_CUE_WARNING][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_MP204_ALARM:
      mpAlarmDP[AC_PUMP_ALARM_MP204_ALARM][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_MP204_WARNING:
      mpAlarmDP[AC_PUMP_ALARM_MP204_WARNING][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_POWER_METER:
      mpAlarmDP[AC_PUMP_ALARM_POWER_METER][PUMP_GROUP_1].Attach(pSubject);
      break;
    case SP_ASP_PG1_ALARM_BLOCKED:
      mpAlarmDP[AC_PUMP_ALARM_BLOCKED][PUMP_GROUP_1].Attach(pSubject);
      break;  
    case SP_ASP_PG1_ALARM_MOISTURE_PTC_IO351:
      mpAlarmDP[AC_PUMP_ALARM_MOISTURE_PTC_IO351][PUMP_GROUP_1].Attach(pSubject);
      break;


    case SP_ASP_PG2_ALARM_ON_OFF_AUTO_SWITCH:
      mpAlarmDP[AC_PUMP_ALARM_ON_OFF_AUTO_SWITCH][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_MOTOR_PROTECTION_TRIPPED:
      mpAlarmDP[AC_PUMP_ALARM_MOTOR_PROTECTION_TRIPPED][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_COMMON_PHASE_ERROR:
      mpAlarmDP[AC_PUMP_ALARM_COMMON_PHASE_ERROR][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_IO111_WARNING:
      mpAlarmDP[AC_PUMP_ALARM_IO111_WARNING][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_IO111_ERROR:
      mpAlarmDP[AC_PUMP_ALARM_IO111_ERROR][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_OVERTEMP_PTC_IO351:
      mpAlarmDP[AC_PUMP_ALARM_OVERTEMP_PTC_IO351][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_WATER_IN_OIL:
      mpAlarmDP[AC_PUMP_ALARM_WATER_IN_OIL][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_GENIBUS_COM_ERROR_IO111:
      mpAlarmDP[AC_PUMP_ALARM_GENIBUS_COM_ERROR_IO111][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_MAX_STARTS_HOUR:
      mpAlarmDP[AC_PUMP_ALARM_MAX_STARTS_HOUR][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_LOW_FLOW:
      mpAlarmDP[AC_PUMP_ALARM_LOW_FLOW][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_TIME_FOR_SERVICE:
      mpAlarmDP[AC_PUMP_ALARM_TIME_FOR_SERVICE][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_WATER_IN_OIL_SENSOR:
      mpAlarmDP[AC_PUMP_ALARM_WATER_IN_OIL_SENSOR][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_MOTOR_CURRENT_OVERLOAD:
      mpAlarmDP[AC_PUMP_ALARM_MOTOR_CURRENT_OVERLOAD][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_MOTOR_CURRENT_UNDERLOAD:
      mpAlarmDP[AC_PUMP_ALARM_MOTOR_CURRENT_UNDERLOAD][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_LATEST_RUNTIME:
      mpAlarmDP[AC_PUMP_ALARM_LATEST_RUNTIME][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_CONTACTOR:
      mpAlarmDP[AC_PUMP_ALARM_CONTACTOR][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_AMPEREMETER_FAULT:
      mpAlarmDP[AC_PUMP_ALARM_AMPEREMETER_FAULT][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_TORGUE:
      mpAlarmDP[AC_PUMP_ALARM_TORGUE][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_VFD_NOT_READY:
      mpAlarmDP[AC_PUMP_ALARM_VFD_NOT_READY][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_CUE_ALARM:
      mpAlarmDP[AC_PUMP_ALARM_CUE_ALARM][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_CUE_WARNING:
      mpAlarmDP[AC_PUMP_ALARM_CUE_WARNING][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_MP204_ALARM:
      mpAlarmDP[AC_PUMP_ALARM_MP204_ALARM][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_MP204_WARNING:
      mpAlarmDP[AC_PUMP_ALARM_MP204_WARNING][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_POWER_METER:
      mpAlarmDP[AC_PUMP_ALARM_POWER_METER][PUMP_GROUP_2].Attach(pSubject);
      break;
    case SP_ASP_PG2_ALARM_BLOCKED:
      mpAlarmDP[AC_PUMP_ALARM_BLOCKED][PUMP_GROUP_2].Attach(pSubject);
      break;     
    case SP_ASP_PG2_ALARM_MOISTURE_PTC_IO351:
      mpAlarmDP[AC_PUMP_ALARM_MOISTURE_PTC_IO351][PUMP_GROUP_2].Attach(pSubject);
      break;


    case SP_ASP_MIX_ALARM_MIXER_CONTACTOR:
      mpAlarmDP[AC_MIX_ALARM_MIXER_CONTACTOR][0].Attach(pSubject);
      break;
    case SP_ASP_MIX_ALARM_MIXER_SERVICE_TIME:
      mpAlarmDP[AC_MIX_ALARM_MIXER_SERVICE_TIME][0].Attach(pSubject);
      break;
    case SP_ASP_MIX_ALARM_MIXER_STARTS_HOUR:
      mpAlarmDP[AC_MIX_ALARM_MIXER_STARTS_HOUR][0].Attach(pSubject);
      break;
    case SP_ASP_COMBI_ALARM_1:
      mpAlarmDP[AC_COMBI_ALARM_1][0].Attach(pSubject);
      break;
    case SP_ASP_COMBI_ALARM_2:
      mpAlarmDP[AC_COMBI_ALARM_2][0].Attach(pSubject);
      break;
    case SP_ASP_COMBI_ALARM_3:
      mpAlarmDP[AC_COMBI_ALARM_3][0].Attach(pSubject);
      break;
    case SP_ASP_COMBI_ALARM_4:
      mpAlarmDP[AC_COMBI_ALARM_4][0].Attach(pSubject);
      break;
    case SP_ASP_CURRENT_ALARM_NO:
      mpCurrentAlarmNumber.Attach(pSubject);
      break;
    case SP_ASP_CURRENT_PUMP_GROUP:
      mpCurrentPumpGroup.Attach(pSubject);
      break;
    case SP_ASP_IS_HIGH_END:
      mpIsHighEnd.Attach(pSubject);
      break;
    case SP_ASP_PUMP_GROUPS_ENABLED:
      mpPumpGroupsEnabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_ALARM_ENABLED:
      mpVirtualAlarmEnabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_ALARM_LIMIT_FLOAT:
      mpVirtualAlarmLimitFloat.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_ALARM_LIMIT_INT:
      mpVirtualAlarmLimitInt.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_AUTO_ACK:
      mpVirtualAutoAck.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_CUSTOM_RELAY_ALARM:
      mpVirtualCustomRelayForAlarmEnabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_CUSTOM_RELAY_WARNING:
      mpVirtualCustomRelayForWarningEnabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_ALARM_DELAY:
      mpVirtualDelay.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_SCADA_ENABLED:
      mpVirtualScadaEnabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_SMS1_ENABLED:
      mpVirtualSms1Enabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_SMS2_ENABLED:
      mpVirtualSms2Enabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_SMS3_ENABLED:
      mpVirtualSms3Enabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_WARNING_ENABLED:
      mpVirtualWarningEnabled.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_WAR_LIMIT_FLOAT:
      mpVirtualWarningLimitFloat.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_WAR_LIMIT_INT:
      mpVirtualWarningLimitInt.Attach(pSubject);
      break;
    case SP_ASP_VIRTUAL_CATEGORY:
      mpVirtualCategory.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AlarmSlipPoint::ConnectToSubjects(void)
{
  for (int group_no = FIRST_PUMP_GROUP; group_no <= LAST_PUMP_GROUP; group_no++)
  {
    for ( int i = 0; i < NO_OF_ALARM_CONFIG; ++i )
    {
      if (mpAlarmDP[i][group_no].IsValid())
      {
        mpAlarmDP[i]->Subscribe(this);
      }
    }
  }

  mpCurrentAlarmNumber->Subscribe(this);
  mpCurrentPumpGroup->Subscribe(this);
  mpVirtualCategory->Subscribe(this);
  mpVirtualSms1Enabled->Subscribe(this);
  mpVirtualSms2Enabled->Subscribe(this);
  mpVirtualSms3Enabled->Subscribe(this);
  mpVirtualScadaEnabled->Subscribe(this);
  mpVirtualWarningEnabled->Subscribe(this);
  mpVirtualAlarmEnabled->Subscribe(this);
  mpVirtualCustomRelayForWarningEnabled->Subscribe(this);
  mpVirtualCustomRelayForAlarmEnabled->Subscribe(this);
  mpVirtualAutoAck->Subscribe(this);
  mpVirtualDelay->Subscribe(this);
  mpVirtualWarningLimitFloat->Subscribe(this);
  mpVirtualWarningLimitInt->Subscribe(this);
  mpVirtualAlarmLimitFloat->Subscribe(this);
  mpVirtualAlarmLimitInt->Subscribe(this);
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void AlarmSlipPoint::UpdateVirtualAlarm()
{
  const int index = mpCurrentAlarmNumber->GetAsInt();
  int group_no = 0;

  if (mpIsHighEnd->GetAsBool() && index >= AC_FIRST_PUMP_ALARM && index <= AC_LAST_PUMP_ALARM) 
  {
    group_no = mpCurrentPumpGroup->GetAsInt();
  }

  if ((index >= 0) && (index < NO_OF_ALARM_CONFIG))
  {
    AlarmConfig* p_cfg = mpAlarmDP[index][group_no].GetSubject();

    mpVirtualAlarmEnabled->SetValue( p_cfg->GetAlarmEnabled() );
    mpVirtualWarningEnabled->SetValue( p_cfg->GetWarningEnabled() );
    mpVirtualAutoAck->SetValue( p_cfg->GetAutoAcknowledge() );

    if (p_cfg->GetAlarmConfigType() == AC_DIGITAL)
    {
      bool custom_relay_set = p_cfg->GetCustomRelayForAlarm();

      if(mpVirtualWarningEnabled->GetValue())
      {
        custom_relay_set = p_cfg->GetCustomRelayForWarning();
      }
      mpVirtualCustomRelayForAlarmEnabled->SetValue(custom_relay_set);
      mpVirtualCustomRelayForWarningEnabled->SetValue(custom_relay_set);
    }
    else
    {
      mpVirtualCustomRelayForAlarmEnabled->SetValue(p_cfg->GetCustomRelayForAlarm());
      mpVirtualCustomRelayForWarningEnabled->SetValue(p_cfg->GetCustomRelayForWarning());
    }
    
    mpVirtualDelay->SetValue( p_cfg->GetAlarmDelay() );
    mpVirtualSms1Enabled->SetValue( p_cfg->GetSms1Enabled() );
    mpVirtualSms2Enabled->SetValue( p_cfg->GetSms2Enabled() );
    mpVirtualSms3Enabled->SetValue( p_cfg->GetSms3Enabled() );
    mpVirtualScadaEnabled->SetValue( p_cfg->GetScadaEnabled() );

    UpdateCategory();

    INumberDataPoint* p_alarm_limit = p_cfg->GetAlarmLimit();
    INumberDataPoint* p_warning_limit = p_cfg->GetWarningLimit();
    if (p_cfg->GetLimitTypeIsFloat())
    {
      mpVirtualAlarmLimitFloat->SetMinValue( p_alarm_limit->GetMinAsFloat() );
      mpVirtualAlarmLimitFloat->SetMaxValue( p_alarm_limit->GetMaxAsFloat() );
      mpVirtualWarningLimitFloat->SetMinValue( p_alarm_limit->GetMinAsFloat() );
      mpVirtualWarningLimitFloat->SetMaxValue( p_alarm_limit->GetMaxAsFloat() );

      mpVirtualAlarmLimitInt->SetQuality( DP_NEVER_AVAILABLE );
      mpVirtualWarningLimitInt->SetQuality( DP_NEVER_AVAILABLE );

      mpVirtualAlarmLimitFloat->SetQuantity( p_alarm_limit->GetQuantity() );
      mpVirtualWarningLimitFloat->SetQuantity( p_warning_limit->GetQuantity() );

      mpVirtualAlarmLimitFloat->SetAsFloat( p_alarm_limit->GetAsFloat() );
      mpVirtualWarningLimitFloat->SetAsFloat( p_warning_limit->GetAsFloat() );
    }
    else
    {
      mpVirtualAlarmLimitInt->SetMinValue( p_alarm_limit->GetMinAsInt() );
      mpVirtualAlarmLimitInt->SetMaxValue( p_alarm_limit->GetMaxAsInt() );
      mpVirtualWarningLimitInt->SetMinValue( p_alarm_limit->GetMinAsInt() );
      mpVirtualWarningLimitInt->SetMaxValue( p_alarm_limit->GetMaxAsInt() );

      mpVirtualAlarmLimitFloat->SetQuality( DP_NEVER_AVAILABLE );
      mpVirtualWarningLimitFloat->SetQuality( DP_NEVER_AVAILABLE );

      mpVirtualAlarmLimitInt->SetQuantity( p_alarm_limit->GetQuantity() );
      mpVirtualWarningLimitInt->SetQuantity( p_warning_limit->GetQuantity() );

      mpVirtualAlarmLimitInt->SetAsInt( p_alarm_limit->GetAsInt() );
      mpVirtualWarningLimitInt->SetAsInt( p_warning_limit->GetAsInt() );
    }

    Display* p_display;
    p_display = GetDisplay( DISPLAY_ALARM_CONFIG_ANALOGE_ID );
    p_display->GetRoot()->Invalidate();
    p_display = GetDisplay( DISPLAY_ALARM_CONFIG_DIGITAL_ID );
    p_display->GetRoot()->Invalidate();

  }
  else
  {
    FatalErrorOccured("ASP: OOR error"); // index out of range
  }
}

void AlarmSlipPoint::UpdateUpperStatusLine()
{
  Display* p_display = NULL;
  char display_number[12];
  int pump_group_submenu = 0;

  int alarm_no = mpCurrentAlarmNumber->GetAsInt();

  p_display = GetDisplay(DISPLAY_PUMP_ALARM_CONFIGS_ID);

  if (mpIsHighEnd->GetAsBool() && mpPumpGroupsEnabled->GetValue())
  {
    pump_group_submenu = mpCurrentPumpGroup->GetValue() + 1;

    switch (mpCurrentPumpGroup->GetValue())
    {
    case PUMP_GROUP_1:
      p_display->SetDisplayNumber("4.5.2.1");
      p_display->SetName(SID_PUMP_ALARMS_GROUP_1);
      break;
    case PUMP_GROUP_2:
      p_display->SetDisplayNumber("4.5.2.2");
      p_display->SetName(SID_PUMP_ALARMS_GROUP_2);
      break;
    default:
      p_display->SetDisplayNumber("4.5.2.?");
      p_display->SetName(SID_DASH);
      break;
    }
  }
  else
  {
    p_display->SetDisplayNumber("4.5.2");
    p_display->SetName(SID_PUMP_ALARMS);
  }

  mpAlarmState->GetStateDisplayNumber(alarm_no, pump_group_submenu, display_number);

  p_display = GetDisplay(DISPLAY_ALARM_CONFIG_ANALOGE_ID);

  SetTitle(p_display, display_number, alarm_no);

  p_display = GetDisplay(DISPLAY_ALARM_CONFIG_DIGITAL_ID);

  SetTitle(p_display, display_number, alarm_no);

  DisplayController::GetInstance()->RequestTitleUpdate();
}

void AlarmSlipPoint::SetTitle(Display* pDisplay, char* displayNumber, int alarmNo)
{
  int title_id = mpAlarmState->GetStateStringId(alarmNo);

  pDisplay->SetDisplayNumber(displayNumber);
  pDisplay->SetName(title_id);

  if (title_id == SID_NONE)
  {
    pDisplay->SetTitleText((char*)mpAlarmState->GetStateAsString(alarmNo));
  }
}

void AlarmSlipPoint::UpdateCategory(bool reversed)
{
  if( reversed )
  {
    //update warning and alarm enabled from category
    switch (mpVirtualCategory->GetValue())
    {
      case ALARM_CATEGORY_WARNING_ALARM:
        mpVirtualWarningEnabled->SetValue(true);
        mpVirtualAlarmEnabled->SetValue(true);
        break;
      case ALARM_CATEGORY_WARNING:
        mpVirtualWarningEnabled->SetValue(true);
        mpVirtualAlarmEnabled->SetValue(false);
        break;
      case ALARM_CATEGORY_ALARM:
        mpVirtualWarningEnabled->SetValue(false);
        mpVirtualAlarmEnabled->SetValue(true);
        break;
      case ALARM_CATEGORY_NONE:
      default:
        mpVirtualWarningEnabled->SetValue(false);
        mpVirtualAlarmEnabled->SetValue(false);
        break;
    }
  }
  else
  {
    //update category from warning and alarm enabled
    if(mpVirtualWarningEnabled->GetAsBool() && mpVirtualAlarmEnabled->GetAsBool())
    {
      mpVirtualCategory->SetValue(ALARM_CATEGORY_WARNING_ALARM);
    }
    else if(mpVirtualWarningEnabled->GetAsBool())
    {
      mpVirtualCategory->SetValue(ALARM_CATEGORY_WARNING);
    }
    else if(mpVirtualAlarmEnabled->GetAsBool())
    {
      mpVirtualCategory->SetValue(ALARM_CATEGORY_ALARM);
    }
    else
    {
      mpVirtualCategory->SetValue(ALARM_CATEGORY_NONE);
    }

  }
}


void AlarmSlipPoint::UpdateCurrentAlarm()
{
  const int index = mpCurrentAlarmNumber->GetAsInt();
  int group_no = 0;

  if (mpIsHighEnd->GetAsBool() && index >= AC_FIRST_PUMP_ALARM && index <= AC_LAST_PUMP_ALARM) 
  {
    group_no = mpCurrentPumpGroup->GetAsInt();
  }

  if ((index >= 0) && (index < NO_OF_ALARM_CONFIG))
  {
    AlarmConfig* p_cfg = mpAlarmDP[index][group_no].GetSubject();

    p_cfg->SetAlarmEnabled( mpVirtualAlarmEnabled->GetValue() );
    p_cfg->SetWarningEnabled( mpVirtualWarningEnabled->GetValue( ) );
    p_cfg->SetAutoAcknowledge( mpVirtualAutoAck->GetValue() );
    p_cfg->SetCustomRelayForAlarm( mpVirtualCustomRelayForAlarmEnabled->GetValue() );
    if (p_cfg->GetAlarmConfigType() == AC_DIGITAL)
    {
      // For digital alarms only Custom Relay for Alarm tick box is shown at display
      // This tick box should in this case be used for both custom relay settings
      p_cfg->SetCustomRelayForWarning( mpVirtualCustomRelayForAlarmEnabled->GetValue() );
    }
    else
    {
      p_cfg->SetCustomRelayForWarning( mpVirtualCustomRelayForWarningEnabled->GetValue() );
    }
    p_cfg->SetAlarmDelay( mpVirtualDelay->GetValue() );
    p_cfg->SetSms1Enabled( mpVirtualSms1Enabled->GetValue() );
    p_cfg->SetSms2Enabled( mpVirtualSms2Enabled->GetValue() );
    p_cfg->SetSms3Enabled( mpVirtualSms3Enabled->GetValue() );
    p_cfg->SetScadaEnabled( mpVirtualScadaEnabled->GetValue() );

    INumberDataPoint* p_alarm_limit = p_cfg->GetAlarmLimit();
    INumberDataPoint* p_warning_limit = p_cfg->GetWarningLimit();
    if (p_cfg->GetLimitTypeIsFloat())
    {
      p_cfg->SetAlarmLimit( mpVirtualAlarmLimitFloat->GetAsFloat() );
      p_cfg->SetWarningLimit( mpVirtualWarningLimitFloat->GetAsFloat() );
    }
    else
    {
      p_cfg->SetAlarmLimit( mpVirtualAlarmLimitInt->GetAsInt() );
      p_cfg->SetWarningLimit( mpVirtualWarningLimitInt->GetAsInt() );
    }

  }
  else
  {
    FatalErrorOccured("ASP: OOR error"); // index out of range
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc
