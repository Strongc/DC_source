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
/* CLASS NAME       : NoOfRunningPumpsCtrl                                  */
/*                                                                          */
/* FILE NAME        : NoOfRunningPumpsCtrl.cpp                              */
/*                                                                          */
/* CREATED DATE     : 21-08-2007 dd-mm-yyyy                                 */
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
#include <NoOfRunningPumpsCtrl.h>

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
NoOfRunningPumpsCtrl::NoOfRunningPumpsCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
NoOfRunningPumpsCtrl::~NoOfRunningPumpsCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void NoOfRunningPumpsCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void NoOfRunningPumpsCtrl::RunSubTask()
{
  U8 running_pumps = 0;
  bool all_available_pumps_running = true;

  mRunRequestedFlag = false;


  /*****************************************************************************
   * Handle change in pump actual operation mode
   * - adjust no of running pumps to reflect pumps in ACTUAL_OPERATION_MODE_STARTED.
   ****************************************************************************/
  for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
  {
    if (mpActualOperationModePump[pump_no]->GetValue() == ACTUAL_OPERATION_MODE_STARTED)
    {
      running_pumps++;
    }

    if( mpActualOperationModePump[pump_no]->GetValue() == ACTUAL_OPERATION_MODE_STOPPED 
      && mpActualOperationModePump[pump_no]->GetQuality() == DP_AVAILABLE)
    {
      all_available_pumps_running = false;
    }
  }

  mpNoOfRunningPumps->SetValue(running_pumps);

  mpAnyPumpRunning->SetValue(running_pumps > 0);

  mpAllPumpsRunning->SetValue(running_pumps > 0 && all_available_pumps_running);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void NoOfRunningPumpsCtrl::ConnectToSubjects()
{
  for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
  {
    mpActualOperationModePump[pump_no]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void NoOfRunningPumpsCtrl::Update(Subject* pSubject)
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
void NoOfRunningPumpsCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void NoOfRunningPumpsCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_NORPC_OPERATION_MODE_ACTUAL_PUMP_1:
      mpActualOperationModePump[PUMP_1].Attach(pSubject);
      break;
    case SP_NORPC_OPERATION_MODE_ACTUAL_PUMP_2:
      mpActualOperationModePump[PUMP_2].Attach(pSubject);
      break;
    case SP_NORPC_OPERATION_MODE_ACTUAL_PUMP_3:
      mpActualOperationModePump[PUMP_3].Attach(pSubject);
      break;
    case SP_NORPC_OPERATION_MODE_ACTUAL_PUMP_4:
      mpActualOperationModePump[PUMP_4].Attach(pSubject);
      break;
    case SP_NORPC_OPERATION_MODE_ACTUAL_PUMP_5:
      mpActualOperationModePump[PUMP_5].Attach(pSubject);
      break;
    case SP_NORPC_OPERATION_MODE_ACTUAL_PUMP_6:
      mpActualOperationModePump[PUMP_6].Attach(pSubject);
      break;
    case SP_NORPC_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_NORPC_ALL_PUMPS_RUNNING:
      mpAllPumpsRunning.Attach(pSubject);
      break;
    case SP_NORPC_ANY_PUMP_RUNNING:
      mpAnyPumpRunning.Attach(pSubject);
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
