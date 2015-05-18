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
/* FILE NAME        : PumpCurves.cpp                                        */
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
#include <PumpCurves.h>

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
PumpCurves::PumpCurves()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
PumpCurves::~PumpCurves()
{
}


/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Parameter for the pressure curve.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return function returning the parameters of the
 *                        pressure polynomial.
 ****************************************************************************/
void PumpCurves::GetPressureCurve(t_calcvar* pPressureParameter, t_calcvar* pGamma, t_calcvar* pTheta, t_calcvar* pPTheta)
{
  t_calcvar f_matrix[4 * 4];
  t_calcvar theta_red[4];
  t_calcvar v_vector[4];

  ParameterReduction(theta_red, pGamma, pTheta, pPTheta);
  SetupFmatrix(f_matrix, pGamma, theta_red);
  v_vector[0] = 0.0f;
  v_vector[1] = 1.0f;
  v_vector[2] = 0.0f;
  v_vector[3] = 0.0f;

  if (GetSystemSolution(theta_red, f_matrix, v_vector, 4))
  {
    pPressureParameter[0] = theta_red[0];
    pPressureParameter[1] = theta_red[1];
    pPressureParameter[2] = theta_red[2];
    pPressureParameter[3] = theta_red[3];
  }
}

/****************************************************************************
 * INPUT(S)             :
 * OUTPUT(S)            : Parameter for the power curve.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return function returning the parameters of the
 *                        power polynomial.
 ****************************************************************************/
void PumpCurves::GetPowerCurve(t_calcvar* pPowerParameters, t_calcvar* pGamma, t_calcvar* pTheta, t_calcvar* pPTheta)
{
  t_calcvar f_matrix[4*4];
  t_calcvar theta_red[4], vector_v[4];

  ParameterReduction(theta_red, pGamma, pTheta, pPTheta);
  SetupFmatrix(f_matrix, pGamma, theta_red);
  vector_v[0] = 0.0f;
  vector_v[1] = 0.0f;
  vector_v[2] = 1.0f;
  vector_v[3] = 0.0f;

  if (GetSystemSolution(theta_red, f_matrix, vector_v, 4))
  {
    pPowerParameters[0] = theta_red[0];
    pPowerParameters[1] = theta_red[1];
    pPowerParameters[2] = theta_red[2];
    pPowerParameters[3] = theta_red[3];
  }

}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/


/****************************************************************************
 * INPUT(S)             : A square matrix A.
 * OUTPUT(S)            : The LU matrix of A.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : LU factorization of a square matrix A.
 ****************************************************************************/
bool PumpCurves::LuFactorization(t_calcvar *pLUMatrix, t_calcvar *pAMatrix, int *pDVector, unsigned int dimensions)
{
  float temp;
  int ii, jj, kk, ll;
  int index;

  for ( ii=0; ii<dimensions; ii++ )
  {
    pDVector[ii] = ii;
  }

  for ( jj=0; jj<dimensions; jj++ )
  {
    /* Calculate beta values */
    for ( ii=0; ii<jj+1; ii++ )
    {
      if ( ii != jj )
      {
        pLUMatrix[ii*dimensions + jj] = 0.0f;
        for ( kk=0; kk<ii; kk++ )
        {
          pLUMatrix[ii*dimensions + jj] = pLUMatrix[ii*dimensions + jj] + pLUMatrix[ii*dimensions + kk]*pLUMatrix[kk*dimensions + jj];
        }
        pLUMatrix[ii*dimensions + jj] = pAMatrix[pDVector[ii]*dimensions + jj] - pLUMatrix[ii*dimensions + jj];
      }
      else
      {
        temp = 0.0f;
        for ( ll=jj; ll<dimensions; ll++ )
        {
          pLUMatrix[ii*dimensions + jj] = 0.0f;
          for ( kk=0; kk<ii; kk++ )
          {
            pLUMatrix[ii*dimensions + jj] = pLUMatrix[ii*dimensions + jj] + pLUMatrix[ll*dimensions + kk]*pLUMatrix[kk*dimensions + jj];
          }
          pLUMatrix[ii*dimensions + jj] = pAMatrix[pDVector[ll]*dimensions + jj] - pLUMatrix[ii*dimensions + jj];
          if ( fabs(pLUMatrix[ii*dimensions + jj]) > fabs(temp) )
          {

            temp  = pLUMatrix[ii*dimensions + jj];
            index = ll;
          }
        }
        if ( temp == 0.0 ) /* The matrix is singular */
        {
          return false;
        }
        if ( index != ii ) /* Rows need to be switched */
        {
          pLUMatrix[ii*dimensions + jj] = temp;
          ll = pDVector[ii];
          pDVector[ii]    = pDVector[index];
          pDVector[index] = ll;
          for ( ll=0; ll<jj; ll++ )
          {
            temp = pLUMatrix[ii*dimensions + ll];
            pLUMatrix[ii*dimensions + ll]    = pLUMatrix[index*dimensions + ll];
            pLUMatrix[index*dimensions + ll] = temp;
          }
        }
        else
        {
          pLUMatrix[ii*dimensions + jj] = temp;
        }
      }
    }
    /* Calculate alpha values */
    for ( ii=jj+1; ii<dimensions; ii++ )
    {
      pLUMatrix[ii*dimensions + jj] = 0.0f;
      if ( jj > 0 )
      {
        for ( kk=0; kk<jj; kk++ )
        {
          pLUMatrix[ii*dimensions + jj] = pLUMatrix[ii*dimensions + jj] + pLUMatrix[ii*dimensions + kk]*pLUMatrix[kk*dimensions + jj];
        }
      }
      pLUMatrix[ii*dimensions + jj] = pAMatrix[pDVector[ii]*dimensions + jj] - pLUMatrix[ii*dimensions + jj];
      if ( pLUMatrix[jj*dimensions + jj] != 0.0 )
      {
        pLUMatrix[ii*dimensions + jj] = pLUMatrix[ii*dimensions + jj] / pLUMatrix[jj*dimensions + jj];
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

/****************************************************************************
 * INPUT(S)             : A square matrix and the deminsion of this.
 * OUTPUT(S)            : The inversion of the input matrix.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return the inverse the input matrix.
 ****************************************************************************/
void PumpCurves::ParameterReduction(t_calcvar* pThetaReduced, t_calcvar* pGamma, t_calcvar* pTheta, t_calcvar* pPTheta)
{
  t_calcvar temp[5], temp2[5];
  t_calcvar theta_err[5];
  t_calcvar p_matrix[5 * 5];

  if ( pGamma[1] != 0.0f )
  {
    theta_err[0] = pTheta[0] - (pGamma[0] * pGamma[0]     / (pGamma[1] * pGamma[1]));
    theta_err[1] = pTheta[1] - (2.0f * pGamma[0]          / pGamma[1]);
    theta_err[2] = pTheta[2] - (2.0f * pGamma[0] * pGamma[2] / (pGamma[1] * pGamma[1]));
    theta_err[3] = pTheta[3] - (pGamma[2] * pGamma[2]     / (pGamma[1] * pGamma[1]));
    theta_err[4] = pTheta[4] - (2.0f * pGamma[2]          / pGamma[1]);

    /*** P2^T*P1^(-1)*theta_err + theta ***/
    for (int i = 0; i < 5; i++)
    {
      for (int j = 0; j < 5; j++)
      {
        p_matrix[i * 5 + j] = pPTheta[i * 9 + j];
      }
    }
    /* P1^{-1}*theta_err */
    if (GetSystemSolution(temp2, p_matrix, theta_err, 5))
    {
      /* P2^T*[P1^{-1}*theta_err] + theta */
      for (int i = 0; i < 4; i++)
      {
        temp[i] = 0.0f;
        for (int j = 0; j < 5; j++)
        {
          temp[i] = temp[i] + pPTheta[(i + 5) * 9 + j] * temp2[j];
        }
        pThetaReduced[i] = -temp[i] + pTheta[i + 5];
      }
    }
  }

}

/****************************************************************************
 * INPUT(S)             : The parameter vector of the flow model and the
 *                        reduced parameter vector for the pressure model.
 * OUTPUT(S)            : The F matrix for parameter calculation.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Return the F matrix for parameter calculation.
 ****************************************************************************/
void PumpCurves::SetupFmatrix(t_calcvar* pFMatrix, t_calcvar* pGamma, t_calcvar* pTheta)
{
  pFMatrix[0 * 4 + 0] = -pTheta[0] * pGamma[1] * pGamma[1] + pGamma[3] * pGamma[3];
  pFMatrix[0 * 4 + 1] =  pGamma[3];
  pFMatrix[0 * 4 + 2] =  1.0f;
  pFMatrix[0 * 4 + 3] =  0.0f;

  pFMatrix[1 * 4 + 0] = -pTheta[1] * pGamma[1] * pGamma[1] + 2.0f * pGamma[1] * pGamma[3];
  pFMatrix[1 * 4 + 1] =  pGamma[1];
  pFMatrix[1 * 4 + 2] =  0.0f;
  pFMatrix[1 * 4 + 3] =  0.0f;

  pFMatrix[2 * 4 + 0] = -pTheta[2] * pGamma[1] * pGamma[1] + 2.0f * pGamma[2] * pGamma[3];
  pFMatrix[2 * 4 + 1] =  pGamma[2];
  pFMatrix[2 * 4 + 2] =  0.0f;
  pFMatrix[2 * 4 + 3] =  0.0f;

  pFMatrix[3 * 4 + 0] = -pTheta[3] * pGamma[1] * pGamma[1] + 2.0f * pGamma[0] * pGamma[3];
  pFMatrix[3 * 4 + 1] =  pGamma[0];
  pFMatrix[3 * 4 + 2] =  0.0f;
  pFMatrix[3 * 4 + 3] =  1.0f;

}


/****************************************************************************
 * INPUT(S)             : A square matrix F and a vector v.
 * OUTPUT(S)            : Solution to F^{-1} v.
 * DESIGN DOC.          :
 * FUNCTION DESCRIPTION : Returns the solution to F^{-1} v.
 ****************************************************************************/
bool PumpCurves::GetSystemSolution(t_calcvar* pResult, t_calcvar* pFMatrix, t_calcvar* pVector, int dimensions)
{
  int ii, kk;
  int D[CURVE_MAX_DIM];
  t_calcvar LU[CURVE_MAX_DIM*CURVE_MAX_DIM];
  t_calcvar temp[CURVE_MAX_DIM];

  if ( dimensions <= CURVE_MAX_DIM )
  {
    if ( true == LuFactorization(LU, pFMatrix, D, dimensions) )
    {
      /* L^(-1)*v */
      for ( ii=0; ii<dimensions; ii++ )
      {
        temp[ii] = 0.0f;
        if ( ii > 0 )
        {
          for ( kk=0; kk<ii; kk++ )
          {
            temp[ii] = temp[ii] + LU[ii*dimensions + kk]*temp[kk];
          }
        }
        temp[ii] = pVector[D[ii]] - temp[ii];
      }
      /* U(^-1)*[L^(-1)*v] */
      for ( ii=(dimensions-1); 0<=ii; ii-- )
      {
        pResult[ii] = 0;
        if ( ii < (dimensions-1) )
        {
          for ( kk=ii+1; kk<dimensions; kk++ )
          {
            pResult[ii] = pResult[ii] + LU[ii*dimensions + kk]*pResult[kk];
          }
        }
        if ( 0 != LU[ii*dimensions + ii] )
        {
          pResult[ii] = (temp[ii] - pResult[ii]) / LU[ii*dimensions + ii];
        }
        else
        {
          return false;
        }
      }
    }
    else
    {
      return false;
    }
    return true;
  }
  else
  {
    return false;
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

