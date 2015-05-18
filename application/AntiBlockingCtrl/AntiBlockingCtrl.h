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
/* CLASS NAME       : AntiBlockingCtrl                                      */
/*                                                                          */
/* FILE NAME        : AntiBlockingCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 23-09-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class to detect pump blocking and               */
/*                          generate anti blocking requests (flushes)       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAntiBlockingCtrl_h
#define mrcAntiBlockingCtrl_h

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
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <U32DataPoint.h>
#include <AlarmDelay.h>
#include <AlarmDataPoint.h>

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
class AntiBlockingCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AntiBlockingCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AntiBlockingCtrl();
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

    void DetectBlocking(void);
    void HandleTriggerAlarmAck(void);
    void UpdateAntiBlockingRequest(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<BoolDataPoint*>    mpAntiBlockingEnabled;
    SubjectPtr<U32DataPoint*>     mpAntiBlockingDirectionChangeTime;
    SubjectPtr<BoolDataPoint*>    mpAntiBlockingReverseFlushEnabled;
    SubjectPtr<U32DataPoint*>     mpAntiBlockingReverseFlushTime;
    SubjectPtr<BoolDataPoint*>    mpAntiBlockingForwardFlushEnabled;
    SubjectPtr<U32DataPoint*>     mpAntiBlockingForwardFlushTime;
    SubjectPtr<U32DataPoint*>     mpAntiBlockingMaxNoOfTries;
    SubjectPtr<BoolDataPoint*>    *mpBlockedTriggerEnabledList;  // A list of bool objects that indicates trigging e/d

    // Variable inputs:
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*>  mpOperationModeActualPump;
    SubjectPtr<AlarmDataPoint*>   *mpBlockedAlarmSourceList;  // A list of alarm objects that indicates blocking
    SubjectPtr<BoolDataPoint*>    mpBlockageDetected;

    // Outputs:
    SubjectPtr<EnumDataPoint<ANTI_BLOCKING_REQUEST_TYPE>*>  mpAntiBlockingRequest;
    SubjectPtr<EventDataPoint*> mpCUEAlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpIO111AlarmResetEvent;
    SubjectPtr<EventDataPoint*> mpMP204AlarmResetEvent;
    SubjectPtr<U32DataPoint*>   mpAntiBlockingPerformedCounter;

    // Alarms
    SubjectPtr<AlarmDataPoint*> mpBlockedAlarmObj;
    AlarmDelay* mpBlockedAlarmDelay;
    bool mBlockedAlarmDelayCheckFlag;

    // Local variables:
    bool *mPendingAlarmAckList;
    bool mRunAntiBlock;
    U32  mTriggerAlarmAckIndex;
    U32  mAntiBlockCountIn24h;
    U32  mAntiBlockCountInRow;
    U32  mTimeSince24HReset;
    U32  mAlarmResetWaitTime;
    U32  mAntiBlockStateWaitTime;
    U32  mAntiBlockReverseTime;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
