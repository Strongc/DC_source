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
/* CLASS NAME       : VfdFlushStateCtrl                                     */
/*                                                                          */
/* FILE NAME        : VfdFlushStateCtrl.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 04-05-2009 dd-mm-yyyy                                 */
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
#include <VfdFlushStateCtrl.h>

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
VfdFlushStateCtrl::VfdFlushStateCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
VfdFlushStateCtrl::~VfdFlushStateCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdFlushStateCtrl::InitSubTask()
{
  mMsSinceUpdateState = 0;
  mTimeInSameState = 0;
  mTimeSinceReverseFlush = 0;
  mTimeInAutoOperation = 0;
  mpVfdFlushRequest->SetValue(VFD_OPERATION_MODE_STOP);
  mOldAntiBlockingState = ANTI_BLOCKING_REQUEST_IDLE;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdFlushStateCtrl::RunSubTask()
{
  auto VFD_OPERATION_MODE_TYPE  prev_state = mpVfdFlushRequest->GetValue();
  auto VFD_OPERATION_MODE_TYPE  next_state = prev_state;
  auto bool update_flush_state = false;

  // Check trigger from vfd master
  if (mpVfdAutoOprRequest.IsUpdated())
  {
    // Vfd start/stop or manual request. Update state.
    update_flush_state = true;
    if (mpVfdAutoOprRequest->GetQuality() == DP_NOT_AVAILABLE)
    {
      next_state = VFD_OPERATION_MODE_NOT_IN_CONTROL;
    }
    else if (mpVfdAutoOprRequest->GetValue() == true)
    {
      if (mTimeInAutoOperation <= 2)
      {
        // Avoid reverse+start flush just after manual start/stop, anti seizing, sensor error etc.
        next_state = VFD_OPERATION_MODE_NORMAL;
      }
      else
      {
        next_state = VFD_OPERATION_MODE_REV_FLUSH;
      }
    }
    else
    {
      if (mTimeInAutoOperation <= 2)
      {
        // Avoid stop flush just after manual start/stop, anti seizing, sensor error etc.
        next_state = VFD_OPERATION_MODE_STOP;
      }
      else
      {
        next_state = VFD_OPERATION_MODE_STOP_FLUSH;
      }
    }
  }

  // Update state/time update every second
  mMsSinceUpdateState += 10;
  if (mMsSinceUpdateState >= 1000)
  {
    update_flush_state = true;
    mTimeSinceReverseFlush++;
    mTimeInAutoOperation++;
  }

  if (update_flush_state == true)
  {
    VFD_OPERATION_MODE_TYPE vfd_anti_blocking_request = CheckAntiBlockingRequest();
    if (vfd_anti_blocking_request != VFD_OPERATION_MODE_NOT_IN_CONTROL)
    {
      // Special anti blocking situation. Use this request instead of the normal vfd flushes.
      next_state = vfd_anti_blocking_request;
      update_flush_state = false;
    }
    mMsSinceUpdateState = 0;
  }

  // Handle/check change of state
  while (update_flush_state == true)
  {
    if (next_state != prev_state)
    {
      mTimeInSameState = 0;
      prev_state = next_state;
    }
    else
    {
      mTimeInSameState++;
    }

    switch (next_state)
    {
      case VFD_OPERATION_MODE_REV_FLUSH:
        if ( (mpVfdReverseFlushEnabled->GetValue() == false)
          || (mTimeSinceReverseFlush < mpVfdReverseFlushInterval->GetValue()) )
        {
          // No reverse flush
          next_state = VFD_OPERATION_MODE_START_FLUSH;
        }
        else
        {
          if (mTimeInSameState >= mpVfdReverseFlushTime->GetValue())
          {
            // Reverse flush done
            mTimeSinceReverseFlush = 0;
            next_state = VFD_OPERATION_MODE_START_FLUSH;
          }
        }
        break;

      case VFD_OPERATION_MODE_START_FLUSH:
        if ( (mpVfdStartFlushEnabled->GetValue() == false)
          || (mTimeInSameState >= mpVfdStartFlushTime->GetValue()) )
        {
          // Start flush done
          next_state = VFD_OPERATION_MODE_NORMAL;
        }
        break;

      case VFD_OPERATION_MODE_NORMAL:
        if ( (mpVfdRunFlushEnabled->GetValue() == true)
          && (mTimeInSameState+mpVfdRunFlushTime->GetValue() >= mpVfdRunFlushInterval->GetValue()) )
        {
          // Run flush interval elapsed
          next_state = VFD_OPERATION_MODE_RUN_FLUSH;
        }
        break;

      case VFD_OPERATION_MODE_RUN_FLUSH:
        if ( (mpVfdRunFlushEnabled->GetValue() == false)
          || (mTimeInSameState >= mpVfdRunFlushTime->GetValue()) )
        {
          // Run flush done
          next_state = VFD_OPERATION_MODE_NORMAL;
        }
        break;

      case VFD_OPERATION_MODE_STOP_FLUSH:
        if ( (mpVfdStopFlushEnabled->GetValue() == false)
          || (mTimeInSameState >= mpVfdStopFlushTime->GetValue())
          || (mpUnderDryRunningLevel->GetValue() == true) )
        {
          // Stop flush done
          next_state = VFD_OPERATION_MODE_STOP;
        }
        break;

      case VFD_OPERATION_MODE_STOP:
        break;

      case VFD_OPERATION_MODE_NOT_IN_CONTROL:
        mTimeInAutoOperation = 0;
        break;

      default:
        next_state = VFD_OPERATION_MODE_NOT_IN_CONTROL;
        break;
    }

    if (prev_state == next_state)
    {
      update_flush_state = false;
    }
  }

  mpVfdFlushRequest->SetValue(next_state);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void VfdFlushStateCtrl::ConnectToSubjects()
{
  mpVfdAutoOprRequest->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class.
 *
 *****************************************************************************/
void VfdFlushStateCtrl::Update(Subject* pSubject)
{
  mpVfdAutoOprRequest.Update(pSubject);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void VfdFlushStateCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void VfdFlushStateCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Configuration inputs:
    case SP_VFF_VFD_REVERSE_FLUSH_ENABLED:
      mpVfdReverseFlushEnabled.Attach(pSubject);
      break;
    case SP_VFF_VFD_REVERSE_FLUSH_TIME:
      mpVfdReverseFlushTime.Attach(pSubject);
      break;
    case SP_VFF_VFD_REVERSE_FLUSH_INTERVAL:
      mpVfdReverseFlushInterval.Attach(pSubject);
      break;
    case SP_VFF_VFD_START_FLUSH_ENABLED:
      mpVfdStartFlushEnabled.Attach(pSubject);
      break;
    case SP_VFF_VFD_START_FLUSH_TIME:
      mpVfdStartFlushTime.Attach(pSubject);
      break;
    case SP_VFF_VFD_RUN_FLUSH_ENABLED:
      mpVfdRunFlushEnabled.Attach(pSubject);
      break;
    case SP_VFF_VFD_RUN_FLUSH_TIME:
      mpVfdRunFlushTime.Attach(pSubject);
      break;
    case SP_VFF_VFD_RUN_FLUSH_INTERVAL:
      mpVfdRunFlushInterval.Attach(pSubject);
      break;
    case SP_VFF_VFD_STOP_FLUSH_ENABLED:
      mpVfdStopFlushEnabled.Attach(pSubject);
      break;
    case SP_VFF_VFD_STOP_FLUSH_TIME:
      mpVfdStopFlushTime.Attach(pSubject);
      break;

    // Variable inputs:
    case SP_VFF_VFD_AUTO_OPR_REQUEST:
      mpVfdAutoOprRequest.Attach(pSubject);
      break;
    case SP_VFF_UNDER_DRY_RUNNING_LEVEL:
      mpUnderDryRunningLevel.Attach(pSubject);
      break;
    case SP_VFF_VFD_ANTI_BLOCKING_REQUEST:
      mpAntiBlockingRequest.Attach(pSubject);
      break;

    // Outputs:
    case SP_VFF_VFD_FLUSH_REQUEST:
      mpVfdFlushRequest.Attach(pSubject);
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
 * Function - CheckAntiBlockingRequest
 * DESCRIPTION: Determine a vft flush request related to anti blocking.
 *              Return VFD_OPERATION_MODE_NOT_IN_CONTROL when no anti blocking.
 *
 *****************************************************************************/
VFD_OPERATION_MODE_TYPE VfdFlushStateCtrl::CheckAntiBlockingRequest(void)
{
  VFD_OPERATION_MODE_TYPE vfd_request = VFD_OPERATION_MODE_NOT_IN_CONTROL;
  ANTI_BLOCKING_REQUEST_TYPE anti_blocking_request = mpAntiBlockingRequest->GetValue();

  if (anti_blocking_request != ANTI_BLOCKING_REQUEST_IDLE || mOldAntiBlockingState != anti_blocking_request)
  {
    mOldAntiBlockingState = anti_blocking_request;
    if (mpVfdAutoOprRequest->GetQuality() == DP_AVAILABLE && mpVfdAutoOprRequest->GetValue() == true)
    {
      switch (anti_blocking_request)
      {
        case ANTI_BLOCKING_REQUEST_REVERSE_FLUSH:
          vfd_request = VFD_OPERATION_MODE_REV_FLUSH;
          break;

        case ANTI_BLOCKING_REQUEST_FORWARD_FLUSH:
          // Note: Use a run flush here, since a start flush is a normal antiblocking operation mode
          vfd_request = VFD_OPERATION_MODE_RUN_FLUSH;
          break;

        case ANTI_BLOCKING_REQUEST_AWAIT_REVERSE:
        case ANTI_BLOCKING_REQUEST_AWAIT_FORWARD:
          vfd_request = VFD_OPERATION_MODE_STOP;
          break;

        case ANTI_BLOCKING_REQUEST_IDLE:
        default:
          vfd_request = VFD_OPERATION_MODE_NORMAL;
          break;
      }
    }
  }

  return vfd_request;
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
