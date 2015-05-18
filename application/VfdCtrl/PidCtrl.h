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
/* CLASS NAME       : PidCtrl                                               */
/*                                                                          */
/* FILE NAME        : PidCtrl.h                                             */
/*                                                                          */
/* CREATED DATE     : 14-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Pid controller for use with vfd                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPidCtrl_h
#define mrcPidCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include "PidBlock.h"
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>

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
class PidCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PidCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PidCtrl();
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

    void UpdatePidConfig(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<EnumDataPoint<PID_TYPE_TYPE>*>           mpPidType;
    SubjectPtr<BoolDataPoint*>                          mpPidInverseControl;
    SubjectPtr<FloatDataPoint*>                         mpPidKp;
    SubjectPtr<FloatDataPoint*>                         mpPidTi;
    SubjectPtr<FloatDataPoint*>                         mpPidTd;
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*>     mpPidAiFeedback;
    SubjectPtr<EnumDataPoint<PID_SETPOINT_TYPE_TYPE>*>  mpPidSetpointType;
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*>     mpPidAiSetpoint;
    SubjectPtr<FloatDataPoint*>                         mpPidFixedSetpoint;
    SubjectPtr<FloatDataPoint*>                         mpVfdMinFrequency;
    SubjectPtr<FloatDataPoint*>                         mpVfdMaxFrequency;

    // Variable inputs:
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*> mpVfdState;
    SubjectPtr<FloatDataPoint*>                         mpVfdActFrequency;
    SubjectPtr<FloatDataPoint*>                         mpMeasuredValue[NO_OF_MEASURED_VALUE];

    // Outputs:
    SubjectPtr<FloatDataPoint*>                         mpVfdPidFrequency;

    // Local variables:
    bool mRunRequestedFlag;
    bool mPidRunTimeOut;
    bool mConfigChanged;
    bool mValidSignals;
    bool mSubjectAttached[NO_OF_MEASURED_VALUE];

    float mPidFrequency;
    PidBlock *mPidBlock;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
