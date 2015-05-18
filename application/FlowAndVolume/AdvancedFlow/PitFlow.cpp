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
/* CLASS NAME       : PitFlow                                               */
/*                                                                          */
/* FILE NAME        : PitFlow.cpp                                           */
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
#include <PitFlow.h>
#include "KalmanCalc.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#if defined(MATLAB_TEST)
  #if SAMP_TIME <= 0
    #error "SAMP_TIME must be larger than zero"
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
PitFlow::PitFlow(KalmanCalc *pKalmanCalc)
{
  mpKalmanCalc = pKalmanCalc;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PitFlow::~PitFlow()
{
}


/****************************************************************************
 * INPUT(S)             :
 *
 * OUTPUT(S)            :
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for initialization of the pit estimation
 *                        routine.
****************************************************************************/
void PitFlow::InitPitFlow(float pressureIn)
{
  /*** Filter implementation ***/
//  mState = PF_WAIT;

//  mTimeDiff = 1;
//  mSampHyst = 1;
//  mSampZero = 1;

//  mPinOld         = pressureIn;
//  mPinFiltered    = pressureIn;
//  mPinFilteredOld = pressureIn;

//  mQpitDiff = 0.0f;

  /*** Kalman implementation ***/
  mTimeDiff = 1;

  z[0] = 0.0f;
  z[1] = 0.0f;

  P[0] = 1.0f;
  P[1] = 0.0f;
  P[2] = 0.0f;
  P[3] = 100.0f/10000.0f;

  mQpitEstimated  = 0.0f;

}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            :
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for updating the flow estimate.
 ****************************************************************************/
void PitFlow::RunPitFlow(float pressureIn)
{

//  /** Hysteresis filter ***/
//  switch (mState)
//  {
//    case PF_WAIT :
//      if ( (pressureIn - mPinOld) > 0.0f )
//      {
//        mSampHyst = 1;
//        mState     = PF_HYSTERESIS_PLUS;
//      }
//      if ( (pressureIn - mPinOld) < 0.0f )
//      {
//        mSampHyst = 1;
//        mState     = PF_HYSTERESIS_MINUS;
//      }
//      break;

//    case PF_HYSTERESIS_PLUS :
//      mSampHyst++;
//      if ( (pressureIn - mPinOld) > 0.0f )
//      {
//        mSampHyst = 1;
//        mPinFiltered = mPinOld;
//      }
//      if ( (pressureIn - mPinOld) < 0.0f )
//      {
//        mSampHyst = 1;
//        mState     = PF_HYSTERESIS_MINUS;
//      }
//      if ( mSampHyst > HYSTERESIS_TIME )
//      {
//        mPinFiltered = pressureIn;
//        mState = PF_WAIT;
//      }
//      break;

//    case PF_HYSTERESIS_MINUS :
//      mSampHyst++;
//      if ( (pressureIn - mPinOld) < 0.0f )
//      {
//        mSampHyst = 1;
//        mPinFiltered = mPinOld;
//      }
//      if ( (pressureIn - mPinOld) > 0.0f )
//      {
//        mSampHyst = 1;
//        mState     = PF_HYSTERESIS_PLUS;
//      }
//      if ( mSampHyst > HYSTERESIS_TIME )
//      {
//        mPinFiltered = pressureIn;
//        mState = PF_WAIT;
//      }
//      break;
//  }
//  mPinOld = pressureIn;

//  /*** Derivative calculation with variable sampling time ***/
//  if ( (mPinFiltered - mPinFilteredOld) == 0.0f )
//  {
//    mSampZero++;
//    if ( mSampZero > MAX_SAMP_ZERO )
//    {
//      mSampZero = MAX_SAMP_ZERO;
//    }
//    if ( mTimeDiff < mSampZero )
//    {
//      mTimeDiff = mSampZero;
//    }
//  }
//  else
//  {
//    mQpitDiff      = 1.0f / (10000.0f * SAMP_TIME) * (mPinFiltered - mPinFilteredOld);
//    mTimeDiff      = mSampZero;
//    mSampZero      = 1;
//    mPinFilteredOld  = mPinFiltered;
//  }
//  mQpitEstimated = mQpitDiff / mTimeDiff;

  /*** Filter implementation ***/
//  #define PITFLOW_FILTER_CONST  0.25f
//  mPinFilteredOld = mPinFiltered;
//  mPinFiltered    = (1.0f - PITFLOW_FILTER_CONST)*mPinFiltered + PITFLOW_FILTER_CONST*pressureIn;
//  mQpitEstimated  = 1.0f / (10000.0f * SAMP_TIME) * (mPinFiltered - mPinFilteredOld);


  /*** Kalman implementation ***/

  /** Prediction in time **/
  mpKalmanCalc->PredictKalman(z, P, A, B, 0, Q0, 2);

  /** Update based on measurement **/
  if ( (pressureIn != pressureIn_old) || (mTimeDiff > 5)  )
  {
    mpKalmanCalc->UpdateKalman(z, P, C, pressureIn, R0, 2);
    mTimeDiff = 0;
  }
  else
  {
    mTimeDiff++;
  }
  pressureIn_old = pressureIn;

  /** Estimated pit flow **/
  mQpitEstimated = z[0];


}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Flow value.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return the pit flow estimate.
****************************************************************************/
float PitFlow::GetPitFlow(void)
{
  return mQpitEstimated;
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

