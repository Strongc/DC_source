/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange Controller                           */
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
/****************************************************************************/
/* CLASS NAME       : NumericalDataPointInterface                           */
/*                                                                          */
/* FILE NAME        : NumericalDataPointInterface.h                         */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __NUMERICAL_DATA_POINT_INTERFACE_H__
#define __NUMERICAL_DATA_POINT_INTERFACE_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "Subject.h"
#include <DataPoint.h>
#include "mpcunit/mpcunits.h"
#include <UnitTypes.h>
#include "FloatDataPoint.h"
#include "U32DataPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

namespace mpc
{
  namespace display
  {

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class NumericalDataPointInterface
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    NumericalDataPointInterface(Subject* pSubject);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~NumericalDataPointInterface();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void GetDataPointAsString(char* string, int numberOfDigits);
    virtual void GetCapturedDataPointAsString(char* string, int numberOfDigits);
    virtual float GetDataPointAsFloat(void);
    virtual int GetValueAsInt(void);
    virtual Subject* CaptureDataPoint(void);
    virtual bool IsCaptured(void);
    virtual void ReleaseDataPoint(void);
    virtual void ChangeCapturedDataPoint(int changeValue,int NumberOfDigits);  // -? .. -1, 0 , 1, ?
    virtual bool SetCapturedDataPoint(void);
    virtual bool IsDataPointMax(void);
    virtual bool IsDataPointMin(void);
    virtual bool IsCapturedDataPointMax(void);
    virtual bool IsCapturedDataPointMin(void);
    virtual bool IsReadyForAcceleration(int changeValue, int numberOfDigits);
    virtual void ResetAccelerationCounter(void);

    virtual bool ChangeAccelerationOnDecades(void);
    virtual int  GetCurrentTimesumChange(int previousTimesumChange, int numberOfDigits);

    /* --------------------------------------------------
    * If the subject is a DataPoint and the quality is
    * DP_NEVER_AVAILABLE this function shall return true
    * --------------------------------------------------*/
    virtual bool IsNeverAvailable();
  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
  	virtual void GetAsString(char* string, int numberOfDigits, INumberDataPoint* number);
    virtual float GetRemaining( FloatDataPoint* pfDp, int numberOfDigits, float changeValue);
    virtual int Log10(float a);
    virtual float Pow10(int n);
    virtual char* FormatStr(char* numberStr ,float number, float max, int numberOfDigits);
    virtual void FormatDateStr(INumberDataPoint* number, char* string, int numberOfDigits);
    
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    INumberDataPoint* mNumber;
    INumberDataPoint* mCapturedNumber;
    int mNumberOfAccelerationRequests;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

  }

}

#endif
