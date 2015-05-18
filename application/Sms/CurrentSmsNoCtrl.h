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
/* CLASS NAME       : CurrentSmsNoCtrl                                      */
/*                                                                          */
/* FILE NAME        : CurrentSmsNoCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 05-12-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcCurrentSmsNoCtrl_h
#define mrcCurrentSmsNoCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <StringDataPoint.h>
#include <I32VectorDataPoint.h>
#include <U8VectorDataPoint.h>

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
class CurrentSmsNoCtrl : public SubTask, public SwTimerBaseClass
{
  public:
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
    static CurrentSmsNoCtrl* GetInstance();
    bool NumberInPhoneBook(const char*);

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    CurrentSmsNoCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~CurrentSmsNoCtrl();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void CpMonToAllDays(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    //Seconds since midnight, for each change period of each day in week
    SubjectPtr<I32VectorDataPoint*> mDpChangeSmsNoStartimes[NO_OF_SMS_SCHEDULE_PERIOD];
    SubjectPtr<EventDataPoint*> mpCpMon2AllDays;

    SubjectPtr<StringDataPoint*>    mDpCurrentPrimarySmsNo;
    SubjectPtr<StringDataPoint*>    mDpCurrentSecondarySmsNo;

    SubjectPtr<U8VectorDataPoint*>  mDpPrimaryNoId[NO_OF_SMS_SCHEDULE_PERIOD];
    SubjectPtr<U8VectorDataPoint*>  mDpSecondaryNoId[NO_OF_SMS_SCHEDULE_PERIOD];
    SubjectPtr<StringDataPoint*>    mDpPhoneNo[NO_OF_PHONE_NO];

    bool mRunRequestedFlag;
    static CurrentSmsNoCtrl* mInstance;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
