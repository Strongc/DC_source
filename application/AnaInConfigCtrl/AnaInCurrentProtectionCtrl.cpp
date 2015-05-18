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
/* CLASS NAME       : AnaInCurrentProtectionCtrl                            */
/*                                                                          */
/* FILE NAME        : AnaInCurrentProtectionCtrl.cpp                        */
/*                                                                          */
/* CREATED DATE     : 17-02-2005 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "AnaInCurrentProtectionCtrl.h"             // class implemented

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
AnaInCurrentProtectionCtrl::AnaInCurrentProtectionCtrl()
{
}

/****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
AnaInCurrentProtectionCtrl::~AnaInCurrentProtectionCtrl()
{
}

/*****************************************************************************
 * Function - InitSubtask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInCurrentProtectionCtrl::InitSubTask()
{
  mReqTaskTime = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void AnaInCurrentProtectionCtrl::RunSubTask()
{
  mReqTaskTime = false;

  if (mpConfMeasuredValue->GetValue() == MEASURED_VALUE_LEVEL_ULTRA_SOUND)
  {
    mpCurrentProtectionAllowed->SetValue(false);
  }
  else
  {
    mpCurrentProtectionAllowed->SetValue(true);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void AnaInCurrentProtectionCtrl::Update(Subject* pSubject)
{
  if (!mReqTaskTime)
  {
    mReqTaskTime = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION : Set pointers to sub.
 *
 *****************************************************************************/
void AnaInCurrentProtectionCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_AICPC_MEASURED_VALUE:
      mpConfMeasuredValue.Attach(pSubject);
      break;
    case SP_AICPC_CURRENT_PROTECTION_ALLOWED:
      mpCurrentProtectionAllowed.Attach(pSubject);
      break;

    default:
      break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInCurrentProtectionCtrl::ConnectToSubjects()
{
  mpConfMeasuredValue->Subscribe(this);
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaInCurrentProtectionCtrl::SubscribtionCancelled(Subject* pSubject)
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
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
