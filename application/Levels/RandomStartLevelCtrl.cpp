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
/* CLASS NAME       : RandomStartLevelCtrl                                  */
/*                                                                          */
/* FILE NAME        : RandomStartLevelCtrl.cpp                              */
/*                                                                          */
/* CREATED DATE     : 03-05-2009 dd-mm-yyyy                                 */
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
#include <RandomStartLevelCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

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
RandomStartLevelCtrl::RandomStartLevelCtrl()
{

}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
RandomStartLevelCtrl::~RandomStartLevelCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void RandomStartLevelCtrl::InitSubTask()
{
  mRunRequestedByConfigFlag = true;
  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void RandomStartLevelCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (!mRunRequestedByConfigFlag && mpNumberOfRunningPumps->GetValue() > 0)
  {
    // don't calculate next random start level. Some pump(s) are still running
    return;
  }

  mRunRequestedByConfigFlag = false;

  float lowest_start_level = mpLowestStartLevel->GetValue();

  if (mpVariationEnabled->GetValue() == true && mSensorType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)
  {
    bool pit_with_single_pump = (mpNoOfPumps->GetValue() == 1);
    float upper_measurement_level = mpUpperMeasurementLevel->GetValue();
    float second_lowest_start_level = mpSecondLowestStartLevel->GetValue();
    float high_level = mpHighLevel->GetAlarmLimit()->GetAsFloat();
    float variation = mpVariation->GetValue();

    // begin calculation of maximum value for random level
    float max_level_with_variation = lowest_start_level + variation;

    // random level must be above upper_measurement_level
    if (lowest_start_level < upper_measurement_level)
    {
      max_level_with_variation = lowest_start_level;
    }

    // random level must be below second_lowest_start_level
    if (!pit_with_single_pump && max_level_with_variation > second_lowest_start_level)
    {
      max_level_with_variation = second_lowest_start_level;
    }

    // random level must be below high_level
    if (mpHighLevel->GetAlarmEnabled() && max_level_with_variation > high_level)
    {
      max_level_with_variation = high_level;
    }

    // random level must be above (or equal to) lowest_start_level
    if (max_level_with_variation < lowest_start_level)
    {
      max_level_with_variation = lowest_start_level;
    }

    // calculate a random level between lowest_start_level and max_level_with_variation
    float level_with_variation = lowest_start_level;

    if (max_level_with_variation > lowest_start_level && mpAdvFlowLearningInProgress->GetValue() == false)
    {
      variation = max_level_with_variation - lowest_start_level;
      // note: rand has already been seeded in main.cpp
      level_with_variation = lowest_start_level + ((variation * rand()) / RAND_MAX);
    }

    mpStartLevelWithVariation->SetValue(level_with_variation);
    mpMaxLevelWithVariation->SetValue(max_level_with_variation);
  }
  else
  {
    mpStartLevelWithVariation->SetValue(lowest_start_level);
    mpMaxLevelWithVariation->SetQuality(DP_NOT_AVAILABLE);
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void RandomStartLevelCtrl::ConnectToSubjects()
{
  mSensorType.Subscribe(this);
  mpVariationEnabled.Subscribe(this);
  mpAdvFlowLearningInProgress.Subscribe(this);
  mpNoOfPumps.Subscribe(this);
  mpNumberOfRunningPumps.Subscribe(this);
  mpUpperMeasurementLevel.Subscribe(this);
  mpHighLevel.Subscribe(this);
  mpLowestStartLevel.Subscribe(this);
  mpSecondLowestStartLevel.Subscribe(this);
  mpVariation.Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void RandomStartLevelCtrl::Update(Subject* pSubject)
{
  if (!mpNumberOfRunningPumps.Update(pSubject))
  {
    mRunRequestedByConfigFlag = true;
  }

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
void RandomStartLevelCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void RandomStartLevelCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
  case SP_RSLC_SENSOR_TYPE:
    mSensorType.Attach(pSubject);
    break;
  case SP_RSLC_VARIATION_ENABLED:
    mpVariationEnabled.Attach(pSubject);
    break;
  case SP_RSLC_ADV_FLOW_LEARNING_IN_PROGRESS:
    mpAdvFlowLearningInProgress.Attach(pSubject);
    break;
  case SP_RSLC_NO_OF_PUMPS:
    mpNoOfPumps.Attach(pSubject);
    break;
  case SP_RSLC_NO_OF_RUNNING_PUMPS:
    mpNumberOfRunningPumps.Attach(pSubject);
    break;
  case SP_RSLC_UPPER_MEASUREMENT_LEVEL:
    mpUpperMeasurementLevel.Attach(pSubject);
    break;
  case SP_RSLC_HIGH_LEVEL:
    mpHighLevel.Attach(pSubject);
    break;
  case SP_RSLC_LOWEST_START_LEVEL:
    mpLowestStartLevel.Attach(pSubject);
    break;
  case SP_RSLC_SECOND_LOWEST_START_LEVEL:
    mpSecondLowestStartLevel.Attach(pSubject);
    break;
  case SP_RSLC_VARIATION:
    mpVariation.Attach(pSubject);
    break;
  case SP_RSLC_LOWEST_START_LEVEL_WITH_VARIATION:
    mpStartLevelWithVariation.Attach(pSubject);
    break;
  case SP_RSLC_MAX_LEVEL_WITH_VARIATION:
    mpMaxLevelWithVariation.Attach(pSubject);
    break;

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
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
