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
/* CLASS NAME       : CombiAlarmCtrl                                        */
/*                                                                          */
/* FILE NAME        : CombiAlarmCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 29-01-2008 dd-mm-yyyy                                 */
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
#include <CombiAlarmCtrl.h>

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
CombiAlarmCtrl::CombiAlarmCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    mpCombiAlarmDelay[fault_id] = new AlarmDelay(this);
    mCombiAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CombiAlarmCtrl::~CombiAlarmCtrl()
{
  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    delete(mpCombiAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CombiAlarmCtrl::InitSubTask()
{
  mpSource1AlarmFlag = mpNoAlarmSelectedFlag.GetSubject();
  mpSource2AlarmFlag = mpNoAlarmSelectedFlag.GetSubject();

  mpSource1.SetUpdated();
  mpSource2.SetUpdated();

  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    mpCombiAlarmDelay[fault_id]->InitAlarmDelay();
  }

  mRunRequestedFlag = true;
  ReqTaskTime();                         // Assures task is run at startup
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CombiAlarmCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    if (mCombiAlarmDelayCheckFlag[fault_id] == true)
    {
      mCombiAlarmDelayCheckFlag[fault_id] = false;
      mpCombiAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  if (mpSource1.IsUpdated())
  {
    SetSourcePointer(mpSource1.GetSubject(), &mpSource1AlarmFlag);
  }
  if (mpSource2.IsUpdated())
  {
    SetSourcePointer(mpSource2.GetSubject(), &mpSource2AlarmFlag);
  }

  // Check for combi alarm
  if (mpSource1AlarmFlag->GetValue() == true && mpSource2AlarmFlag->GetValue() == true)
  {
    // Both alarms defining the combi alarm is present - set the combi alarm
    mpCombiAlarmDelay[CAC_FAULT_OBJ_COMBI_ALARM]->SetFault();
  }
  else
  {
    // At least one of the alarms defining the combi alarm is not present - reset the combi alarm
    mpCombiAlarmDelay[CAC_FAULT_OBJ_COMBI_ALARM]->ResetFault();
  }

  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    mpCombiAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void CombiAlarmCtrl::ConnectToSubjects()
{
  mpSource1->Subscribe(this);
  mpSource2->Subscribe(this);

  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    mpCombiAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void CombiAlarmCtrl::Update(Subject* pSubject)
{
  mpSource1.Update(pSubject);
  mpSource2.Update(pSubject);

  if (pSubject == mpCombiAlarmDelay[CAC_FAULT_OBJ_COMBI_ALARM])
  {
    mCombiAlarmDelayCheckFlag[CAC_FAULT_OBJ_COMBI_ALARM] = true;
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
void CombiAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_CAC_FAULT_OBJ; fault_id < NO_OF_CAC_FAULT_OBJ; fault_id++)
  {
    mpCombiAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void CombiAlarmCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_CAC_SOURCE_1:
      mpSource1.Attach(pSubject);
      break;
    case SP_CAC_SOURCE_2:
      mpSource2.Attach(pSubject);
      break;
    case SP_CAC_PUMP_1_ALARM_FLAG:
      mpPumpAlarmFlag[PUMP_1].Attach(pSubject);
      break;
    case SP_CAC_PUMP_2_ALARM_FLAG:
      mpPumpAlarmFlag[PUMP_2].Attach(pSubject);
      break;
    case SP_CAC_PUMP_3_ALARM_FLAG:
      mpPumpAlarmFlag[PUMP_3].Attach(pSubject);
      break;
    case SP_CAC_PUMP_4_ALARM_FLAG:
      mpPumpAlarmFlag[PUMP_4].Attach(pSubject);
      break;
    case SP_CAC_PUMP_5_ALARM_FLAG:
      mpPumpAlarmFlag[PUMP_5].Attach(pSubject);
      break;
    case SP_CAC_PUMP_6_ALARM_FLAG:
      mpPumpAlarmFlag[PUMP_6].Attach(pSubject);
      break;
    case SP_CAC_ALL_PUMP_ALARM_FLAG:
      mpAllPumpAlarmFlag.Attach(pSubject);
      break;
    case SP_CAC_ANY_PUMP_ALARM_FLAG:
      mpAnyPumpAlarmFlag.Attach(pSubject);
      break;
    case SP_CAC_PUMP_1_GENI_ALARM_FLAG:
      mpPumpGeniIO111AlarmFlag[PUMP_1].Attach(pSubject);
      break;
    case SP_CAC_PUMP_2_GENI_ALARM_FLAG:
      mpPumpGeniIO111AlarmFlag[PUMP_2].Attach(pSubject);
      break;
    case SP_CAC_PUMP_3_GENI_ALARM_FLAG:
      mpPumpGeniIO111AlarmFlag[PUMP_3].Attach(pSubject);
      break;
    case SP_CAC_PUMP_4_GENI_ALARM_FLAG:
      mpPumpGeniIO111AlarmFlag[PUMP_4].Attach(pSubject);
      break;
    case SP_CAC_PUMP_5_GENI_ALARM_FLAG:
      mpPumpGeniIO111AlarmFlag[PUMP_5].Attach(pSubject);
      break;
    case SP_CAC_PUMP_6_GENI_ALARM_FLAG:
      mpPumpGeniIO111AlarmFlag[PUMP_6].Attach(pSubject);
      break;
    case SP_CAC_OVERFLOW_ALARM_FLAG:
      mpOverflowAlarmFlag.Attach(pSubject);
      break;
    case SP_CAC_LEVEL_ALARM_FLAG:
      mpLevelAlarmFlag.Attach(pSubject);
      break;
    case SP_CAC_HIGH_LEVEL_ALARM_FLAG:
      mpHighLevelAlarmFlag.Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_1_FLAG:
      mpUserFuncFlag[USER_IO_1].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_2_FLAG:
      mpUserFuncFlag[USER_IO_2].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_3_FLAG:
      mpUserFuncFlag[USER_IO_3].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_4_FLAG:
      mpUserFuncFlag[USER_IO_4].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_5_FLAG:
      mpUserFuncFlag[USER_IO_5].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_6_FLAG:
      mpUserFuncFlag[USER_IO_6].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_7_FLAG:
      mpUserFuncFlag[USER_IO_7].Attach(pSubject);
      break;
    case SP_CAC_USER_FUNC_8_FLAG:
      mpUserFuncFlag[USER_IO_8].Attach(pSubject);
      break;

    case SP_CAC_NO_ALARM_SELECTED_FLAG:
      mpNoAlarmSelectedFlag.Attach(pSubject);
      break;

    case SP_CAC_COMBI_ALARM_OBJ:
      mpCombiAlarmDelay[CAC_FAULT_OBJ_COMBI_ALARM]->SetSubjectPointer(Id, pSubject);
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
 * Function - SetSourcePointer
 * DESCRIPTION: Set the specified Source Alarm Obj pointer to the alarm object
 *              indicated by the specified source
 *
 *****************************************************************************/
void CombiAlarmCtrl::SetSourcePointer(EnumDataPoint<ALARM_TYPE>* p_source, BoolDataPoint** p_alarm)
{
  switch (p_source->GetValue())
  {
    case ALARM_HIGH_LEVEL:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpHighLevelAlarmFlag.GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_ALARM_LEVEL:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpLevelAlarmFlag.GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_ALL_PUMPS_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpAllPumpAlarmFlag.GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_ANY_PUMP_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpAnyPumpAlarmFlag.GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_OVERFLOW:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpOverflowAlarmFlag.GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_PUMP_1_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpAlarmFlag[PUMP_1].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_PUMP_2_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpAlarmFlag[PUMP_2].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_PUMP_3_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpAlarmFlag[PUMP_3].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_PUMP_4_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpAlarmFlag[PUMP_4].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_PUMP_5_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpAlarmFlag[PUMP_5].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_PUMP_6_IN_ALARM:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpAlarmFlag[PUMP_6].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_GENI_ERROR_PUMP_1:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpGeniIO111AlarmFlag[PUMP_1].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_GENI_ERROR_PUMP_2:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpGeniIO111AlarmFlag[PUMP_2].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_GENI_ERROR_PUMP_3:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpGeniIO111AlarmFlag[PUMP_3].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_GENI_ERROR_PUMP_4:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpGeniIO111AlarmFlag[PUMP_4].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_GENI_ERROR_PUMP_5:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpGeniIO111AlarmFlag[PUMP_5].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_GENI_ERROR_PUMP_6:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpPumpGeniIO111AlarmFlag[PUMP_6].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_1:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_1].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_2:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_2].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_3:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_3].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_4:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_4].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_5:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_5].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_6:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_6].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_7:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_7].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    case ALARM_USER_IO_8:
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpUserFuncFlag[USER_IO_8].GetSubject();
      (*p_alarm)->Subscribe(this);
      break;
    default:
      // We end here when ALARM_NOT_USED or if an unknown sourcecode is received
      (*p_alarm)->Unsubscribe(this);
      (*p_alarm) = mpNoAlarmSelectedFlag.GetSubject();
      // No need to subscribe to mpNoAlarmSelectedFlag as this alarm will never be active.
      break;
  }

}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
