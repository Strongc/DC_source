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
#define PI_MAX_ON_TIME   20*60       /* [sec] Maximal acceptable on time for parameter identification. */
#define W_MIN            100.0f       /* [red/sec] velocity threshold, descides when the motor is assumed on. */
//#define PI_MAX_FLOW_CHANGE  1.0f         /* [value] compared to scaled flow values, hence 1.0 should be the larges possible values */
#define PI_MAX_FLOW_CHANGE  0.1f         /* [value] compared to scaled flow values, hence 1.0 should be the larges possible values */

#define PI_STARTDELAY      1           /* [sampels] Related to the calculation of the pit flow (should be solved by measurement delays, meaning that STARTDELAY = 0) */
#define R_Q_0              1.0f
#define R_P_0              1.0f

#define PUMPPARCONFI_MAX_SLOW           0.01f
#define PUMPPARCONFI_MAX_FAST         (10.0f * PUMPPARCONFI_MAX_SLOW)

//#define PUMPPARCONFI_FLOW_LIMIT         0.002f  // Version 8.1
#define PUMPPARCONFI_FLOW_LIMIT         0.005f
//#define PUMPPARCONFI_FLOW_LIMIT        0.01f
#define SCALE_PUMPPARCONFI_FLOW_LIMIT   1.0f    //0.1f
//#define PUMPPARCONFI_PRES_LIMIT         0.01f
#define PUMPPARCONFI_PRES_LIMIT        0.05f
#define SCALE_PUMPPARCONFI_PRES_LIMIT   1.0f    //0.1f

#define PARIDENT_PITFLOW_CHECK_TIME     15

//#define PARIDENT_QPIT_FILTER1           0.02f
#define PARIDENT_QPIT_FILTER1           0.05f
#define PARIDENT_QPIT_MAX_FAST          0.2f
#define PARIDENT_QPIT_MAX_SLOW          0.001f
#define PARIDENT_QPIT_CUSUM             0.1f
//#define PARIDENT_QPIT_CUSUM             0.2f

