/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : SwTimer                                               */
/*                                                                          */
/* FILE NAME        : SwTimer.h                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcSwTimer_h
#define mpcSwTimer_h
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

#ifdef __PC__
#include "rtos.h"
#endif

#ifndef __PC__
#include <rtos.h>
#endif

#include "cu351_cpu_types.h"
#include "SwTimerBassClass.h"
#include <Subject.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  MS,
  S,
  MINUTE
}TIMER_UNIT_TYPE;

/*****************************************************************************
  C functions declarations
*****************************************************************************/
extern "C" void call_back(void);
/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class SwTimer : public Subject
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SwTimer(unsigned int timerValue, TIMER_UNIT_TYPE unit, bool autoReTrigger, bool startSwTimer, Observer* pObserver);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SwTimer();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void StartSwTimer();
    void StopSwTimer();
    void RetriggerSwTimer();
    void SetSwTimerPeriod(unsigned int timerValue, TIMER_UNIT_TYPE unit, bool autoReTrigger);
    unsigned int GetSwTimerPeriode();
    unsigned int GetSwTimerValue();
    bool GetSwTimerStatus();
    void RealCallback();
    void TimeOut();

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void RealTimeOut();
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    OS_TIMER mTimer;
    TIMER_UNIT_TYPE mUnit;
    bool mAutoReTrigger;
    bool mTimerStarted;

    unsigned int mTimerValue;

    unsigned int mMs;
    bool mMsRunningLastTime;
    unsigned int mMsPrescaler;
    //unsigned int mMsPrescalerCounter;


    unsigned int mSec;
    unsigned int mSecCounter;
    unsigned int mMin;
    unsigned int mHalfMinutesCounter;
    SwTimerBaseClass* mpCallBackObj;
    unsigned int mTimeOutType;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
