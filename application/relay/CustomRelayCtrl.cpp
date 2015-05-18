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
/* CLASS NAME       : CustomRelayCtrl.                                      */
/*                                                                          */
/* FILE NAME        : CustomRelayCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 12-02-2009 dd-mm-yyyy                                 */
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
#include "CustomRelayCtrl.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
enum  // TYPES OF SW TIMERS
{
  DELAY
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
CustomRelayCtrl::CustomRelayCtrl()
{
}

 /*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CustomRelayCtrl::~CustomRelayCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void CustomRelayCtrl::InitSubTask()
{
  mpTimerObjList[DELAY] = new SwTimer(2, S, FALSE, FALSE, this);
  mTimeoutFlag = true;
  mPulseState = PULSE_DISABLED;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *****************************************************************************/
void CustomRelayCtrl::RunSubTask()
{
  if(mpDeActivateRelayCmd.IsUpdated())
  {
    mpCustomRelay->SetValue(false);
    mPulseState = PULSE_DISABLED;
  }
  if(mpActivateRelayCmd.IsUpdated())
  {
    mpCustomRelay->SetValue(true);
    mPulseState = PULSE_DISABLED;
  }
  if(mpPulseRelayCmd.IsUpdated())
  {
    if(mpCustomRelay->GetValue()==false)
    {
      mpCustomRelay->SetValue(true);
      mPulseState = PULSE_ON;
    }
    else
    {
      mPulseState = PULSE_OFF;
      mpCustomRelay->SetValue(false);
    }
    mpTimerObjList[DELAY]->StopSwTimer();
    mpTimerObjList[DELAY]->RetriggerSwTimer();
    mpTimerObjList[DELAY]->StartSwTimer();
  }

  if( mTimeoutFlag==true )
  {
    mTimeoutFlag = false;
    switch ( mPulseState )
    {
      case  PULSE_DISABLED:
        break;
      case  PULSE_ON:
        mpCustomRelay->SetValue(false);
        mPulseState = PULSE_DISABLED;
        break;
      case  PULSE_OFF:
        mpCustomRelay->SetValue(true);
        mPulseState = PULSE_ON;
        mpTimerObjList[DELAY]->StopSwTimer();
        mpTimerObjList[DELAY]->RetriggerSwTimer();
        mpTimerObjList[DELAY]->StartSwTimer();
        break;
    }
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *****************************************************************************/
void CustomRelayCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_CRC_ACTIVATE_CMD:
      mpActivateRelayCmd.Attach(pSubject);
      break;
    case SP_CRC_DEACTIVE_CMD:
      mpDeActivateRelayCmd.Attach(pSubject);
      break;
    case SP_CRC_PULSE_CMD:
      mpPulseRelayCmd.Attach(pSubject);
      break;
    case SP_CRC_CUSTOM_RELAY:
      mpCustomRelay.Attach(pSubject);
      break;
  }
}
 /*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *****************************************************************************/
void CustomRelayCtrl::ConnectToSubjects()
{
  mpActivateRelayCmd->Subscribe(this);
  mpDeActivateRelayCmd->Subscribe(this);
  mpPulseRelayCmd->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 *****************************************************************************/
void CustomRelayCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[DELAY])
  {
    mTimeoutFlag = true;
  }
  mpActivateRelayCmd.Update(pSubject);
  mpDeActivateRelayCmd.Update(pSubject);
  mpPulseRelayCmd.Update(pSubject);

  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *****************************************************************************/
void CustomRelayCtrl::SubscribtionCancelled(Subject* pSubject)
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
