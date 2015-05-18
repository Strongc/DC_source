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
/* FILE NAME        : KalmanCalc.cpp                                        */
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
#include "KalmanCalc.h"

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
KalmanCalc::KalmanCalc()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
KalmanCalc::~KalmanCalc()
{
}

/****************************************************************************
* INPUT(S)             : Parameter vector theta (dim x 1), matrix P
*                        (dim x dim), vector C (1 x dim), and scalar R
* OUTPUT(S)            : Updated vector theta (dim x 1) and updated matrix P
*                        (dim x dim)
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculation of the time prediction of the Kalman filter.
****************************************************************************/
bool KalmanCalc::PredictKalman(t_calcvar* pTheta, t_calcvar* pP, const t_calcvar* pA, const t_calcvar* pB, t_calcvar u, const t_calcvar* pQ0, int dimensions)
{
  t_calcvar pTheta_new[MAX_DIM];
  t_calcvar temp[MAX_DIM * MAX_DIM];

  /* z(k+1) = A*z(k) + B*u */
  for (int i = 0; i < dimensions; i++)
  {
    pTheta_new[i] = 0.0f;
    for (int j = 0; j < dimensions; j++)
    {
      pTheta_new[i] = pTheta_new[i] + pA[i*dimensions + j]*pTheta[j];
    }
    pTheta_new[i] = pTheta_new[i] + pB[i]*u;
  }
  for (int i = 0; i < dimensions; i++)
  {
    pTheta[i] = pTheta_new[i];
  }

  /* P(k+1) = A*P(k)*A^T + Q0 */
  for (int i = 0; i < dimensions; i++ )
  {
    for (int j = 0; j < dimensions; j++)
    {
      temp[i*dimensions + j] = 0.0f;

      for (int k = 0; k < dimensions; k++)
      {
        temp[i*dimensions + j] = temp[i*dimensions + j] + pP[i*dimensions + k]*pA[j*dimensions + k];
      }
    }
  }
  for (int i = 0; i < dimensions; i++ )
  {
    for (int j = 0; j < dimensions; j++)
    {
      pP[i*dimensions + j] = 0.0f;

      for (int k = 0; k < dimensions; k++)
      {
        pP[i*dimensions + j] = pP[i*dimensions + j] + pA[i*dimensions + k]*temp[k*dimensions + j];
      }
      pP[i*dimensions + j] = pP[i*dimensions + j] + pQ0[i*dimensions + j];
    }
  }

  return true;
}

/****************************************************************************
* INPUT(S)             : Parameter vector theta (dim x 1), matrix P
*                        (dim x dim), vector C (1 x dim), and scalar R
* OUTPUT(S)            : Updated vector theta (dim x 1) and updated matrix P
*                        (dim x dim)
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculation of the Kalman filter.
****************************************************************************/
bool KalmanCalc::UpdateKalman(t_calcvar* pTheta, t_calcvar* pP, const t_calcvar* pC, t_calcvar y, t_calcvar R0, int dimensions)
{
  t_calcvar k_kal_vector[MAX_DIM];
  t_calcvar invterm;

  /* Kq = Pq*Cq'*inv(Cq*Pq*Cq' + R_Q_0) */
  invterm = CalculateK(k_kal_vector, pP, pC, R0, dimensions);

	if (invterm > 0.0f)
	{
    /* zq = zq + Kq*(mQKal - Cq*zq) */
    CalculateTheta(pTheta, k_kal_vector, pC, y, dimensions);

    /* Pq = Pq - Kq*Cq*Pq */
    CalculateP(pP, k_kal_vector, pC, invterm, dimensions);

    #if defined(KALMAN_PRINT)
    if (dimensions == 4)
    {
      printf("UpdateKalman():\n");
      printf("theta = [%f, %f, %f, %f];\n",pTheta[0],pTheta[1],pTheta[2],pTheta[3]);
      printf("P     = [%f, %f, %f, %f; ...\n"  ,pP[0], pP[1], pP[2], pP[3]);
      printf("         %f, %f, %f, %f; ...\n"  ,pP[4], pP[5], pP[6], pP[7]);
      printf("         %f, %f, %f, %f; ...\n"  ,pP[8], pP[9], pP[10],pP[11]);
      printf("         %f, %f, %f, %f];\n"  ,pP[12],pP[13],pP[14],pP[15]);
    }
    #endif

		return true;
	}
	else
	{
		return false;
	}

}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            : Confidence value for the point given in C
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculation confidence value.
****************************************************************************/
t_calcvar KalmanCalc::ConfidenceOfKalman(t_calcvar* pP, const t_calcvar* pC, int dimensions)
{
  t_calcvar temp1[MAX_DIM];
  t_calcvar temp2;

  /*** Calc P*C' ***/
  for (int i = 0; i < dimensions; i++)
  {
    temp1[i] = 0.0f;
    for (int j = 0; j < dimensions; j++)
    {
      temp1[i] = temp1[i] + pP[i * dimensions + j] * pC[j];
    }
  }

  /* Calc C*[P*C'] */
  temp2 = 0.0f;
  for (int i = 0; i < dimensions; i++)
  {
    temp2 = temp2 + pC[i] * temp1[i];
  }

  return temp2;
}

/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Copy parameter vector and P matrix.
****************************************************************************/
void KalmanCalc::CopyMatrices(t_calcvar* pThetaOut, t_calcvar* pMatrixOut, t_calcvar* pThetaIn, t_calcvar* pMatrixIn, int dimensions)
{
  for (int i = 0; i < dimensions; i++)
  {
    pThetaOut[i] = pThetaIn[i];

    for (int j = 0; j < dimensions; j++)
    {
      pMatrixOut[i * dimensions + j] = pMatrixIn[i * dimensions + j];
    }
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
* INPUT(S)             : Matrix P (dim x dim), vector C (1 x dim), and scalar R
* OUTPUT(S)            : Kalman gain vector K (dim x 1)
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculation of the Kalman gain K:
*                           K = P*C'*inv(C*P*C' + R)
****************************************************************************/
t_calcvar KalmanCalc::CalculateK(t_calcvar* pK, t_calcvar* pP, const t_calcvar* pC, t_calcvar R, int dimensions)
{
  t_calcvar temp1[MAX_DIM];
  t_calcvar temp2;

  /*** Calc P*C' ***/
  for (int i = 0; i < dimensions; i++ )
  {
    temp1[i] = 0.0f;
    for (int j = 0; j < dimensions; j++ )
    {
      temp1[i] = temp1[i] + pP[i * dimensions + j] * pC[j];
    }
  }

  /*** 1/(C*[P*C'] + R) ***/
  temp2 = 0.0f;
  for (int i = 0; i < dimensions; i++)
  {
    temp2 = temp2 + pC[i] * temp1[i];
  }
	if (temp2 + R > 0.0f)
	{
  	temp2 = 1.0f / (temp2 + R);
	}
	else
	{
		temp2 = 0.0f;
	}

  /*** [P*C'][/(C*P*C' + R)] ***/
  for (int i = 0; i < dimensions; i++)
  {
    pK[i] = temp1[i] * temp2;
  }

  #if defined(KALMAN_PRINT)
  if (dimensions == 4)
  {
    printf("CalcK():\n");
    printf("C = [%f, %f, %f, %f];\n\n",pC[0],pC[1],pC[2],pC[3]);
    printf("temp1  = [%f, %f, %f, %f];\n\n"  ,temp1[0], temp1[1], temp1[2], temp1[3]);
    printf("temp2 =%f; R=%f;\n\n",temp2,R);
    printf("P     = [%f, %f, %f, %f; ...\n"  ,pP[0], pP[1], pP[2], pP[3]);
    printf("         %f, %f, %f, %f; ...\n"  ,pP[4], pP[5], pP[6], pP[7]);
    printf("         %f, %f, %f, %f; ...\n"  ,pP[8], pP[9], pP[10],pP[11]);
    printf("         %f, %f, %f, %f];\n"  ,pP[12],pP[13],pP[14],pP[15]);
    printf("--------\n\n");
  }
  #endif

  return temp2;

}

/****************************************************************************
* INPUT(S)             : Vector theta (dim x 1), vector K (dim x 1), and scalar y
* OUTPUT(S)            : parameter vector theta (dim x 1)
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculation of the parameter vector theta:
*                           theta = theta + K*(y - C*theta)
****************************************************************************/
void KalmanCalc::CalculateTheta(t_calcvar* pTheta, t_calcvar* pK, const t_calcvar* pC, t_calcvar y, int dimensions)
{
  t_calcvar temp1;

  /*** C*theta ***/
  temp1 = 0.0f;
  for (int i = 0; i < dimensions; i++)
  {
    temp1 = temp1 + pC[i] * pTheta[i];
  }

  /*** theta + K*(y - [C*theta]) ***/
  for (int i = 0; i < dimensions; i++)
  {
    pTheta[i] = pTheta[i] + pK[i] * (y - temp1);
  }

}

/****************************************************************************
* INPUT(S)             : Matrix P (dim x dim), vector C (1 x dim),
 *                       matrix Q (dim x dim), and scalar invterm.
* OUTPUT(S)            : Correlation matrix P (dim x dim)
* DESIGN DOC.          :
* FUNCTION DESCRIPTION : Calculation of the correlation matrix P:
*                           P = P - K*C*P + Q
****************************************************************************/
void KalmanCalc::CalculateP(t_calcvar* pP, t_calcvar* pK, const t_calcvar* pC, t_calcvar invterm, int dimensions)
{
  t_calcvar temp[MAX_DIM * MAX_DIM];

  /*** K*C*P ***/
  for (int i = 0; i < dimensions; i++ )
  {
    for (int j = 0; j < dimensions; j++)
    {
      temp[i * dimensions + j] = 0.0f;

      for (int k = 0; k < dimensions; k++)
      {
        temp[i * dimensions + j] = temp[i * dimensions + j] + pK[i] * pC[k] * pP[k * dimensions + j];
      }
    }
  }

  /*** P - [K*C*P] + Q ***/
  for (int i = 0; i < dimensions; i++)
  {
    for (int j = i; j < dimensions; j++)
    {
      pP[i * dimensions + j] = pP[i * dimensions + j] - temp[i * dimensions + j]; // + pQ[i*dimensions + j];
      pP[j * dimensions + i] = pP[i * dimensions + j];
		}
	}

  #if defined(KALMAN_PRINT)
  if (dimensions == 4)
  {
    printf("calc_P():\n");
    printf("temp1 = [%f, %f, %f, %f; ...\n"  ,temp1[0], temp1[1], temp1[2], temp1[3]);
    printf("         %f, %f, %f, %f; ...\n"  ,temp1[4], temp1[5], temp1[6], temp1[7]);
    printf("         %f, %f, %f, %f; ...\n"  ,temp1[8], temp1[9], temp1[10],temp1[11]);
    printf("         %f, %f, %f, %f];\n\n"   ,temp1[12],temp1[13],temp1[14],temp1[15]);
    printf("P     = [%f, %f, %f, %f; ...\n"  ,pP[0], pP[1], pP[2], pP[3]);
    printf("         %f, %f, %f, %f; ...\n"  ,pP[4], pP[5], pP[6], pP[7]);
    printf("         %f, %f, %f, %f; ...\n"  ,pP[8], pP[9], pP[10],pP[11]);
    printf("         %f, %f, %f, %f];\n\n"   ,pP[12],pP[13],pP[14],pP[15]);
    printf("-------\n\n");
  }
  #endif
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/


