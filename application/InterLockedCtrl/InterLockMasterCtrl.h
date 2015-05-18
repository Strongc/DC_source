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
/* CLASS NAME       : InterLockMasterCtrl                                   */
/*                                                                          */
/* FILE NAME        : InterLockMasterCtrl.h                                 */
/*                                                                          */
/* CREATED DATE     : 22-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcInterLockMasterCtrl_h
#define mrcInterLockMasterCtrl_h

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
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>
#include <StringDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

typedef enum
{
  ILMS_UNLOCKED,
  ILMS_WANT_TO_LOCK,
  ILMS_LOCKED
} INTERLOCK_MASTER_STATE_TYPE;



/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class InterLockMasterCtrl : public SubTask, public SwTimerBaseClass
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    InterLockMasterCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~InterLockMasterCtrl();
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
    void SendInterLockSMS(char* command);
    void CalcInterLockRecallTime(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<BoolDataPoint*> mpLevelAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpUnderLowestStopLevel;

    SubjectPtr<BoolDataPoint*> mpInterLockEnabledPit;
    SubjectPtr<BoolDataPoint*> mpInterLockPinEnabledPit;
    SubjectPtr<U32DataPoint*> mpInterLockPinPit;
    SubjectPtr<U32DataPoint*> mpInterLockTimeoutPit;
    SubjectPtr<StringDataPoint*> mpInterLockPhoneNoPit;
    SubjectPtr<StringDataPoint*> mpInterLockNamePit;

    INTERLOCK_MASTER_STATE_TYPE mInterLockMasterState;

    U32 mInterLockRecallTime;

    bool mSettingChangedFlag;
    bool mInterLockTimeoutTimerFlag;
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
