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
/* CLASS NAME       : ServiceModeCtrl                                       */
/*                                                                          */
/* FILE NAME        : ServiceModeCtrl.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 16-07-2012 dd-mm-yyyy                                 */
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
#include <ServiceModeCtrl.h>
#include <ctrl\ServiceModeEnabled.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
using namespace mpc::display::ctrl;

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
ServiceModeCtrl::ServiceModeCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ServiceModeCtrl::~ServiceModeCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ServiceModeCtrl::InitSubTask()
{
  mReqTaskTimeFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ServiceModeCtrl::RunSubTask()
{
  mReqTaskTimeFlag = false;
  ServiceModeEnabled* p_show_service_icon = ServiceModeEnabled::GetInstance();

  if (mpDIServiceMode->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
  {
    mpScadaCallBackEnabled->SetValue(false);
    mpServiceModeEnabled->SetValue(true);
    p_show_service_icon->SetValue(true);
  }
  else
  {
    mpScadaCallBackEnabled->SetValue(mpDisplayScadaCallBackEnabled->GetValue());
    mpServiceModeEnabled->SetValue(false);
    p_show_service_icon->SetValue(mpServiceLanguageEnabled->GetValue());
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ServiceModeCtrl::ConnectToSubjects()
{
  mpDisplayScadaCallBackEnabled->Subscribe(this);
  mpDIServiceMode->Subscribe(this);
  mpServiceLanguageEnabled->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void ServiceModeCtrl::Update(Subject* pSubject)
{
  mpDisplayScadaCallBackEnabled.Update(pSubject);
  mpDIServiceMode.Update(pSubject);

  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void ServiceModeCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void ServiceModeCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // inputs:
    case SP_SMC_DSIPLAY_SCADA_CALLBACK_ENABLED:
      mpDisplayScadaCallBackEnabled.Attach(pSubject);
      break;
    case SP_SMC_DIG_IN_FUNC_STATE_SERVICE_MODE:
      mpDIServiceMode.Attach(pSubject);
      break;
    case SP_SMC_SERVICE_LANGUAGE_ENABLED:
      mpServiceLanguageEnabled.Attach(pSubject);
      break;

    // Outputs:
     case SP_SMC_SCADA_CALLBACK_ENABLED:
      mpScadaCallBackEnabled.Attach(pSubject);
      break;
     case SP_SMC_SERVICE_MODE_ENABLED:
      mpServiceModeEnabled.Attach(pSubject);
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
