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
/* CLASS NAME       : AppModeCtrl                                           */
/*                                                                          */
/* FILE NAME        : AppModeCtrl.cpp                                       */
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
#include <AppModeCtrl.h>

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
AppModeCtrl::AppModeCtrl()
{
  mRunRequestedFlag = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AppModeCtrl::~AppModeCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AppModeCtrl::InitSubTask()
{
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AppModeCtrl::RunSubTask()
{
  mRunRequestedFlag = false;


  /*****************************************************************************
   * Handle change in any of the incoming statusses
   * - adjust app mode
   ****************************************************************************/


  if (mpServiceModeEnabled->GetValue())
  {
    mpAppMode->SetValue(APPLICATION_MODE_SERVICE_MODE);
  }
  else if (mpPumpCtrlMode->GetValue() == APPLICATION_MODE_STARTUP_DELAY)
  {
    mpAppMode->SetValue(APPLICATION_MODE_STARTUP_DELAY);
  }
  else if (mpMainsFailure->GetAlarmPresent() == ALARM_STATE_ALARM)
  {
    mpAppMode->SetValue(APPLICATION_MODE_MAINS_FAILURE);
  }
  else if (mpAllPumpAlarmFlag->GetValue() == true)
  {
    mpAppMode->SetValue(APPLICATION_MODE_ALL_PUMP_ALARMS);
  }
  else if (AllPumpsOutOfOperation())
  {
    mpAppMode->SetValue(APPLICATION_MODE_ALL_PUMPS_OUT_OF_OPERATION);
  }
  else if (AllPumpsManual())
  {
    mpAppMode->SetValue(APPLICATION_MODE_MANUAL_CONTROL);
  }
  else if (mpPumpCtrlMode->GetValue() == APPLICATION_MODE_INTERLOCKED)
  {
    mpAppMode->SetValue(APPLICATION_MODE_INTERLOCKED);
  }
  else if ((mpConflictingLevelsAlarmObj->GetAlarmPresent() == ALARM_STATE_ALARM) ||
           (mpLevelSensorAlarmObj->GetAlarmPresent() == ALARM_STATE_ALARM))
  {
    mpAppMode->SetValue(APPLICATION_MODE_LEVEL_SENSOR_ERROR);
  }
  else
  {
    mpAppMode->SetValue(mpPumpCtrlMode->GetValue());
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AppModeCtrl::ConnectToSubjects()
{
  mpMainsFailure->Subscribe(this);
  mpAllPumpAlarmFlag->Subscribe(this);
  
  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    mpOperationModeReqPump[i]->Subscribe(this);
    mpOperationModeActualPump[i]->Subscribe(this);
  }

  mpConflictingLevelsAlarmObj->Subscribe(this);
  mpLevelSensorAlarmObj->Subscribe(this);
  mpPumpCtrlMode->Subscribe(this);
  mpServiceModeEnabled->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AppModeCtrl::Update(Subject* pSubject)
{
  if (mRunRequestedFlag == false)
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
void AppModeCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void AppModeCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_AMC_MAINS_FAILURE_ALARM_OBJ:
      mpMainsFailure.Attach(pSubject);
      break;
    case SP_AMC_ALL_PUMP_ALARM_FLAG:
      mpAllPumpAlarmFlag.Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_REQ_PUMP_1:
      mpOperationModeReqPump[PUMP_1].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_REQ_PUMP_2:
      mpOperationModeReqPump[PUMP_2].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_REQ_PUMP_3:
      mpOperationModeReqPump[PUMP_3].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_REQ_PUMP_4:
      mpOperationModeReqPump[PUMP_4].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_REQ_PUMP_5:
      mpOperationModeReqPump[PUMP_5].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_REQ_PUMP_6:
      mpOperationModeReqPump[PUMP_6].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_ACTUAL_PUMP_1:
      mpOperationModeActualPump[PUMP_1].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_ACTUAL_PUMP_2:
      mpOperationModeActualPump[PUMP_2].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_ACTUAL_PUMP_3:
      mpOperationModeActualPump[PUMP_3].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_ACTUAL_PUMP_4:
      mpOperationModeActualPump[PUMP_4].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_ACTUAL_PUMP_5:
      mpOperationModeActualPump[PUMP_5].Attach(pSubject);
      break;
    case SP_AMC_OPERATION_MODE_ACTUAL_PUMP_6:
      mpOperationModeActualPump[PUMP_6].Attach(pSubject);
      break;
    case SP_AMC_CONFLICTING_LEVELS_ALARM_OBJ:
      mpConflictingLevelsAlarmObj.Attach(pSubject);
      break;
    case SP_AMC_LEVEL_SENSOR_ALARM_OBJ:
      mpLevelSensorAlarmObj.Attach(pSubject);
      break;
    case SP_AMC_PUMP_CTRL_MODE:
      mpPumpCtrlMode.Attach(pSubject);
      break;
    case SP_AMC_APPLICATION_MODE:
      mpAppMode.Attach(pSubject);
      break;
    case SP_AMC_SERVICE_MODE_ENABLED:
      mpServiceModeEnabled.Attach(pSubject);
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
 * Function - AllPumpSOutOfOperation
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AppModeCtrl::AllPumpsOutOfOperation(void)
{
  bool ret_val = true;

  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    if (mpOperationModeActualPump[i]->GetValue()==ACTUAL_OPERATION_MODE_STOPPED ||
        mpOperationModeActualPump[i]->GetValue()==ACTUAL_OPERATION_MODE_STARTED)
    {
      ret_val = false;
    }
  }
  return ret_val;
}

/*****************************************************************************
 * Function - AllPumpManual
 * DESCRIPTION:
 *
 *****************************************************************************/
bool AppModeCtrl::AllPumpsManual(void)
{
  bool ret_val = true;

  for (int i = 0; i < NO_OF_PUMPS; i++)
  {
    if ((mpOperationModeActualPump[i]->GetValue() == ACTUAL_OPERATION_MODE_STOPPED ||
         mpOperationModeActualPump[i]->GetValue() == ACTUAL_OPERATION_MODE_STARTED) &&
         mpOperationModeReqPump[i]->GetValue() == REQ_OPERATION_MODE_AUTO)
    {
      ret_val = false;
    }
  }
  return ret_val;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
