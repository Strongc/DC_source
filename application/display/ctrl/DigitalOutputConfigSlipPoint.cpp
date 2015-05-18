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
/* CLASS NAME       : DigitalOutputConfigSlipPoint                           */
/*                                                                          */
/* FILE NAME        : DigitalOutputConfigSlipPoint.CPP                       */
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
#include <Factory.h>
#include <DataPoint.h>
#include <RelayFuncHandler.h>
#include <DisplayController.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "DigitalOutputConfigSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_DO_CONFIG_SELECT_FUNC_ID 36

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
DigitalOutputConfigSlipPoint::DigitalOutputConfigSlipPoint()
{
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DigitalOutputConfigSlipPoint::~DigitalOutputConfigSlipPoint()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalOutputConfigSlipPoint::InitSubTask(void)
{
  if (mpCurrentDigitalOutputNumber.IsValid())
  {
    mpCurrentDigitalOutputNumber->SetAsInt(1);
  }

  UpdateVirtualDigitalOutputConfig();
  UpdateUpperStatusLine();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalOutputConfigSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if ( mpCurrentDigitalOutputNumber.IsUpdated() )
  {
    UpdateVirtualDigitalOutputConfig();
    UpdateUpperStatusLine();
  }

  if ( mpVirtualDigitalOutputConfig.IsUpdated() )
  {
    UpdateCurrentDigitalOutputConfig();
  }


  {
    const int index = mpCurrentDigitalOutputNumber->GetAsInt() - 1;

    if ((index >= 0) && (index < NO_OF_RELAYS))
    {
      if (mpDigitalConfigDP[index].IsUpdated())
      {
        UpdateVirtualDigitalOutputConfig();
      }
    }
    else
    {
      FatalErrorOccured("DOCSP1 index out of range!");
    }
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalOutputConfigSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reacting on updates

  for (int i = 0; i < NO_OF_RELAYS; ++i )
  {
    if (mpDigitalConfigDP[i].Detach(pSubject))
    {
      return;
    }
  }

  mpCurrentDigitalOutputNumber.Detach(pSubject);
  mpVirtualDigitalOutputConfig.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalOutputConfigSlipPoint::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    if (mpVirtualDigitalOutputConfig.Update(pSubject))
    {
      // nop
    }
    else if (mpCurrentDigitalOutputNumber.Update(pSubject))
    {
      // nop
    }
    else
    {
      for (int i = 0; i < NO_OF_RELAYS; ++i )
      {
        if (mpDigitalConfigDP[i].Update(pSubject))
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
void DigitalOutputConfigSlipPoint::SetSubjectPointer(int Id,Subject* pSubject)
{
  switch ( Id )
  {
    case SP_DOCSP_DIG_OUT_1_CONF_RELAY_FUNC:
      mpDigitalConfigDP[0].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_2_CONF_RELAY_FUNC:
      mpDigitalConfigDP[1].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_3_CONF_RELAY_FUNC:
      mpDigitalConfigDP[2].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_4_CONF_RELAY_FUNC:
      mpDigitalConfigDP[3].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_5_CONF_RELAY_FUNC:
      mpDigitalConfigDP[4].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_6_CONF_RELAY_FUNC:
      mpDigitalConfigDP[5].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_7_CONF_RELAY_FUNC:
      mpDigitalConfigDP[6].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_8_CONF_RELAY_FUNC:
      mpDigitalConfigDP[7].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_9_CONF_RELAY_FUNC:
      mpDigitalConfigDP[8].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_10_CONF_RELAY_FUNC:
      mpDigitalConfigDP[9].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_11_CONF_RELAY_FUNC:
      mpDigitalConfigDP[10].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_12_CONF_RELAY_FUNC:
      mpDigitalConfigDP[11].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_13_CONF_RELAY_FUNC:
      mpDigitalConfigDP[12].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_14_CONF_RELAY_FUNC:
      mpDigitalConfigDP[13].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_15_CONF_RELAY_FUNC:
      mpDigitalConfigDP[14].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_16_CONF_RELAY_FUNC:
      mpDigitalConfigDP[15].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_17_CONF_RELAY_FUNC:
      mpDigitalConfigDP[16].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_18_CONF_RELAY_FUNC:
      mpDigitalConfigDP[17].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_19_CONF_RELAY_FUNC:
      mpDigitalConfigDP[18].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_20_CONF_RELAY_FUNC:
      mpDigitalConfigDP[19].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_21_CONF_RELAY_FUNC:
      mpDigitalConfigDP[20].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_22_CONF_RELAY_FUNC:
      mpDigitalConfigDP[21].Attach(pSubject);
      break;
    case SP_DOCSP_DIG_OUT_23_CONF_RELAY_FUNC:
      mpDigitalConfigDP[22].Attach(pSubject);
      break;
    case SP_DOCSP_CURRENT_NO:
      mpCurrentDigitalOutputNumber.Attach(pSubject);
      break;
    case SP_DOCSP_VIRTUAL_CONF_RELAY_FUNC:
      mpVirtualDigitalOutputConfig.Attach(pSubject);
      break;

    case SP_DOCSP_IS_HIGH_END:
      mpIsHighEnd.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void DigitalOutputConfigSlipPoint::ConnectToSubjects(void)
{
  for ( int i = 0; i < NO_OF_RELAYS; ++i )
  {
    mpDigitalConfigDP[i]->Subscribe(this);
  }

  mpCurrentDigitalOutputNumber->Subscribe(this);
  mpVirtualDigitalOutputConfig->Subscribe(this);
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void DigitalOutputConfigSlipPoint::UpdateVirtualDigitalOutputConfig()
{
  const int index = (mpCurrentDigitalOutputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < NO_OF_RELAYS))
  {
    mpVirtualDigitalOutputConfig->SetValue(mpDigitalConfigDP[index]->GetValue());
  }
  else
  {
    FatalErrorOccured("DOCSP2 index out of range!");
  }
}

void DigitalOutputConfigSlipPoint::UpdateCurrentDigitalOutputConfig()
{
  const int index = (mpCurrentDigitalOutputNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < NO_OF_RELAYS))
  {
    mpDigitalConfigDP[index]->SetValue(mpVirtualDigitalOutputConfig->GetValue());
  }
  else
  {
    FatalErrorOccured("DOCSP3 index out of range!");
  }
}

void DigitalOutputConfigSlipPoint::UpdateUpperStatusLine()
{
  Display* p_display = NULL;
  char display_number[10];

  int index = mpCurrentDigitalOutputNumber->GetAsInt();

  if (mpIsHighEnd->GetAsBool())
  {
    sprintf(display_number, "4.4.4.%i", index);
  }
  else
  {
    sprintf(display_number, "4.4.3.%i", index);
  }

  p_display = GetDisplay( DISPLAY_DO_CONFIG_SELECT_FUNC_ID );
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
