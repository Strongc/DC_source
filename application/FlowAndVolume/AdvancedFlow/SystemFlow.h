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
/* FILE NAME        : SystemFlow.h                                          */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : System flow.                                    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef SystemFlow_h
#define SystemFlow_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "SystemFlowCnf.h"
#include "GeneralCnf.h"
#include "PitFlow.h"
#include "DynparIdent.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define FILTER_ON
#define MAX_NO_OF_PUMPS 6

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
// a pump model used for advanced flow calculation
typedef struct{
  float torque;
  float speed;
  bool  pumpRampUpPassed;
  AFC_PUMP_STATE_TYPE pumpState;
  t_calcvar theta[FLOW_DIM];
} AFC_PUMP_STRUCT;

// a structure containing all arguments for CalculatePumpFlow
typedef struct{
  float torque;
  float speed;
  float deltaPressure;
  AFC_PUMP_STATE_TYPE pumpState;
  t_calcvar theta[FLOW_DIM];
  bool pumpRampUpPassed;
} AFC_CALCULATE_PUMP_FLOW_STRUCT;

//a structure containing all arguments for RunSystemFlow
typedef struct{
  float pressureOut;
  float pressureIn;
  float pitArea;
  AFC_PUMP_STRUCT pump[MAX_NO_OF_PUMPS];
} AFC_RUN_SYSTEM_FLOW_STRUCT;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class SystemFlow
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SystemFlow(PitFlow* pPitFlow, DynparIdent* pDynparIdent);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SystemFlow();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /****************************************************************************
    * INPUT(S)             : Scale values. Only the max flow value is used.
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Function for initialization of the pit estimation
    *                        routine.
    ****************************************************************************/
    void InitSystemFlow(float maxFlowValue);

    /****************************************************************************
    * INPUT(S)             : Variables and parameters necessary for updating the
    *                        flow estimates.
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Function for updating the flow estimate.
    ****************************************************************************/
    void RunSystemFlow(AFC_RUN_SYSTEM_FLOW_STRUCT* pArguments);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Flow value.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return the inflow estimate.
    ****************************************************************************/
    float GetInflow(void);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Flow value.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return the pump flow estimate.
    ****************************************************************************/
    float GetPumpFlow(int pumpIndex);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    float CalculatePumpFlow(AFC_CALCULATE_PUMP_FLOW_STRUCT* pArguments);

    float CalculatePipeFlow(float qPipe, float pout, bool isAnyPumpRunning);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    float mQpump[MAX_NO_OF_PUMPS];
    float mQpumpFiltered[MAX_NO_OF_PUMPS];
    float mQpipe;
    float mQinFiltered;
    float mMaxFlow;
    float mPoutStop;
    U8   flow_runoff_timer;

    PitFlow* mpPitFlow;
    DynparIdent* mpDynparIdent;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};


#endif




