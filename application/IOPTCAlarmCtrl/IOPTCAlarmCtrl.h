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
/* CLASS NAME       : IOPTCAlarmCtrl                                        */
/*                                                                          */
/* FILE NAME        : IOPTCAlarmCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     : 01-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Each instance of the IOPTCAlarmCtrl is related  */
/*                          to one pump, but can surveil two PTC alarm      */
/*                          inputs. If one or both of the alarm inputs      */
/*                          signal an alarm - and the pump is present and   */
/*                          ready - an alarm will be indicated. Otherwise   */
/*                          it is cleared.                                  */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcIOPTCAlarmCtrl_h
#define mrcIOPTCAlarmCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <U32DataPoint.h>
#include <EnumDataPoint.h>
#include <AlarmDelay.h>

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
class IOPTCAlarmCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    IOPTCAlarmCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~IOPTCAlarmCtrl();
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
    void SetSubjectPointer(int Id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void CheckForPtcAlarm();
    void RaisePtcAlarm();
    void PumpBasedPtcAlarmOn();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    U32 mPtcStatus;
    bool mRunRequestedFlag;    
    bool mPtcAlarmOn[NO_OF_PTC_INPUT_ALARM_FUNC];    
    bool mPumpPtcAlarmOn[MAX_NO_OF_PUMPS];
    bool mPumpMoistureAlarmOn[MAX_NO_OF_PUMPS];
    AlarmDelay* mpIOPtcAlarmDelay[MAX_NO_OF_PUMPS];
    bool mIOPtcAlarmDelayCheckFlag[MAX_NO_OF_PUMPS];
    AlarmDelay* mpIOMoistureAlarmDelay[MAX_NO_OF_PUMPS];
    bool mIOMoistureAlarmDelayCheckFlag[MAX_NO_OF_PUMPS];

    SubjectPtr<U32DataPoint*> mpIO351PtcInStatusBits[MAX_NO_OF_IO351];
    SubjectPtr<EnumDataPoint<PTC_INPUT_FUNC_TYPE>*> mpPtcConfigDP[MAX_NO_OF_PTC_INPUTS];
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpActualOperationMode[MAX_NO_OF_PUMPS];
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
