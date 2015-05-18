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
/* CLASS NAME      : AnalogInputConfigSlipPoint                             */
/*                                                                          */
/* FILE NAME       : AnalogInputConfigSlipPoint.CPP                         */
/*                                                                          */
/* CREATED DATE    :                                                        */
/*                                                                          */
/* SHORT FILE DESCRIPTION:                                                  */
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
#include <DiFuncHandler.h>
#include <DisplayController.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "AnalogInputConfigSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_AI_CONFIG_ID 33
#define DISPLAY_AI_CONFIG_SELECT_MEAS_ID 34

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
AnalogInputConfigSlipPoint::AnalogInputConfigSlipPoint()
{
  mCurrentlyUpdating = false;

  #if NO_OF_ANA_IN == 7
  #error "NO_OF_ANA_IN error"
  #endif
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AnalogInputConfigSlipPoint::~AnalogInputConfigSlipPoint()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AnalogInputConfigSlipPoint::InitSubTask(void)
{
  if (mpCurrentAnalogInputNumber.IsValid())
  {
    mpCurrentAnalogInputNumber->SetAsInt(1);
  }

  UpdateVirtualAnalogInputConfig();
  UpdateUpperStatusLine();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AnalogInputConfigSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if ( mpCurrentAnalogInputNumber.IsUpdated() )
  {
    UpdateVirtualAnalogInputConfig();
    UpdateUpperStatusLine();
  }

  if ( mpVirtualAnalogInputConfig.IsUpdated() )
  {
    UpdateCurrentAnalogInputConfig();
  }

  if ( mpVirtualAnalogInputElectricalConfig.IsUpdated() )
  {
    UpdateCurrentAnalogInputConfig();
  }

  if ( mpVirtualAnalogMinSensorRange.IsUpdated() )
  {
    UpdateCurrentAnalogInputConfig();
  }

  if ( mpVirtualAnalogMaxSensorRange.IsUpdated() )
  {
    UpdateCurrentAnalogInputConfig();
  }

  {
    const int index = mpCurrentAnalogInputNumber->GetAsInt() - 1;

    if ((index >= 0) && (index < NO_OF_ANA_IN))
    {
      if (mpAnalogConfigDP[index].IsUpdated())
      {
        UpdateVirtualAnalogInputConfig();
      }

      if (mpAnalogElectricalConfigDP[index].IsUpdated())
      {
        UpdateVirtualAnalogInputConfig();
      }

      if (mpAnalogMinSensorRange[index].IsUpdated())
      {
        UpdateVirtualAnalogInputConfig();
      }

      if (mpAnalogMaxSensorRange[index].IsUpdated())
      {
        UpdateVirtualAnalogInputConfig();
      }
    }
    else
    {
      FatalErrorOccured("AICSP index out of range!");
    }
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AnalogInputConfigSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reaction on updates

  for (int i = 0; i < NO_OF_ANA_IN; ++i )
  {
    mpAnalogConfigDP[i].Detach(pSubject);
    mpAnalogElectricalConfigDP[i].Detach(pSubject);
    mpAnalogMinSensorRange[i].Detach(pSubject);
    mpAnalogMaxSensorRange[i].Detach(pSubject);
  }

  mpVirtualAnalogInputConfig.Detach(pSubject);
  mpVirtualAnalogInputElectricalConfig.Detach(pSubject);
  mpVirtualAnalogMinSensorRange.Detach(pSubject);
  mpVirtualAnalogMaxSensorRange.Detach(pSubject);
  mpCurrentAnalogInputNumber.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AnalogInputConfigSlipPoint::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    if ( mpVirtualAnalogInputConfig.Update(pSubject) )
    {
    }
    else if ( mpVirtualAnalogInputElectricalConfig.Update(pSubject) )
    {
    }
    else if ( mpVirtualAnalogMinSensorRange.Update(pSubject) )
    {
    }
    else if ( mpVirtualAnalogMaxSensorRange.Update(pSubject) )
    {
    }
    else if ( mpCurrentAnalogInputNumber.Update(pSubject) )
    {
    }
    else
    {
      for (int i = 0; i < NO_OF_ANA_IN; ++i )
      {
        if ( mpAnalogConfigDP[i].Update(pSubject) )
        {
          break;
        }
				else if ( mpAnalogElectricalConfigDP[i].Update(pSubject) )
				{
          break;
				}
				else if ( mpAnalogMinSensorRange[i].Update(pSubject) )
				{
          break;
				}
        else if ( mpAnalogMaxSensorRange[i].Update(pSubject) )
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
void AnalogInputConfigSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
    case SP_AICSP_ANA_IN_1_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[0].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_2_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[1].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_3_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[2].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_4_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[3].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_5_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[4].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_6_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[5].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_7_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[6].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_8_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[7].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_9_CONF_MEASURED_VALUE:
      mpAnalogConfigDP[8].Attach(pSubject);
      break;

    case SP_AICSP_ANA_IN_1_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[0].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_2_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[1].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_3_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[2].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_4_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[3].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_5_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[4].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_6_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[5].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_7_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[6].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_8_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[7].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_9_CONF_SENSOR_ELECTRIC:
      mpAnalogElectricalConfigDP[8].Attach(pSubject);
      break;

    case SP_AICSP_ANA_IN_1_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[0].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_2_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[1].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_3_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[2].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_4_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[3].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_5_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[4].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_6_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[5].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_7_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[6].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_8_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[7].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_9_CONF_SENSOR_MIN_VALUE:
      mpAnalogMinSensorRange[8].Attach(pSubject);
      break;

    case SP_AICSP_ANA_IN_1_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[0].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_2_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[1].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_3_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[2].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_4_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[3].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_5_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[4].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_6_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[5].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_7_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[6].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_8_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[7].Attach(pSubject);
      break;
    case SP_AICSP_ANA_IN_9_CONF_SENSOR_MAX_VALUE:
      mpAnalogMaxSensorRange[8].Attach(pSubject);
      break;

    case SP_AICSP_CURRENT_NO:
      mpCurrentAnalogInputNumber.Attach(pSubject);
      break;

    case SP_AICSP_VIRTUAL_CONF_MEASURED_VALUE:
      mpVirtualAnalogInputConfig.Attach(pSubject);
      break;
    case SP_AICSP_VIRTUAL_CONF_SENSOR_ELECTRIC:
      mpVirtualAnalogInputElectricalConfig.Attach(pSubject);
      break;
    case SP_AICSP_VIRTUAL_CONF_SENSOR_MIN_VALUE:
      mpVirtualAnalogMinSensorRange.Attach(pSubject);
      break;
    case SP_AICSP_VIRTUAL_CONF_SENSOR_MAX_VALUE:
      mpVirtualAnalogMaxSensorRange.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void AnalogInputConfigSlipPoint::ConnectToSubjects(void)
{
  for (int i = 0; i < NO_OF_ANA_IN; ++i )
  {
    mpAnalogConfigDP[i]->Subscribe(this);
    mpAnalogElectricalConfigDP[i]->Subscribe(this);
    mpAnalogMinSensorRange[i]->Subscribe(this);
    mpAnalogMaxSensorRange[i]->Subscribe(this);
  }

  mpVirtualAnalogInputConfig->Subscribe(this);
  mpVirtualAnalogInputElectricalConfig->Subscribe(this);
  mpVirtualAnalogMinSensorRange->Subscribe(this);
  mpVirtualAnalogMaxSensorRange->Subscribe(this);
  mpCurrentAnalogInputNumber->Subscribe(this);
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void AnalogInputConfigSlipPoint::UpdateVirtualAnalogInputConfig()
{
  int i = (mpCurrentAnalogInputNumber->GetAsInt() - 1);

  if (( i < NO_OF_ANA_IN ) && ( i >= 0 ))
  {
    mpVirtualAnalogInputConfig->SetValue(mpAnalogConfigDP[i]->GetValue());
    mpVirtualAnalogInputElectricalConfig->SetValue(mpAnalogElectricalConfigDP[i]->GetValue());
    mpVirtualAnalogMinSensorRange->CopyValues(mpAnalogMinSensorRange[i].GetSubject());
    mpVirtualAnalogMaxSensorRange->CopyValues(mpAnalogMaxSensorRange[i].GetSubject());
  }
}

void AnalogInputConfigSlipPoint::UpdateCurrentAnalogInputConfig()
{
  int i = (mpCurrentAnalogInputNumber->GetAsInt() - 1);

  if (( i < NO_OF_ANA_IN ) && ( i >= 0 ))
  {
    mpAnalogConfigDP[i]->SetValue(mpVirtualAnalogInputConfig->GetValue());
    mpAnalogElectricalConfigDP[i]->SetValue(mpVirtualAnalogInputElectricalConfig->GetValue());
    mpAnalogMinSensorRange[i]->SetValue(mpVirtualAnalogMinSensorRange->GetValue());
    mpAnalogMaxSensorRange[i]->SetValue(mpVirtualAnalogMaxSensorRange->GetValue());
  }
}

void AnalogInputConfigSlipPoint::UpdateUpperStatusLine()
{
    Display* p_display = NULL;
    char display_number[10];

    int index = mpCurrentAnalogInputNumber->GetAsInt();

    sprintf(display_number, "4.4.1.%i", index);

    p_display = GetDisplay( DISPLAY_AI_CONFIG_ID );
    p_display->SetDisplayNumber( display_number );

    sprintf(display_number, "4.4.1.%i.1", index);

    p_display = GetDisplay( DISPLAY_AI_CONFIG_SELECT_MEAS_ID );
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
