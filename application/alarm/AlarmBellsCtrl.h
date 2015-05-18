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
/* CLASS NAME       : AlarmBellCtrl                                         */
/*                                                                          */
/* FILE NAME        : AlarmBellCtrl.h                                       */
/*                                                                          */
/* CREATED DATE     : 02-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Generates alarm bell status parameters to be    */
/*                          used by the display                             */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAlarmBellsCtrl_h
#define mrcAlarmBellsCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
#include <AlarmLog.h>
#include <AppTypeDefs.h>
#include <EnumDataPoint.h>

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
class AlarmBellsCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmBellsCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmBellsCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void UpdateState(AlarmEvent* pAlarmEvent, ALARM_STATE_TYPE* state);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<AlarmLog*> mpAlarmLog;
    SubjectPtr<EnumDataPoint<ALARM_STATE_TYPE>*> mpPumpBell[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<ALARM_STATE_TYPE>*> mpSystemBell ;
    SubjectPtr<EnumDataPoint<ALARM_STATE_TYPE>*> mpMixerBell;

    bool mReqTaskTimeFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
