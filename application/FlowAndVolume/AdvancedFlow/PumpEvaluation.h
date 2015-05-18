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
/* FILE NAME        : PumpEvaluation.h                                      */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Pump evaluation.                                */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef PumpEvaluation_h
#define PumpEvaluation_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PumpEvaluationCnf.h"
#include "GeneralCnf.h"
#include "PumpCurves.h"
#include "ParIdent.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MODEL_SIZE      4
#define SYSP_SIZE       9
#define SYSQ_SIZE       4

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PumpEvaluation
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PumpEvaluation(ParIdent* pParameterIdentifier, PumpCurves* pPumpCurves);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PumpEvaluation();
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
    * FUNCTION DESCRIPTION : Initialization of the pump evaluation.
    ****************************************************************************/
    void InitPumpEvaluation(void);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Updating the stored parameters. These parameters
    *                        used as reference model when duing fault evaluation.
    ****************************************************************************/
    bool LogDataSet(void);

    /****************************************************************************
    * INPUT(S)             : Measurements.
    * OUTPUT(S)            : Decision of fault evaluation.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the result of evaluation
    *                        for fault based on a structural residaul evaluation.
    ****************************************************************************/
    float ModelEvaluation(float deltaPressure, float torque, float speed, AFC_PUMP_STATE_TYPE pumpState);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Reset the model used in the model evaluation. This
    *                        is the same as resetting the fault evaluation
    *                        signal.
    *****************************************************************************/
    void ResetModelEvaluation(void);

    /****************************************************************************
    * INPUT(S)             : Measurements.
    * OUTPUT(S)            : Loss of efficiency compared to the reference model.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return the loss of efficiency between the actual
    *                        operating conditions and the the efficiency
    *                        calculated based on the reference model.
    ****************************************************************************/
    float EfficiencyEvaluation(float deltaPressure, float torque, float speed, float flow, AFC_PUMP_STATE_TYPE pumpState, float pitArea);

    /****************************************************************************
    * INPUT(S)             : Measurements.
    * OUTPUT(S)            : Get model efficiency.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the efficiency at the
    *                        given operating point calculated based on the
    *                        identified model parameters.
    ****************************************************************************/
    float GetModelEfficiency(float speed, float flow, float pitArea);

    /****************************************************************************
    * INPUT(S)             : Measurements.
    * OUTPUT(S)            : Get actual efficiency.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the actual efficiency
    *                        based on the available measurements.
    ****************************************************************************/
    float GetEfficiency(float deltaPressure, float torque, float speed, float flow);

    /****************************************************************************
    * INPUT(S)             : Measurements.
    * OUTPUT(S)            : Get actual efficiency.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the actual efficiency
    *                        based on the available measurements.
    ****************************************************************************/
    float GetEfficiency2(float speed, float flow, float pitArea);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    t_calcvar mPressureParameters[MODEL_SIZE];
    t_calcvar mPowerParameters[MODEL_SIZE];
    t_calcvar mSystemParameters[SYSP_SIZE];

    PumpCurves* mpPumpCurves;
    ParIdent*   mpParameterIdentifier;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};


#endif


