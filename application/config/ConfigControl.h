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
/* CLASS NAME       : ConfigControl                                         */
/*                                                                          */
/* FILE NAME        : ConfigControl.h                                       */
/*                                                                          */
/* CREATED DATE     : 19-09-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __CONFIG_CONTROL_H__
#define __CONFIG_CONTROL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>
#include <map>
#include <queue>
#include <fifo.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <SubTask.h>
#include <SwTimerBassClass.h>
#include <SwTimer.h>
#include <AlarmDataPoint.h>
#include <AlarmLog.h>
#include <AlarmDelay.h>
#include <U16VectorDataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <FlashControl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define NO_OF_CONFIG_SETUP_ID_CODE  4

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

typedef enum
{
  CONFIG_BLOCK,
  LOG_BLOCK,
  GSC_BLOCK,
  NO_BOOT_BLOCK,
  USER_LOG_BLOCK_1,
  USER_LOG_BLOCK_2,
  ALL_BLOCKS
} BLOCK_ID_TYPE;

typedef std::map<U16, Subject*> SUBJECT_MAP_TYPE;
typedef SUBJECT_MAP_TYPE::iterator SUBJECT_MAP_ITR;
typedef Fifo<Subject*, 1000> SUBJECT_QUEUE_TYPE;

/* Enum for alarms in this module */
typedef enum
{
  FIRST_CC_FAULT_OBJ,
  CC_FAULT_OBJ_HW_ERROR = FIRST_CC_FAULT_OBJ,

  NO_OF_CC_FAULT_OBJ,
  LAST_CC_FAULT_OBJ = NO_OF_CC_FAULT_OBJ - 1
}CC_FAULT_OBJ_TYPE;


/*****************************************************************************
  FORWARDS
 *****************************************************************************/
class FlashBlock;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class ConfigControl : public SwTimerBaseClass
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static ConfigControl* GetInstance();

    // Observer
    void SetSubjectPointer(int id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // Operations called from RunFactory (in the order listed below)
    void SetSubjectPointerSpecial(Subject* pSubject);
    void LoadFromFlash();
    void ConnectToSubjectsSpecial();


    #ifdef __PC__
    // called by DllExportedMethods
    void AsyncCopyFlashFiles(const char* path, bool save);
    #endif //__PC__

    void Init();
    void Run();

    // Misc.
    void Boot(BLOCK_ID_TYPE blockId);
    void PowerDown();
    void SaveLogSeriesToFlash();

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ConfigControl();

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ConfigControl();

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void SignalConfigTask(void);
    void ExecuteBoot();
    void CheckAlarmFlags(void);
    void CopyLogSeriesToCache();
    void Doit();
    void ResetConfigSetUpId();

    #ifdef __PC__
    void CopyFlashFiles(bool save);
    #endif //__PC__

    friend class FlashBlock;
    Subject* GetSubject(U16 subjectId);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static ConfigControl *mInstance;

    bool mOkToSaveFlag;
    bool mExecuteBootFlag;
    bool mBootConfigBlockFlag;
    bool mBootLogBlockFlag;
    bool mBootGscBlockFlag;
    bool mPowerDownFlag;
    bool mTimeToSaveLogDataFlag;
    bool mTimeToSaveAlarmLogFlag;

    FlashBlock* mpConfigBlock;
    FlashBlock* mpLogBlock;
    FlashBlock* mpGSCBlock;
    FlashBlock* mpNoBootBlock;
    FlashBlock* mpUserLogBlock1;
    FlashBlock* mpUserLogBlock2;
    BLOCK_ID_TYPE mNextLogBlockToSave;

    SUBJECT_MAP_TYPE mSubjects;
    SUBJECT_QUEUE_TYPE mSubjectQueue;

    SubjectPtr<AlarmLog*> mpAlarmLog;

    OS_TASK mTCBConfigTask;
    OS_STACKPTR int mConfigTaskStack[2048];
    OS_RSEMA mSemaSubjectQueue;

    /* Variables for alarm handling */
    AlarmDelay* mpConfigControlAlarmDelay[NO_OF_CC_FAULT_OBJ];
    bool mConfigControlAlarmDelayCheckFlag[NO_OF_CC_FAULT_OBJ];

    /* Variables for Config setup id code */
    SubjectPtr<U32DataPoint*> mpConfigSetupIDCode[NO_OF_CONFIG_SETUP_ID_CODE];
    SubjectPtr<BoolDataPoint*> mpResetConfigSetIDCode;

  #ifdef __PC__
    // needed for AsyncCopyFlashFiles
    bool mLoadFromFlashRequested;
    bool mSaveToFlashRequested;
    char mFlashPath[MAX_PATH];
  #endif


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
