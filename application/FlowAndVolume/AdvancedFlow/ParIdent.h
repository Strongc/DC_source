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
/* CLASS NAME       : ParIdent                                              */
/*                                                                          */
/* FILE NAME        : ParIdent.h                                            */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Algorithm for parameter identification.         */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef ParIdent_h
#define ParIdent_h
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "GeneralCnf.h"
#include "ParIdentCnf.h"
#include "PitFlow.h"
#include "KalmanCalc.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum {
  S0_DATA_LOG,
  S1_DECLINE_RECORD,
  S2_SAVE_RECORD,
  S3_WAIT
} PARAMETER_IDENTIFICATION_STATE_TYPE;

typedef enum {
  RECORD_OK,
  RECORD_DECLINED
} PARAMETER_IDENTIFICATION_STATUS_TYPE;

// a structure containing all arguments for RunParUpdate
typedef struct {
  t_calcvar pressureIn;
  t_calcvar pressureOut;
  t_calcvar torque;
  t_calcvar speed;
  bool      pumpRampUpPassed;
  AFC_PUMP_STATE_TYPE pumpState;
  AFC_TRAINING_STATE_TYPE trainingState;
} AFC_RUN_PAR_UPDATE_STRUCT;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class ParIdent
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ParIdent(PitFlow* pPitFlow, KalmanCalc* pKalmanCalc);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ParIdent();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Function for initialization of the parameter
     *                        estimate routine.
    ****************************************************************************/
    void InitParIdent(float nominalFlow, float nominalPressure, float nominalTorque, float nominalSpeed);

    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Function for restarting the training of the model.
     *                        This is to follow changes in the system.
     ****************************************************************************/
    void ReInitTraining(void);

    /****************************************************************************
     * INPUT(S)             : Dimension of the persistance storage vector.
     * OUTPUT(S)            : - The values stored in the parameter vectors and the
     *                        parameter identification matrices. The values are
     *                        stored in one single storage vector.
     *                        - And an error code if the parameters are collected
     *                        correctly.
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Get the parameter vectors and the parameter
     *                        identification matrices.
     *****************************************************************************/
    int GetForPersistentStorage(t_calcvar* pPersistanceStorage, int dimension);

    /****************************************************************************
     * INPUT(S)             : - Values of the parameter vectors and the parameter
     *                        identification matrices. The values are stored in one
     *                        single storage vector.
     *                        - Dimension of the persistance storage vector.
     * OUTPUT(S)            : And an error code if the parameters are collected
     *                        correctly.
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Set the parameteres to the values provided in the
     *                        input.
     *****************************************************************************/
    int SetFromPersistentStorage(t_calcvar* pPersistanceStorage, int dimension);

    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Function for updating the parameter estimate with
     *                        respect to new measurements.
    ****************************************************************************/
    void RunParUpdate(AFC_RUN_PAR_UPDATE_STRUCT* pArguments);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : .
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : .
    ****************************************************************************/
    PARAMETER_IDENTIFICATION_STATUS_TYPE GetStateOfParametersData(void);

    /****************************************************************************
     * INPUT(S)             : Parameter for the flow polynomial.
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Return function returning the parameters of the
     *                        flow polynomial.
    ****************************************************************************/
    t_calcvar* GetPumpFlowParameters(void);

    /****************************************************************************
     * INPUT(S)             : Parameter for the flow polynomial.
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Return function returning the parameters of the
     *                        flow polynomial.
    ****************************************************************************/
    void GetPumpFlowPmatrix(t_calcvar *pScale, int dimensions);

    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            : Parameter for the arevage confidentiality in the
     *                        flow polynomial.
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Return function returning the aravage
     *                        confidentiality in parameters of the flow
     *                        polynomial.
    ****************************************************************************/
    float GetPumpModelConfidence(void);

    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            : Parameter for the pressure polynomial.
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Return function returning the parameters of the
     *                        pressure polynomial.
    ****************************************************************************/
    t_calcvar* GetPumpPresParameters(void);

    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            : Parameter for the pressure polynomial.
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Return function returning the parameters of the
     *                        pressure polynomial.
    ****************************************************************************/
    void GetPumpPresPmatrix(t_calcvar* pScale, int dimensions);

  /****************************************************************************
  * INPUT(S)             :
  * OUTPUT(S)            : Parameter for the pressure polynomial.
  * DESIGN DOC.          :
  * FUNCTION DESCRIPTION : Return function returning the parameters of the
  *                        pressure polynomial.
  ****************************************************************************/
  void SetTrainingState(AFC_TRAINING_STATE_TYPE learn_trainingstate_in);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void PitFlowEvaluation(void);
    void ParameterEstimationForFlowPolynominal(t_calcvar Q_Kal, t_calcvar Dp_kal, t_calcvar T_kal, t_calcvar w_kal);
    void ParameterEstimationForPressurePolynominal(t_calcvar Q_Kal, t_calcvar Dp_kal, t_calcvar T_kal, t_calcvar w_kal);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    PARAMETER_IDENTIFICATION_STATE_TYPE mPiState;

    t_calcvar mPq[FLOW_DIM*FLOW_DIM];
    t_calcvar mPqOld[FLOW_DIM*FLOW_DIM];
    t_calcvar mThetaQ[FLOW_DIM];
    t_calcvar mThetaQOld[FLOW_DIM];
    t_calcvar mThetaQOutput[FLOW_DIM];

    t_calcvar mPp[PRESSURE_DIM*PRESSURE_DIM];
    t_calcvar mPpOld[PRESSURE_DIM*PRESSURE_DIM];
    t_calcvar mThetaP[PRESSURE_DIM];
    t_calcvar mThetaPOld[PRESSURE_DIM];
    t_calcvar mThetaPOutput[PRESSURE_DIM];

    float mScales[4];

    float mFlowConfidenceMax;
    float mPressureConfidenceMax;
    float mFlowConfidenceMaxOld;
    float mPressureConfidenceMaxOld;

    int mInitCount;
    int mOnCount;
    int mOffCount;
    int mStartUpDelayTimer;
    t_calcvar mQinFlowEstimated;
    t_calcvar mQinFlowStartValue;

    PitFlow* mpPitFlow;
    KalmanCalc* mpKalmanCalc;

    t_calcvar Q_pit_filter1;
    t_calcvar Q_pit_max;
    t_calcvar Q_pit_CUSUMp;
    t_calcvar Q_pit_CUSUMm;
    t_calcvar Q_pit_CUSUM;

    AFC_TRAINING_STATE_TYPE learn_trainingstate;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif

