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
/* MODULE NAME      : General configuration for advanced flow calculation   */
/*                                                                          */
/* FILE NAME        : GeneralCnf.h                                          */
/*                                                                          */
/* FILE DESCRIPTION : Algorithm for parameter identification.               */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef GeneralCnf_h
#define GeneralCnf_h

/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/
#define SAMP_TIME          1.0f //must be larger than zero

#define FLOW_DIM              4

#define PRESSURE_DIM          9

#define PI_NUMBER_OF_STORED_PARAMETERS  (2 + FLOW_DIM + (FLOW_DIM*FLOW_DIM-FLOW_DIM)/2+FLOW_DIM + PRESSURE_DIM + (PRESSURE_DIM*PRESSURE_DIM-PRESSURE_DIM)/2+PRESSURE_DIM)
#define PI_PARAMETER_STORE_OK            0
#define PI_PARAMETER_STORE_ERROR         1

// type used for all measurements and calculations
#define t_calcvar double

#ifndef U8
#define U8 unsigned char
#endif

// Advanced flow calculation: training state
typedef enum {
  AFC_TRAINING_OFF = 0,
  AFC_TRAINING_ON
} AFC_TRAINING_STATE_TYPE;

// Advanced flow: pump state
typedef enum {
  AFC_PUMP_STOPPED = 0,
  AFC_PUMP_RUNNING,
  AFC_PUMP_NOT_THERE
} AFC_PUMP_STATE_TYPE;

// Test variables for development:
#if defined(MATLAB_TEST)
extern float test_var1;
extern float test_var2;
extern float test_var3;
extern float test_var4;
extern float test_var5;
extern float test_var6;
#endif

#endif

