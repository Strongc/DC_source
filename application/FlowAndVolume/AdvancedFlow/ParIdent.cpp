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
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      : Module ParIdent                                       */
/*                                                                          */
/* FILE NAME        : ParIdent.c                                            */
/*                                                                          */
/* FILE DESCRIPTION : See h-file                                            */
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
#include <ParIdent.h>
#include "GeneralCnf.h"
#include "KalmanCalc.h"
#include "PitFlow.h"
#include "FlowLearnControl.h"         /* Included to check setup values */
#include "ParIdent.h"
#include "ParIdentCnf.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

#define FLOW_FILTER_CONSTANT   0.055f
#define PARIDENT_MIN_OFF_TIME  (int)(2.0f/(FLOW_FILTER_CONSTANT))    /* 3*tau to ensure correct value for the flow filter */
#define PARIDENT_QPIT_CHECK    (int)(2.0f/(PARIDENT_QPIT_FILTER1))

#if defined(MATLAB_TEST)
  #if LCTRL_MIN_DTIME_STARTS < (PARIDENT_MIN_OFF_TIME*SAMP_TIME)
    #error "Min delay time in PersistentControl < Min delay time in ParIdent."
  #endif
#endif

//#define PARIDENT_PRINT
//#define PRINT_PARIDENT_STATES

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
ParIdent::ParIdent(PitFlow* pPitFlow, KalmanCalc* pKalmanCalc)
{
  mOnCount = 0;
  mOffCount = 0;
  mStartUpDelayTimer = 0;
  mQinFlowEstimated = 0;
  mQinFlowStartValue = 0;

  mpPitFlow = pPitFlow;
  mpKalmanCalc = pKalmanCalc;

  learn_trainingstate = AFC_TRAINING_ON;
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION:
*
****************************************************************************/
ParIdent::~ParIdent()
{
}


/****************************************************************************
* INPUT(S)             : pointer to array with the 4 scale values
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function for initialization of the parameter
*                        estimate routine.
****************************************************************************/
void ParIdent::InitParIdent(float nominalFlow, float nominalPressure, float nominalTorque, float nominalSpeed)
{

  for (int i = 0; i < FLOW_DIM; i++)
  {
    mThetaQOutput[i] = 0.0f;
  }

  for (int i = 0; i < PRESSURE_DIM; i++)
  {
    mThetaPOutput[i] = 0.0f;
  }


  /*** Model parameters ***/
  ReInitTraining();

  /*** Set scaling values ***/
  mScales[0] = 1.0f / nominalFlow;
  mScales[1] = 1.0f / nominalPressure;
  mScales[2] = 1.0f / nominalTorque;
  mScales[3] = 1.0f / nominalSpeed;

  /*** Set start state ***/
  mPiState = S3_WAIT;

  /*** Pit flow evaluation ***/
  Q_pit_filter1 = 0.0f;
  Q_pit_max     = 0.0f;
  Q_pit_CUSUM   = 0.0f;

}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function for restarting the training of the model.
*                        This is to follow changes in the system.
****************************************************************************/
void ParIdent::ReInitTraining(void)
{
  int ii;

  /*** Initialization counter ***/
  mInitCount = 0;

  /*** Flow model parameters ***/
  for ( ii=0; ii<FLOW_DIM; ii++ )
  {
    mThetaQ[ii] = 0.0f;
    mThetaQOld[ii] = mThetaQ[ii];
  }

  for ( ii=0; ii<FLOW_DIM*FLOW_DIM; ii++ )
  {
    mPq[ii] = 0.0f;
  }
  mPq[0]  = 200.0f;
  mPq[5]  = 200.0f;
  mPq[10] = 200.0f;
  mPq[15] = 200.0f;

  for ( ii=0; ii<FLOW_DIM*FLOW_DIM; ii++ )
  {
    mPqOld[ii] = mPq[ii];
  }


  /*** Pressure model parameters ***/
  for ( ii=0; ii<PRESSURE_DIM; ii++ )
  {
    mThetaP[ii] = 0.0f;
    mThetaPOld[ii] = mThetaP[ii];
  }

  for ( ii=0; ii<PRESSURE_DIM*PRESSURE_DIM; ii++ )
  {
    mPp[ii] = 0.0f;
  }
  mPp[0]  = 200.0f;
  mPp[10] = 200.0f;
  mPp[20] = 200.0f;
  mPp[30] = 200.0f;
  mPp[40] = 200.0f;
  mPp[50] = 200.0f;
  mPp[60] = 200.0f;
  mPp[70] = 200.0f;
  mPp[80] = 200.0f;

  for ( ii=0; ii<PRESSURE_DIM*PRESSURE_DIM; ii++ )
  {
    mPpOld[ii] = mPp[ii];
  }


  /*** Confident limits ***/
  mFlowConfidenceMax        = 10.0f * PUMPPARCONFI_FLOW_LIMIT;
  mPressureConfidenceMax    = 10.0f * PUMPPARCONFI_PRES_LIMIT;
  mFlowConfidenceMaxOld     = mFlowConfidenceMax;
  mPressureConfidenceMaxOld = mPressureConfidenceMax;


}


/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : The values stored in the parameter vectors and the
*                        parameter identification matrices. The values are
*                        stored in one single storage vector.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Get the parameter vectors and the parameter
*                        identification matrices.
*****************************************************************************/
int ParIdent::GetForPersistentStorage(t_calcvar* pPersistentStorage, int dimension)
{
  int i,j,k;

  if ( dimension == PI_NUMBER_OF_STORED_PARAMETERS )
  {
    /*** Flow model storage ***/
    k = 0;
    for ( i=0; i<(FLOW_DIM); i++ )
    {
      pPersistentStorage[k] = mThetaQ[i];
      k++;
    }
    for ( i=0; i<(FLOW_DIM); i++ )
    {
      for ( j=i; j<(FLOW_DIM); j++ )
      {
        pPersistentStorage[k] = mPq[i*FLOW_DIM + j];
        k++;
      }
    }

    /*** Pressure model storage ***/
    for ( i=0; i<(PRESSURE_DIM); i++)
    {
      pPersistentStorage[k] = mThetaP[i];
      k++;
    }
    for ( i=0; i<(PRESSURE_DIM); i++ )
    {
      for ( j=i; j<(PRESSURE_DIM); j++ )
      {
        pPersistentStorage[k] = mPp[i*PRESSURE_DIM + j];
        k++;
      }
    }
    return PI_PARAMETER_STORE_OK;
  }
  else
  {
    return PI_PARAMETER_STORE_ERROR;
  }
}

/****************************************************************************
* INPUT(S)             : Values of the parameter vectors and the parameter
*                        identification matrices. The values are stored in one
*                        single storage vector.
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Set the parameteres to the values provided in the
*                        input.
*****************************************************************************/
int ParIdent::SetFromPersistentStorage(t_calcvar* pPersistentStorage, int dimension)
{
  int i,j,k;

  if ( dimension == PI_NUMBER_OF_STORED_PARAMETERS )
  {
    /*** Flow model storage ***/
    k = 0;
    for ( i=0; i<(FLOW_DIM); i++ )
    {
      mThetaQ[i] = pPersistentStorage[k];
      k++;
    }
    for ( i=0; i<(FLOW_DIM); i++ )
    {
      for ( j=i; j<(FLOW_DIM); j++ )
      {
        mPq[i*FLOW_DIM + j] = pPersistentStorage[k];
        if ( i != j )
        {
          mPq[j*FLOW_DIM + i] = pPersistentStorage[k];
        }
        k++;
      }
    }
    mFlowConfidenceMax    = 0.1f * PUMPPARCONFI_FLOW_LIMIT;
    mFlowConfidenceMaxOld = mFlowConfidenceMax;

    /*** Copy to parameter mirror ***/
    for ( i=0; i<(FLOW_DIM); i++ )
    {
      mThetaQOld[i] = mThetaQ[i];
    }
    for ( i=0; i<(FLOW_DIM*FLOW_DIM); i++ )
    {
      mPqOld[i] = mPq[i];
    }

    /*** Pressure model storage ***/
    for ( i=0; i<(PRESSURE_DIM); i++)
    {
      mThetaP[i] = pPersistentStorage[k];
      k++;
    }
    for ( i=0; i<(PRESSURE_DIM); i++ )
    {
      for ( j=i; j<(PRESSURE_DIM); j++ )
      {
        mPp[i*PRESSURE_DIM + j] = pPersistentStorage[k];
        if ( i != j )
        {
          mPp[j*PRESSURE_DIM + i] = pPersistentStorage[k];
        }
        k++;
      }
    }
    mPressureConfidenceMax    = 0.1f * PUMPPARCONFI_PRES_LIMIT;
    mPressureConfidenceMaxOld = mPressureConfidenceMax;

    /*** Copy to parameter mirror ***/
    for ( i=0; i<(PRESSURE_DIM); i++ )
    {
      mThetaPOld[i] = mThetaP[i];
    }
    for ( i=0; i<(PRESSURE_DIM*PRESSURE_DIM); i++ )
    {
      mPpOld[i] = mPp[i];
    }


    return PI_PARAMETER_STORE_OK;
  }
  else
  {
    return PI_PARAMETER_STORE_ERROR;
  }

}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function for updating the parameter estimate with
*                        respect to new measurements.
****************************************************************************/
void ParIdent::RunParUpdate(AFC_RUN_PAR_UPDATE_STRUCT* pArguments)
{

  float pinMeas          = pArguments->pressureIn;
  float poutMeas         = pArguments->pressureOut;
  float TMeas            = pArguments->torque;
  float wMeas            = pArguments->speed;
  bool  pumpRampUpPassed = pArguments->pumpRampUpPassed;
  AFC_PUMP_STATE_TYPE pumpState = pArguments->pumpState;


  //  printf("state=%d, (q=%f, Dp=%f, T=%f, w=%f), (pumpState=%d)\n",state,getPitFlow(),pout_meas-pin_meas,T_meas,w_meas,pumpState);

  /*** Initialization counter ***/
  mInitCount++;
  if ( mInitCount > 30000 )
  {
    mInitCount = 30000;
  }

  /*** Pit flow evaluation ***/
  PitFlowEvaluation();

  /*** State machine for parameter check ***/
  switch ( mPiState )
  {
    case S0_DATA_LOG :
      #if defined(MATLAB_TEST)
      test_var6 = 0.0;
      #endif
      if ( pumpState == AFC_PUMP_RUNNING )
      {
        mOnCount++;
        t_calcvar Q_kal, Dp_kal, T_kal, w_kal;
        Q_kal    = mScales[0] * (mQinFlowEstimated - mpPitFlow->GetPitFlow());
        Dp_kal   = mScales[1] * (poutMeas - pinMeas);
        T_kal    = mScales[2] * TMeas;
        w_kal    = mScales[3] * wMeas;
        ParameterEstimationForFlowPolynominal(Q_kal, Dp_kal, T_kal, w_kal);
        ParameterEstimationForPressurePolynominal(Q_kal, Dp_kal, T_kal, w_kal);
      }
      if ( pumpState == AFC_PUMP_STOPPED )
      {
        #if defined(PRINT_PARIDENT_STATES)
        printf(" pump stopped (%d) -> S2_SAVE_RECORD \n",mOnCount);
        #endif
        mPiState = S2_SAVE_RECORD;
      }
      if ( Q_pit_CUSUM > Q_pit_max )
      {
        #if defined(PRINT_PARIDENT_STATES)
        printf(" Training off, Bad flow behaviour -> S1_DECLINE_RECORD\n");
        #endif
        mPiState = S1_DECLINE_RECORD;
      }
      if ( mOnCount > PI_MAX_ON_TIME )
      {
        #if defined(PRINT_PARIDENT_STATES)
        printf(" Training off, (%d > %d) -> S1_DECLINE_RECORD\n",mOnCount,PI_MAX_ON_TIME);
        #endif
        mPiState = S1_DECLINE_RECORD;
      }
      if ( (AFC_TRAINING_OFF == pArguments->trainingState) || (AFC_TRAINING_OFF == learn_trainingstate) )
      {
        #if defined(PRINT_PARIDENT_STATES)
        printf(" Training off, training switch off (extern: %d, ctrl: %d /%d) -> S1_DECLINE_RECORD\n",pArguments->trainingState,learn_trainingstate,AFC_TRAINING_OFF);
        #endif
        mPiState = S1_DECLINE_RECORD;
      }
      break;

    case S1_DECLINE_RECORD :
      #if defined(MATLAB_TEST)
      test_var6 = 1.0;
      #endif
      mQinFlowEstimated = (1.0f-FLOW_FILTER_CONSTANT) * mQinFlowEstimated + FLOW_FILTER_CONSTANT * mpPitFlow->GetPitFlow();
      if ( pumpState == AFC_PUMP_RUNNING )
      {
        mOnCount++;
        //Q_kal    = mScales[0] * (mQinFlowEstimated - mpPitFlow->GetPitFlow());
        //w_kal    = mScales[3] * wMeas;
      }
      if ( pumpState == AFC_PUMP_STOPPED )
      {
        mOffCount = 0;
        mpKalmanCalc->CopyMatrices(mThetaQ, mPq, mThetaQOld, mPqOld, FLOW_DIM);
        mpKalmanCalc->CopyMatrices(mThetaP, mPp, mThetaPOld, mPpOld, PRESSURE_DIM);
        mFlowConfidenceMax     = mFlowConfidenceMaxOld;
        mPressureConfidenceMax = mPressureConfidenceMaxOld;
        mPiState = S3_WAIT;
        #if defined(PRINT_PARIDENT_STATES)
        printf(" Data rejected -> S3_WAIT\n");
        #endif
      }
//      if ( (pumpState == AFC_PUMP_RUNNING) && (AFC_TRAINING_ON == learn_trainingstate) && (AFC_TRAINING_ON == pArguments->trainingState) )
//      {
//        mpKalmanCalc->CopyMatrices(mThetaQ, mPq, mThetaQOld, mPqOld, FLOW_DIM);
//        mpKalmanCalc->CopyMatrices(mThetaP, mPp, mThetaPOld, mPpOld, PRESSURE_DIM);
//        mFlowConfidenceMax     = mFlowConfidenceMaxOld;
//        mPressureConfidenceMax = mPressureConfidenceMaxOld;
//        mStartUpDelayTimer = 0;
//        mOnCount = 0;
//        mQinFlowStartValue = mQinFlowEstimated;
//        mPiState = S0_DATA_LOG;
//        #if defined(PRINT_PARIDENT_STATES)
//        printf(" Data rejected -> S0_DATA_LOG\n");
//        #endif
//      }
      break;

    case S2_SAVE_RECORD :
      #if defined(MATLAB_TEST)
      test_var6 = 0.0;
      #endif
  //    printf("  ---------offtime=%d, filtercount=%d, maxfiltercount=%d\n",mOffCount,mStartUpDelayTimer,FILTER_COUNT);
      mStartUpDelayTimer++;
      if ( (pumpState == AFC_PUMP_STOPPED) && (AFC_TRAINING_ON == pArguments->trainingState) )
      {
        mQinFlowEstimated = (1.0f-FLOW_FILTER_CONSTANT) * mQinFlowEstimated + FLOW_FILTER_CONSTANT * mpPitFlow->GetPitFlow();
        //printf("pumpstate (%d/%d, %d/%d): Qin=%f,  Qfilt=%f\n",pumpState,AFC_PUMP_STOPPED,pArguments->trainingState,AFC_TRAINING_ON,mpPitFlow->GetPitFlow(),mQinFlowEstimated);
      }
      if ( (mStartUpDelayTimer > PARIDENT_MIN_OFF_TIME) || (pumpState == AFC_PUMP_RUNNING) || (AFC_TRAINING_OFF == pArguments->trainingState) )
      {
        if ( (mScales[0] * fabs(mQinFlowEstimated - mQinFlowStartValue) < PI_MAX_FLOW_CHANGE) && (mOffCount >= PARIDENT_MIN_OFF_TIME) )
        {
          mpKalmanCalc->CopyMatrices(mThetaQOld, mPqOld, mThetaQ, mPq, FLOW_DIM);
          mpKalmanCalc->CopyMatrices(mThetaPOld, mPpOld, mThetaP, mPp, PRESSURE_DIM);
          mFlowConfidenceMaxOld     = mFlowConfidenceMax;
          mPressureConfidenceMaxOld = mPressureConfidenceMax;
          mPiState = S3_WAIT;
          #if defined(PRINT_PARIDENT_STATES)
          printf("Save parameters: DQ=%f < %f -> S3_WAIT\n",(mQinFlowEstimated - mQinFlowStartValue),PI_MAX_FLOW_CHANGE/mScales[0] );
          #endif
        }
        if ( mScales[0] * fabs(mQinFlowEstimated - mQinFlowStartValue) >= PI_MAX_FLOW_CHANGE )
        {
          #if defined(PRINT_PARIDENT_STATES)
          printf("Reject on flow change: offtime=%d,  DQ: |%f| > %f -> S1_DECLINE_RECORD\n",mOffCount,(mQinFlowEstimated - mQinFlowStartValue),PI_MAX_FLOW_CHANGE/mScales[0]);
          #endif
          mPiState = S1_DECLINE_RECORD;
        }
        if ( mOffCount < PARIDENT_MIN_OFF_TIME )
        {
          #if defined(PRINT_PARIDENT_STATES)
          printf("Reject on offtime: DT=%d < offtime=%d -> S1_DECLINE_RECORD\n",PARIDENT_MIN_OFF_TIME,mOffCount);
          #endif
          mPiState = S1_DECLINE_RECORD;
        }
        mOffCount = mStartUpDelayTimer;
        mStartUpDelayTimer = 0;
      }
      break;

    case S3_WAIT :
      mOffCount++;
      if ( pumpState == AFC_PUMP_RUNNING )
      {
        mStartUpDelayTimer++;
      }
      else if ( (pumpState == AFC_PUMP_STOPPED) && (AFC_TRAINING_ON == pArguments->trainingState) )
      {
        mQinFlowEstimated = (1.0f-FLOW_FILTER_CONSTANT) * mQinFlowEstimated + FLOW_FILTER_CONSTANT * mpPitFlow->GetPitFlow();
        //printf("pumpstate (%d/%d, %d/%d): Qin=%f,  Qfilt=%f\n",pumpState,AFC_PUMP_STOPPED,pArguments->trainingState,AFC_TRAINING_ON,mpPitFlow->GetPitFlow(),mQinFlowEstimated);
      }
      if ( (pumpState == AFC_PUMP_RUNNING) && (pumpRampUpPassed == true) && (mStartUpDelayTimer > PI_STARTDELAY) && (mInitCount > 60) )
      {
        mStartUpDelayTimer = 0;
        mOnCount = 0;
        mQinFlowStartValue = mQinFlowEstimated;
  //      printf("Parindent start: Qin=%f, offtime=%d -> S0_DATA_LOG\n ",mQinFlowEstimated,mOffCount);
        mPiState = S0_DATA_LOG;
      }
      break;
  }
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : .
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : .
****************************************************************************/
PARAMETER_IDENTIFICATION_STATUS_TYPE ParIdent::GetStateOfParametersData(void)
{
  if ( mPiState == S1_DECLINE_RECORD )
  {
    //printf("state=%d ~ %d\n",mPiState,S1_DECLINE_RECORD);
    return RECORD_DECLINED;
  }
  else
  {
    //printf("state=%d ~ %d\n",mPiState,S1_DECLINE_RECORD);
    return RECORD_OK;
  }
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Parameter for the flow polynomial.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the 4 parameters of the
*                        flow polynomial.
****************************************************************************/
t_calcvar* ParIdent::GetPumpFlowParameters(void)
{
  if (mFlowConfidenceMax < PUMPPARCONFI_FLOW_LIMIT)
  {
    mThetaQOutput[0] = mThetaQ[0]              / (mScales[0] * mScales[3]);
    mThetaQOutput[1] = mThetaQ[1] * mScales[1] / (mScales[0] * mScales[3]);
    mThetaQOutput[2] = mThetaQ[2] * mScales[2] / (mScales[0] * mScales[3]);
    mThetaQOutput[3] = mThetaQ[3] * mScales[3] /  mScales[0];
  }

  return mThetaQOutput;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Parameter for the flow polynomial.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the parameters of the
*                        flow polynomial.
****************************************************************************/
void ParIdent::GetPumpFlowPmatrix(t_calcvar* pScale, int dimensions)
{
  t_calcvar s_a[FLOW_DIM];

  if (dimensions == FLOW_DIM)
  {
    s_a[0] = 1.0f / mScales[3];
    s_a[1] = mScales[1] / mScales[3];
    s_a[2] = mScales[2] / mScales[3];
    s_a[3] = mScales[3];

    for (int i = 0; i < FLOW_DIM; i++)
    {
      for (int j = 0; j < FLOW_DIM; j++)
      {
        pScale[i * FLOW_DIM + j] = s_a[i] * mPq[i * FLOW_DIM + j] * s_a[j];
      }
    }
  }
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Number showning the confidence of the identified
*                        parmaeters: - smaller than 0 implies that the
*                        parmaeters can be used in the flow estimation
*                        routine.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the aravage
*                        confidentiality in parameters of the flow
*                        polynomial.
****************************************************************************/
float ParIdent::GetPumpModelConfidence(void)
{
  float confidence;

  confidence = 0.0f;
  if ( mPressureConfidenceMaxOld >= (PUMPPARCONFI_PRES_LIMIT) )
  {
    confidence = mPressureConfidenceMaxOld - (PUMPPARCONFI_PRES_LIMIT);
  }
  if ( mFlowConfidenceMaxOld >= (PUMPPARCONFI_FLOW_LIMIT) )
  {
    confidence = confidence + mFlowConfidenceMaxOld - (PUMPPARCONFI_FLOW_LIMIT);
  }
  if ( confidence <= 0.0f)
  {
    confidence = -1.0f;
  }
  return confidence;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Parameter for the pressure polynomial.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the 9 parameters of the
*                        pressure polynomial.
****************************************************************************/
t_calcvar* ParIdent::GetPumpPresParameters(void)
{
  if ( mPressureConfidenceMax < (PUMPPARCONFI_PRES_LIMIT) )
  {
    /* The scaling vector: (s_q,  s_Dp,  s_T,  s_w) */
    mThetaPOutput[0] = mThetaP[0] / (mScales[1] * mScales[1]);
    mThetaPOutput[1] = mThetaP[1] / (mScales[1]);
    mThetaPOutput[2] = mThetaP[2] * mScales[2]  / (mScales[1] * mScales[1]);
    mThetaPOutput[3] = mThetaP[3] * mScales[2] * mScales[2] / (mScales[1] * mScales[1]);
    mThetaPOutput[4] = mThetaP[4] * mScales[2] / (mScales[1]);
    mThetaPOutput[5] = mThetaP[5] * mScales[3] * mScales[3] * mScales[3] * mScales[3] / (mScales[1] * mScales[1]);
    mThetaPOutput[6] = mThetaP[6] * mScales[3] * mScales[3] / (mScales[1]);
    mThetaPOutput[7] = mThetaP[7] * mScales[2] * mScales[3] * mScales[3] / (mScales[1] * mScales[1]);
    mThetaPOutput[8] = mThetaP[8] * mScales[3] * mScales[3] / (mScales[1] * mScales[1]);
  }

  return mThetaPOutput;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Parameter for the pressure polynomial.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the parameters of the
*                        pressure polynomial.
****************************************************************************/
void ParIdent::GetPumpPresPmatrix(t_calcvar* pScale, int dimensions)
{
  t_calcvar s_a[PRESSURE_DIM];

  if (dimensions == PRESSURE_DIM )
  {
    /* The scaling vector: (s_q,  s_Dp,  s_T,  s_w) */
    s_a[0] = 1.0f;
    s_a[1] = mScales[1];
    s_a[2] = mScales[2];
    s_a[3] = mScales[2] * mScales[2];
    s_a[4] = mScales[1] * mScales[2];
    s_a[5] = mScales[3] * mScales[3] * mScales[3] * mScales[3];
    s_a[6] = mScales[1] * mScales[3] * mScales[3];
    s_a[7] = mScales[2] * mScales[3] * mScales[3];
    s_a[8] = mScales[3] * mScales[3];

    for (int i = 0; i < PRESSURE_DIM; i++ )
    {
      for (int j = 0; j < PRESSURE_DIM; j++ )
      {
        pScale[i * PRESSURE_DIM + j] = s_a[i] * mPp[i * PRESSURE_DIM + j] * s_a[j];
      }
    }
  }
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Parameter for the pressure polynomial.
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Return function returning the parameters of the
*                        pressure polynomial.
****************************************************************************/
void ParIdent::SetTrainingState(AFC_TRAINING_STATE_TYPE learn_trainingstate_in)
{
  learn_trainingstate = learn_trainingstate_in;
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
* FUNCTION DESCRIPTION : Evaluate the pit flow behaviour to ensure proper
*                        parameter identification.
****************************************************************************/
void ParIdent::PitFlowEvaluation(void)
{
  t_calcvar Q_pit;
  t_calcvar Q_pit_res;


  /*** Residual calcualtion ***/
  Q_pit         = mpPitFlow->GetPitFlow();
  Q_pit_filter1 = (1.0f - PARIDENT_QPIT_FILTER1)*Q_pit_filter1 + PARIDENT_QPIT_FILTER1*Q_pit;

  /*** Threshold calculation ***/
  if ( mInitCount > 60 )
  {
    if ( fabs(Q_pit_filter1) > Q_pit_max )
    {
      Q_pit_max = (1.0f - PARIDENT_QPIT_MAX_FAST)*Q_pit_max + PARIDENT_QPIT_MAX_FAST*fabs(Q_pit);
    }
    else
    {
      Q_pit_max = (1.0f - PARIDENT_QPIT_MAX_SLOW)*Q_pit_max + PARIDENT_QPIT_MAX_SLOW*fabs(Q_pit);
    }
  }
  else
  {
    Q_pit_max = 0.0;
  }

  /*** Residual evaluation ***/
  Q_pit_res   = Q_pit - Q_pit_filter1;
  if ( mPiState == S0_DATA_LOG )
  {
    Q_pit_CUSUMp = Q_pit_CUSUMp + PARIDENT_QPIT_CUSUM*(Q_pit_res - 0.01*Q_pit_max);
    if ( Q_pit_CUSUMp > 2.0f*Q_pit_max )
    {
      Q_pit_CUSUMp = 2.0f*Q_pit_max;
    }
    else if ( Q_pit_CUSUMp < 0.0f )
    {
      Q_pit_CUSUMp = 0.0f;
    }
    if ( mOnCount > 60 )
    {
      Q_pit_CUSUMm = Q_pit_CUSUMm + PARIDENT_QPIT_CUSUM*(-Q_pit_res - 0.01*Q_pit_max);
      if ( Q_pit_CUSUMm > 2.0f*Q_pit_max )
      {
        Q_pit_CUSUMm = 2.0f*Q_pit_max;
      }
      else if ( Q_pit_CUSUMm < 0.0f )
      {
        Q_pit_CUSUMm = 0.0f;
      }
    }
    else
    {
      Q_pit_CUSUMm = 0.0f;
    }
  }
  else
  {
    Q_pit_CUSUMm = 0.0f;
    Q_pit_CUSUMp = 0.0f;
  }
  Q_pit_CUSUM = Q_pit_CUSUMp + Q_pit_CUSUMm;

  //printf("(%d==%d)?: Qpit=%f -> Qpit_filt=%f -> Qpit_res=%f and Qpit_max=%f -> Qpit_CUSUM = %f\n",mPiState,S0_DATA_LOG,Q_pit,Q_pit_filter1,Q_pit_res,Q_pit_max,Q_pit_CUSUM);

  #if defined(MATLAB_TEST)
  test_var1 = Q_pit;
  test_var2 = Q_pit_filter1;
  test_var3 = Q_pit_res;
  test_var4 = Q_pit_max;
  test_var5 = Q_pit_CUSUM;
  #endif

}



/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Executing parameter estimation for the flow
*                       polynomial.
****************************************************************************/
void ParIdent::ParameterEstimationForFlowPolynominal(t_calcvar Q_Kal, t_calcvar Dp_kal, t_calcvar T_kal, t_calcvar w_kal)
{
  t_calcvar Cq[FLOW_DIM];
  float confidence;

  if ( w_kal > 0.0f )
  {
    /*** Setup variable vector ***/
    Cq[0] = 1.0f / w_kal;
    Cq[1] = Dp_kal / w_kal;
    Cq[2] = T_kal / w_kal;
    Cq[3] = w_kal;

    /*** Execute kalman filter ***/
    confidence = mpKalmanCalc->ConfidenceOfKalman(mPq, Cq, FLOW_DIM);
    if ( confidence > (SCALE_PUMPPARCONFI_FLOW_LIMIT)*(PUMPPARCONFI_FLOW_LIMIT) )
    {
      mpKalmanCalc->UpdateKalman(mThetaQ, mPq, Cq, Q_Kal, R_Q_0, FLOW_DIM);
      #if defined(PARIDENT_PRINT)
      printf("ParameterEstimationForFlowPolynominal():\n");
      printf("C = [%f, %f, %f, %f];\n\n",Cq[0],Cq[1],Cq[2],Cq[3]);
      printf("theta = [%f, %f, %f, %f];\n\n",mThetaQ[0],mThetaQ[1],mThetaQ[2],mThetaQ[3]);
      printf("P     = [%f, %f, %f, %f; ...\n"  ,mPq[0], mPq[1], mPq[2], mPq[3]);
      printf("         %f, %f, %f, %f; ...\n"  ,mPq[4], mPq[5], mPq[6], mPq[7]);
      printf("         %f, %f, %f, %f; ...\n"  ,mPq[8], mPq[9], mPq[10],mPq[11]);
      printf("         %f, %f, %f, %f];\n"  ,mPq[12],mPq[13],mPq[14],mPq[15]);
      printf("--------\n\n");
      #endif
    }

    /*** max filter ***/
    if ( mFlowConfidenceMax < confidence )
    {
      mFlowConfidenceMax = (1.0f - PUMPPARCONFI_MAX_FAST) * mFlowConfidenceMax + (PUMPPARCONFI_MAX_FAST * confidence);
    }
    else
    {
      mFlowConfidenceMax = (1.0f - PUMPPARCONFI_MAX_SLOW) * mFlowConfidenceMax + (PUMPPARCONFI_MAX_SLOW * confidence);
    }
    if ( mFlowConfidenceMax > (10.0f * PUMPPARCONFI_FLOW_LIMIT) )
    {
      mFlowConfidenceMax = 10.0f * PUMPPARCONFI_FLOW_LIMIT;
    }

    #if defined(PARIDENT_PRINT)
    printf("point:  (q,w,Dp,T) = (%f, %f, %f, %f) -> confidence q = %f / %f  -> ",Q_Kal,w_kal,Dp_kal,T_kal,confidence,mFlowConfidenceMax);
    printf("qest=%f\n",Cq[0]*mThetaQ[0]+Cq[1]*mThetaQ[1]+Cq[2]*mThetaQ[2]+Cq[3]*mThetaQ[3]);
    printf("%f, %f, %f, %f; ... \n",Q_Kal,w_kal,Dp_kal,T_kal);
    #endif
  }
}


/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Executing parameter estimation for the pressure
*                       polynomial.
****************************************************************************/
void ParIdent::ParameterEstimationForPressurePolynominal(t_calcvar Q_Kal, t_calcvar Dp_kal, t_calcvar T_kal, t_calcvar w_kal)
{
  t_calcvar Cp[PRESSURE_DIM];
  float confidence;

  /*** Setup variable vector ***/
  Cp[0] = -1.0f;
  Cp[1] = -Dp_kal;
  Cp[2] = -T_kal;
  Cp[3] = -T_kal * T_kal;
  Cp[4] = -Dp_kal * T_kal;
  Cp[5] = -w_kal * w_kal * w_kal * w_kal;
  Cp[6] = -Dp_kal * w_kal * w_kal;
  Cp[7] = -T_kal * w_kal * w_kal;
  Cp[8] = -w_kal * w_kal;

  /*** Execute kalman filter ***/
  confidence = mpKalmanCalc->ConfidenceOfKalman(mPp, Cp, PRESSURE_DIM);
  if ( confidence > (SCALE_PUMPPARCONFI_PRES_LIMIT)*(PUMPPARCONFI_PRES_LIMIT) )
  {
    mpKalmanCalc->UpdateKalman(mThetaP, mPp, Cp, Dp_kal * Dp_kal, R_P_0, PRESSURE_DIM);
  }

  /*** max filter ***/
  if ( mPressureConfidenceMax < confidence )
  {
    mPressureConfidenceMax = (1.0f - (PUMPPARCONFI_MAX_FAST)) * mPressureConfidenceMax + (PUMPPARCONFI_MAX_FAST) * confidence;
  }
  else
  {
    mPressureConfidenceMax = (1.0f - (PUMPPARCONFI_MAX_SLOW)) * mPressureConfidenceMax + (PUMPPARCONFI_MAX_SLOW) * confidence;
  }
  if ( mPressureConfidenceMax > 10.0f * (PUMPPARCONFI_PRES_LIMIT) )
  {
    mPressureConfidenceMax = 10.0f * (PUMPPARCONFI_PRES_LIMIT);
  }

}

/*****************************************************************************
*
*
*              PROTECTED FUNCTIONS
*                 - RARE USED -
*
****************************************************************************/

