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
/* CLASS NAME       : IOPTCAlarmCtrl                                        */
/*                                                                          */
/* FILE NAME        : IOPTCAlarmCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 01-04-2008 dd-mm-yyyy                                 */
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
#include <IOPTCAlarmCtrl.h>

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
IOPTCAlarmCtrl::IOPTCAlarmCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpIOPtcAlarmDelay[i] = new AlarmDelay(this);
    mIOPtcAlarmDelayCheckFlag[i] = false;

    mpIOMoistureAlarmDelay[i] = new AlarmDelay(this);
    mIOMoistureAlarmDelayCheckFlag[i] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
IOPTCAlarmCtrl::~IOPTCAlarmCtrl()
{
  /* Delete objects for handling setting, clearing and delaying of alarm and warnings */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    delete(mpIOPtcAlarmDelay[i]);
    delete(mpIOMoistureAlarmDelay[i]);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IOPTCAlarmCtrl::InitSubTask()
{
  mPtcStatus = 0xFFFFFFFF;
  mRunRequestedFlag = true;

  for (int i = 0; i < NO_OF_PTC_INPUT_ALARM_FUNC; i++)
  {
    mPtcAlarmOn[i] = false;
  }

  /* Initialize AlarmDelay's */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpIOPtcAlarmDelay[i]->InitAlarmDelay();
    mpIOMoistureAlarmDelay[i]->InitAlarmDelay();

    mIOPtcAlarmDelayCheckFlag[i] = false;
    mIOMoistureAlarmDelayCheckFlag[i] = false;

    mPumpPtcAlarmOn[i] = false;
    mPumpMoistureAlarmOn[i] = false;
  }

  ReqTaskTime();                        // Assures running of task at start
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IOPTCAlarmCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  //check for alarm
  CheckForPtcAlarm();

  //Separage PTC and Moisture alarms for pumps
  PumpBasedPtcAlarmOn();

  //raise alarm
  RaisePtcAlarm();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void IOPTCAlarmCtrl::ConnectToSubjects()
{  
  for (int i = 0; i < MAX_NO_OF_IO351; i++)
    mpIO351PtcInStatusBits[i]->Subscribe(this);

  /* Redirect ConnectToSubjects to AlarmDelay's */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpIOPtcAlarmDelay[i]->ConnectToSubjects();
    mpIOMoistureAlarmDelay[i]->ConnectToSubjects();
    mpActualOperationMode[i]->Subscribe(this);
  }

  for (int i = 0; i < MAX_NO_OF_PTC_INPUTS; ++i)
  {
    mpPtcConfigDP[i]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void IOPTCAlarmCtrl::Update(Subject* pSubject)
{
  /* Updates from AlarmDelay's */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    if (pSubject == mpIOPtcAlarmDelay[i])
    {
      mIOPtcAlarmDelayCheckFlag[i] = true;
    }
    if (pSubject == mpIOMoistureAlarmDelay[i])
    {
      mIOMoistureAlarmDelayCheckFlag[i] = true;
    }
  }

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
void IOPTCAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  /*Redirect cancelling of subscribtions to AlarmDelay's */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpIOPtcAlarmDelay[i]->SubscribtionCancelled(pSubject);
    mpIOMoistureAlarmDelay[i]->SubscribtionCancelled(pSubject);
  }

  for (int i = 0; i < MAX_NO_OF_PTC_INPUTS; ++i)
  {
    mpPtcConfigDP[i].Detach(pSubject);
  }
  for (int i = 0; i < MAX_NO_OF_PUMPS; ++i)
  {
    mpActualOperationMode[i].Detach(pSubject);
  }
  for (int i = 0; i < MAX_NO_OF_IO351; ++i)
  {
    mpIO351PtcInStatusBits[i].Detach(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void IOPTCAlarmCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_IOPAC_IO351_IO_MODULE_1_PTC_IN_STATUS_BITS:
      mpIO351PtcInStatusBits[0].Attach(pSubject);
      break;
    case SP_IOPAC_IO351_IO_MODULE_2_PTC_IN_STATUS_BITS:
      mpIO351PtcInStatusBits[1].Attach(pSubject);
      break;
    case SP_IOPAC_IO351_IO_MODULE_3_PTC_IN_STATUS_BITS:
      mpIO351PtcInStatusBits[2].Attach(pSubject);
      break;

    case SP_IOPAC_ACTUAL_OPERATION_MODE_PUMP_1:
      mpActualOperationMode[0].Attach(pSubject);
      break;
    case SP_IOPAC_ACTUAL_OPERATION_MODE_PUMP_2:
      mpActualOperationMode[1].Attach(pSubject);
      break;
    case SP_IOPAC_ACTUAL_OPERATION_MODE_PUMP_3:
      mpActualOperationMode[2].Attach(pSubject);
      break;
    case SP_IOPAC_ACTUAL_OPERATION_MODE_PUMP_4:
      mpActualOperationMode[3].Attach(pSubject);
      break;
    case SP_IOPAC_ACTUAL_OPERATION_MODE_PUMP_5:
      mpActualOperationMode[4].Attach(pSubject);
      break;
    case SP_IOPAC_ACTUAL_OPERATION_MODE_PUMP_6:
      mpActualOperationMode[5].Attach(pSubject);
      break;

    case SP_IOPAC_PTC_IN_1_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[0].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_2_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[1].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_3_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[2].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_4_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[3].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_5_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[4].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_6_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[5].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_7_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[6].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_8_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[7].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_9_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[8].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_10_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[9].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_11_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[10].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_12_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[11].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_13_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[12].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_14_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[13].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_15_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[14].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_16_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[15].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_17_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[16].Attach(pSubject);
      break;
    case SP_IOPAC_PTC_IN_18_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[17].Attach(pSubject);
      break;   

    case SP_IOPAC_ALARM_OBJ_PTC_PUMP_1:
      mpIOPtcAlarmDelay[0]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_PTC_PUMP_2:
      mpIOPtcAlarmDelay[1]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_PTC_PUMP_3:
      mpIOPtcAlarmDelay[2]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_PTC_PUMP_4:
      mpIOPtcAlarmDelay[3]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_PTC_PUMP_5:
      mpIOPtcAlarmDelay[4]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_PTC_PUMP_6:
      mpIOPtcAlarmDelay[5]->SetSubjectPointer(Id, pSubject);
      break;

    case SP_IOPAC_ALARM_OBJ_MOISTURE_PUMP_1:
      mpIOMoistureAlarmDelay[0]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_MOISTURE_PUMP_2:
      mpIOMoistureAlarmDelay[1]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_MOISTURE_PUMP_3:
      mpIOMoistureAlarmDelay[2]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_MOISTURE_PUMP_4:
      mpIOMoistureAlarmDelay[3]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_MOISTURE_PUMP_5:
      mpIOMoistureAlarmDelay[4]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_IOPAC_ALARM_OBJ_MOISTURE_PUMP_6:
      mpIOMoistureAlarmDelay[5]->SetSubjectPointer(Id, pSubject);
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
 * Function - CheckForPtcAlarm
 * DESCRIPTION: This function checks PTC terminals for alarm
 *****************************************************************************/
void IOPTCAlarmCtrl::CheckForPtcAlarm()
{
  mPtcStatus = 0;
  for (int i = 0; i < MAX_NO_OF_IO351; i++)
  {
    if (mpIO351PtcInStatusBits[i]->GetQuality() == DP_AVAILABLE)
    {
      mPtcStatus = ((mPtcStatus) ^ (mpIO351PtcInStatusBits[i]->GetValue() << (i * NO_OF_PTC_INPUTS_IO351)));
    }
    else
    {
      mPtcStatus = ((mPtcStatus) ^ (mpIO351PtcInStatusBits[i]->GetMaxValue() << (i * NO_OF_PTC_INPUTS_IO351)));
    }
  }

  for (int i = 0; i < NO_OF_PTC_INPUT_ALARM_FUNC; i++)
  {
    mPtcAlarmOn[i] = false;
  }

  for (int i = 0; i < MAX_NO_OF_PTC_INPUTS; i++ )
  {
    PTC_INPUT_FUNC_TYPE ptc_func = mpPtcConfigDP[i]->GetValue();
    bool ptc_status = mPtcStatus & (1 << i);
    if((ptc_status == false) && (ptc_func != PTC_INPUT_FUNC_NO_FUNCTION))
    {
      mPtcAlarmOn[ptc_func] = true;
    }
  }
}

/*****************************************************************************
 * Function - PumpBasedPtcAlarmOn
 * DESCRIPTION: This function separages PTC and Moisture alarms based on PUMPS.
 *****************************************************************************/
void IOPTCAlarmCtrl::PumpBasedPtcAlarmOn()
{
  int pump_ptc_alarm = 0;
  int pump_moisture_alarm = 0;

  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mPumpPtcAlarmOn[i] = false;
    mPumpMoistureAlarmOn[i] = false;
  }

  //derive PTC1 alarm
  for (int i = FIRST_PTC_1_INPUT_FUNC; i < LAST_PTC_1_INPUT_FUNC; i++)
  {
    mPumpPtcAlarmOn[pump_ptc_alarm] = mPtcAlarmOn[i];
	  pump_ptc_alarm++;
  }

  pump_ptc_alarm = 0;
  //derive PTC2 alarm (single alarm for both PTC1 and PTC2)
  for (int i = FIRST_PTC_2_INPUT_FUNC; i < LAST_PTC_2_INPUT_FUNC; i++)
  {
	  mPumpPtcAlarmOn[pump_ptc_alarm] = mPumpPtcAlarmOn[pump_ptc_alarm]|| mPtcAlarmOn[i];
	  pump_ptc_alarm++;
  }

  //derive Moisture alarm
  for (int i = FIRST_MOISTURE_INPUT_FUNC; i < LAST_MOISTURE_INPUT_FUNC; i++)
  {
    mPumpMoistureAlarmOn[pump_moisture_alarm] = mPtcAlarmOn[i];        
	  pump_moisture_alarm++;
  }
}

/*****************************************************************************
 * Function - RaisePtcAlarm
 * DESCRIPTION: This function creates PTC alarm
 *****************************************************************************/
void IOPTCAlarmCtrl::RaisePtcAlarm()
{
  /* Check if an AlarmDelay requests for attention - expired error timer. */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    if (mIOPtcAlarmDelayCheckFlag[i] == true)
    {
      mIOPtcAlarmDelayCheckFlag[i] = false;
      mpIOPtcAlarmDelay[i]->CheckErrorTimers();
    }
    if (mIOMoistureAlarmDelayCheckFlag[i] == true)
    {
      mIOMoistureAlarmDelayCheckFlag[i] = false;
      mpIOMoistureAlarmDelay[i]->CheckErrorTimers();
    }
  }

  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    if ((mpActualOperationMode[i]->GetValue() != ACTUAL_OPERATION_MODE_NOT_INSTALLED) &&
       (mpActualOperationMode[i]->GetValue() != ACTUAL_OPERATION_MODE_DISABLED))
    {
      if (mPumpPtcAlarmOn[i])
      {
        mpIOPtcAlarmDelay[i]->SetFault();
      }
      else
      {
        mpIOPtcAlarmDelay[i]->ResetFault();
      }
      
      if (mPumpMoistureAlarmOn[i])
      {
        mpIOMoistureAlarmDelay[i]->SetFault();
      }
      else
      {
        mpIOMoistureAlarmDelay[i]->ResetFault();
      }
    }
  }  

  /* Update the AlarmDataPoints with the changes made in AlarmDelay */
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpIOPtcAlarmDelay[i]->UpdateAlarmDataPoint();
    mpIOMoistureAlarmDelay[i]->UpdateAlarmDataPoint();
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
