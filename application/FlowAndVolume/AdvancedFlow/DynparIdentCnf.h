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
#define DPI_MAX_ON_TIME  120*60            /* [sec] Maximal acceptable on time for parameter identification. */
#define W_MIN            100.0f             /* [red/sec] velocity threshold, descides when the motor is assumed on. */

#define PRE_FILTER_CONST 1.0f

#define DPI_STARTDELAY       30//210  // 7 min delay
#define R_DYN_0          1.0f

#define DYNPARCONFI_MAX_SLOW   0.0002f
#define DYNPARCONFI_MAX_FAST   100.0f*DYNPARCONFI_MAX_SLOW
#define DYNPARCONFI_LIMIT      0.005f

#define POUT_ZERO_FILTER       0.001f       /* Filter for parameter validation */
