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
/* CLASS NAME       : SimulatedAlarmCtrl                                    */
/*                                                                          */
/* FILE NAME        : SimulatedAlarmCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 03-06-2008 dd-mm-yyyy                                 */
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
#include <SimulatedAlarmCtrl.h>

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
SimulatedAlarmCtrl::SimulatedAlarmCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    mpSimulatedAlarmDelay[fault_id] = new AlarmDelay(this);
    mSimulatedAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SimulatedAlarmCtrl::~SimulatedAlarmCtrl()
{
  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    delete(mpSimulatedAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SimulatedAlarmCtrl::InitSubTask()
{
  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    mpSimulatedAlarmDelay[fault_id]->InitAlarmDelay();
  }

  mRunRequestedFlag = true;
  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SimulatedAlarmCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    if (mSimulatedAlarmDelayCheckFlag[fault_id] == true)
    {
      mSimulatedAlarmDelayCheckFlag[fault_id] = false;
      mpSimulatedAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  if (mpSimActivate->GetValue() == 0xAA)
  {
    // The alarm object is configured as a digital alarm - thus only SetFault and ResetFault are used
    // to generate an alarm OR a warning. Both cannot be present at the same time.
    if (mpAlarmCodeSim->GetValue() == ALARM_ID_NO_ALARM)
    {
      // The simulated alarm should be reset
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->SetValue(ALARM_ID_NO_ALARM);
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->SetErroneousUnit(ERRONEOUS_UNIT_SYSTEM);
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->SetErroneousUnitNumber(0);
      mpSimulatedAlarmDelay[SAC_FAULT_OBJ_SIMULATED]->ResetFault();
    }
    else
    {
      // Set all the simulated alarm parameters and activate the alarm
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->SetValue((ALARM_ID_TYPE)mpAlarmCodeSim->GetValue());
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->SetErroneousUnit((ERRONEOUS_UNIT_TYPE)mpDeviceTypeCodeSim->GetValue());
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->SetErroneousUnitNumber(mpDeviceNoSim->GetValue());
      if (mpResetTypeSim->GetValue() == RESET_MANUAL)
      {
        mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->GetAlarmConfig()->SetAutoAcknowledge(false);
      }
      else
      {
        mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->GetAlarmConfig()->SetAutoAcknowledge(true);
      }

      switch (mpActionTypeSim->GetValue())
      {
        case ALARM_STATE_WARNING:
          mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->GetAlarmConfig()->SetWarningEnabled(true);
          mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->GetAlarmConfig()->SetAlarmEnabled(false);
          mpSimulatedAlarmDelay[SAC_FAULT_OBJ_SIMULATED]->SetFault();
          break;
        case ALARM_STATE_ALARM:
          mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->GetAlarmConfig()->SetAlarmEnabled(true);
          mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED]->GetAlarmConfig()->SetWarningEnabled(false);
          mpSimulatedAlarmDelay[SAC_FAULT_OBJ_SIMULATED]->SetFault();
          break;
        case ALARM_STATE_READY:
          mpSimulatedAlarmDelay[SAC_FAULT_OBJ_SIMULATED]->ResetFault();
          break;
        default:
          break;
      }
    }

    // Reset all values after use
    mpSimActivate->SetValue(0);
    mpAlarmCodeSim->SetValue(0);
    mpDeviceTypeCodeSim->SetValue(0);
    mpDeviceNoSim->SetValue(0);
    mpResetTypeSim->SetValue(0);
    mpActionTypeSim->SetValue(0);
  }


  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    mpSimulatedAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void SimulatedAlarmCtrl::ConnectToSubjects()
{
  mpSimActivate->Subscribe(this);

  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    mpSimulatedAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void SimulatedAlarmCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpSimulatedAlarmDelay[SAC_FAULT_OBJ_SIMULATED])
  {
    mSimulatedAlarmDelayCheckFlag[SAC_FAULT_OBJ_SIMULATED] = true;
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
void SimulatedAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_SAC_FAULT_OBJ; fault_id < NO_OF_SAC_FAULT_OBJ; fault_id++)
  {
    mpSimulatedAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void SimulatedAlarmCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_SAC_SIM_ACTIVATE:
      mpSimActivate.Attach(pSubject);
      break;
    case SP_SAC_ALARM_CODE_SIM:
      mpAlarmCodeSim.Attach(pSubject);
      break;
    case SP_SAC_DEVICE_TYPE_CODE_SIM:
      mpDeviceTypeCodeSim.Attach(pSubject);
      break;
    case SP_SAC_DEVICE_NO_SIM:
      mpDeviceNoSim.Attach(pSubject);
      break;
    case SP_SAC_RESET_TYPE_SIM:
      mpResetTypeSim.Attach(pSubject);
      break;
    case SP_SAC_ACTION_TYPE_SIM:
      mpActionTypeSim.Attach(pSubject);
      break;

    case SP_SAC_SIMULATED_ALARM_OBJ:
      mpSimulatedAlarmObj[SAC_FAULT_OBJ_SIMULATED].Attach(pSubject);
      mpSimulatedAlarmDelay[SAC_FAULT_OBJ_SIMULATED]->SetSubjectPointer(Id, pSubject);
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
