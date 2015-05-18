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
/* CLASS NAME       : SystemFlow                                            */
/*                                                                          */
/* FILE NAME        : SystemFlow.cpp                                        */
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
#include <SystemFlow.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define CPF_DYNCONST_MAX      -0.36f
#define CPF_DYNCONST_MIN   -3600.0f

#if defined(MATLAB_TEST)
  #if MIN_EMPTY_TIME <= 0
    #error "MIN_EMPTY_TIME must be larger than zero"
  #endif
#endif


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
SystemFlow::SystemFlow(PitFlow* pPitFlow, DynparIdent* pDynparIdent)
{
   mpPitFlow = pPitFlow;
   mpDynparIdent = pDynparIdent;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SystemFlow::~SystemFlow()
{
}


/****************************************************************************
 * INPUT(S)             : Scale values. Only the max flow value is used.
 * OUTPUT(S)            :
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for initialization of the pit estimation
 *                        routine.
****************************************************************************/
void SystemFlow::InitSystemFlow(float maxFlowValue)
{
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mQpump[i] = 0.0f;
    mQpumpFiltered[i] = 0.0f;
  }

  mQpipe    = 0.0f;
  mQinFiltered = 0.0f;

  flow_runoff_timer = 0;

  /** Set scale values ***/
  mMaxFlow = maxFlowValue;

}

/****************************************************************************
 * INPUT(S)             : Variables and parameters necessary for updating the
 *                        flow estimates.
 * OUTPUT(S)            :
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for updating the flow estimate.
 ****************************************************************************/
void SystemFlow::RunSystemFlow(AFC_RUN_SYSTEM_FLOW_STRUCT* pArguments)
{
  bool is_any_pump_running = false;

  float number_of_pumps_present = 0;

  float delta_pressure = pArguments->pressureOut - pArguments->pressureIn;

  float q_pump_sum = 0.0f;

  /*** Pump flows ***/
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    AFC_CALCULATE_PUMP_FLOW_STRUCT calculate_pump_flow_parameters;
    calculate_pump_flow_parameters.speed = pArguments->pump[i].speed;
    calculate_pump_flow_parameters.torque = pArguments->pump[i].torque;
    calculate_pump_flow_parameters.pumpState = pArguments->pump[i].pumpState;
    calculate_pump_flow_parameters.pumpRampUpPassed = pArguments->pump[i].pumpRampUpPassed;
    calculate_pump_flow_parameters.deltaPressure = delta_pressure;
    calculate_pump_flow_parameters.theta[0] = pArguments->pump[i].theta[0];
    calculate_pump_flow_parameters.theta[1] = pArguments->pump[i].theta[1];
    calculate_pump_flow_parameters.theta[2] = pArguments->pump[i].theta[2];
    calculate_pump_flow_parameters.theta[3] = pArguments->pump[i].theta[3];

    mQpump[i] = CalculatePumpFlow(&calculate_pump_flow_parameters);
    q_pump_sum += mQpump[i];
  }


  /*** Pipe flows ***/
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    if (pArguments->pump[i].pumpState == AFC_PUMP_RUNNING)
    {
      is_any_pump_running = true;
    }

    if (pArguments->pump[i].pumpState != AFC_PUMP_NOT_THERE)
    {
      number_of_pumps_present++;
    }
  }

  if (!is_any_pump_running)
  {
    mPoutStop = (1.0f - STOP_PRESSURE_FILTER) * mPoutStop + STOP_PRESSURE_FILTER * pArguments->pressureOut;
  }

  bool is_all_running_pumps_ok = true;

  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    if (pArguments->pump[i].pumpState == AFC_PUMP_RUNNING
      && ((pArguments->pump[i].theta[0]
        + pArguments->pump[i].theta[1]
        + pArguments->pump[i].theta[2]
        + pArguments->pump[i].theta[3]) == 0.0f))
    {
      is_all_running_pumps_ok = false;
      break;
    }
  }


  if (is_all_running_pumps_ok)
  {
    mpDynparIdent->RunDynparUpdate(pArguments->pressureOut, q_pump_sum, is_any_pump_running);
  }

  mQpipe = CalculatePipeFlow(mQpipe, pArguments->pressureOut, is_any_pump_running);

  if (mQpipe < 0.0f)
  {
    mQpipe = q_pump_sum;
  }
  else
  {
    for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
    {
      if ( (pArguments->pump[i].pumpState != AFC_PUMP_NOT_THERE) && (number_of_pumps_present != 0) )
      {
        mQpump[i] = mQpipe / number_of_pumps_present;
      }
      else
      {
        mQpump[i] = 0.0f;
      }
    }
  }


  /*** Filtering pump flows ***/
  for (int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mQpumpFiltered[i] = (1.0f - QPUMP_FILT) * mQpumpFiltered[i] + QPUMP_FILT * pArguments->pitArea * mQpump[i];
  }


  /*** Inflow estimate and filtering ***/
  float q_pit = pArguments->pitArea * mpPitFlow->GetPitFlow();

  if ( is_any_pump_running )
  {
    flow_runoff_timer = 0;
  }
  else
  {
    flow_runoff_timer++;
    if ( flow_runoff_timer > FLOW_RUNOFF_TIME )
    {
      flow_runoff_timer = FLOW_RUNOFF_TIME;
    }
  }
  if ( (flow_runoff_timer <= 0) || (FLOW_RUNOFF_TIME <= flow_runoff_timer) )
  {
    mQinFiltered = (1.0f - QPIT_FILT) * mQinFiltered + QPIT_FILT * (q_pit + pArguments->pitArea * mQpipe);
  }
  if ( mQinFiltered < 0.0f )
  {
    mQinFiltered = 0.0f;
  }

}


/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Flow value.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return the inflow estimate.
****************************************************************************/
float SystemFlow::GetInflow(void)
{
  return mQinFiltered;
}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Flow value.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return the pump flow estimate.
****************************************************************************/
float SystemFlow::GetPumpFlow(int pumpIndex)
{
  #if defined(FILTER_ON)
  return mQpumpFiltered[pumpIndex];
  #else
  return mQpump[pumpIndex];
  #endif
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : The calculated pump flow.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculate pump flow.
****************************************************************************/
float SystemFlow::CalculatePumpFlow(AFC_CALCULATE_PUMP_FLOW_STRUCT* pArguments)
{
  float q_pump = 0.0f;
  float w = pArguments->speed;
  float T = pArguments->torque;

  if ( (pArguments->pumpState == AFC_PUMP_RUNNING) && (pArguments->pumpRampUpPassed == true) && (w > 1.0f) )
  {
    q_pump = pArguments->theta[0] / w
           + pArguments->theta[1] * pArguments->deltaPressure / w
           + pArguments->theta[2] * T / w
           + pArguments->theta[3] * w;
  }
  else
  {
    q_pump = 0.0f;
  }

  if ( (q_pump < 0.0f) )
  {
    q_pump = 0.0f;
  }

  return q_pump;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : The calculated pipe flow.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculate pipe flow.
****************************************************************************/
float SystemFlow::CalculatePipeFlow(float qPipeEstimated, float pout, bool isAnyPumpRunning)
{
  float delta_q_pipe;

  t_calcvar *pTheta = mpDynparIdent->GetDynpar();

  if (!isAnyPumpRunning) /* All pumps are off */
  {

    if ( (pTheta[0] == 0.0f) && (pTheta[1] == 0.0f) && (pTheta[2] == 0.0f) ) /* Check of the parameters are valid */
    {
      qPipeEstimated = 0.0f;
    }
    else
    {
      /* To ensure that the parameters forms a stable system */
      if ( (CPF_DYNCONST_MIN < pTheta[0]) && (pTheta[0] < CPF_DYNCONST_MAX) && (pout < mPoutStop) )
      {
        delta_q_pipe = pTheta[0] * fabs(qPipeEstimated) * qPipeEstimated + pTheta[1] * pout + pTheta[2];

        if (delta_q_pipe > (-(mMaxFlow / 3.0f) / (MIN_EMPTY_TIME)))
        {
          delta_q_pipe = -(mMaxFlow / 3.0f) / (MIN_EMPTY_TIME);
        }

        qPipeEstimated = qPipeEstimated + delta_q_pipe;
      }
      else
      {
        qPipeEstimated = 0.0f;
      }
    }
  }
  else
  {
    qPipeEstimated = -1.0f;
  }

  return qPipeEstimated;
}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

