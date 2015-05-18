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
/* CLASS NAME       : UpsFaultCtrl                                          */
/*                                                                          */
/* FILE NAME        : UpsFaultCtrl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 30-05-2008 dd-mm-yyyy                                 */
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
#include <UpsFaultCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

#define LIMIT_LOW_SHORT_CIRCUIT          -0.1   //  0,8 V --- Negative value MUST not have f-suffix due to compiler error in the Multi2000
#define LIMIT_HIGH_SHORT_CIRCUIT          0.2   //  0,8 V
#define LIMIT_HIGH_NO_BATTERY_CONNECTED   0.7   //  1,0 V
#define LIMIT_HIGH_BATTERY_CRITICAL       10.0f  //  3,2 V
#define LIMIT_HIGH_BATTERY_CHARGING       11.0f

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
UpsFaultCtrl::UpsFaultCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
UpsFaultCtrl::~UpsFaultCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void UpsFaultCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime();                         // Assures task is run at startup to set
                                         // the fault flag and battery status correct
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void UpsFaultCtrl::RunSubTask()
{
  bool ups_fault;
  float battery_voltage;
  BATTERY_STATE_TYPE battery_status;

  mRunRequestedFlag = false;

  if (mpBatteryBackupInstalled->GetValue() == true)
  {
    battery_voltage = mpBatteryVoltageMeasured->GetValue();
    battery_status = mpBatteryStatus->GetValue();
    ups_fault = mpUpsFaultFlag->GetValue();

    if (battery_voltage < LIMIT_LOW_SHORT_CIRCUIT)
    {
      battery_status = BATTERY_STATE_WRONG_POLARITY;
      ups_fault = true;
    }
    if (battery_voltage >= LIMIT_LOW_SHORT_CIRCUIT)
    {
      battery_status = BATTERY_STATE_SHORT_CIRCUIT;
      ups_fault = true;
    }
    if (battery_voltage >= LIMIT_HIGH_SHORT_CIRCUIT)
    {
      battery_status = BATTERY_STATE_BATTERY_MISSING;
      ups_fault = true;
    }
    if (battery_voltage > LIMIT_HIGH_NO_BATTERY_CONNECTED)
    {
      battery_status = BATTERY_STATE_BATTERY_DEFECT;
      ups_fault = true;
    }
    if (battery_voltage > LIMIT_HIGH_BATTERY_CRITICAL)
    {
      battery_status = BATTERY_STATE_LOW_BATTERY;
      ups_fault = false;
    }
    if (battery_voltage > LIMIT_HIGH_BATTERY_CHARGING)
    {
      battery_status = BATTERY_STATE_OK;
      ups_fault = false;
    }
    mpBatteryStatus->SetValue(battery_status);
    mpUpsFaultFlag->SetValue(ups_fault);
    if(ups_fault)
    {
      mpBatteryVoltageDisplayed->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      mpBatteryVoltageDisplayed->SetValue(battery_voltage);
    }

  }
  else
  {
    mpUpsFaultFlag->SetValue(false);
    mpBatteryVoltageDisplayed->SetQuality(DP_NEVER_AVAILABLE);
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void UpsFaultCtrl::ConnectToSubjects()
{
  mpBatteryBackupInstalled.Subscribe(this);
  mpBatteryVoltageMeasured.Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void UpsFaultCtrl::Update(Subject* pSubject)
{
  mpBatteryBackupInstalled.Update(pSubject);

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
void UpsFaultCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void UpsFaultCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch (Id)
  {
    case SP_UFC_BATTERY_BACKUP_INSTALLED:
      mpBatteryBackupInstalled.Attach(pSubject);
      break;
    case SP_UFC_BATTERY_VOLTAGE:
      mpBatteryVoltageMeasured.Attach(pSubject);
      break;
    case SP_UFC_BATTERY_VOLTAGE_ACT:
      mpBatteryVoltageDisplayed.Attach(pSubject);
      break;
    case SP_UFC_UPS_FAULT_FLAG:
      mpUpsFaultFlag.Attach(pSubject);
      break;
    case SP_UFC_BATTERY_STATUS:
      mpBatteryStatus.Attach(pSubject);
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
