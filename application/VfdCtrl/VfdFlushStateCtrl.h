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
/* CLASS NAME       : VfdFlushStateCtrl                                     */
/*                                                                          */
/* FILE NAME        : VfdFlushStateCtrl.h                                   */
/*                                                                          */
/* CREATED DATE     : 04-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class to generate flush requests for            */
/*                          variable frequency (VFD) controlled pumps.      */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcVfdFlushStateCtrl_h
#define mrcVfdFlushStateCtrl_h

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
class VfdFlushStateCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    VfdFlushStateCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~VfdFlushStateCtrl();
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

    VFD_OPERATION_MODE_TYPE CheckAntiBlockingRequest(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<BoolDataPoint*>    mpVfdReverseFlushEnabled;
    SubjectPtr<U32DataPoint*>     mpVfdReverseFlushTime;
    SubjectPtr<U32DataPoint*>     mpVfdReverseFlushInterval;
    SubjectPtr<BoolDataPoint*>    mpVfdStartFlushEnabled;
    SubjectPtr<U32DataPoint*>     mpVfdStartFlushTime;
    SubjectPtr<BoolDataPoint*>    mpVfdRunFlushEnabled;
    SubjectPtr<U32DataPoint*>     mpVfdRunFlushTime;
    SubjectPtr<U32DataPoint*>     mpVfdRunFlushInterval;
    SubjectPtr<BoolDataPoint*>    mpVfdStopFlushEnabled;
    SubjectPtr<U32DataPoint*>     mpVfdStopFlushTime;

    // Variable inputs:
    SubjectPtr<BoolDataPoint*>    mpVfdAutoOprRequest;
    SubjectPtr<BoolDataPoint*>    mpUnderDryRunningLevel;
    SubjectPtr<EnumDataPoint<ANTI_BLOCKING_REQUEST_TYPE>*>  mpAntiBlockingRequest;

    // Outputs:
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*>   mpVfdFlushRequest;

    // Local variables:
    U32 mMsSinceUpdateState;
    U32 mTimeInSameState;
    U32 mTimeSinceReverseFlush;
    U32 mTimeInAutoOperation;
    ANTI_BLOCKING_REQUEST_TYPE mOldAntiBlockingState;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
