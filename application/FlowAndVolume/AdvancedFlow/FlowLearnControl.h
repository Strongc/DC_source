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
#ifndef FlowLearnControl_h
#define FlowLearnControl_h
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
#include "ParIdent.h"
#include "FlowLearnControlCnf.h"
#include "PitFlow.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum {
  LCTRL_INIT,
  LCTRL_WAIT,
  LCTRL_FIRST_RUN,
  LCTRL_DELAY,
  LCTRL_SECOND_RUN,
  LCTRL_EMPTY_PIT
} LCTRL_STATE_TYPE;

typedef struct {
  t_calcvar level;
  t_calcvar speed;
  AFC_PUMP_STATE_TYPE pumpState;
} AFC_RUN_FLOW_LEARN_STRUCT;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class FlowLearnControl
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FlowLearnControl(ParIdent* pParIdent, PitFlow* pPitFlow);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FlowLearnControl();
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
    void SetFlowLearnParameters( float level_l0, float level_l1, float in_min_speed, float in_max_speed );


    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Function for initialization of the parameter
     *                        estimate routine.
    ****************************************************************************/
    void CtrlFlowLearnControl( AFC_RUN_FLOW_LEARN_STRUCT *flow_learn_struct );


    /****************************************************************************
     * INPUT(S)             :
     * OUTPUT(S)            :
     * DESIGN DOC.          :
     * FUNCTION DESCRIPTION : Function for initialization of the parameter
     *                        estimate routine.
    ****************************************************************************/
    float GetFlowLearnFrequency( void );


    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Function for restarting the training proceedure.
    ****************************************************************************/
    void ReInitLearning( void );


  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void PrintLearningStateChange( LCTRL_STATE_TYPE old_persistent_state, LCTRL_STATE_TYPE new_persistent_state );

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    LCTRL_STATE_TYPE persistent_state;

    int mOnCount;
    int mOffCount;

    int nr_firstruns;

    int step_nr;
    float min_speed;
    float max_speed;
    float min_speed_firstrun;
    float min_speed_secondrun;

    float level_start;
    float level_learn;
    float level_stop;

    float speed_ref;

    float pit_flow;

    ParIdent *mpParIdent;
    PitFlow  *mpPitFlow;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif

