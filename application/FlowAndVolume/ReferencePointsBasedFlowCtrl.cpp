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
/* CLASS NAME       : ReferencePointsBasedFlowCtrl                          */
/*                                                                          */
/* FILE NAME        : ReferencePointsBasedFlowCtrl.cpp                      */
/*                                                                          */
/* CREATED DATE     : 07-11-2011 dd-mm-yyyy                                 */
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
#include <ReferencePointsBasedFlowCtrl.h>

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
ReferencePointsBasedFlowCtrl::ReferencePointsBasedFlowCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ReferencePointsBasedFlowCtrl::~ReferencePointsBasedFlowCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::InitSubTask()
{
  mOverflowLevelOffset = 0.0f;
  mNumberOfReferencePoints = 0;
  
  for (int i = 0; i < NO_OF_REF_POINTS; i++)
  {
    mSortedLevels[i] = 0.0f;
    mFlowsOfLevels[i] = 0.0f;
  }

  mPreviousSwitchState = DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED;

  mConfigurationChanged = true;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::RunSubTask()
{
  if (mConfigurationChanged)
  {
    mConfigurationChanged = false;
    HandleConfigurationChange();
  }

  HandleReferencePointsBasedFlow();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::ConnectToSubjects()
{
  for (int i = 0; i < NO_OF_REF_POINTS; i++)
  {
    mRefPointsEnabled[i].Subscribe(this);
    mRefPointsLevel[i].Subscribe(this);
    mRefPointsFlow[i].Subscribe(this);
  }

}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::Update(Subject* pSubject)
{
  mConfigurationChanged = true;
  
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::SubscribtionCancelled(Subject* pSubject)
{
  //Not implemented / not used in this application.
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_RPBFC_REFERENCE_POINT_ENABLED_1  : mRefPointsEnabled[0].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_2  : mRefPointsEnabled[1].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_3  : mRefPointsEnabled[2].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_4  : mRefPointsEnabled[3].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_5  : mRefPointsEnabled[4].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_6  : mRefPointsEnabled[5].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_7  : mRefPointsEnabled[6].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_8  : mRefPointsEnabled[7].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_9  : mRefPointsEnabled[8].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_ENABLED_10 : mRefPointsEnabled[9].Attach(pSubject); break;

    case SP_RPBFC_REFERENCE_POINT_LEVEL_1  : mRefPointsLevel[0].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_2  : mRefPointsLevel[1].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_3  : mRefPointsLevel[2].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_4  : mRefPointsLevel[3].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_5  : mRefPointsLevel[4].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_6  : mRefPointsLevel[5].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_7  : mRefPointsLevel[6].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_8  : mRefPointsLevel[7].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_9  : mRefPointsLevel[8].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_LEVEL_10 : mRefPointsLevel[9].Attach(pSubject); break;

    case SP_RPBFC_REFERENCE_POINT_FLOW_1  : mRefPointsFlow[0].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_2  : mRefPointsFlow[1].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_3  : mRefPointsFlow[2].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_4  : mRefPointsFlow[3].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_5  : mRefPointsFlow[4].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_6  : mRefPointsFlow[5].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_7  : mRefPointsFlow[6].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_8  : mRefPointsFlow[7].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_9  : mRefPointsFlow[8].Attach(pSubject); break;
    case SP_RPBFC_REFERENCE_POINT_FLOW_10 : mRefPointsFlow[9].Attach(pSubject); break;
      
    case SP_RPBFC_CONTROL_TYPE : 
      mControlType.Attach(pSubject); 
      break;

    // Variable inputs:
    case SP_RPBFC_DI_OVERFLOW_FUNC_STATE:
      mDiOverflowFuncState.Attach(pSubject);
      break;
    case SP_RPBFC_SURFACE_LEVEL:
      mSurfaceLevel.Attach(pSubject);
      break;

    // Outputs:
    case SP_RPBFC_REFERENCE_POINTS_BASED_FLOW:
      mReferencePointsBasedFlow.Attach(pSubject);
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
 * Function - HandleConfigurationChange
 * DESCRIPTION: 
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::HandleConfigurationChange()
{
  //clear arrays
  for (int i = 0; i < NO_OF_REF_POINTS; i++)
  {
    mSortedLevels[i] = 0.0f;
    mFlowsOfLevels[i] = 0.0f;
  }
  mNumberOfReferencePoints = 0;

  // read values of enabled reference points
  int j = 0;
  for (int i = 0; i < NO_OF_REF_POINTS; i++)
  {
    if (mRefPointsEnabled[i]->GetValue())
    {
      mSortedLevels[j] = mRefPointsLevel[i]->GetValue();
      mFlowsOfLevels[j] = mRefPointsFlow[i]->GetValue();
      j++;
    }
  }
  mNumberOfReferencePoints = j;

  bool sorting_completed = false;
  
  // sort the reference points by level
  // use simple bubble sort because number of elements is low
  while (!sorting_completed)
  {
    sorting_completed = true;

    for (int i = 0; i < mNumberOfReferencePoints - 1; i++)
    {
      if (mSortedLevels[i] > mSortedLevels[i + 1])
      {
        float level = mSortedLevels[i];
        float flow = mFlowsOfLevels[i];

        mSortedLevels[i] = mSortedLevels[i + 1];
        mFlowsOfLevels[i] = mFlowsOfLevels[i + 1];
        
        mSortedLevels[i + 1] = level;
        mFlowsOfLevels[i + 1] = flow;

        sorting_completed = false;
      }
    }
  }
  
}

/*****************************************************************************
 * Function - HandleReferencePointsBasedFlow
 * DESCRIPTION: 
 *
 *****************************************************************************/
void ReferencePointsBasedFlowCtrl::HandleReferencePointsBasedFlow()
{
  bool overflow_switch_installed = mDiOverflowFuncState->GetValue() != DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED;
  bool is_level_controlled = mControlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES;
  bool level_available = mSurfaceLevel->IsAvailable();

  if (overflow_switch_installed && is_level_controlled)
  {
    if (level_available)
    {
      DIGITAL_INPUT_FUNC_STATE_TYPE switch_state = mDiOverflowFuncState->GetValue();
      float surface_level = mSurfaceLevel->GetValue();
      
      bool is_overflow_active = switch_state == DIGITAL_INPUT_FUNC_STATE_ACTIVE;

      if (switch_state != mPreviousSwitchState)
      {
        mPreviousSwitchState = switch_state;

        if (is_overflow_active)
        {
          mOverflowLevelOffset = surface_level;
        }
      }

      float level_in_overflow_channel = 0.0f;

      if (is_overflow_active && surface_level > mOverflowLevelOffset)
      {
        level_in_overflow_channel = surface_level - mOverflowLevelOffset;
      }

      float flow_in_overflow_channel = GetFlowAtLevel(level_in_overflow_channel);

      mReferencePointsBasedFlow->SetValue(flow_in_overflow_channel);
    }
    else
    {
      mReferencePointsBasedFlow->SetQuality(DP_NOT_AVAILABLE);
    }
  }
  else
  {
    mReferencePointsBasedFlow->SetQuality(DP_NEVER_AVAILABLE);
  }
}

/*****************************************************************************
 * Function - GetFlowAtLevel
 * DESCRIPTION: Calculate flow from array of reference points sorted with ascending levels
 *
 *****************************************************************************/
float ReferencePointsBasedFlowCtrl::GetFlowAtLevel(float level)
{
  float flow = 0.0f;

  bool flow_found = false;

  if (mNumberOfReferencePoints == 1)
  {
    flow =  mFlowsOfLevels[0];
  }
  else if (mNumberOfReferencePoints > 1)
  {
    // find the two adherent reference points to use (or single point on exact match)
    for (int i = 0; i < mNumberOfReferencePoints - 1; i++)
    {
      if (level == mSortedLevels[i])
      {
        flow = mFlowsOfLevels[i];
        flow_found = true;
        break;
      }

      // liniear coeff between current and next point
      float coeff = (mFlowsOfLevels[i + 1] - mFlowsOfLevels[i]) / (mSortedLevels[i + 1] - mSortedLevels[i]); 

      if (level < mSortedLevels[i])
      {
        if (i == 0)
        {
          coeff = (mFlowsOfLevels[i] - 0) / (mSortedLevels[i] - 0);
          flow =  mFlowsOfLevels[i] - (mSortedLevels[i] - level) * coeff;
          flow_found = true;
          break;
        }
        else
        {
          flow =  mFlowsOfLevels[i] - (mSortedLevels[i] - level) * coeff;
          flow_found = true;
          break;
        }
      }
      else if (mSortedLevels[i] < level && level < mSortedLevels[i + 1])
      {
        flow = mFlowsOfLevels[i] + (level - mSortedLevels[i]) * coeff;
        flow_found = true;
        break;
      }
      //else the level is higher than next point

    }
  }

  int last_point = mNumberOfReferencePoints - 1;

  if (!flow_found && last_point > 0)
  {
    if (level == mSortedLevels[last_point])
    {
      flow = mFlowsOfLevels[last_point];
    }
    else //if level > mSortedLevels[last_point]
    {
      // liniear coeff between last two points
      float coeff = (mFlowsOfLevels[last_point] - mFlowsOfLevels[last_point - 1]) / (mSortedLevels[last_point] - mSortedLevels[last_point - 1]); 

      flow = mFlowsOfLevels[last_point] + (level - mSortedLevels[last_point]) * coeff;
    }
  }

  return flow;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
