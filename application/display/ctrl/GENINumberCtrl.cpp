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
/* CLASS NAME       : GENINumberCtrl                                        */
/*                                                                          */
/* FILE NAME        : GENINumberCtrl.CPP                                    */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the digital input configuration DataPoints into one      */
/* virtual DataPoint for the display to look at...                          */
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
#include "GENINumberCtrl.h"

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
GeniNumberCtrl::GeniNumberCtrl()
{
  mCurrentlyUpdating = true;  // if trying to do the updating - DO NOT do it !
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GeniNumberCtrl::~GeniNumberCtrl()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void GeniNumberCtrl::InitSubTask(void)
{
  mCurrentlyUpdating = false;                 // End of guarding the SubTask
  ReqTaskTime();
  mpGeniAddress.SetUpdated();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void GeniNumberCtrl::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if (mpGeniAddress.IsUpdated())
  {
    if ((mpGeniAddress->GetAsInt() < 32) || (mpGeniAddress->GetAsInt() > (64 + 32)))
    {
      mpGeniNumber->SetAsInt(1);
      mpGeniNumber->SetQuality(DP_NOT_AVAILABLE);
    }
    else
    {
      mpGeniNumber->SetAsInt(mpGeniAddress->GetAsInt() - 31);
      mpGeniNumber->SetQuality(DP_AVAILABLE);
    }
  }
  
  if (mpGeniNumber.IsUpdated())
  {
    mpGeniAddress->SetAsInt(mpGeniNumber->GetAsInt() + 31);
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void GeniNumberCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;	// stop reacting on updates
  mpGeniAddress.Detach(pSubject);
  mpGeniNumber.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void GeniNumberCtrl::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    mpGeniAddress.Update(pSubject);
    mpGeniNumber.Update(pSubject);
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void GeniNumberCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
    case SP_GENI_NUMBER_CTRL_ADDRESS:
      mpGeniAddress.Attach(pSubject);
      break;
    case SP_GENI_NUMBER_CTRL_NUMBER:
      mpGeniNumber.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void GeniNumberCtrl::ConnectToSubjects(void)
{
  mpGeniAddress->Subscribe(this);
  mpGeniNumber->Subscribe(this);
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
    } // namespace ctrl
  } // namespace display
} // namespace mpc
