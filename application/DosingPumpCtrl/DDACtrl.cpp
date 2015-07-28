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
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();
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
  for (unsigned int i = FIRST_DDAC_FAULT_OBJ; i < NO_OF_DDAC_FAULT_OBJ; i++)
  {
    mpAlarmDelay[i]->InitAlarmDelay();
    mpAlarmDelay[i]->ResetFault();
    mpAlarmDelay[i]->ResetWarning();
    mAlarmDelayCheckFlag[i] = false;
  }
  mpDDAInstalled->SetValue(mpDosingPumpInstalled->GetValue() == true && (mpDosingPumpType->GetValue() == DOSING_PUMP_TYPE_DDA));
  mpDDARef->SetValue((U32)(10000.0 * mpSetDosingRef->GetValue()));  // 0.1l/h -> 1ml/h
  ReqTaskTime();                         // Assures task is run at startup
}
/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DDACtrl::RunSubTask()
{
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
  
  // set reference value
  if (mpSetDosingRef.IsUpdated())
  {
    mpDDARef->SetValue((U32)(10000.0 * mpSetDosingRef->GetValue()));  // 0.1l/h -> 1ml/h
    mpDosingRefAct->SetValue(mpSetDosingRef->GetValue());
  }

  if (mpSetH2SLevel.IsUpdated())
  {
    mpH2SLevelAct->SetValue(mpSetH2SLevel->GetValue());
  }

  if (mpDDAInstalled->GetValue())
  {
    if (mpSetH2SFault->GetValue() & 0x01)   //bit0 means the fault
    {
      mpAlarmDelay[DDA_FAULT_OBJ_H2S]->SetFault();
    }
    else
    {
      mpAlarmDelay[DDA_FAULT_OBJ_H2S]->ResetFault();
    }
  }
  else
  {
    mpAlarmDelay[DDA_FAULT_OBJ_H2S]->ResetFault();
  }

  //set AI measured value to the feed tank level, displayed in HMI
  if (mpMeasuredValue->IsAvailable())
  {
    mpDDADosingFeedTankLevel->SetValue(mpMeasuredValue->GetValue());
  }
  else
  {
    mpDDADosingFeedTankLevel->SetQuality(DP_NOT_AVAILABLE);
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
  mpSetDosingRef->Subscribe(this);
  mpDosingRefAct->Subscribe(this);
  mpSetH2SFault->Subscribe(this);
  mpDDARef->Subscribe(this);
  mpH2SLevelAct->Subscribe(this);
  mpSetH2SLevel->Subscribe(this);
  mpDDADosingFeedTankLevel->Subscribe(this);
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
  mpDosingRefAct.Update(pSubject);
  mpSetH2SFault.Update(pSubject);
  mpDDARef.Update(pSubject);
  mpH2SLevelAct.Update(pSubject);
  mpSetH2SLevel.Update(pSubject);
  mpDDADosingFeedTankLevel.Update(pSubject);

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
    case SP_DDAC_DOSING_REF_ACT:
      mpDosingRefAct.Attach(pSubject);
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
    case SP_DDAC_DOSING_FEED_TANK_LEVEL:
      mpDDADosingFeedTankLevel.Attach(pSubject);
      break;
    case SP_DDAC_MEASURED_VALUE_CHEMICAL_CONTAINER:
      mpMeasuredValue.Attach(pSubject);
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
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

