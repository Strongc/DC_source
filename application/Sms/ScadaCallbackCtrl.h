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
/* CLASS NAME       : ScadaCallbackCtrl                                     */
/*                                                                          */
/* FILE NAME        : ScadaCallbackCtrl.h                                   */
/*                                                                          */
/* CREATED DATE     : 30-05-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Triggering of scada callback via Geni           */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcScadaCallbackCtrl_h
#define mrcScadaCallbackCtrl_h

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
#include <EventDataPoint.h>
#include <AlarmDataPoint.h>
#include <U8DataPoint.h>
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
class ScadaCallbackCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ScadaCallbackCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ScadaCallbackCtrl();
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
    void HandleCallbackAlarmRequest(bool alarm_request);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Input:
    SubjectPtr<BoolDataPoint*>    mpScadaCallbackEnabled;
    SubjectPtr<EventDataPoint*>   mpAlarmToScadaArrived;
    SubjectPtr<U8DataPoint*>      mpGsmState;
    SubjectPtr<EventDataPoint*>   mpScadaAckEvent;
    SubjectPtr<BoolDataPoint*>    mpServiceModeEnabled;

    // Output:
    SubjectPtr<BoolDataPoint*>    mpScadaCallbackRequest;
    SubjectPtr<BoolDataPoint*>    mpScadaCallbackPending;
    SubjectPtr<U32DataPoint*>     mpTimeStampLastScadaAck;

    // Alarm
    SubjectPtr<BoolDataPoint*>    mpScadaCallbackFault; // Output to bool alarm class
    SubjectPtr<AlarmDataPoint*>   mpScadaCallbackAlarmObj;

    // Internal use
    bool                          mRunRequestedFlag;
    bool                          mScadaCallbackInProgress;
    bool                          mCallbackSessionWaiting;
    bool                          mCheckAlarmRestart;
    bool                          mCallbackTimeout;
    U32                           mConnectionLostCount;
    PHONE_CALL_STATE_TYPE         mLastPhoneCallState;
    bool                          mLastCallBackAck;
    bool                          mCallbackEnabledSuppressFirstUpdated;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
