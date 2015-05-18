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
/* CLASS NAME       : PowerOnWarningCtrl                                    */
/*                                                                          */
/* FILE NAME        : PowerOnWarningCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 29-01-2008 dd-mm-yyyy                                 */
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
#include <PowerOnWarningCtrl.h>

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
PowerOnWarningCtrl::PowerOnWarningCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    mpPowerOnWarningDelay[fault_id] = new AlarmDelay(this);
    mPowerOnWarningDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PowerOnWarningCtrl::~PowerOnWarningCtrl()
{
  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    delete(mpPowerOnWarningDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PowerOnWarningCtrl::InitSubTask()
{
  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    mpPowerOnWarningDelay[fault_id]->InitAlarmDelay();
  }

  mSetPowerOnWarning = true;
  mClearPowerOnWarning = false;

  mRunRequestedFlag = true;
  ReqTaskTime();                         // Assures task is run at startup
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PowerOnWarningCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    if (mPowerOnWarningDelayCheckFlag[fault_id] == true)
    {
      mPowerOnWarningDelayCheckFlag[fault_id] = false;
      mpPowerOnWarningDelay[fault_id]->CheckErrorTimers();
    }
  }

  // The very first time RunSubTask is called mSetPowerOnWarning is true (set in InitSubTask).
  // Thus the warning will be set - and mSetPowerOnWarning is cleared.
  // The PowerOnWarningDealy object will trigger the RunSubTask when the delay has expired and this
  // call will assure that the warning is set - at the same time a request for TaskTime is sent again,
  // this time for a run of the task to clear the warning again.
  if (mSetPowerOnWarning == true)
  {
    mpPowerOnWarningDelay[POWC_FAULT_OBJ_POWER_ON_WARNING]->SetFault();
    mSetPowerOnWarning = false;
  }
  else
  {
    if (mClearPowerOnWarning == false)
    {
      mClearPowerOnWarning = true;
    }
    else
    {
      mpPowerOnWarningDelay[POWC_FAULT_OBJ_POWER_ON_WARNING]->ResetFault();
    }
  }

  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    mpPowerOnWarningDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void PowerOnWarningCtrl::ConnectToSubjects()
{
  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    mpPowerOnWarningDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void PowerOnWarningCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpPowerOnWarningDelay[POWC_FAULT_OBJ_POWER_ON_WARNING])
  {
    mPowerOnWarningDelayCheckFlag[POWC_FAULT_OBJ_POWER_ON_WARNING] = true;
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
void PowerOnWarningCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_POWC_FAULT_OBJ; fault_id < NO_OF_POWC_FAULT_OBJ; fault_id++)
  {
    mpPowerOnWarningDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void PowerOnWarningCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_POWC_POWER_ON_WARNING_OBJ:
      mpPowerOnWarningDelay[POWC_FAULT_OBJ_POWER_ON_WARNING]->SetSubjectPointer(Id, pSubject);
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
