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
/* CLASS NAME       : CustomRelayCtrl                                       */
/*                                                                          */
/* FILE NAME        : CustomRelayCtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 12-02-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __CUSTOM_RELAY_CTRL_H__
#define __CUSTOM_RELAY_CTRL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <SubjectPtr.h>
#include <BoolDataPoint.h>
#include <EventDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
typedef enum
{
  PULSE_DISABLED,
  PULSE_ON,
  PULSE_OFF
} PULSE_STATE_TYPE;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: SimpleRelayCtrl
 * DESCRIPTION:
 *****************************************************************************/
class CustomRelayCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    CustomRelayCtrl();

    /********************************************************************
    LIFECYCLE - Destructor
    ********************************************************************/
    ~CustomRelayCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Functions from Subtask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

    // Functions from Observer
    virtual void SetSubjectPointer(int Id, Subject* pSubject);
    virtual void ConnectToSubjects();
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    PULSE_STATE_TYPE mPulseState;
    bool mTimeoutFlag;

    SubjectPtr<EventDataPoint*> mpActivateRelayCmd;
    SubjectPtr<EventDataPoint*> mpDeActivateRelayCmd;
    SubjectPtr<EventDataPoint*> mpPulseRelayCmd;
    SubjectPtr<BoolDataPoint*> mpCustomRelay;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
