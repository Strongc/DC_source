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
/* CLASS NAME       : AlarmLogDataHistory                                   */
/*                                                                          */
/* FILE NAME        : AlarmLogDataHistory.cpp                               */
/*                                                                          */
/* CREATED DATE     : 17-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <ActTime.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmLogDataHistory.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  CREATES AN OBJECT.
 ******************************************************************************/
AlarmLogDataHistory* AlarmLogDataHistory::mInstance = 0;


/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
AlarmLogDataHistory* AlarmLogDataHistory::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new AlarmLogDataHistory();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
AlarmLogDataHistory::AlarmLogDataHistory()
{
  OS_CREATERSEMA(&mSemaDataHistory);

  mDataHistoryIndex = 0;
  mDataHistoryFull = false;

  mScanSkipCounter = 0;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AlarmLogDataHistory::~AlarmLogDataHistory()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLogDataHistory::InitSubTask(void)
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLogDataHistory::RunSubTask(void)
{
  if (++mScanSkipCounter < 50) //Only run every 500 ms
  {
    return;
  }
  mScanSkipCounter = 0;

  DataHistoryEntry h;

  h.mSurfaceLevelReady = mpSurfaceLevelReady ->GetValue();
  h.mSurfaceLevel      = mpSurfaceLevel      ->GetValue();
  h.mAverageFlow       = mpAverageFlow       ->GetValue();

  h.mSurfaceLevelReadyQuality = mpSurfaceLevelReady ->GetQuality();
  h.mSurfaceLevelQuality      = mpSurfaceLevel      ->GetQuality();
  h.mAverageFlowQuality       = mpAverageFlow       ->GetQuality();

  for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
  {
    h.mPumpOperatingMode[i] = mpPumpOperatingMode[i] ->GetValue();
    h.mPumpVoltage[i]       = mpPumpVoltage[i]       ->GetValue();
    h.mPumpCurrent[i]       = mpPumpCurrent[i]       ->GetValue();
    h.mPumpCosPhi[i]        = mpPumpCosPhi[i]        ->GetValue();
    h.mPumpPower[i]         = mpPumpPower[i]         ->GetValue();
    h.mPumpFlow[i]          = mpPumpFlow[i]          ->GetValue();
    h.mPumpTemperature[i]   = mpPumpTemperature[i]   ->GetValue();

    h.mPumpOperatingModeQuality[i] = mpPumpOperatingMode[i] ->GetQuality();
    h.mPumpVoltageQuality[i]       = mpPumpVoltage[i]       ->GetQuality();
    h.mPumpCurrentQuality[i]       = mpPumpCurrent[i]       ->GetQuality();
    h.mPumpCosPhiQuality[i]        = mpPumpCosPhi[i]        ->GetQuality();
    h.mPumpPowerQuality[i]         = mpPumpPower[i]         ->GetQuality();
    h.mPumpFlowQuality[i]          = mpPumpFlow[i]          ->GetQuality();
    h.mPumpTemperatureQuality[i]   = mpPumpTemperature[i]   ->GetQuality();
  }

  OS_Use(&mSemaDataHistory);

  mDataHistory[mDataHistoryIndex] = h;

  if (++mDataHistoryIndex >= (sizeof(mDataHistory) / sizeof(*mDataHistory)))
  {
    mDataHistoryIndex = 0;
    mDataHistoryFull = true;
  }

  OS_Unuse(&mSemaDataHistory);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLogDataHistory::ConnectToSubjects()
{
  //ignore
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLogDataHistory::Update(Subject* pSubject)
{
  //ignore
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION:
*
*****************************************************************************/
void AlarmLogDataHistory::SubscribtionCancelled(Subject* pSubject)
{
  mpSurfaceLevelReady .Detach(pSubject);
  mpSurfaceLevel      .Detach(pSubject);
  mpAverageFlow       .Detach(pSubject);

  for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
  {
    mpPumpOperatingMode[i] .Detach(pSubject);
    mpPumpVoltage[i]       .Detach(pSubject);
    mpPumpCurrent[i]       .Detach(pSubject);
    mpPumpCosPhi[i]        .Detach(pSubject);
    mpPumpPower[i]         .Detach(pSubject);
    mpPumpFlow[i]          .Detach(pSubject);
    mpPumpTemperature[i]   .Detach(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void AlarmLogDataHistory::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    case SP_ALDH_SURFACE_LEVEL_READY:                mpSurfaceLevelReady         .Attach(pSubject); break;
    case SP_ALDH_SURFACE_LEVEL:                      mpSurfaceLevel              .Attach(pSubject); break;
    case SP_ALDH_AVERAGE_FLOW:                       mpAverageFlow               .Attach(pSubject); break;

    case SP_ALDH_PUMP_1_STARTED:                     mpPumpOperatingMode[PUMP_1] .Attach(pSubject); break;
    case SP_ALDH_PUMP_1_AVERAGE_VOLTAGE:             mpPumpVoltage[PUMP_1]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_1_AVERAGE_CURRENT:             mpPumpCurrent[PUMP_1]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_1_COS_PHI:                     mpPumpCosPhi[PUMP_1]        .Attach(pSubject); break;
    case SP_ALDH_PUMP_1_POWER:                       mpPumpPower[PUMP_1]         .Attach(pSubject); break;
    case SP_ALDH_PUMP_1_AVERAGE_FLOW:                mpPumpFlow[PUMP_1]          .Attach(pSubject); break;
    case SP_ALDH_PUMP_1_TEMPERATURE:                 mpPumpTemperature[PUMP_1]   .Attach(pSubject); break;

    case SP_ALDH_PUMP_2_STARTED:                     mpPumpOperatingMode[PUMP_2] .Attach(pSubject); break;
    case SP_ALDH_PUMP_2_AVERAGE_VOLTAGE:             mpPumpVoltage[PUMP_2]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_2_AVERAGE_CURRENT:             mpPumpCurrent[PUMP_2]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_2_COS_PHI:                     mpPumpCosPhi[PUMP_2]        .Attach(pSubject); break;
    case SP_ALDH_PUMP_2_POWER:                       mpPumpPower[PUMP_2]         .Attach(pSubject); break;
    case SP_ALDH_PUMP_2_AVERAGE_FLOW:                mpPumpFlow[PUMP_2]          .Attach(pSubject); break;
    case SP_ALDH_PUMP_2_TEMPERATURE:                 mpPumpTemperature[PUMP_2]   .Attach(pSubject); break;

    case SP_ALDH_PUMP_3_STARTED:                     mpPumpOperatingMode[PUMP_3] .Attach(pSubject); break;
    case SP_ALDH_PUMP_3_AVERAGE_VOLTAGE:             mpPumpVoltage[PUMP_3]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_3_AVERAGE_CURRENT:             mpPumpCurrent[PUMP_3]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_3_COS_PHI:                     mpPumpCosPhi[PUMP_3]        .Attach(pSubject); break;
    case SP_ALDH_PUMP_3_POWER:                       mpPumpPower[PUMP_3]         .Attach(pSubject); break;
    case SP_ALDH_PUMP_3_AVERAGE_FLOW:                mpPumpFlow[PUMP_3]          .Attach(pSubject); break;
    case SP_ALDH_PUMP_3_TEMPERATURE:                 mpPumpTemperature[PUMP_3]   .Attach(pSubject); break;

    case SP_ALDH_PUMP_4_STARTED:                     mpPumpOperatingMode[PUMP_4] .Attach(pSubject); break;
    case SP_ALDH_PUMP_4_AVERAGE_VOLTAGE:             mpPumpVoltage[PUMP_4]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_4_AVERAGE_CURRENT:             mpPumpCurrent[PUMP_4]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_4_COS_PHI:                     mpPumpCosPhi[PUMP_4]        .Attach(pSubject); break;
    case SP_ALDH_PUMP_4_POWER:                       mpPumpPower[PUMP_4]         .Attach(pSubject); break;
    case SP_ALDH_PUMP_4_AVERAGE_FLOW:                mpPumpFlow[PUMP_4]          .Attach(pSubject); break;
    case SP_ALDH_PUMP_4_TEMPERATURE:                 mpPumpTemperature[PUMP_4]   .Attach(pSubject); break;

    case SP_ALDH_PUMP_5_STARTED:                     mpPumpOperatingMode[PUMP_5] .Attach(pSubject); break;
    case SP_ALDH_PUMP_5_AVERAGE_VOLTAGE:             mpPumpVoltage[PUMP_5]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_5_AVERAGE_CURRENT:             mpPumpCurrent[PUMP_5]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_5_COS_PHI:                     mpPumpCosPhi[PUMP_5]        .Attach(pSubject); break;
    case SP_ALDH_PUMP_5_POWER:                       mpPumpPower[PUMP_5]         .Attach(pSubject); break;
    case SP_ALDH_PUMP_5_AVERAGE_FLOW:                mpPumpFlow[PUMP_5]          .Attach(pSubject); break;
    case SP_ALDH_PUMP_5_TEMPERATURE:                 mpPumpTemperature[PUMP_5]   .Attach(pSubject); break;

    case SP_ALDH_PUMP_6_STARTED:                     mpPumpOperatingMode[PUMP_6] .Attach(pSubject); break;
    case SP_ALDH_PUMP_6_AVERAGE_VOLTAGE:             mpPumpVoltage[PUMP_6]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_6_AVERAGE_CURRENT:             mpPumpCurrent[PUMP_6]       .Attach(pSubject); break;
    case SP_ALDH_PUMP_6_COS_PHI:                     mpPumpCosPhi[PUMP_6]        .Attach(pSubject); break;
    case SP_ALDH_PUMP_6_POWER:                       mpPumpPower[PUMP_6]         .Attach(pSubject); break;
    case SP_ALDH_PUMP_6_AVERAGE_FLOW:                mpPumpFlow[PUMP_6]          .Attach(pSubject); break;
    case SP_ALDH_PUMP_6_TEMPERATURE:                 mpPumpTemperature[PUMP_6]   .Attach(pSubject); break;
  }
}


/*****************************************************************************
 * Function - GetBestDataHistoryEntry
 * DESCRIPTION: Get the history entry where the current of the pump indicated
 * by (the zero-based) pumpErrorIndex was highest.
 * If that pump's current was only 0.0, or if pumpErrorIndex < 0, use the
 * first pump with current != 0.0.
 *
 *****************************************************************************/
const AlarmLogDataHistory::DataHistoryEntry AlarmLogDataHistory::GetBestDataHistoryEntry(int pumpErrorIndex) const
{
  DataHistoryEntry retval;

  OS_Use(&mSemaDataHistory);

  if (!mDataHistoryFull && mDataHistoryIndex == 0)
  {
    retval.mSurfaceLevelReadyQuality = DP_NEVER_AVAILABLE;
    retval.mSurfaceLevelQuality      = DP_NEVER_AVAILABLE;
    retval.mAverageFlowQuality       = DP_NEVER_AVAILABLE;

    for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
    {
      retval.mPumpOperatingModeQuality[i] = DP_NEVER_AVAILABLE;
      retval.mPumpVoltageQuality[i]       = DP_NEVER_AVAILABLE;
      retval.mPumpCurrentQuality[i]       = DP_NEVER_AVAILABLE;
      retval.mPumpCosPhiQuality[i]        = DP_NEVER_AVAILABLE;
      retval.mPumpPowerQuality[i]         = DP_NEVER_AVAILABLE;
      retval.mPumpFlowQuality[i]          = DP_NEVER_AVAILABLE;
      retval.mPumpTemperatureQuality[i]   = DP_NEVER_AVAILABLE;
    }
  }
  else
  {
    // Find the entry with the highest current for each pump. When multiple entries share the same high current, use the latest entry.

    float highest_currents[NO_OF_PUMPS];
    unsigned highest_current_entry_indices[NO_OF_PUMPS];

    for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
    {
      highest_currents[i] = 0.0;
      highest_current_entry_indices[i] = 0;
    }

    unsigned int entry_index = mDataHistoryFull ? mDataHistoryIndex : 0;
    do
    {
      for (unsigned int pump_index = 0; pump_index < NO_OF_PUMPS; ++pump_index)
      {
        if (mDataHistory[entry_index].mPumpCurrentQuality[pump_index] == DP_AVAILABLE && mDataHistory[entry_index].mPumpCurrent[pump_index] >= highest_currents[pump_index])
        {
          highest_currents[pump_index] = mDataHistory[entry_index].mPumpCurrent[pump_index];
          highest_current_entry_indices[pump_index] = entry_index;
        }
      }

      if (++entry_index >= (sizeof(mDataHistory) / sizeof(*mDataHistory)))
      {
        entry_index = 0;
      }
    } while (entry_index != mDataHistoryIndex);

    unsigned int best_index = (mDataHistoryIndex != 0) ? (mDataHistoryIndex - 1) : ((sizeof(mDataHistory) / sizeof(*mDataHistory)) - 1); // Latest entry

    if (pumpErrorIndex >= 0 && pumpErrorIndex < NO_OF_PUMPS && highest_currents[pumpErrorIndex] != 0.0)
    {
      best_index = highest_current_entry_indices[pumpErrorIndex];
    }
    else
    {
      for (unsigned int pump_index = 0; pump_index < NO_OF_PUMPS; ++pump_index)
      {
        if (highest_currents[pump_index] != 0.0)
        {
          best_index = highest_current_entry_indices[pump_index];
        }
      }
    }

    retval = mDataHistory[best_index];
  }

  OS_Unuse(&mSemaDataHistory);

  return retval;
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
