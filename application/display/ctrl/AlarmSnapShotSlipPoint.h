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
/* CLASS NAME       : AlarmSnapShotSlipPoint                                */
/*                                                                          */
/* FILE NAME        : AlarmSnapShotSlipPoint.H                              */
/*                                                                          */
/* CREATED DATE     : 19-06-2009                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef _AlarmSnapShotSlipPoint_h
#define _AlarmSnapShotSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <AlarmLog.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>
#include <StringDataPoint.h>
#include <BoolDataPoint.h>
#include <TimeText.h>

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
class AlarmSnapShotSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmSnapShotSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~AlarmSnapShotSlipPoint();
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
    virtual void UpdateAlarmSnapShot(void);
    
    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<IIntegerDataPoint*>          mDpCurrentNumber;

    SubjectPtr<AlarmLog*>	                  mpAlarmLog;

    SubjectPtr<EnumDataPoint<ALARM_STATE_TYPE>*>	mDpVirtualAlarmState;
    SubjectPtr<StringDataPoint*>	          mDpVirtualErrorUnit;
    SubjectPtr<StringDataPoint*>	          mDpVirtualAlarmText;
    SubjectPtr<StringDataPoint*>	          mDpVirtualOccurredAt;
    SubjectPtr<StringDataPoint*>	          mDpVirtualDisappearedAt;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualSystemLevel;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualSystemFlow;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*>	mDpVirtualOperationModePump[NO_OF_PUMP];
    SubjectPtr<FloatDataPoint*>	            mDpVirtualPumpFlow;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualPumpTemp;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualPumpMainsVoltage;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualPumpLatestCurrent;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualPumpCosPhi;
    SubjectPtr<FloatDataPoint*>	            mDpVirtualPumpPower;
    SubjectPtr<BoolDataPoint*>	            mDpVirtualIsAPumpAlarm;

    bool                                    mCurrentlyUpdating;

    TimeText* mpTimeAsText;
    
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
