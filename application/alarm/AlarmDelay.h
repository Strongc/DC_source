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
/* CLASS NAME       : AlarmDelay                                            */
/*                                                                          */
/* FILE NAME        : AlarmDelay.h                                          */
/*                                                                          */
/* CREATED DATE     : 06-12-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAlarmDelay_h
#define mrcAlarmDelay_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AlarmDataPoint.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <AlarmConfig.h>                // for access to AC_TYPE enum



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
class AlarmDelay : public SwTimerBaseClass, public Subject
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmDelay(Observer* pCreator);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmDelay(void);

    /********************************************************************
    ASSIGNMENT OPERATORS
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void Update(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int Id, Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    void InitAlarmDelay(void);

    void SetFault(void);
    void SetWarning(void);
    void ResetFault(void);
    void ResetWarning(void);

    bool GetFault(void);
    bool GetWarning(void);

    void UpdateAlarmDataPoint(void);
    void CheckErrorTimers(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/


    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    Observer* mpCreator;

    bool mCheckAlarmDelayFlag;

    AC_TYPE mAlarmType;
    SubjectPtr<AlarmDataPoint*> mpAlarm;
    bool mFault;
    bool mWarning;

    bool mFaultTimerFlag;
    bool mWarningTimerFlag;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

