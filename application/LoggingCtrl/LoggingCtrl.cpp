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
/* CLASS NAME       : LoggingCtrl                                           */
/*                                                                          */
/* FILE NAME        : LoggingCtrl.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 20-02-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <LoggingCtrl.h>
#include <MpcTime.h>
#include <GeniAppTestMode.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INVALID_MARK   -1

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
LoggingCtrl::LoggingCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
LoggingCtrl::~LoggingCtrl()
{
  delete mTimeChangeObj;
  delete mTimeCompareObs;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LoggingCtrl::InitSubTask()
{
  mTimeChangeObj = new MpcTime(false);
  mTimeCompareObs = new MpcTimeCmpCtrl(mTimeChangeObj, this, MINUTE_TRIGGER);

  mLastFreeRunningPumpedVolume    = mpFreeRunningPumpedVolume->GetValue();
  mLastFreeRunningOverflowVolume  = mpFreeRunningOverflowVolume->GetValue();
  mLastOverflowCount              = mpOverflowCount->GetValue();
  mLastOverflowTime               = mpOverflowTime->GetValue();
  mLastParallelOperationTime      = mpParallelOperationTime->GetValue();
  mLastFreeRunningEnergy          = mpFreeRunningEnergy->GetValue();
  mLastDosingVolume               = mpRunningDosingVolume->GetValue();
  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    mLastOperationTime[pump_no]   = mpOperationTime[pump_no]->GetValue();
    mLastNoOfStarts[pump_no]      = mpNoOfStarts[pump_no]->GetValue();
  }

  for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
  {
    mLastPumpRunTimeXPumps[i] = mPumpRunTimeXPumps[i]->GetValue();
  }

  mLogDataSimulation = false;
  mFirstTimeToRun = true;
  mRunRequestedFlag = true;
  ReqTaskTime(); // Start up
}


/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LoggingCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  MpcTime new_time(true);
  U32 new_timestamp = new_time.GetSecondsSince1Jan1970();
  U32 new_hour = new_timestamp/3600;
  U32 old_hour = mpHourLogTimestamp->GetValue()/3600;

  if (old_hour == 0)
  {
    // Special handling first time after log data clear (or flash boot)
    MarkHourLogInvalid(mpPumpedVolume72hLog->GetMaxSize());
    UpdateDayLog();
    mpHourLogTimestamp->SetValue(new_timestamp);
    mFirstTimeToRun = false;
  }
  if (mFirstTimeToRun == true)
  {
    if (new_hour > old_hour)
    {
      // Check/update loggings for the time elapsed during power down
      if (new_hour - old_hour > mpPumpedVolume72hLog->GetMaxSize())
      {
        old_hour = new_hour - mpPumpedVolume72hLog->GetMaxSize();
      }
      UpdateHourLog();  // Fill old hour accumulators in log
      MarkHourLogInvalid(new_hour-old_hour-1);

      while (new_hour > old_hour)
      {
        old_hour++;
        if (old_hour%24 == mpDayLoggingStartHour->GetValue())
        {
          // Time for day logging has passed
          UpdateDayLog();
        }
      }
      mpHourLogTimestamp->SetValue(new_timestamp);
    }
    mFirstTimeToRun = false;
  }

  UpdateRunningAccumulators();
  if (new_hour > old_hour && mLogDataSimulation == false)
  {
    // The clock hour has increased. Move hour accumulators to log
    UpdateHourLog();
    if (new_hour%24 == mpDayLoggingStartHour->GetValue())
    {
      // Time for day logging has passed
      UpdateTodayAverages(24);
      UpdateDayLog();
    }
    UpdateTodayAverages((new_hour-mpDayLoggingStartHour->GetValue()) % 24);
    mpHourLogTimestamp->SetValue(new_timestamp);
  }
  else if (new_hour < old_hour)
  {
    // The clock must have been set back, e.g. daylight saving period gone
    // Just prepare to trig the logging on next clock hour
    mpHourLogTimestamp->SetValue(new_timestamp);
  }

  // Special test function:
  if (mpFillSimulatedLogData.IsUpdated() && GeniAppTestMode::GetInstance()->GetTestMode() == true)
  {
    mLogDataSimulation = true;
    FillValuesForTest();
  }
  else if (mLogDataSimulation == true && GeniAppTestMode::GetInstance()->GetTestMode() == false)
  {
    // Disable simulation if test mode is disabled
    mLogDataSimulation = false;
    // These function calls will destroy the simulated data
    UpdateHourLog();
    MarkHourLogInvalid(mpPumpedVolume72hLog->GetMaxSize());
    UpdateDayLog();
    UpdateDayLog(); // Twice to get rid of yesterday log as well
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void LoggingCtrl::ConnectToSubjects()
{
  mpFillSimulatedLogData->Subscribe(this); // Used for test
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void LoggingCtrl::Update(Subject* pSubject)
{
  mpFillSimulatedLogData.Update(pSubject);

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void LoggingCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void LoggingCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration / power on handling
    case SP_LOG_DAY_LOGGING_START_HOUR:
      mpDayLoggingStartHour.Attach(pSubject);
      break;
    case SP_LOG_HOUR_LOG_TIMESTAMP:
      mpHourLogTimestamp.Attach(pSubject);
      break;


    // Inputs:
    case SP_LOG_FILL_SIMULATED_LOG_DATA:
      mpFillSimulatedLogData.Attach(pSubject);
      break;
    case SP_LOG_FREE_RUNNING_PUMPED_VOLUME:
      mpFreeRunningPumpedVolume.Attach(pSubject);
      break;
    case SP_LOG_FREE_RUNNING_OVERFLOW_VOLUME:
      mpFreeRunningOverflowVolume.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_COUNT:
      mpOverflowCount.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_TIME:
      mpOverflowTime.Attach(pSubject);
      break;
    case SP_LOG_PARALLEL_OPERATION_TIME:
      mpParallelOperationTime.Attach(pSubject);
      break;
    case SP_LOG_FREE_RUNNING_ENERGY:
      mpFreeRunningEnergy.Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_OPERATION_TIME:
      mpOperationTime[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_OPERATION_TIME:
      mpOperationTime[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_OPERATION_TIME:
      mpOperationTime[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_OPERATION_TIME:
      mpOperationTime[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_OPERATION_TIME:
      mpOperationTime[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_OPERATION_TIME:
      mpOperationTime[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_NO_OF_STARTS:
      mpNoOfStarts[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_NO_OF_STARTS:
      mpNoOfStarts[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_NO_OF_STARTS:
      mpNoOfStarts[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_NO_OF_STARTS:
      mpNoOfStarts[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_NO_OF_STARTS:
      mpNoOfStarts[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_NO_OF_STARTS:
      mpNoOfStarts[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_FILTERED_FLOW:
      mpFilteredFlow[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_FLOW:
      mpFilteredFlow[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_FLOW:
      mpFilteredFlow[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_FLOW:
      mpFilteredFlow[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_FLOW:
      mpFilteredFlow[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_FLOW:
      mpFilteredFlow[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_FILTERED_CURRENT:
      mpFilteredCurrent[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_CURRENT:
      mpFilteredCurrent[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_CURRENT:
      mpFilteredCurrent[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_CURRENT:
      mpFilteredCurrent[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_CURRENT:
      mpFilteredCurrent[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_CURRENT:
      mpFilteredCurrent[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_H2S_LEVEL_ACT:
      mpH2SLevelAct.Attach(pSubject);
      break;
    case SP_LOG_RUNNING_DOSING_VOLUME:
      mpRunningDosingVolume.Attach(pSubject);
      break;

    // Outputs:
    case SP_LOG_PUMPED_VOLUME_1H_ACC:
      mpPumpedVolume1hAcc.Attach(pSubject);
      break;
    case SP_LOG_PUMPED_VOLUME_72H_LOG:
      mpPumpedVolume72hLog.Attach(pSubject);
      break;
    case SP_LOG_PUMPED_VOLUME_TODAY_LOG:
      mpPumpedVolumeTodayLog.Attach(pSubject);
      break;
    case SP_LOG_PUMPED_VOLUME_YESTERDAY_LOG:
      mpPumpedVolumeYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_OVERFLOW_VOLUME_1H_ACC:
      mpOverflowVolume1hAcc.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_VOLUME_72H_LOG:
      mpOverflowVolume72hLog.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_VOLUME_TODAY_LOG:
      mpOverflowVolumeTodayLog.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_VOLUME_YESTERDAY_LOG:
      mpOverflowVolumeYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_OVERFLOW_COUNT_1H_ACC:
      mpOverflowCount1hAcc.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_COUNT_72H_LOG:
      mpOverflowCount72hLog.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_COUNT_TODAY_LOG:
      mpOverflowCountTodayLog.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_COUNT_YESTERDAY_LOG:
      mpOverflowCountYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_OVERFLOW_TIME_1H_ACC:
      mpOverflowTime1hAcc.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_TIME_72H_LOG:
      mpOverflowTime72hLog.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_TIME_TODAY_LOG:
      mpOverflowTimeTodayLog.Attach(pSubject);
      break;
    case SP_LOG_OVERFLOW_TIME_YESTERDAY_LOG:
      mpOverflowTimeYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_PARALLEL_OPERATION_TIME_1H_ACC:
      mpParallelOperationTime1hAcc.Attach(pSubject);
      break;
    case SP_LOG_PARALLEL_OPERATION_TIME_72H_LOG:
      mpParallelOperationTime72hLog.Attach(pSubject);
      break;
    case SP_LOG_PARALLEL_OPERATION_TIME_TODAY_LOG:
      mpParallelOperationTimeTodayLog.Attach(pSubject);
      break;
    case SP_LOG_PARALLEL_OPERATION_TIME_YESTERDAY_LOG:
      mpParallelOperationTimeYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_ENERGY_CONSUMPTION_1H_ACC:
      mpEnergyConsumption1hAcc.Attach(pSubject);
      break;
    case SP_LOG_ENERGY_CONSUMPTION_72H_LOG:
      mpEnergyConsumption72hLog.Attach(pSubject);
      break;
    case SP_LOG_ENERGY_CONSUMPTION_TODAY_LOG:
      mpEnergyConsumptionTodayLog.Attach(pSubject);
      break;
    case SP_LOG_ENERGY_CONSUMPTION_YESTERDAY_LOG:
      mpEnergyConsumptionYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_EFFICIENCY_72H_LOG:
      mpEfficiency72hLog.Attach(pSubject);
      break;
    case SP_LOG_EFFICIENCY_TODAY_LOG:
      mpEfficiencyTodayLog.Attach(pSubject);
      break;
    case SP_LOG_EFFICIENCY_YESTERDAY_LOG:
      mpEfficiencyYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_H2S_LEVEL_72H_LOG:
      mpH2SLevel72hLog.Attach(pSubject);
      break;
    case SP_LOG_H2S_LEVEL_TODAY_LOG:
      mpH2SLevelTodayLog.Attach(pSubject);
      break;
    case SP_LOG_H2S_LEVEL_YESTERDAY_LOG:
      mpH2SLevelYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_DOSING_VOLUME_1H_ACC:
      mpDosingVolume1hAcc.Attach(pSubject);
      break;
    case SP_LOG_DOSING_VOLUME_72H_LOG:
      mpDosingVolume72hLog.Attach(pSubject);
      break;
    case SP_LOG_DOSING_VOLUME_TODAY_LOG:
      mpDosingVolumeTodayLog.Attach(pSubject);
      break;
    case SP_LOG_DOSING_VOLUME_YESTERDAY_LOG:
      mpDosingVolumeYesterdayLog.Attach(pSubject);
      break;

    case SP_LOG_PUMP_1_OPERATION_TIME_1H_ACC:
      mpOperationTime1hAcc[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_OPERATION_TIME_72H_LOG:
      mpOperationTime72hLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_OPERATION_TIME_TODAY_LOG:
      mpOperationTimeTodayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_OPERATION_TIME_YESTERDAY_LOG:
      mpOperationTimeYesterdayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_OPERATION_TIME_1H_ACC:
      mpOperationTime1hAcc[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_OPERATION_TIME_72H_LOG:
      mpOperationTime72hLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_OPERATION_TIME_TODAY_LOG:
      mpOperationTimeTodayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_OPERATION_TIME_YESTERDAY_LOG:
      mpOperationTimeYesterdayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_OPERATION_TIME_1H_ACC:
      mpOperationTime1hAcc[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_OPERATION_TIME_72H_LOG:
      mpOperationTime72hLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_OPERATION_TIME_TODAY_LOG:
      mpOperationTimeTodayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_OPERATION_TIME_YESTERDAY_LOG:
      mpOperationTimeYesterdayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_OPERATION_TIME_1H_ACC:
      mpOperationTime1hAcc[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_OPERATION_TIME_72H_LOG:
      mpOperationTime72hLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_OPERATION_TIME_TODAY_LOG:
      mpOperationTimeTodayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_OPERATION_TIME_YESTERDAY_LOG:
      mpOperationTimeYesterdayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_OPERATION_TIME_1H_ACC:
      mpOperationTime1hAcc[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_OPERATION_TIME_72H_LOG:
      mpOperationTime72hLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_OPERATION_TIME_TODAY_LOG:
      mpOperationTimeTodayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_OPERATION_TIME_YESTERDAY_LOG:
      mpOperationTimeYesterdayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_OPERATION_TIME_1H_ACC:
      mpOperationTime1hAcc[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_OPERATION_TIME_72H_LOG:
      mpOperationTime72hLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_OPERATION_TIME_TODAY_LOG:
      mpOperationTimeTodayLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_OPERATION_TIME_YESTERDAY_LOG:
      mpOperationTimeYesterdayLog[PUMP_6].Attach(pSubject);
      break;


    case SP_LOG_PUMP_1_NO_OF_STARTS_1H_ACC:
      mpNoOfStarts1hAcc[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_NO_OF_STARTS_72H_LOG:
      mpNoOfStarts72hLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_NO_OF_STARTS_TODAY_LOG:
      mpNoOfStartsTodayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_NO_OF_STARTS_YESTERDAY_LOG:
      mpNoOfStartsYesterdayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_NO_OF_STARTS_1H_ACC:
      mpNoOfStarts1hAcc[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_NO_OF_STARTS_72H_LOG:
      mpNoOfStarts72hLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_NO_OF_STARTS_TODAY_LOG:
      mpNoOfStartsTodayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_NO_OF_STARTS_YESTERDAY_LOG:
      mpNoOfStartsYesterdayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_NO_OF_STARTS_1H_ACC:
      mpNoOfStarts1hAcc[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_NO_OF_STARTS_72H_LOG:
      mpNoOfStarts72hLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_NO_OF_STARTS_TODAY_LOG:
      mpNoOfStartsTodayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_NO_OF_STARTS_YESTERDAY_LOG:
      mpNoOfStartsYesterdayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_NO_OF_STARTS_1H_ACC:
      mpNoOfStarts1hAcc[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_NO_OF_STARTS_72H_LOG:
      mpNoOfStarts72hLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_NO_OF_STARTS_TODAY_LOG:
      mpNoOfStartsTodayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_NO_OF_STARTS_YESTERDAY_LOG:
      mpNoOfStartsYesterdayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_NO_OF_STARTS_1H_ACC:
      mpNoOfStarts1hAcc[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_NO_OF_STARTS_72H_LOG:
      mpNoOfStarts72hLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_NO_OF_STARTS_TODAY_LOG:
      mpNoOfStartsTodayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_NO_OF_STARTS_YESTERDAY_LOG:
      mpNoOfStartsYesterdayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_NO_OF_STARTS_1H_ACC:
      mpNoOfStarts1hAcc[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_NO_OF_STARTS_72H_LOG:
      mpNoOfStarts72hLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_NO_OF_STARTS_TODAY_LOG:
      mpNoOfStartsTodayLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_NO_OF_STARTS_YESTERDAY_LOG:
      mpNoOfStartsYesterdayLog[PUMP_6].Attach(pSubject);
      break;

    case SP_LOG_PUMP_1_FILTERED_FLOW_72H_LOG:
      mpFilteredFlow72hLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_FILTERED_FLOW_TODAY_LOG:
      mpFilteredFlowTodayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_FILTERED_FLOW_YESTERDAY_LOG:
      mpFilteredFlowYesterdayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_FLOW_72H_LOG:
      mpFilteredFlow72hLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_FLOW_TODAY_LOG:
      mpFilteredFlowTodayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_FLOW_YESTERDAY_LOG:
      mpFilteredFlowYesterdayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_FLOW_72H_LOG:
      mpFilteredFlow72hLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_FLOW_TODAY_LOG:
      mpFilteredFlowTodayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_FLOW_YESTERDAY_LOG:
      mpFilteredFlowYesterdayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_FLOW_72H_LOG:
      mpFilteredFlow72hLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_FLOW_TODAY_LOG:
      mpFilteredFlowTodayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_FLOW_YESTERDAY_LOG:
      mpFilteredFlowYesterdayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_FLOW_72H_LOG:
      mpFilteredFlow72hLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_FLOW_TODAY_LOG:
      mpFilteredFlowTodayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_FLOW_YESTERDAY_LOG:
      mpFilteredFlowYesterdayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_FLOW_72H_LOG:
      mpFilteredFlow72hLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_FLOW_TODAY_LOG:
      mpFilteredFlowTodayLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_FLOW_YESTERDAY_LOG:
      mpFilteredFlowYesterdayLog[PUMP_6].Attach(pSubject);
      break;

    case SP_LOG_PUMP_1_FILTERED_CURRENT_72H_LOG:
      mpFilteredCurrent72hLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_FILTERED_CURRENT_TODAY_LOG:
      mpFilteredCurrentTodayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_1_FILTERED_CURRENT_YESTERDAY_LOG:
      mpFilteredCurrentYesterdayLog[PUMP_1].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_CURRENT_72H_LOG:
      mpFilteredCurrent72hLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_CURRENT_TODAY_LOG:
      mpFilteredCurrentTodayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_2_FILTERED_CURRENT_YESTERDAY_LOG:
      mpFilteredCurrentYesterdayLog[PUMP_2].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_CURRENT_72H_LOG:
      mpFilteredCurrent72hLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_CURRENT_TODAY_LOG:
      mpFilteredCurrentTodayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_3_FILTERED_CURRENT_YESTERDAY_LOG:
      mpFilteredCurrentYesterdayLog[PUMP_3].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_CURRENT_72H_LOG:
      mpFilteredCurrent72hLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_CURRENT_TODAY_LOG:
      mpFilteredCurrentTodayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_4_FILTERED_CURRENT_YESTERDAY_LOG:
      mpFilteredCurrentYesterdayLog[PUMP_4].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_CURRENT_72H_LOG:
      mpFilteredCurrent72hLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_CURRENT_TODAY_LOG:
      mpFilteredCurrentTodayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_5_FILTERED_CURRENT_YESTERDAY_LOG:
      mpFilteredCurrentYesterdayLog[PUMP_5].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_CURRENT_72H_LOG:
      mpFilteredCurrent72hLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_CURRENT_TODAY_LOG:
      mpFilteredCurrentTodayLog[PUMP_6].Attach(pSubject);
      break;
    case SP_LOG_PUMP_6_FILTERED_CURRENT_YESTERDAY_LOG:
      mpFilteredCurrentYesterdayLog[PUMP_6].Attach(pSubject);
      break;

    case SP_LOG_PUMP_RUN_TIME_0_PUMPS: mPumpRunTimeXPumps[0].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_1_PUMPS: mPumpRunTimeXPumps[1].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_2_PUMPS: mPumpRunTimeXPumps[2].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_3_PUMPS: mPumpRunTimeXPumps[3].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_4_PUMPS: mPumpRunTimeXPumps[4].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_5_PUMPS: mPumpRunTimeXPumps[5].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_6_PUMPS: mPumpRunTimeXPumps[6].Attach(pSubject); break;

    case SP_LOG_PUMP_RUN_TIME_0_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[0].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_1_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[1].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_2_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[2].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_3_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[3].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_4_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[4].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_5_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[5].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_6_PUMPS_1H_ACC: mPumpRunTimeXPumps1hAcc[6].Attach(pSubject); break;

    case SP_LOG_PUMP_RUN_TIME_0_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[0].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_1_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[1].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_2_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[2].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_3_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[3].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_4_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[4].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_5_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[5].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_6_PUMPS_72H_LOG: mPumpRunTimeXPumps72hLog[6].Attach(pSubject); break;

    case SP_LOG_PUMP_RUN_TIME_0_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[0].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_1_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[1].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_2_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[2].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_3_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[3].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_4_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[4].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_5_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[5].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_6_PUMPS_TODAY_LOG: mPumpRunTimeXPumpsTodayLog[6].Attach(pSubject); break;

    case SP_LOG_PUMP_RUN_TIME_0_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[0].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_1_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[1].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_2_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[2].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_3_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[3].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_4_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[4].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_5_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[5].Attach(pSubject); break;
    case SP_LOG_PUMP_RUN_TIME_6_PUMPS_YESTERDAY_LOG: mPumpRunTimeXPumpsYesterdayLog[6].Attach(pSubject); break;

    default:
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
 * Function - UpdateU32Accumulator
 * DESCRIPTION: Update the increment of a log value
 *
 *****************************************************************************/
void LoggingCtrl::UpdateU32Accumulator(SubjectPtr<U32DataPoint*> newValueDp, U32 &oldValue,
                                       SubjectPtr<U32DataPoint*> p1HourAcc, SubjectPtr<U32DataPoint*> pTodayAcc)
{
  U32 new_value = newValueDp->GetValue();
  I32 inc_value = new_value - oldValue;

  if (inc_value > 0 && inc_value < p1HourAcc->GetMaxValue())
  {
    p1HourAcc->SetValue(inc_value + p1HourAcc->GetValue());
    pTodayAcc->SetValue(inc_value + pTodayAcc->GetValue());
  }
  oldValue = new_value;
}
/*****************************************************************************
 * Function - UpdateFloatAccumulator
 * DESCRIPTION: Update the increment of a log value
 *
 *****************************************************************************/
void LoggingCtrl::UpdateFloatAccumulator(SubjectPtr<U32DataPoint*> newValueDp, U32 &oldValue,
                                         SubjectPtr<U32DataPoint*> p1HourAcc, SubjectPtr<FloatDataPoint*> pTodayAcc, float scaleFactor)
{
  U32 new_value = newValueDp->GetValue();
  I32 inc_value = new_value - oldValue;

  if (inc_value > 0 && inc_value < p1HourAcc->GetMaxValue())
  {
    p1HourAcc->SetValue(inc_value + p1HourAcc->GetValue());
    pTodayAcc->SetValue((float)inc_value*scaleFactor + pTodayAcc->GetValue());
  }
  oldValue = new_value;
}

/*****************************************************************************
 * Function - UpdateRunningAccumulators
 * DESCRIPTION: Maintain the increments of the log values within the last hour
 *
 *****************************************************************************/
void LoggingCtrl::UpdateRunningAccumulators()
{
  UpdateFloatAccumulator(mpFreeRunningPumpedVolume, mLastFreeRunningPumpedVolume, mpPumpedVolume1hAcc, mpPumpedVolumeTodayLog, 0.001f);
  UpdateFloatAccumulator(mpFreeRunningOverflowVolume, mLastFreeRunningOverflowVolume, mpOverflowVolume1hAcc, mpOverflowVolumeTodayLog, 0.001f);
  UpdateU32Accumulator(mpOverflowCount, mLastOverflowCount, mpOverflowCount1hAcc, mpOverflowCountTodayLog);
  UpdateU32Accumulator(mpOverflowTime,  mLastOverflowTime, mpOverflowTime1hAcc, mpOverflowTimeTodayLog);
  UpdateU32Accumulator(mpParallelOperationTime,  mLastParallelOperationTime, mpParallelOperationTime1hAcc, mpParallelOperationTimeTodayLog);
  UpdateFloatAccumulator(mpFreeRunningEnergy, mLastFreeRunningEnergy, mpEnergyConsumption1hAcc, mpEnergyConsumptionTodayLog, 0.001f);
  UpdateFloatAccumulator(mpRunningDosingVolume, mLastDosingVolume, mpDosingVolume1hAcc, mpDosingVolumeTodayLog, 0.001f);

  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    UpdateU32Accumulator(mpOperationTime[pump_no],  mLastOperationTime[pump_no], mpOperationTime1hAcc[pump_no], mpOperationTimeTodayLog[pump_no]);
    UpdateU32Accumulator(mpNoOfStarts[pump_no],  mLastNoOfStarts[pump_no], mpNoOfStarts1hAcc[pump_no], mpNoOfStartsTodayLog[pump_no]);
  }

  for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
  {
    UpdateU32Accumulator(mPumpRunTimeXPumps[i],  mLastPumpRunTimeXPumps[i], mPumpRunTimeXPumps1hAcc[i], mPumpRunTimeXPumpsTodayLog[i]);
  }
}

/*****************************************************************************
 * Function - UpdateHourLog
 * DESCRIPTION: Determine the last hour increment/sample and update the hour log
 *
 *****************************************************************************/
void LoggingCtrl::UpdateHourLog()
{
  float hour_avg;
  U32   hour_acc;

  // Calculate efficiency before clearing hour accumulators for energy and volume
  hour_avg = 0.0f;
  if (mpPumpedVolume1hAcc->GetValue() > 0)
  {
    hour_avg = (float)mpEnergyConsumption1hAcc->GetValue()/(float)mpPumpedVolume1hAcc->GetValue();  // [Wh/l] = [kWh/m3]
  }
  mpEfficiency72hLog->PushValue(hour_avg / 2.7777778e-07f);  // [kWh/m3] --> [J/m3]

  hour_acc = mpPumpedVolume1hAcc->GetValue();
  mpPumpedVolume1hAcc->SetValue(0);
  mpPumpedVolume72hLog->PushValue(0.001f*hour_acc);  // l --> m3

  hour_acc = mpOverflowVolume1hAcc->GetValue();
  mpOverflowVolume1hAcc->SetValue(0);
  mpOverflowVolume72hLog->PushValue(0.001f*hour_acc);  // l --> m3

  hour_acc = mpOverflowCount1hAcc->GetValue();
  mpOverflowCount1hAcc->SetValue(0);
  mpOverflowCount72hLog->PushValue(hour_acc);

  hour_acc = mpOverflowTime1hAcc->GetValue();
  mpOverflowTime1hAcc->SetValue(0);
  mpOverflowTime72hLog->PushValue(hour_acc);

  hour_acc = mpParallelOperationTime1hAcc->GetValue();
  mpParallelOperationTime1hAcc->SetValue(0);
  mpParallelOperationTime72hLog->PushValue(hour_acc);

  hour_acc = mpEnergyConsumption1hAcc->GetValue();
  mpEnergyConsumption1hAcc->SetValue(0);
  mpEnergyConsumption72hLog->PushValue(0.001f*hour_acc); // Wh --> kWh

  mpH2SLevel72hLog->PushValue(mpH2SLevelAct->GetValue());

  hour_acc = mpDosingVolume1hAcc->GetValue();
  mpDosingVolume1hAcc->SetValue(0);
  mpDosingVolume72hLog->PushValue(hour_acc);

  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    hour_acc = mpOperationTime1hAcc[pump_no]->GetValue();
    mpOperationTime1hAcc[pump_no]->SetValue(0);
    mpOperationTime72hLog[pump_no]->PushValue(hour_acc);

    hour_acc = mpNoOfStarts1hAcc[pump_no]->GetValue();
    mpNoOfStarts1hAcc[pump_no]->SetValue(0);
    mpNoOfStarts72hLog[pump_no]->PushValue(hour_acc);

    hour_avg = mpFilteredFlow[pump_no]->GetValue();
    mpFilteredFlow72hLog[pump_no]->PushValue(hour_avg);

    hour_avg = mpFilteredCurrent[pump_no]->GetValue();
    mpFilteredCurrent72hLog[pump_no]->PushValue(hour_avg);
  }

  for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
  {
    hour_acc = mPumpRunTimeXPumps1hAcc[i]->GetValue();
    mPumpRunTimeXPumps1hAcc[i]->SetValue(0);
    mPumpRunTimeXPumps72hLog[i]->PushValue(hour_acc);
  }
}


/*****************************************************************************
 * Function - UpdateDayLog
 * DESCRIPTION: Move the actual today log into the yesterday log
 *              and clear the today log.
 *
 *****************************************************************************/
void LoggingCtrl::UpdateDayLog()
{
  mpPumpedVolumeYesterdayLog->SetValue(mpPumpedVolumeTodayLog->GetValue());
  mpPumpedVolumeTodayLog->SetValue(0);
  mpOverflowVolumeYesterdayLog->SetValue(mpOverflowVolumeTodayLog->GetValue());
  mpOverflowVolumeTodayLog->SetValue(0);
  mpOverflowCountYesterdayLog->SetValue(mpOverflowCountTodayLog->GetValue());
  mpOverflowCountTodayLog->SetValue(0);
  mpOverflowTimeYesterdayLog->SetValue(mpOverflowTimeTodayLog->GetValue());
  mpOverflowTimeTodayLog->SetValue(0);
  mpParallelOperationTimeYesterdayLog->SetValue(mpParallelOperationTimeTodayLog->GetValue());
  mpParallelOperationTimeTodayLog->SetValue(0);
  mpEnergyConsumptionYesterdayLog->SetValue(mpEnergyConsumptionTodayLog->GetValue());
  mpEnergyConsumptionTodayLog->SetValue(0);
  mpEfficiencyYesterdayLog->SetValue(mpEfficiencyTodayLog->GetValue());
  mpEfficiencyTodayLog->SetValue(0);
  mpH2SLevelYesterdayLog->SetValue(mpH2SLevelTodayLog->GetValue());
  mpH2SLevelTodayLog->SetValue(0);
  mpDosingVolumeYesterdayLog->SetValue(mpDosingVolumeTodayLog->GetValue());
  mpDosingVolumeTodayLog->SetValue(0);

  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    mpOperationTimeYesterdayLog[pump_no]->SetValue(mpOperationTimeTodayLog[pump_no]->GetValue());
    mpOperationTimeTodayLog[pump_no]->SetValue(0);
    mpNoOfStartsYesterdayLog[pump_no]->SetValue(mpNoOfStartsTodayLog[pump_no]->GetValue());
    mpNoOfStartsTodayLog[pump_no]->SetValue(0);
    mpFilteredFlowYesterdayLog[pump_no]->SetValue(mpFilteredFlowTodayLog[pump_no]->GetValue());
    mpFilteredFlowTodayLog[pump_no]->SetValue(0);
    mpFilteredCurrentYesterdayLog[pump_no]->SetValue(mpFilteredCurrentTodayLog[pump_no]->GetValue());
    mpFilteredCurrentTodayLog[pump_no]->SetValue(0);
  }

  for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
  {
    mPumpRunTimeXPumpsYesterdayLog[i]->SetValue(mPumpRunTimeXPumpsTodayLog[i]->GetValue());
    mPumpRunTimeXPumpsTodayLog[i]->SetValue(0);
  }
}

/*****************************************************************************
 * Function - MarkHourLogInvalid
 * DESCRIPTION: Pushes a number of invalid marks in the hour log vectors
 *
 *****************************************************************************/
void LoggingCtrl::MarkHourLogInvalid(U32 noOfHours)
{
  mpPumpedVolume72hLog->PushValue(INVALID_MARK, noOfHours);
  mpOverflowVolume72hLog->PushValue(INVALID_MARK, noOfHours);
  mpOverflowCount72hLog->PushValue(INVALID_MARK, noOfHours);
  mpOverflowTime72hLog->PushValue(INVALID_MARK, noOfHours);
  mpParallelOperationTime72hLog->PushValue(INVALID_MARK, noOfHours);
  mpEnergyConsumption72hLog->PushValue(INVALID_MARK, noOfHours);
  mpEfficiency72hLog->PushValue(INVALID_MARK, noOfHours);
  mpH2SLevel72hLog->PushValue(INVALID_MARK, noOfHours);
  mpDosingVolume72hLog->PushValue(INVALID_MARK, noOfHours);

  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    mpOperationTime72hLog[pump_no]->PushValue(INVALID_MARK, noOfHours);
    mpNoOfStarts72hLog[pump_no]->PushValue(INVALID_MARK, noOfHours);
    mpFilteredFlow72hLog[pump_no]->PushValue(INVALID_MARK, noOfHours);
    mpFilteredCurrent72hLog[pump_no]->PushValue(INVALID_MARK, noOfHours);
  }

  for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
  {
    mPumpRunTimeXPumps72hLog[i]->PushValue(INVALID_MARK, noOfHours);
  }
}

/*****************************************************************************
 * Function - UpdateTodayAverages
 * DESCRIPTION: Update the today averages based on the last
 *              hours in the log vectors
 *
 *****************************************************************************/
void LoggingCtrl::UpdateTodayAverages(U32 noOfHours)
{
  float temp_float;
  U32   temp_int;

  if (noOfHours == 0)
  {
    noOfHours = 1;  // just get the value for the last hour
  }

  temp_float = mpEfficiency72hLog->GetAverage(0, noOfHours, INVALID_MARK);
  mpEfficiencyTodayLog->SetValue(temp_float);

  temp_int = mpH2SLevel72hLog->GetAverage(0, noOfHours, INVALID_MARK);
  mpH2SLevelTodayLog->SetValue(temp_int);

  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    temp_float = mpFilteredFlow72hLog[pump_no]->GetAverage(0, noOfHours, INVALID_MARK);
    mpFilteredFlowTodayLog[pump_no]->SetValue(temp_float);
    temp_float = mpFilteredCurrent72hLog[pump_no]->GetAverage(0, noOfHours, INVALID_MARK);
    mpFilteredCurrentTodayLog[pump_no]->SetValue(temp_float);
  }
}

/*****************************************************************************
 * Function - UpdateTodayAccumulators
 * DESCRIPTION: Update/sync the today accumulators based on the last
 *              hours in the log vectors
 *
 *****************************************************************************/
void LoggingCtrl::UpdateTodayAccumulators(U32 noOfHours)
{
  float temp_float;
  U32   temp_int;

  temp_float = mpPumpedVolume72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_float += 0.001f*mpPumpedVolume1hAcc->GetValue();
  mpPumpedVolumeTodayLog->SetValue(temp_float);

  temp_float = mpOverflowVolume72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_float += 0.001f*mpOverflowVolume1hAcc->GetValue();
  mpOverflowVolumeTodayLog->SetValue(temp_float);

  temp_int = mpOverflowCount72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_int += mpOverflowCount1hAcc->GetValue();
  mpOverflowCountTodayLog->SetValue(temp_int);

  temp_int = mpOverflowTime72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_int += mpOverflowTime1hAcc->GetValue();
  mpOverflowTimeTodayLog->SetValue(temp_int);

  temp_int = mpParallelOperationTime72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_int += mpParallelOperationTime1hAcc->GetValue();
  mpParallelOperationTimeTodayLog->SetValue(temp_int);

  temp_float = mpEnergyConsumption72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_float += 0.001f*mpEnergyConsumption1hAcc->GetValue();
  mpEnergyConsumptionTodayLog->SetValue(temp_float);

  temp_float = mpDosingVolume72hLog->GetSum(0, noOfHours, INVALID_MARK);
  temp_float += mpDosingVolume1hAcc->GetValue();
  mpDosingVolumeTodayLog->SetValue(temp_float*0.001f);

  for (int pump_no = FIRST_PUMP_NO; pump_no < NO_OF_PUMPS; pump_no++)
  {
    temp_int = mpOperationTime72hLog[pump_no]->GetSum(0, noOfHours, INVALID_MARK);
    temp_int += mpOperationTime1hAcc[pump_no]->GetValue();
    mpOperationTimeTodayLog[pump_no]->SetValue(temp_int);

    temp_int = mpNoOfStarts72hLog[pump_no]->GetSum(0, noOfHours, INVALID_MARK);
    temp_int += mpNoOfStarts1hAcc[pump_no]->GetValue();
    mpNoOfStartsTodayLog[pump_no]->SetValue(temp_int);
  }

  for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
  {
    temp_int = mPumpRunTimeXPumps72hLog[i]->GetSum(0, noOfHours, INVALID_MARK);
    temp_int += mPumpRunTimeXPumps1hAcc[i]->GetValue();
    mPumpRunTimeXPumpsTodayLog[i]->SetValue(temp_int);
  }
}

/*****************************************************************************
 * Function - FillValuesForTest
 * DESCRIPTION: Fill in some fixed values in the different log vectors.
 *              For test purpose only !
 *
 *****************************************************************************/
void LoggingCtrl::FillValuesForTest()
{
  if (GeniAppTestMode::GetInstance()->GetTestMode() == true)
  {
    int value = 0;
    for (int hour = 1; hour <= 72; hour++)
    {
      value = hour;
      mpPumpedVolume1hAcc->SetValue(value*4*1000);                          //  59,90:  4000-288000 ->  40-2880 0.1 m3 in log
      mpOverflowVolume1hAcc->SetValue(value*2*1000);                        //  59,70:  2000-144000 ->  20-1440 0.1 m3 in log
      mpOverflowCount1hAcc->SetValue(value*1);                              //  59,10:  1-72        ->  1-72 counts in log
      mpOverflowTime1hAcc->SetValue(value*5);                               //  59,00:  5-360s      ->  0-6 minutes in log
      mpParallelOperationTime1hAcc->SetValue(value*10);                     //  59,20:  10-720s     ->  0-12 min. (Release 1)
      mpEnergyConsumption1hAcc->SetValue(10000+value*0.5*1000);             //  59,100: 10500-46000 ->  105-460 0.1 kWh in log
      for (int pump_idx = FIRST_PUMP_NO; pump_idx < NO_OF_PUMPS; pump_idx++)
      {
        int pump_no = (pump_idx + 1);                                       //  Data below is for pump 1.
        mpOperationTime1hAcc[pump_idx]->SetValue(pump_no*1000 + value*10);  //  59,200: 1010-1720s  ->  16-28 minutes in log
        mpNoOfStarts1hAcc[pump_idx]->SetValue(pump_no*10+100 + value);      //  59,210: 111-182     ->  111-182 counts in log
        mpFilteredFlow[pump_idx]->SetValue(pump_no/10.0 + value/2000.0);    //  59,220: 0.1005-0.136->  1005-1360 0.1 l/s in log
        mpFilteredCurrent[pump_idx]->SetValue(pump_no*10.0 + value/10.0);   //  59,230: 10.1-17.2   ->  101-172 0.1 A in log
      }

      for (unsigned int i = 0; i < (sizeof(mLastPumpRunTimeXPumps) / sizeof(*mLastPumpRunTimeXPumps)); ++i)
      {
        mPumpRunTimeXPumps1hAcc[i]->SetValue(value * 6 + i * 480);
        // 0. 59,??:     6 -  432s ->  0 - 7  minutes in log
        // 1. 59,??:   486 -  912s ->  8 - 15 minutes in log
        // 2. 59,20:   966 - 1392s -> 16 - 23 minutes in log
        // 3. 59,30:  1446 - 1872s -> 24 - 31 minutes in log
        // 4. 59,40:  1926 - 2352s -> 32 - 39 minutes in log
        // 5. 59,50:  2406 - 2832s -> 40 - 47 minutes in log
        // 6. 59,60:  2886 - 3312s -> 48 - 55 minutes in log
      }

      UpdateHourLog();
      if (hour == 24 || hour == 48)
      {
        UpdateTodayAverages(24);
        UpdateTodayAccumulators(24);
        UpdateDayLog();
      }
    }
    UpdateTodayAverages(24);
    UpdateTodayAccumulators(24);
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
