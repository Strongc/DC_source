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
/* CLASS NAME       : PumpCurves                                            */
/*                                                                          */
/* FILE NAME        : PumpCurves.h                                          */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Pressure and Power Curves for a pump            */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef PumpCurves_h
#define PumpCurves_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PumpCurvesCnf.h"
#include "GeneralCnf.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MODEL_SIZE      4
#define CURVE_MAX_DIM   5

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PumpCurves
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PumpCurves();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PumpCurves();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Parameter for the pressure curve.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the parameters of the
    *                        pressure polynomial.
    ****************************************************************************/
    void GetPressureCurve(t_calcvar* pPressureParameter, t_calcvar* pGamma, t_calcvar* pTheta, t_calcvar* pPTheta);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Parameter for the power curve.
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Return function returning the parameters of the
    *                        power polynomial.
    ****************************************************************************/
    void GetPowerCurve(t_calcvar* pPowerParameters, t_calcvar* pGamma, t_calcvar* pTheta, t_calcvar* pPTheta);


  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool LuFactorization(t_calcvar *pLuMatrix, t_calcvar *pAMatrix, int *pDVector, unsigned int dimensions);

    void ParameterReduction(t_calcvar* pThetaReduced, t_calcvar* pGamma, t_calcvar* pTheta, t_calcvar* pPTheta);

    void SetupFmatrix(t_calcvar* pFMatrix, t_calcvar* pGamma, t_calcvar* pTheta);

    bool GetSystemSolution(t_calcvar* pResult, t_calcvar* pFMatrix, t_calcvar* pVector, int dimensions);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};


#endif

