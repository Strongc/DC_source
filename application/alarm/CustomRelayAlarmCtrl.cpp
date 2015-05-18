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
/* CLASS NAME       : CustomRelayAlarmCtrl                                  */
/*                                                                          */
/* FILE NAME        : CustomRelayAlarmCtrl.cpp                              */
/*                                                                          */
/* CREATED DATE     : 15-04-2008 dd-mm-yyyy                                 */
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
#include <CustomRelayAlarmCtrl.h>
#include <RelayFuncHandler.h>
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
CustomRelayAlarmCtrl::CustomRelayAlarmCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    mpCustomRelayAlarmAlarmDelay[fault_id] = new AlarmDelay(this);
    mCustomRelayAlarmAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CustomRelayAlarmCtrl::~CustomRelayAlarmCtrl()
{
  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    delete(mpCustomRelayAlarmAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CustomRelayAlarmCtrl::InitSubTask()
{
  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    mpCustomRelayAlarmAlarmDelay[fault_id]->InitAlarmDelay();
  }

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CustomRelayAlarmCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    if (mCustomRelayAlarmAlarmDelayCheckFlag[fault_id] == true)
    {
      mCustomRelayAlarmAlarmDelayCheckFlag[fault_id] = false;
      mpCustomRelayAlarmAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  if (RelayFuncHandler::GetInstance()->IsFuncConfiged(RELAY_FUNC_RELAY_CUSTOM) == true)
  {
    if (mpRelayCustomStatus->GetValue() == true)
    {
      mpCustomRelayAlarmAlarmDelay[CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE]->SetFault();
    }
    else
    {
      mpCustomRelayAlarmAlarmDelay[CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE]->ResetFault();
    }
  }
  else
  {
    mpCustomRelayAlarmAlarmDelay[CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE]->ResetFault();
  }


  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    mpCustomRelayAlarmAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void CustomRelayAlarmCtrl::ConnectToSubjects()
{
  mpRelayCustomStatus->Subscribe(this);
  mpRelayConfigurationChanged->Subscribe(this);

  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    mpCustomRelayAlarmAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void CustomRelayAlarmCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpCustomRelayAlarmAlarmDelay[CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE])
  {
    mCustomRelayAlarmAlarmDelayCheckFlag[CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE] = true;
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
void CustomRelayAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_CRAC_FAULT_OBJ; fault_id < NO_OF_CRAC_FAULT_OBJ; fault_id++)
  {
    mpCustomRelayAlarmAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void CustomRelayAlarmCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_CRAC_RELAY_CUSTOM_STATUS:
      mpRelayCustomStatus.Attach(pSubject);
      break;
    case SP_CRAC_RELAY_CONFIG_CHANGED:
      mpRelayConfigurationChanged.Attach(pSubject);
      break;

    case SP_CRAC_CUSTOM_RELAY_ACTIVE_FAULT_OBJ :
      mpCustomRelayAlarmAlarmDelay[CRAC_FAULT_OBJ_CUSTOM_RELAY_ACTIVE]->SetSubjectPointer(Id, pSubject);
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
