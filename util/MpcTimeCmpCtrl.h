/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : MpcTimeCmpCtrl                                        */
/*                                                                          */
/* FILE NAME        : MpcTimeCmpCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef MpcTimeCmpCtrl_h
#define MpcTimeCmpCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <clock.h>
#include <Observer.h>
#include <Acttime.h>
#include <SubTask.h>
#include <MPCTime.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  CMP_DATE_AND_TIME,
  CMP_WEEKDAY_AND_TIME,
  CMP_TIME,
  DAY_TRIGGER,        // To generate an event when the clock passes midnight
  HOUR_TRIGGER,       // To generate an event every clock hour
  MINUTE_TRIGGER      // To generate an event every clock minute
}TIMER_CMP_PRECISION;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class MpcTimeCmpCtrl : public Observer, public SubTask, public Subject
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    MpcTimeCmpCtrl(MpcTime* pTimeObj, Observer* pObserver, TIMER_CMP_PRECISION cmpPrecision);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~MpcTimeCmpCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void ChangeTime(MpcTime* pTimeObj);
    // Functions from Subtask
    virtual void RunSubTask();
    virtual void InitSubTask(){};

    // Functions from Observer.
    virtual void SetSubjectPointer(int Id, Subject* pSubject){};
    virtual void ConnectToSubjects(){};
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject) {};

    protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool mNewTime;

    int mCurrentMinute;
    int mCurrentHour;
    int mCurrentDay;

    MpcTime* mpTimeObj;

    TIMER_CMP_PRECISION mCmpPrecision;
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
