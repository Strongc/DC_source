/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : DDA                                           */
/*                                                                          */
/* FILE NAME        : DDAFuncHandler.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 10-08-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include "IO351.h"
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <GeniSlaveIf.h>
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <DDA.h>
#include <SwTimer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INIT_DELAY_TIME 200
#define RUN_DELAY_TIME 200

// SW Timers
enum
{
  DDA_INIT_TIMER,
  DDA_RUN_TIMER
};

typedef struct
{
  ALARM_ID_TYPE event_code;
  ALARM_ID_TYPE alarm_code;
} DDA_ALARM_CONVERSION_TYPE;

const DDA_ALARM_CONVERSION_TYPE alarm_table[] =
{ // DDA ALARM code       CU361 alarm code
  {(ALARM_ID_TYPE)210,    (ALARM_ID_TYPE)210},            // Over pressure
  {(ALARM_ID_TYPE)211,    (ALARM_ID_TYPE)211},            // Mean pressure to low (Under pressure).
  {(ALARM_ID_TYPE)35,     (ALARM_ID_TYPE)35},             // Gas in pump head, deaerating problem
  {(ALARM_ID_TYPE)208,    (ALARM_ID_TYPE)208},            // Cavitations
  {(ALARM_ID_TYPE)36,     (ALARM_ID_TYPE)36},             // Pressure valve leakage
  {(ALARM_ID_TYPE)37,     (ALARM_ID_TYPE)37},             // Suction valve leakage
  {(ALARM_ID_TYPE)38,     (ALARM_ID_TYPE)38},             // Venting valve defect
  {(ALARM_ID_TYPE)12,     (ALARM_ID_TYPE)12},             // Time for service is exceed
  {(ALARM_ID_TYPE)33,     (ALARM_ID_TYPE)33},             // Soon time for service
  {(ALARM_ID_TYPE)17,     (ALARM_ID_TYPE)17},             // Capacity too low (Perform. requirem. not met)
  {(ALARM_ID_TYPE)19,     (ALARM_ID_TYPE)19},             // Diaphragm break - dosing pump
  {(ALARM_ID_TYPE)51,     (ALARM_ID_TYPE)51},             // Blocked motor/pump
  {(ALARM_ID_TYPE)206,    (ALARM_ID_TYPE)206},            // Pre empty tank
  {(ALARM_ID_TYPE)57,     (ALARM_ID_TYPE)57},             // Empty tank (Dry Running)
  {(ALARM_ID_TYPE)169,    (ALARM_ID_TYPE)169},            // Cable breakdown on Flow Monitor (Flow sensor sig. fault)
  {(ALARM_ID_TYPE)47,     (ALARM_ID_TYPE)47}              // Cable breakdown on Analogue (Reference input sig. fault)
};
#define ALARM_TABLE_SIZE  (sizeof(alarm_table)/sizeof(alarm_table[0]))


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
//system mode define
#define SYSTEM_MODE_NORMAL 0
#define SYSTEM_MODE_NORMAL_WITH_WARNING 1
#define SYSTEM_MODE_ALARM 2

//operating mode define
#define REQUEST_START 0
#define REQUEST_STOP 1
#define CALIBRATING 2
#define SERVICE_POSITION 3


//control mode define
#define MANUEL_DOSING_MODE 0
#define PULSE_DOSING_MODE 1
#define ANALOGUE_DOSING_MODE 2
#define TIMER_DOSING_MODE 3
#define BATCH_DOSING_MODE 4

//pumping state define
#define NOT_PUMPING 0
#define PUMPING 1


/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
DDA::DDA(IO351_NO_TYPE moduleNo) : IO351(moduleNo)
{
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i] = new AlarmDelay(this);
    mDDAAlarmDelayCheckFlag[i] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
DDA::~DDA()
{
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    delete mpDDAAlarmDelay[i];
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *****************************************************************************/
void DDA::SetSubjectPointer(int id, Subject* pSubject)
{
   switch(id)
   {
     case SP_DDA_DDA_REFERENCE:
       mpDDARef.Attach(pSubject);
       break;
     case SP_DDA_DDA_INSTALLED:
       mpDDAInstalled.Attach(pSubject);
       break;
     case SP_DDA_OPERATION_MODE_DOSING_PUMP:
       mpOprModeDosingPump.Attach(pSubject);
       break;
     case SP_DDA_CHEMICAL_TOTAL_DOSED:
       mpChemicalTotalDosed.Attach(pSubject);
       break;
     case SP_DDA_RUNNING_DOSING_VOLUME:
       mpRunningDosingVolume.Attach(pSubject);
       break;
     case SP_DDA_DOSING_VOLUME_TOTAL_LOG:
       mpDosingVolumeTotalLog.Attach(pSubject);
       break;
     case SP_DDA_SYSTEM_ALARM_RESET_EVENT:
       mpSystemAlarmResetEvent.Attach(pSubject);
       break;
     //alarms
     case SP_DDA_DDA_GENI_COMM_FAULT_OBJ :
       mDDAAlarms[DDA_FAULT_OBJ_GENI_COMM].Attach(pSubject);
       mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->SetSubjectPointer(id, pSubject);
       break;
     case SP_DDA_SYS_ALARM_DDA_FAULT_ALARM_OBJ:
       mDDAAlarms[DDA_FAULT_OBJ_ALARM].Attach(pSubject);
       mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->SetSubjectPointer(id, pSubject);
       break;
     default:
       break;
   }
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void DDA::Update(Subject* pSubject)
{
  mpDDARef.Update(pSubject);
  mpDDAInstalled.Update(pSubject);
  mpChemicalTotalDosed.Update(pSubject);
  mpSystemAlarmResetEvent.Update(pSubject);
  if (pSubject == mpTimerObjList[DDA_INIT_TIMER])
  {
    mInitTimeOutFlag = true;
  } else if (pSubject == mpTimerObjList[DDA_RUN_TIMER])
  {
    mRunTimeOutFlag = true;
  }
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    if (pSubject == mpDDAAlarmDelay[i])
    {
      mDDAAlarmDelayCheckFlag[i] = true;
      break;
    }
  }
}


/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void DDA::ConnectToSubjects()
{
  mpDDARef->Subscribe(this);
  mpDDAInstalled->Subscribe(this);
  mpChemicalTotalDosed->Subscribe(this);
  mpSystemAlarmResetEvent->Subscribe(this);
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void DDA::SubscribtionCancelled(Subject* pSubject)
{
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->SubscribtionCancelled(pSubject);
  }
}


/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DDA::InitSubTask()
{
  mDDAStatus = DDA_INIT;
  mExistingAlarmCode = ALARM_ID_NO_ALARM;
  mExistingWarnings = 0;
  mMaxDosingCapacity = 0xFFFFFFFF;
  mpTimerObjList[DDA_INIT_TIMER] = new SwTimer(INIT_DELAY_TIME, MS, false, false, this);
  mpTimerObjList[DDA_RUN_TIMER] = new SwTimer(RUN_DELAY_TIME, MS, true, false, this);
  mInitTimeOutFlag = false;
  mRunTimeOutFlag = false;
  mDosingVolumeWhenStart = 0;
  mpDDAInstalled.SetUpdated();
  mpChemicalTotalDosed->SetValue(mpDosingVolumeTotalLog->GetValue()/1000.f);  //l->m3
  //SetDDADataAvailability(DP_NOT_AVAILABLE);

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
 ****************************************************************************/
void DDA::RunSubTask()
{
  ALARM_ID_TYPE new_alarm_code = ALARM_ID_NO_ALARM;
  U8 pStatus;

  // Service AlarmDelays
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    if (mDDAAlarmDelayCheckFlag[i] == true)
    {
      mDDAAlarmDelayCheckFlag[i] = false;
      mpDDAAlarmDelay[i]->CheckErrorTimers();
    }
  }

  // Reset alarms
  if (mpSystemAlarmResetEvent.IsUpdated())
  {
    if (mpDDAInstalled->GetValue() == true)
    {
      DDAInit();
      mDDAStatus = DDA_INIT;
      mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->ResetFault();
    }
  }

  if (mpDDAInstalled.IsUpdated())
  {
    if (mpDDAInstalled->GetValue() == true)
    {
      // Insert in auto poll list
      mpGeniSlaveIf->ConnectDDA(mModuleNo);
      mDDAStatus = DDA_INIT;
    }
    else
    {
      // Remove from auto poll list
	    mpGeniSlaveIf->DisconnectDDA(mModuleNo);

      // The module is not present - make certain that all errors and data from the module is cleared
      HandleDDAAlarm(ALARM_ID_NO_ALARM);
      //SetDDADataAvailability(DP_NOT_AVAILABLE);
      //request stop
      mpGeniSlaveIf->DDARequestStop(mModuleNo);
      mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->ResetFault();
    }
  }

  if (mpDDAInstalled->GetValue() == true)
  {
    if (mpGeniSlaveIf->GetDDAAlarmCode(mModuleNo, &new_alarm_code))
    {
      // communication OK?
      if (new_alarm_code == ALARM_ID_NO_ALARM)
      {
        mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->ResetFault();
        HandleDDAAlarm(new_alarm_code);
        RunDDA();
      }
      else
      {
        if ( new_alarm_code == ALARM_ID_COMMUNICATION_FAULT)
        {
          // clear any alarms and/or warnings caused by the DDA
          HandleDDAAlarm(ALARM_ID_NO_ALARM);
          mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->SetFault();
          mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_DISABLED);
        }
        else
        {
          mpGeniSlaveIf->GetDDASystemMode(mModuleNo, &pStatus);
          if (pStatus == SYSTEM_MODE_ALARM)
          {
            HandleDDAAlarm(new_alarm_code);
            mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->ResetFault();
            mDDAStatus = DDA_INIT;
            mpGeniSlaveIf->DDARequestStop(mModuleNo);
            mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_STOPPED);
          }
        }
      }
    }
    else
    {
      // we were unable to get the alarm code from GENI, set alarm and set data not available
      mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->SetFault();
      //SetDDADataAvailability(DP_NOT_AVAILABLE);
      mDDAStatus = DDA_INIT;
    }
  }

  UpdateDosingVolume();
  // Service AlarmDelays
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    mpDDAAlarmDelay[i]->UpdateAlarmDataPoint();
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
 * Function - RunDDA
 * DESCRIPTION:
 ****************************************************************************/
void DDA::RunDDA()
{
  switch (mDDAStatus)
  {
    case DDA_INIT:
      DDAInit();
      mpTimerObjList[DDA_INIT_TIMER]->SetSwTimerPeriod(INIT_DELAY_TIME, MS, false);
      mpTimerObjList[DDA_INIT_TIMER]->RetriggerSwTimer();
      mDDAStatus = DDA_INIT_WAITING;
      mInitTimeOutFlag = false;
      break;
    case DDA_INIT_WAITING:
      if (mInitTimeOutFlag)
      {
        mInitTimeOutFlag = false;
        mDDAStatus = CheckInitRespond() ? DDA_START : DDA_INIT;
      }
      break;

    case DDA_START:
      //mpGeniSlaveIf->SetDDAReference(mModuleNo, mpDDARef->GetValue());
      if (ValidateSetpoint())
      {
        mpGeniSlaveIf->DDARequestStart(mModuleNo);
        mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_STARTED);
        mpGeniSlaveIf->GetDDATotalVolume(mModuleNo, &mDosingVolumeWhenStart);
        mLastChemicalTotalDosed = mpChemicalTotalDosed->GetValue();
        mpTimerObjList[DDA_RUN_TIMER]->SetSwTimerPeriod(RUN_DELAY_TIME, MS, false);
        mpTimerObjList[DDA_RUN_TIMER]->RetriggerSwTimer();
        mRunTimeOutFlag = false;
        mDDAStatus = DDA_RUNNING;
      }
      break;

    case DDA_RUNNING:
      if (mRunTimeOutFlag)
      {
        HandleDDAMeasuredValues();
        mRunTimeOutFlag = false;
        mpTimerObjList[DDA_RUN_TIMER]->RetriggerSwTimer();
        if (ValidateSetpoint())
        {
          mDDAStatus = CheckRunRespond() ? DDA_RUNNING : DDA_INIT;
        }
        else
        {
          mDDAStatus = DDA_INIT;
        }
      }
      break;

    default:
      break;
  }
}

/*****************************************************************************
 * Function - HandleDDAAlarm
 * DESCRIPTION:
 ****************************************************************************/
void DDA::HandleDDAAlarm(ALARM_ID_TYPE alarm_code)
{
  if (mExistingAlarmCode != alarm_code)
  {
    if (alarm_code == ALARM_ID_NO_ALARM || mExistingAlarmCode != ALARM_ID_NO_ALARM)
    {
      // The existing alarm code is no longer reported from DDA - it can be cleared
      mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->ResetFault();
      mExistingAlarmCode = ALARM_ID_NO_ALARM;
    }
    else if (alarm_code != 255) // Use an 'else' here to ensure a bit of time between alarm Reset/Set
    {
      // Convert from DDA alarm code to CU361 alarm code
      int index = 0;
      ALARM_ID_TYPE cu361_alarm = ALARM_ID_OTHER;
      while (cu361_alarm == ALARM_ID_OTHER && index < ALARM_TABLE_SIZE)
      {
        if (alarm_code == alarm_table[index].event_code)
        {
          cu361_alarm = alarm_table[index].alarm_code;
        }
        index++;
      }
      mDDAAlarms[DDA_FAULT_OBJ_ALARM]->SetValue(cu361_alarm);
      mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->SetFault();
      mExistingAlarmCode = alarm_code;
    }
  }
}

/*****************************************************************************
 * Function - DDAInit
 * DESCRIPTION:
 ****************************************************************************/
void DDA::DDAInit()
{
  mpGeniSlaveIf->DDAReset(mModuleNo);
  mpGeniSlaveIf->DDAAlarmReset(mModuleNo);
  mpGeniSlaveIf->DDASetToUserMode(mModuleNo);
  mpGeniSlaveIf->DDASetToManualDosing(mModuleNo);
  mpGeniSlaveIf->DDARequestStop(mModuleNo);
  mpGeniSlaveIf->DDAPressStartKey(mModuleNo);
}

bool DDA::CheckInitRespond()
{
  U8 pStatus;
  bool retval = false;

  mpGeniSlaveIf->GetDDASystemMode(mModuleNo, &pStatus);
  retval = ((pStatus == SYSTEM_MODE_NORMAL) || (pStatus == SYSTEM_MODE_NORMAL_WITH_WARNING)) ? true : false;

  if (retval)
  {
    mpGeniSlaveIf->GetDDAOperatingMode(mModuleNo, &pStatus);
    retval = (pStatus == REQUEST_STOP) ? true : false;
  }

  if (retval)
  {
    mpGeniSlaveIf->GetDDAControlMode(mModuleNo, &pStatus);
    retval = (pStatus == MANUEL_DOSING_MODE) ? true : false;
  }
  return retval;
}

bool DDA::CheckRunRespond()
{
  bool pStatus;
  bool retval = false;

  mpGeniSlaveIf->GetDDAPumpingState(mModuleNo, &pStatus);
  retval = (pStatus == PUMPING) ? true : false;
  return retval;
}

bool DDA::ValidateSetpoint()
{
  U32 min_capacity = 26; //26 0.1ml/h
  bool retval = true;
  U32 set_point = 0;
  if (mMaxDosingCapacity == 0xFFFFFFFF)
  {
    mpGeniSlaveIf->GetDDAMaxDosingCap(mModuleNo, &mMaxDosingCapacity); // For DDA, it should be 75000(0.1ml/h)
  }

  set_point = mpDDARef->GetValue();

  if (set_point < min_capacity)
  {
    mpGeniSlaveIf->DDARequestStop(mModuleNo);
    mpOprModeDosingPump->SetValue(ACTUAL_OPERATION_MODE_STOPPED);
    retval = false;
  }
  else if (set_point > mMaxDosingCapacity)
  {
    set_point = mMaxDosingCapacity;
    retval = true;
  }
  mpGeniSlaveIf->SetDDAReference(mModuleNo, set_point);
  return retval;
}

void DDA::HandleDDAMeasuredValues()
{
  U32 total_dosing_volume_value;

  if (mpGeniSlaveIf->GetDDATotalVolume(mModuleNo, &total_dosing_volume_value))
  {
    U32 value = total_dosing_volume_value - mDosingVolumeWhenStart;
    mpChemicalTotalDosed->SetValue(value/1000000.0f + mLastChemicalTotalDosed);     //ml -> m3
    mpRunningDosingVolume->SetValue((U32)(mpChemicalTotalDosed->GetValue()*1000000));    //m3 -> ml
  }
  else
  {
    mpChemicalTotalDosed->SetQuality(DP_NOT_AVAILABLE);
  }
}

void DDA::SetDDADataAvailability(DP_QUALITY_TYPE quality)
{
  mpChemicalTotalDosed->SetQuality(quality);
}

void DDA::UpdateDosingVolume()
{
  mpDosingVolumeTotalLog->SetValue(1000.0f*mpChemicalTotalDosed->GetValue());  //m3->l
}
/*****************************************************************************
 * Function - ConfigReceived
 * DESCRIPTION: Empty function to satisfy virtual function in base class.
 ****************************************************************************/
void DDA::ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType)
{
}

