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
/* CLASS NAME       : AlarmSlipPoint                                        */
/*                                                                          */
/* FILE NAME        : AlarmSlipPoint.h                                      */
/*                                                                          */
/* CREATED DATE     : 05-11-2007                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the alarm config DataPoints into common                  */
/* virtual DataPoints for the display to look at...                         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_AlarmSlipPoint_h
#define mrc_AlarmSlipPoint_h

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
#include <AlarmConfig.h>
#include <AlarmState.h>

#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <I32DataPoint.h>
#include <U32DataPoint.h>
#include <U8DataPoint.h>
#include <FloatDataPoint.h>
#include <StringDataPoint.h>

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
class AlarmSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~AlarmSlipPoint();
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
    virtual void UpdateVirtualAlarm(void);
    virtual void UpdateCurrentAlarm(void);
    virtual void UpdateUpperStatusLine(void);
    virtual void SetTitle(Display* pDisplay, char* displayNumber, int alarmNo);
    virtual void UpdateCategory(bool reversed = false);
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<IIntegerDataPoint*>  mpCurrentAlarmNumber;
    SubjectPtr<EnumDataPoint<PUMP_GROUP_TYPE>*> mpCurrentPumpGroup;
    SubjectPtr<BoolDataPoint*> 	    mpIsHighEnd;
    SubjectPtr<BoolDataPoint*> 	    mpPumpGroupsEnabled;

    SubjectPtr<AlarmConfig*> 	      mpAlarmDP[NO_OF_ALARM_CONFIG][NO_OF_PUMP_GROUPS];
    SubjectPtr<StringDataPoint*> 	  mpExtraFaultName[4];
    
    SubjectPtr<EnumDataPoint<ALARM_CATEGORY_TYPE>*> mpVirtualCategory;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualSms1Enabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualSms2Enabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualSms3Enabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualScadaEnabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualWarningEnabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualAlarmEnabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualCustomRelayForWarningEnabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualCustomRelayForAlarmEnabled;
    SubjectPtr<BoolDataPoint*> 	    mpVirtualAutoAck;
    SubjectPtr<FloatDataPoint*> 	  mpVirtualDelay;
    SubjectPtr<FloatDataPoint*> 	  mpVirtualWarningLimitFloat;
    SubjectPtr<I32DataPoint*> 	    mpVirtualWarningLimitInt;
    SubjectPtr<FloatDataPoint*> 	  mpVirtualAlarmLimitFloat;
    SubjectPtr<I32DataPoint*> 	    mpVirtualAlarmLimitInt;

    bool                            mCurrentlyUpdating;

    AlarmState*                     mpAlarmState;
    
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
