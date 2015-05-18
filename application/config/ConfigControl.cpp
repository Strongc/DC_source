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
/* FILE NAME        : ConfigControl.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 19-09-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <string.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Subject.h>
#include <AppTypeDefs.h>
#include <NonVolatileDataTables.h>
#include <StringDataPoint.h>
#include <FactoryTypes.h>
#include <PowerDown.h>
#include <SoftwareVersion.h>
#include <FloatDataPoint.h>
#include <I32DataPoint.h>
#include <AlarmDataPoint.h>

#ifdef __PC__
#include <BaseDirectory.h>
#include <PcMessageService.h>
#endif //__PC__

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ConfigControl.h>
#include <FlashBlock.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  SAVE_LOG_DATA_TIMER,
  DELAY_SAVE_OF_ALARM_LOG_TIMER
};

/* Task prio bellow 100 dosn't run on pc */
#ifdef __PC__
#define CONFIG_TASK_PRIO        100
#else
#define CONFIG_TASK_PRIO        5
#endif

#define RUN_EVENT   1

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  INITIALIZE STATIC ATTRBUTES
 ******************************************************************************/
ConfigControl* ConfigControl::mInstance = NULL;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
ConfigControl* ConfigControl::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new ConfigControl();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - ConfigTask
 * DESCRIPTION:
 *
 *****************************************************************************/
EXTERN void ConfigTask(void)
{
  ConfigControl::GetInstance()->Run();
}

/*****************************************************************************
 * Function - Init
 * DESCRIPTION:
 *
 *****************************************************************************/
void ConfigControl::Init()
{
  OS_CREATETASK(&mTCBConfigTask, "Config Control Task", ConfigTask, CONFIG_TASK_PRIO, mConfigTaskStack);
}

/*****************************************************************************
 * Function - SignalConfigTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void ConfigControl::SignalConfigTask(void)
{
  OS_SignalEvent(RUN_EVENT, &mTCBConfigTask);
}

/*****************************************************************************
 * Function - Run
 * DESCRIPTION: Empties event queue, calls save rutines if data is changed.
 * If boot event is received, then boot is executed.
 * The actual job is done in Doit
 *****************************************************************************/
void ConfigControl::Run()
{
  FlashControl::GetInstance()->Init();

  /* Initialize AlarmDelay's */
  for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
  {
    mpConfigControlAlarmDelay[fault_id]->InitAlarmDelay();
  }

  // check alarm flags
  CheckAlarmFlags();

  // save some blocks to flash initially
  mpConfigBlock->SaveToFlash();
  mpNoBootBlock->SaveToFlash();

  // enter loop until power down
  while (!mPowerDownFlag)
  {
    OS_WaitEvent(RUN_EVENT);

    /* Check if an AlarmDelay requests for attention - expired error timer. */
    for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
    {
      if (mConfigControlAlarmDelayCheckFlag[fault_id] == true)
      {
        mConfigControlAlarmDelayCheckFlag[fault_id] = false;
        mpConfigControlAlarmDelay[fault_id]->CheckErrorTimers();
      }
    }

    // execute boot?
    if (mExecuteBootFlag)
    {
      mExecuteBootFlag = false;
      ExecuteBoot();
    }

  #ifdef __PC__
    if (mLoadFromFlashRequested)
    {
      mLoadFromFlashRequested = false;
      CopyFlashFiles(false);
    }
    if (mSaveToFlashRequested)
    {
      mSaveToFlashRequested = false;
      CopyFlashFiles(true);
    }
  #endif // __PC__

    // ok to save?
    if (mOkToSaveFlag)
    {
      // DO IT
      Doit();

      if (!mPowerDownFlag)
      {
        // check alarm flags
        CheckAlarmFlags();
      }
    }
    else
    {
      // NOT ok to save (during boot up), ensure that our queue doesn't grow
      OS_Use(&mSemaSubjectQueue);
      while (!mSubjectQueue.empty())
      {
        mSubjectQueue.read();
      }
      OS_Unuse(&mSemaSubjectQueue);
    }

    /* Update the AlarmDataPoints with the changes made in AlarmDelay */
    for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
    {
      mpConfigControlAlarmDelay[fault_id]->UpdateAlarmDataPoint();
    }
  }

  // power down detected. If data has arrived during last run of Doit() these data must be saved
  if (mSubjectQueue.size() > 0)
  {
    Doit();
  }

  // signal power down complete
  SignalEventToPowerDown(CONFIG_CTRL_POWERED_DOWN_EVENT);

  while (1)
  {
    OS_Delay(1000);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void ConfigControl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    /* Redirect SetSubjectPointer to AlarmDelays - and create an attach to the subject if
     * access is needed to alarm limits */
    case SP_CC_ALARM_OBJ:
      mpConfigControlAlarmDelay[CC_FAULT_OBJ_HW_ERROR]->SetSubjectPointer(id, pSubject);
      break;

    case SP_CC_CONFIG_SETUP_ID_CODE_1:
      mpConfigSetupIDCode[0].Attach(pSubject);
      break;
    case SP_CC_CONFIG_SETUP_ID_CODE_2:
      mpConfigSetupIDCode[1].Attach(pSubject);
      break;
    case SP_CC_CONFIG_SETUP_ID_CODE_3:
      mpConfigSetupIDCode[2].Attach(pSubject);
      break;
    case SP_CC_CONFIG_SETUP_ID_CODE_4:
      mpConfigSetupIDCode[3].Attach(pSubject);
      break;
    case SP_CC_RESET_CONFIG_SETUP_ID_CODE:
      mpResetConfigSetIDCode.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void ConfigControl::ConnectToSubjects()
{
  /* Redirect ConnectToSubjects to AlarmDelay's */
  for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
  {
    mpConfigControlAlarmDelay[fault_id]->ConnectToSubjects();
  }

  for (int i = 0; i < NO_OF_CONFIG_SETUP_ID_CODE; i++)
  {
    mpConfigSetupIDCode[i]->Subscribe(this);
  }      

  mpResetConfigSetIDCode->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void ConfigControl::Update(Subject* pSubject)
{
  /* Updates from AlarmDelay's */
  if (pSubject == mpConfigControlAlarmDelay[CC_FAULT_OBJ_HW_ERROR])
  {
    mConfigControlAlarmDelayCheckFlag[CC_FAULT_OBJ_HW_ERROR] = true;
  }

  else if (pSubject == mpTimerObjList[SAVE_LOG_DATA_TIMER])
  {
    mTimeToSaveLogDataFlag = true;
  }
  else if (pSubject == mpTimerObjList[DELAY_SAVE_OF_ALARM_LOG_TIMER])
  {
    mTimeToSaveAlarmLogFlag = true;
  }
  else
  {
  #ifdef __PC__
    if (mOkToSaveFlag)
    {
      OS_Use(&mSemaSubjectQueue);
      mSubjectQueue.write(pSubject);
      OS_Unuse(&mSemaSubjectQueue);
    }
    //else ignore subject if not ready (during boot up) on pc
  #else
    OS_Use(&mSemaSubjectQueue);
    mSubjectQueue.write(pSubject);
    OS_Unuse(&mSemaSubjectQueue);
  #endif
  }

  SignalConfigTask();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION: If pSubject is a pointer of ConfigContol then remove from list
 *
 *****************************************************************************/
void ConfigControl::SubscribtionCancelled(Subject* pSubject)
{
  SUBJECT_MAP_ITR itr = mSubjects.find(pSubject->GetSubjectId());

  if (itr != mSubjects.end())
  {
    mSubjects.erase(itr);
    pSubject->Unsubscribe(this);

    if (pSubject->GetSubjectId() == SUBJECT_ID_ALARM_LOG)
    {
      mpAlarmLog.Detach(pSubject);
    }
  }

  /*Redirect cancelling of subscribtions to AlarmDelay's */
  for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
  {
    mpConfigControlAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - SetSubjectIdAndPointer
 * DESCRIPTION: This is a special function that takes the SUBJECT ID as parameter
 * The function is called from FactoryCode3.cpp
 *
 *****************************************************************************/
void ConfigControl::SetSubjectPointerSpecial(Subject* pSubject)
{
  // store subject
  mSubjects[pSubject->GetSubjectId()] = pSubject;

  // special case: we need the alarm log subject for "delayed save handling"
  if (pSubject->GetSubjectId() == SUBJECT_ID_ALARM_LOG)
  {
    mpAlarmLog.Attach(pSubject);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjectsSpecial
 * DESCRIPTION: This is a special function that Subscribe to the subjects
 * saved by ConfigControl. The function is called from FactoryCode3.cpp
 *****************************************************************************/
void ConfigControl::ConnectToSubjectsSpecial()
{
  for (SUBJECT_MAP_ITR itr = mSubjects.begin(); itr != mSubjects.end(); itr++)
  {
    switch (itr->second->GetSubjectId())
    {
      // For log series a subscription makes too many updates, handled othervise.
      case SUBJECT_ID_LOG_SERIES_1_DATA:
      case SUBJECT_ID_LOG_SERIES_2_DATA:
      case SUBJECT_ID_LOG_SERIES_3_DATA:
      case SUBJECT_ID_LOG_SERIES_4_DATA:
      case SUBJECT_ID_LOG_SERIES_5_DATA:
        break;

      default:
        // auto subscribe to the subject
        itr->second->Subscribe(this);
        break;
    }
  }
}

/*****************************************************************************
 * Function - LoadFromFlash
 * DESCRIPTION: Loads configuration from flash if it is valid.
 * If not valid then defaults defined in the DB is used.
 * The function is called from FactoryCode3.cpp
 *****************************************************************************/
void ConfigControl::LoadFromFlash()
{
  FlashControl* pFlashControl = FlashControl::GetInstance();
  const U32 oldSWVersionNo = pFlashControl->ReadSwVersionNo();

  if (oldSWVersionNo != CPU_SW_VERSION_NO)
  {
    // maybe do something here
  }

  // load config block
  mpConfigBlock->LoadFromFlash();

  // load log block
  mpLogBlock->LoadFromFlash();

  // load GSC block
  mpGSCBlock->LoadFromFlash();

  // load no-boot block
  mpNoBootBlock->LoadFromFlash();

  // load user log blocks
  mpUserLogBlock1->LoadFromFlash();
  mpUserLogBlock2->LoadFromFlash();

  // write new software version to flash
  pFlashControl->WriteSwVersionNo(CPU_SW_VERSION_NO);

}


#ifdef __PC__
/*****************************************************************************
 * Function - AsyncCopyFlashFiles
 * DESCRIPTION: sets flag to request saving or reload of flash files
 *
 *****************************************************************************/
void ConfigControl::AsyncCopyFlashFiles(const char* path, bool save)
{
  strncpy(mFlashPath, path, MAX_PATH-1);
  mFlashPath[MAX_PATH-1] = '\0';

  if (save)
  {
    mSaveToFlashRequested = true;
  }
  else
  {
    mLoadFromFlashRequested = true;
  }
}
#endif


/*****************************************************************************
 * Function - Boot
 * DESCRIPTION: Boot of a block. CONFIG_BLOCK | LOG_BLOCK | GSC_BLOCK | ALL_BLOCKS
 * It is not possible to boot NO_BOOT_BLOCK
 *
 *****************************************************************************/
void ConfigControl::Boot(BLOCK_ID_TYPE blockId)
{
  mExecuteBootFlag = true;

  // don't allow any more flashwrites before reboot (flag is set true in constructor).
  mOkToSaveFlag = false;

  switch (blockId)
  {
    case ALL_BLOCKS:
    case CONFIG_BLOCK:
      mBootConfigBlockFlag = true;
    if (blockId != ALL_BLOCKS) break;

    case LOG_BLOCK:
      mBootLogBlockFlag = true;
    if (blockId != ALL_BLOCKS) break;

    case GSC_BLOCK:
      mBootGscBlockFlag = true;
    break;
  }

  SignalConfigTask();
}

/*****************************************************************************
 * Function - PowerDown
 * DESCRIPTION: Set power down flag, set task priority high and request task time
 *
 *****************************************************************************/
void ConfigControl::PowerDown()
{
#ifndef __PC__
  mPowerDownFlag = true;
  OS_SetPriority(&mTCBConfigTask, POWER_DOWN_PRIO);
  SignalConfigTask();
#else
  Doit();
#endif // __PC__
}

/*****************************************************************************
 * Function - SaveLogSeriesToFlash
 * DESCRIPTION: Trigger for saving log series to flash
 *
 *****************************************************************************/
void ConfigControl::SaveLogSeriesToFlash()
{
  mNextLogBlockToSave = USER_LOG_BLOCK_1;
  // trig the task
  SignalConfigTask();
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION: Initialise member data
 *
 *****************************************************************************/
ConfigControl::ConfigControl()
{
  mOkToSaveFlag = true;
  mExecuteBootFlag = false;
  mBootConfigBlockFlag = false;
  mBootLogBlockFlag = false;
  mBootGscBlockFlag = false;
  mPowerDownFlag = false;
  mTimeToSaveLogDataFlag = true; // save log data at first run
  mTimeToSaveAlarmLogFlag = false;
#ifdef __PC__
  mLoadFromFlashRequested = false;
  mSaveToFlashRequested = false;
#endif

  // NOTE: Any block size below 64K is allowed (power of two is not required).
  mpConfigBlock = new FlashBlock(this, "WWMidrange Config v1", 34000, NON_VOLATILE_CONFIG_SUBJECTS, NON_VOLATILE_CONFIG_SUBJECTS_CNT, FLASH_CONTROL_BLOCK_ID_CONFIG);
  // NOTE: A large log block (> 3000 bytes) should be avoided to ensure saving of log data during power down. (So I think we fucked up some time ago)
 // mpLogBlock = new FlashBlock(this, "WWMidrange Log v1", 4096, NON_VOLATILE_LOG_SUBJECTS, NON_VOLATILE_LOG_SUBJECTS_CNT, FLASH_CONTROL_BLOCK_ID_LOG);
  mpLogBlock = new FlashBlock(this, "WWMidrange Log v1", 4160, NON_VOLATILE_LOG_SUBJECTS, NON_VOLATILE_LOG_SUBJECTS_CNT, FLASH_CONTROL_BLOCK_ID_LOG);
  mpGSCBlock = new FlashBlock(this, "WWMidrange GSC v1", 512, NON_VOLATILE_GSC_SUBJECTS, NON_VOLATILE_GSC_SUBJECTS_CNT, FLASH_CONTROL_BLOCK_ID_GSC);
  mpNoBootBlock = new FlashBlock(this, "WWMidrange No-Boot v1", 512, NON_VOLATILE_NOBOOT_SUBJECTS, NON_VOLATILE_NOBOOT_SUBJECTS_CNT, FLASH_CONTROL_BLOCK_ID_NO_BOOT);
  // Some special handling for user log (not factory generated)
  static const NON_VOLATILE_SUBJECT_TYPE NON_VOLATILE_USER_LOG_1_SUBJECTS[] =
  { {SUBJECT_ID_LOG_SERIES_1_DATA, FLASH_SAVE_ALL},
    {SUBJECT_ID_LOG_SERIES_2_DATA, FLASH_SAVE_ALL},
    {SUBJECT_ID_LOG_SERIES_3_DATA, FLASH_SAVE_ALL},
    {SUBJECT_ID_LOG_SERIES_4_DATA, FLASH_SAVE_ALL} };
  static const NON_VOLATILE_SUBJECT_TYPE NON_VOLATILE_USER_LOG_2_SUBJECTS[] =
  { {SUBJECT_ID_LOG_SERIES_5_DATA, FLASH_SAVE_ALL} };
  mpUserLogBlock1 = new FlashBlock(this, "WWMidrange UserLog 1", 65200, NON_VOLATILE_USER_LOG_1_SUBJECTS, 4, FLASH_CONTROL_BLOCK_ID_USER_LOG_1);
  mpUserLogBlock2 = new FlashBlock(this, "WWMidrange UserLog 2", 16384, NON_VOLATILE_USER_LOG_2_SUBJECTS, 1, FLASH_CONTROL_BLOCK_ID_USER_LOG_2);

  mpTimerObjList[SAVE_LOG_DATA_TIMER] = new SwTimer(60, MINUTE, true, true, this);
  mpTimerObjList[DELAY_SAVE_OF_ALARM_LOG_TIMER] = new SwTimer(1000, MS, false, false, this);

  OS_CREATERSEMA(&mSemaSubjectQueue);

  /* Create objects for handling setting, clearing and delaying of alarm and warnings */
  for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
  {
    mpConfigControlAlarmDelay[fault_id] = new AlarmDelay(this);
    mConfigControlAlarmDelayCheckFlag[fault_id] = false;
  }
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
ConfigControl::~ConfigControl()
{
  /* Delete objects for handling setting, clearing and delaying of alarm and warnings */
  for (int fault_id = FIRST_CC_FAULT_OBJ; fault_id < NO_OF_CC_FAULT_OBJ; fault_id++)
  {
    delete(mpConfigControlAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - ExecuteBoot
 * DESCRIPTION:
 *****************************************************************************/
void ConfigControl::ExecuteBoot()
{
  bool erase_failed = false;

  if (mBootConfigBlockFlag)
  {
    mBootConfigBlockFlag = false;
    erase_failed |= !FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_CONFIG);
  }

  if (mBootLogBlockFlag)
  {
    mBootLogBlockFlag = false;
    erase_failed |= !FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_LOG);
    erase_failed |= !FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_USER_LOG_1);
    erase_failed |= !FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_USER_LOG_2);
  }

  if (mBootGscBlockFlag)
  {
    mBootGscBlockFlag = false;
    erase_failed |= !FlashControl::GetInstance()->EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_GSC);
  }

  if (erase_failed)
  {
    mpConfigControlAlarmDelay[CC_FAULT_OBJ_HW_ERROR]->SetFault();
  }
}

/*****************************************************************************
 * Function - CheckAlarmFlags
 * DESCRIPTION:
 *****************************************************************************/
void ConfigControl::CheckAlarmFlags(void)
{
  if ( mpConfigBlock->GetErrorFlags()
    || mpLogBlock->GetErrorFlags()
    || mpGSCBlock->GetErrorFlags()
    || mpNoBootBlock->GetErrorFlags()
    || mpUserLogBlock1->GetErrorFlags()
    || mpUserLogBlock2->GetErrorFlags())
  {
    mpConfigControlAlarmDelay[CC_FAULT_OBJ_HW_ERROR]->SetFault();
  }
  else
  {
    mpConfigControlAlarmDelay[CC_FAULT_OBJ_HW_ERROR]->ResetFault();
  }
}

/*****************************************************************************
 * Function - CopyLogSeriesToCache
 * DESCRIPTION: Copy log series data to their flash storage caches
 *
 *****************************************************************************/
void ConfigControl::CopyLogSeriesToCache()
{
  #define NO_OF_LOG_STORAGE 5
  static U16VectorDataPoint* *mpLogSeriesData = NULL;

  if (mpLogSeriesData == NULL)
  {
    mpLogSeriesData = new U16VectorDataPoint*[NO_OF_LOG_STORAGE];
    mpLogSeriesData[0] = dynamic_cast<U16VectorDataPoint*>(GetSubject(SUBJECT_ID_LOG_SERIES_1_DATA));
    mpLogSeriesData[1] = dynamic_cast<U16VectorDataPoint*>(GetSubject(SUBJECT_ID_LOG_SERIES_2_DATA));
    mpLogSeriesData[2] = dynamic_cast<U16VectorDataPoint*>(GetSubject(SUBJECT_ID_LOG_SERIES_3_DATA));
    mpLogSeriesData[3] = dynamic_cast<U16VectorDataPoint*>(GetSubject(SUBJECT_ID_LOG_SERIES_4_DATA));
    mpLogSeriesData[4] = dynamic_cast<U16VectorDataPoint*>(GetSubject(SUBJECT_ID_LOG_SERIES_5_DATA));
  }

  // lock all the log series data while updating the cache
  for (int idx=0; idx<NO_OF_LOG_STORAGE; idx++)
  {
    if (mpLogSeriesData[idx] != NULL)
    {
      mpLogSeriesData[idx]->Lock();
    }
  }

  // update caches. Log series data vector 1-4 in user log block 1. Log 5 in block 2.
  mpUserLogBlock1->SaveToCache(GetSubject(SUBJECT_ID_LOG_SERIES_1_DATA));
  mpUserLogBlock1->SaveToCache(GetSubject(SUBJECT_ID_LOG_SERIES_2_DATA));
  mpUserLogBlock1->SaveToCache(GetSubject(SUBJECT_ID_LOG_SERIES_3_DATA));
  mpUserLogBlock1->SaveToCache(GetSubject(SUBJECT_ID_LOG_SERIES_4_DATA));
  mpUserLogBlock2->SaveToCache(GetSubject(SUBJECT_ID_LOG_SERIES_5_DATA));

  // unlock again
  for (int idx=0; idx<NO_OF_LOG_STORAGE; idx++)
  {
    if (mpLogSeriesData[idx] != NULL)
    {
      mpLogSeriesData[idx]->UnLock();
    }
  }
}

/*****************************************************************************
 * Function - Doit
 * DESCRIPTION: Empties event queue, calls save rutines if data is changed.
 * If boot event is received, then boot is executed.
 *****************************************************************************/
void ConfigControl::Doit()
{
  Subject* pSubject;

  OS_Use(&mSemaSubjectQueue);
  pSubject = mSubjectQueue.read();
  while (pSubject != NULL)
  {
    OS_Unuse(&mSemaSubjectQueue);

    // special handling of the alarm log
    if (mpAlarmLog.Equals(pSubject))
    {
      // delay save of alarm log to avoid communication fault from pumps and modules on power down
      mpTimerObjList[DELAY_SAVE_OF_ALARM_LOG_TIMER]->RetriggerSwTimer();
    }
    else
    {
      // save subject to block, try with the most likely block first
      if (!mpLogBlock->SaveToCache(pSubject))
      {
        if (!mpConfigBlock->SaveToCache(pSubject))
        {
          if (!mpNoBootBlock->SaveToCache(pSubject))
          {
            if (!mpGSCBlock->SaveToCache(pSubject))
            {
              // Not found, something is wrong, just a break point
              pSubject = NULL;
            }
          }
        }
      }
    }

    OS_Use(&mSemaSubjectQueue);
    pSubject = mSubjectQueue.read();
  }
  OS_Unuse(&mSemaSubjectQueue);

  // handle delayed save of the alarm log
  if (!mPowerDownFlag && mTimeToSaveAlarmLogFlag)
  {
    mTimeToSaveAlarmLogFlag = false;
    mpLogBlock->SaveToCache(mpAlarmLog.GetSubject());  // we assume that the alarm log subject is stored in the log block
  }

  // save log block on power down or if it is time to save now and data has been modified
  if (mPowerDownFlag || mTimeToSaveLogDataFlag)
  {
    mTimeToSaveLogDataFlag = false;
    if (mpLogBlock->IsModified())
    {
      mpLogBlock->SaveToFlash();
    }
    SaveLogSeriesToFlash(); // save the log series now as well
  }

  // save config block if modified
  if (mpConfigBlock->IsModified())
  {
    mpConfigBlock->SaveToFlash();
    ResetConfigSetUpId();
  }

  // save GSC block if modified
  if (mpGSCBlock->IsModified())
  {
    mpGSCBlock->SaveToFlash();
  }

  // save no-boot block if modified
  if (mpNoBootBlock->IsModified())
  {
    mpNoBootBlock->SaveToFlash();
  }

  // save log series block 1, but only when no subjects are in the queue
  if (mNextLogBlockToSave == USER_LOG_BLOCK_1 && mSubjectQueue.size() == 0)
  {
    CopyLogSeriesToCache(); // update all the user log caches now to keep them in sync
    if (mpUserLogBlock1->IsModified())
    {
      mpUserLogBlock1->SaveToFlash();
    }
    mNextLogBlockToSave = USER_LOG_BLOCK_2;
  }

  // save log series block 2, but only when no subjects are in the queue
  if (mNextLogBlockToSave == USER_LOG_BLOCK_2 && mSubjectQueue.size() == 0)
  {
    if (mpUserLogBlock2->IsModified())
    {
      mpUserLogBlock2->SaveToFlash();
    }
    mNextLogBlockToSave = LOG_BLOCK;
  }
}

/*****************************************************************************
 * Function - ResetConfigSetUpId
 * DESCRIPTION: Resets config setup id code 
 *****************************************************************************/
void ConfigControl::ResetConfigSetUpId()
{
  if (mpResetConfigSetIDCode->GetValue())
  { 
    mpConfigSetupIDCode[0]->SetValue(0);
    mpConfigSetupIDCode[1]->SetValue(0);
    mpConfigSetupIDCode[2]->SetValue(0);
    mpConfigSetupIDCode[3]->SetValue(0);
  }
  else
  {
    mpResetConfigSetIDCode->SetValue(true);
  }
}

#ifdef __PC__
/*****************************************************************************
 * Function - CopyFlashFiles
 * DESCRIPTION:
 *
 *****************************************************************************/
void ConfigControl::CopyFlashFiles(bool save)
{
  // clear error flags
  mpConfigBlock->ClearErrorFlags();
  mpLogBlock->ClearErrorFlags();
  mpGSCBlock->ClearErrorFlags();
  mpNoBootBlock->ClearErrorFlags();
  mpUserLogBlock1->ClearErrorFlags();
  mpUserLogBlock2->ClearErrorFlags();

  // empty the queue
  OS_Use(&mSemaSubjectQueue);
  while (!mSubjectQueue.empty())
  {
    mSubjectQueue.read();
  }
  OS_Unuse(&mSemaSubjectQueue);

  char original_path[MAX_PATH];

  strncpy(original_path, BaseDirectory::GetInstance()->Get(), MAX_PATH-1);
  original_path[MAX_PATH-1] = '\0';

  BaseDirectory::GetInstance()->Set(mFlashPath);

  if (save)
  {
    mpConfigBlock->SaveToFlash();
    mpLogBlock->SaveToFlash();
    mpGSCBlock->SaveToFlash();
    mpNoBootBlock->SaveToFlash();
    mpUserLogBlock1->SaveToFlash();
    mpUserLogBlock2->SaveToFlash();
  }
  else
  {
    LoadFromFlash();
  }

  BaseDirectory::GetInstance()->Set(original_path);

  if (mpConfigBlock->GetErrorFlags()
      || mpLogBlock->GetErrorFlags()
      || mpGSCBlock->GetErrorFlags()
      || mpNoBootBlock->GetErrorFlags()
      || mpUserLogBlock1->GetErrorFlags()
      || mpUserLogBlock2->GetErrorFlags())
  {
    PcMessageService::GetInstance()->SendMsg( save ? "Failed to save files" : "Failed to load files");
  }
  else
  {
    PcMessageService::GetInstance()->SendMsg( save ? "Files successfully saved" : "Files successfully loaded");
  }
}

#endif

/*****************************************************************************
 * Function - GetSubject
 * DESCRIPTION:
 *****************************************************************************/
Subject* ConfigControl::GetSubject(U16 subjectId)
{
  SUBJECT_MAP_ITR itr = mSubjects.find(subjectId);
  if (itr != mSubjects.end())
    return itr->second;
  else
    return NULL; // Unknown subject Id!
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
