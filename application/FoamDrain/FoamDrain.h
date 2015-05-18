/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRange                                      */
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
/* CLASS NAME       : FoamDrain                                             */
/*                                                                          */
/* FILE NAME        : FoamDrain.h                                           */
/*                                                                          */
/* CREATED DATE     : 30-06-2009  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class specification for FoamDrain.              */
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef wwmrFoamDrain_h
#define wwmrFoamDrain_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <DiFuncHandler.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <AlarmDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>
#include <AppTypeDefs.h>
#include <SwTimer.h>


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
class FoamDrain : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FoamDrain(void);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FoamDrain(void);

    /********************************************************************
    ASSIGNMENT OPERATORS
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
    ATTRIBUTES
    ********************************************************************/
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;
    SubjectPtr<BoolDataPoint*> mpSurfaceLevelReady;
    SubjectPtr<FloatDataPoint*> mpSurfaceLevel;
    SubjectPtr<U8DataPoint*> mpNoOfPumps;
    SubjectPtr<BoolDataPoint*> mpUnderDryRunLevel;
    SubjectPtr<BoolDataPoint*> mpPumpRefLevel[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>mpFoamDrainLevel;
    SubjectPtr<U32DataPoint*> mpFoamDrainIntervalTime;
    SubjectPtr<U32DataPoint*> mpFoamDrainTime;
    SubjectPtr<BoolDataPoint*> mpFoamDrainEnabled;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpFoamDrainDigInRequest;
    SubjectPtr<BoolDataPoint*> mpFoamDrainRequest;

    bool mReqTaskTimeFlag;

    enum
    {
      FD_IDLE,
      FD_WAIT_FOR_PUMP_START,
      FD_WAIT_FOR_FOAM_DRAIN_LEVEL,
      FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT,
      FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT
    }  mFoamDrainState;
    bool mFoamDrainIntervalTimeOut;
    bool mFoamDrainTimeOut;
    bool mFoamDrainCrossedDryRunAlarm;
    bool mFoamDrainBlockDryRunAlarm;
    bool mFoamDrainCrossedDryRunWarn;
    bool mFoamDrainBlockDryRunWarn;
    bool mUnderDryRunAlarmLevel;

    SwTimer* mpFoamDrainIntervalTimer;
    SwTimer* mpFoamDrainTimer;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

