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
/* FILE NAME        : DynparIdent.h                                         */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Dynamic parameter identification.               */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef DynparIdent_h
#define DynparIdent_h

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
#include "DynparIdentCnf.h"
#include "PitFlow.h"
#include "KalmanCalc.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define PIPEDYN_DIM  3

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum {
  S2_DATA_LOG,       // 0
  S3_DECLINE_RECORD, // 1
  S4_SAVE_RECORD,    // 2
  S5_WAIT            // 3
} DPI_STATE_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class DynparIdent
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    DynparIdent(KalmanCalc* pKalmanCalc);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~DynparIdent();
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
    void InitDynparIdent(float nominalFlow, float nominalPressure);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Function for updating the parameter estimate with
    *                        respect to new measurements.
    ****************************************************************************/
    void RunDynparUpdate(float poutMeas, float qPumps, bool isAnyPumpRunning);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Parameter for the pressure polynomial.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the parameters of the
    *                        pressure polynomial.
    ****************************************************************************/
    t_calcvar* GetDynpar(void);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Parameter for the pressure polynomial.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the parameters of the
    *                        pressure polynomial.
    ****************************************************************************/
    t_calcvar* GetPdynpar(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void ParameterEstimationOfPipeDynamic(t_calcvar qKal, t_calcvar qKalOld, t_calcvar qKal2Old, t_calcvar poutKal);


    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    DPI_STATE_TYPE mState;
    
    t_calcvar mPdyn[PIPEDYN_DIM * PIPEDYN_DIM];
    t_calcvar mPdynOld[PIPEDYN_DIM * PIPEDYN_DIM];

    t_calcvar mThetaDyn[PIPEDYN_DIM];
    t_calcvar mThetaDynOld[PIPEDYN_DIM];
    t_calcvar mThetaDynOut[PIPEDYN_DIM];

    float mConfidenceMax;

    float mPoutZero;

    t_calcvar mScaleFlow;
    t_calcvar mScalePressure;

    int mOnCount;
    int mStartUpDelayTimer;
    t_calcvar mQKalOld, mQKal2Old, mPoutKalOld;
    t_calcvar mQKal, mQKal2, mPoutKal;

    KalmanCalc* mpKalmanCalc;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};


#endif
