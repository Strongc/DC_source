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
/* CLASS NAME       : AntiSeizingCtrl                                       */
/*                                                                          */
/* FILE NAME        : AntiSeizingCtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 28-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Controls the anti seizing functionality for each*/
/*                          pump and sets a request for anti seizing when   */
/*                          required (based upon the operation mode and an  */
/*                          adjustable wait time). The decision upon if the */
/*                          pump is actually allowed to antiseize when      */
/*                          requested is taken somewhere else.              */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAntiSeizingCtrl_h
#define mrcAntiSeizingCtrl_h

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
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>

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
  ANTI_SEIZING_PREVENTED,
  ANTI_SEIZING_LEGAL
} ANTI_SEIZING_STATE;


typedef enum
{
  ANTI_SEIZING_LEGAL_WAITING,
  ANTI_SEIZING_LEGAL_REQUESTED,
  ANTI_SEIZING_LEGAL_RUNNING
} ANTI_SEIZING_LEGAL_STATE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class AntiSeizingCtrl : public SubTask, public SwTimerBaseClass

{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AntiSeizingCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AntiSeizingCtrl();
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
    void StopTimers(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<U16DataPoint*> mpAntiSeizingRunTime;
    SubjectPtr<U32DataPoint*> mpAntiSeizingWaitTime;
    SubjectPtr<BoolDataPoint*> mpAntiSeizingED;

    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationModePump;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpAntiSeizingDigInRequest;

    SubjectPtr<BoolDataPoint*> mpPumpAntiSeizingReq;

    ANTI_SEIZING_STATE mAntiSeizingState;
    ANTI_SEIZING_LEGAL_STATE mAntiSeizingLegalState;

    bool mAntiSeizingWaitTimerFlag;
    bool mAntiSeizingRunTimerFlag;
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
