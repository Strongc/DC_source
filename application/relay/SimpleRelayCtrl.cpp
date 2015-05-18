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
/* CLASS NAME       : SimpleRelayCtrl.                                      */
/*                                                                          */
/* FILE NAME        : SimpleRelayCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 03-11-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SwTimer.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "SimpleRelayCtrl.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  DELAY_BEFORE_SET_RELAY
};

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
SimpleRelayCtrl::SimpleRelayCtrl(const int setRelayInputValue) : mSetRelayInputValue(setRelayInputValue)
{
}
 
 /*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SimpleRelayCtrl::~SimpleRelayCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void SimpleRelayCtrl::InitSubTask()
{
  mpTimerObjList[DELAY_BEFORE_SET_RELAY] = new SwTimer(20, MS, FALSE, FALSE, this);
  mSetRelayFlag = false;
  mpInputValue.SetUpdated();
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *****************************************************************************/
void SimpleRelayCtrl::RunSubTask()
{
  const bool setRelay = mpInputValue->GetAsInt() == mSetRelayInputValue;
  
  if (mpInputValue.IsUpdated())
  {
    if (setRelay && !mpRelayStatus->GetValue())
    {
      // start delay timer if not started
      if (mpTimerObjList[DELAY_BEFORE_SET_RELAY]->GetSwTimerStatus() == false)
      {
        mpTimerObjList[DELAY_BEFORE_SET_RELAY]->StopSwTimer();
        mpTimerObjList[DELAY_BEFORE_SET_RELAY]->RetriggerSwTimer();
        mpTimerObjList[DELAY_BEFORE_SET_RELAY]->StartSwTimer();
      }
      else
      {
        // nothing timer is started
      }
    }
    else if (!setRelay)
    {
      mpTimerObjList[DELAY_BEFORE_SET_RELAY]->StopSwTimer();
      mSetRelayFlag = false;
      mpRelayStatus->SetValue(false);
    }
  }
  
  // TIME OUT
  if (mSetRelayFlag)
  {
    mSetRelayFlag = false;
    mpRelayStatus->SetValue(true);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *****************************************************************************/
void SimpleRelayCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
  case SP_SRC_INPUT_VALUE:
    mpInputValue.Attach(pSubject);
    break;
  case SP_SRC_RELAY_STATUS:
    mpRelayStatus.Attach(pSubject);
    break;
  }
}
 /*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *****************************************************************************/
void SimpleRelayCtrl::ConnectToSubjects()
{
  mpInputValue->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *****************************************************************************/
void SimpleRelayCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[DELAY_BEFORE_SET_RELAY])
  {
    mSetRelayFlag = true;
  }
  else
  {
    if (mpInputValue.Update(pSubject))
    {
      // nop
    } 
  }
  
  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *****************************************************************************/
void SimpleRelayCtrl::SubscribtionCancelled(Subject* pSubject)
{
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
