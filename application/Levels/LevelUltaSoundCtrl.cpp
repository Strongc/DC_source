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
/* CLASS NAME       : LevelUltaSoundCtrl                                    */
/*                                                                          */
/* FILE NAME        : LevelUltaSoundCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 01-10-2007 dd-mm-yyyy                                 */
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
#include <LevelUltaSoundCtrl.h>         // class implemented

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
LevelUltaSoundCtrl::LevelUltaSoundCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
LevelUltaSoundCtrl::~LevelUltaSoundCtrl()
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void LevelUltaSoundCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if (mpMeasuredValueUltraSoundDepth->GetQuality() != DP_AVAILABLE)
  {
    mpWaterLevelUltraSound->SetQuality( mpMeasuredValueUltraSoundDepth->GetQuality() );
  }
  else if (mpInverse->GetValue() == true)
  {
    mpWaterLevelUltraSound->SetValue( mpPitDepth->GetValue() + mpOffset->GetValue() - mpMeasuredValueUltraSoundDepth->GetValue() );
  }
  else
  {
    mpWaterLevelUltraSound->SetValue( mpMeasuredValueUltraSoundDepth->GetValue() );
  }

}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *
 *****************************************************************************/
void LevelUltaSoundCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_LUSC_INVERSE:
      mpInverse.Attach(pSubject);
      break;
    case SP_LUSC_OFFSET:
      mpOffset.Attach(pSubject);
      break;
    case SP_LUSC_PIT_DEPTH:
      mpPitDepth.Attach(pSubject);
      break;
    case SP_LUSC_MEASURED_VALUE_ULTRA_SOUND_DEPTH:
      mpMeasuredValueUltraSoundDepth.Attach(pSubject);
      break;
    case SP_LUSC_WATER_LEVEL_ULTRA_SOUND:
      mpWaterLevelUltraSound.Attach(pSubject);
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
void LevelUltaSoundCtrl::Update(Subject* pSubject)
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
void LevelUltaSoundCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelUltaSoundCtrl::ConnectToSubjects()
{
  mpInverse->Subscribe(this);
  mpOffset->Subscribe(this);
  mpMeasuredValueUltraSoundDepth->Subscribe(this);
  mpPitDepth->Subscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelUltaSoundCtrl::InitSubTask()
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

