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
/* CLASS NAME       : InfoFieldCtrl                                         */
/*                                                                          */
/* FILE NAME        : InfoFieldCtrl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 11-12-2006                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See .h-file                                     */
/*                                                                          */
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
#include <InfoFieldCtrl.h>
#include "GUI_VNC.h"

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
// SW Timers
enum
{
  POLL_VNC_CNT_TIMER
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
InfoFieldCtrl::InfoFieldCtrl()
{
  mReqTaskTimeFlag = false;
  mPollVNCCntFlag = true;

  mpDisplayInfoRemoteVncActive = 0;

  mpTimerObjList[POLL_VNC_CNT_TIMER] = new SwTimer(1, S, true, true, this);
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
InfoFieldCtrl::~InfoFieldCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 ****************************************************************************/
void InfoFieldCtrl::InitSubTask()
{
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 ****************************************************************************/
void InfoFieldCtrl::RunSubTask()
{
  mReqTaskTimeFlag = false;

  if (mPollVNCCntFlag)
  {
    mPollVNCCntFlag = false;
    if (GUI_VNC_GetNumConnections() > 0)
    {
      mpDisplayInfoRemoteVncActive->SetValue(true);
    }
    else
    {
      mpDisplayInfoRemoteVncActive->SetValue(false);
    }
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 ****************************************************************************/
void InfoFieldCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 ****************************************************************************/
void InfoFieldCtrl::Update(Subject* pSubject)
{

  if (pSubject == mpTimerObjList[POLL_VNC_CNT_TIMER])
  {
    mPollVNCCntFlag = true;
  }

  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Connect to Datapoints.
 *
 ****************************************************************************/
void InfoFieldCtrl::ConnectToSubjects()
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Sets subjects from Databasen
 *
 *****************************************************************************/
void InfoFieldCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

