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
/* CLASS NAME       : StartPumpCnt                                          */
/*                                                                          */
/* FILE NAME        : StartPumpCnt.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
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
#include <StartPumpCnt.h>

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
StartPumpCnt::StartPumpCnt()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    mpStartPumpCntAlarmDelay[fault_id] = new AlarmDelay(this);
    mStartPumpCntAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
StartPumpCnt::~StartPumpCnt()
{
  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    delete(mpStartPumpCntAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void StartPumpCnt::InitSubTask()
{
  mActTime = 0;
  mStartQue.clear();
  mplastStartTime->SetValue( mActTime );

  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    mpStartPumpCntAlarmDelay[fault_id]->InitAlarmDelay();
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void StartPumpCnt::RunSubTask()
{
  std::deque<int>::iterator iter;
  int i;

  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    if (mStartPumpCntAlarmDelayCheckFlag[fault_id] == true)
    {
      mStartPumpCntAlarmDelayCheckFlag[fault_id] = false;
      mpStartPumpCntAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  // Task must run each second to make mActTime a timer in seconds
  mActTime++;

  if (mpActualOperationMode.IsUpdated() == true)
  {
    if (mpActualOperationMode->GetValue() == ACTUAL_OPERATION_MODE_STARTED)
    {
      // Operation mode has changed to started - increment start counter if no danger for overrun
      if (mpPumpStartCnt->GetValue() != mpPumpStartCnt->GetMaxValue())
      {
        mpPumpStartCnt->SetValue(mpPumpStartCnt->GetValue() + 1);
      }
      mStartQue.push_front( mActTime );
      mplastStartTime->SetValue( mActTime );
    }
  }

  for(i=0;i<mStartQue.size();i++)
  {
    if( (mStartQue.back()+3600) < mActTime  )
      mStartQue.pop_back();
    else
      break;
  }
  mpStartPrHour->SetValue(mStartQue.size());

  // Check up against warning and alarm limits.
  if (mpStartPrHour->GetValue() > mpStartPumpCntFaultObj[SPC_FAULT_OBJ_STARTS_PR_HOUR]->GetAlarmConfig()->GetWarningLimit()->GetAsInt())
  {
    // Warning limit is exceeded - set warning
    mpStartPumpCntAlarmDelay[SPC_FAULT_OBJ_STARTS_PR_HOUR]->SetWarning();
  }
  else
  {
    mpStartPumpCntAlarmDelay[SPC_FAULT_OBJ_STARTS_PR_HOUR]->ResetWarning();
  }

  if (mpStartPrHour->GetValue() > mpStartPumpCntFaultObj[SPC_FAULT_OBJ_STARTS_PR_HOUR]->GetAlarmConfig()->GetAlarmLimit()->GetAsInt())
  {
    // Alarm limit is exceeded - set alarm
    mpStartPumpCntAlarmDelay[SPC_FAULT_OBJ_STARTS_PR_HOUR]->SetFault();
  }
  else
  {
    mpStartPumpCntAlarmDelay[SPC_FAULT_OBJ_STARTS_PR_HOUR]->ResetFault();
  }

  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    mpStartPumpCntAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void StartPumpCnt::ConnectToSubjects()
{
  mpActualOperationMode->Subscribe(this);

  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    mpStartPumpCntAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void StartPumpCnt::Update(Subject* pSubject)
{
  mpActualOperationMode.Update(pSubject);

  if (pSubject == mpStartPumpCntAlarmDelay[SPC_FAULT_OBJ_STARTS_PR_HOUR])
  {
    mStartPumpCntAlarmDelayCheckFlag[SPC_FAULT_OBJ_STARTS_PR_HOUR] = true;
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void StartPumpCnt::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_SPC_FAULT_OBJ; fault_id < NO_OF_SPC_FAULT_OBJ; fault_id++)
  {
    mpStartPumpCntAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void StartPumpCnt::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_SPC_ACTUAL_OPERATION_MODE:
      mpActualOperationMode.Attach(pSubject);
      break;
    case SP_SPC_PUMP_START_CNT:
      mpPumpStartCnt.Attach(pSubject);
      break;
    case SP_SPC_STARTS_PR_HOUR:
      mpStartPrHour.Attach(pSubject);
      break;
    case SP_SPC_LAST_START_TIME:
      mplastStartTime.Attach(pSubject);
      break;
    case SP_SPC_STARTS_PR_HOUR_FAULT_OBJ :
      mpStartPumpCntFaultObj[SPC_FAULT_OBJ_STARTS_PR_HOUR].Attach(pSubject);
      mpStartPumpCntAlarmDelay[SPC_FAULT_OBJ_STARTS_PR_HOUR]->SetSubjectPointer(Id, pSubject);
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
