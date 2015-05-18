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
/* CLASS NAME       : AnalogSensorAlarmCtrl                                 */
/*                                                                          */
/* FILE NAME        : AnalogSensorAlarmCtrl.cpp                             */
/*                                                                          */
/* CREATED DATE     : 15-04-2008 dd-mm-yyyy                                 */
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
#include <AnalogSensorAlarmCtrl.h>

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
AnalogSensorAlarmCtrl::AnalogSensorAlarmCtrl()
{
  // Create objects for handling setting, clearing and delaying of alarm and warnings
  mpAlarmDelay = new AlarmDelay(this);
  mAlarmDelayCheckFlag = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AnalogSensorAlarmCtrl::~AnalogSensorAlarmCtrl()
{
  delete(mpAlarmDelay);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnalogSensorAlarmCtrl::InitSubTask()
{
  mpAlarmDelay->InitAlarmDelay();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnalogSensorAlarmCtrl::RunSubTask()
{
  // Pre alarm update
  if (mAlarmDelayCheckFlag == true)
  {
    mAlarmDelayCheckFlag = false;
    mpAlarmDelay->CheckErrorTimers();
  }

  // Evaluate alarm
  if (mpMeasuredValue->GetQuality() == DP_NOT_AVAILABLE)
  {
    mpAlarmDelay->SetFault();
  }
  else
  {
    mpAlarmDelay->ResetFault();
  }

  // Post alarm update
  mpAlarmDelay->UpdateAlarmDataPoint();
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AnalogSensorAlarmCtrl::ConnectToSubjects()
{
  mpAlarmDelay->ConnectToSubjects();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void AnalogSensorAlarmCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpAlarmDelay)
  {
    mAlarmDelayCheckFlag = true;
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnalogSensorAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpAlarmDelay->SubscribtionCancelled(pSubject);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void AnalogSensorAlarmCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Variable inputs:
    case SP_ASAC_MEASURED_VALUE:
      mpMeasuredValue.Attach(pSubject);
      break;

    // Alarms
    case SP_ASAC_SENSOR_ALARM_OBJ:
      mpAlarmDelay->SetSubjectPointer(id, pSubject);
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
