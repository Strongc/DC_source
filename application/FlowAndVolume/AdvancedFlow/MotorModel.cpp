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
/* CLASS NAME       : MotorModel                                            */
/*                                                                          */
/* FILE NAME        : MotorModel.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
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
#include <MotorModel.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

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
MotorModel::MotorModel()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
MotorModel::~MotorModel()
{
}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Shaft speed.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for calculating the shaft speed based on
 *                        electrical measurements. The calculations are
 *                        expected to differ dependent on the hardware
 *                        settings of the system.
 ****************************************************************************/
float MotorModel::GetShaftSpeed(float powerOfEngine, float angularVelocityOfEngine)
{
  return angularVelocityOfEngine;
}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Shaft torque.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Function for calculating shaft torque based on
 *                        electrical measurements. The calculations are
 *                        expected to differ dependent on the hardware
 *                        settings of the system.
 ****************************************************************************/
float MotorModel::GetShaftTorque(float powerOfEngine, float angularVelocityOfEngine)
{
  if (angularVelocityOfEngine > 0.0f)
  {
    return powerOfEngine / angularVelocityOfEngine;
  }
  else
  {
    return 0.0f;
  }
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
