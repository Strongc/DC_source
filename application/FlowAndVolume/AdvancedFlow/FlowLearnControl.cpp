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
/* MODULE NAME      : Module PersistentControl                              */
/*                                                                          */
/* FILE NAME        : PersistentControl.h                                   */
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
#include <FlowLearnControl.h>
#include "GeneralCnf.h"
#include "ParIdent.h"
#include "PitFlow.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define LCTRL_MIN_DCOUNT_STARTS       (int)(LCTRL_MIN_DTIME_STARTS/SAMP_TIME)
#define LCTRL_MAX_FIRST_RUN_COUNT     (int)(LCTRL_MAX_FIRST_RUN_TIME/SAMP_TIME)
#define LCTRL_LEVEL_CHECK_DELAY_COUNT (int)(LCTRL_LEVEL_CHECK_DELAY_TIME/SAMP_TIME)

//#define PRINT_LEARN_STATE
//#include "stdio.h"

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
FlowLearnControl::FlowLearnControl(ParIdent* pParIdent, PitFlow* pPitFlow)
{
  mOnCount  = 0;
  mOffCount = 0;

  step_nr   = 0;
  persistent_state = LCTRL_INIT;

  mpParIdent = pParIdent;
  mpPitFlow  = pPitFlow;
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION:
*
****************************************************************************/
FlowLearnControl::~FlowLearnControl()
{
}

/****************************************************************************
* INPUT(S)             : pointer to array with the 4 scale values
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function for initialization of the parameter
*                        estimate routine.
****************************************************************************/
void FlowLearnControl::SetFlowLearnParameters(float stop_level, float start_level, float in_min_speed, float in_max_speed)
{
  min_speed           = in_min_speed;
  min_speed_firstrun  = in_min_speed;
  min_speed_secondrun = in_min_speed;
  max_speed           = in_max_speed;

  level_start = start_level;
  level_learn = stop_level + LCTRL_LEVELSCALE*(start_level - stop_level);
  level_stop  = stop_level;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function to control the pump speed during training.
*
****************************************************************************/
void FlowLearnControl::CtrlFlowLearnControl( AFC_RUN_FLOW_LEARN_STRUCT *pflow_learn_struct )
{
  LCTRL_STATE_TYPE old_persistent_state = persistent_state;

  switch ( persistent_state )
  {
    case LCTRL_INIT :
      min_speed_firstrun  = min_speed;
      min_speed_secondrun = min_speed;
      speed_ref = max_speed;
      level_learn = level_stop + LCTRL_LEVELSCALE*(level_start - level_stop);
      step_nr   = 0;
      if ( pflow_learn_struct->pumpState == AFC_PUMP_RUNNING )
      {
        persistent_state = LCTRL_EMPTY_PIT;
      }
      else
      {
        persistent_state = LCTRL_WAIT;
      }
      break;

    case LCTRL_WAIT :
      speed_ref = max_speed;
      if ( pflow_learn_struct->pumpState == AFC_PUMP_RUNNING && pflow_learn_struct->level > level_learn )
      {
        mOnCount  = 0;
        nr_firstruns     = 0;
        persistent_state = LCTRL_FIRST_RUN;
      }
      break;

    case LCTRL_FIRST_RUN :
      mpParIdent->SetTrainingState(AFC_TRAINING_OFF);
      /*** Speed proportional with level ***/
      speed_ref = (pflow_learn_struct->level - level_learn)/(level_start - level_learn)*(max_speed - min_speed_firstrun) + min_speed_firstrun;
      if ( speed_ref > max_speed )
      {
        speed_ref = max_speed;
      }
      mOnCount++;
      if ( mOnCount > LCTRL_MAX_FIRST_RUN_COUNT )
      {
        min_speed_firstrun = min_speed_firstrun + 0.1f*(max_speed-min_speed);
        if ( min_speed_firstrun > max_speed)
        {
          min_speed_firstrun = max_speed;
        }
        mOnCount = LCTRL_MAX_FIRST_RUN_COUNT/3;
      }
      if ( pflow_learn_struct->level <= level_learn )
      {
        #if defined(PRINT_LEARN_STATE)
        printf("Level %f <= %f\n",pflow_learn_struct->level,level_learn);
        #endif
        nr_firstruns++;
        mOffCount = 0;
        persistent_state = LCTRL_DELAY;
      }
      break;

    case LCTRL_DELAY :
      speed_ref = 0.0;
      mOffCount++;
      if ( (pflow_learn_struct->level > level_start) )  // Might include number of starts too, if this is the case jump to learning should also be included.
      {
        if ( nr_firstruns > 1 )
        {
          mOnCount = 0;
          level_learn = level_learn - 0.05f*(level_start-level_stop);
          if ( level_learn < (level_stop+0.1f*(level_start-level_stop)) )
          {
            level_learn = level_stop+0.1f*(level_start-level_stop);
          }
          persistent_state = LCTRL_EMPTY_PIT;
        }
        else
        {
          mOnCount = 0;
          persistent_state = LCTRL_FIRST_RUN;
        }
      }
      if ( (mOffCount > LCTRL_MIN_DCOUNT_STARTS) )
      {
        mpParIdent->SetTrainingState(AFC_TRAINING_ON);
        step_nr   = (step_nr+1) % LCTRL_SPEED_STEPS;
        pit_flow = 0.0f;
        mOnCount = 0;
        if ( pflow_learn_struct->level < (level_stop+0.9f*(level_start-level_stop)) )
        {
          level_learn = level_learn + 0.05f*(level_start-level_stop);
          if ( level_learn > (level_stop+0.9f*(level_start-level_stop)) )
          {
            level_learn = level_stop+0.9f*(level_start-level_stop);
          }
        }
        persistent_state = LCTRL_SECOND_RUN;
      }
      break;

    case LCTRL_SECOND_RUN :
      mpParIdent->SetTrainingState(AFC_TRAINING_ON);
      speed_ref = min_speed_secondrun + step_nr * (max_speed - min_speed_secondrun) / (float)(LCTRL_SPEED_STEPS - 1);
      if ( speed_ref > max_speed )
      {
        speed_ref = max_speed;
      }
      if ( speed_ref < min_speed )
      {
        speed_ref = min_speed;
      }
      mOnCount++;
      pit_flow = (1.0f-LCTRL_PITFLOW_FILTER)*pit_flow + LCTRL_PITFLOW_FILTER*mpPitFlow->GetPitFlow();
      if ( ((pit_flow > 0.0f) && (mOnCount > 30)) || (mOnCount > LCTRL_MAX_SECOND_RUN_TIME) )
      {
        mOnCount = 0;
        min_speed_secondrun = min_speed_secondrun + 0.1f*(max_speed-min_speed);
        if ( min_speed_secondrun > max_speed)
        {
          min_speed_secondrun = max_speed;
        }
        step_nr   = (step_nr-1);
        if ( step_nr < 0 )
        {
          step_nr = LCTRL_SPEED_STEPS-1;
        }
        persistent_state = LCTRL_EMPTY_PIT;
      }
      if ( (pflow_learn_struct->pumpState != AFC_PUMP_RUNNING) && (mOnCount > 2) )
      {
        mOnCount = 0;
        persistent_state = LCTRL_WAIT;
      }
      if ( (RECORD_DECLINED == mpParIdent->GetStateOfParametersData()) && ( mOnCount > 1 ) )
      {
        mOnCount = 0;
        step_nr   = (step_nr-1);
        if ( step_nr < 0 )
        {
          step_nr = LCTRL_SPEED_STEPS-1;
        }
        mOnCount = 0;
        persistent_state = LCTRL_EMPTY_PIT;
      }
      break;

    case LCTRL_EMPTY_PIT :
      mpParIdent->SetTrainingState(AFC_TRAINING_OFF);
      speed_ref = max_speed;
      if ( pflow_learn_struct->pumpState != AFC_PUMP_RUNNING && pflow_learn_struct->level < level_stop + (0.1f*(level_start-level_stop)) )
      {
        persistent_state = LCTRL_WAIT;
      }
      break;

    default :
      persistent_state = LCTRL_WAIT;
      break;
  }

  if (old_persistent_state != persistent_state)
  {
    #if defined(PRINT_LEARN_STATE)
    PrintLearningStateChange(old_persistent_state, persistent_state);
    #endif
    old_persistent_state = persistent_state; // Just a break point
  }
}


/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function to return the pump speed during training.
*
****************************************************************************/
float FlowLearnControl::GetFlowLearnFrequency( void )
{
  return speed_ref;
}


/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Function for restarting the training procedure.
*
****************************************************************************/
void FlowLearnControl::ReInitLearning( void )
{
  persistent_state = LCTRL_INIT;
}


/*****************************************************************************
*
*
*              PRIVATE FUNCTIONS
*
*
****************************************************************************/
void FlowLearnControl::PrintLearningStateChange( LCTRL_STATE_TYPE old_persistent_state, LCTRL_STATE_TYPE new_persistent_state )
{
  #if defined(PRINT_LEARN_STATE)
  typedef char* text_type;

  text_type text_array[] =
  {
    "Init",
    "Wait",
    "First run",
    "Delay",
    "Second run",
    "Empty pit",
    "Out of range",
    "Out of range",
  };
  printf("Learning state changed: %s -> %s\n", text_array[old_persistent_state], text_array[new_persistent_state]);
  #endif
}



/*****************************************************************************
*
*
*              PROTECTED FUNCTIONS
*                 - RARE USED -
*
****************************************************************************/

