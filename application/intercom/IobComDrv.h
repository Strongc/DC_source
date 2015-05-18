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
/* CLASS NAME       : IobComDrv                                             */
/*                                                                          */
/* FILE NAME        : IobComDrv.h                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __IOB_COMDRV_H__
#define __IOB_COMDRV_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AppTypeDefs.h>
#include <Observer.h>
#include <SubTask.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <BoolDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define INTER_COM_TASK_STACK_SIZE    2000
#define TASK_EVENT_IOB_DATA_RXED    (1)
#define TASK_EVENT_IOB_DATA_TXED    (2)

#define RXTGM_LEN  9
#define TXTGM_LEN  6

#define RXTGM_BUFFER 50 //No of rx tgms that can be buffered
#define TXTGM_BUFFER 50 //No of tx tgms that can be buffered


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
 // analog input numbers
typedef enum
{
  FIRST_IOB_ANA_IN_NO = 0,
  IOB_ANA_IN_NO_0 = FIRST_IOB_ANA_IN_NO,
  IOB_ANA_IN_NO_1,
  IOB_ANA_IN_NO_2,
  IOB_ANA_IN_NO_3,
  IOB_ANA_IN_NO_4,
  IOB_ANA_IN_NO_5,
  IOB_ANA_IN_NO_6,
  IOB_ANA_IN_NO_7,

  /* Insert new types above */
  NO_OF_IOB_ANA_IN_NO,
  LAST_IOB_ANA_IN_NO = NO_OF_IOB_ANA_IN_NO - 1
} IOB_ANA_IN_NO_TYPE;

typedef enum
{
  FIRST_IOB_DIG_OUT_NO = 0,
  IOB_DIG_OUT_NO_0 = FIRST_IOB_DIG_OUT_NO,
  IOB_DIG_OUT_NO_1,
  IOB_DIG_OUT_NO_2,
  IOB_DIG_OUT_NO_3,
  IOB_DIG_OUT_NO_4,

  /* Insert new types above */
  NO_OF_IOB_DIG_OUT_NO,
  LAST_IOB_DIG_OUT_NO = NO_OF_IOB_DIG_OUT_NO - 1
} IOB_DIG_OUT_NO_TYPE;

typedef enum
{
  IOB_DIG_IN_NO_0 = 0,
  IOB_DIG_IN_NO_1,
  IOB_DIG_IN_NO_2,
  IOB_DIG_IN_NO_3,
  IOB_DIG_IN_NO_4,

  /* Insert new types above */
  NO_OF_IOB_DIG_IN,
  LAST_IOB_DIG_IN = NO_OF_IOB_DIG_IN - 1
} IOB_DIG_IN_NO_TYPE;

typedef  enum
{
  IOB_TGM_STATE_SD_EXPECTED,
  IOB_TGM_STATE_BODY_EXPECTED
} IOB_TGM_STATE_TYPE;

typedef enum
{
  IOB_VERSION_NUMBER_HI_BYTE,
  IOB_VERSION_NUMBER_MID_BYTE,
  IOB_VERSION_NUMBER_LO_BYTE,
  NO_MORE_IOB_VERSION_BYTES
} IOB_VERSION_NUMBER_TYPE;

typedef enum
{
  IOB_DEBUG_REQUEST_VERSION_NUMBER,
  IOB_DEBUG_REQUEST_DIGITAL_INPUT,
  IOB_DEBUG_REQUEST_AI_PRESSURE,

  /* Insert new types above */
  NO_OF_IOB_DEBUG_REQUEST,
  LAST_IOB_DEBUG_REQUEST = NO_OF_IOB_DEBUG_REQUEST - 1
} IOB_DEBUG_REQUEST_TYPE;

typedef enum
{
  IOB_AI_PRESSURE_HI_BYTE,
  IOB_AI_PRESSURE_LO_BYTE,
  NO_MORE_IOB_AI_PRESSURE_BYTES
} IOB_AI_PRESSURE_TYPE;

/*****************************************************************************
 * CLASS: IobComDrv
 * DESCRIPTION:
 *
 *****************************************************************************/
class IobComDrv : public Observer
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static IobComDrv* GetInstance();
    void InitInterComTask();

    void Run();

    U8 StoreIobRxTgm(U8* rxTgm);
    U8 RetrieveIobTxTgm(U8* txTgm);
    void SignalIobDataRxed();
    void SignalIobDataTxed();

    U8 GetIobTxTgmCnt();

    U32 GetAnalogInput(IOB_ANA_IN_NO_TYPE no);
    void SetDigitalOutput(IOB_DIG_OUT_NO_TYPE no);
    void ClearDigitalOutput(IOB_DIG_OUT_NO_TYPE no);
    void ToggleDigitalOutput(IOB_DIG_OUT_NO_TYPE no);
    void GetAdd8Counters(U32* pOKCnt, U32* pErrorCnt);
    bool TestDigitalInput(IOB_DIG_IN_NO_TYPE no);
    void GetDigitalInputs(U8* pDigInBuf);
    void SetDigitalOutputTestMode(U8 dig_out);

    // Test and simulation
    void SetTestModeFlag();
    void ClearTestModeFlag();
    void SetInputSimulationFlag();
    void ClearInputSimulationFlag();
    void SetInputSimulationMode(U16 mode);
    void SetDigitalInputSimulationValue(U16 value);
    void SetAnalogInputSimulationValue(U16 value, IOB_ANA_IN_NO_TYPE no);
    U8  GetIOSimulationStatus();

    //GENI interface.
    void StartBusTransmit();
    bool StoreTxByte(U8 txByte);
    void ResetBus();

    // Observer operations
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int Id, Subject* pSubject);


  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    IobComDrv();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~IobComDrv();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InterpreteRxTgm();
    void BuildTxTgm();

    /********************************************************************
    TASK ATTRIBUTES
    ********************************************************************/
    OS_TASK mTCBInterComTask;
    OS_STACKPTR int mStackInterComTask[INTER_COM_TASK_STACK_SIZE];

    bool mTaskRunPhase;

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    static IobComDrv* mInstance;
    U8 mRxTgm[RXTGM_LEN];
    U8 mTxTgm[TXTGM_LEN];
    IOB_TGM_STATE_TYPE mRxTgmState;
    int mRxTgmIndx;
    int mNoOfTxTgmsToSend;
    int mAdd8ErrorCnt;
    int mAdd8OkCnt;

    U32 mAnaInBuf[NO_OF_IOB_ANA_IN_NO];
    U8 mDigOutBuf;
    U8 mDigInBuf;
    U8 mDigOutBufTestMode;
    bool mBusBusyFlag;
    bool mBus1TxFlag;
    bool mBus2TxFlag;
    U8 mBus1TxByte;
    U8 mBus2TxByte;
    bool mBusTransmit;
    bool mBusReset;
    bool mTestModeActiveFlag;
    bool mInputSimulationFlag;
    U16  mDigitalInputSimulationMode;
    U16  mDigitalInputSimulationBuffer;
    U16  mAnalogInputSimulationMode;
    U16  mAnalogInputSimulationBuffer[NO_OF_IOB_ANA_IN_NO];

    bool mIOBActiveFlag;
    bool mIOBVersionRequestFlag;
    bool mIOBDigitalInputRequestFlag;
    bool mIOBAnalogInputPressureRequestFlag;
    int mWaitForDebugReply;
    IOB_VERSION_NUMBER_TYPE mRequestedIOBVersionByte;

    bool mDebugAddrHiByteReady;
    bool mDebugAddrLoByteReady;
    U8 mDebugAddrHiByte;
    U8 mDebugAddrLoByte;

    U32 IOBVersionNumber;
    SubjectPtr<U32DataPoint*> mIobLviCounter;
    SubjectPtr<U32DataPoint*> mIobWatchdogCounter;
    SubjectPtr<U32DataPoint*> mIobAdd8ErrCounter;

    IOB_DEBUG_REQUEST_TYPE mCurrentDebugRequest;
    IOB_AI_PRESSURE_TYPE mRequestedIOBAnalogInputPressureByte;
    U16 mAnalogInputPressure;

    SubjectPtr<BoolDataPoint*> mpIobBusModulePressent;
    SubjectPtr<BoolDataPoint*> mpIobSupplyStatus;
    SubjectPtr<BoolDataPoint*> mpIobBatteryStatus;
    SubjectPtr<U16DataPoint*> mpIobAiPressureRaw;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
