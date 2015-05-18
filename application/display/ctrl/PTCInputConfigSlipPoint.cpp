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
/* CLASS NAME       : PTCInputConfigSlipPoint                               */
/*                                                                          */
/* FILE NAME        : PTCInputConfigSlipPoint.CPP                           */
/*                                                                          */
/* CREATED DATE     : 2012-02-21                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the PTC input configuration and the configuration        */
/* logic DataPoints into one virtual DataPoint for the display to look at.. */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Factory.h>
#include <DataPoint.h>
#include <DisplayController.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PTCInputConfigSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_PTC_CONFIG_SELECT_FUNC_ID 177

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
PTCInputConfigSlipPoint::PTCInputConfigSlipPoint()
{
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PTCInputConfigSlipPoint::~PTCInputConfigSlipPoint()
{

}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void PTCInputConfigSlipPoint::InitSubTask(void)
{
  if (mpCurrentPtcInputNumber.IsValid())
  {
    mpCurrentPtcInputNumber->SetAsInt(1);
  }

  UpdateVirtualPtcInputConfig();  
  UpdateUpperStatusLine();

  /* -----------------21-02-2012 15:58-----------------
   * The function call above causes an update of mpVirtualPtcInputConfig, which
   * this module (PTCInputConfigSlipPoint) subscribes at.
   * However the purpose of this call during the initialization is only to syncronize
   * the virtual and the actual PTCInputConfig. Thus no reaction due to the update
   * of mpVirtualPtcInputConfig is needed.
   * At this stage of the code we are only at initialization of the task - it might take
   * a long time before the RunSubTask is activated and during this time the
   * PTC inputs will be reconfigured. A reaction upon the mpVirtualPtcInputConfig
   * will cause the reconfiguration of PTC input 1 to be cancelled. Thus a reset of the
   * update is done here to prevent the reaction.  
   * --------------------------------------------------*/
  mpVirtualPtcInputConfig.ResetUpdated();  
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void PTCInputConfigSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if (mpCurrentPtcInputNumber.IsUpdated())
  {
    UpdateVirtualPtcInputConfig();
    UpdateUpperStatusLine();
  }

  if (mpVirtualPtcInputConfig.IsUpdated())
  {
    UpdateCurrentPtcInputConfig();
  }

  {
    const int index = mpCurrentPtcInputNumber->GetAsInt() - 1;

    if ((index >= 0) && (index < MAX_NO_OF_PTC_INPUTS))
    {
      UpdateVirtualPtcInputConfig();    
    }
    else
    {
      FatalErrorOccured("PTCICSP1 index out of range!");
    }
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void PTCInputConfigSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = false;
  mpCurrentPtcInputNumber.Detach(pSubject);
  mpVirtualPtcInputConfig.Detach(pSubject);  

  for (int i = 0; i < MAX_NO_OF_PTC_INPUTS; ++i)
  {
    mpPtcConfigDP[i].Detach(pSubject);
  }

}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void PTCInputConfigSlipPoint::Update(Subject* pSubject)
{
  if (!mCurrentlyUpdating)
  {
    if (mpVirtualPtcInputConfig.Update(pSubject))
    {
      // nop
    }
    else if (mpCurrentPtcInputNumber.Update(pSubject))
    {
      // nop
    }
    else
    {
      for (int i = 0; i < MAX_NO_OF_PTC_INPUTS; ++i)
      {
        if (mpPtcConfigDP[i].Update(pSubject))
        {
          break;
        }
      }
    }

    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void PTCInputConfigSlipPoint::SetSubjectPointer(int Id,Subject* pSubject)
{
  switch (Id)
  {
    case SP_PTCICSP_PTC_IN_1_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[0].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_2_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[1].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_3_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[2].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_4_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[3].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_5_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[4].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_6_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[5].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_7_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[6].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_8_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[7].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_9_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[8].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_10_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[9].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_11_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[10].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_12_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[11].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_13_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[12].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_14_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[13].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_15_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[14].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_16_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[15].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_17_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[16].Attach(pSubject);
      break;
    case SP_PTCICSP_PTC_IN_18_CONF_PTC_INPUT_FUNC:
      mpPtcConfigDP[17].Attach(pSubject);
      break;    

    case SP_PTCICSP_CURRENT_NO:
      mpCurrentPtcInputNumber.Attach(pSubject);
      break;
    case SP_PTCICSP_VIRTUAL_CONF_PTC_INPUT_FUNC:
      mpVirtualPtcInputConfig.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void PTCInputConfigSlipPoint::ConnectToSubjects(void)
{
  for (int i = 0; i < MAX_NO_OF_PTC_INPUTS; ++i)
  {
    mpPtcConfigDP[i]->Subscribe(this);
  }
  mpCurrentPtcInputNumber->Subscribe(this);
  mpVirtualPtcInputConfig->Subscribe(this);
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

void PTCInputConfigSlipPoint::UpdateVirtualPtcInputConfig()
{
  const int index = (mpCurrentPtcInputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < MAX_NO_OF_PTC_INPUTS))
  {
    mpVirtualPtcInputConfig->SetValue(mpPtcConfigDP[index]->GetValue());
  }
  else
  {
    FatalErrorOccured("PTCICSP3 index out of range!");
  }
}

void PTCInputConfigSlipPoint::UpdateCurrentPtcInputConfig()
{
  const int index = (mpCurrentPtcInputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < MAX_NO_OF_PTC_INPUTS))
  {
    mpPtcConfigDP[index]->SetValue(mpVirtualPtcInputConfig->GetValue());
  }
  else
  {
    FatalErrorOccured("PTCICSP4 index out of range!");
  }
}

void PTCInputConfigSlipPoint::UpdateUpperStatusLine()
{
    Display* p_display = NULL;
    char display_number[10];

    int index = mpCurrentPtcInputNumber->GetAsInt();

    sprintf(display_number, "4.4.7.%i", index);

    p_display = GetDisplay(DISPLAY_PTC_CONFIG_SELECT_FUNC_ID);
    p_display->SetDisplayNumber(display_number);

    DisplayController::GetInstance()->RequestTitleUpdate();
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
