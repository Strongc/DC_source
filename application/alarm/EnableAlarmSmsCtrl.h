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
/* CLASS NAME       : EnableAlarmSmsCtrl                                    */
/*                                                                          */
/* FILE NAME        : EnableAlarmSmsCtrl.h                                  */
/*                                                                          */
/* CREATED DATE     : 25-02-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Subscribes to 2 Event datapoints. One event sets SCADA enabled for all   */
/* alarm configs. The other event sets SMS 1-3 enabled for all alarm configs*/
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcEnableAlarmSmsCtrl_h
#define mrcEnableAlarmSmsCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SubTask.h>
#include <SubjectPtr.h>
#include <EventDataPoint.h>
#include <AlarmConfig.h>

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
 * CLASS: EnableAlarmSmsCtrl
 *****************************************************************************/
class EnableAlarmSmsCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    EnableAlarmSmsCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~EnableAlarmSmsCtrl();
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
    void SetScadaAlarms(bool enable);
    void SetSmsAlarms(bool enable);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EventDataPoint*> mDpEnableScadaForAllAlarmConfigsEvent;
    SubjectPtr<EventDataPoint*> mDpEnableSmsForAllAlarmConfigsEvent;

    std::vector< SubjectPtr<AlarmConfig*> > mAlarmConfigs;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
