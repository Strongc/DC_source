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
#include <MpcTime.h>
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
  mpChemicalTotalDosed->SetValue(mpDosingVolumeTotalLog->GetValue()/1000.f);  //l->m3
  mLastChemicalTotalDosed = mpChemicalTotalDosed->GetValue();
  //mpRunningDosingVolume->SetValue((U32)(mpChemicalTotalDosed->GetValue()*1000000));    //m3 -> ml
  mpDosingPumpInstalled.SetUpdated();
  mRestartFlag = false;

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
  bool non_gf_dosing_pump_installed = false;


  for (unsigned int i = FIRST_DOSING_PUMP_FAULT_OBJ; i < NO_OF_DOSING_PUMP_FAULT_OBJ; i++)
  {
    if (mDosingPumpAlarmDelayCheckFlag[i] == true)
    {
      mDosingPumpAlarmDelayCheckFlag[i] = false;
      mpDosingPumpAlarmDelay[i]->CheckErrorTimers();
    }
  }

  //Check Dosing Setup
  if (mpDosingPumpInstalled.IsUpdated() || mpDosingPumpType.IsUpdated())
  {
    mLastChemicalTotalDosed = mpChemicalTotalDosed->GetValue();
    mRestartFlag = true;
  }
  non_gf_dosing_pump_installed = (mpDosingPumpInstalled->GetValue() == true) && (mpDosingPumpType->GetValue() == DOSING_PUMP_TYPE_ANALOG);

  if (non_gf_dosing_pump_installed)
  {
    if (mpDosingPumpDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
    {
      mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->ResetFault();
      if (mpDosingRefAct.IsUpdated())
      {
        mRestartFlag = true;
      }
      StartNonGFDosingPump();
    }
    else
    {
      //Alarm Active
      mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->SetFault();
      StopNonGFDosingPump();
      //Pumping mode disabled
      mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_DISABLED);
    }
  }
  else
  {
    //Reset Alarm Once Dosing Disable
    mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->ResetFault();
    StopNonGFDosingPump();
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
  mpDosingPumpInstalled->Subscribe(this);
  mpDosingPumpType->Subscribe(this);
  mpChemicalTotalDosed->Subscribe(this);
  mpDosingRefAct->Subscribe(this);
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
  mpDosingPumpInstalled.Update(pSubject);
  mpDosingPumpType.Update(pSubject);
  mpChemicalTotalDosed.Update(pSubject);
  mpDosingRefAct.Update(pSubject);
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
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void NonGFDosingPumpCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_DPC_DOSING_PUMP_INSTALLED:
      mpDosingPumpInstalled.Attach(pSubject);
      break;
    case SP_DPC_DOSING_PUMP_TYPE:
      mpDosingPumpType.Attach(pSubject);
      break;
    case SP_DPC_OPERATION_MODE_DOSING_PUMP:
      mpOprModeDosingPump.Attach(pSubject);
      break;
    case SP_DPC_CHEMICAL_TOTAL_DOSED:
      mpChemicalTotalDosed.Attach(pSubject);
      break;
    case SP_DPC_RUNNING_DOSING_VOLUME:
      mpRunningDosingVolume.Attach(pSubject);
      break;
    case SP_DPC_DOSING_VOLUME_TOTAL_LOG:
      mpDosingVolumeTotalLog.Attach(pSubject);
      break;
    case SP_DPC_DOSING_PUMP_DIG_IN_REQUEST:
      mpDosingPumpDigInRequest.Attach(pSubject);
      break;
    case SP_DPC_RELAY_STATUS_RELAY_FUNC_DOSING_PUMP:
      mpDosingPumpStart.Attach(pSubject);
      break;
    case SP_DPC_DOSING_REF_ACT:
      mpDosingRefAct.Attach(pSubject);
      break;
    case SP_DPC_AO_DOSING_PUMP_SETPOINT:
      mpDosingPumpAOSetting.Attach(pSubject);
      break;
    case SP_DPC_SYS_ALARM_DOSING_PUMP_ALARM_OBJ:
      mDosingPumpAlarms[DOSING_PUMP_FAULT_OBJ].Attach(pSubject);
      mpDosingPumpAlarmDelay[DOSING_PUMP_FAULT_OBJ]->SetSubjectPointer(id, pSubject);
      break;
    default:
      break;
  }
}

void NonGFDosingPumpCtrl::StartNonGFDosingPump()
{
  MpcTime new_time(true);
  static U32 new_start_time = new_time.GetSecondsSince1Jan1970();
  U32 current_time = 0;
  bool run_flag = false;
  float ref_h2s = mpDosingRefAct->GetValue();
  float max_ao_value = 0;

  run_flag = (ref_h2s <= mpDosingPumpAOSetting->GetMinValue()) ? false : true;
  max_ao_value = mpDosingPumpAOSetting->GetMaxValue();

  ref_h2s = ((ref_h2s*1000.0f) > max_ao_value)? max_ao_value/1000.0f : ref_h2s;

  if (mRestartFlag)
  {
    mLastChemicalTotalDosed = mpChemicalTotalDosed->GetValue();
    new_start_time = new_time.GetSecondsSince1Jan1970();
  }

  mpDosingPumpStart->SetValue(run_flag);

  if (run_flag)
  {
    mRestartFlag = false;
    current_time = new_time.GetSecondsSince1Jan1970();
    mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_STARTED);
    // convert ref_h2s from l/h to l/s, multiple seconds is 1l, then convert l to m3
    mpChemicalTotalDosed->SetValue(((current_time-new_start_time)*(ref_h2s/3600))/1000.0f + mLastChemicalTotalDosed);
    mpRunningDosingVolume->SetValue((U32)(mpChemicalTotalDosed->GetValue()*1000000));    //m3 -> ml
  }
  else
  {
    mRestartFlag = true;
    mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_STOPPED);
  }
  mpDosingPumpAOSetting->SetValue(ref_h2s*1000.0);

  return;
}

void NonGFDosingPumpCtrl::StopNonGFDosingPump()
{
  mRestartFlag = true;
  mpDosingPumpStart->SetValue(false);
  //AO Off
  mpDosingPumpAOSetting->SetValue(0);
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

