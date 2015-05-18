/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AlarmLog                                              */
/*                                                                          */
/* FILE NAME        : AlarmLog.h                                            */
/*                                                                          */
/* CREATED DATE     : 17-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : AlarmLog take care of adding a new element to   */
/* the alarm log, and know which element to delete if the log is full. It   */
/* also take care of marking departured alarms. Reset and clear of the      */
/* alarm log is handled by this class.                                      */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ALARM_LOG_H__
#define __ALARM_LOG_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <deque>
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subject.h>
#include <SubjectPtr.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmDef.h>
#include <AlarmEvent.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define ALARM_LOG_SIZE  24

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: AlarmLog
 *****************************************************************************/
class AlarmLog : public Subject, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmLog();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmLog();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    AlarmEvent* GetAlarmLogElement(int index);
    std::deque<AlarmEvent*>::iterator GetFirstAlarmLogElement();
    bool PutAlarmLogElement(AlarmDataPoint* pErrorSource, U32 alarmEventNumber);
    void ErrorDepartured(AlarmDataPoint* pErrorSource);
    void ResetAlarmLog(RESET_TYPE resetType);
    void ClearAlarmLog();
    bool HasUnAckAlarms();
    bool ExecuteNotify();
    void SetDepartureTimeInAllActiveEvents();
    void UseAlarmLog();
    void UnuseAlarmLog();

    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

  	virtual FLASH_ID_TYPE GetFlashId(void);
  	virtual void SaveToFlash(IFlashWriter* pWrite, FLASH_SAVE_TYPE save);
  	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void LoadFromFlashLogVersion1(IFlashReader* pReader, AlarmEvent* pAlarmEvent);
    void LoadFromFlashLogVersion2(IFlashReader* pReader, AlarmEvent* pAlarmEvent);
    void LoadFromFlashLogVersion3(IFlashReader* pReader, AlarmEvent* pAlarmEvent);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    std::deque<AlarmEvent*> mAlarmLog;
    bool mAlarmLogChangedFlag;
    OS_RSEMA mSemaAlarmLog;

    SubjectPtr<BoolDataPoint*> mDpHasUnAckAlarms;
    SubjectPtr<U32DataPoint*>  mDateTimeOfLatestClear;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
