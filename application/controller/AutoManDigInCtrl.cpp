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
/* CLASS NAME       : AutoManDigInCtrl                                      */
/*                                                                          */
/* FILE NAME        : AutoManDigInCtrl.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 16-07-2007 dd-mm-yyyy                                 */
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
#include <AutoManDigInCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

 // Functiontable for AutoManDigInCtrl

 const REQ_OPERATION_MODE_TYPE mcFunctionTable[NO_OF_DIGITAL_INPUT_FUNC_STATE][NO_OF_DIGITAL_INPUT_FUNC_STATE] =
 {
   {REQ_OPERATION_MODE_OFF, REQ_OPERATION_MODE_AUTO, REQ_OPERATION_MODE_OFF /* Illegal */},              // on_off not active
   {REQ_OPERATION_MODE_ON, REQ_OPERATION_MODE_OFF /* Illegal */, REQ_OPERATION_MODE_OFF /* Illegal */},  // on_off active
   {REQ_OPERATION_MODE_OFF, REQ_OPERATION_MODE_AUTO, REQ_OPERATION_MODE_AUTO}                            // on_off not configured
 };

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
AutoManDigInCtrl::AutoManDigInCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AutoManDigInCtrl::~AutoManDigInCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AutoManDigInCtrl::InitSubTask()
{
  mpReferenceForContactorFeedbackAvailable->SetValue(true);

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AutoManDigInCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  mpOprModeDI->SetValue(mcFunctionTable[mpDigInFuncStateOnOffPump->GetValue()][mpDigInFuncStateAutoManPump->GetValue()]);

  if ((mpDigInFuncStateOnOffPump->GetValue() == DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED) &&
      (mpDigInFuncStateAutoManPump->GetValue() == DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE))
  {
    // Manual pump control is wired outside the controller - we cannot know which state the pump
    // is in. Mark contactor feedback as unknown.
    mpReferenceForContactorFeedbackAvailable->SetValue(false);
  }
  else
  {
    mpReferenceForContactorFeedbackAvailable->SetValue(true);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AutoManDigInCtrl::ConnectToSubjects()
{
  mpDigInFuncStateOnOffPump->Subscribe(this);
  mpDigInFuncStateAutoManPump->Subscribe(this);

}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AutoManDigInCtrl::Update(Subject* pSubject)
{
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
void AutoManDigInCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AutoManDigInCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_AMDIC_OPERATION_MODE_DI:
      mpOprModeDI.Attach(pSubject);
      break;
    case SP_AMDIC_DIG_IN_FUNC_STATE_ON_OFF_PUMP:
      mpDigInFuncStateOnOffPump.Attach(pSubject);
      break;
    case SP_AMDIC_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP:
      mpDigInFuncStateAutoManPump.Attach(pSubject);
      break;
    case SP_AMDIC_REFERENCE_FOR_CONTACTOR_FEEDBACK_AVAILABLE:
      mpReferenceForContactorFeedbackAvailable.Attach(pSubject);
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
