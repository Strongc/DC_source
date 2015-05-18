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
/* CLASS NAME       : PidBlock                                              */
/*                                                                          */
/* FILE NAME        : PidBlock.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 02-06-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <math.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <PidBlock.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

#define SENSOR_QUANTIZATION     0.5f  // [%]
#define PID_ERROR_SS_TIME       2.0f  // [s]
#define PID_ERROR_FILTER_TIME   2.0f  // [s]
#define D_ERROR_FILTER_TIME     0.5f  // [s]
#define D_ERROR_MAX             1.0f  // [%] Max allowable d_error before gain

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
PidBlock::PidBlock()
{
  mPidOutput = 0.0f;
  mOldPidOutput = 0.0f;

  mGp = 0.0f;
  mGi = 0.0f;
  mGd = 0.0f;
  mTs = 1.0f;
  mOutputMin = 0.0f;;
  mOutputMax = 100.0f;
  mIterm = 0.0f;

  mPidErrorSteadyTime = 0.0f;
  mPidErrorFiltered = 0.0f;
  mOldPidError = 0.0f;
  mDErrorFiltered = 0.0f;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PidBlock::~PidBlock()
{
}

/*****************************************************************************
 * Function - InitPid
 * DESCRIPTION:
 *
 *****************************************************************************/
void PidBlock::ConfigPid(float kp, float ti, float td, float ts, float outputMin, float outputMax)
{
  mGp = kp;
  if (ti > 0.0f && ts > 0.0f)
  {
    mGi = ts/ti;
  }
  else
  {
    mGi = 0.0f;
  }
  if (td > 0.0f && ts > 0.0f)
  {
    mGd = td/ts;
  }
  else
  {
    mGd = 0.0f;
  }
  mTs = ts;
  mOutputMin = outputMin;
  mOutputMax = outputMax;
}

/*****************************************************************************
 * Function - PresetPid
 * DESCRIPTION: Reset the history 'parameters' and set the output to the
 *              preset value.
 *
 *****************************************************************************/
void PidBlock::PresetPid(float spValue, float pvValue, float presetValue)
{
  mPidErrorSteadyTime = 0.0f;

  mOldPidError = spValue - pvValue;
  mPidErrorFiltered = mOldPidError;
  mDErrorFiltered = 0.0f;

  mIterm = presetValue - CalcPterm(mOldPidError);
  mPidOutput = presetValue;
}

/*****************************************************************************
 * Function - UpdatePid
 * DESCRIPTION: Calculate the Pid output. To be called every 'sample time'.
 *
 *****************************************************************************/
float PidBlock::UpdatePid(float spValue, float pvValue)
{
  float pid_error;

  pid_error = (spValue - pvValue);

  // In steady state, make a low pass filtering of the pid error
  mPidErrorFiltered += (mTs/PID_ERROR_FILTER_TIME)*(pid_error-mPidErrorFiltered);
  if (fabs(pid_error - mPidErrorFiltered) <= SENSOR_QUANTIZATION)
  {
    if (mPidErrorSteadyTime > PID_ERROR_SS_TIME)
    {
      pid_error = mPidErrorFiltered;
    }
    else
    {
      mPidErrorSteadyTime += mTs;
      if (mPidErrorSteadyTime > PID_ERROR_SS_TIME)
      {
        // Going to steady state. Next line ensures that this transition doesn't create noise
        mPidErrorFiltered = pid_error;
      }
    }
  }
  else
  {
    mPidErrorSteadyTime = 0;
  }

  // Calculate and add P I D
  float p_term = CalcPterm(pid_error);
  float i_term = CalcIterm(pid_error, mOutputMin-p_term, mOutputMax-p_term); // Including anti windup
  float d_term = CalcDterm(pid_error);
  mPidOutput = p_term + i_term + d_term;

  // Validate pid output
  if (mPidOutput < mOutputMin)
  {
    mPidOutput = mOutputMin;
  }
  if (mPidOutput > mOutputMax)
  {
    mPidOutput = mOutputMax;
  }

  return mPidOutput;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - CalcPterm
 * DESCRIPTION: Calculate the new contribution from P part
 *
 *****************************************************************************/
float PidBlock::CalcPterm(float pidError)
{
  return mGp*pidError + mOutputMin; // TBD offset !!
}

/*****************************************************************************
 * Function - CalcIterm
 * DESCRIPTION: Calculate the new contribution from I part
 *
 *****************************************************************************/
float PidBlock::CalcIterm(float pidError, float iMin, float iMax)
{
  if (mGi > 0.0f)
  {
    mIterm += mGp*mGi*pidError;

    // Anti windup
    if (mIterm < iMin)
    {
      mIterm = iMin;
    }
    if (mIterm > iMax)
    {
      mIterm = iMax;
    }
  }
  else
  {
    mIterm = 0;
  }

  return mIterm;
}

/*****************************************************************************
 * Function - CalcDterm
 * DESCRIPTION: Calculate the new contribution from D part
 *
 *****************************************************************************/
float PidBlock::CalcDterm(float pidError)
{
  float d_error = pidError-mOldPidError;
  mOldPidError = pidError;

  // Low pass filtering of the D error
  mDErrorFiltered += (mTs/D_ERROR_FILTER_TIME)*(d_error-mDErrorFiltered);
  d_error = mDErrorFiltered;

  // Limit the D error
  if (d_error > D_ERROR_MAX)
  {
    d_error = D_ERROR_MAX;
  }
  else if (-d_error > D_ERROR_MAX)
  {
    d_error = -D_ERROR_MAX;
  }

  return mGp*mGd*d_error;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
