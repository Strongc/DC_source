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
/* CLASS NAME       : UserDefinedCounterCtrl                                */
/*                                                                          */
/* FILE NAME        : UserDefinedCounterCtrl.h                              */
/*                                                                          */
/* CREATED DATE     : 16-12-2011 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcUserDefinedCounterCtrl_h
#define mrcUserDefinedCounterCtrl_h

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
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <U16Datapoint.h>
#include <FloatDataPoint.h>
#include <StringDataPoint.h>
#include <EventDataPoint.h>
#include <MpcTimeCmpCtrl.h>

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
class UserDefinedCounterCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    UserDefinedCounterCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~UserDefinedCounterCtrl();
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
    void UpdateAccumulatedUSDCounter(int use_counter);
    void CheckUSDCounterConfig(int use_counter); 
    void HandleResetEvents();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs
      SubjectPtr<U32DataPoint*>     mpPulseUSDCntRatioDisplay[NO_OF_USD_COUNTERS];

    // Variable inputs:
      SubjectPtr<U32DataPoint*>     mpRawUserDefinedCntPulses[NO_OF_USD_COUNTERS];

    // Outputs:
      SubjectPtr<U32DataPoint*>     mpTotalUserDefinedCount[NO_OF_USD_COUNTERS];

    // Outputs:
      SubjectPtr<EventDataPoint*>   mpResetUserDefinedCounter[NO_OF_USD_COUNTERS];

	  SubjectPtr<U32DataPoint*>     mpHourLogTimestamp[NO_OF_USD_COUNTERS];

	  SubjectPtr<U16DataPoint*>     mpYesterdayCounter[NO_OF_USD_COUNTERS];

	  SubjectPtr<U16DataPoint*>     mpTodayCounter[NO_OF_USD_COUNTERS];

    // Class variables
      bool                          mPulsesReady[NO_OF_USD_COUNTERS];
      U8                            mOldPulses[NO_OF_USD_COUNTERS];
	    float                         mIncrement[NO_OF_USD_COUNTERS];
      int                           mOldPulseRatioDisplay[NO_OF_USD_COUNTERS];

	  //Yesterday/Today counter
	  int							today_cnt[NO_OF_USD_COUNTERS];
	  int							total_today_cnt[NO_OF_USD_COUNTERS];
	  int							yesterday[NO_OF_USD_COUNTERS];

	  MpcTime*          mTimeChangeObj;
	  MpcTimeCmpCtrl*   mTimeCompareObs;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
