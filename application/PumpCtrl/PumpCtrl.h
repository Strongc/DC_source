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
/* CLASS NAME       : PumpCtrl                                              */
/*                                                                          */
/* FILE NAME        : PumpCtrl.h                                            */
/*                                                                          */
/* CREATED DATE     : 12-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPumpCtrl_h
#define mrcPumpCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <subtask.h>
#include <U32DataPoint.h>
#include <U8DataPoint.h>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_NO_OF_GROUPS 2

typedef  enum
{
  TEMP_STOP_IGNORE = 0,
  TEMP_STOP_STOP,
  TEMP_STOP_RESUME,
  // insert above here
  NO_OF_TEMP_STOP,
  LAST_TEMP_STOP = NO_OF_TEMP_STOP - 1
} TEMP_STOP_TYPE;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

class PumpRef
{
    public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PumpRef();
    PumpRef(int noOfPumps);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PumpRef();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    PumpRef& operator=(const PumpRef& src);

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void SetPumpState(int number, bool started);
    bool GetPumpState(int number);
    //void SetPumpRef(PumpRef ref);
    //PumpRef GetPumpRef(void);
    int GetNumberOfStartedPumps( void );
    void Clear(void);
    void SetRef(int number, bool state=true);
    void ClearRef(int number);
    int NoOfPumps( void );
    void ShiftRefs( int shift );
    bool GetRef(int number);
    void SetNoOfPumps(int noOfPumps);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    //PumpRef mPumpRef;
    bool mPumpStart[MAX_NO_OF_PUMPS];
    int mNoOfPumps;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

 /*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class PumpCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PumpCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PumpCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void FindAvailablePumps(void);
    int GetPumpByTime(int index);
    TEMP_STOP_TYPE CheckTempStop(int no_of_pumps_wish, int manual_started, U8 *tempStopPump);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<BoolDataPoint*> mpDailyEmptyingRequested;
    SubjectPtr<BoolDataPoint*> mpPumpReadyForAutoOpr[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*> mpAlternatingActive;
    SubjectPtr<EnumDataPoint<APPLICATION_MODE_TYPE>*> mpAppMode;
    SubjectPtr<U32DataPoint*> mpStartUpDelay;
    SubjectPtr<BoolDataPoint*> mpAntiSeizingRequestPump[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*> mpMaxNoOfStartedPumps;
    SubjectPtr<BoolDataPoint*>mpInterlocked;
    SubjectPtr<U8DataPoint*> mpNoOfAvailablePumps;
    SubjectPtr<BoolDataPoint*>mpFoamDrainRequest;
    SubjectPtr<BoolDataPoint*>mpGroup2Enabled;
    SubjectPtr<U32DataPoint*> mpGroup2FirstPump;
    SubjectPtr<BoolDataPoint*>mpRefLevel[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>mpPumpRunForLowestStop[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>mpGroupAlternation;
    SubjectPtr<BoolDataPoint*>mpAlternationInGroup[MAX_NO_OF_GROUPS];
    SubjectPtr<U32DataPoint*> mpMaxStartedPumpsInGroup[MAX_NO_OF_GROUPS];
    SubjectPtr<U32DataPoint*> mpMinStartedPumpsInGroup[MAX_NO_OF_GROUPS];
    SubjectPtr<BoolDataPoint*>mpPumpReady[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpOprModeActPump[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<PUMP_OPERATION_MODE_TYPE>*> mpOprModeRefPump[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>mpGroupsMayRunTogether;
    SubjectPtr<U32DataPoint*> mpMinStartedPumpsInTotal;
    SubjectPtr<U32DataPoint*> mpMaxStartedPumpsInTotal;
    SubjectPtr<U8DataPoint*> mpNoOfPumps;
    SubjectPtr<U8DataPoint*> mpAdvFlowTempStopRequest;
    SubjectPtr<BoolDataPoint*> mpHighLevelStateDetected;
    SubjectPtr<U32DataPoint*> mpLastRunTime[NO_OF_PUMPS];

    bool mPumpAvailable[MAX_NO_OF_PUMPS];
    bool mRunRequestedFlag;
    //bool mPump1StartOld;
    //bool mPump2StartOld;
    int mAlternationStartGroup;
    //PumpRef mNextPumpChange;
    //int mPumpsAvailable;
    //PumpRef mAvailablePump;
    bool mStartUpDelayTimeout;
    PumpRef mPumpRefOld;
    PumpRef mFoamDrainRef;
    U8    mPumpInTempStop;
    bool  mTempStopAllowed;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};


#endif
