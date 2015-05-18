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
/* CLASS NAME       : SystemRunTimeCnt                                      */
/*                                                                          */
/* FILE NAME        : SystemRunTimeCnt.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 21-11-2007 dd-mm-yyyy                                 */
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
#include <SystemRunTimeCnt.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  SYSTEM_RUN_TIMER
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
SystemRunTimeCnt::SystemRunTimeCnt()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SystemRunTimeCnt::~SystemRunTimeCnt()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SystemRunTimeCnt::InitSubTask()
{
  mpTimerObjList[SYSTEM_RUN_TIMER] = new SwTimer(1, S, true, true, this);
  mpPowerOnCnt->SetValue( mpPowerOnCnt->GetValue()+1 );
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SystemRunTimeCnt::RunSubTask()
{
  if (mpSystemRunTime->GetValue() != mpSystemRunTime->GetMaxValue())
  {
    mpSystemRunTime->SetValue(mpSystemRunTime->GetValue() + 1);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void SystemRunTimeCnt::ConnectToSubjects()
{
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void SystemRunTimeCnt::Update(Subject* pSubject)
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void SystemRunTimeCnt::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void SystemRunTimeCnt::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_SRTC_SYSTEM_RUN_TIME:
      mpSystemRunTime.Attach(pSubject);
      break;
    case SP_SRTC_POWER_ON_CNT:
      mpPowerOnCnt.Attach(pSubject);
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
