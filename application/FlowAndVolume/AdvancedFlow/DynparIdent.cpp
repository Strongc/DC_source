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
/* CLASS NAME       : DynparIdent                                           */
/*                                                                          */
/* FILE NAME        : DynparIdent.cpp                                       */
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
#include <DynparIdent.h>

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
DynparIdent::DynparIdent(KalmanCalc* pKalmanCalc)
{
  mOnCount = 0;
  mStartUpDelayTimer = 0;

  mpKalmanCalc = pKalmanCalc;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DynparIdent::~DynparIdent()
{
}

/****************************************************************************
 * INPUT(S)             : ? + ?
 * OUTPUT(S)            :
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for initialization of the parameter
 *                        estimate routine.
****************************************************************************/
void DynparIdent::InitDynparIdent(float nominalFlow, float nominalPressure)
{
  /*** Start state ***/
  mState = S5_WAIT;

  /*** Pipe dynamics model parameters ***/
  mThetaDyn[0] = 0.0f;
  mThetaDyn[1] = 0.0f;
  mThetaDyn[2] = 0.0f;

  mThetaDynOut[0] = 0.0f;
  mThetaDynOut[1] = 0.0f;
  mThetaDynOut[2] = 0.0f;

  mPdyn[0] = 200.0f;
  mPdyn[1] = 0.0f;
  mPdyn[2] = 0.0f;

  mPdyn[3] = 0.0f;
  mPdyn[4] = 200.0f;
  mPdyn[5] = 0.0f;

  mPdyn[6] = 0.0f;
  mPdyn[7] = 0.0f;
  mPdyn[8] = 200.0f;

  /*** Confi settings ***/
  mConfidenceMax = 10.0f * (DYNPARCONFI_LIMIT);

  /*** Scaling values ***/
  mScaleFlow = 1.0f / nominalFlow;
  mScalePressure = 1.0f / nominalPressure;
}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            :
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for updating the parameter estimate with
 *                        respect to new measurements.
****************************************************************************/
void DynparIdent::RunDynparUpdate(float poutMeas, float qPumps, bool isAnyPumpRunning)
{
  /*** Pre filters ***/
  mQKal    = (1.0f - PRE_FILTER_CONST) * mQKal + PRE_FILTER_CONST * mScaleFlow * qPumps;

  mQKal2   = fabs(mQKal)*mQKal;
  mPoutKal = (1.0f - PRE_FILTER_CONST) * mPoutKal + PRE_FILTER_CONST * mScalePressure * poutMeas;

  /*** State machine for parameter check ***/
  switch (mState)
  {
    case S2_DATA_LOG :
      ParameterEstimationOfPipeDynamic(mQKal, mQKalOld, mQKal2Old, mPoutKalOld);
      mQKalOld    = mQKal;
      mQKal2Old   = mQKal2;
      mPoutKalOld = mPoutKal;
      //printf("data: (Q,Q^2,p) = (%f, %f, %f), pzero = %f\n",mQKalOld,mQKal2Old,mPoutKalOld,mPoutZero);
      if ( isAnyPumpRunning )
      {
        mStartUpDelayTimer = 0;
        mOnCount++;
      }
      else
      {
        mPoutZero = (1.0f - POUT_ZERO_FILTER) * mPoutZero + POUT_ZERO_FILTER * mPoutKal;
      }
      if ((!isAnyPumpRunning) && (mStartUpDelayTimer == 0))
      {
        mState = S4_SAVE_RECORD;
      }
      if (mOnCount > DPI_MAX_ON_TIME)
      {
        mState = S3_DECLINE_RECORD;
      }
      break;

    case S3_DECLINE_RECORD :
      if (!isAnyPumpRunning)
      {
        mpKalmanCalc->CopyMatrices(mThetaDyn, mPdyn, mThetaDynOld, mPdynOld, PIPEDYN_DIM);
        mState = S5_WAIT;
      }
      break;

    case S4_SAVE_RECORD :
      mpKalmanCalc->CopyMatrices(mThetaDynOld, mPdynOld, mThetaDyn, mPdyn, PIPEDYN_DIM);
      mState = S5_WAIT;
      break;

    case S5_WAIT :
      mStartUpDelayTimer++;
      if ((mStartUpDelayTimer > DPI_STARTDELAY) || (isAnyPumpRunning))
      {
        mOnCount    = 0;
        mQKalOld    = 0.0f;
        mPoutKalOld = 0.0f;
        mState = S2_DATA_LOG;
      }
      break;
  }

}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Parameter for the pipe dynamics model.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return function returning the parameters of the
 *                        pipe dynamics model.
****************************************************************************/
t_calcvar* DynparIdent::GetDynpar(void)
{
  /* thetadyn[0] ~ models pressure losses in the pipe which must affect negatively */
  /* thetadyn[1] ~ models the affect of the pump pressure which must affect positively */
  /* thetadyn[2] ~ models the lift of the pipe which must affect negatively */
  /* thetadyn[1]*mPoutZero + thetadyn[2] ~ Models the expected flow change at pump off */
  if ( (mConfidenceMax < DYNPARCONFI_LIMIT) &&
       (mThetaDyn[0] < 0.0f) && (mThetaDyn[1] > 0.0f) && (mThetaDyn[2] < 0.0f) &&
       (fabs(mThetaDyn[1] * mPoutZero + mThetaDyn[2]) < fabs(0.1f * mThetaDyn[2])) )
  {
    mThetaDynOut[0] = mThetaDyn[0] * mScaleFlow;
    mThetaDynOut[1] = mThetaDyn[1] * mScalePressure / mScaleFlow;
    mThetaDynOut[2] = mThetaDyn[2] / mScaleFlow;
  }

  return mThetaDynOut;

}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : P matrix for the pipe dynamics model.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return function returning the P matrix of the
 *                        pipe dynamics model.
****************************************************************************/
t_calcvar* DynparIdent::GetPdynpar(void)
{
  return mPdyn;
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
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Executing parameter estimation for the pressure
 *                       polynomial.
****************************************************************************/
void DynparIdent::ParameterEstimationOfPipeDynamic(t_calcvar qKal, t_calcvar qKalOld, t_calcvar qKal2Old, t_calcvar poutKalOld)
{
  t_calcvar c_dyn[PIPEDYN_DIM];
  float confidence;

  /*** Setup variable vector ***/
  c_dyn[0] = qKal2Old;
  c_dyn[1] = poutKalOld;
  c_dyn[2] = 1.0f;

  /*** Execute kalman filter ***/
  if (mConfidenceMax > (0.1f * DYNPARCONFI_LIMIT))
  {
    mpKalmanCalc->UpdateKalman(mThetaDyn, mPdyn, c_dyn, qKal - qKalOld, R_DYN_0, PIPEDYN_DIM);
//    printf("%f, %f, %f, %f -> ",c_dyn[0],c_dyn[1],c_dyn[2],qKal - qKalOld);
  }
//  printf("confi=%f/%f, (%f, %f, %f), (%f, %f)\n",mConfidenceMax,0.1f * (DYNPARCONFI_LIMIT),mThetaDyn[0],mThetaDyn[1],mThetaDyn[2],c_dyn[0],c_dyn[1]);
//  printf(" -> %f ~ %f\n",c_dyn[0]*mThetaDyn[0]+c_dyn[1]*mThetaDyn[1]+c_dyn[2]*mThetaDyn[2],qKal-qKalOld);

  /*** max filter ***/
  confidence = mpKalmanCalc->ConfidenceOfKalman(mPdyn, c_dyn, PIPEDYN_DIM);
  if (mConfidenceMax < confidence)
  {
    mConfidenceMax = (1.0f - DYNPARCONFI_MAX_FAST) * mConfidenceMax + (DYNPARCONFI_MAX_FAST * confidence);
  }
  else
  {
    mConfidenceMax = (1.0f - DYNPARCONFI_MAX_SLOW) * mConfidenceMax + (DYNPARCONFI_MAX_SLOW * confidence);
  }

}
/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

