/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : CounterInputConfCtrl                                  */
/*                                                                          */
/* FILE NAME        : CounterInputConfCtrl.CPP                              */
/*                                                                          */
/* CREATED DATE     : 2008-03-05                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class enforce rules of digital inputs dedicated for counters.       */
/* That is DI1 & DI2 of IO351, equal to DI index 4 & 5.                     */
/*                                                                          */
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
#include <Factory.h>
#include <DataPoint.h>
#include "CounterInputConfCtrl.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
namespace mpc
{
  namespace display
  {
    namespace ctrl
    {

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
CounterInputConfCtrl::CounterInputConfCtrl()
{
  mCurrentlyUpdating = true;  // if trying to do the updating - DO NOT do it !
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CounterInputConfCtrl::~CounterInputConfCtrl()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CounterInputConfCtrl::InitSubTask(void)
{
  mCurrentlyUpdating = false;                 // End of guarding the SubTask

  // use SetUpdated to make sure DPs are updated
  mDpVolumeCounterNotUsed.SetUpdated();
  mDpEnergyCounterNotUsed.SetUpdated();
  mDpUserDefineCounter1NotUsed.SetUpdated();
  mDpUserDefineCounter2NotUsed.SetUpdated();
  mDpUserDefineCounter3NotUsed.SetUpdated();
  mDpDi4Function.SetUpdated();
  mDpDi5Function.SetUpdated();
  mDpDi13Function.SetUpdated();
  mDpDi14Function.SetUpdated();
  mDpDi22Function.SetUpdated();
  mDpDi23Function.SetUpdated();
  ReqTaskTime();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CounterInputConfCtrl::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask


  // check if "not used" option has been selected for Volume Counter
  if (mDpVolumeCounterNotUsed.IsUpdated() && mDpVolumeCounterNotUsed->GetValue() )
  {
    // clear all Volume counter DI if set
    if (mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT )
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT )
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }

  // check if "not used" option has been selected for Energy Counter
  if (mDpEnergyCounterNotUsed.IsUpdated() && mDpEnergyCounterNotUsed->GetValue() )
  {
    // clear all Energy counter DI if set
    if (mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT )
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT )
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }

  // check if "not used" option has been selected for User Define Counter 1
  if (mDpUserDefineCounter1NotUsed.IsUpdated() && mDpUserDefineCounter1NotUsed->GetValue() )
  {
    // clear all Energy counter DI if set
    if (mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 )
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 )
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi13Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 )
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi14Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 )
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi22Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 )
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi23Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 )
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }

  // check if "not used" option has been selected for User Define Counter 2
  if (mDpUserDefineCounter2NotUsed.IsUpdated() && mDpUserDefineCounter2NotUsed->GetValue() )
  {
    // clear all Energy counter DI if set
    if (mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2 )
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2 )
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi13Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2 )
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi14Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2 )
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi22Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2 )
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi23Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2 )
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }

  // check if "not used" option has been selected for User Define Counter 3
  if (mDpUserDefineCounter3NotUsed.IsUpdated() && mDpUserDefineCounter3NotUsed->GetValue() )
  {
    // clear all Energy counter DI if set
    if (mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi13Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi14Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi22Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if (mDpDi23Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }

  // check if function has been set twice (and clear the first one if it is the case)
  if (mDpDi4Function.IsUpdated() )
  {
    if ( mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT
      && mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if ( mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT
      && mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    ClearFuncTwiceDi4(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    ClearFuncTwiceDi4(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    ClearFuncTwiceDi4(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
  }

  // check if function has been set twice (and clear the first one if it is the case)
  if (mDpDi5Function.IsUpdated() )
  {
    if ( mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT
      && mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if ( mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT
      && mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    ClearFuncTwiceDi5(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    ClearFuncTwiceDi5(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    ClearFuncTwiceDi5(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
  }

  // check if function has been set twice (and clear the first one if it is the case)
  if (mDpDi13Function.IsUpdated() )
  {
    ClearFuncTwiceDi13(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    ClearFuncTwiceDi13(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    ClearFuncTwiceDi13(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
  }

  // check if function has been set twice (and clear the first one if it is the case)
  if (mDpDi14Function.IsUpdated() )
  {
    ClearFuncTwiceDi14(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    ClearFuncTwiceDi14(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    ClearFuncTwiceDi14(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
  }

  // check if function has been set twice (and clear the first one if it is the case)
  if (mDpDi22Function.IsUpdated() )
  {
    ClearFuncTwiceDi22(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    ClearFuncTwiceDi22(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    ClearFuncTwiceDi22(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
  }

  // check if function has been set twice (and clear the first one if it is the case)
  if (mDpDi23Function.IsUpdated() )
  {
    ClearFuncTwiceDi23(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);
    ClearFuncTwiceDi23(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);
    ClearFuncTwiceDi23(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);
  }

  bool volume_counter_used = (
       mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT
    || mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_VOLUME_CNT);

  mDpVolumeCounterNotUsed->SetValue( !volume_counter_used );

  bool energy_counter_used = (
       mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT
    || mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_ENERGY_CNT);

  mDpEnergyCounterNotUsed->SetValue( !energy_counter_used );
  
  bool user_defined_counter_1_used = (
       mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
    || mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
    || mDpDi13Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
    || mDpDi14Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
    || mDpDi22Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
    || mDpDi23Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1);

  mDpUserDefineCounter1NotUsed->SetValue( !user_defined_counter_1_used );

  bool user_defined_counter_2_used = (
       mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
    || mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
    || mDpDi13Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
    || mDpDi14Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
    || mDpDi22Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
    || mDpDi23Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2);

  mDpUserDefineCounter2NotUsed->SetValue( !user_defined_counter_2_used );

  bool user_defined_counter_3_used = (
       mDpDi4Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3
    || mDpDi5Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3
    || mDpDi13Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3
    || mDpDi14Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3
    || mDpDi22Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3
    || mDpDi23Function->GetValue() == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3);

  mDpUserDefineCounter3NotUsed->SetValue( !user_defined_counter_3_used );

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CounterInputConfCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;	// stop reacting on updates
  
  mDpVolumeCounterNotUsed.Detach(pSubject);
  mDpEnergyCounterNotUsed.Detach(pSubject);
  mDpUserDefineCounter1NotUsed.Detach(pSubject);
  mDpUserDefineCounter2NotUsed.Detach(pSubject);
  mDpUserDefineCounter3NotUsed.Detach(pSubject);
  mDpDi4Function.Detach(pSubject);
  mDpDi5Function.Detach(pSubject);
  mDpDi13Function.Detach(pSubject);
  mDpDi14Function.Detach(pSubject);
  mDpDi22Function.Detach(pSubject);
  mDpDi23Function.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CounterInputConfCtrl::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    mDpVolumeCounterNotUsed.Update(pSubject);
    mDpEnergyCounterNotUsed.Update(pSubject);
    mDpUserDefineCounter1NotUsed.Update(pSubject);
    mDpUserDefineCounter2NotUsed.Update(pSubject);
    mDpUserDefineCounter3NotUsed.Update(pSubject);
    mDpDi4Function.Update(pSubject);
    mDpDi5Function.Update(pSubject);
    mDpDi13Function.Update(pSubject);
    mDpDi14Function.Update(pSubject);
    mDpDi22Function.Update(pSubject);
    mDpDi23Function.Update(pSubject);

    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CounterInputConfCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
    case SP_CICC_VOLUME_CNT_NOT_USED:
      mDpVolumeCounterNotUsed.Attach(pSubject);
      break;
    case SP_CICC_ENERGY_CNT_NOT_USED:
      mDpEnergyCounterNotUsed.Attach(pSubject);
      break;
    case SP_CICC_USER_DEFINED_CNT_1_NOT_USED:
      mDpUserDefineCounter1NotUsed.Attach(pSubject);
      break;
    case SP_CICC_USER_DEFINED_CNT_2_NOT_USED:
      mDpUserDefineCounter2NotUsed.Attach(pSubject);
      break;
    case SP_CICC_USER_DEFINED_CNT_3_NOT_USED:
      mDpUserDefineCounter3NotUsed.Attach(pSubject);
      break;
    case SP_CICC_DI4_FUNC:
      mDpDi4Function.Attach(pSubject);
      break;
    case SP_CICC_DI5_FUNC:
      mDpDi5Function.Attach(pSubject);
      break;
    case SP_CICC_DI13_FUNC:
      mDpDi13Function.Attach(pSubject);
      break;
    case SP_CICC_DI14_FUNC:
      mDpDi14Function.Attach(pSubject);
      break;
    case SP_CICC_DI22_FUNC:
      mDpDi22Function.Attach(pSubject);
      break;
    case SP_CICC_DI23_FUNC:
      mDpDi23Function.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CounterInputConfCtrl::ConnectToSubjects(void)
{
  mDpVolumeCounterNotUsed->Subscribe(this);
  mDpEnergyCounterNotUsed->Subscribe(this);
  mDpUserDefineCounter1NotUsed->Subscribe(this);
  mDpUserDefineCounter2NotUsed->Subscribe(this);
  mDpUserDefineCounter3NotUsed->Subscribe(this);
  mDpDi4Function->Subscribe(this);
  mDpDi5Function->Subscribe(this);
  mDpDi13Function->Subscribe(this);
  mDpDi14Function->Subscribe(this);
  mDpDi22Function->Subscribe(this);
  mDpDi23Function->Subscribe(this);
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void CounterInputConfCtrl::ClearFuncTwiceDi4(DIGITAL_INPUT_FUNC_TYPE diType)
{
  if ( mDpDi4Function->GetValue() == diType)
  {
    if(mDpDi5Function->GetValue() == diType)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi13Function->GetValue() == diType)
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi14Function->GetValue() == diType)
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi22Function->GetValue() == diType)
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi23Function->GetValue() == diType)
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }
}

void CounterInputConfCtrl::ClearFuncTwiceDi5(DIGITAL_INPUT_FUNC_TYPE diType)
{
  if ( mDpDi5Function->GetValue() == diType)
  {
    if(mDpDi4Function->GetValue() == diType)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi13Function->GetValue() == diType)
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi14Function->GetValue() == diType)
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi22Function->GetValue() == diType)
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi23Function->GetValue() == diType)
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }
}

void CounterInputConfCtrl::ClearFuncTwiceDi13(DIGITAL_INPUT_FUNC_TYPE diType)
{
  if ( mDpDi13Function->GetValue() == diType)
  {
    if(mDpDi4Function->GetValue() == diType)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi5Function->GetValue() == diType)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi14Function->GetValue() == diType)
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi22Function->GetValue() == diType)
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi23Function->GetValue() == diType)
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }
}

void CounterInputConfCtrl::ClearFuncTwiceDi14(DIGITAL_INPUT_FUNC_TYPE diType)
{
  if ( mDpDi14Function->GetValue() == diType)
  {
    if(mDpDi4Function->GetValue() == diType)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi5Function->GetValue() == diType)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi13Function->GetValue() == diType)
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi22Function->GetValue() == diType)
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi23Function->GetValue() == diType)
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }
}

void CounterInputConfCtrl::ClearFuncTwiceDi22(DIGITAL_INPUT_FUNC_TYPE diType)
{
  if ( mDpDi22Function->GetValue() == diType)
  {
    if(mDpDi4Function->GetValue() == diType)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi5Function->GetValue() == diType)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi13Function->GetValue() == diType)
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi14Function->GetValue() == diType)
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi23Function->GetValue() == diType)
      mDpDi23Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }
}

void CounterInputConfCtrl::ClearFuncTwiceDi23(DIGITAL_INPUT_FUNC_TYPE diType)
{
  if ( mDpDi23Function->GetValue() == diType)
  {
    if(mDpDi4Function->GetValue() == diType)
      mDpDi4Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi5Function->GetValue() == diType)
      mDpDi5Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi13Function->GetValue() == diType)
      mDpDi13Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi14Function->GetValue() == diType)
      mDpDi14Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );

    if(mDpDi22Function->GetValue() == diType)
      mDpDi22Function->SetValue( DIGITAL_INPUT_FUNC_NO_FUNCTION );
  }
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc
