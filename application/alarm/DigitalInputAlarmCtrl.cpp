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
/* CLASS NAME       : DigitalInputAlarmCtrl                                 */
/*                                                                          */
/* FILE NAME        : DigitalInputAlarmCtrl.cpp                             */
/*                                                                          */
/* CREATED DATE     : 17-04-2008 dd-mm-yyyy                                 */
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
#include <DigitalInputAlarmCtrl.h>

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
DigitalInputAlarmCtrl::DigitalInputAlarmCtrl(bool inverse)
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  mInverse = inverse;
  mpAlarmDelay = new AlarmDelay(this);
  mAlarmDelayCheckFlag = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DigitalInputAlarmCtrl::~DigitalInputAlarmCtrl()
{
  delete(mpAlarmDelay);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DigitalInputAlarmCtrl::InitSubTask()
{
  mpAlarmDelay->InitAlarmDelay();

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DigitalInputAlarmCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  // Pre handling of alarms
  if (mAlarmDelayCheckFlag == true)
  {
    mAlarmDelayCheckFlag = false;
    mpAlarmDelay->CheckErrorTimers();
  }

  // Do the real stuff
  if(mInverse==false)
  {
    if (mpDigInFuncState->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
    {
      mpAlarmDelay->SetFault();
    }
    else
    {
      mpAlarmDelay->ResetFault();
    }
  }
  else
  {
    if (mpDigInFuncState->GetValue() == DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE)
    {
      mpAlarmDelay->SetFault();
    }
    else
    {
      mpAlarmDelay->ResetFault();
    }
  }

  // Post handling of alarms
  mpAlarmDelay->UpdateAlarmDataPoint();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void DigitalInputAlarmCtrl::ConnectToSubjects()
{
  mpDigInFuncState->Subscribe(this);
  mpAlarmDelay->ConnectToSubjects();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void DigitalInputAlarmCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpAlarmDelay)
  {
    mAlarmDelayCheckFlag = true;
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
void DigitalInputAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpAlarmDelay->SubscribtionCancelled(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void DigitalInputAlarmCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_DIAC_DIG_IN_FUNC_STATE:
      mpDigInFuncState.Attach(pSubject);
      break;
    case SP_DIAC_FAULT_OBJ :
      mpAlarmDelay->SetSubjectPointer(Id, pSubject);
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
