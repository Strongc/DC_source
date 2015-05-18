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
#define W_MIN               100.0f           // Min on velocity [rad/sec].
#define RHO                1000.0f           // Fluid density.
#define G                     9.82f          // Gravity constant.
#define STOP_PRESSURE_FILTER  0.001f
#define QPIT_FILT             0.05f
#define QPUMP_FILT            0.7f
#define STATE_STOP_DELAY      0
#define MIN_EMPTY_TIME        3.0f*60.0f     // Max empty time
#define FLOW_RUNOFF_TIME      (unsigned char)(30/SAMP_TIME)   // run off time [sec] / sampling time [sec]
