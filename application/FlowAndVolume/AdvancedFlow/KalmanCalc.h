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
/* CLASS NAME       : KalmanCalc                                            */
/*                                                                          */
/* FILE NAME        : KalmanCalc.h                                          */
/*                                                                          */
/* CREATED DATE     : 09-11-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Kalman filter                                   */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef KalmanCalc_h
#define KalmanCalc_h

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

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_DIM   10

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class KalmanCalc
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    KalmanCalc();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~KalmanCalc();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /****************************************************************************
    * INPUT(S)             : Parameter vector theta (dim x 1), matrix P
    *                        (dim x dim), vector C (1 x dim), and scalar R
    * OUTPUT(S)            : Updated vector theta (dim x 1) and updated matrix P
    *                        (dim x dim)
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Calculation of the time prediction of the Kalman filter.
    ****************************************************************************/
    bool PredictKalman(t_calcvar* pTheta, t_calcvar* pP, const t_calcvar* pA, const t_calcvar* pB, t_calcvar u, const t_calcvar* pQ0, int dimensions);

    /****************************************************************************
    * INPUT(S)             : Parameter vector theta (dim x 1), matrix P
    *                        (dim x dim), vector C (1 x dim), and scalar R
    * OUTPUT(S)            : Updated vector theta (dim x 1) and updated matrix P
    *                        (dim x dim)
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Calculation of the Kalman filter.
    ****************************************************************************/
    bool UpdateKalman(t_calcvar* pTheta, t_calcvar* pP, const t_calcvar* pC, t_calcvar y, t_calcvar R0, int dimensions);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            : Confidence value for the point given in C
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Calculation confidence value.
    ****************************************************************************/
    t_calcvar ConfidenceOfKalman(t_calcvar* pP, const t_calcvar* pC, int dimensions);

    /****************************************************************************
    * INPUT(S)             :
    * OUTPUT(S)            :
    * DESIGN DOC.          :
    * FUNCTION DESCRIPTION : Copy parameter vector and P matrix.
    ****************************************************************************/
    void CopyMatrices(t_calcvar* pThetaOut, t_calcvar* pMatrixOut, t_calcvar* pThetaIn, t_calcvar* pMatrixIn, int dimensions);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    t_calcvar CalculateK(t_calcvar *pK, t_calcvar *pP, const t_calcvar *pC, t_calcvar R, int dimensions);

    void CalculateTheta(t_calcvar *pTheta, t_calcvar *pK, const t_calcvar *pC, t_calcvar y, int dimensions);

    void CalculateP(t_calcvar *pP, t_calcvar *pK, const t_calcvar *pC, t_calcvar invterm, int dimensions);

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
