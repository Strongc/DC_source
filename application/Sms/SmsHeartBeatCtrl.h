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
/* CLASS NAME       : SmsHeartBeatCtrl                                      */
/*                                                                          */
/* FILE NAME        : SmsHeartBeatCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 27-11-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : SmsHeartbeat sends an sms each day to say:      */
/*                          "I am alive"                                    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcSmsHeartBeatCtrl_h
#define mrcSmsHeartBeatCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <MpcTimeCmpCtrl.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <BoolDataPoint.h>
#include <I32DataPoint.h>
#include <U8DataPoint.h>

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
class SmsHeartBeatCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    SmsHeartBeatCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~SmsHeartBeatCtrl();
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
    void SendHeartBeatSms(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    MpcTime* mpHeartBeatTimeObj;
    MpcTimeCmpCtrl* mpHeartBeatTimeCompareObs;
    SubjectPtr<BoolDataPoint*> mpDaysED[7];        //One for each weekday
    SubjectPtr<I32DataPoint*> mpHeartBeatTimeCfg;  //Seounds since midnight
    SubjectPtr<U8DataPoint*> mpNoOfPumps;

    bool mTimeForHeartBeat;
    bool mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
