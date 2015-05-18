/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : SmsScheduleSlipPoint                                  */
/*                                                                          */
/* FILE NAME        : SmsScheduleSlipPoint.H                                */
/*                                                                          */
/* CREATED DATE     : 2007-11-27                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef _SmsScheduleSlipPoint_h
#define _SmsScheduleSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <BoolVectorDataPoint.h>
#include <I32VectorDataPoint.h>
#include <U8VectorDataPoint.h>
#include <BoolDataPoint.h>
#include <I32DataPoint.h>
#include <U8DataPoint.h>
#include <EnumDataPoint.h>
#include <StringDataPoint.h>
#include <WeekdayHeadlineState.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

namespace mpc
{
  namespace display
  {
    namespace ctrl
    {

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class SmsScheduleSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SmsScheduleSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~SmsScheduleSlipPoint();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void InitSubTask(void);
    virtual void RunSubTask(void);

    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject);
    virtual void ConnectToSubjects(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void UpdateVirtualSmsSchedule(void);
    virtual void UpdateCurrentSmsSchedule(void);

    virtual void UpdateVirtualSelectedPhoneNo(void);
    virtual void UpdateCurrentPhoneNoSelection(void);

    virtual void UpdateUpperStatusLineOfEditDisplay(void);
    virtual void UpdateUpperStatusLineOfViewDisplay(void);
    virtual void UpdateUpperStatusLineOfPhoneNoSelectionDisplay(void);

    virtual const char* GetPhoneNo(U8 phoneNoId);
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<EnumDataPoint<SMS_SCHEDULE_TYPE_TYPE>*> mDpCurrentScheduleViewed;

    SubjectPtr<IIntegerDataPoint*>      mDpCurrentWeekday;

    SubjectPtr<BoolVectorDataPoint*>    mDpWorkEnabled;
    SubjectPtr<BoolVectorDataPoint*>    mDpOffEnabled;
    SubjectPtr<BoolVectorDataPoint*>    mDpSleepEnabled;

    SubjectPtr<I32VectorDataPoint*>     mDpWorkStarttime;
    SubjectPtr<I32VectorDataPoint*>     mDpOffStarttime;
    SubjectPtr<I32VectorDataPoint*>     mDpSleepStarttime;

    SubjectPtr<I32VectorDataPoint*>     mDp1stStarttime;
    SubjectPtr<I32VectorDataPoint*>     mDp2ndStarttime;
    SubjectPtr<I32VectorDataPoint*>     mDp3thStarttime;

    SubjectPtr<U8VectorDataPoint*>      mDp1stPriNoId;
    SubjectPtr<U8VectorDataPoint*>      mDp1stSecNoId;
    SubjectPtr<U8VectorDataPoint*>      mDp2ndPriNoId;
    SubjectPtr<U8VectorDataPoint*>      mDp2ndSecNoId;
    SubjectPtr<U8VectorDataPoint*>      mDp3thPriNoId;
    SubjectPtr<U8VectorDataPoint*>      mDp3thSecNoId;

    SubjectPtr<BoolDataPoint*>          mDpVirtualWorkEnabled;
    SubjectPtr<BoolDataPoint*>          mDpVirtualOffEnabled;
    SubjectPtr<BoolDataPoint*>          mDpVirtualSleepEnabled;

    SubjectPtr<I32DataPoint*>           mDpVirtualWorkStarttime;
    SubjectPtr<I32DataPoint*>           mDpVirtualOffStarttime;
    SubjectPtr<I32DataPoint*>           mDpVirtualSleepStarttime;

    SubjectPtr<I32DataPoint*>           mDpVirtual1stStarttime;
    SubjectPtr<I32DataPoint*>           mDpVirtual2ndStarttime;
    SubjectPtr<I32DataPoint*>           mDpVirtual3thStarttime;

    SubjectPtr<StringDataPoint*>        mDpVirtual1stPriNo;
    SubjectPtr<StringDataPoint*>        mDpVirtual1stSecNo;
    SubjectPtr<StringDataPoint*>        mDpVirtual2ndPriNo;
    SubjectPtr<StringDataPoint*>        mDpVirtual2ndSecNo;
    SubjectPtr<StringDataPoint*>        mDpVirtual3thPriNo;
    SubjectPtr<StringDataPoint*>        mDpVirtual3thSecNo;

    SubjectPtr<U8DataPoint*>            mDpCurrentPhoneNoSource;
    SubjectPtr<U8DataPoint*>            mDpVirtualSelectedPhoneNo;

    SubjectPtr<StringDataPoint*>        mDpPhoneNo1;
    SubjectPtr<StringDataPoint*>        mDpPhoneNo2;
    SubjectPtr<StringDataPoint*>        mDpPhoneNo3;

    bool                                mCurrentlyUpdating;

    WeekdayHeadlineState*               mpWeekdayHeadlineState;
    
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
    } // namespace ctrl
  } // namespace display
} // namespace mpc
#endif
