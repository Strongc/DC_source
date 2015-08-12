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
/* CLASS NAME       : InterLockedCtrl                                       */
/*                                                                          */
/* FILE NAME        : InterLockedCtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 17-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcInterLockedCtrl_h
#define mrcInterLockedCtrl_h

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
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>
#include <EventDataPoint.h>
#include <EnumDataPoint.h>

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
class InterLockedCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    InterLockedCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~InterLockedCtrl();
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

    SubjectPtr<EventDataPoint*> mpInterLockRequestEvent;
    SubjectPtr<EventDataPoint*> mpInterLockOffEvent;
    SubjectPtr<BoolDataPoint*> mpInterLocked;
    SubjectPtr<U32DataPoint*> mpInterLockTimeout;
    SubjectPtr<U32DataPoint*> mpInterLockTimeRemain;
    SubjectPtr<BoolDataPoint*> mpInterLockIgnore;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpInterlockDigInRequest;

    bool mInterLockTimeoutTimerFlag;
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
