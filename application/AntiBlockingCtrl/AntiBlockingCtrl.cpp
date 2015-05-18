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
/* CLASS NAME       : AntiBlockingCtrl                                      */
/*                                                                          */
/* FILE NAME        : AntiBlockingCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 23-09-2009 dd-mm-yyyy                                 */
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
#include <AntiBlockingCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// Trigger source
typedef enum
{
  FIRST_TRIGGER_SOURCE = 0,
  TRIGGER_SOURCE_CU361 = FIRST_TRIGGER_SOURCE,
  TRIGGER_SOURCE_CUE_ALARM,
  TRIGGER_SOURCE_CUE_WARNING,
  TRIGGER_SOURCE_MP204_ALARM,
  TRIGGER_SOURCE_MP204_WARNING,
  TRIGGER_SOURCE_IO111_ALARM,
  TRIGGER_SOURCE_IO111_WARNING,
  // insert above here
  NO_OF_TRIGGER_SOURCE,
  LAST_TRIGGER_SOURCE = NO_OF_TRIGGER_SOURCE - 1
} TRIGGER_SOURCE_TYPE;

// Trigger enabled type
typedef enum
{
  FIRST_TRIGGER_TYPE = 0,
  TRIGGER_TYPE_CURRENT = FIRST_TRIGGER_TYPE,
  TRIGGER_TYPE_TORQUE,
  TRIGGER_TYPE_COSPHI,
  TRIGGER_TYPE_LOWFLOW,
  TRIGGER_TYPE_OVERTEMP,
  // insert above here
  NO_OF_TRIGGER_TYPE,
  LAST_TRIGGER_TYPE = NO_OF_TRIGGER_TYPE - 1
} TRIGGER_TYPE;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef struct
{
  U8            alarm_code;
  TRIGGER_TYPE  triggerType;
} ALARM_CODE_AND_TYPE;
typedef struct
{
  U16                 subjectRelation;
  U16                 triggerSource;
  const ALARM_CODE_AND_TYPE *codeAndTypeList;
} ALARM_SOURCE_CONFIGURATION_TYPE;

const ALARM_CODE_AND_TYPE all_current[] =
{
  { 255 , TRIGGER_TYPE_CURRENT},// 255 means all codes
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE all_cosphi[] =
{
  { 255 , TRIGGER_TYPE_COSPHI},// 255 means all codes
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE all_lowflow[] =
{
  { 255 , TRIGGER_TYPE_LOWFLOW},// 255 means all codes
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE all_overtemp[] =
{
  { 255 , TRIGGER_TYPE_OVERTEMP},// 255 means all codes
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE cue_alarms[] =
{
  { 48  , TRIGGER_TYPE_CURRENT},
  { 64  , TRIGGER_TYPE_OVERTEMP},
  { 70  , TRIGGER_TYPE_OVERTEMP},
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE cue_warnings[] =
{
  { 48  , TRIGGER_TYPE_CURRENT},
  { 64  , TRIGGER_TYPE_OVERTEMP},
  { 70  , TRIGGER_TYPE_OVERTEMP},
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE io111_alarms[] =
{
  { 64  , TRIGGER_TYPE_OVERTEMP},
  { 69  , TRIGGER_TYPE_OVERTEMP},
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE io111_warnings[] =
{
  { 64  , TRIGGER_TYPE_OVERTEMP},
  { 0 } // End of list, must be there
};
const ALARM_CODE_AND_TYPE mp204_alarms[] =
{
  { 48  , TRIGGER_TYPE_CURRENT},
  { 56  , TRIGGER_TYPE_CURRENT},
  { 112 , TRIGGER_TYPE_COSPHI},
  { 113 , TRIGGER_TYPE_COSPHI},
  { 64  , TRIGGER_TYPE_OVERTEMP},
  { 71  , TRIGGER_TYPE_OVERTEMP},
  { 0 } // End of list, must be there
};

const ALARM_SOURCE_CONFIGURATION_TYPE alarm_source_configuration[] =
{
  { SP_ABL_CUE_ALARM                            ,   TRIGGER_SOURCE_CUE_ALARM    ,   cue_alarms      },
  { SP_ABL_CUE_WARNING                          ,   TRIGGER_SOURCE_CUE_WARNING  ,   cue_warnings    },
  { SP_ABL_IO111_ALARM                          ,   TRIGGER_SOURCE_IO111_ALARM  ,   io111_alarms    },
  { SP_ABL_IO111_WARNING                        ,   TRIGGER_SOURCE_IO111_WARNING,   io111_warnings  },
  { SP_ABL_MP204_ALARM                          ,   TRIGGER_SOURCE_MP204_ALARM  ,   mp204_alarms    },
  { SP_ABL_MP204_WARNING_OVERLOAD               ,   TRIGGER_SOURCE_MP204_WARNING,   all_current     },
  { SP_ABL_MP204_WARNING_UNDERLOAD              ,   TRIGGER_SOURCE_MP204_WARNING,   all_current     },
  { SP_ABL_MP204_WARNING_COS_PHI_MAX            ,   TRIGGER_SOURCE_MP204_WARNING,   all_cosphi      },
  { SP_ABL_MP204_WARNING_COS_PHI_MIN            ,   TRIGGER_SOURCE_MP204_WARNING,   all_cosphi      },
  { SP_ABL_MP204_WARNING_OVERTEMPERATURE_PT100  ,   TRIGGER_SOURCE_MP204_WARNING,   all_overtemp    },
  { SP_ABL_MP204_WARNING_OVERTEMPERATURE_TEMPCON,   TRIGGER_SOURCE_MP204_WARNING,   all_overtemp    },
  { SP_ABL_PUMP_ALARM_LOW_FLOW                  ,   TRIGGER_SOURCE_CU361        ,   all_lowflow     },
  { SP_ABL_PUMP_ALARM_MOTOR_CURRENT_OVERLOAD    ,   TRIGGER_SOURCE_CU361        ,   all_current     },
  { SP_ABL_PUMP_ALARM_MOTOR_CURRENT_UNDERLOAD   ,   TRIGGER_SOURCE_CU361        ,   all_current     },
  { SP_ABL_PUMP_ALARM_OVERTEMP_PTC_IO351        ,   TRIGGER_SOURCE_CU361        ,   all_overtemp    }
};
#define ALARM_SOURCE_TABLE_SIZE (sizeof(alarm_source_configuration)/sizeof(alarm_source_configuration[0]))


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
AntiBlockingCtrl::AntiBlockingCtrl()
{
  mpBlockedAlarmDelay = new AlarmDelay(this);
  mBlockedAlarmDelayCheckFlag = false;

  // Allocate a list for alarms that indicates a blocked situation
  mpBlockedTriggerEnabledList = new SubjectPtr<BoolDataPoint*>[NO_OF_TRIGGER_TYPE];
  mpBlockedAlarmSourceList = new SubjectPtr<AlarmDataPoint*>[ALARM_SOURCE_TABLE_SIZE];
  mPendingAlarmAckList = new bool[ALARM_SOURCE_TABLE_SIZE];
  for (int index=0; index<ALARM_SOURCE_TABLE_SIZE; index++)
  {
    mPendingAlarmAckList[index] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AntiBlockingCtrl::~AntiBlockingCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AntiBlockingCtrl::InitSubTask()
{
  mAntiBlockCountIn24h = 0;
  mAntiBlockCountInRow = 0;
  mTimeSince24HReset = 0;
  mAlarmResetWaitTime = 0;
  mRunAntiBlock = false;
  mTriggerAlarmAckIndex = ALARM_SOURCE_TABLE_SIZE;
  mAntiBlockStateWaitTime = 0;
  mAntiBlockReverseTime = 0;

  mpBlockedAlarmDelay->InitAlarmDelay();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AntiBlockingCtrl::RunSubTask()
{
//**************** NOTE: This sub task must run every second. ****************

  if (mBlockedAlarmDelayCheckFlag == true)
  {
    mBlockedAlarmDelayCheckFlag = false;
    mpBlockedAlarmDelay->CheckErrorTimers();
  }

  if (mpAntiBlockingEnabled->GetValue() == true)
  {
    mpAntiBlockingPerformedCounter->SetQuality(DP_AVAILABLE);
  }
  else
  {
    mpAntiBlockingPerformedCounter->SetQuality(DP_NEVER_AVAILABLE);
  }

  DetectBlocking();
  HandleTriggerAlarmAck();
  UpdateAntiBlockingRequest();

  mpBlockedAlarmDelay->UpdateAlarmDataPoint();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AntiBlockingCtrl::ConnectToSubjects()
{
  mpOperationModeActualPump->Subscribe(this);
  mpBlockedAlarmDelay->ConnectToSubjects();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void AntiBlockingCtrl::Update(Subject* pSubject)
{
  mpOperationModeActualPump.Update(pSubject);

  if (pSubject == mpBlockedAlarmDelay)
  {
    mBlockedAlarmDelayCheckFlag = true;
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AntiBlockingCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpBlockedAlarmDelay->SubscribtionCancelled(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void AntiBlockingCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Outputs:
    case SP_ABL_ANTI_BLOCKING_REQUEST:
      mpAntiBlockingRequest.Attach(pSubject);
      break;
    case SP_ABL_BLOCKED_ALARM_OBJ :
      mpBlockedAlarmObj.Attach(pSubject);
      mpBlockedAlarmDelay->SetSubjectPointer(id, pSubject);
    case SP_ABL_CUE_ALARM_RESET_EVENT:
      mpCUEAlarmResetEvent.Attach(pSubject);
      break;
    case SP_ABL_IO111_ALARM_RESET_EVENT:
      mpIO111AlarmResetEvent.Attach(pSubject);
      break;
    case SP_ABL_MP204_ALARM_RESET_EVENT:
      mpMP204AlarmResetEvent.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_PERFORMED_COUNTER:
      mpAntiBlockingPerformedCounter.Attach(pSubject);
      break;

    // Configuration inputs:
    case SP_ABL_ANTI_BLOCKING_ENABLED:
      mpAntiBlockingEnabled.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_DIRECTION_CHANGE_TIME:
      mpAntiBlockingDirectionChangeTime.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_REVERSE_FLUSH_ENABLED:
      mpAntiBlockingReverseFlushEnabled.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_REVERSE_FLUSH_TIME:
      mpAntiBlockingReverseFlushTime.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_FORWARD_FLUSH_ENABLED:
      mpAntiBlockingForwardFlushEnabled.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_FORWARD_FLUSH_TIME:
      mpAntiBlockingForwardFlushTime.Attach(pSubject);
      break;
    case SP_ABL_ANTI_BLOCKING_MAX_NO_OF_TRIES:
      mpAntiBlockingMaxNoOfTries.Attach(pSubject);
      break;
    case SP_ABL_TRIGGER_TYPE_CURRENT_ENABLED:
      mpBlockedTriggerEnabledList[TRIGGER_TYPE_CURRENT].Attach(pSubject);
      break;
    case SP_ABL_TRIGGER_TYPE_TORQUE_ENABLED:
      mpBlockedTriggerEnabledList[TRIGGER_TYPE_TORQUE].Attach(pSubject);
      break;
    case SP_ABL_TRIGGER_TYPE_COSPHI_ENABLED:
      mpBlockedTriggerEnabledList[TRIGGER_TYPE_COSPHI].Attach(pSubject);
      break;
    case SP_ABL_TRIGGER_TYPE_LOWFLOW_ENABLED:
      mpBlockedTriggerEnabledList[TRIGGER_TYPE_LOWFLOW].Attach(pSubject);
      break;
    case SP_ABL_TRIGGER_TYPE_OVERTEMP_ENABLED:
      mpBlockedTriggerEnabledList[TRIGGER_TYPE_OVERTEMP].Attach(pSubject);
      break;

    // Variable inputs:
    case SP_ABL_OPERATION_MODE_ACTUAL_PUMP:
      mpOperationModeActualPump.Attach(pSubject);
      break;
    case SP_ABL_BLOCKAGE_DETECTED:
      mpBlockageDetected.Attach(pSubject);
      break;

    default:
      // For alarm triggers: Look up the relation in a table and attach the subject
      for (int index=0; index<ALARM_SOURCE_TABLE_SIZE; index++)
      {
        if (id == alarm_source_configuration[index].subjectRelation)
        {
          mpBlockedAlarmSourceList[index].Attach(pSubject);
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
 * Function - DetectBlocking
 * DESCRIPTION: Detect if the pump is blocked by inspecting misc. load alarms.
 *
 *****************************************************************************/
void AntiBlockingCtrl::DetectBlocking(void)
{
  if ( (mpOperationModeActualPump.IsUpdated())
    && (mpOperationModeActualPump->GetValue() == ACTUAL_OPERATION_MODE_STOPPED)
    && (mpAntiBlockingRequest->GetValue() == ANTI_BLOCKING_REQUEST_IDLE) )
  {
    // Check if an antiblock-trigging warning or alarm is present when the pump has just stopped
    int source_index = 0;
    bool blocked = false;
    for (source_index=0; source_index<ALARM_SOURCE_TABLE_SIZE; source_index++)
    {
      if ( (mpBlockedAlarmSourceList[source_index].IsValid() )
        && (mpBlockedAlarmSourceList[source_index]->GetAlarmPresent() != ALARM_STATE_READY) )
      {
        // Possible trigger present. Run throgh the attached list to check alarm code and trigger enabled
        const ALARM_CODE_AND_TYPE *code_and_type = alarm_source_configuration[source_index].codeAndTypeList;
        int check_index = 0;
        U8 check_code = code_and_type[check_index].alarm_code;
        U8 alarm_code = mpBlockedAlarmSourceList[source_index]->GetValue();
        while (check_code > 0)
        {
          if (check_code == alarm_code || check_code == 0xFF)
          {
            if (mpBlockedTriggerEnabledList[code_and_type[check_index].triggerType]->GetValue() == true)
            {
              blocked = true;
              mPendingAlarmAckList[source_index] = true;
            }
          }
          check_index++;
          check_code = code_and_type[check_index].alarm_code;
        }
      }
    }

    if (mpBlockageDetected->GetValue() == true)
    {
      //a blockage was detected by Blockage Detection Ctrl
      blocked = true;
    }

    mRunAntiBlock = false;
    if (blocked == true)
    {
      if ( (mAntiBlockCountIn24h < mpAntiBlockingMaxNoOfTries->GetValue())
        && (mAntiBlockCountInRow < mpAntiBlockingMaxNoOfTries->GetValue()) )
      {
        mAntiBlockCountIn24h++;
        mAntiBlockCountInRow++;
        mRunAntiBlock = true;
      }
      else
      {
        // Too many times blocked, set fault and do not perform anti blocking before we have a better situation
        mpBlockedAlarmDelay->SetFault();
        mAlarmResetWaitTime = 0;
      }
    }
    else
    {
      mAntiBlockCountInRow = 0;
      mpBlockedAlarmDelay->ResetFault();
    }
  } // End handling of stop event

  if (mpBlockedAlarmObj->GetAlarmPresent() == ALARM_STATE_ALARM)
  {
    mAlarmResetWaitTime++;
    // Reset the alarm in case the blocked fault is configured to be an alarm
    // (to avoid keeping the pump stopped unless blocked fault is setup to manual acknowledge)
    if (mAlarmResetWaitTime > 10)
    {
      mpBlockedAlarmDelay->ResetFault();
    }
  }

  // After 24 hours, allow anti blocking again
  if (mAntiBlockCountIn24h > 0)
  {
    mTimeSince24HReset++;
    if (mTimeSince24HReset >= 24*3600)
    {
      mTimeSince24HReset = 0;
      mAntiBlockCountIn24h = 0;
    }
  }
  else
  {
    mTimeSince24HReset = 0;
  }
}

/*****************************************************************************
 * Function - HandleTriggerAlarmAck
 * DESCRIPTION: Force reset of trigger alarms in case that anti blocking
 *              should run.
 *
 *****************************************************************************/
void AntiBlockingCtrl::HandleTriggerAlarmAck(void)
{
  if (mRunAntiBlock == true && mpAntiBlockingEnabled->GetValue() == true)
  {
    // Handle only one alarm at a time to avoid sending too many reset telegrams via the geni bus
    U32 index = mTriggerAlarmAckIndex+1;
    if (index >= ALARM_SOURCE_TABLE_SIZE)
    {
      index = 0;
    }
    mTriggerAlarmAckIndex = index;
    if (mPendingAlarmAckList[index] == true)
    {
      ALARM_STATE_TYPE alarm_present = mpBlockedAlarmSourceList[index]->GetAlarmPresent();
      ALARM_STATE_TYPE error_present = mpBlockedAlarmSourceList[index]->GetErrorPresent();
      if (alarm_present == ALARM_STATE_READY)
      {
        // Everything is ok (or trigger alarm/warning is disabled)
        mPendingAlarmAckList[index] = false;
      }
      // Handling of slave module alarm reset. (Check if the alarm source is still present (using error_present))
      else if (error_present != ALARM_STATE_READY)
      {
        // Reset alarms in the slave
        switch (alarm_source_configuration[index].triggerSource)
        {
          case TRIGGER_SOURCE_CUE_ALARM:
            mpCUEAlarmResetEvent->SetEvent();
            break;
          case TRIGGER_SOURCE_IO111_ALARM:
            mpIO111AlarmResetEvent->SetEvent();
            break;
          case TRIGGER_SOURCE_MP204_ALARM:
            mpMP204AlarmResetEvent->SetEvent();
            break;
          default:
            // Nothing to do for this trigger source
            mPendingAlarmAckList[index] = false;
            break;
        }
      }
      else
      {
        // Ready to run again
        mPendingAlarmAckList[index] = false;
      }
    }
  }
}

/*****************************************************************************
 * Function - UpdateAntiBlockingRequest
 * DESCRIPTION: Determine an anti blocking request for use by pump/vfd control.
 *
 *****************************************************************************/
void AntiBlockingCtrl::UpdateAntiBlockingRequest(void)
{
  bool perform_anti_blocking = (mRunAntiBlock == true && mpAntiBlockingEnabled->GetValue() == true);
  ANTI_BLOCKING_REQUEST_TYPE old_anti_blocking_request = mpAntiBlockingRequest->GetValue();
  ANTI_BLOCKING_REQUEST_TYPE new_anti_blocking_request = old_anti_blocking_request;

  mAntiBlockStateWaitTime++;
  do
  {
    if (old_anti_blocking_request != new_anti_blocking_request)
    {
      old_anti_blocking_request = new_anti_blocking_request;
      mAntiBlockStateWaitTime = 0;
    }
    switch (old_anti_blocking_request)
    {
      case ANTI_BLOCKING_REQUEST_IDLE:
        // mAntiBlockStateWaitTime > xx is required to ensure that operation mode is updated before using it
        if (perform_anti_blocking && mAntiBlockStateWaitTime > 1 && mpOperationModeActualPump->GetValue() == ACTUAL_OPERATION_MODE_STOPPED)
        {
          // In case of flush, some seconds in stop should elapse. That's handled by the _AWAIT_XXX request
          if (mpAntiBlockingReverseFlushEnabled->GetValue())
          {
            new_anti_blocking_request = ANTI_BLOCKING_REQUEST_AWAIT_REVERSE;
          }
          else if (mpAntiBlockingForwardFlushEnabled->GetValue())
          {
            new_anti_blocking_request = ANTI_BLOCKING_REQUEST_AWAIT_FORWARD;
          }
        }
        break;

      case ANTI_BLOCKING_REQUEST_AWAIT_REVERSE:
        if (perform_anti_blocking == false || mpAntiBlockingReverseFlushEnabled->GetValue() == false)
        {
          new_anti_blocking_request = ANTI_BLOCKING_REQUEST_IDLE;
        }
        else if (mAntiBlockStateWaitTime >= mpAntiBlockingDirectionChangeTime->GetValue())
        {
          new_anti_blocking_request = ANTI_BLOCKING_REQUEST_REVERSE_FLUSH;
          mAntiBlockReverseTime = 0;
        }
        break;

      case ANTI_BLOCKING_REQUEST_REVERSE_FLUSH:
        if ( (perform_anti_blocking == false)
          || (mpAntiBlockingReverseFlushEnabled->GetValue() == false)
          || (mAntiBlockReverseTime >= mpAntiBlockingReverseFlushTime->GetValue())
          || (mAntiBlockReverseTime > 0 && mpOperationModeActualPump->GetValue() == ACTUAL_OPERATION_MODE_STOPPED) )
        {
          new_anti_blocking_request = ANTI_BLOCKING_REQUEST_AWAIT_FORWARD;
        }
        else if (mpOperationModeActualPump->GetValue() != ACTUAL_OPERATION_MODE_STOPPED)
        {
          mAntiBlockReverseTime++;
        }
        break;

      case ANTI_BLOCKING_REQUEST_AWAIT_FORWARD:
        if (mAntiBlockStateWaitTime >= mpAntiBlockingDirectionChangeTime->GetValue())
        {
          if (perform_anti_blocking == false || mpAntiBlockingForwardFlushEnabled->GetValue() == false)
          {
            new_anti_blocking_request = ANTI_BLOCKING_REQUEST_IDLE;
            mpAntiBlockingPerformedCounter->SetValue(1+mpAntiBlockingPerformedCounter->GetValue());
          }
          else if (mpOperationModeActualPump->GetValue() != ACTUAL_OPERATION_MODE_STOPPED)
          {
            new_anti_blocking_request = ANTI_BLOCKING_REQUEST_FORWARD_FLUSH;
          }
        }
        break;

      case ANTI_BLOCKING_REQUEST_FORWARD_FLUSH:
        if ( (perform_anti_blocking == false)
          || (mpAntiBlockingForwardFlushEnabled->GetValue() == false)
          || (mAntiBlockStateWaitTime >= mpAntiBlockingForwardFlushTime->GetValue())
          || (mpOperationModeActualPump->GetValue() == ACTUAL_OPERATION_MODE_STOPPED) )
        {
          new_anti_blocking_request = ANTI_BLOCKING_REQUEST_IDLE;
          mpAntiBlockingPerformedCounter->SetValue(1+mpAntiBlockingPerformedCounter->GetValue());
        }
        break;

      default:
        new_anti_blocking_request = ANTI_BLOCKING_REQUEST_IDLE;
        break;
    }
  } while (new_anti_blocking_request != old_anti_blocking_request);

  mpAntiBlockingRequest->SetValue(new_anti_blocking_request);
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
