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
/* CLASS NAME       : ServiceLanguageCtrl                                   */
/*                                                                          */
/* FILE NAME        : ServiceLanguageCtrl.cpp                               */
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
#include "ServiceLanguageCtrl.h"
#include <Languages.h>
#include <MpcUnits.h>
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
ServiceLanguageCtrl::ServiceLanguageCtrl()
{
  mCurrentlyUpdating = true;  // if trying to do the updating - DO NOT do it !
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ServiceLanguageCtrl::~ServiceLanguageCtrl()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void ServiceLanguageCtrl::InitSubTask(void)
{
  mpServiceLanguageEnabled->SetValue(false);
  mCurrentlyUpdating = false;                 // End of guarding the SubTask
  ReqTaskTime();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void ServiceLanguageCtrl::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask
  if (mpServiceLanguageEnabled.IsUpdated())
  {
    if (mpServiceLanguageEnabled->GetAsBool())
    {
      Languages::GetInstance()->SetLanguage(UK_LANGUAGE);
      MpcUnits::GetInstance()->SetTempDefaultSiUnit();
    }
    else
    {
      Languages::GetInstance()->SetLanguage((LANGUAGE_TYPE)(mpCurrentLanguage->GetValue()));
      MpcUnits::GetInstance()->ReleaseTempDefaultSiUnit();
    }
  }
  else if (mpCurrentLanguage.IsUpdated())
  {
    // nop
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void ServiceLanguageCtrl::SubscribtionCancelled(Subject* pSubject)
{
  mpServiceLanguageEnabled.Detach(pSubject);
  mpCurrentLanguage.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void ServiceLanguageCtrl::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    if (mpCurrentLanguage.Update(pSubject))
    {
      // nop
    }
    else if (mpServiceLanguageEnabled.Update(pSubject))
    {
      // nop
    }
    
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void ServiceLanguageCtrl::SetSubjectPointer(int Id,Subject* pSubject)
{
  switch ( Id )
  {
    case SP_SERVICE_LC_LANGUAGE:
      mpCurrentLanguage.Attach(pSubject);
      break;
    case SP_SERVICE_LC_DISPLAY_SERVICE_MODE_ENABLED:
      mpServiceLanguageEnabled.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void ServiceLanguageCtrl::ConnectToSubjects(void)
{
  mpCurrentLanguage->Subscribe(this);
  mpServiceLanguageEnabled->Subscribe(this);
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
