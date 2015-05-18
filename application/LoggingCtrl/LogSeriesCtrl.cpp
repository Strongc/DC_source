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
/* CLASS NAME       : LogSeriesCtrl                                         */
/*                                                                          */
/* FILE NAME        : LogSeriesCtrl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 22-07-2008 dd-mm-yyyy                                 */
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
#include <LogSeriesCtrl.h>
#include <ConfigControl.h>
#include <GeniAppTestMode.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INVALID_MARK  0xFFFF

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef struct
{
  U16   logItem;
  U16   subjectRelation;
  float scale;
} FLOAT_LOG_ITEM_TABLE_TYPE;

const FLOAT_LOG_ITEM_TABLE_TYPE float_log_item_table[] =
{
  // Insert FloatDataPoint relations in this list. There are handled automatically.
  // System
  { LOG_ITEM_WATER_LEVEL,             SP_LOGSE_SURFACE_LEVEL,                            100.0f },  //  m --> cm
  { LOG_ITEM_FLOW,                    SP_LOGSE_SYSTEM_FLOW,                            10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_POWER,                   SP_LOGSE_SYSTEM_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_SPECIFIC_ENERGY,         SP_LOGSE_SPECIFIC_ENERGY,                       1/3600.0f },  //  J/m3 --> Wh/m3
  { LOG_ITEM_OUTLET_PRESSURE,         SP_LOGSE_MEASURED_VALUE_OUTLET_PRESSURE,            0.01f },  //  Pa --> mBar
  { LOG_ITEM_ESTIMATED_FLOW,          SP_LOGSE_ESTIMATED_FLOW,                         10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_USER_DEFINED_SOURCE_1,   SP_LOGSE_MEASURED_VALUE_USER_DEFINED_SOURCE_1,      10.0f },  //  % --> pro mille
  { LOG_ITEM_USER_DEFINED_SOURCE_2,   SP_LOGSE_MEASURED_VALUE_USER_DEFINED_SOURCE_2,      10.0f },  //  % --> pro mille
  { LOG_ITEM_USER_DEFINED_SOURCE_3,   SP_LOGSE_MEASURED_VALUE_USER_DEFINED_SOURCE_3,      10.0f },  //  % --> pro mille
  //LOG_ITEM_SWITCH_WATER_LEVEL       Not a float
  //LOG_ITEM_GSM_SIGNAL_LEVEL         Not a float
  //LOG_ITEM_MIXER_STARTS_PR_HOUR     Not a float

  // Pump 1
  { LOG_ITEM_PUMP_1_FILTERED_FLOW,    SP_LOGSE_PUMP_1_FILTERED_FLOW,                   10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_1_LATEST_FLOW,      SP_LOGSE_PUMP_1_LATEST_FLOW,                     10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_1_ACTUAL_CURRENT,   SP_LOGSE_PUMP_1_ACTUAL_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_1_LATEST_CURRENT,   SP_LOGSE_PUMP_1_LATEST_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_1_INSULATION,       SP_LOGSE_PUMP_1_INSULATION,                    1/10000.0f },  //  Ohm  --> 10 kOhm
  { LOG_ITEM_PUMP_1_WATER_IN_OIL,     SP_LOGSE_PUMP_1_WATER_IN_OIL,                       10.0f },  //  % --> pro mille
  { LOG_ITEM_PUMP_1_TEMPERATURE1,     SP_LOGSE_PUMP_1_TEMPERATURE1,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_1_TEMPERATURE2,     SP_LOGSE_PUMP_1_TEMPERATURE2,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_1_POWER,            SP_LOGSE_PUMP_1_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_PUMP_1_MAINS_VOLTAGE,    SP_LOGSE_PUMP_1_MAINS_VOLTAGE,                       1.0f },  //  Volt
  { LOG_ITEM_PUMP_1_FREQUENCY,        SP_LOGSE_PUMP_1_FREQUENCY,                         100.0f },  //  Hz --> 0.01Hz
  { LOG_ITEM_PUMP_1_TORQUE,           SP_LOGSE_PUMP_1_TORQUE,                             10.0f },  //  Nm --> 0.1Nm
  { LOG_ITEM_PUMP_1_CURRENT_ASYMMETRY,SP_LOGSE_PUMP_1_CURRENT_ASYMMETRY,                  10.0f },  //  % --> 0.1%
  //LOG_ITEM_PUMP_1_STARTS_PR_HOUR    Not a float
  //LOG_ITEM_PUMP_1_LATEST_RUN_TIME   Not a float
  //LOG_ITEM_PUMP_1_MOISTURE_SWITCH   Not a float

  // Pump 2
  { LOG_ITEM_PUMP_2_FILTERED_FLOW,    SP_LOGSE_PUMP_2_FILTERED_FLOW,                   10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_2_LATEST_FLOW,      SP_LOGSE_PUMP_2_LATEST_FLOW,                     10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_2_ACTUAL_CURRENT,   SP_LOGSE_PUMP_2_ACTUAL_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_2_LATEST_CURRENT,   SP_LOGSE_PUMP_2_LATEST_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_2_INSULATION,       SP_LOGSE_PUMP_2_INSULATION,                    1/10000.0f },  //  Ohm  --> 10 kOhm
  { LOG_ITEM_PUMP_2_WATER_IN_OIL,     SP_LOGSE_PUMP_2_WATER_IN_OIL,                       10.0f },  //  % --> pro mille
  { LOG_ITEM_PUMP_2_TEMPERATURE1,     SP_LOGSE_PUMP_2_TEMPERATURE1,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_2_TEMPERATURE2,     SP_LOGSE_PUMP_2_TEMPERATURE2,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_2_POWER,            SP_LOGSE_PUMP_2_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_PUMP_2_MAINS_VOLTAGE,    SP_LOGSE_PUMP_2_MAINS_VOLTAGE,                       1.0f },  //  Volt
  { LOG_ITEM_PUMP_2_FREQUENCY,        SP_LOGSE_PUMP_2_FREQUENCY,                         100.0f },  //  Hz --> 0.01Hz
  { LOG_ITEM_PUMP_2_TORQUE,           SP_LOGSE_PUMP_2_TORQUE,                             10.0f },  //  Nm --> 0.1Nm
  { LOG_ITEM_PUMP_2_CURRENT_ASYMMETRY,SP_LOGSE_PUMP_2_CURRENT_ASYMMETRY,                  10.0f },  //  % --> 0.1%
  //LOG_ITEM_PUMP_2_STARTS_PR_HOUR    Not a float
  //LOG_ITEM_PUMP_2_LATEST_RUN_TIME   Not a float
  //LOG_ITEM_PUMP_2_MOISTURE_SWITCH   Not a float

  // Pump 3
  { LOG_ITEM_PUMP_3_FILTERED_FLOW,    SP_LOGSE_PUMP_3_FILTERED_FLOW,                   10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_3_LATEST_FLOW,      SP_LOGSE_PUMP_3_LATEST_FLOW,                     10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_3_ACTUAL_CURRENT,   SP_LOGSE_PUMP_3_ACTUAL_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_3_LATEST_CURRENT,   SP_LOGSE_PUMP_3_LATEST_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_3_INSULATION,       SP_LOGSE_PUMP_3_INSULATION,                    1/10000.0f },  //  Ohm  --> 10 kOhm
  { LOG_ITEM_PUMP_3_WATER_IN_OIL,     SP_LOGSE_PUMP_3_WATER_IN_OIL,                       10.0f },  //  % --> pro mille
  { LOG_ITEM_PUMP_3_TEMPERATURE1,     SP_LOGSE_PUMP_3_TEMPERATURE1,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_3_TEMPERATURE2,     SP_LOGSE_PUMP_3_TEMPERATURE2,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_3_POWER,            SP_LOGSE_PUMP_3_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_PUMP_3_MAINS_VOLTAGE,    SP_LOGSE_PUMP_3_MAINS_VOLTAGE,                       1.0f },  //  Volt
  { LOG_ITEM_PUMP_3_FREQUENCY,        SP_LOGSE_PUMP_3_FREQUENCY,                         100.0f },  //  Hz --> 0.01Hz
  { LOG_ITEM_PUMP_3_TORQUE,           SP_LOGSE_PUMP_3_TORQUE,                             10.0f },  //  Nm --> 0.1Nm
  { LOG_ITEM_PUMP_3_CURRENT_ASYMMETRY,SP_LOGSE_PUMP_3_CURRENT_ASYMMETRY,                  10.0f },  //  % --> 0.1%
  //LOG_ITEM_PUMP_3_STARTS_PR_HOUR    Not a float
  //LOG_ITEM_PUMP_3_LATEST_RUN_TIME   Not a float
  //LOG_ITEM_PUMP_3_MOISTURE_SWITCH   Not a float

  // Pump 4
  { LOG_ITEM_PUMP_4_FILTERED_FLOW,    SP_LOGSE_PUMP_4_FILTERED_FLOW,                   10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_4_LATEST_FLOW,      SP_LOGSE_PUMP_4_LATEST_FLOW,                     10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_4_ACTUAL_CURRENT,   SP_LOGSE_PUMP_4_ACTUAL_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_4_LATEST_CURRENT,   SP_LOGSE_PUMP_4_LATEST_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_4_INSULATION,       SP_LOGSE_PUMP_4_INSULATION,                    1/10000.0f },  //  Ohm  --> 10 kOhm
  { LOG_ITEM_PUMP_4_WATER_IN_OIL,     SP_LOGSE_PUMP_4_WATER_IN_OIL,                       10.0f },  //  % --> pro mille
  { LOG_ITEM_PUMP_4_TEMPERATURE1,     SP_LOGSE_PUMP_4_TEMPERATURE1,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_4_TEMPERATURE2,     SP_LOGSE_PUMP_4_TEMPERATURE2,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_4_POWER,            SP_LOGSE_PUMP_4_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_PUMP_4_MAINS_VOLTAGE,    SP_LOGSE_PUMP_4_MAINS_VOLTAGE,                       1.0f },  //  Volt
  { LOG_ITEM_PUMP_4_FREQUENCY,        SP_LOGSE_PUMP_4_FREQUENCY,                         100.0f },  //  Hz --> 0.01Hz
  { LOG_ITEM_PUMP_4_TORQUE,           SP_LOGSE_PUMP_4_TORQUE,                             10.0f },  //  Nm --> 0.1Nm
  { LOG_ITEM_PUMP_4_CURRENT_ASYMMETRY,SP_LOGSE_PUMP_4_CURRENT_ASYMMETRY,                  10.0f },  //  % --> 0.1%
  //LOG_ITEM_PUMP_4_STARTS_PR_HOUR    Not a float
  //LOG_ITEM_PUMP_4_LATEST_RUN_TIME   Not a float
  //LOG_ITEM_PUMP_4_MOISTURE_SWITCH   Not a float

  // Pump 5
  { LOG_ITEM_PUMP_5_FILTERED_FLOW,    SP_LOGSE_PUMP_5_FILTERED_FLOW,                   10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_5_LATEST_FLOW,      SP_LOGSE_PUMP_5_LATEST_FLOW,                     10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_5_ACTUAL_CURRENT,   SP_LOGSE_PUMP_5_ACTUAL_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_5_LATEST_CURRENT,   SP_LOGSE_PUMP_5_LATEST_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_5_INSULATION,       SP_LOGSE_PUMP_5_INSULATION,                    1/10000.0f },  //  Ohm  --> 10 kOhm
  { LOG_ITEM_PUMP_5_WATER_IN_OIL,     SP_LOGSE_PUMP_5_WATER_IN_OIL,                       10.0f },  //  % --> pro mille
  { LOG_ITEM_PUMP_5_TEMPERATURE1,     SP_LOGSE_PUMP_5_TEMPERATURE1,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_5_TEMPERATURE2,     SP_LOGSE_PUMP_5_TEMPERATURE2,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_5_POWER,            SP_LOGSE_PUMP_5_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_PUMP_5_MAINS_VOLTAGE,    SP_LOGSE_PUMP_5_MAINS_VOLTAGE,                       1.0f },  //  Volt
  { LOG_ITEM_PUMP_5_FREQUENCY,        SP_LOGSE_PUMP_5_FREQUENCY,                         100.0f },  //  Hz --> 0.01Hz
  { LOG_ITEM_PUMP_5_TORQUE,           SP_LOGSE_PUMP_5_TORQUE,                             10.0f },  //  Nm --> 0.1Nm
  { LOG_ITEM_PUMP_5_CURRENT_ASYMMETRY,SP_LOGSE_PUMP_5_CURRENT_ASYMMETRY,                  10.0f },  //  % --> 0.1%
  //LOG_ITEM_PUMP_5_STARTS_PR_HOUR    Not a float
  //LOG_ITEM_PUMP_5_LATEST_RUN_TIME   Not a float
  //LOG_ITEM_PUMP_5_MOISTURE_SWITCH   Not a float

  // Pump 6
  { LOG_ITEM_PUMP_6_FILTERED_FLOW,    SP_LOGSE_PUMP_6_FILTERED_FLOW,                   10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_6_LATEST_FLOW,      SP_LOGSE_PUMP_6_LATEST_FLOW,                     10000.0f },  //  m3/s --> 0.1l/s
  { LOG_ITEM_PUMP_6_ACTUAL_CURRENT,   SP_LOGSE_PUMP_6_ACTUAL_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_6_LATEST_CURRENT,   SP_LOGSE_PUMP_6_LATEST_CURRENT,                     10.0f },  //  A --> 0.1A
  { LOG_ITEM_PUMP_6_INSULATION,       SP_LOGSE_PUMP_6_INSULATION,                    1/10000.0f },  //  Ohm  --> 10 kOhm
  { LOG_ITEM_PUMP_6_WATER_IN_OIL,     SP_LOGSE_PUMP_6_WATER_IN_OIL,                       10.0f },  //  % --> pro mille
  { LOG_ITEM_PUMP_6_TEMPERATURE1,     SP_LOGSE_PUMP_6_TEMPERATURE1,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_6_TEMPERATURE2,     SP_LOGSE_PUMP_6_TEMPERATURE2,                     -273.15 },  //  K --> C (Offset)
  { LOG_ITEM_PUMP_6_POWER,            SP_LOGSE_PUMP_6_POWER,                               0.1f },  //  W --> 10W
  { LOG_ITEM_PUMP_6_MAINS_VOLTAGE,    SP_LOGSE_PUMP_6_MAINS_VOLTAGE,                       1.0f },  //  Volt
  { LOG_ITEM_PUMP_6_FREQUENCY,        SP_LOGSE_PUMP_6_FREQUENCY,                         100.0f },  //  Hz --> 0.01Hz
  { LOG_ITEM_PUMP_6_TORQUE,           SP_LOGSE_PUMP_6_TORQUE,                             10.0f },  //  Nm --> 0.1Nm
  { LOG_ITEM_PUMP_6_CURRENT_ASYMMETRY,SP_LOGSE_PUMP_6_CURRENT_ASYMMETRY,                  10.0f },  //  % --> 0.1%
  //LOG_ITEM_PUMP_6_STARTS_PR_HOUR    Not a float
  //LOG_ITEM_PUMP_6_LATEST_RUN_TIME   Not a float
  //LOG_ITEM_PUMP_6_MOISTURE_SWITCH   Not a float
};
#define FLOAT_LOG_ITEM_TABLE_SIZE (sizeof(float_log_item_table)/sizeof(float_log_item_table[0]))

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
LogSeriesCtrl::LogSeriesCtrl()
{
  // Allocate a list for log value subjects of type FloatDataPoint
  mpSourceValueList = new SubjectPtr<FloatDataPoint*>[FLOAT_LOG_ITEM_TABLE_SIZE];
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
LogSeriesCtrl::~LogSeriesCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LogSeriesCtrl::InitSubTask()
{
  if ( (mpLogSeriesItem->GetValue(0) == 0 && mpLogSeriesEnabled->GetValue(0) == false)
    && (mpLogSeriesItem->GetValue(1) == 0 && mpLogSeriesEnabled->GetValue(1) == false)
    && (mpLogSeriesItem->GetValue(2) == 0 && mpLogSeriesEnabled->GetValue(2) == false) )
  {
    // Must be a flash boot. Set defaults.
    mpLogSeriesEnabled->SetValue(0, true);
    mpLogSeriesItem->SetValue(0, LOG_ITEM_WATER_LEVEL);
    mpLogSeriesSampleRate->SetValue(0, 30);
    mpLogSeriesNoOfSamples->SetValue(0, 1440);

    mpLogSeriesEnabled->SetValue(1, true);
    mpLogSeriesItem->SetValue(1, LOG_ITEM_PUMP_1_ACTUAL_CURRENT);
    mpLogSeriesSampleRate->SetValue(1, 30);
    mpLogSeriesNoOfSamples->SetValue(1, 1440);

    mpLogSeriesEnabled->SetValue(2, true);
    mpLogSeriesItem->SetValue(2, LOG_ITEM_PUMP_2_ACTUAL_CURRENT);
    mpLogSeriesSampleRate->SetValue(2, 30);
    mpLogSeriesNoOfSamples->SetValue(2, 1440);
  }

  // Build lists for internal use
  mOffsetList[0] = 0;
  for (int log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    int no_of_samples = 0;
    int offset = mOffsetList[log_no];
    if (mpLogSeriesEnabled->GetValue(log_no) == true)
    {
      no_of_samples = mpLogSeriesNoOfSamples->GetValue(log_no);
    }
    mpLogSeriesDataOffsetTable->SetValue(log_no, offset);
    // Now update the start of next log series (and at last the end of all data, so index log_no+1 is ok)
    mOffsetList[log_no+1] = offset + no_of_samples + LOG_HEAD_SIZE + LOG_TAIL_SIZE;
    // Clear the index list
    mIndexList[log_no] =  0xFFFFFFFF;
  }

  // Check the log data
  ValidateLogData();

  mLogDataSimulation = false;
  mAwaitStartCount = 2; // Wait a few seconds to ensure that everything is up and running
  mRunRequestedFlag = false;
}


/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LogSeriesCtrl::RunSubTask()
{
  mRunRequestedFlag = false;
  bool config_changed = false;

  if (mpRtcSecondsSince1970.IsUpdated())  // One second has elapsed.
  {
    mpLogSeriesEnabled->Lock();
    mpLogSeriesItem->Lock();
    mpLogSeriesSampleRate->Lock();
    mpLogSeriesNoOfSamples->Lock();
    mpLogSeriesDataOffsetTable->Lock();
    for (int idx=0; idx<NO_OF_LOG_STORAGE; idx++)
    {
      mpLogSeriesData[idx]->Lock();
    }

    if (mAwaitStartCount > 0)
    {
      mAwaitStartCount--;
      // Special power on handling:
      // Mark samples invalid for the time passed since last power down.
      if (mAwaitStartCount == 0)
      {
        FillLogDataAtPowerUp();
      }
    }
    else
    {
      // The next few lines is all the basic functionality for the logging.
      // For each log series, check the if the sample time has elapsed.
      // If so, get the related log value and store it.
      int log_no, offset, sample_rate;
      U32 seconds_since_1970 = mpRtcSecondsSince1970->GetValue();
      for (log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
      {
        offset = mOffsetList[log_no];
        sample_rate = mpLogSeriesSampleRate->GetValue(log_no);
        if ( (mpLogSeriesEnabled->GetValue(log_no) == true)
          && (mLogDataSimulation == false)
          && (sample_rate > 0)
          && (seconds_since_1970 % sample_rate == 0) )
        {
          // Something to be logged
          AddLogValues(offset, GetMeasuredValueToLog(log_no), 1, seconds_since_1970);
        }
      }

      // Now check changes in configuration (but just once per second)
      if (mpLogSeriesEnabled.IsUpdated())
      {
        HandleChangedEnabled();
        config_changed = true;
      }
      if (mpLogSeriesItem.IsUpdated())
      {
        HandleChangedLogItem();
        config_changed = true;
      }
      if (mpLogSeriesSampleRate.IsUpdated())
      {
        HandleChangedSampleRate();
        config_changed = true;
      }
      if (mpLogSeriesNoOfSamples.IsUpdated())
      {
        HandleChangedNoOfSamples();
        config_changed = true;
      }
      if (config_changed == true || (seconds_since_1970 % 600) == 30)
      {
        ValidateLogData();
      }
    }
    mpLogSeriesEnabled->UnLock();
    mpLogSeriesItem->UnLock();
    mpLogSeriesSampleRate->UnLock();
    mpLogSeriesNoOfSamples->UnLock();
    mpLogSeriesDataOffsetTable->UnLock();
    for (int idx=0; idx<NO_OF_LOG_STORAGE; idx++)
    {
      mpLogSeriesData[idx]->UnLock();
    }

    if (config_changed == true)
    {
      // Force saving log series since they are updated to match the new configuration
      ConfigControl::GetInstance()->SaveLogSeriesToFlash();
    }
  }

  // Special test function:
  if (mpFillSimulatedLogData.IsUpdated() && GeniAppTestMode::GetInstance()->GetTestMode() == true)
  {
    mLogDataSimulation = true;
    FillValuesForTest(true);
  }
  else if (mLogDataSimulation == true && GeniAppTestMode::GetInstance()->GetTestMode() == false)
  {
    // If test mode is disabled, then disable the simulation and destroy the simulated data
    mLogDataSimulation = false;
    FillValuesForTest(false);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void LogSeriesCtrl::ConnectToSubjects()
{
  mpRtcSecondsSince1970->Subscribe(this);
  mpLogSeriesEnabled->Subscribe(this);
  mpLogSeriesItem->Subscribe(this);
  mpLogSeriesSampleRate->Subscribe(this);
  mpLogSeriesNoOfSamples->Subscribe(this);

  mpFillSimulatedLogData->Subscribe(this); // Used for test
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void LogSeriesCtrl::Update(Subject* pSubject)
{
  mpRtcSecondsSince1970.Update(pSubject);
  mpLogSeriesEnabled.Update(pSubject);
  mpLogSeriesItem.Update(pSubject);
  mpLogSeriesSampleRate.Update(pSubject);
  mpLogSeriesNoOfSamples.Update(pSubject);
  mpFillSimulatedLogData.Update(pSubject);

  if (mRunRequestedFlag == false)
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
void LogSeriesCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void LogSeriesCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration
    case SP_LOGSE_LOG_SERIES_ENABLED:
      mpLogSeriesEnabled.Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_ITEM:
      mpLogSeriesItem.Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_SAMPLE_RATE:
      mpLogSeriesSampleRate.Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_NO_OF_SAMPLES:
      mpLogSeriesNoOfSamples.Attach(pSubject);
      break;

    // Outputs:
    case SP_LOGSE_LOG_SERIES_DATA_OFFSET_TABLE:
      mpLogSeriesDataOffsetTable.Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_1_DATA:
      mpLogSeriesData[0].Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_2_DATA:
      mpLogSeriesData[1].Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_3_DATA:
      mpLogSeriesData[2].Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_4_DATA:
      mpLogSeriesData[3].Attach(pSubject);
      break;
    case SP_LOGSE_LOG_SERIES_5_DATA:
      mpLogSeriesData[4].Attach(pSubject);
      break;

    // Inputs:
    case SP_LOGSE_FILL_SIMULATED_LOG_DATA:
      mpFillSimulatedLogData.Attach(pSubject);
      break;
    case SP_LOGSE_RTC_SECONDS_SINCE_1970:
      mpRtcSecondsSince1970.Attach(pSubject);
      break;
    // Source value for log series:
    case SP_LOGSE_WATER_SWITCH_LEVEL:
      mpFloatSwitchWaterLevel.Attach(pSubject);
      break;
    case SP_LOGSE_GSM_SIGNAL_LEVEL:
      mpGsmSignalLevel.Attach(pSubject);
      break;
    case SP_LOGSE_MIXER_STARTS_PR_HOUR:
      mpMixerStartsPrHour.Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_1_STARTS_PR_HOUR:
      mpPumpStartsPrHour[PUMP_1].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_2_STARTS_PR_HOUR:
      mpPumpStartsPrHour[PUMP_2].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_3_STARTS_PR_HOUR:
      mpPumpStartsPrHour[PUMP_3].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_4_STARTS_PR_HOUR:
      mpPumpStartsPrHour[PUMP_4].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_5_STARTS_PR_HOUR:
      mpPumpStartsPrHour[PUMP_5].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_6_STARTS_PR_HOUR:
      mpPumpStartsPrHour[PUMP_6].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_1_LATEST_RUN_TIME:
      mpPumpLatestRunTime[PUMP_1].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_2_LATEST_RUN_TIME:
      mpPumpLatestRunTime[PUMP_2].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_3_LATEST_RUN_TIME:
      mpPumpLatestRunTime[PUMP_3].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_4_LATEST_RUN_TIME:
      mpPumpLatestRunTime[PUMP_4].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_5_LATEST_RUN_TIME:
      mpPumpLatestRunTime[PUMP_5].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_6_LATEST_RUN_TIME:
      mpPumpLatestRunTime[PUMP_6].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_1_MOISTURE_SWITCH:
      mpPumpMoistureSwitch[PUMP_1].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_2_MOISTURE_SWITCH:
      mpPumpMoistureSwitch[PUMP_2].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_3_MOISTURE_SWITCH:
      mpPumpMoistureSwitch[PUMP_3].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_4_MOISTURE_SWITCH:
      mpPumpMoistureSwitch[PUMP_4].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_5_MOISTURE_SWITCH:
      mpPumpMoistureSwitch[PUMP_5].Attach(pSubject);
      break;
    case SP_LOGSE_PUMP_6_MOISTURE_SWITCH:
      mpPumpMoistureSwitch[PUMP_6].Attach(pSubject);
      break;
    case SP_LOGSE_TOTAL_USD_CNT_1:
      mpUserDefinedCounter_1.Attach(pSubject);
      break;
    case SP_LOGSE_TOTAL_USD_CNT_2:
      mpUserDefinedCounter_2.Attach(pSubject);
      break;
    case SP_LOGSE_TOTAL_USD_CNT_3:
      mpUserDefinedCounter_3.Attach(pSubject);
      break;
    default:
      // For float data point log values: Look up the relation in a table and attach the subject
      for (int index=0; index<FLOAT_LOG_ITEM_TABLE_SIZE; index++)
      {
        if (id == float_log_item_table[index].subjectRelation)
        {
          mpSourceValueList[index].Attach(pSubject);
          break;
        }
      }
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
 * Function - GetLogValue
 * DESCRIPTION: Get a log value from the log storage
 *              (no matter how the storage handling is working)
 *
 *****************************************************************************/
U16 LogSeriesCtrl::GetLogValue(int offset)
{
  int idx = offset/NO_OF_WORDS_PER_STORAGE;
  if (idx < NO_OF_LOG_STORAGE)
  {
    return mpLogSeriesData[idx]->GetValue(offset%NO_OF_WORDS_PER_STORAGE);
  }
  return(0);
}
/*****************************************************************************
 * Function - SetLogValue
 * DESCRIPTION: Set a log value in the log storage
 *              (no matter how the storage handling is implemented)
 *
 *****************************************************************************/
void LogSeriesCtrl::SetLogValue(int offset, U16 value)
{
  int idx = offset/NO_OF_WORDS_PER_STORAGE;
  if (idx < NO_OF_LOG_STORAGE)
  {
    mpLogSeriesData[idx]->SetValue(offset%NO_OF_WORDS_PER_STORAGE, value);
  }
}
/*****************************************************************************
 * Function - FillLogValues
 * DESCRIPTION: Fill a number of log value in the log storage
 *
 *****************************************************************************/
void LogSeriesCtrl::FillLogValues(int dest, U16 value, int count)
{
  while (count > 0)
  {
    SetLogValue(dest, value);
    dest++; count--;
  }
}
/*****************************************************************************
 * Function - CopyLogValues
 * DESCRIPTION: Copy a number of log value in the log storage
 *              (and overlap is allowed)
 *
 *****************************************************************************/
void LogSeriesCtrl::CopyLogValues(int dest, int src, int count)
{
  if (src > dest) // Start from bottom in case of overlap when src > dest
  {
    while (count > 0)
    {
      SetLogValue(dest, GetLogValue(src));
      dest++; src++; count--;
    }
  }
  else if (src < dest)// Start from top in case of overlap when src < dest
  {
    dest+=count; src+=count;
    while (count > 0)
    {
      dest--; src--; count--;
      SetLogValue(dest, GetLogValue(src));
    }
  }
}

/*****************************************************************************
 * Function - ReorganizeLogValues
 * DESCRIPTION: Make the data within a log series start from cyclic index 0
 *
 *****************************************************************************/
void LogSeriesCtrl::ReorganizeLogValues(int offset)
{
  if (GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE)
  {
    U16 cyclic_index = GetLogValue(offset+LOG_CYCLIC_IDX);
    U16 no_of_samples = GetLogValue(offset+LOG_SIZE_IDX);

    if (cyclic_index > 0 && no_of_samples > cyclic_index)
    {
      U16 *temp_buffer = new U16[no_of_samples];
      if (temp_buffer != NULL)
      {
        int i, base;
        base = offset+LOG_DATA_IDX;
        // Move oldest data to temp buffer (data below cyclic index)
        for (i=0; i<cyclic_index; i++)
        {
          temp_buffer[i] = GetLogValue(base+i);
        }
        // Move newest data down to index 0 (data at and above cyclic index)
        CopyLogValues(offset+LOG_DATA_IDX+0, offset+LOG_DATA_IDX+cyclic_index, no_of_samples-cyclic_index);
        // Move oldest data from temp buffer to the upper part of the log data
        base = offset+LOG_DATA_IDX+no_of_samples-cyclic_index;
        for (i=0; i<cyclic_index; i++)
        {
          SetLogValue(base+i, temp_buffer[i]);
        }
        SetLogValue(offset+LOG_CYCLIC_IDX, 0);
        delete[] temp_buffer;
      }
    }
  }
}

/*****************************************************************************
 * Function - AddLogValues
 * DESCRIPTION: Add log values (cyclic) to a log series and set the time stamp
 *
 *****************************************************************************/
void LogSeriesCtrl::AddLogValues(int offset, U16 value, int count, U32 time_stamp)
{
  int no_of_samples = GetLogValue(offset+LOG_SIZE_IDX);
  int cyclic_index = GetLogValue(offset+LOG_CYCLIC_IDX);
  if ( (GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE)
    && (no_of_samples > 0)
    && (no_of_samples > cyclic_index) )
  {
    while (count > 0)
    {
      if (cyclic_index > 0)
      {
        cyclic_index--;
      }
      else
      {
        cyclic_index = no_of_samples-1;
      }
      SetLogValue(offset+LOG_DATA_IDX+cyclic_index, value);
      count--;
    }
    SetLogValue(offset+LOG_CYCLIC_IDX, cyclic_index);
    SetLogValue(offset+LOG_STAMP_HI_IDX, time_stamp/0x10000);
    SetLogValue(offset+LOG_STAMP_LO_IDX, time_stamp%0x10000);
    // Use the time stamp as an extra validation in case the log series is stored in different flash blocks
    SetLogValue(offset+no_of_samples+LOG_CHECK_IDX, time_stamp%0x10000);
  }
}

/*****************************************************************************
 * Function - ValidateLogData
 * DESCRIPTION: Validate the log data header versus the log configuration
 *
 *****************************************************************************/
void LogSeriesCtrl::ValidateLogData(void)
{
  U16 log_item, sample_rate, no_of_samples, cyclic_index, low_stamp, check_stamp;
  for (int log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    int offset    = mOffsetList[log_no];
    log_item      = mpLogSeriesItem->GetValue(log_no);
    sample_rate   = mpLogSeriesSampleRate->GetValue(log_no);
    no_of_samples = 0;
    if (mpLogSeriesEnabled->GetValue(log_no) == true)
    {
      no_of_samples = mpLogSeriesNoOfSamples->GetValue(log_no);
    }
    cyclic_index  = GetLogValue(offset+LOG_CYCLIC_IDX);
    low_stamp     = GetLogValue(offset+LOG_STAMP_LO_IDX);
    check_stamp   = GetLogValue(offset+no_of_samples+LOG_CHECK_IDX);
    if ( (GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE)
      && ((GetLogValue(offset+LOG_NUMBER_IDX)&0x7F) == (log_no+1))
      && (GetLogValue(offset+LOG_ITEM_IDX) == log_item)
      && (GetLogValue(offset+LOG_RATE_IDX) == sample_rate)
      && (GetLogValue(offset+LOG_SIZE_IDX) == no_of_samples)
      && ((no_of_samples == 0 || no_of_samples > cyclic_index))
      && (low_stamp == check_stamp) )
    {
      ;  // Everything seems ok
    }
    else // Something is wrong. Create a valid header and mark data invalid for the log series
    {
      SetLogValue(offset+LOG_HEAD_IDX, LOG_HEAD_CHECK_VALUE);
      SetLogValue(offset+LOG_NUMBER_IDX, (LOG_HEAD_VERSION<<8) | (mpLogSeriesEnabled->GetValue(log_no)<<7) | (log_no+1));
      SetLogValue(offset+LOG_ITEM_IDX, log_item);
      SetLogValue(offset+LOG_RATE_IDX, sample_rate);
      SetLogValue(offset+LOG_SIZE_IDX, no_of_samples);
      SetLogValue(offset+LOG_CYCLIC_IDX, 0);
      FillLogValues(offset+LOG_DATA_IDX, INVALID_MARK, no_of_samples); // Mark all data invalid
      SetLogValue(offset+no_of_samples+LOG_CHECK_IDX, low_stamp);
      SetLogValue(offset+no_of_samples+LOG_TAIL_IDX, LOG_TAIL_CHECK_VALUE);
    }
  }
}

/*****************************************************************************
 * Function - FillLogDataAtPowerUp
 * DESCRIPTION: Mark samples invalid for the time passed since last power down.
 *              (i.e. fill in FFFF's in log data for the no of samples passed)
 *
 *****************************************************************************/
void LogSeriesCtrl::FillLogDataAtPowerUp(void)
{
  int   log_no, sample_rate, no_of_samples, offset, samples_to_fill;
  U32   last_log_time_stamp, seconds_since_1970 = mpRtcSecondsSince1970->GetValue();

  for (log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    offset = mOffsetList[log_no];
    sample_rate = mpLogSeriesSampleRate->GetValue(log_no);
    no_of_samples = mpLogSeriesNoOfSamples->GetValue(log_no);
    if ( (mpLogSeriesEnabled->GetValue(log_no) == true)
      && (sample_rate > 0)
      && (no_of_samples > 0)
      && (GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE) )
    {
      last_log_time_stamp = 0x10000L*GetLogValue(offset+LOG_STAMP_HI_IDX) + GetLogValue(offset+LOG_STAMP_LO_IDX);
      if (seconds_since_1970-sample_rate >= last_log_time_stamp)
      {
        // More than one sample has been lost, add these samples as invalid
        samples_to_fill = (seconds_since_1970-last_log_time_stamp)/sample_rate;
        if (samples_to_fill > no_of_samples)
        {
          samples_to_fill = no_of_samples;
        }
        AddLogValues(offset, INVALID_MARK, samples_to_fill, seconds_since_1970);
      }
    }
  }
}

/*****************************************************************************
 * Function - FillValuesForTest
 * DESCRIPTION: Fill in some fixed values in the different log vectors.
 *              For test purpose only !
 *
 *****************************************************************************/
void LogSeriesCtrl::FillValuesForTest(bool test_active)
{
  U32 seconds_since_1970 = mpRtcSecondsSince1970->GetValue();
  for (int log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    if (mpLogSeriesEnabled->GetValue(log_no) == true) // Fill in some simulated values
    {
      U32 no_of_samples = mpLogSeriesNoOfSamples->GetValue(log_no);
      for (int i=0; i<no_of_samples; i++)
      {
        U16 value = 0xFFFF;
        if (test_active == true)
        {
          // Make some kind of simulation data, e.g. log item = water level(1)
          // Data = 1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4 etc.
          value = mpLogSeriesItem->GetValue(log_no) + i/8;
        }
        AddLogValues(mOffsetList[log_no], value, 1, seconds_since_1970);
      }
    }
  }
}

/*****************************************************************************
 * Function - HandleChangedEnabled
 * DESCRIPTION: Set the enabled/disabled flag in the log series data
 *
 *****************************************************************************/
void LogSeriesCtrl::HandleChangedEnabled(void)
{
  int log_no, offset;
  U16 number, old_e_d, new_e_d;
  for (log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    offset = mOffsetList[log_no];
    number = GetLogValue(offset+LOG_NUMBER_IDX);
    old_e_d = number & (1<<7);
    new_e_d = mpLogSeriesEnabled->GetValue(log_no)<<7;
    if (GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE && (number & 0x7F) == log_no+1)
    {
      if (new_e_d != old_e_d)
      {
        SetLogValue(offset+LOG_NUMBER_IDX, (LOG_HEAD_VERSION<<8) | new_e_d | (log_no+1));
        // Extra update since the number of samples is treated as 0 when a log series is disabled
        mpLogSeriesNoOfSamples.SetUpdated();
      }
    }
  }
}

/*****************************************************************************
 * Function - HandleChangedLogItem
 * DESCRIPTION: Check if a log item is changed and then mark the data invalid
 *
 *****************************************************************************/
void LogSeriesCtrl::HandleChangedLogItem(void)
{
  U16 new_item, old_item;
  int log_no, offset;

  for (log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    offset = mOffsetList[log_no];
    new_item = mpLogSeriesItem->GetValue(log_no);
    old_item = GetLogValue(offset+LOG_ITEM_IDX);
    if (old_item != new_item && GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE)
    {
      int no_of_samples = GetLogValue(offset+LOG_SIZE_IDX);
      if (no_of_samples > mpLogSeriesNoOfSamples->GetValue(log_no))
      {
        // Important: Since the number of samles could have been changed as well, use the smallest old/new value
        no_of_samples = mpLogSeriesNoOfSamples->GetValue(log_no);
      }
      FillLogValues(offset+LOG_DATA_IDX, INVALID_MARK, no_of_samples); // Mark all log data invalid
      SetLogValue(offset+LOG_ITEM_IDX, new_item);
      SetLogValue(offset+LOG_CYCLIC_IDX, 0);
      SetLogValue(offset+LOG_STAMP_HI_IDX, 0);
      SetLogValue(offset+LOG_STAMP_LO_IDX, 0);
      SetLogValue(offset+no_of_samples+LOG_CHECK_IDX, 0);
      mIndexList[log_no] = 0xFFFFFFFF; // To clear the source value reference
    }
  }
}

/*****************************************************************************
 * Function - HandleChangedSampleRate
 * DESCRIPTION: Set the new sample rate in the log series data.
 *              Move existing samples to match the new rate.
 *
 *****************************************************************************/
void LogSeriesCtrl::HandleChangedSampleRate(void)
{
  int log_no, offset;
  U16 number;
  for (log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    offset = mOffsetList[log_no];
    number = GetLogValue(offset+LOG_NUMBER_IDX);
    if (GetLogValue(offset+LOG_HEAD_IDX) == LOG_HEAD_CHECK_VALUE && (number & 0x7F) == log_no+1)
    {
      int new_rate = mpLogSeriesSampleRate->GetValue(log_no);
      int old_rate = GetLogValue(offset+LOG_RATE_IDX);
      if (new_rate != old_rate && new_rate > 0 && old_rate > 0)
      {
        int no_of_samples = GetLogValue(offset+LOG_SIZE_IDX);
        ReorganizeLogValues(offset);
        SetLogValue(offset+LOG_RATE_IDX, new_rate);
        if (new_rate > old_rate)
        {
          // Sample rate increased. Pick every x of the existing samples
          int old_idx, new_idx = 0;
          for (old_idx=0; old_idx<no_of_samples; old_idx++)
          {
            if (old_idx*old_rate >= new_idx*new_rate)
            {
              SetLogValue(offset+LOG_DATA_IDX+new_idx, GetLogValue(offset+LOG_DATA_IDX+old_idx));
              new_idx++;
            }
          }
          // Then fill up with invalids
          FillLogValues(offset+LOG_DATA_IDX+new_idx, INVALID_MARK, no_of_samples-new_idx);
        }
        else
        {
          // Sample rate decreased. Insert invalids in every x of the existing samples
          int new_idx, old_idx = ((no_of_samples-1)*new_rate)/old_rate;
          for (new_idx=no_of_samples-1; new_idx>=0; new_idx--)
          {
            U16 value = INVALID_MARK;
            if (old_idx*old_rate >= new_idx*new_rate)
            {
              value = GetLogValue(offset+LOG_DATA_IDX+old_idx);
              old_idx--;
            }
            SetLogValue(offset+LOG_DATA_IDX+new_idx, value);
          }
        }
      }
    }
  }
}

/*****************************************************************************
 * Function - HandleChangedNoOfSamples
 * DESCRIPTION: Reorganize the log data to match the actual no of samples
 *
 *****************************************************************************/
void LogSeriesCtrl::HandleChangedNoOfSamples(void)
{
  U16 new_size, old_size;
  int log_no = 0, expand_count = 0;
  int old_offset = 0, new_offset = 0;

  // Step 1: First shrink the log data storage and keep track of how much to expand later
  for (log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    old_offset = mpLogSeriesDataOffsetTable->GetValue(log_no);
    new_offset = mOffsetList[log_no];
    old_size = GetLogValue(old_offset+LOG_SIZE_IDX);
    new_size = 0;
    if (mpLogSeriesEnabled->GetValue(log_no) == true)
    {
      new_size = mpLogSeriesNoOfSamples->GetValue(log_no);
    }
    if (new_size < old_size)
    {
      // Less data for this log, copy with new size
      ReorganizeLogValues(old_offset);
      CopyLogValues(new_offset, old_offset, new_size + LOG_HEAD_SIZE);
      mOffsetList[log_no+1] = new_offset + new_size + LOG_HEAD_SIZE + LOG_TAIL_SIZE;
      SetLogValue(new_offset+LOG_SIZE_IDX, new_size);
    }
    else
    {
      // More or equal data for this log, copy with old size, i.e. move data
      CopyLogValues(new_offset, old_offset, old_size + LOG_HEAD_SIZE);
      mOffsetList[log_no+1] = new_offset + old_size + LOG_HEAD_SIZE + LOG_TAIL_SIZE;
      expand_count += (new_size - old_size);
    }
    mpLogSeriesDataOffsetTable->SetValue(log_no, new_offset);
  }

  // Step 2: Then expand the log data storage and fill in invalid marks
  if (expand_count > 0)
  {
    // Expand from the max location to be used and go down throug all log series
    mOffsetList[MAX_NO_OF_LOG_SERIES] += expand_count;
    for (log_no=MAX_NO_OF_LOG_SERIES-1; log_no>=0; log_no--)
    {
      old_offset = mpLogSeriesDataOffsetTable->GetValue(log_no);
      old_size = GetLogValue(old_offset+LOG_SIZE_IDX);
      new_size = 0;
      if (mpLogSeriesEnabled->GetValue(log_no) == true)
      {
        new_size = mpLogSeriesNoOfSamples->GetValue(log_no);
      }
      new_offset = mOffsetList[log_no+1] - new_size - (LOG_HEAD_SIZE+LOG_TAIL_SIZE);
      if (new_size < old_size)
      {
        CopyLogValues(new_offset, old_offset, 0); // Less data, should not be possible here. Just a break point.
      }
      else if (new_size == old_size)
      {
        // Same size, i.e. just move data
        CopyLogValues(new_offset, old_offset, old_size + LOG_HEAD_SIZE);
      }
      else
      {
        // More data for this log, reorganize and copy with old size and fill up with invalids
        ReorganizeLogValues(old_offset);
        CopyLogValues(new_offset, old_offset, old_size + LOG_HEAD_SIZE);
        FillLogValues(new_offset+old_size+LOG_HEAD_SIZE, INVALID_MARK, new_size-old_size);
        SetLogValue(new_offset+LOG_SIZE_IDX, new_size);
      }
      mpLogSeriesDataOffsetTable->SetValue(log_no, new_offset);
      mOffsetList[log_no] = new_offset;
    }
  }

  // Step 3: Set the tailing check data for the log series
  for (int log_no=0; log_no<MAX_NO_OF_LOG_SERIES; log_no++)
  {
    int offset        = mOffsetList[log_no];
    int no_of_samples = GetLogValue(offset+LOG_SIZE_IDX);
    SetLogValue(offset+no_of_samples+LOG_CHECK_IDX, GetLogValue(offset+LOG_STAMP_LO_IDX));
    SetLogValue(offset+no_of_samples+LOG_TAIL_IDX, LOG_TAIL_CHECK_VALUE);
  }
}

/*****************************************************************************
 * Function - GetMeasuredValueToLog
 * DESCRIPTION: Get the actual source value to log in the normal run situation.
 *              The value is scaled and returned as an U16.
 *
 *****************************************************************************/
U16 LogSeriesCtrl::GetMeasuredValueToLog(int log_no)
{
  U16 return_value = INVALID_MARK;
  U32 value_index  = mIndexList[log_no];

  if (value_index > FLOAT_LOG_ITEM_TABLE_SIZE)
  {
    // The value index not initialized yet. Try to find the log item in the float data point table
    U16 log_item = mpLogSeriesItem->GetValue(log_no);
    for (int index=0; index<FLOAT_LOG_ITEM_TABLE_SIZE; index++)
    {
      if (log_item == float_log_item_table[index].logItem)
      {
        mIndexList[log_no] = index;
        value_index = mIndexList[log_no];
        break;
      }
    }
  }

  if (value_index < FLOAT_LOG_ITEM_TABLE_SIZE) // It's a FloatDataPoint, i.e. handled by look up in list
  {
    SubjectPtr<FloatDataPoint*> pSubject;
    pSubject = mpSourceValueList[value_index];
    if (pSubject->GetQuality() == DP_AVAILABLE)
    {
      float scale = float_log_item_table[value_index].scale;
      float value = pSubject->GetValue();
      if (scale >= 0.0f) // A multiply factor
      {
        return_value = value*scale + 0.5f;
      }
      else if (value > -scale) // An offset
      {
        return_value = value + scale + 0.5f;
      }
    }
    // For simple test: return_value = (mpRtcSecondsSince1970->GetValue() % 120) + log_no*256;
  }
  else // Dedicated handling for other than FloatDataPoint's
  {
    mIndexList[log_no] = FLOAT_LOG_ITEM_TABLE_SIZE; // Valid, but not a FloatDataPoint. Use some dedicated handling
    switch (mpLogSeriesItem->GetValue(log_no))
    {
      default:
      case LOG_ITEM_NOT_USED:
        return_value = INVALID_MARK;
        break;
      case LOG_ITEM_SWITCH_WATER_LEVEL:
        if (mpFloatSwitchWaterLevel->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpFloatSwitchWaterLevel->GetValue();
        }
        break;
      case LOG_ITEM_SIGNAL_LEVEL:
        if (mpGsmSignalLevel->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpGsmSignalLevel->GetValue();
        }
        break;
      case LOG_ITEM_MIXER_STARTS_PR_HOUR:
        if (mpMixerStartsPrHour->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpMixerStartsPrHour->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_1_STARTS_PR_HOUR:
        if (mpPumpStartsPrHour[0]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpStartsPrHour[0]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_2_STARTS_PR_HOUR:
        if (mpPumpStartsPrHour[1]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpStartsPrHour[1]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_3_STARTS_PR_HOUR:
        if (mpPumpStartsPrHour[2]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpStartsPrHour[2]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_4_STARTS_PR_HOUR:
        if (mpPumpStartsPrHour[3]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpStartsPrHour[3]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_5_STARTS_PR_HOUR:
        if (mpPumpStartsPrHour[4]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpStartsPrHour[4]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_6_STARTS_PR_HOUR:
        if (mpPumpStartsPrHour[5]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpStartsPrHour[5]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_1_LATEST_RUN_TIME:
        if (mpPumpLatestRunTime[0]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpLatestRunTime[0]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_2_LATEST_RUN_TIME:
        if (mpPumpLatestRunTime[1]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpLatestRunTime[1]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_3_LATEST_RUN_TIME:
        if (mpPumpLatestRunTime[2]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpLatestRunTime[2]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_4_LATEST_RUN_TIME:
        if (mpPumpLatestRunTime[3]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpLatestRunTime[3]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_5_LATEST_RUN_TIME:
        if (mpPumpLatestRunTime[4]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpLatestRunTime[4]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_6_LATEST_RUN_TIME:
        if (mpPumpLatestRunTime[5]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpLatestRunTime[5]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_1_MOISTURE_SWITCH:
        if (mpPumpMoistureSwitch[0]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpMoistureSwitch[0]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_2_MOISTURE_SWITCH:
        if (mpPumpMoistureSwitch[1]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpMoistureSwitch[1]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_3_MOISTURE_SWITCH:
        if (mpPumpMoistureSwitch[2]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpMoistureSwitch[2]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_4_MOISTURE_SWITCH:
        if (mpPumpMoistureSwitch[3]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpMoistureSwitch[3]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_5_MOISTURE_SWITCH:
        if (mpPumpMoistureSwitch[4]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpMoistureSwitch[4]->GetValue();
        }
        break;
      case LOG_ITEM_PUMP_6_MOISTURE_SWITCH:
        if (mpPumpMoistureSwitch[5]->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpPumpMoistureSwitch[5]->GetValue();
        }
        break;
      case LOG_ITEM_USER_DEFINED_COUNTER_1:
        if (mpUserDefinedCounter_1->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpUserDefinedCounter_1->GetValue();
        }
        break;
      case LOG_ITEM_USER_DEFINED_COUNTER_2:
        if (mpUserDefinedCounter_2->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpUserDefinedCounter_2->GetValue();
        }
        break;
      case LOG_ITEM_USER_DEFINED_COUNTER_3:
        if (mpUserDefinedCounter_3->GetQuality() == DP_AVAILABLE)
        {
          return_value = mpUserDefinedCounter_3->GetValue();
        }
        break;
    }
  }

  return (return_value);
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
