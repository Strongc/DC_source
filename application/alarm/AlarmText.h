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
/* CLASS NAME       : AlarmText                                             */
/*                                                                          */
/* FILE NAME        : AlarmText.h                                           */
/*                                                                          */
/* CREATED DATE     : 25-11-2010 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Converts alarm id to string id and string.      */
/*   Used by ActualAlarmString, AlarmListItem and AlarmSnapShotSlipPoint.   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ALARM_TEXT_H__
#define __ALARM_TEXT_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <SubjectPtr.h>
#include <Observer.h>
#include <StringDataPoint.h>


/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AlarmDef.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 * CLASS: AlarmLog
 *****************************************************************************/
class AlarmText : public Observer
{
  public:

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static STRING_ID GetStringId(ALARM_ID_TYPE alarmId);
    const char* GetString(ALARM_ID_TYPE alarmId, int erroneousUnitNumber);
    StringDataPoint* GetCombiAlarm(int erroneousUnitNumber);
    StringDataPoint* GetExtraFault(int erroneousUnitNumber);


    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

    static AlarmText* GetInstance();
private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmText();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmText();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static AlarmText* mInstance;
    SubjectPtr<StringDataPoint*> mDpCombiAlarmName[4];
    SubjectPtr<StringDataPoint*> mDpExtraFaultName[4];

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif

