/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: H2S Prevention                                   */
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
/* CLASS NAME       : NonGFDosingPumpCtrl                                   */
/*                                                                          */
/* FILE NAME        : NonGFDosingPumpCtrl.cpp                               */
/*                                                                          */
/* CREATED DATE     : 05-07-2013 dd-mm-yyyy                                 */
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
#include "NonGFDosingPumpCtrl.h"
#include <GeniSlaveIf.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
int NonGFDosingPumpCtrl::counter;
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
NonGFDosingPumpCtrl::NonGFDosingPumpCtrl()
{
  //mDosingPumpType = DOSING_PUMP_TYPE_DDA;
  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    mpDosingPumpAlarmDelay[i] = new AlarmDelay(this);
    mDosingPumpAlarmDelayCheckFlag[i] = false;
  }
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
NonGFDosingPumpCtrl::~NonGFDosingPumpCtrl()
{
  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    delete mpDosingPumpAlarmDelay[i];
  }
}
/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::InitSubTask()
{
  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    mpDosingPumpAlarmDelay[i]->InitAlarmDelay();
    mpDosingPumpAlarmDelay[i]->ResetFault();
    mpDosingPumpAlarmDelay[i]->ResetWarning();
    mDosingPumpAlarmDelayCheckFlag[i] = false;
  }
  ReqTaskTime();                         // Assures task is run at startup
}
/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::RunSubTask()
{
  bool dosing_pump_ed = 0;
  ACTUAL_OPERATION_MODE_TYPE actual_operation_mode;
  ALARM_ID_TYPE new_alarm_code = ALARM_ID_NO_ALARM;

  dosing_pump_ed = mpDosingPumpEnable->GetValue();

  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    if (mDosingPumpAlarmDelayCheckFlag[i] == true)
    {
      mDosingPumpAlarmDelayCheckFlag[i] = false;
      mpDosingPumpAlarmDelay[i]->CheckErrorTimers();
    }
  }

  /* test through alarm */
  //I guess this digital in is singal external, not the check state of checkbox in menu DI
  //if (mpDosingPumpDigInRequest.IsUpdated())
  //{
    //if (mpDosingPumpDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
  if (dosing_pump_ed && (mpDosingPumpType->GetValue() == DOSING_PUMP_TYPE_ANALOG))
  {
    mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->SetFault();
  }
  else
  {
    mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->ResetFault();
  }
  
  // Service AlarmDelays
  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    mpDosingPumpAlarmDelay[i]->UpdateAlarmDataPoint();
  }
}
/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::ConnectToSubjects()
{
  mpDosingPumpEnable->Subscribe(this);
  mpDosingPumpType->Subscribe(this);
  mpDosingPumpDigInRequest->Subscribe(this);
  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    mpDosingPumpAlarmDelay[i]->ConnectToSubjects();
  }
}
/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::Update(Subject* pSubject)
{
  mpDosingPumpEnable.Update(pSubject);
  mpDosingPumpType.Update(pSubject);
  mpDosingPumpDigInRequest.Update(pSubject);

  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    if (pSubject == mpDosingPumpAlarmDelay[i])
    {
      mDosingPumpAlarmDelayCheckFlag[i] = true;
      break;
    }
  }
  ReqTaskTime();
}
/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    mpDosingPumpAlarmDelay[i]->SubscribtionCancelled(pSubject);
  }
}
/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_DPC_DOSING_PUMP_INSTALLED:
      mpDosingPumpEnable.Attach(pSubject);
      break;
    case SP_DPC_DOSING_PUMP_TYPE:
      mpDosingPumpType.Attach(pSubject);
      break;
    case SP_DPC_DOSING_PUMP_DIG_IN_REQUEST:
      mpDosingPumpDigInRequest.Attach(pSubject);
      break;
    case SP_DPC_MEASURED_VALUE_CHEMICAL_CONTAINER:
      mpMeasuredValue.Attach(pSubject);
      break;
    case SP_DPC_SYS_ALARM_DOSING_PUMP_ALARM_OBJ:
      mDosingPumpAlarms[DOSING_PUMP_FAULT_OBJ].Attach(pSubject);
      mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->SetSubjectPointer(id, pSubject);
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

