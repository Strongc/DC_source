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
#define RHO             1000.0f    // Fluid density.
#define G                  9.82f   // Gravity constant.
#define MAX_SAMP_ZERO      254

/*** Kalman implementation ***/
const t_calcvar A[2*2]  = {1.0f, 0.0f, (10000.0f * SAMP_TIME), 1.0f};
const t_calcvar C[2]    = {0.0f, 1.0f};
const t_calcvar B[2]    = {0.0f, 0.0f};
const t_calcvar R0      = 1.0f;
const t_calcvar Q0[2*2] = {0.0000000001f, 0.0f, 0.0f, 0.0f};
