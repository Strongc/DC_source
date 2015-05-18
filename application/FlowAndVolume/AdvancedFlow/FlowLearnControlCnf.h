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
/*                                                                          */
/* MODULE NAME      : Module ParIdent                                       */
/*                                                                          */
/* FILE NAME        : ParIdent.c                                            */
/*                                                                          */
/* FILE DESCRIPTION : Algorithm for parameter identification.               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
#define LCTRL_LEVELSCALE        0.80     // [%/100] Scale value that defines the intermediary stop level, such that: level_learn = stop_level + LCTRL_LEVELSCALE*(start_level - stop_level)
//#define LCTRL_TIMESCALE         1.0

#define LCTRL_SPEED_STEPS       4

//#define LCTRL_ADD_LEVEL         0.05

#define LCTRL_MAX_FIRST_RUN_TIME   5*60   // [sec]
#define LCTRL_MAX_SECOND_RUN_TIME  10*60   // [sec]

#define LCTRL_LEVEL_CHECK_DELAY_TIME 60    // [sec]

#define LCTRL_MIN_DTIME_STARTS  160      // [sec]


#define LCTRL_MAX_FIRSTRUNS     2


#define LCTRL_PITFLOW_FILTER    0.1f
