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

typedef struct
{
  unsigned int event_code;
  ALARM_ID_TYPE   alarm_code;
} DDA_ALARM_CONVERSION_TYPE;

const DDA_ALARM_CONVERSION_TYPE alarm_table[] =
{
  {210, (ALARM_ID_TYPE)256},             // Over pressure
  {211, (ALARM_ID_TYPE)257},             // Mean pressure to low (Under pressure).
  {35,  (ALARM_ID_TYPE)258},             // Gas in pump head, deaerating problem
  {208, (ALARM_ID_TYPE)259},             // Cavitations
  {36,  (ALARM_ID_TYPE)260},             // Pressure valve leakage
  {37,  (ALARM_ID_TYPE)261},             // Suction valve leakage
  {38,  (ALARM_ID_TYPE)262},             // Venting valve defect
  {12,  (ALARM_ID_TYPE)263},             // Time for service is exceed
  {33,  (ALARM_ID_TYPE)264},             // Soon time for service
  {17,  (ALARM_ID_TYPE)265},             // Capacity too low (Perform. requirem. not met)
  {19,  (ALARM_ID_TYPE)266},             // Diaphragm break - dosing pump
  {51,  (ALARM_ID_TYPE)267},             // Blocked motor/pump
  {206, (ALARM_ID_TYPE)268},             // Pre empty tank
  {57,  (ALARM_ID_TYPE)269},             // Empty tank (Dry Running)
  {169, (ALARM_ID_TYPE)270},             // Cable breakdown on Flow Monitor (Flow sensor sig. fault)
  {47,  (ALARM_ID_TYPE)271}              // Cable breakdown on Analogue (Reference input sig. fault)
};
#define ALARM_TABLE_SIZE  (sizeof(alarm_table)/sizeof(alarm_table[0]))

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
int DDACtrl::counter;
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
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i] = new AlarmDelay(this);
    mDDAAlarmDelayCheckFlag[i] = false;
  }
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DDACtrl::~DDACtrl()
{
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    delete mpDDAAlarmDelay[i];
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
  mpDDALevelAct->SetValue(10);
  mpDDADosingFeedTankLevel->SetValue(20);
  mpDDAChemicalTotalDosed->SetValue(30);
  //mRunRequestedFlag = true;
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->InitAlarmDelay();
    mpDDAAlarmDelay[i]->ResetFault();
    mpDDAAlarmDelay[i]->ResetWarning();
    mDDAAlarmDelayCheckFlag[i] = false;
  }
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
  bool dda_ed = 0;
  U32 dda_h2s_level_act;
  U32 dda_dosing_feed_tank_level;
  U32 dda_chemical_total_dosed;

  dda_ed = mpDDAed->GetValue();
  dda_h2s_level_act = mpDDALevelAct->GetValue();
  dda_dosing_feed_tank_level = mpDDADosingFeedTankLevel->GetValue();
  dda_chemical_total_dosed = mpDDAChemicalTotalDosed->GetValue();

  //mpDosingPumpType->SetValue(DOSING_PUMP_TYPE_ANALOG);

  // Service AlarmDelays
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    if (mDDAAlarmDelayCheckFlag[i] == true)
    {
      mDDAAlarmDelayCheckFlag[i] = false;
      mpDDAAlarmDelay[i]->CheckErrorTimers();
    }
  }

  if (dda_ed)
  {
    mDDAAlarms[DDA_FAULT_OBJ]->SetValue((ALARM_ID_TYPE)258);
    mpDDAAlarmDelay[DDA_FAULT_OBJ]->SetFault();
  }
  else
  {
    mpDDAAlarmDelay[DDA_FAULT_OBJ]->ResetFault();
  }
  
  // Service AlarmDelays
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->UpdateAlarmDataPoint();
  }
}
/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void DDACtrl::ConnectToSubjects()
{
  mpDDAed->Subscribe(this);
  mpDDALevelAct->Subscribe(this);
  mpDDADosingFeedTankLevel->Subscribe(this);
  mpDDAChemicalTotalDosed->Subscribe(this);
  mpDosingPumpType->Subscribe(this);
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->ConnectToSubjects();
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
  mpDDAed.Update(pSubject);
  mpDDALevelAct.Update(pSubject);
  mpDDADosingFeedTankLevel.Update(pSubject);
  mpDDAChemicalTotalDosed.Update(pSubject);
  mpDosingPumpType.Update(pSubject);

  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    if (pSubject == mpDDAAlarmDelay[i])
    {
      mDDAAlarmDelayCheckFlag[i] = true;
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
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->SubscribtionCancelled(pSubject);
  }
}
/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void DDACtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_DDA_DOSING_PUMP_ENABLED:
      mpDDAed.Attach(pSubject);
      break;
    case SP_DDA_H2S_LEVEL_ACT:
      mpDDALevelAct.Attach(pSubject);
      break;
    case SP_DDA_DOSING_FEED_TANK_LEVEL:
      mpDDADosingFeedTankLevel.Attach(pSubject);
      break;
    case SP_DDA_CHEMICAL_TOTAL_DOSED:
      mpDDAChemicalTotalDosed.Attach(pSubject);
      break;
    case SP_DDA_DOSING_PUMP_TYPE:
      mpDosingPumpType.Attach(pSubject);
      break;
    case SP_DDA_SYS_ALARM_DDA_FAULT_ALARM_OBJ:
      mDDAAlarms[DDA_FAULT_OBJ].Attach(pSubject);
      mpDDAAlarmDelay[DDA_FAULT_OBJ]->SetSubjectPointer(id, pSubject);
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

