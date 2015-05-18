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
/* CLASS NAME       : ScadaStateCtrl                                        */
/*                                                                          */
/* FILE NAME        : ScadaStateCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     : 26-07-2012 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Monitors the scada connection                   */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcScadaStateCtrl_h
#define mrcScadaStateCtrl_h

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
#include <EnumDataPoint.h>
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
typedef  enum
{
  SCADA_TIMER_NOT_STARTED = 0,
  SCADA_TIMER_STARTED,
  SCADA_TIMER_TIME_OUT,

  // insert states above
  NO_OF_SCADA_TIMER_STATES,
  FIRST_SCADA_TIMER_STATE = 0,
  LAST_SCADA_TIMER_STATE = NO_OF_SCADA_TIMER_STATES - 1
} SCADA_TIMER_STATE;
/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class ScadaStateCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ScadaStateCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ScadaStateCtrl();
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

    // Input:
    SubjectPtr<EventDataPoint*>                mpAlarmToScadaArrived;
    SubjectPtr<U8DataPoint*>                   mpGsmState;  
    SubjectPtr<EnumDataPoint<COM_STATE_TYPE>*> mpGprsCommunicationState;
    SubjectPtr<AlarmDataPoint*>                mpScadaCallbackAlarmObj;

    // Output:
    SubjectPtr<EnumDataPoint<SCADA_STATE_TYPE>*> mpScadaState;
    SubjectPtr<EventDataPoint*>                  mpScadaStateUpdated;

    // Internal use
    bool                          mRunRequestedFlag;
    bool                          mScadaStateTimerRestart;
    SCADA_TIMER_STATE             mScadaTimerState;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
