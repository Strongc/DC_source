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
/* CLASS NAME       : AlarmRelay                                            */
/*                                                                          */
/* FILE NAME        : AlarmRelay.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 16-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SwTimer.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmRelay.h>
#include <AlarmLog.h>                   // For ALARM_LOG_SIZE

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
 * DESCRIPTION: Initialises private data and reads configuration.
 *
 *****************************************************************************/
AlarmRelay::AlarmRelay(AlarmControl* pAlarmControl, ALARM_RELAY_TYPE id)
{
  mpAlarmControl = pAlarmControl;
  mAlarmRelayID = id;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: Unsubscirbes the observed AlarmDataPoint's
 *
 ****************************************************************************/
AlarmRelay::~AlarmRelay()
{
}


/*****************************************************************************
 * Function - InitAlarmList
 * DESCRIPTION: Initialize alarm vector - called from AlarmControl InitSubTask
 *
 *****************************************************************************/
void AlarmRelay::InitAlarmList()
{
  // Assure a check of alarm vector at startup
  mCheckAlarmListFlag = true;
  mpAlarmControl->ReqCheckAlarmList();

  // Set alarm vector to a minimum size that assures reallocation of memory is
  // not needed. The size of the alarm log is used as this is the limit for how
  // many alarms the system can contain at a given time
  mAlarmVector.reserve(ALARM_LOG_SIZE);
}

/*****************************************************************************
 * Function - CheckAlarmList
 * DESCRIPTION: Checks the alarm vector to see, if any of the elements
 *              should be removed due to a reset or reconfiguration of the
 *              alarm
 *
 *****************************************************************************/
void AlarmRelay::CheckAlarmList()
{
  std::vector<AlarmDataPoint*>::iterator iter;

  bool remove_allowed = false;

  mCheckAlarmListFlag = false;

  // Search vector to find alarms, that are not present anymore.
  // Take care not to disturb the iter pointing into the vector when
  // removing elements.
  iter = mAlarmVector.begin();
  while (iter != mAlarmVector.end())
  {
    if (mpAlarmRelayAck->GetValue() == true)
    {
      // Check if alarm can be removed.
      switch (mAlarmRelayID)
      {
        case ALARM_RELAY_ALL_ALARMS:
          remove_allowed = RemoveWhenNoAlarm(*iter);
          break;
        case ALARM_RELAY_HIGH_LEVEL:
        case ALARM_RELAY_CRITICAL:
        case ALARM_RELAY_ALL_ALARMS_AND_WARNINGS:
          remove_allowed = RemoveWhenReady(*iter);
          break;
        case ALARM_RELAY_CUSTOM:
          remove_allowed = RemoveCustom(*iter);
          break;
        default:
          break;
      }
      if (remove_allowed == true)
      {
        iter = mAlarmVector.erase(iter); // erase returns an iter to the element after the erased element
      }
    }

    // If an erase has been done the iter must not be incremented - otherwise it is
    // moved forward to the next element.
    if ( remove_allowed != true)
    {
      iter++;
    }
  }

  // Check the presence of manual acknowledged alarm relay.
  if (mpAlarmRelayAck->GetValue() == true)
  {
    // If relay ack is auto, the there is no way a manual ack can be present
    mpManualAckAlarmPresent->SetValue(false);
  }
  else
  {
    // If relay ack is manual, a manual ack alarm is present if there is any alarms in the alarm vector.
    if (mAlarmVector.empty() == false)
    {
      mpManualAckAlarmPresent->SetValue(true);
    }
  }

  if (mpAlarmRelayResetEvent.IsUpdated())
  {
    mAlarmVector.clear();
    mpManualAckAlarmPresent->SetValue(false);
  }

  if (mAlarmVector.empty() == true && mpManualAckAlarmPresent->GetValue() == false)
  {
    mpAlarmRelayOutputValue->SetValue(false);
  }
  else
  {
    mpAlarmRelayOutputValue->SetValue(true);
  }
}

/*****************************************************************************
 * Function - AddAlarm
 * DESCRIPTION: Adds the specified alarm to the vector - if it is not already there
 *
 *****************************************************************************/
void AlarmRelay::AddAlarm(AlarmDataPoint* pAlarm)
{
  std::vector<AlarmDataPoint*>::iterator iter;
  bool alarm_found;

  alarm_found = false;
  for (iter = mAlarmVector.begin(); iter != mAlarmVector.end(); iter++)
  {
    if (*iter == pAlarm)
    {
      alarm_found = true;
    }
  }

  if (alarm_found == false)
  {
    // Alarm was not in the list - add it
    mAlarmVector.push_back(pAlarm);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects. Only subscribes to error sources, because
 * updates from the other subjects are not wanted.
 *
 *****************************************************************************/
void AlarmRelay::ConnectToSubjects()
{
    mpAlarmRelayResetEvent->Subscribe(this);
    mpAlarmRelayAck->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Request the AlarmControl for a check of the alarm vector as an
 *              observed datapoint has changed
 *
 *****************************************************************************/
void AlarmRelay::Update(Subject* pSubject)
{
  mpAlarmRelayResetEvent.Update(pSubject);
  mpAlarmRelayAck.Update(pSubject);

  if (mCheckAlarmListFlag == false)
  {
    mCheckAlarmListFlag = true;
    mpAlarmControl->ReqCheckAlarmList();
  }
}

/*****************************************************************************
* Function - SubscribtionCancelled
* DESCRIPTION: If pSubject is a pointer of AlarmRelay then set to 0
*
*****************************************************************************/
void AlarmRelay::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed by AlarmRelay.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AlarmRelay::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    case ALARM_RELAY_CONNECT_OUTPUT_VALUE:
      mpAlarmRelayOutputValue.Attach(pSubject);
      break;
    case ALARM_RELAY_CONNECT_ACK:
      mpAlarmRelayAck.Attach(pSubject);
      break;
    case ALARM_RELAY_CONNECT_RESET_EVENT:
      mpAlarmRelayResetEvent.Attach(pSubject);
      break;
    case ALARM_RELAY_MANUAL_ACK_PRESENT:
      mpManualAckAlarmPresent.Attach(pSubject);
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
 * Function - RemoveWhenNoAlarm
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmRelay::RemoveWhenNoAlarm(AlarmDataPoint* element)
{
  bool remove_allowed = false;

  if (element->GetAlarmPresent() != ALARM_STATE_ALARM)
  {
    remove_allowed = true;
  }
  return remove_allowed;
}

/*****************************************************************************
 * Function - RemoveWhenReady
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmRelay::RemoveWhenReady(AlarmDataPoint* element)
{
  bool remove_allowed = false;

  if (element->GetAlarmPresent() == ALARM_STATE_READY)
  {
    remove_allowed = true;
  }
  return remove_allowed;
}

/*****************************************************************************
 * Function - RemoveCustom
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AlarmRelay::RemoveCustom(AlarmDataPoint* element)
{
  bool remove_allowed = true;

  if (element->GetAlarmConfig()->GetCustomRelayForAlarm() == true)
  {
    if (element->GetAlarmPresent() == ALARM_STATE_ALARM)
    {
      remove_allowed = false;
    }
  }

  if (element->GetAlarmConfig()->GetCustomRelayForWarning() == true)
  {
    if (element->GetAlarmPresent() == ALARM_STATE_WARNING)
    {
      remove_allowed = false;
    }
  }

  return remove_allowed;
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
