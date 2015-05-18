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
/* CLASS NAME       : AnaInMeasureValueCtrl                                 */
/*                                                                          */
/* FILE NAME        : AnaInMeasureValueCtrl.cpp                             */
/*                                                                          */
/* CREATED DATE     : 17-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "AnaInMeasureValueCtrl.h"             // class implemented

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  //NB: the 8 REDUNDANT_SENSOR_WARNING_TIMER_[n] need to follow in an uninterrupted row, since this is assumed in the code
  REDUNDANT_SENSOR_WARNING_TIMER_1,
  REDUNDANT_SENSOR_WARNING_TIMER_2,
  REDUNDANT_SENSOR_WARNING_TIMER_3,
  REDUNDANT_SENSOR_WARNING_TIMER_4,
  REDUNDANT_SENSOR_WARNING_TIMER_5,
  REDUNDANT_SENSOR_WARNING_TIMER_6,
  REDUNDANT_SENSOR_WARNING_TIMER_7,
  REDUNDANT_SENSOR_WARNING_TIMER_8
};
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
 *****************************************************************************/
AnaInMeasureValueCtrl::AnaInMeasureValueCtrl()
{
  for (int i = 0; i < NO_OF_REDUNDANCY_TIMERS; i++)
  {
    mRedundantErrorStatus[i] = NO_REDUNDANT_ERROR;
  }
}
/****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
AnaInMeasureValueCtrl::~AnaInMeasureValueCtrl()
{
}

/*****************************************************************************
 * Function - InitSubtask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInMeasureValueCtrl::InitSubTask()
{
  int i;

  for (i = 0; i < AIMVC_NO_OF_ANA_IN; i++)
  {
    mRedundantPartner[i] = 0;
  }

  for (i = 0; i < NO_OF_REDUNDANCY_TIMERS; i++)
  {
    mpTimerObjList[i + REDUNDANT_SENSOR_WARNING_TIMER_1] = new SwTimer(5, S, FALSE, FALSE, this);
  }

  for (i = 0; i < NO_OF_REDUNDANCY_TIMERS; i++)
  {
    mRedundantErrorStatus[i] = NO_REDUNDANT_ERROR;
    mListOfRedundancyTimerEvents[i] = false;
  }

  mAnaInValueUpdated = true;    // inital update
  mSensorConfigUpdated = true;
  mHandleRedundancyTimers = false;
  mReqTaskTime = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void AnaInMeasureValueCtrl::RunSubTask()
{
  mReqTaskTime = false;

  for (int i = 0; i < AIMVC_NO_OF_ANA_IN; i++)
  {
    if (mpConfMeasuredValue[i].IsUpdated())
    {
      switch (mpConfMeasuredValue[i]->GetValue())
      {
        case MEASURED_VALUE_NOT_SELECTED:
          mpConfSensorMinValue[i]->SetQuantity(Q_NO_UNIT);
          mpConfSensorMinValue[i]->SetMinValue(0);
          mpConfSensorMinValue[i]->SetMaxValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0);
          }
          mpConfSensorMaxValue[i]->SetQuantity(Q_NO_UNIT);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          mpConfSensorMaxValue[i]->SetMaxValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(0);
          }
          break;

        case MEASURED_VALUE_INLET_PRESS:
				case MEASURED_VALUE_DISCHARGE_PRESS:
          mpConfSensorMinValue[i]->SetQuantity(Q_PRESSURE);
          mpConfSensorMinValue[i]->SetMinValue(-100000);
          mpConfSensorMinValue[i]->SetMaxValue(10000000);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0bar
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_PRESSURE);
          mpConfSensorMaxValue[i]->SetMinValue(-100000);
          mpConfSensorMaxValue[i]->SetMaxValue(10000000);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(1600000.0f); //16bar
          }
          break;

        case MEASURED_VALUE_OUTLET_PRESSURE:
          mpConfSensorMinValue[i]->SetQuantity(Q_PRESSURE);
          mpConfSensorMinValue[i]->SetMinValue(0);
          mpConfSensorMinValue[i]->SetMaxValue(10000000);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0bar
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_PRESSURE);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          mpConfSensorMaxValue[i]->SetMaxValue(10000000);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(100000.0f); //1bar
          }
          break;

        case MEASURED_VALUE_PUMP_DIFF_PRESS:
          mpConfSensorMinValue[i]->SetQuantity(Q_DIFFERENCIAL_PRESSURE);
          mpConfSensorMinValue[i]->SetMinValue(0);
          mpConfSensorMinValue[i]->SetMaxValue(10000000);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0bar
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_DIFFERENCIAL_PRESSURE);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          mpConfSensorMaxValue[i]->SetMaxValue(10000000);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(1600000.0f); //16bar
          }
          break;

        case MEASURED_VALUE_FLOW:
          mpConfSensorMinValue[i]->SetQuantity(Q_FLOW);
          mpConfSensorMinValue[i]->SetMaxValue(3);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0m3/h
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_FLOW);
          mpConfSensorMaxValue[i]->SetMaxValue(3);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(20.0f/3600.0f);       //20m3/h
          }
          break;

        case MEASURED_VALUE_LEVEL_ULTRA_SOUND:
          mpConfSensorMinValue[i]->SetQuantity(Q_HEIGHT);
          mpConfSensorMinValue[i]->SetMaxValue(100.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0m
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_HEIGHT);
          mpConfSensorMaxValue[i]->SetMaxValue(100.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(5.0f);   //5.0m
          }
          break;

        case MEASURED_VALUE_LEVEL_PRESSURE:
          mpConfSensorMinValue[i]->SetQuantity(Q_HEIGHT);
          mpConfSensorMinValue[i]->SetMaxValue(100.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0m
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_HEIGHT);
          mpConfSensorMaxValue[i]->SetMaxValue(100.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(5.0f);   //5.0m
          }
          break;

        case MEASURED_VALUE_MOTOR_CURRENT_PUMP_1:
        case MEASURED_VALUE_MOTOR_CURRENT_PUMP_2:
        case MEASURED_VALUE_MOTOR_CURRENT_PUMP_3:
        case MEASURED_VALUE_MOTOR_CURRENT_PUMP_4:
        case MEASURED_VALUE_MOTOR_CURRENT_PUMP_5:
        case MEASURED_VALUE_MOTOR_CURRENT_PUMP_6:
          mpConfSensorMinValue[i]->SetQuantity(Q_HIGH_CURRENT);
          mpConfSensorMinValue[i]->SetMaxValue(1000.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); //0 A
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_HIGH_CURRENT);
          mpConfSensorMaxValue[i]->SetMaxValue(1000.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(10.0f); //10.0 A
          }
          break;

        case MEASURED_VALUE_WATER_IN_OIL_PUMP_1:
        case MEASURED_VALUE_WATER_IN_OIL_PUMP_2:
        case MEASURED_VALUE_WATER_IN_OIL_PUMP_3:
        case MEASURED_VALUE_WATER_IN_OIL_PUMP_4:
        case MEASURED_VALUE_WATER_IN_OIL_PUMP_5:
        case MEASURED_VALUE_WATER_IN_OIL_PUMP_6:
          mpConfSensorMinValue[i]->SetQuantity(Q_PERCENT);
          mpConfSensorMinValue[i]->SetMaxValue(100.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); // 0 %
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_PERCENT);
          mpConfSensorMaxValue[i]->SetMaxValue(100.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(20.0f); // 20.0 %
          }
          break;

        case MEASURED_VALUE_INSULATION_RESISTANCE:
          mpConfSensorMinValue[i]->SetQuantity(Q_RESISTANCE);
          mpConfSensorMinValue[i]->SetMaxValue(10000000.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); // 0 ohm
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_RESISTANCE);
          mpConfSensorMaxValue[i]->SetMaxValue(10000000.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(10000000.0f); // 10,000,000 ohm
          }
          break;

        case MEASURED_VALUE_POWER:
        case MEASURED_VALUE_POWER_PUMP_1:
        case MEASURED_VALUE_POWER_PUMP_2:
        case MEASURED_VALUE_POWER_PUMP_3:
        case MEASURED_VALUE_POWER_PUMP_4:
        case MEASURED_VALUE_POWER_PUMP_5:
        case MEASURED_VALUE_POWER_PUMP_6:
          mpConfSensorMinValue[i]->SetQuantity(Q_POWER);
          mpConfSensorMinValue[i]->SetMaxValue(1000000.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); // 0 W
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_POWER);
          mpConfSensorMaxValue[i]->SetMaxValue(1000000.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(10000.0f); // 10 kW
          }
          break;

        case MEASURED_VALUE_USER_DEFINED_SOURCE_1:
        case MEASURED_VALUE_USER_DEFINED_SOURCE_2:
        case MEASURED_VALUE_USER_DEFINED_SOURCE_3:
          mpConfSensorMinValue[i]->SetQuantity(Q_PERCENT);
          mpConfSensorMinValue[i]->SetMaxValue(100.0f);
          mpConfSensorMinValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set min.
          {
            mpConfSensorMinValue[i]->SetValue(0); // 0 %
          }

          mpConfSensorMaxValue[i]->SetQuantity(Q_PERCENT);
          mpConfSensorMaxValue[i]->SetMaxValue(100.0f);
          mpConfSensorMaxValue[i]->SetMinValue(0);
          if (mpAnaInSetupFromGeniFlag->GetValue() == false)  // the change is not arisen from GENI, so set max.
          {
            mpConfSensorMaxValue[i]->SetValue(100.0f); // 20.0 %
          }
          break;

        default:
          FatalErrorOccured("AnaInMeasureValueCtrl: measured value type not supported");
          break;
      }

      mpConfSensorMinValue[i]->SetWritable();
      mpConfSensorMaxValue[i]->SetWritable();
    }

    //electrical type changed?
    if (mpConfSensorElectric[i].IsUpdated())
    {
      if (mpConfSensorElectric[i]->GetValue() == SENSOR_ELECTRIC_DISABLED)
      {
        mpConfMeasuredValue[i]->SetValuePure(MEASURED_VALUE_NOT_SELECTED);
        //We use SetValuePure, cause this does not lead to setting quality to DP_AVAILABLE
        //measured value type may DP_NEVER_AVAILABLE in case ai is on a not installed io module
        //so SetValuePure enables us to hack this difficult design of ai's and their relation
        //to measured values
      }
    }
  }   //for ( i= 0; i<AIMVC_NO_OF_ANA_IN ;i++)
  mpAnaInSetupFromGeniFlag->SetValue(false);

  if (mSensorConfigUpdated)
  {
    //now we need to:
    //  - check AI's for redundant sensor
    //  - check those redundant for consistency (and issue warnings if needed)
    //  - update measured values
    //  - update AI measured values for electrical overview picture
    //  - update sensor output
    mSensorConfigUpdated = false;

    for (int i = 0; i < AIMVC_NO_OF_ANA_IN; i++)
    {
      mRedundantPartner[i] = -1;
      if (i < NO_OF_REDUNDANCY_TIMERS)
      {
        ResetRedundancyStateMachine(i);   //in case it was previously engaged in redundancy
      }
    }

    for (int i = FIRST_MEASURED_VALUE; i < NO_OF_MEASURED_VALUE; i++)
    {
      bool measured_value_found = false;
      for (int j = 0; j < AIMVC_NO_OF_ANA_IN; j++)
      {
        if (mpConfMeasuredValue[j]->GetValue() == i)   //if this AI has this type of sensor
        {
          if (mpConfMeasuredValue[j]->GetValue() == MEASURED_VALUE_NOT_SELECTED)   //no sensor on this AI
          {
            DisableMeasuredValueAI(j);   //disable this AI in electrical overview
          }
          else   //the AI has a sensor, we need to handle it
          {
            measured_value_found = true;
            if (mRedundantPartner[j] == -1)   //this AI has not yet been investigated for presence of a redundant sensor
            {
              for ( int k = j + 1; k < AIMVC_NO_OF_ANA_IN; k++)
              {
                if ((int)(mpConfMeasuredValue[k]->GetValue()) == i)   //redundant sensor found, hurray!
                {
                  if ( mRedundantPartner[j] == -1)   //if the AI has not yet been investigated, mark it and its partner to point at each other
                  {
                    mRedundantPartner[j] = k;
                    mRedundantPartner[k] = j;

                    HandleRedundantSensor(j, k, i);
                  }
                  else
                  //AI no. k is the same measured value as AI no. i, but a redundant sensor was already found once for AI no. i!
                  //we mark this sensor, so that we'll know, that it shall not be used for updating any measured value
                  {
                    mRedundantPartner[k] = -10;   //the user has been very naughty!
                  }
                }
              }   //for ( int k = j + 1; k < AIMVC_NO_OF_ANA_IN; k++)
            }
            if ( mRedundantPartner[j] == -1)   //we didn't find a redundant sensor for the valid sensor j, just use it as is
            {
              ResetRedundancyStateMachine(j);   //in case it was previously engaged in redundancy
              HandleNotRedundantSensor(j, i);
            }

            EnableMeasuredValueAI(j);   //enable this AI in electrical overview

          }   //mpMeasuredValueTypeFromAI[j]->GetValue() != NOT_SELECTED
        }
      }   //for ( int j = 0; j < AIMVC_NO_OF_ANA_IN; j++)

      if (!measured_value_found)  //we didn't find this measured value type on any AI
      {
        if (mpOutputList[i].IsValid())   //handle a null pointer
        {
          mpOutputList[i]->SetQuality(DP_NEVER_AVAILABLE);
        }
      }
    }   //for ( int i = 0; i < (LAST_MEASURED_VALUE + 1); i++)
  }

  if (mAnaInValueUpdated)
  {
    bool redundant_partner_checked[AIMVC_NO_OF_ANA_IN] = {false, false, false, false, false, false, false, false, false};

    mAnaInValueUpdated = false;

    for (int i = 0; i < AIMVC_NO_OF_ANA_IN; i++)
    {
      if (mpAnaInValue[i].IsUpdated())
      {
        if (mpConfMeasuredValue[i]->GetValue() != MEASURED_VALUE_NOT_SELECTED)   //only if there is a sensor on this AI
        {
          if (mRedundantPartner[i] >= 0)   //the sensor is redundant
          {
            if (!redundant_partner_checked[i])   //make sure handling of sensor and redundant partner is only performed once
            {
              redundant_partner_checked[mRedundantPartner[i]] = true;
              HandleRedundantSensor(i, mRedundantPartner[i], mpConfMeasuredValue[i]->GetValue());
            }
          }
          else if ( mRedundantPartner[i] == -1)   //the sensor is not redundant
          {
            UpdateNotRedundantSensor(i, mpConfMeasuredValue[i]->GetValue());
          }
          else   //redundandy to be ignored - don't do anything!
          {
            //do nothing
          }

          WriteToMeasuredValueAI(i);
        }
      }
    }

    mpAnalogValuesMeasuredFlag->SetValue(true);  // The analog values have been distributed - now other tasks can use them.
  }

  if (mHandleRedundancyTimers)
  {
    mHandleRedundancyTimers = false;
    for (int i = 0; i <= REDUNDANT_SENSOR_WARNING_TIMER_6 - REDUNDANT_SENSOR_WARNING_TIMER_1; i++)
    {
      if (mListOfRedundancyTimerEvents[i])
      {
        mListOfRedundancyTimerEvents[i] = false;
        HandleRedundancyStateMachineEvents(i);
        HandleRedundantSensor(i, mRedundantPartner[i], mpConfMeasuredValue[i]->GetValue());
      }
    }
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void AnaInMeasureValueCtrl::Update(Subject* pSubject)
{
  bool update_found = false;

  mpAnaInSetupFromGeniFlag.Update(pSubject);

  for (int i= 0; i < AIMVC_NO_OF_ANA_IN; i++)
  {
    if (mpAnaInValue[i].Update(pSubject))
    {
      mAnaInValueUpdated = true;
      update_found = true;
      break;   //no need to waste anymore time in loop
    }
  }

  if (!update_found)   //check the rest
  {
    update_found = false;

    for (int i = 0; i < NO_OF_REDUNDANCY_TIMERS; i++)
    {
      if (pSubject == mpTimerObjList[i + REDUNDANT_SENSOR_WARNING_TIMER_1])
      {
        mListOfRedundancyTimerEvents[i] = true;
        mHandleRedundancyTimers = true;
        update_found = true;
        break;   //no need to waste anymore time in loop
      }
    }

    if( !update_found)   //check the rest of the rest
    {
      for (int i= 0; i < AIMVC_NO_OF_ANA_IN; i++)
      {
        if (mpConfMeasuredValue[i].Update(pSubject))
        {
          mSensorConfigUpdated = true;
          break;   //no need to waste anymore time in loop
        }
        else if (mpConfSensorMinValue[i].Update(pSubject))
        {
          mSensorConfigUpdated = true;
          mpConfSensorMinValue[i].ResetUpdated();
          break;   //no need to waste anymore time in loop
        }
        else if (mpConfSensorMaxValue[i].Update(pSubject))
        {
          mSensorConfigUpdated = true;
          mpConfSensorMaxValue[i].ResetUpdated();
          break;   //no need to waste anymore time in loop
        }
        else if (mpConfSensorElectric[i].Update(pSubject))
        {
          mSensorConfigUpdated = true;
          break;   //no need to waste anymore time in loop
        }
      }
    }
  }

  if (!mReqTaskTime)
  {
    mReqTaskTime = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION : Set pointers to sub.
 *
 *****************************************************************************/
void AnaInMeasureValueCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    // redundant sensor error objects
    case SP_AIMVC_ANA_IN_1_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_REDUNDANT_SENSOR_ALARM_OBJ:
      mpRedundantSensorWarningObj[7].Attach(pSubject);
      break;

    // analog input 1: value, configuration and measured value
    case SP_AIMVC_ANA_IN_1_VALUE:
      mpAnaInValue[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_1_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_1_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_1_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_1_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_1_MEASURED_VALUE:
      mpMeasuredValueAI[0].Attach(pSubject);
      break;

    // analog input 2: value, configuration and measured value
    case SP_AIMVC_ANA_IN_2_VALUE:
      mpAnaInValue[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_MEASURED_VALUE:
      mpMeasuredValueAI[1].Attach(pSubject);
      break;

    // analog input 3: value, configuration and measured value
    case SP_AIMVC_ANA_IN_3_VALUE:
      mpAnaInValue[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_MEASURED_VALUE:
      mpMeasuredValueAI[2].Attach(pSubject);
      break;

    // analog input 4: value, configuration and measured value
    case SP_AIMVC_ANA_IN_4_VALUE:
      mpAnaInValue[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_MEASURED_VALUE:
      mpMeasuredValueAI[3].Attach(pSubject);
      break;

    // analog input 5: value, configuration and measured value
    case SP_AIMVC_ANA_IN_5_VALUE:
      mpAnaInValue[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_MEASURED_VALUE:
      mpMeasuredValueAI[4].Attach(pSubject);
      break;

    // analog input 6: value, configuration and measured value
    case SP_AIMVC_ANA_IN_6_VALUE:
      mpAnaInValue[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_MEASURED_VALUE:
      mpMeasuredValueAI[5].Attach(pSubject);
      break;

    // analog input 7: value, configuration and measured value
    case SP_AIMVC_ANA_IN_7_VALUE:
      mpAnaInValue[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_MEASURED_VALUE:
      mpMeasuredValueAI[6].Attach(pSubject);
      break;

    // analog input 8: value, configuration and measured value
    case SP_AIMVC_ANA_IN_8_VALUE:
      mpAnaInValue[7].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[7].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[7].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[7].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[7].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_MEASURED_VALUE:
      mpMeasuredValueAI[7].Attach(pSubject);
      break;

    // analog input 9: value, configuration and measured value
    case SP_AIMVC_ANA_IN_9_VALUE:
      mpAnaInValue[8].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_9_CONF_MEASURED_VALUE:
      mpConfMeasuredValue[8].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_9_CONF_SENSOR_ELECTRIC:
      mpConfSensorElectric[8].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_9_CONF_SENSOR_MIN_VALUE:
      mpConfSensorMinValue[8].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_9_CONF_SENSOR_MAX_VALUE:
      mpConfSensorMaxValue[8].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_9_MEASURED_VALUE:
      mpMeasuredValueAI[8].Attach(pSubject);
      break;


    // Electrical values
    case SP_AIMVC_ANA_IN_1_ELECTRICAL_VALUE:
      mpElectricalValueAI[0].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_2_ELECTRICAL_VALUE:
      mpElectricalValueAI[1].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_3_ELECTRICAL_VALUE:
      mpElectricalValueAI[2].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_4_ELECTRICAL_VALUE:
      mpElectricalValueAI[3].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_5_ELECTRICAL_VALUE:
      mpElectricalValueAI[4].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_6_ELECTRICAL_VALUE:
      mpElectricalValueAI[5].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_7_ELECTRICAL_VALUE:
      mpElectricalValueAI[6].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_8_ELECTRICAL_VALUE:
      mpElectricalValueAI[7].Attach(pSubject);
      break;
    case SP_AIMVC_ANA_IN_9_ELECTRICAL_VALUE:
      mpElectricalValueAI[8].Attach(pSubject);
      break;

    //Measured values
    case SP_AIMVC_MEASURED_VALUE_LEVEL_ULTRA_SOUND:
      mpOutputList[MEASURED_VALUE_LEVEL_ULTRA_SOUND].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_LEVEL_PRESSURE:
      mpOutputList[MEASURED_VALUE_LEVEL_PRESSURE].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_OUTLET_PRESSURE:
      mpOutputList[MEASURED_VALUE_OUTLET_PRESSURE].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_FLOW:
      mpOutputList[MEASURED_VALUE_FLOW].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER:
      mpOutputList[MEASURED_VALUE_POWER].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER_PUMP_1:
      mpOutputList[MEASURED_VALUE_POWER_PUMP_1].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER_PUMP_2:
      mpOutputList[MEASURED_VALUE_POWER_PUMP_2].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER_PUMP_3:
      mpOutputList[MEASURED_VALUE_POWER_PUMP_3].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER_PUMP_4:
      mpOutputList[MEASURED_VALUE_POWER_PUMP_4].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER_PUMP_5:
      mpOutputList[MEASURED_VALUE_POWER_PUMP_5].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_POWER_PUMP_6:
      mpOutputList[MEASURED_VALUE_POWER_PUMP_6].Attach(pSubject);
      break;

    case SP_AIMVC_MEASURED_VALUE_MOTOR_CURRENT_PUMP_1:
      mpOutputList[MEASURED_VALUE_MOTOR_CURRENT_PUMP_1].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_MOTOR_CURRENT_PUMP_2:
      mpOutputList[MEASURED_VALUE_MOTOR_CURRENT_PUMP_2].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_MOTOR_CURRENT_PUMP_3:
      mpOutputList[MEASURED_VALUE_MOTOR_CURRENT_PUMP_3].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_MOTOR_CURRENT_PUMP_4:
      mpOutputList[MEASURED_VALUE_MOTOR_CURRENT_PUMP_4].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_MOTOR_CURRENT_PUMP_5:
      mpOutputList[MEASURED_VALUE_MOTOR_CURRENT_PUMP_5].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_MOTOR_CURRENT_PUMP_6:
      mpOutputList[MEASURED_VALUE_MOTOR_CURRENT_PUMP_6].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_WATER_IN_OIL_PUMP_1:
      mpOutputList[MEASURED_VALUE_WATER_IN_OIL_PUMP_1].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_WATER_IN_OIL_PUMP_2:
      mpOutputList[MEASURED_VALUE_WATER_IN_OIL_PUMP_2].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_WATER_IN_OIL_PUMP_3:
      mpOutputList[MEASURED_VALUE_WATER_IN_OIL_PUMP_3].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_WATER_IN_OIL_PUMP_4:
      mpOutputList[MEASURED_VALUE_WATER_IN_OIL_PUMP_4].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_WATER_IN_OIL_PUMP_5:
      mpOutputList[MEASURED_VALUE_WATER_IN_OIL_PUMP_5].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_WATER_IN_OIL_PUMP_6:
      mpOutputList[MEASURED_VALUE_WATER_IN_OIL_PUMP_6].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_USER_DEFINED_SOURCE_1:
      mpOutputList[MEASURED_VALUE_USER_DEFINED_SOURCE_1].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_USER_DEFINED_SOURCE_2:
      mpOutputList[MEASURED_VALUE_USER_DEFINED_SOURCE_2].Attach(pSubject);
      break;
    case SP_AIMVC_MEASURED_VALUE_USER_DEFINED_SOURCE_3:
      mpOutputList[MEASURED_VALUE_USER_DEFINED_SOURCE_3].Attach(pSubject);
      break;

    case SP_AIMVC_ANA_IN_SETUP_FROM_GENI_FLAG:
      mpAnaInSetupFromGeniFlag.Attach(pSubject);
      break;

    case SP_AIMVC_ANALOG_VALUES_MEASURED:
      mpAnalogValuesMeasuredFlag.Attach(pSubject);
      break;

    default:
      break;
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInMeasureValueCtrl::ConnectToSubjects()
{
  for ( int i = 0; i < AIMVC_NO_OF_ANA_IN; i++)
  {
    mpAnaInValue[i]->Subscribe(this);
    mpConfMeasuredValue[i]->Subscribe(this);
    mpConfSensorElectric[i]->Subscribe(this);
    mpConfSensorMinValue[i]->Subscribe(this);
    mpConfSensorMaxValue[i]->Subscribe(this);

    mpAnaInValue[i].SetUpdated(); // initial update
  }
  mpAnaInSetupFromGeniFlag->Subscribe(this);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInMeasureValueCtrl::SubscribtionCancelled(Subject* pSubject)
{
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *              IsThereToBigDifferenceWithRedundantSensor
 *
 *
 ****************************************************************************/
bool AnaInMeasureValueCtrl::IsThereToBigDifferenceBetween2RedundantSensors(FloatDataPoint* pSensor1, FloatDataPoint* pSensor2)
{
  bool to_big_diff = false;

  FloatDataPoint* p_sensor_with_widest_range;
  if((pSensor1->GetMaxValue() - pSensor1->GetMinValue()) > (pSensor2->GetMaxValue() - pSensor2->GetMinValue()))
  {
    p_sensor_with_widest_range = pSensor1;
  }
  else
  {
    p_sensor_with_widest_range = pSensor2;
  }

  //align so that calculation can be performed in a range 0 - [positive number]
  float range = p_sensor_with_widest_range->GetMaxValue() - p_sensor_with_widest_range->GetMinValue();

  float min, max;
  if ( pSensor1->GetValue() > pSensor2->GetValue())
  {
    max = pSensor1->GetValue() - p_sensor_with_widest_range->GetMinValue();
    min = pSensor2->GetValue() - p_sensor_with_widest_range->GetMinValue();
  }
  else
  {
    min = pSensor1->GetValue() - p_sensor_with_widest_range->GetMinValue();
    max = pSensor2->GetValue() - p_sensor_with_widest_range->GetMinValue();
  }

  if ( max == 0.0f)   //It shouldn't be, but don't risc division with 0!
  {
    if ( (max - min) > range * 0.02f)   //absolute difference is too big
    {
      to_big_diff = true;
    }
  }
  else if ( min / max <= 0.90f)   //difference in percentage is to big
  {
    if ( (max - min) > range * 0.02f)   //absolute difference is too big
    {
      to_big_diff = true;
    }
  }

  return to_big_diff;
}

/*****************************************************************************
 *              HandleRedundantSensor
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::HandleRedundantSensor(unsigned int AnaInFirst, unsigned int AnaInSecond, const int MeasuredValueNo)
{
  if ( AnaInFirst < AIMVC_NO_OF_ANA_IN && AnaInSecond < AIMVC_NO_OF_ANA_IN)   //don't handle illegal AI
  {
    if (AnaInFirst > AnaInSecond)   //make sure AnaInFirst has the lowest AI no.
    {
      int temp = AnaInFirst;
      AnaInFirst = AnaInSecond;
      AnaInSecond = temp;
    }

    //TEST JLY: for ttp #992
    //prepare erroneous unit number for warning object:
    //err. unit number is used in the following way:
    // - 3 digit (decimal are used)
    // - 1. (left most) digit is 1 (100)
    // - 2. digit is no. of first (and lowest) AI starting with no. 1
    // - 3. digit is no. of second (and highest) AI
    // NB: This design leaves room for a maximum of 9 AI, starting with AI 0
    mpRedundantSensorWarningObj[AnaInFirst]->SetErroneousUnitNumber(100 + (AnaInFirst + 1) * 10 + AnaInSecond + 1 );

    FloatDataPoint sensor_1;
    FloatDataPoint* p_sensor_1 = &sensor_1;
    p_sensor_1->SetQuantity(mpConfSensorMinValue[AnaInFirst]->GetQuantity());
    p_sensor_1->SetMinValue(mpConfSensorMinValue[AnaInFirst]->GetValue());
    p_sensor_1->SetMaxValue(mpConfSensorMaxValue[AnaInFirst]->GetValue());
    p_sensor_1->SetRedundancy(true);
    if (mpAnaInValue[AnaInFirst]->GetQuality() == DP_NOT_AVAILABLE)
    {
      p_sensor_1->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      p_sensor_1->SetValueAsPercent(mpAnaInValue[AnaInFirst]->GetValue());
    }

    FloatDataPoint sensor_2;
    FloatDataPoint* p_sensor_2 = &sensor_2;
    p_sensor_2->SetQuantity(mpConfSensorMinValue[AnaInSecond]->GetQuantity());
    p_sensor_2->SetMinValue(mpConfSensorMinValue[AnaInSecond]->GetValue());
    p_sensor_2->SetMaxValue(mpConfSensorMaxValue[AnaInSecond]->GetValue());
    p_sensor_2->SetRedundancy(true);
    if (mpAnaInValue[AnaInSecond]->GetQuality() == DP_NOT_AVAILABLE)
    {
      p_sensor_2->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      p_sensor_2->SetValueAsPercent(mpAnaInValue[AnaInSecond]->GetValue());
    }

    //handle sensor electrical failure if any
    if ( p_sensor_1->GetQuality() == DP_NOT_AVAILABLE || p_sensor_2->GetQuality() == DP_NOT_AVAILABLE)   //at least one has elec. failure
    {
      ResetRedundancyStateMachine(AnaInFirst);

      if (p_sensor_1->GetQuality() == DP_AVAILABLE)
      {
        if (mpOutputList[MeasuredValueNo].IsValid())   //handle a null pointer
        {
          mpOutputList[MeasuredValueNo]->CopyValues(p_sensor_1);   //use this sensor
        }
      }
      else if (p_sensor_2->GetQuality() == DP_AVAILABLE)
      {
        if (mpOutputList[MeasuredValueNo].IsValid())   //handle a null pointer
        {
          mpOutputList[MeasuredValueNo]->CopyValues(p_sensor_2);   //use this sensor
        }
      }
      else
      {
        if (mpOutputList[MeasuredValueNo].IsValid())   //handle a null pointer
        {
          mpOutputList[MeasuredValueNo]->CopyValues(p_sensor_1);   //use this sensor, since we have to use one, even though both are not available
        }
      }
    }
    else   //there was no electrical fail - then we must apply the redundancy check
    {
      switch (mRedundantErrorStatus[AnaInFirst])
      {
        case NO_REDUNDANT_ERROR:
          if (IsThereToBigDifferenceBetween2RedundantSensors(p_sensor_1, p_sensor_2) == true)
          {
            mpTimerObjList[AnaInFirst + REDUNDANT_SENSOR_WARNING_TIMER_1]->RetriggerSwTimer();
            mRedundantErrorStatus[AnaInFirst] = CHECK_REDUNDANT_ERROR;
          }
          break;
        case CHECK_REDUNDANT_ERROR:
          if (IsThereToBigDifferenceBetween2RedundantSensors(p_sensor_1, p_sensor_2) == true)
          {
            //just wait for timeout
          }
          else
          {
             mpTimerObjList[AnaInFirst + REDUNDANT_SENSOR_WARNING_TIMER_1]->StopSwTimer();
             mRedundantErrorStatus[AnaInFirst] = NO_REDUNDANT_ERROR;
          }
          break;
        case CHECK_REDUNDANT_OK:
          if (IsThereToBigDifferenceBetween2RedundantSensors(p_sensor_1, p_sensor_2) == true)
          {
            mpTimerObjList[AnaInFirst + REDUNDANT_SENSOR_WARNING_TIMER_1]->StopSwTimer();
            mRedundantErrorStatus[AnaInFirst] = REDUNDANT_ERROR;
          }
          break;
        case REDUNDANT_ERROR:
          if (IsThereToBigDifferenceBetween2RedundantSensors(p_sensor_1, p_sensor_2) == true)
          {
            //do nothing - just stay in this state
          }
          else   //try to go back to ok
          {
             mpTimerObjList[AnaInFirst + REDUNDANT_SENSOR_WARNING_TIMER_1]->RetriggerSwTimer();
             mRedundantErrorStatus[AnaInFirst] = CHECK_REDUNDANT_OK;
          }
          break;
        default:   //should never happen
          mRedundantErrorStatus[AnaInFirst] = NO_REDUNDANT_ERROR;
          break;
      }

      if (mpOutputList[MeasuredValueNo].IsValid())   //handle a null pointer
      {
        mpOutputList[MeasuredValueNo]->CopyValues(p_sensor_1);   //use this sensor, inconsistency or not - it has the lowest AI no.
      }
    }
  }
}

/*****************************************************************************
 *              HandleNotRedundantSensor
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::HandleNotRedundantSensor(const unsigned int AnaInNo, const int MeasuredValueNo)
{
  if (AnaInNo < AIMVC_NO_OF_ANA_IN)   //don't handle illegal AI
  {
    if (mpOutputList[MeasuredValueNo].IsValid())   //handle a null pointer
    {
      mpOutputList[MeasuredValueNo]->SetQuantity(mpConfSensorMinValue[AnaInNo]->GetQuantity());
      mpOutputList[MeasuredValueNo]->SetMinValue(mpConfSensorMinValue[AnaInNo]->GetValue());
      mpOutputList[MeasuredValueNo]->SetMaxValue(mpConfSensorMaxValue[AnaInNo]->GetValue());
      mpOutputList[MeasuredValueNo]->SetRedundancy(false);
    }
  }

  UpdateNotRedundantSensor(AnaInNo, MeasuredValueNo);
}

/*****************************************************************************
 *              UpdateNotRedundantSensor
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::UpdateNotRedundantSensor(const unsigned int AnaInNo, const int MeasuredValueNo)
{
  if (AnaInNo < AIMVC_NO_OF_ANA_IN)   //don't handle illegal AI
  {
    if (mpOutputList[MeasuredValueNo].IsValid())   //handle a null pointer
    {
      if (mpAnaInValue[AnaInNo]->GetQuality() == DP_NOT_AVAILABLE)
      {
        mpOutputList[MeasuredValueNo]->SetQuality(DP_NOT_AVAILABLE);
      }
      else
      {
        mpOutputList[MeasuredValueNo]->SetValueAsPercent(mpAnaInValue[AnaInNo]->GetValue());
      }
    }
  }
}

/*****************************************************************************
 *              DisableMeasuredValueAI
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::DisableMeasuredValueAI(const unsigned int AnaInNo)
{
  if (AnaInNo < AIMVC_NO_OF_ANA_IN)   //don't handle illegal AI
  {
    mpMeasuredValueAI[AnaInNo]->SetQuantity(Q_NO_UNIT);
    mpMeasuredValueAI[AnaInNo]->SetQuality(DP_NOT_AVAILABLE);

    mpElectricalValueAI[AnaInNo]->SetQuantity(Q_NO_UNIT);
    mpElectricalValueAI[AnaInNo]->SetQuality(DP_NOT_AVAILABLE);
  }
}

/*****************************************************************************
 *              EnableMeasuredValueAI
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::EnableMeasuredValueAI(const unsigned int AnaInNo)
{
  if (AnaInNo < AIMVC_NO_OF_ANA_IN)   //don't handle illegal AI
  {
    mpMeasuredValueAI[AnaInNo]->SetQuantity(mpConfSensorMinValue[AnaInNo]->GetQuantity());
    mpMeasuredValueAI[AnaInNo]->SetMinValue(mpConfSensorMinValue[AnaInNo]->GetValue());
    mpMeasuredValueAI[AnaInNo]->SetMaxValue(mpConfSensorMaxValue[AnaInNo]->GetValue());

    switch( mpConfSensorElectric[AnaInNo]->GetValue() )
    {
    case SENSOR_ELECTRIC_TYPE_0_10V:
      mpElectricalValueAI[AnaInNo]->SetMinValue(0);
      mpElectricalValueAI[AnaInNo]->SetMaxValue(10);
      mpElectricalValueAI[AnaInNo]->SetQuantity(Q_VOLTAGE);
      break;
    case SENSOR_ELECTRIC_TYPE_2_10V:
      mpElectricalValueAI[AnaInNo]->SetMinValue(2);
      mpElectricalValueAI[AnaInNo]->SetMaxValue(10);
      mpElectricalValueAI[AnaInNo]->SetQuantity(Q_VOLTAGE);
      break;
    case SENSOR_ELECTRIC_TYPE_0_20mA:
      mpElectricalValueAI[AnaInNo]->SetMinValue(0.000);
      mpElectricalValueAI[AnaInNo]->SetMaxValue(0.020);
      mpElectricalValueAI[AnaInNo]->SetQuantity(Q_LOW_CURRENT);
      break;
    case SENSOR_ELECTRIC_TYPE_4_20mA:
      mpElectricalValueAI[AnaInNo]->SetMinValue(0.004);
      mpElectricalValueAI[AnaInNo]->SetMaxValue(0.020);
      mpElectricalValueAI[AnaInNo]->SetQuantity(Q_LOW_CURRENT);
      break;
    case SENSOR_ELECTRIC_DISABLED:
      break;

    }

    WriteToMeasuredValueAI(AnaInNo);
  }
}

/*****************************************************************************
 *              WriteToMeasuredValueAI
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::WriteToMeasuredValueAI(const unsigned int AnaInNo)
{
  if (AnaInNo < AIMVC_NO_OF_ANA_IN)
  {
    if (mpAnaInValue[AnaInNo]->GetQuality() == DP_NOT_AVAILABLE)   //electrical fault
    {
      mpMeasuredValueAI[AnaInNo]->SetQuality(DP_NOT_AVAILABLE);
      mpElectricalValueAI[AnaInNo]->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      SUBJECT_ID_TYPE id = mpMeasuredValueAI[AnaInNo]->GetSubjectId(); // DEBUG should be removed
      mpMeasuredValueAI[AnaInNo]->SetValueAsPercent(mpAnaInValue[AnaInNo]->GetValue());
      mpElectricalValueAI[AnaInNo]->SetValueAsPercent(mpAnaInValue[AnaInNo]->GetValue());
    }

  }
}

/*****************************************************************************
 *              ResetRedundancyStateMachine
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::ResetRedundancyStateMachine(const unsigned int RedundantTimerNo)
{
  if (RedundantTimerNo < NO_OF_REDUNDANCY_TIMERS)
  {
    mRedundantErrorStatus[RedundantTimerNo] = NO_REDUNDANT_ERROR;
    mpRedundantSensorWarningObj[RedundantTimerNo]->SetErrorPresent(ALARM_STATE_READY);   //terminate hanging alarm
  }
}

/*****************************************************************************
 *              HandleRedundancyStateMachineEvents
 *
 *
 ****************************************************************************/
void AnaInMeasureValueCtrl::HandleRedundancyStateMachineEvents(const unsigned int RedundantTimerNo)
{
  //handle state transitions and action of the redundancy timer state machine
  switch ( mRedundantErrorStatus[RedundantTimerNo])
  {
    case NO_REDUNDANT_ERROR:   //there should be no timer events in this state
      break;
    case CHECK_REDUNDANT_ERROR:
      mRedundantErrorStatus[RedundantTimerNo] = REDUNDANT_ERROR;
// XHE test: Der skal tages stilling til om redundant sensor skal understøttes !!
//    mpRedundantSensorWarningObj[RedundantTimerNo]->SetErrorPresent(ALARM_STATE_ALARM);
      break;
    case REDUNDANT_ERROR:   //there should be no timer events in this state
      break;
    case CHECK_REDUNDANT_OK:
      mRedundantErrorStatus[RedundantTimerNo] = NO_REDUNDANT_ERROR;
      mpRedundantSensorWarningObj[RedundantTimerNo]->SetErrorPresent(ALARM_STATE_READY);
      break;
    default:
      //illegal!
      break;
  }
}

/*****************************************************************************
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
