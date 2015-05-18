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
/* CLASS NAME       : EventLogCtrl                                          */
/*                                                                          */
/* FILE NAME        : EventLogCtrl.h                                        */
/*                                                                          */
/* CREATED DATE     : 29-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class to maintain the event log vector.         */
/*                          Based on changes in alarms/warnings.            */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcEventLogCtrl_h
#define mrcEventLogCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>
#include <SubTask.h>
#include <EventDataPoint.h>
#include <AlarmDataPoint.h>
#include <U16DataPoint.h>
#include <EventLogVectorDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_EVENT_LOG_TYPE = 0,
  NO_EVENT_LOG = 0,
  EVENT_LOG_ALARM_APPEAR,
  EVENT_LOG_ALARM_DISAPPEAR,
  EVENT_LOG_WARNING_APPEAR,
  EVENT_LOG_WARNING_DISAPPEAR,

  // insert above here
  NO_OF_EVENT_LOG,
  LAST_EVENT_LOG = NO_OF_EVENT_LOG - 1
} EVENT_LOG_TYPE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class EventLogCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static EventLogCtrl* GetInstance();

    virtual void InitSubTask();
    virtual void RunSubTask();
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void ConnectToSubjects();
    virtual void SetSubjectPointer(int Id, Subject* pSubject);

    void AddEventLogElement(AlarmDataPoint* pChangedAlarm, ALARM_STATE_TYPE oldAlarmState);

  private:
    /********************************************************************
    LIFECYCLE - Default constructor. Private because it is a Singleton
    ********************************************************************/
    EventLogCtrl();
    /********************************************************************
    LIFECYCLE - Destructor. Private because it is a Singleton
    ********************************************************************/
    ~EventLogCtrl();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void PutEventLogOnGeniForTest(void);
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static EventLogCtrl* mInstance;

    // Inputs
    SubjectPtr<EventDataPoint*> mpResetEventLog;
    SubjectPtr<U16DataPoint*>   mpEventLogClearId;

    // Outputs
    SubjectPtr<U16DataPoint*>             mpEventLogLatestId;
    SubjectPtr<EventLogVectorDataPoint*>  mpEventLogVector;

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
