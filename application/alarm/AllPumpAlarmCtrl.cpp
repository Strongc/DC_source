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
/* CLASS NAME       : AllPumpAlarmCtrl                                      */
/*                                                                          */
/* FILE NAME        : AllPumpAlarmCtrl.cpp                                  */
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
#include <AllPumpAlarmCtrl.h>

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
AllPumpAlarmCtrl::AllPumpAlarmCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AllPumpAlarmCtrl::~AllPumpAlarmCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AllPumpAlarmCtrl::InitSubTask()
{
  mReqTaskTimeFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AllPumpAlarmCtrl::RunSubTask()
{
  bool all_pumps_in_alarm = true;
  bool any_pump_in_alarm = false;
  bool pump_available = false;

  mReqTaskTimeFlag = false;

  for (int i=0; i<NO_OF_PUMPS; i++)
  {
    if (mpPumpAvailableFlag[i]->GetValue() == true)
    {
      pump_available = true;
      if (mpPumpAlarmFlag[i]->GetValue() == false)
      {
        // We have an available pump without alarm - then all pumps cannot be in alarm
        all_pumps_in_alarm = false;
      }
      else
      {
        // We have at least one available pump in alarm
        any_pump_in_alarm = true;
      }
    }
  }

  // We have one or more available pumps - and they are all in alarm
  mpAllPumpAlarmFlag->SetValue(all_pumps_in_alarm && pump_available);

  
  // We have at least one available pump in alarm
  mpAnyPumpAlarmFlag->SetValue(any_pump_in_alarm);
  
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void AllPumpAlarmCtrl::ConnectToSubjects()
{
  for (int i=0; i<NO_OF_PUMPS; i++)
  {
    mpPumpAlarmFlag[i]->Subscribe(this);
    mpPumpAvailableFlag[i]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void AllPumpAlarmCtrl::Update(Subject* pSubject)
{
  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AllPumpAlarmCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void AllPumpAlarmCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // input
    case SP_APAC_PUMP_1_ALARM_FLAG:
      mpPumpAlarmFlag[0].Attach(pSubject);
      break;
    case SP_APAC_PUMP_2_ALARM_FLAG:
      mpPumpAlarmFlag[1].Attach(pSubject);
      break;
    case SP_APAC_PUMP_3_ALARM_FLAG:
      mpPumpAlarmFlag[2].Attach(pSubject);
      break;
    case SP_APAC_PUMP_4_ALARM_FLAG:
      mpPumpAlarmFlag[3].Attach(pSubject);
      break;
    case SP_APAC_PUMP_5_ALARM_FLAG:
      mpPumpAlarmFlag[4].Attach(pSubject);
      break;
    case SP_APAC_PUMP_6_ALARM_FLAG:
      mpPumpAlarmFlag[5].Attach(pSubject);
      break;

    case SP_APAC_PUMP_1_AVAILABLE_FLAG:
      mpPumpAvailableFlag[0].Attach(pSubject);
      break;
    case SP_APAC_PUMP_2_AVAILABLE_FLAG:
      mpPumpAvailableFlag[1].Attach(pSubject);
      break;
    case SP_APAC_PUMP_3_AVAILABLE_FLAG:
      mpPumpAvailableFlag[2].Attach(pSubject);
      break;
    case SP_APAC_PUMP_4_AVAILABLE_FLAG:
      mpPumpAvailableFlag[3].Attach(pSubject);
      break;
    case SP_APAC_PUMP_5_AVAILABLE_FLAG:
      mpPumpAvailableFlag[4].Attach(pSubject);
      break;
    case SP_APAC_PUMP_6_AVAILABLE_FLAG:
      mpPumpAvailableFlag[5].Attach(pSubject);
      break;

    // output
    case SP_APAC_ALL_PUMP_ALARM_FLAG:
      mpAllPumpAlarmFlag.Attach(pSubject);
      break;
    case SP_APAC_ANY_PUMP_ALARM_FLAG:
      mpAnyPumpAlarmFlag.Attach(pSubject);
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
