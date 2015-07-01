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
/* CLASS NAME       : DDACtrl                                       */
/*                                                                          */
/* FILE NAME        : DDACtrl.cpp                                   */
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
#include "DDACtrl.h"
#include <GeniSlaveIf.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/
// SW Timers
enum
{
  DDA_WAIT_TIMER,
  DDA_RUN_TIMER
};


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
DDACtrl::DDACtrl()
{
  //mDosingPumpType = DOSING_PUMP_TYPE_DDA;
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    mpAlarmDelay[i] = new AlarmDelay(this);
    mAlarmDelayCheckFlag[i] = false;
  }
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DDACtrl::~DDACtrl()
{
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    delete mpAlarmDelay[i];
  }
}
/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DDACtrl::InitSubTask()
{
  //mpTimerObjList[DDA_RUN_TIMER] = new SwTimer(mpDDARunTime->GetValue(), S, false, false, this);
  //mpTimerObjList[ANTI_SEIZE_WAIT_TIMER] = new SwTimer(mpDDAWaitTime->GetValue(), S, true, false, this);
  mpH2SLevelAct->SetValue(10);
  mpDDADosingFeedTankLevel->SetAsFloat(23.55555);
  mpDDAChemicalTotalDosed->SetAsFloat(32.55555);
  //mRunRequestedFlag = true;
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    mpAlarmDelay[i]->InitAlarmDelay();
    mpAlarmDelay[i]->ResetFault();
    mpAlarmDelay[i]->ResetWarning();
    mAlarmDelayCheckFlag[i] = false;
  }
  mpDDAInstalled->SetValue(mpDosingPumpInstalled->GetValue() == true && (mpDosingPumpType->GetValue() == DOSING_PUMP_TYPE_DDA));
  ReqTaskTime();                         // Assures task is run at startup
}
/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DDACtrl::RunSubTask()
{
  ACTUAL_OPERATION_MODE_TYPE actual_operation_mode;
  ALARM_ID_TYPE new_alarm_code = ALARM_ID_NO_ALARM;
  U32 dda_h2s_level_act;
  float dda_dosing_feed_tank_level;
  float dda_chemical_total_dosed;

  dda_h2s_level_act = mpH2SLevelAct->GetValue();
  dda_dosing_feed_tank_level = mpDDADosingFeedTankLevel->GetValue();
  dda_chemical_total_dosed = mpDDAChemicalTotalDosed->GetValue();

  //mpDosingPumpType->SetValue(DOSING_PUMP_TYPE_ANALOG);

  // Service AlarmDelays
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    if (mAlarmDelayCheckFlag[i] == true)
    {
      mAlarmDelayCheckFlag[i] = false;
      mpAlarmDelay[i]->CheckErrorTimers();
    }
  }

  // Set a DDA installed flag.
  if (mpDosingPumpInstalled.IsUpdated() || mpDosingPumpType.IsUpdated())
  {
    mpDDAInstalled->SetValue(mpDosingPumpInstalled->GetValue() == true && (mpDosingPumpType->GetValue() == DOSING_PUMP_TYPE_DDA));
  }
  
  // set reference value everytime
  mpDDARef->SetValue((U32)(10.0 * mpSetDosingRef->GetValue()));

  //test
  //if (mpDDARef->GetValue() == 20)
  //{
    //mpAlarmDelay[DDA_FAULT_OBJ_H2S]->SetFault();
  //}
  //mAlarms[DDA_FAULT_OBJ_H2S]->SetValue((ALARM_ID_TYPE)118);
  //mpAlarmDelay[DDA_FAULT_OBJ_H2S]->SetFault();
  if (mpSetH2SLevel.IsUpdated())
  {
    mpH2SLevelAct->SetValue(mpSetH2SLevel->GetValue());
  }
  if (mpSetH2SFault->GetValue() & 0x01)
  {
    mpAlarmDelay[DDA_FAULT_OBJ_H2S]->SetFault();
  }

  // Service AlarmDelays
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    mpAlarmDelay[i]->UpdateAlarmDataPoint();
  }

}
/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void DDACtrl::ConnectToSubjects()
{
  mpDosingPumpInstalled->Subscribe(this);
  mpDosingPumpType->Subscribe(this);
  mpSetDosingRef->Subscribe(this);
  mpSetH2SFault->Subscribe(this);
  mpDDARef->Subscribe(this);
  mpH2SLevelAct->Subscribe(this);
  mpSetH2SLevel->Subscribe(this);
  mpDDALevelToday->Subscribe(this);
  mpDDALevelYesterday->Subscribe(this);
  mpDDADosingFeedTankLevel->Subscribe(this);
  mpDDAChemicalTotalDosed->Subscribe(this);
  mpDosingPumpType->Subscribe(this);
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    mpAlarmDelay[i]->ConnectToSubjects();
  }
}
/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void DDACtrl::Update(Subject* pSubject)
{
  mpDosingPumpInstalled.Update(pSubject);
  mpDosingPumpType.Update(pSubject);
  mpSetDosingRef.Update(pSubject);
  mpSetH2SFault.Update(pSubject);
  mpDDARef.Update(pSubject);
  mpH2SLevelAct.Update(pSubject);
  mpSetH2SLevel.Update(pSubject);
  mpDDALevelToday.Update(pSubject);
  mpDDALevelYesterday.Update(pSubject);
  mpDDADosingFeedTankLevel.Update(pSubject);
  mpDDAChemicalTotalDosed.Update(pSubject);

  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    if (pSubject == mpAlarmDelay[i])
    {
      mAlarmDelayCheckFlag[i] = true;
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
void DDACtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    mpAlarmDelay[i]->SubscribtionCancelled(pSubject);
  }
}
/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void DDACtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_DDAC_DOSING_PUMP_INSTALLED:
      mpDosingPumpInstalled.Attach(pSubject);
      break;
    case SP_DDAC_DOSING_PUMP_TYPE:
      mpDosingPumpType.Attach(pSubject);
      break;
    case SP_DDAC_DDA_INSTALLED:
      mpDDAInstalled.Attach(pSubject);
      break;
    case SP_DDAC_SET_H2S_FAULT:
      mpSetH2SFault.Attach(pSubject);
      break;
    case SP_DDAC_SET_DOSING_REF:
      mpSetDosingRef.Attach(pSubject);
      break;
    case SP_DDAC_DDA_REFERENCE:
      mpDDARef.Attach(pSubject);
      break;
    case SP_DDAC_H2S_LEVEL_ACT:
      mpH2SLevelAct.Attach(pSubject);
      break;
    case SP_DDAC_SET_H2S_LEVEL:
      mpSetH2SLevel.Attach(pSubject);
      break;
    case SP_DDAC_H2S_LEVEL_TODAY:
      mpDDALevelToday.Attach(pSubject);
      break;
    case SP_DDAC_H2S_LEVEL_YESTERDAY:
      mpDDALevelYesterday.Attach(pSubject);
      break;
    case SP_DDAC_DOSING_FEED_TANK_LEVEL:
      mpDDADosingFeedTankLevel.Attach(pSubject);
      break;
    case SP_DDAC_CHEMICAL_TOTAL_DOSED:
      mpDDAChemicalTotalDosed.Attach(pSubject);
      break;
    case SP_DDAC_H2S_SENSOR_FAULT_OBJ:
      mAlarms[DDA_FAULT_OBJ_H2S].Attach(pSubject);
      mpAlarmDelay[DDA_FAULT_OBJ_H2S]->SetSubjectPointer(id, pSubject);
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
 * Function - StopTimers
 * DESCRIPTION: Stop the run- and waittimer and clear all timerflags. Intended
 *              for initializing the timers.
 *
 *****************************************************************************/
void DDACtrl::StopTimers(void)
{
  mDDAWaitTimerFlag = false;
  mDDARunTimerFlag = false;
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

