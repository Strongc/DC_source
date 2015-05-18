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
/* CLASS NAME       : DigitalInputConfigSlipPoint                           */
/*                                                                          */
/* FILE NAME        : DigitalInputConfigSlipPoint.CPP                       */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the digital input configuration and the configuration    */
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
#include "DigitalInputConfigSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_DI_CONFIG_SELECT_FUNC_ID 31

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
DigitalInputConfigSlipPoint::DigitalInputConfigSlipPoint()
{
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DigitalInputConfigSlipPoint::~DigitalInputConfigSlipPoint()
{

}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalInputConfigSlipPoint::InitSubTask(void)
{
  if (mpCurrentDigitalInputNumber.IsValid())
  {
    mpCurrentDigitalInputNumber->SetAsInt(2);
  }

  UpdateVirtualDigitalInputConfig();
  UpdateVirtualDigitalInputConfLogic();
  UpdateUpperStatusLine();

  /* -----------------13-09-2007 15:58-----------------
   * The function call above causes an update of mpVirtualDigitalInputConfig, which
   * this module (DigitalInputConfigSlipPoint) subscribes at.
   * However the purpose of this call during the initialization is only to syncronize
   * the virtual and the actual DigitalInputConfig. Thus no reaction due to the update
   * of mpVirtualDigitalInputConfig is needed.
   * At this stage of the code we are only at initialization of the task - it might take
   * a long time before the RunSubTask is activated and during this time the
   * Digital inputs will be reconfigured. A reaction upon the mpVirtualDigitalInputConfig
   * will cause the reconfiguration of digital input 2 to be cancelled. Thus a reset of the
   * update is done here to prevent the reaction.
   * Same reasoning is valid for mpVirtualDigitalInputConfLogic.
   * --------------------------------------------------*/
  mpVirtualDigitalInputConfig.ResetUpdated();
  mpVirtualDigitalInputConfLogic.ResetUpdated();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalInputConfigSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if (mpCurrentDigitalInputNumber.IsUpdated())
  {
    UpdateVirtualDigitalInputConfig();
    UpdateVirtualDigitalInputConfLogic();
    UpdateUpperStatusLine();
  }

  if (mpVirtualDigitalInputConfig.IsUpdated())
  {
    UpdateCurrentDigitalInputConfig();
  }

  if (mpVirtualDigitalInputConfLogic.IsUpdated())
  {
    UpdateCurrentDigitalInputConfLogic();
  }

  {
    const int index = mpCurrentDigitalInputNumber->GetAsInt() - 1;

    if ((index >= 0) && (index < MAX_NO_OF_DIG_INPUTS))
    {
      UpdateVirtualDigitalInputConfig();
      UpdateVirtualDigitalInputConfLogic();
    }
    else
    {
      FatalErrorOccured("DICSP1 index out of range!");
    }
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalInputConfigSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = false;
  mpCurrentDigitalInputNumber.Detach(pSubject);
  mpVirtualDigitalInputConfig.Detach(pSubject);
  mpVirtualDigitalInputConfLogic.Detach(pSubject);

  for (int i = 0; i < MAX_NO_OF_DIG_INPUTS; ++i )
  {
    mpDigitalConfigDP[i].Detach(pSubject);
    mpDigitalConfLogicDP[i].Detach(pSubject);
  }

}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalInputConfigSlipPoint::Update(Subject* pSubject)
{
  if (!mCurrentlyUpdating)
  {
    if (mpVirtualDigitalInputConfig.Update(pSubject))
    {
      // nop
    }
    else if (mpVirtualDigitalInputConfLogic.Update(pSubject))
    {
      // nop
    }
    else if (mpCurrentDigitalInputNumber.Update(pSubject))
    {
      // nop
    }
    else
    {
      for (int i = 0; i < MAX_NO_OF_DIG_INPUTS; ++i )
      {
        if (mpDigitalConfigDP[i].Update(pSubject))
        {
          break;
        }
        if (mpDigitalConfLogicDP[i].Update(pSubject))
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
void DigitalInputConfigSlipPoint::SetSubjectPointer(int Id,Subject* pSubject)
{
  switch ( Id )
  {
    case SP_DICSP_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[0].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_2_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[1].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_3_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[2].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_4_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[3].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_5_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[4].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_6_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[5].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_7_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[6].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_8_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[7].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_9_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[8].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_10_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[9].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_11_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[10].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_12_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[11].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_13_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[12].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_14_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[13].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_15_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[14].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_16_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[15].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_17_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[16].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_18_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[17].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_19_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[18].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_20_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[19].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_21_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[20].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_22_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[21].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_23_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[22].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_24_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[23].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_25_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[24].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_26_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[25].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_27_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[26].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_28_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[27].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_29_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[28].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_30_CONF_DIGITAL_INPUT_FUNC:
      mpDigitalConfigDP[29].Attach(pSubject);
      break;

    case SP_DICSP_DIG_IN_1_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[0].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_2_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[1].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_3_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[2].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_4_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[3].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_5_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[4].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_6_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[5].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_7_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[6].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_8_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[7].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_9_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[8].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_10_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[9].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_11_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[10].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_12_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[11].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_13_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[12].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_14_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[13].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_15_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[14].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_16_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[15].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_17_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[16].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_18_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[17].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_19_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[18].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_20_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[19].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_21_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[20].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_22_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[21].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_23_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[22].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_24_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[23].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_25_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[24].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_26_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[25].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_27_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[26].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_28_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[27].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_29_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[28].Attach(pSubject);
      break;
    case SP_DICSP_DIG_IN_30_CONF_DIGITAL_INPUT_LOGIC:
      mpDigitalConfLogicDP[29].Attach(pSubject);
      break;

    case SP_DICSP_CURRENT_NO:
      mpCurrentDigitalInputNumber.Attach(pSubject);
      break;
    case SP_DICSP_VIRTUAL_CONF_DIGITAL_INPUT_FUNC:
      mpVirtualDigitalInputConfig.Attach(pSubject);
      break;
    case SP_DICSP_VIRTUAL_CONF_DIGITAL_INPUT_LOGIC:
      mpVirtualDigitalInputConfLogic.Attach(pSubject);
      break;

  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalInputConfigSlipPoint::ConnectToSubjects(void)
{
  for ( int i = 0; i < MAX_NO_OF_DIG_INPUTS; ++i )
  {
    mpDigitalConfigDP[i]->Subscribe(this);
    mpDigitalConfLogicDP[i]->Subscribe(this);
  }
  mpCurrentDigitalInputNumber->Subscribe(this);
  mpVirtualDigitalInputConfig->Subscribe(this);
  mpVirtualDigitalInputConfLogic->Subscribe(this);
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void DigitalInputConfigSlipPoint::UpdateVirtualDigitalInputConfLogic()
{
  const int index = (mpCurrentDigitalInputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < MAX_NO_OF_DIG_INPUTS))
  {
    mpVirtualDigitalInputConfLogic->SetValue(mpDigitalConfLogicDP[index]->GetValue());
  }
  else
  {
    FatalErrorOccured("DICSP2 index out of range!");
  }
}

void DigitalInputConfigSlipPoint::UpdateVirtualDigitalInputConfig()
{
  const int index = (mpCurrentDigitalInputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < MAX_NO_OF_DIG_INPUTS))
  {
    mpVirtualDigitalInputConfig->SetValue(mpDigitalConfigDP[index]->GetValue());
  }
  else
  {
    FatalErrorOccured("DICSP3 index out of range!");
  }
}

void DigitalInputConfigSlipPoint::UpdateCurrentDigitalInputConfig()
{
  const int index = (mpCurrentDigitalInputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < MAX_NO_OF_DIG_INPUTS))
  {
    mpDigitalConfigDP[index]->SetValue(mpVirtualDigitalInputConfig->GetValue());
  }
  else
  {
    FatalErrorOccured("DICSP4 index out of range!");
  }
}

void DigitalInputConfigSlipPoint::UpdateCurrentDigitalInputConfLogic()
{
  const int index = (mpCurrentDigitalInputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < MAX_NO_OF_DIG_INPUTS))
  {
    mpDigitalConfLogicDP[index]->SetValue(mpVirtualDigitalInputConfLogic->GetValue());
  }
  else
  {
    FatalErrorOccured("DICSP5 index out of range!");
  }
}

void DigitalInputConfigSlipPoint::UpdateUpperStatusLine()
{
    Display* p_display = NULL;
    char display_number[10];

    int index = mpCurrentDigitalInputNumber->GetAsInt();

    sprintf(display_number, "4.4.2.%i", index);

    p_display = GetDisplay( DISPLAY_DI_CONFIG_SELECT_FUNC_ID );
    p_display->SetDisplayNumber( display_number );

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
