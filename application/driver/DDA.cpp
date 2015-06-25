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

/*****************************************************************************
  DEFINES
 *****************************************************************************/
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


/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
DDA::DDA()
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
  mpSystemAlarmResetEvent.Update(pSubject);
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    if (pSubject == mpDDAAlarmDelay[i])
    {
      mDDAAlarmDelayCheckFlag[i] = true;
      break;
    }
  }
}


void DDA::ConnectToSubjects()
{
  mpDDARef->Subscribe(this);
  mpDDAInstalled->Subscribe(this);
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
  ALARM_ID_TYPE new_alarm_code;
  U32 new_warning_code;

  // NOTICE: Send the reference as fast as possible.
  // There is no need to check if the DDA is installed as the reference is not sent directly
  // to the DDA. It is put into the class 5 table and here we must always have the newest reference - no
  // matter if the DDA is present or not.
  if (mpDDARef.IsUpdated())
  {
    mpGeniSlaveIf->SetDDAReference(DDA_NO_1, mpDDARef->GetValue());
  }

  // Service AlarmDelays
  for (unsigned int i = FIRST_DDA_FAULT_OBJ; i < NO_OF_DDA_FAULT_OBJ; i++)
  {
    if (mDDAAlarmDelayCheckFlag[i] == true)
    {
      mDDAAlarmDelayCheckFlag[i] = false;
      mpDDAAlarmDelay[i]->CheckErrorTimers();
    }
  }

  if (mpDDAInstalled.IsUpdated())
  {
    if (mpDDAInstalled->GetValue() == true)
    {
      mpGeniSlaveIf->ConnectDDA(DDA_NO_1);
      // Test
      mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->SetFault();
      mDDAAlarms[DDA_FAULT_OBJ_ALARM]->SetValue((ALARM_ID_TYPE)258);
      mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->SetFault();
    }
    else
    {
	    mpGeniSlaveIf->DisconnectDDA(DDA_NO_1);
      // Test
      mpDDAAlarmDelay[DDA_FAULT_OBJ_GENI_COMM]->ResetFault();
      mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->ResetFault();
    }
  }

  // module expected?
  if (mpDDAInstalled->GetValue() == true)
  {
    // check reset alarm
    //if (mpSystemAlarmResetEvent.IsUpdated() || mpModuleAlarmResetEvent.IsUpdated())
    if (mpSystemAlarmResetEvent.IsUpdated())
    {
      //if (mpMP204Installed->GetValue() == true)
      {
        mpGeniSlaveIf->DDA_AlarmReset(DDA_NO_1);
        mpDDAAlarmDelay[DDA_FAULT_OBJ_ALARM]->ResetFault();
      }
    }

    if (mpGeniSlaveIf->GetDDAAlarmCode(DDA_NO_1, &new_alarm_code) &&
        mpGeniSlaveIf->GetDDAWarningCode(DDA_NO_1, &new_warning_code))
    {
    }
  }
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

 
