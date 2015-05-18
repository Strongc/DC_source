/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : PidBlock                                              */
/*                                                                          */
/* FILE NAME        : PidBlock.h                                            */
/*                                                                          */
/* CREATED DATE     : 02-06-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Generic pid block                               */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPidBlock_h
#define mrcPidBlock_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PidBlock
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PidBlock();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PidBlock();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/

    void  ConfigPid(float kp, float ti, float td, float Ts, float outputMin, float outputMax);
    void  PresetPid(float spValue, float pvValue, float presetValue);
    float UpdatePid(float spValue, float pvValue);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    float CalcPterm(float pidError);
    float CalcIterm(float pidError, float iMin, float iMax);
    float CalcDterm(float pidError);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    float mPidOutput;         // pid output variable
    float mOldPidOutput;      // last pid output
    float mOutputMin;         // pid output is limited to mOutputMin [%]
    float mOutputMax;         // pid output is limited to mOutputMax [%]

    float mGp;                // P gain
    float mGi;                // I gain
    float mGd;                // D gain
    float mTs;                // Sample time

    float mIterm;             // Integrator state

    float mPidErrorFiltered;  // pid error filtered
    float mPidErrorSteadyTime;// counter to detect steady state
    float mOldPidError;       // old pid error
    float mDErrorFiltered;    // D error filtered

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
