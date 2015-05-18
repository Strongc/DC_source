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
/* CLASS NAME       : ServiceTimeAlarmCtrl                                  */
/*                                                                          */
/* FILE NAME        : ServiceTimeAlarmCtrl.cpp                              */
/*                                                                          */
/* CREATED DATE     : 22-08-2007 dd-mm-yyyy                                 */
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
#include <ServiceTimeAlarmCtrl.h>

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
ServiceTimeAlarmCtrl::ServiceTimeAlarmCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    mpServiceTimeAlarmDelay[fault_id] = new AlarmDelay(this);
    mServiceTimeAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ServiceTimeAlarmCtrl::~ServiceTimeAlarmCtrl()
{
  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    delete(mpServiceTimeAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ServiceTimeAlarmCtrl::InitSubTask()
{
  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    mpServiceTimeAlarmDelay[fault_id]->InitAlarmDelay();
  }

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ServiceTimeAlarmCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  AlarmConfig* alarm_config = mpServiceTimeAlarmObj[STA_FAULT_OBJ_SERVICE_TIME]->GetAlarmConfig();
  U32 time_since_service = mpRunTimeSinceService->GetValue();
  U32 time_for_service;

  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    if (mServiceTimeAlarmDelayCheckFlag[fault_id] == true)
    {
      mServiceTimeAlarmDelayCheckFlag[fault_id] = false;
      mpServiceTimeAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }


  /*****************************************************************************
   * Check for crossing the warnig/alarm limit
   ****************************************************************************/
  if (time_since_service >= alarm_config->GetWarningLimit()->GetAsInt())
  {
    // Warning limit is reached - set warning
    mpServiceTimeAlarmDelay[STA_FAULT_OBJ_SERVICE_TIME]->SetWarning();
  }
  else
  {
    mpServiceTimeAlarmDelay[STA_FAULT_OBJ_SERVICE_TIME]->ResetWarning();
  }
  if (time_since_service >= alarm_config->GetAlarmLimit()->GetAsInt())
  {
    // Alarm limit is reached - set alarm
    mpServiceTimeAlarmDelay[STA_FAULT_OBJ_SERVICE_TIME]->SetFault();
  }
  else
  {
    mpServiceTimeAlarmDelay[STA_FAULT_OBJ_SERVICE_TIME]->ResetFault();
  }


  // Determine time to service
  time_for_service = alarm_config->GetAlarmLimit()->GetAsInt();
  if (alarm_config->GetAlarmEnabled() == false && alarm_config->GetWarningEnabled() == true)
  {
    // Special: Use warning limit if only warning is enabled
    time_for_service = alarm_config->GetWarningLimit()->GetAsInt();
  }
  if (time_since_service < time_for_service)
  {
    mpRunTimeToService->SetValue(time_for_service-time_since_service);
  }
  else
  {
    mpRunTimeToService->SetValue(0);
  }


  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    mpServiceTimeAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ServiceTimeAlarmCtrl::ConnectToSubjects()
{
  mpRunTimeSinceService->Subscribe(this);

  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    mpServiceTimeAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void ServiceTimeAlarmCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpServiceTimeAlarmDelay[STA_FAULT_OBJ_SERVICE_TIME])
  {
    mServiceTimeAlarmDelayCheckFlag[STA_FAULT_OBJ_SERVICE_TIME] = true;
  }

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
void ServiceTimeAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_STA_FAULT_OBJ; fault_id < NO_OF_STA_FAULT_OBJ; fault_id++)
  {
    mpServiceTimeAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ServiceTimeAlarmCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_STAC_RUN_TIME_SINCE_SERVICE:
      mpRunTimeSinceService.Attach(pSubject);
      break;
    case SP_STAC_RUN_TIME_TO_SERVICE:
      mpRunTimeToService.Attach(pSubject);
      break;
    case SP_STAC_SERVICE_TIME_ALARM_OBJ:
      mpServiceTimeAlarmObj[STA_FAULT_OBJ_SERVICE_TIME].Attach(pSubject);
      mpServiceTimeAlarmDelay[STA_FAULT_OBJ_SERVICE_TIME]->SetSubjectPointer(Id, pSubject);
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
