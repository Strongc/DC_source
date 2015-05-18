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
/* CLASS NAME       : DailyEmptyingCtrl                                      */
/*                                                                          */
/* FILE NAME        : DailyEmptyingCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 24-07-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcDailyEmptyingCtrl_h
#define mrcDailyEmptyingCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <MpcTimeCmpCtrl.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <I32DataPoint.h>
#include <U32DataPoint.h>

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
class DailyEmptyingCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    DailyEmptyingCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~DailyEmptyingCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitSubTask();
    void RunSubTask();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    MpcTime* mpDailyEmptyingActivationTime;
    MpcTimeCmpCtrl* mpDailyEmptyingActivationTimeObs;
    SubjectPtr<I32DataPoint*> mpPitEmptyingTimeOfTheDay;
    SubjectPtr<U32DataPoint*> mpPitEmptyingMaxTime;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;
    SubjectPtr<BoolDataPoint*> mpDailyEmptyingRequested;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpDailyEmptyingDigInRequest;
    SubjectPtr<BoolDataPoint*> mpDailyEmptyingED;

    bool mPitEmptyingMaxTimeOutFlag;
    bool mTimeForDailyEmptying;
    bool mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
