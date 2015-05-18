/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: Dedicated Controls                               */
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
/*                                                                          */
/****************************************************************************/
/* CLASS NAME       : PumpEvaluation                                        */
/*                                                                          */
/* FILE NAME        : PumpEvaluation.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/*                                                                          */
/****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <math.h>
#if defined(MATLAB_TEST)
#include "mex.h"
#endif

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PumpEvaluation.h"

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
*
*****************************************************************************/
PumpEvaluation::PumpEvaluation(ParIdent* pParameterIdentifier, PumpCurves* pPumpCurves)
{
  mpParameterIdentifier = pParameterIdentifier;
  mpPumpCurves = pPumpCurves;
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION:
*
****************************************************************************/
PumpEvaluation::~PumpEvaluation()
{
}


/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Initialization of the pump evaluation.
****************************************************************************/
void PumpEvaluation::InitPumpEvaluation(void)
{
  int i;

  for (i = 0; i < SYSP_SIZE; i++)
  {
    mSystemParameters[i] = 0.0f;
  }

  for (i = 0; i < MODEL_SIZE; i++)
  {
    mPressureParameters[i] = 0.0f;
    mPowerParameters[i]    = 0.0f;
  }
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Updating the stored parameters. These parameters
*                        used as reference model when duing fault evaluation.
****************************************************************************/
bool PumpEvaluation::LogDataSet(void)
{
  bool log_result;
  t_calcvar p_Ptheta[SYSP_SIZE * SYSP_SIZE];
  t_calcvar* p_flow_parameters;
  t_calcvar* p_pressure_parameters;

  log_result = true;
  p_flow_parameters = mpParameterIdentifier->GetPumpFlowParameters();
  if ( (p_flow_parameters[0] == 0.0f) &&
    (p_flow_parameters[1] == 0.0f) &&
    (p_flow_parameters[2] == 0.0f) &&
    (p_flow_parameters[3] == 0.0f) )
  {
    log_result = false;
  }
  p_pressure_parameters = mpParameterIdentifier->GetPumpPresParameters();
  if ( (p_pressure_parameters[0] == 0.0f) &&
    (p_pressure_parameters[1] == 0.0f) &&
    (p_pressure_parameters[2] == 0.0f) &&
    (p_pressure_parameters[3] == 0.0f) &&
    (p_pressure_parameters[4] == 0.0f) &&
    (p_pressure_parameters[5] == 0.0f) &&
    (p_pressure_parameters[6] == 0.0f) &&
    (p_pressure_parameters[7] == 0.0f) &&
    (p_pressure_parameters[8] == 0.0f) )
  {
    log_result = false;
  }

  if (log_result)
  {
    mpParameterIdentifier->GetPumpPresPmatrix(p_Ptheta, SYSP_SIZE);
    mpPumpCurves->GetPowerCurve(mPowerParameters, p_flow_parameters, p_pressure_parameters, p_Ptheta);
    mpPumpCurves->GetPressureCurve(mPressureParameters, p_flow_parameters, p_pressure_parameters, p_Ptheta);

    for (int i = 0; i < SYSP_SIZE; i++)
    {
      mSystemParameters[i] = p_pressure_parameters[i];
    }
  }
  return log_result;
}

/****************************************************************************
* INPUT(S)             : Measurements.
* OUTPUT(S)            : Decision of fault evaluation.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the result of evaluation
*                        for fault based on a structural residaul evaluation.
****************************************************************************/
float PumpEvaluation::ModelEvaluation(float deltaP, float T, float w, AFC_PUMP_STATE_TYPE pumpState)
{
  float res;

  if ( ((0.0f != mSystemParameters[0]) ||
    (0.0f != mSystemParameters[1]) ||
    (0.0f != mSystemParameters[2]) ||
    (0.0f != mSystemParameters[3]) ||
    (0.0f != mSystemParameters[4]) ||
    (0.0f != mSystemParameters[5]) ||
    (0.0f != mSystemParameters[6]) ||
    (0.0f != mSystemParameters[7]) ||
    (0.0f != mSystemParameters[8])) && (pumpState == AFC_PUMP_RUNNING) )
  {
    res = deltaP * deltaP +
      mSystemParameters[0] +
      mSystemParameters[1] * deltaP +
      mSystemParameters[2] * T +
      mSystemParameters[3] * T * T +
      mSystemParameters[4] * deltaP * T +
      mSystemParameters[5] * w * w * w * w +
      mSystemParameters[6] * deltaP * w * w +
      mSystemParameters[7] * T * w * w +
      mSystemParameters[8] * w * w;
  }
  else
  {
    res = 0.0f;
  }
  return res;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Reset the model used in the model evaluation. This
*                        is the same as resetting the fault evaluation
*                        signal.
*****************************************************************************/
void PumpEvaluation::ResetModelEvaluation(void)
{
  bool log_result = true;
  t_calcvar* p_pressure_parameters;

  p_pressure_parameters = mpParameterIdentifier->GetPumpPresParameters();
  if ( (p_pressure_parameters[0] == 0.0f) &&
    (p_pressure_parameters[1] == 0.0f) &&
    (p_pressure_parameters[2] == 0.0f) &&
    (p_pressure_parameters[3] == 0.0f) &&
    (p_pressure_parameters[4] == 0.0f) &&
    (p_pressure_parameters[5] == 0.0f) &&
    (p_pressure_parameters[6] == 0.0f) &&
    (p_pressure_parameters[7] == 0.0f) &&
    (p_pressure_parameters[8] == 0.0f) )
  {
    log_result = false;
  }
  if (log_result)
  {
    for (int i = 0; i < SYSP_SIZE; i++)
    {
      mSystemParameters[i] = p_pressure_parameters[i];
    }
  }
}

/****************************************************************************
* INPUT(S)             : Measurements.
* OUTPUT(S)            : Loss of efficiency compared to the reference model.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return the loss of efficiency between the actual
*                        operating conditions and the the efficiency
*                        calculated based on the reference model.
****************************************************************************/
float PumpEvaluation::EfficiencyEvaluation(float deltaP, float T, float w, float q, AFC_PUMP_STATE_TYPE pumpState, float pitArea)
{
  float eff_reduction, eff_model;

  if ( ((0.0f != mPressureParameters[0]) ||
    (0.0f != mPressureParameters[1]) ||
    (0.0f != mPressureParameters[2]) ||
    (0.0f != mPressureParameters[3]) ||
    (0.0f != mPowerParameters[0]) ||
    (0.0f != mPowerParameters[1]) ||
    (0.0f != mPowerParameters[2]) ||
    (0.0f != mPowerParameters[3])) && (pumpState == AFC_PUMP_RUNNING) )
  {
    eff_model = GetModelEfficiency(w, q, pitArea);

    if (eff_model > 0.0f)
    {
      eff_reduction = GetEfficiency(deltaP, T, w, q) / eff_model;
      //eff_reduction = GetEfficiency2(w, q, pitArea) / eff_model;
    }
    else
    {
      eff_reduction = 0.0f;
    }
    if (eff_reduction > 1.0f)
    {
      eff_reduction = 1.0f;
    }
    if (eff_reduction < 0.0f)
    {
      eff_reduction = 0.0f;
    }
  }
  else
  {
    eff_reduction = 0.0f;
  }
  return eff_reduction;
}

/****************************************************************************
* INPUT(S)             : Measurements.
* OUTPUT(S)            : Get model efficiency.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the efficiency at the
*                        given operating point calculated based on the
*                        identified model parameters.
****************************************************************************/
float PumpEvaluation::GetModelEfficiency(float w, float q, float pitArea)
{
  float P, delta_p;

  if ( pitArea > 0.0f )
  {
    delta_p = mPressureParameters[0] * q * q / (pitArea * pitArea)
      + mPressureParameters[1] * q * w / pitArea
      + mPressureParameters[2] * w * w
      + mPressureParameters[3];

    P  = mPowerParameters[0] * q * q * w / (pitArea * pitArea)
      + mPowerParameters[1] * q * w * w / pitArea
      + mPowerParameters[2] * w * w * w
      + mPowerParameters[3] * w;

    if ( P > 0.0f )
    {
      return delta_p * q / P;
    }
    else
    {
      return -1.0f;
    }
  }
  else
  {
    return -1.0f;
  }
}

/****************************************************************************
* INPUT(S)             : Measurements.
* OUTPUT(S)            : Get actual efficiency.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the actual efficiency
*                        based on the available measurements.
****************************************************************************/
float PumpEvaluation::GetEfficiency(float deltaP, float T, float w, float q)
{
  if ((w > 0.0f) && (T > 0.0f))
  {
    return deltaP * q / (T * w);
  }
  else
  {
    return -1.0f;
  }
}

/****************************************************************************
* INPUT(S)             : Measurements.
* OUTPUT(S)            : Get actual efficiency.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the actual efficiency
*                        based on the available measurements.
****************************************************************************/
float PumpEvaluation::GetEfficiency2(float w, float q, float pitArea)
{
  float delta_p, P;
  bool log_result = true;
  t_calcvar p_theta[SYSP_SIZE * SYSP_SIZE];
  t_calcvar act_pressure_parameters[MODEL_SIZE];
  t_calcvar act_power_parameters[MODEL_SIZE];
  t_calcvar* p_flow_parameters;
  t_calcvar* p_pressure_parameters;

  /*** Get and recalculate model parameters ***/

  p_flow_parameters = mpParameterIdentifier->GetPumpFlowParameters();
  if ( (p_flow_parameters[0] == 0.0f) &&
    (p_flow_parameters[1] == 0.0f) &&
    (p_flow_parameters[2] == 0.0f) &&
    (p_flow_parameters[3] == 0.0f) )
  {
    log_result = false;
  }
  p_pressure_parameters = mpParameterIdentifier->GetPumpPresParameters();
  if ( (p_pressure_parameters[0] == 0.0f) &&
    (p_pressure_parameters[1] == 0.0f) &&
    (p_pressure_parameters[2] == 0.0f) &&
    (p_pressure_parameters[3] == 0.0f) &&
    (p_pressure_parameters[4] == 0.0f) &&
    (p_pressure_parameters[5] == 0.0f) &&
    (p_pressure_parameters[6] == 0.0f) &&
    (p_pressure_parameters[7] == 0.0f) &&
    (p_pressure_parameters[8] == 0.0f) )
  {
    log_result = false;
  }
  if (log_result)
  {
    mpParameterIdentifier->GetPumpPresPmatrix(p_theta, SYSP_SIZE);
    mpPumpCurves->GetPowerCurve(act_power_parameters, p_flow_parameters, p_pressure_parameters, p_theta);
    mpPumpCurves->GetPressureCurve(act_pressure_parameters, p_flow_parameters, p_pressure_parameters, p_theta);
  }

  /*** Calculate efficiency ***/
  if ( pitArea > 0.0f )
  {
    delta_p = act_pressure_parameters[0] * q * q / (pitArea * pitArea)
      + act_pressure_parameters[1] * q * w / pitArea
      + act_pressure_parameters[2] * w * w
      + act_pressure_parameters[3];

    P  = act_power_parameters[0] * q * q * w / (pitArea * pitArea)
      + act_power_parameters[1] * q * w * w / pitArea
      + act_power_parameters[2] * w * w * w
      + act_power_parameters[3] * w;

    if ( (P > 0.0f) && log_result )
    {
      return delta_p * q / P;
    }
    else
    {
      return -1.0f;
    }
  }
  else
  {
    return -1.0f;
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

