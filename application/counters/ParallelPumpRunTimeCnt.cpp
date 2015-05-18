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
/* CLASS NAME       : ParallelPumpRunTimeCnt                                */
/*                                                                          */
/* FILE NAME        : ParallelPumpRunTimeCnt.cpp                            */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
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
#include <ParallelPumpRunTimeCnt.h>

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
ParallelPumpRunTimeCnt::ParallelPumpRunTimeCnt()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ParallelPumpRunTimeCnt::~ParallelPumpRunTimeCnt()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ParallelPumpRunTimeCnt::InitSubTask()
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ParallelPumpRunTimeCnt::RunSubTask()
{
  int no = mpNoOfRunningPumps->GetValue();

  if (no > 1)
  {
    if (mpParallelPumpRunTime->GetValue() != mpParallelPumpRunTime->GetMaxValue())
    {
      mpParallelPumpRunTime->SetValue(mpParallelPumpRunTime->GetValue() + 1);
    }
  }

  if (no >= 0 && no <= 6)
  {
    if (mpPumpRunningTime[no]->GetValue() != mpPumpRunningTime[no]->GetMaxValue())
    {
      mpPumpRunningTime[no]->SetValue(mpPumpRunningTime[no]->GetValue() + 1);
    }
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ParallelPumpRunTimeCnt::ConnectToSubjects()
{
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void ParallelPumpRunTimeCnt::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void ParallelPumpRunTimeCnt::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ParallelPumpRunTimeCnt::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_PPRTC_NO_OF_RUNNING_PUMPS:
      mpNoOfRunningPumps.Attach(pSubject);
      break;
    case SP_PPRTC_PARALLEL_PUMP_RUN_TIME:
      mpParallelPumpRunTime.Attach(pSubject);
      break;
    case SP_PPRTC_0_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[0].Attach(pSubject);
      break;
    case SP_PPRTC_1_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[1].Attach(pSubject);
      break;
    case SP_PPRTC_2_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[2].Attach(pSubject);
      break;
    case SP_PPRTC_3_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[3].Attach(pSubject);
      break;
    case SP_PPRTC_4_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[4].Attach(pSubject);
      break;
    case SP_PPRTC_5_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[5].Attach(pSubject);
      break;
    case SP_PPRTC_6_PUMPS_RUN_RUN_TIME:
      mpPumpRunningTime[6].Attach(pSubject);
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
