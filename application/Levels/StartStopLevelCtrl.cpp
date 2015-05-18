/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : StartStopLevelCtrl                                    */
/*                                                                          */
/* FILE NAME        : StartStopLevelCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 26-09-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <StartStopLevelCtrl.h>         // class implemented

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
 *****************************************************************************/
StartStopLevelCtrl::StartStopLevelCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
StartStopLevelCtrl::~StartStopLevelCtrl()
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void StartStopLevelCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  mpLowestStartLevel->CopyValues(mpStartLevel[PUMP_1].GetSubject());
  mpLowestStopLevel->CopyValues(mpStopLevel[PUMP_1].GetSubject());

  for (int pump_no = FIRST_PUMP_NO; pump_no < mpNumberOfPumps->GetValue(); pump_no++)
  {
    if (mpStartLevel[pump_no]->GetValue() < mpLowestStartLevel->GetValue())
    {
      mpLowestStartLevel->CopyValues(mpStartLevel[pump_no].GetSubject());
    }
    
    if (mpStopLevel[pump_no]->GetValue() < mpLowestStopLevel->GetValue())
    {
      mpLowestStopLevel->CopyValues(mpStopLevel[pump_no].GetSubject());
    }
  }

  
  float second_lowest = mpSecondLowestStartLevel->GetMaxAsFloat();
  bool second_lowest_found = false;

  for (int pump_no = FIRST_PUMP_NO; pump_no < mpNumberOfPumps->GetValue(); pump_no++)
  {
    if (mpStartLevel[pump_no]->GetValue() < second_lowest 
      && mpStartLevel[pump_no]->GetValue() > mpLowestStartLevel->GetValue())
    {
      second_lowest_found = true;
      second_lowest = mpStartLevel[pump_no]->GetValue();
      mpSecondLowestStartLevel->CopyValues(mpStartLevel[pump_no].GetSubject());
    }
  }

  if (!second_lowest_found)
  {
    //lowest start level and 2nd lowest start level must be identical
    mpSecondLowestStartLevel->CopyValues(mpLowestStartLevel.GetSubject());
  }

}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *
 *****************************************************************************/
void StartStopLevelCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_SSLC_START_LEVEL_1:
      mpStartLevel[PUMP_1].Attach(pSubject);
      break;
    case SP_SSLC_START_LEVEL_2:
      mpStartLevel[PUMP_2].Attach(pSubject);
      break;
    case SP_SSLC_START_LEVEL_3:
      mpStartLevel[PUMP_3].Attach(pSubject);
      break;
    case SP_SSLC_START_LEVEL_4:
      mpStartLevel[PUMP_4].Attach(pSubject);
      break;
    case SP_SSLC_START_LEVEL_5:
      mpStartLevel[PUMP_5].Attach(pSubject);
      break;
    case SP_SSLC_START_LEVEL_6:
      mpStartLevel[PUMP_6].Attach(pSubject);
      break;
    case SP_SSLC_STOP_LEVEL_1:
      mpStopLevel[PUMP_1].Attach(pSubject);
      break;
    case SP_SSLC_STOP_LEVEL_2:
      mpStopLevel[PUMP_2].Attach(pSubject);
      break;
    case SP_SSLC_STOP_LEVEL_3:
      mpStopLevel[PUMP_3].Attach(pSubject);
      break;
    case SP_SSLC_STOP_LEVEL_4:
      mpStopLevel[PUMP_4].Attach(pSubject);
      break;
    case SP_SSLC_STOP_LEVEL_5:
      mpStopLevel[PUMP_5].Attach(pSubject);
      break;
    case SP_SSLC_STOP_LEVEL_6:
      mpStopLevel[PUMP_6].Attach(pSubject);
      break;
    case SP_SSLC_LOWEST_STOP_LEVEL:
      mpLowestStopLevel.Attach(pSubject);
      break;
    case SP_SSLC_LOWEST_START_LEVEL:
      mpLowestStartLevel.Attach(pSubject);
      break;
    case SP_SSLC_SECOND_LOWEST_START_LEVEL:
      mpSecondLowestStartLevel.Attach(pSubject);
      break;
    case SP_SSLC_NUMBER_OF_PUMPS:
      mpNumberOfPumps.Attach(pSubject);
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
void StartStopLevelCtrl::Update(Subject* pSubject)
{
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
void StartStopLevelCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void StartStopLevelCtrl::ConnectToSubjects()
{
  for (int pump_no = FIRST_PUMP_NO; pump_no <= LAST_PUMP_NO; pump_no++)
  {
    mpStartLevel[pump_no]->Subscribe(this);
    mpStopLevel[pump_no]->Subscribe(this);
  }
  mpNumberOfPumps->Subscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void StartStopLevelCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

