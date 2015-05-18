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
/* CLASS NAME       : AlarmRelay                                            */
/*                                                                          */
/* FILE NAME        : AlarmRelay.h                                          */
/*                                                                          */
/* CREATED DATE     : 16-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAlarmRelayCtrl_h
#define mrcAlarmRelayCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AlarmControl.h>
#include <SwTimerBassClass.h>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// These values cannot be made as an enum because they are used as input for
// the SetSubjectPointer function and this function requires an int as
// parameter.
#define  ALARM_RELAY_CONNECT_RESET_EVENT     0
#define  ALARM_RELAY_CONNECT_ACK             1
#define  ALARM_RELAY_CONNECT_OUTPUT_VALUE    2
#define  ALARM_RELAY_MANUAL_ACK_PRESENT      3

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  FORWARD DECLARATIONS
 *****************************************************************************/
class AlarmControl;

/*****************************************************************************
 * CLASS: AlarmRelay
 *****************************************************************************/
class AlarmRelay : public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmRelay(AlarmControl* pAlarmControl, ALARM_RELAY_TYPE id);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmRelay();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitAlarmList();
    void CheckAlarmList();
    void AddAlarm(AlarmDataPoint* pAlarm);
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);


  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool RemoveWhenNoAlarm(AlarmDataPoint*);
    bool RemoveWhenReady(AlarmDataPoint*);
    bool RemoveCustom(AlarmDataPoint*);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    AlarmControl* mpAlarmControl;
    ALARM_RELAY_TYPE mAlarmRelayID;
    std::vector<AlarmDataPoint*> mAlarmVector;

    SubjectPtr<BoolDataPoint*> mpManualAckAlarmPresent;
    SubjectPtr<BoolDataPoint*> mpAlarmRelayOutputValue;
    SubjectPtr<BoolDataPoint*> mpAlarmRelayAck;
    SubjectPtr<EventDataPoint*> mpAlarmRelayResetEvent;

    bool mCheckAlarmListFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
