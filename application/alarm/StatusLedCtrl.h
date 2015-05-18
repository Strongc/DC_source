/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : StatusLedCtrl                                         */
/*                                                                          */
/* FILE NAME        : StatusLedCtrl.h                                       */
/*                                                                          */
/* CREATED DATE     : 03-11-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Controlles the Red and Green status led                                  */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __STATUS_LED_CTRL_H__
#define __STATUS_LED_CTRL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <AlarmDef.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <BoolDataPoint.h>

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
  RED_LED_ACK_TIMER_STATUS_IDLE,
  RED_LED_ACK_TIMER_STATUS_RUNNING,
  RED_LED_ACK_TIMER_STATUS_TIMEOUT
} RED_LED_ACK_TIMER_STATUS_TYPE;

/*****************************************************************************
 * CLASS: StatusLedCtrl
 *****************************************************************************/
class StatusLedCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    StatusLedCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~StatusLedCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Functions from Subtask
    virtual void InitSubTask();
    virtual void RunSubTask();
    // Functions from Observer.
    virtual void SetSubjectPointer(int Id, Subject* pSubject);
    virtual void ConnectToSubjects();
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject) {};

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<STATUS_LED_STATE_TYPE>*> mpRedStatusLedState;
    SubjectPtr<EnumDataPoint<STATUS_LED_STATE_TYPE>*> mpGreenStatusLedState;
    SubjectPtr<EnumDataPoint<SYSTEM_MODE_TYPE>*> mpSystemMode;
    SubjectPtr<EnumDataPoint<OPERATION_MODE_TYPE>*> mpOperationMode;
    SubjectPtr<BoolDataPoint*> mHasUnackAlarms;
    SubjectPtr<EventDataPoint*> mpAlarmResetEvent;

    bool mGreenLedFlashTimeOut;
    RED_LED_ACK_TIMER_STATUS_TYPE mRedLedAckTimerStatus;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
