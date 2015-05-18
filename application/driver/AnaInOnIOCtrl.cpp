/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AnaInOnIOCtrl                                         */
/*                                                                          */
/* FILE NAME        : AnaInOnIOCtrl.CPP                                     */
/*                                                                          */
/* CREATED DATE     : 17-02-2005                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <SwTimer.h>
#include <FactoryTypes.h>
#include <ErrorLog.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AnaInOnIOCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define Unit_for_0_20mA (1023.0f/23.5470f)   // UDTRYK FOR SP힟DINGSDELING I HW
#define Unit_for_4_20mA (1023.0f/23.5470f)   // UDTRYK FOR SP힟DINGSDELING I HW
#define Unit_for_0_10V  (1023.0f/11.5712f)   // UDTRYK FOR SP힟DINGSDELING I HW
#define Unit_for_2_10V  (1023.0f/11.5712f)   // UDTRYK FOR SP힟DINGSDELING I HW

/*****************************************************************************
 DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER,
  CHECK_NO_ADC_ERROR_DELAY_TIMER,
  CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER,
  HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER,
  HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER
};
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
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
AnaInOnIOCtrl::AnaInOnIOCtrl(const int noOnIoBoard) : mNoOnIoBoard(noOnIoBoard)
{

  if ((mNoOnIoBoard < 1) || (mNoOnIoBoard > 3))
  {
    FatalErrorOccured("AIOIOC index out of range!");
  }

  mpVoltageCurrentDOSelectorNo = 0;
  mpVoltageAINoOnIOBoard = 0;
  mpCurrentAINoOnIOBoard = 0;
  mpAnaInErrorObj = 0;

  mpAnaInErrorObj = new EnumDataPoint<AI_ALARM_ID_TYPE>();
  mpVoltageCurrentDOSelectorNo = new EnumDataPoint<IOB_DIG_OUT_NO_TYPE>();
  mpVoltageAINoOnIOBoard = new EnumDataPoint<IOB_ANA_IN_NO_TYPE>();
  mpCurrentAINoOnIOBoard = new EnumDataPoint<IOB_ANA_IN_NO_TYPE>();

  mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER] = new SwTimer(500, MS, FALSE, FALSE, this);
  mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER] = new SwTimer(500, MS, FALSE, FALSE, this);
  mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER] = new SwTimer(200, MS, FALSE, FALSE, this);
  mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER] = new SwTimer(500, MS, FALSE, FALSE, this);
  mpTimerObjList[HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER] = new SwTimer(50, MS, FALSE, FALSE, this);
}
/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInOnIOCtrl::InitSubTask()
{
  mpAnaInErrorObj->SetMaxValue(LAST_AI_ALARM_ID);
  mpAnaInErrorObj->SetMinValue(FIRST_AI_ALARM_ID);
  mpAnaInErrorObj->SetValue(AnaInOk);
  // Alarm from AnaIn is not used in WW midrange
  // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_READY);

  mpVoltageCurrentDOSelectorNo->SetMaxValue(LAST_IOB_DIG_OUT_NO);
  mpVoltageCurrentDOSelectorNo->SetMinValue(FIRST_IOB_DIG_OUT_NO);

  mpVoltageAINoOnIOBoard->SetMaxValue(LAST_IOB_ANA_IN_NO);
  mpVoltageAINoOnIOBoard->SetMinValue(FIRST_IOB_ANA_IN_NO);

  mpCurrentAINoOnIOBoard->SetMaxValue(LAST_IOB_ANA_IN_NO);
  mpCurrentAINoOnIOBoard->SetMinValue(FIRST_IOB_ANA_IN_NO);

  switch (mNoOnIoBoard)
  {
  case 1:
    mpVoltageCurrentDOSelectorNo->SetValue(IOB_DIG_OUT_NO_0);
    mpVoltageAINoOnIOBoard->SetValue(IOB_ANA_IN_NO_0);
    mpCurrentAINoOnIOBoard->SetValue(IOB_ANA_IN_NO_1);
    break;
  case 2:
    mpVoltageCurrentDOSelectorNo->SetValue(IOB_DIG_OUT_NO_1);
    mpVoltageAINoOnIOBoard->SetValue(IOB_ANA_IN_NO_2);
    mpCurrentAINoOnIOBoard->SetValue(IOB_ANA_IN_NO_3);
    break;
  case 3:
    mpVoltageCurrentDOSelectorNo->SetValue(IOB_DIG_OUT_NO_2);
    mpVoltageAINoOnIOBoard->SetValue(IOB_ANA_IN_NO_4);
    mpCurrentAINoOnIOBoard->SetValue(IOB_ANA_IN_NO_5);
    break;
  }

  if ( mpAnaInConfSensorElectric->GetValue() == SENSOR_ELECTRIC_TYPE_0_20mA)
  {
    mA = ((100.0f-0.0f)/((Unit_for_0_20mA * 20.0f)-(Unit_for_0_20mA * 0.0f)));
    mB = 0;
    IobComDrv::GetInstance()->SetDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
    mLowAlarmLimit_1 = 0;
    mLowAlarmLimit_2 = 0;
    mHighAlarmLimit_1 = (Unit_for_0_20mA * 21.0f);
    mHighAlarmLimit_2 = (Unit_for_0_20mA * 22.0f);
  }
  else if ( mpAnaInConfSensorElectric->GetValue() == SENSOR_ELECTRIC_TYPE_4_20mA)
  {
    mA = ((100.0f-0.0f)/((Unit_for_4_20mA * 20.0f)-(Unit_for_4_20mA * 4.0f)));
    mB = (-mA*(Unit_for_4_20mA * 4.0f));
    IobComDrv::GetInstance()->SetDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
    mLowAlarmLimit_1 = (Unit_for_4_20mA * 2.0f);
    mLowAlarmLimit_2 = (Unit_for_4_20mA * 3.0f);
    mHighAlarmLimit_1 = (Unit_for_4_20mA * 21.0f);
    mHighAlarmLimit_2 = (Unit_for_4_20mA * 22.0f);
  }
  else if ( mpAnaInConfSensorElectric->GetValue() == SENSOR_ELECTRIC_TYPE_0_10V)
  {
    mA = ((100-0)/((Unit_for_0_10V * 10.0f)-(Unit_for_0_10V * 0.0f)));
    mB = 0;
    IobComDrv::GetInstance()->ClearDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
    mLowAlarmLimit_1 = (Unit_for_0_10V * 0);
    mLowAlarmLimit_2 = (Unit_for_0_10V * 0);
    mHighAlarmLimit_1 = (Unit_for_0_10V * 10.5f);
    mHighAlarmLimit_2 = (Unit_for_0_10V * 11.0f);
  }
  else if ( mpAnaInConfSensorElectric->GetValue() == SENSOR_ELECTRIC_TYPE_2_10V)
  {
    mA = ((100-0)/((Unit_for_2_10V * 10.0f)-(Unit_for_2_10V * 2.0f)));
    mB = (-mA*(Unit_for_2_10V * 2.0f));
    IobComDrv::GetInstance()->ClearDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
    mLowAlarmLimit_1 = (Unit_for_2_10V * 1.0f);
    mLowAlarmLimit_2 = (Unit_for_2_10V * 1.5f);
    mHighAlarmLimit_1 = (Unit_for_2_10V * 10.5f);
    mHighAlarmLimit_2 = (Unit_for_2_10V * 11.0f);
  }

  else if ( mpAnaInConfSensorElectric->GetValue() == SENSOR_ELECTRIC_DISABLED)
  {
    IobComDrv::GetInstance()->ClearDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
    mpAnaInErrorObj->SetValue(AnaInOk);
    // Alarm from AnaIn is not used in WW midrange
    // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_READY);
  }
  mAdcAlarmStatus = NO_ADC_ERROR;

  mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
  mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
  mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
  mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->StopSwTimer();
  mpTimerObjList[HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER]->StopSwTimer();
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AnaInOnIOCtrl::~AnaInOnIOCtrl()
{
}
/*****************************************************************************
 * Function - RunSubTask()
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInOnIOCtrl::RunSubTask()
{
  U32 pure_adc_value;

  if ( mpAnaInConfSensorElectric.IsUpdated() || mpCurrentProtectionAllowed.IsUpdated() )
  {
    InitSubTask();
  }

  switch (mpAnaInConfSensorElectric->GetValue())
  {
  case SENSOR_ELECTRIC_DISABLED:
    pure_adc_value = 0;
    break;
  case SENSOR_ELECTRIC_TYPE_0_10V:
  case SENSOR_ELECTRIC_TYPE_2_10V:
    pure_adc_value = (IobComDrv::GetInstance()->GetAnalogInput(mpVoltageAINoOnIOBoard->GetValue()));
    break;
  case SENSOR_ELECTRIC_TYPE_0_20mA:
  case SENSOR_ELECTRIC_TYPE_4_20mA:
    pure_adc_value = (IobComDrv::GetInstance()->GetAnalogInput(mpCurrentAINoOnIOBoard->GetValue()));
    break;
  default:
    LogError(ERROR_LOG_ID_ANAINONIOCTRL_UNKNOWNSENSORELECTRIC);
  }

  if ( mpAnaInConfSensorElectric->GetValue() != SENSOR_ELECTRIC_DISABLED)
  {
    if ( mNewValueToDataPoint == true)
    {
      switch (mAdcAlarmStatus)
      {
        case LOW_CURRENT_ADC_ERROR :
          mpAnaInErrorObj->SetValue(AnaInUnderCurrent);
         // Alarm from AnaIn is not used in WW midrange
         // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_ALARM);
          break;
        case NO_ADC_ERROR :
          mpAnaInErrorObj->SetValue(AnaInOk);
          // Alarm from AnaIn is not used in WW midrange
          // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_READY);
          break;
        case HIGH_ADC_ERROR :
            if ( mpAnaInConfSensorElectric->GetValue() != SENSOR_ELECTRIC_TYPE_0_10V)
            {
              mpAnaInErrorObj->SetValue(AnaInOverCurrent);
              // Alarm from AnaIn is not used in WW midrange
              // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_ALARM);
            }
            else
            {
              mpAnaInErrorObj->SetValue(AnaInOverVoltage);
              // Alarm from AnaIn is not used in WW midrange
              // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_ALARM);
            }
          break;
        case WAIT_FOR_SET_BACK_TO_CURRENT :
          mpAnaInErrorObj->SetValue(AnaInOverCurrent);
          // Alarm from AnaIn is not used in WW midrange
          // mpAnaInAlarmObj->SetErrorPresent(ALARM_STATE_ALARM);
      }
      mNewValueToDataPoint = false;
    }
    if (mAdcAlarmStatus == WAIT_FOR_SET_BACK_TO_CURRENT)
    {
      IobComDrv::GetInstance()->SetDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
      mpTimerObjList[HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER]->StopSwTimer();
      mpTimerObjList[HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER]->RetriggerSwTimer();
      mpTimerObjList[HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER]->StartSwTimer();
      mAdcAlarmStatus = WAIT_FOR_STABLE_CURRENT;
    }
    else if ( pure_adc_value < (mLowAlarmLimit_1)) // -1
    {
      switch (mAdcAlarmStatus)
      {
        case NO_ADC_ERROR:
          mAdcAlarmStatus = CHECK_LOW_ADC_CURRENT_ERROR;
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
           break;
        case CHECK_LOW_ADC_CURRENT_ERROR:
          /* Empty */
          break;
        case CHECK_NO_ADC_ERROR:
          mAdcAlarmStatus = CHECK_LOW_ADC_CURRENT_ERROR;
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          break;
        case CHECK_HIGH_ADC_ERROR:
           mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
           mAdcAlarmStatus = CHECK_LOW_ADC_CURRENT_ERROR;
           mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
           mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
           mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case LOW_CURRENT_ADC_ERROR:
          /* Empty */
          break;
        case HIGH_ADC_ERROR:
          mAdcAlarmStatus = CHECK_LOW_ADC_CURRENT_ERROR;
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case CHECK_ADC_OVER_CURRENT:
          mAdcAlarmStatus = CHECK_LOW_ADC_CURRENT_ERROR;
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case HIGH_ADC_ERROR_CURRENT_PROTECTION:
          /* Empty */
          break;
      }
    }
    else if ( pure_adc_value < (mLowAlarmLimit_2)) // -2
    {
      switch (mAdcAlarmStatus)
      {
        case NO_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_LOW_ADC_CURRENT_ERROR:
          /* Empty */
          break;
        case CHECK_NO_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_HIGH_ADC_ERROR:
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          if (mpAnaInErrorObj->GetValue() == AnaInOk)
          {
            mAdcAlarmStatus = NO_ADC_ERROR;
          }
          else if (mpAnaInErrorObj->GetValue() != AnaInOk)
          {
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          }
          break;
        case LOW_CURRENT_ADC_ERROR:
          /* Empty */
          break;
        case HIGH_ADC_ERROR:
          mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case CHECK_ADC_OVER_CURRENT:
          if (mpAnaInErrorObj->GetValue() == AnaInOk)
          {
            mAdcAlarmStatus = NO_ADC_ERROR;
          }
          else if (mpAnaInErrorObj->GetValue() != AnaInOk)
          {
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          }
          break;
        case HIGH_ADC_ERROR_CURRENT_PROTECTION:
          /* Empty */
          break;
      }
    }
    else if ( pure_adc_value < (mHighAlarmLimit_1)) // -3
    {
      switch (mAdcAlarmStatus)
      {
        case NO_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_LOW_ADC_CURRENT_ERROR:
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          if (mpAnaInErrorObj->GetValue() == AnaInOk)
          {
            mAdcAlarmStatus = NO_ADC_ERROR;
          }
          else if (mpAnaInErrorObj->GetValue() != AnaInOk)
          {
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          }
          break;
        case CHECK_NO_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_HIGH_ADC_ERROR:
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          if (mpAnaInErrorObj->GetValue() == AnaInOk)
          {
            mAdcAlarmStatus = NO_ADC_ERROR;
          }
          else if (mpAnaInErrorObj->GetValue() != AnaInOk)
          {
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          }
          break;
        case LOW_CURRENT_ADC_ERROR:
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case HIGH_ADC_ERROR:
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case CHECK_ADC_OVER_CURRENT:
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case HIGH_ADC_ERROR_CURRENT_PROTECTION:
          /* Empty */
          break;
      }
    }
    else if ( pure_adc_value < (mHighAlarmLimit_2)) // -4
    {
      switch (mAdcAlarmStatus)
      {
        case NO_ADC_ERROR:
            /* Empty */
          break;
        case CHECK_LOW_ADC_CURRENT_ERROR:
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          if (mpAnaInErrorObj->GetValue() == AnaInOk)
          {
            mAdcAlarmStatus = NO_ADC_ERROR;
          }
          else if (mpAnaInErrorObj->GetValue() != AnaInOk)
          {
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
            mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
            mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          }
          break;
        case CHECK_NO_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_HIGH_ADC_ERROR:
          /* Empty */
          break;
        case LOW_CURRENT_ADC_ERROR:
          mAdcAlarmStatus = CHECK_NO_ADC_ERROR;
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          break;
        case HIGH_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_ADC_OVER_CURRENT:
          mAdcAlarmStatus = HIGH_ADC_ERROR_CURRENT_PROTECTION;
          if(mpCurrentProtectionAllowed->GetValue() == true)
          {
            IobComDrv::GetInstance()->ClearDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
          }
          mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->StartSwTimer();
          break;
        case HIGH_ADC_ERROR_CURRENT_PROTECTION:
          /* Empty */
          break;
      }
    }
    else //( pure_adc_value > (Unit_for_4_20mA * 21)) // -5
    {
      switch (mAdcAlarmStatus)
      {
        case NO_ADC_ERROR:
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          mAdcAlarmStatus = CHECK_HIGH_ADC_ERROR;
          break;
        case CHECK_LOW_ADC_CURRENT_ERROR:
          mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();

          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          mAdcAlarmStatus = CHECK_HIGH_ADC_ERROR;
          break;
        case CHECK_NO_ADC_ERROR:
          mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER]->StopSwTimer();

          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          mAdcAlarmStatus = CHECK_HIGH_ADC_ERROR;
          break;
        case CHECK_HIGH_ADC_ERROR:
          /* Empty */
          break;
        case LOW_CURRENT_ADC_ERROR:
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER]->StartSwTimer();
          mAdcAlarmStatus = CHECK_HIGH_ADC_ERROR;
          break;
        case HIGH_ADC_ERROR:
          /* Empty */
          break;
        case CHECK_ADC_OVER_CURRENT:
          mAdcAlarmStatus = HIGH_ADC_ERROR_CURRENT_PROTECTION;
          if(mpCurrentProtectionAllowed->GetValue() == true)
          {
            IobComDrv::GetInstance()->ClearDigitalOutput(mpVoltageCurrentDOSelectorNo->GetValue());
          }
          mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->StopSwTimer();
          mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->RetriggerSwTimer();
          mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER]->StartSwTimer();
          break;
        case HIGH_ADC_ERROR_CURRENT_PROTECTION:
          /* Empty */
          break;
      }
    }
    /**
    * IF THE DATA IS available -> mAvailable = TRUE
    * Change the ADC value to Percentage - to do that, a formel for the LINE is neaded.
    *  a = (y2-y1)/(x2-x1) -> a = (100-0)/((Unit_for_4_20_mA * 20)/(Unit_for_4_20_mA * 4))
    *  y = a*x + b
    */
    auto float value_to_set_in_dp;
    value_to_set_in_dp = ((float)((pure_adc_value * mA) + mB )); // change the value to a percentage.
    if ( value_to_set_in_dp < 0)
    {
      value_to_set_in_dp = 0;
    }

  // ERROR CORRECTION FINISHED... - READY TO SET THE RIGHT VALUE TO THE DATAPOINT
    if (mpAnaInErrorObj->GetValue() != AnaInOk)
    {
      mpAnaInValue->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      mpAnaInValue->SetValueAsPercent(value_to_set_in_dp);
    }
  }
}
/*****************************************************************************
 * Function - ConnectToSubjects()
 * DESCRIPTION:
 *
 *****************************************************************************/
 void AnaInOnIOCtrl::ConnectToSubjects()
 {
    mpAnaInConfSensorElectric->Subscribe(this); // the electrical type of the Ana In, incl. disabled
    mpCurrentProtectionAllowed->Subscribe(this);
 }

/*****************************************************************************
 * Function - update
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInOnIOCtrl::Update(Subject* pSubject)
{
  if ( pSubject == mpTimerObjList[CHECK_LOW_CURRENT_ADC_ERROR_DELAY_TIMER])
  {
    mAdcAlarmStatus = LOW_CURRENT_ADC_ERROR;
    mNewValueToDataPoint = true;
  }

  if ( pSubject == mpTimerObjList[CHECK_NO_ADC_ERROR_DELAY_TIMER])
  {
    mAdcAlarmStatus = NO_ADC_ERROR;
    mNewValueToDataPoint = true;
  }

  if (pSubject == mpTimerObjList[CHECK_HIGH_CURRENT_ADC_ERROR_DELAY_TIMER])
  {
    if (mpAnaInConfSensorElectric->GetValue() != SENSOR_ELECTRIC_TYPE_0_10V)
    {
      mAdcAlarmStatus = WAIT_FOR_SET_BACK_TO_CURRENT;
    }
    else
    {
      mAdcAlarmStatus = HIGH_ADC_ERROR;
    }
    mNewValueToDataPoint = true;
  }

  if ( pSubject == mpTimerObjList[HIGH_ADC_ERROR_PROTECTION_DELAY_TIMER])
  {
    mAdcAlarmStatus = WAIT_FOR_SET_BACK_TO_CURRENT;
  }

  if ( pSubject == mpTimerObjList[HIGH_ADC_ERROR_SET_BACK_TO_CURRENT_TIMER])
  {
    mAdcAlarmStatus = CHECK_ADC_OVER_CURRENT;
  }

  if (mpAnaInConfSensorElectric.Update(pSubject))
  {
    // nop
  }
  if (mpCurrentProtectionAllowed.Update(pSubject))
  {
    // nop
  }

}
/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInOnIOCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_AIOIOC_ALARM_OBJ:
      mpAnaInAlarmObj.Attach(pSubject);
      break;
    case SP_AIOIOC_CONF_SENSOR_ELECTRIC:
      mpAnaInConfSensorElectric.Attach(pSubject);
      break;
    case SP_AIOIOC_VALUE:
      mpAnaInValue.Attach(pSubject);
      break;
    case SP_AIOIOC_CURRENT_PROTECTION_ALLOWED :
      mpCurrentProtectionAllowed.Attach(pSubject);
      break;
  }
}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
