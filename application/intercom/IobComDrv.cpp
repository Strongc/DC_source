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
/* FILE NAME        : IobComDrv.cpp                                         */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Implementation of the cpu351 IOBoard            */
/*                          communication interface                         */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <IobComDrv.h>
#include <IobHwIf.h>
#include <microp.h>
#include <binary.h>
#include <mailbox.h>
#include <softwareversion.h>

extern "C"                              // TEST JUA: Scope extern "C" can be removed
{                                       // when new GENIpro version is available.
#include <rs232_drv_if.h>
}

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define IOB_VERSION_HI_BYTE_ADDR      0x1FE2
#define IOB_VERSION_MID_BYTE_ADDR     0x1FE1
#define IOB_VERSION_LO_BYTE_ADDR      0x1FE0

#define IOB_DIGITAL_INPUT_ADDR        0xFEDE

#define IOB_AI_PRESSURE_HI_BYTE_ADDR  0xFEDD
#define IOB_AI_PRESSURE_LO_BYTE_ADDR  0xFEDC

#define DEBUG_REPLY_TIMEOUT  20

/*****************************************************************************
  MACROS
 *****************************************************************************/
#define COPY_BIT(src,dest,bit_number)   if (TEST_BIT_LOW(src,bit_number))  \
                                          SET_BIT_LOW(dest,bit_number);    \
                                        else                               \
                                          SET_BIT_HIGH(dest,bit_number);   \

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  IOCTRL_STATUS_RXPOS = 0,
  BUS_RX1_7_1_RXPOS,
  BUS_RX2_7_1_RXPOS,
  BUS_RX1_0_RXPOS,
  BUS_RX2_0_RXPOS = BUS_RX1_0_RXPOS,
  DIG_IN_RXPOS = BUS_RX2_0_RXPOS,
  AD1_9_3_RXPOS,
  AD2_9_3_RXPOS,
  AD_2_0_RXPOS,
  DEBUG_BYTE0_RXPOS = AD_2_0_RXPOS,
  DEBUG_BYTE7_1_RXPOS,
  ADD8_RXPOS
} RXTGM_ELEMENT_POS;

typedef enum
{
  IOCTRL_STATUS_TXPOS = 0,
  BUS_TX1_7_1_TXPOS,
  BUS_TX2_7_1_TXPOS,
  BUS_TX1_0_TXPOS,
  BUS_TX2_0_TXPOS = BUS_TX1_0_TXPOS,
  DIG_OUT_TXPOS = BUS_TX2_0_TXPOS,
  DEBUG_ADDR7_1_TXPOS,
  ADD8_TXPOS
} TXTGM_ELEMENT_POS;

typedef enum
{
  COM_ERR_RX            = 0x01,
  AD_CHANNEL            = 0x06,
  DATA_DEBUG            = 0x08,
  BUS_BUSY              = 0x10,
  BUS2_INPUT_DATA_VALID = 0x20,
  BUS1_INPUT_DATA_VALID = 0x40
} RXTGM_BIT_POS;

typedef enum
{
  COM_ERR_TX             = 0x01,
  DEBUG_ADDR0            = 0x02,
  LO_BYTE                = 0x04,
  HI_BYTE                = 0x08,
  RESET_BUS              = 0x10,
  BUS2_OUTPUT_DATA_VALID = 0x20,
  BUS1_OUTPUT_DATA_VALID = 0x40
} TXTGM_BIT_POS;

typedef enum
{
  ADD8_ERR_DEBUG_BYTE_BIT_POS     = 0x01,
  IDLE_TIMEOUT_DEBUG_BYTE_BIT_POS = 0x02,
  WD_CNT_DEBUG_BYTE_BIT_POS       = 0x04,
  LVI_CNT_DEBUG_BYTE_BIT_POS      = 0x08
} DEBUG_BYTE_BIT_POS;

/*****************************************************************************
  CREATES AN OBJECT.
 ******************************************************************************/
IobComDrv* IobComDrv::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
 /*****************************************************************************
  * Function - InterComTask
  * DESCRIPTION: embOS Taskfunction
  *
  *****************************************************************************/
extern "C" void InterComTask(void)
{
  IobComDrv::GetInstance()->Run();
}


 /*****************************************************************************
  * Function - GetInstance
  * DESCRIPTION: Returns pointer to singleton object of IobComDrv.
  *
  *****************************************************************************/
IobComDrv* IobComDrv::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new IobComDrv();
  }
  return mInstance;
}


void IobComDrv::InitInterComTask(void)
{
  // Initiate debug variables here so that they are not depending upon if the
  // software is compiled for PC or not.
  mIOBActiveFlag = false;
  mIOBVersionRequestFlag = true;
  mIOBDigitalInputRequestFlag = false;
  mIOBAnalogInputPressureRequestFlag = false;
  mWaitForDebugReply = 0;
  mRequestedIOBVersionByte = IOB_VERSION_NUMBER_HI_BYTE;
  mRequestedIOBAnalogInputPressureByte = IOB_AI_PRESSURE_HI_BYTE;
  mDebugAddrHiByteReady = false;
  mDebugAddrLoByteReady = false;
  mDebugAddrHiByte = 0;
  mDebugAddrLoByte = 0;
  IOBVersionNumber = 0;
  mAnalogInputPressure = 0;

#ifndef __PC__
  OS_CREATETASK(&mTCBInterComTask, "InterCom Task", InterComTask, 200, mStackInterComTask);
#endif
}


/*****************************************************************************
 * Function - Run
 * DESCRIPTION: Main function of IobComDrv
 *
 *****************************************************************************/
void IobComDrv::Run(void)
{
#ifndef __PC__
  OS_U8 inter_com_event;
  char rx_byte;
  int mb_empty, mb_full, i, j;


  mNoOfTxTgmsToSend = GetTxBufferSize() / TXTGM_LEN;
  /* Fill IOB tx mailbox with mNoOfTxTgmsToSend elements */
  i = 0;
  do
  {
    /* Build next IOB tx tgm */
    BuildTxTgm();
    j = 0;
    do
    {
      /* Put in mailbox bytewise */
      mb_full = OS_PutMailCond1(&MBIobTransmit, (char *)&mTxTgm[j]);
      j++;
    } while (!mb_full && j<TXTGM_LEN);
    i++;
  } while (!mb_full && i<mNoOfTxTgmsToSend);

  mTaskRunPhase = true;

  InitInterComChannel();
  //StartInterComTransmit();

  while (1)
  {
    inter_com_event = OS_WaitEvent(TASK_EVENT_IOB_DATA_RXED | TASK_EVENT_IOB_DATA_TXED);

    /* Handle transmission of IOB telegrams */
    if ( inter_com_event & TASK_EVENT_IOB_DATA_TXED )
    {
      i = 0;
      do
      {
        /* Build next IOB tx tgm */
        BuildTxTgm();
        j = 0;
        do
        {
          /* Put in mailbox bytewise */
          mb_full = OS_PutMailCond1(&MBIobTransmit, (char *)&mTxTgm[j]);
          j++;
        } while (!mb_full && j<TXTGM_LEN);
        i++;
      } while (!mb_full && i<mNoOfTxTgmsToSend);
    }

    /* Handle reception of IOB telegrams */
    if ( inter_com_event & TASK_EVENT_IOB_DATA_RXED )
    {
      /* Get first byte from mailbox */
      mb_empty = OS_GetMailCond1(&MBIobReceive, &rx_byte);
      while (!mb_empty)
      {
        switch (mRxTgmState)
        {
          case IOB_TGM_STATE_SD_EXPECTED :
            /* Check for start delimiter bit (SD). No bytes is stored before
               SD is found */
            if (TEST_BIT_HIGH(rx_byte, 7))
            {
              mRxTgm[0] = rx_byte;
              mRxTgmIndx = 1;
              mRxTgmState = IOB_TGM_STATE_BODY_EXPECTED;
            }
            break;
          case IOB_TGM_STATE_BODY_EXPECTED :
            if (TEST_BIT_HIGH(rx_byte, 7))
            {
              /* We are in the middle of a tgm where no SD should occur. If SD found
                 anyway mTgmIndx must be reset */
              mRxTgmIndx = 0;
            }
            mRxTgm[mRxTgmIndx] = rx_byte;
            if (mRxTgmIndx >= RXTGM_LEN-1)
            {
              /* mRxTgm now holds a complete IOB tgm. Process tgm and search for next SD */
              InterpreteRxTgm();
              mRxTgmState = IOB_TGM_STATE_SD_EXPECTED;
            }
            else
            {
              mRxTgmIndx++;
            }
            break;
          default:
            /* no action */
            break;
        }
        /* Get next byte from mailbox */
        mb_empty = OS_GetMailCond1(&MBIobReceive, &rx_byte);
      }
    }
  }
#endif // __PC__
}

/*****************************************************************************
 * Function - StoreIobRxTgm
 * DESCRIPTION: Puts one rx tgm in IOB receive mailbox
 *
 *****************************************************************************/
U8 IobComDrv::StoreIobRxTgm(U8* rxTgm)
{
  U8 ret_val = 1;

  if ( mTaskRunPhase == true )
  {
//    ret_val = OS_PutMailCond(&mMBIobReceive, rxTgm);
  }
  return ret_val;
}

/*****************************************************************************
 * Function - SignalIobDataRxed
 * DESCRIPTION: Signal InterComTask that rx data is ready in mailbox
 *
 *****************************************************************************/
void IobComDrv::SignalIobDataRxed(void)
{
  if ( mTaskRunPhase == true )
  {
    OS_SignalEvent(TASK_EVENT_IOB_DATA_RXED, &mTCBInterComTask);
  }
}
/*****************************************************************************
 * Function - RetrieveIobTxTgm
 * DESCRIPTION: Gets one tx tgm from IOB transmit mailbox
 *
 *****************************************************************************/
U8 IobComDrv::RetrieveIobTxTgm(U8* txTgm)
{
  U8 ret_val = 1;

  if ( mTaskRunPhase == true )
  {
//    ret_val = OS_GetMailCond(&mMBIobTransmit, txTgm);
  }
  return ret_val;
}

/*****************************************************************************
 * Function - SignalIobDataTxed
 * DESCRIPTION: Signal InterComTask that serial channel is ready to transmit data
 *
 *****************************************************************************/
void IobComDrv::SignalIobDataTxed(void)
{
  if ( mTaskRunPhase == true )
  {
    OS_SignalEvent(TASK_EVENT_IOB_DATA_TXED, &mTCBInterComTask);
  }
}
/*****************************************************************************
 * Function - GetIobTxTgmCnt
 * DESCRIPTION:
 *
 *****************************************************************************/
U8 IobComDrv::GetIobTxTgmCnt(void)
{
  U8 ret_val = 0;

  if ( mTaskRunPhase == true )
  {
//    ret_val = OS_GetMessageCnt(&mMBIobTransmit);
  }
  return ret_val;
}


/*****************************************************************************
 * Function - GetAnalogInput
 * DESCRIPTION: Return value of analog input channel specified by argument
 *
 *****************************************************************************/
U32 IobComDrv::GetAnalogInput(IOB_ANA_IN_NO_TYPE no)
{
  if ( mInputSimulationFlag == true && TEST_BIT_HIGH(mAnalogInputSimulationMode,no) )
  {
    return mAnalogInputSimulationBuffer[no];
  }
  else
  {
    return mAnaInBuf[no];
  }
}

U8 IobComDrv::GetIOSimulationStatus(void)
/*****************************************************************************
 * Function - GetIOSimulationStatus
 * DESCRIPTION: Return the status of digital outputs packet in mDigOutBuf
 *              + status of master simulation flag
 *****************************************************************************/
{
  return mDigOutBuf + 0x80*mInputSimulationFlag;
}

/*****************************************************************************
 * Function - SetDigitalOutput
 * DESCRIPTION: Set digital output specified by argument
 *
 *****************************************************************************/
void IobComDrv::SetDigitalOutput(IOB_DIG_OUT_NO_TYPE no)
{
  SET_BIT_HIGH(mDigOutBuf, no);
}

/*****************************************************************************
 * Function - ClearDigitalOutput
 * DESCRIPTION: Clear digital output specified by argument
 *
 *****************************************************************************/
void IobComDrv::ClearDigitalOutput(IOB_DIG_OUT_NO_TYPE no)
{
  SET_BIT_LOW(mDigOutBuf, no);
}

/*****************************************************************************
 * Function - ToggleDigitalOutput
 * DESCRIPTION: Toggle digital output specified by argument
 *
 *****************************************************************************/
void IobComDrv::ToggleDigitalOutput(IOB_DIG_OUT_NO_TYPE no)
{
  TOGGLE_BIT(mDigOutBuf, no);
}

/*****************************************************************************
 * Function - GetAdd8Counters
 * DESCRIPTION: Deliver IOB com add8 counters via pointer argument
 *
 *****************************************************************************/
void IobComDrv::GetAdd8Counters(U32* pOkCnt, U32 * pErrorCnt)
{
  *pOkCnt    = mAdd8OkCnt;
  *pErrorCnt = mAdd8ErrorCnt;
}

/*****************************************************************************
 * Function - StartBusTransmit
 * DESCRIPTION: Start transmission of bus bytes
 *
 *****************************************************************************/
void IobComDrv::StartBusTransmit()
{
  mBusTransmit = true;
  mBusReset = false;
}

/*****************************************************************************
 * Function - StoreTxByte
 * DESCRIPTION: Store bus byte delivered via argument in buffer. If more bytes
 *              can be stored true is returned.
 *****************************************************************************/
bool IobComDrv::StoreTxByte(U8 txByte)
{
  bool request_more_bytes = false;

  if ( mBus1TxFlag == false )
  {
    mBus1TxByte = txByte;
    mBus1TxFlag = true;
    request_more_bytes = true;
  }
  else if ( mBus2TxFlag == false )
  {
    mBus2TxByte = txByte;
    mBus2TxFlag = true;
  }
  return request_more_bytes;

}

/*****************************************************************************
 * Function - ResetBus
 * DESCRIPTION: Reset transmission of bus bytes
 *
 *****************************************************************************/
void IobComDrv::ResetBus()
{
  mBusReset = true;
}

/*****************************************************************************
 * Function - TestDigitalInput
 * DESCRIPTION: Returns state of digital input specified by argument
 *              true: High
 *              false: Low
 *****************************************************************************/
bool IobComDrv::TestDigitalInput(IOB_DIG_IN_NO_TYPE no)
{
  if (mInputSimulationFlag == true && TEST_BIT_HIGH(mDigitalInputSimulationMode,no))
  {
    return (bool)(TEST_BIT_HIGH(mDigitalInputSimulationBuffer,no));
  }
  else
  {
    return (bool)(TEST_BIT_HIGH(mDigInBuf,no));
  }
}

/*****************************************************************************
 * Function - GetDigitalInputs
 * DESCRIPTION: Deliver digital inputs via pointer argument
 *
 *****************************************************************************/
void IobComDrv::GetDigitalInputs(U8* pDigInBuf)
{
  if (mInputSimulationFlag == true)
  {
    auto U8 temp_dig_in_buf = mDigInBuf;
    if (TEST_BIT_HIGH(mDigitalInputSimulationMode,0))
      COPY_BIT(mDigitalInputSimulationBuffer,temp_dig_in_buf,0);
    if (TEST_BIT_HIGH(mDigitalInputSimulationMode,1))
      COPY_BIT(mDigitalInputSimulationBuffer,temp_dig_in_buf,1);
    if (TEST_BIT_HIGH(mDigitalInputSimulationMode,2))
      COPY_BIT(mDigitalInputSimulationBuffer,temp_dig_in_buf,2);
    *pDigInBuf = temp_dig_in_buf;
  }
  else
  {
    *pDigInBuf = mDigInBuf;
  }
}

/*****************************************************************************
 * Function - SetDigitalOutputTestMode
 * DESCRIPTION: Set digital output used in test mode
 *
 *****************************************************************************/
void IobComDrv::SetDigitalOutputTestMode(U8 dig_out)
{
  mDigOutBufTestMode = dig_out;
}

/*****************************************************************************
 * Function - SetTestModeFlag
 * DESCRIPTION: Set digital output specified by argument
 *
 *****************************************************************************/
void IobComDrv::SetTestModeFlag()
{
  mTestModeActiveFlag = true;
}

/*****************************************************************************
 * Function - ClearTestModeFlag
 * DESCRIPTION: Set digital output specified by argument
 *
 *****************************************************************************/
void IobComDrv::ClearTestModeFlag()
{
  mTestModeActiveFlag = false;
}

/*****************************************************************************
 * Function - SetInputSimulationFlag
 * DESCRIPTION: Set the master simulation flag
 *
 *****************************************************************************/
void IobComDrv::SetInputSimulationFlag()
{
  mInputSimulationFlag = true;
}

/*****************************************************************************
 * Function - ClearInputSimulationFlag
 * DESCRIPTION: Clear the master simulation flag
 *
 *****************************************************************************/
void IobComDrv::ClearInputSimulationFlag()
{
  mInputSimulationFlag = false;
}

/*****************************************************************************
 * Function - SetInputSimulationMode
 * DESCRIPTION: As named
 *
 *****************************************************************************/
void IobComDrv::SetInputSimulationMode(U16 mode)
{
  mDigitalInputSimulationMode = ((mode>>8) & 0x07);
  mAnalogInputSimulationMode = (mode & 0x3F);
}

/*****************************************************************************
 * Function - SetDigitalInputSimulationValue
 * DESCRIPTION: As named
 *
 *****************************************************************************/
void IobComDrv::SetDigitalInputSimulationValue(U16 value)
{
  mDigitalInputSimulationBuffer = value;
}

/*****************************************************************************
 * Function - SetAnalogInputSimulationValue
 * DESCRIPTION: As named
 *
 *****************************************************************************/
void IobComDrv::SetAnalogInputSimulationValue(U16 value, IOB_ANA_IN_NO_TYPE no)
{
  if (no<NO_OF_IOB_ANA_IN_NO)
  {
    mAnalogInputSimulationBuffer[no] = value;
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void IobComDrv::Update(Subject* pSubject)
{
  // IobComDrv does not do anything on update - it only sets DataPoints
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobComDrv::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobComDrv::ConnectToSubjects(void)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobComDrv::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_IOBC_LVI_COUNT:
      mIobLviCounter.Attach(pSubject);
      break;
    case SP_IOBC_WD_COUNT:
      mIobWatchdogCounter.Attach(pSubject);
      break;
    case SP_IOBC_ADD8ERR_COUNT:
      mIobAdd8ErrCounter.Attach(pSubject);
      break;
    case SP_IOBC_BUS_MODULE_PRESSENT:
      mpIobBusModulePressent.Attach(pSubject);
      break;
    case SP_IOBC_SUPPLY_STATUS:
      mpIobSupplyStatus.Attach(pSubject);
      break;
    case SP_IOBC_BATTERY_STATUS:
      mpIobBatteryStatus.Attach(pSubject);
      break;
    case SP_IOBC_AI_PRESSURE_RAW:
      mpIobAiPressureRaw.Attach(pSubject);
      break;
  }
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
 * DESCRIPTION:
 *
 *****************************************************************************/
IobComDrv::IobComDrv()
{
  U8 i;

  mBusTransmit = false;
  mBusReset = false;
  mBus1TxFlag = false;
  mBus2TxFlag = false;
  mBusBusyFlag = false;
  mDigOutBuf = 0;
  mAdd8ErrorCnt = 0;
  mAdd8OkCnt = 0;
  mTaskRunPhase = false;
  mRxTgmState = IOB_TGM_STATE_SD_EXPECTED;
  mTestModeActiveFlag = false;
  mInputSimulationFlag = false;
  mDigitalInputSimulationMode = 0;
  mDigitalInputSimulationBuffer = 0;
  mAnalogInputSimulationMode = 0;
  mCurrentDebugRequest = IOB_DEBUG_REQUEST_VERSION_NUMBER;

  for (i = 0; i< NO_OF_IOB_ANA_IN_NO; i++)
  {
    mAnalogInputSimulationBuffer[i]=0;
  }

  for (i = 0; i < TXTGM_LEN; i++)
  {
    mTxTgm[i]=0;
  }

}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
IobComDrv::~IobComDrv()
{
  mIobLviCounter.Unsubscribe(this);
  mIobWatchdogCounter.Unsubscribe(this);
  mIobAdd8ErrCounter.Unsubscribe(this);
}

/*****************************************************************************
 * Function - InterpreteRxTgm
 * DESCRIPTION: Interprete IOB rx tgm, placed in mRxTgm
 *
 *****************************************************************************/
void IobComDrv::InterpreteRxTgm()
{
  int i, add8_sum;
  U8 bus_rx_byte, ana_in_index;
  U8 debug_byte;

  /* 8-bits add checksum with bit7 clear */
  add8_sum = 0;
  for (i=0; i<(RXTGM_LEN-1); i++)
  {
    add8_sum += mRxTgm[i];
  }
  add8_sum &= b01111111;

  /* Process telegram if check sum OK */
  if ( mRxTgm[ADD8_RXPOS] == add8_sum )
  {
    mIOBActiveFlag = true;
    mAdd8OkCnt++;

    /* Send the received GENI data to the GENI interface (Only if there is valid data in the telegram) */
    if (mRxTgm[IOCTRL_STATUS_RXPOS] & BUS1_INPUT_DATA_VALID)
    {
      bus_rx_byte = (mRxTgm[BUS_RX1_7_1_RXPOS]<<1) | (mRxTgm[BUS_RX1_0_RXPOS]&b00000001);
      SaveRxByte(RS232_CH_INDX, bus_rx_byte);
      if (mRxTgm[IOCTRL_STATUS_RXPOS] & BUS2_INPUT_DATA_VALID)
      {
        bus_rx_byte = (mRxTgm[BUS_RX2_7_1_RXPOS]<<1) | ((mRxTgm[BUS_RX2_0_RXPOS]&b00000010)>>1);
        SaveRxByte(RS232_CH_INDX, bus_rx_byte);
      }
    }

    /* Signal busy or idle to the GENI interface in case of state change */
    if ( mRxTgm[IOCTRL_STATUS_RXPOS] & BUS_BUSY )
    {
      if ( mBusBusyFlag == false )
      {
        mBusBusyFlag = true;
        GeniChannelBusy(RS232_CH_INDX, true);
      }
    }
    else
    {
      if ( mBusBusyFlag == true )
      {
        mBusBusyFlag = false;
        GeniChannelBusy(RS232_CH_INDX, false);
      }
    }

    /* Move analog input to mAnaInBuf */
    ana_in_index = (mRxTgm[IOCTRL_STATUS_RXPOS] & AD_CHANNEL) >> 1;
    mAnaInBuf[ana_in_index*2] = ((U32)mRxTgm[AD1_9_3_RXPOS] << 3) |
                               (((U32)mRxTgm[AD_2_0_RXPOS] & 0x000E) >> 1);
    mAnaInBuf[(ana_in_index*2)+1] = ((U32)mRxTgm[AD2_9_3_RXPOS] << 3) |
                               (((U32)mRxTgm[AD_2_0_RXPOS] & 0x0070) >> 4);

    /* Move digital input to mDigInBuf */
    mDigInBuf = mRxTgm[DIG_IN_RXPOS] >> 2;

    /* Extract debug data - if any */
    debug_byte = ( (mRxTgm[DEBUG_BYTE7_1_RXPOS]<<1) | (mRxTgm[DEBUG_BYTE0_RXPOS]&b00000001) );
    if ( (mRxTgm[IOCTRL_STATUS_RXPOS] & DATA_DEBUG) == DATA_DEBUG )
    {
      if ( mWaitForDebugReply != 0 )
      {
        mWaitForDebugReply = 0;
        switch (mCurrentDebugRequest)
        {
          case IOB_DEBUG_REQUEST_VERSION_NUMBER:
            IOBVersionNumber =  (IOBVersionNumber<<8) + debug_byte;
            switch ( mRequestedIOBVersionByte )
            {
              case IOB_VERSION_NUMBER_HI_BYTE :
                mRequestedIOBVersionByte = IOB_VERSION_NUMBER_MID_BYTE;
                mIOBVersionRequestFlag = true;
                break;
              case IOB_VERSION_NUMBER_MID_BYTE :
                mRequestedIOBVersionByte = IOB_VERSION_NUMBER_LO_BYTE;
                mIOBVersionRequestFlag = true;
                break;
              case IOB_VERSION_NUMBER_LO_BYTE :
                /* Don't ask for any further version number bytes */
                /* Update the version number data point */
                mIOBVersionRequestFlag = false;
                IoSoftwareVersion::GetInstance()->SetValue(IOBVersionNumber);
                /* Start to request other debug data */
                mIOBDigitalInputRequestFlag = true;
                mCurrentDebugRequest = IOB_DEBUG_REQUEST_DIGITAL_INPUT;
                break;
              default:
                break;
            }
            break;

          case IOB_DEBUG_REQUEST_DIGITAL_INPUT:
            mpIobBatteryStatus->SetValue(TEST_BIT_HIGH(debug_byte, 5));     // bit 5
            mpIobSupplyStatus->SetValue(TEST_BIT_HIGH(debug_byte, 6));      // bit 6
            mpIobBusModulePressent->SetValue(TEST_BIT_HIGH(debug_byte, 7)); // bit 7

            mRequestedIOBAnalogInputPressureByte = IOB_AI_PRESSURE_HI_BYTE;
            mIOBAnalogInputPressureRequestFlag = true;
            mCurrentDebugRequest = IOB_DEBUG_REQUEST_AI_PRESSURE;
            break;

          case IOB_DEBUG_REQUEST_AI_PRESSURE:
            mAnalogInputPressure =  (mAnalogInputPressure<<8) + debug_byte;
            switch ( mRequestedIOBAnalogInputPressureByte )
            {
              case IOB_AI_PRESSURE_HI_BYTE:
                mRequestedIOBAnalogInputPressureByte = IOB_AI_PRESSURE_LO_BYTE;
                mIOBAnalogInputPressureRequestFlag = true;
                mCurrentDebugRequest = IOB_DEBUG_REQUEST_AI_PRESSURE;
                break;
              case IOB_AI_PRESSURE_LO_BYTE:
                mAnalogInputPressure = mAnalogInputPressure>>6; // Place the same way as the other AD values
                mpIobAiPressureRaw->SetValue(mAnalogInputPressure);
                mIOBDigitalInputRequestFlag = true;
                mCurrentDebugRequest = IOB_DEBUG_REQUEST_DIGITAL_INPUT;
                break;
              default:
                break;
            }
            break;

          default:
            break;
        }
      }
    }
    else
    {
      /* Check if we are waiting for requested debug byte from IOB */
      if ( mWaitForDebugReply != 0)
      {
        mWaitForDebugReply--;
        if ( mWaitForDebugReply == 0 )
        {
          static int reply_time_out_cnt = 0;
          reply_time_out_cnt++;
          /* Requested data has not arrived - try again */
          switch (mCurrentDebugRequest)
          {
            case IOB_DEBUG_REQUEST_VERSION_NUMBER:
              mIOBVersionRequestFlag = true;
              break;
            case IOB_DEBUG_REQUEST_DIGITAL_INPUT:
              mIOBDigitalInputRequestFlag = true;
              break;
            case IOB_DEBUG_REQUEST_AI_PRESSURE:
              mIOBAnalogInputPressureRequestFlag = true;
              break;
            default:
              mIOBVersionRequestFlag = true;
              break;
          }
        }
      }

      /* Debug flags is handled here */
      if ( (debug_byte & ADD8_ERR_DEBUG_BYTE_BIT_POS) == ADD8_ERR_DEBUG_BYTE_BIT_POS)
      {
        /* Increment Add8err datapoint */
        i = mIobAdd8ErrCounter->GetValue();
        i++;
        mIobAdd8ErrCounter->SetValue(i);
      }
      if ( (debug_byte & WD_CNT_DEBUG_BYTE_BIT_POS) == WD_CNT_DEBUG_BYTE_BIT_POS)
      {
        /* Increment WDcount datapoint */
        i = mIobWatchdogCounter->GetValue();
        i++;
        mIobWatchdogCounter->SetValue(i);
      }
      if ( (debug_byte & LVI_CNT_DEBUG_BYTE_BIT_POS) == LVI_CNT_DEBUG_BYTE_BIT_POS)
      {
        /* Increment LVI datapoint */
        i = mIobLviCounter->GetValue();
        i++;
        mIobLviCounter->SetValue(i);
      }
    }
  }
  else
  {
    /* add8 error */
    mAdd8ErrorCnt++;
  }
}

/*****************************************************************************
 * Function - BuildTxTgm
 * DESCRIPTION: Build next IOB tx tgm and stores it in mTxTgm
 *
 *****************************************************************************/
void IobComDrv::BuildTxTgm()
{
  int i, add8_temp;

  /* Clear ioctrl status word and set start delimiter bit */
  mTxTgm[IOCTRL_STATUS_TXPOS] = b10000000;

  /* Set digital output */
  if (mTestModeActiveFlag)
  {
    mTxTgm[DIG_OUT_TXPOS] = (mDigOutBufTestMode<<2)&0x7F;
  }
  else
  {
    mTxTgm[DIG_OUT_TXPOS] = mDigOutBuf<<2;
  }

  /* Handle GENI transmission if any */
  if ( mBusTransmit == true )
  {
    if ( mBus1TxFlag == true )
    {
      mTxTgm[BUS_TX1_7_1_TXPOS] = mBus1TxByte>>1;
      mTxTgm[BUS_TX1_0_TXPOS] &= b11111110;
      mTxTgm[BUS_TX1_0_TXPOS] |= mBus1TxByte & b00000001;
      mTxTgm[IOCTRL_STATUS_TXPOS] |= BUS1_OUTPUT_DATA_VALID;
      mBus1TxFlag = false;
      if ( mBus2TxFlag == true )
      {
        mTxTgm[BUS_TX2_7_1_TXPOS] = mBus2TxByte>>1;
        mTxTgm[BUS_TX2_0_TXPOS] &= b11111101;
        mTxTgm[BUS_TX2_0_TXPOS] |= (mBus2TxByte & b00000001)<<1;
        mTxTgm[IOCTRL_STATUS_TXPOS] |= BUS2_OUTPUT_DATA_VALID;
        mBus2TxFlag = false;
      }
    }
    if ( mBusReset == true )
    {
      mBusTransmit = false;
      mBusReset = false;
    }
    else
    {
      SendTxByte(RS232_CH_INDX);

    }
  }

  else if ( mBusReset == true )
  {
    mBusTransmit = false;
    mBusReset = false;
  }

  /* Request for data via debug system */
  if (mIOBActiveFlag == true)
  {
    switch (mCurrentDebugRequest)
    {
      case IOB_DEBUG_REQUEST_VERSION_NUMBER:
        /* Request for IOB version number */
        if (mIOBVersionRequestFlag == true)
        {
          switch ( mRequestedIOBVersionByte )
          {
            case IOB_VERSION_NUMBER_HI_BYTE :
              mDebugAddrHiByte = (U8)( IOB_VERSION_HI_BYTE_ADDR >> 8 );
              mDebugAddrLoByte = (U8)( IOB_VERSION_HI_BYTE_ADDR );
              mDebugAddrHiByteReady = true;
              mDebugAddrLoByteReady = true;
              mIOBVersionRequestFlag = false;
              break;
            case IOB_VERSION_NUMBER_MID_BYTE :
              mDebugAddrHiByte = (U8)( IOB_VERSION_MID_BYTE_ADDR >> 8 );
              mDebugAddrLoByte = (U8)( IOB_VERSION_MID_BYTE_ADDR );
              mDebugAddrHiByteReady = true;
              mDebugAddrLoByteReady = true;
              mIOBVersionRequestFlag = false;
              break;
            case IOB_VERSION_NUMBER_LO_BYTE :
              mDebugAddrHiByte = (U8)( IOB_VERSION_LO_BYTE_ADDR >> 8 );
              mDebugAddrLoByte = (U8)( IOB_VERSION_LO_BYTE_ADDR );
              mDebugAddrHiByteReady = true;
              mDebugAddrLoByteReady = true;
              mIOBVersionRequestFlag = false;
              break;
            default:
              /* Not a relevant request - stop IOB version requesting */
              mIOBVersionRequestFlag = false;
              break;
          }
        }
        break;

      case IOB_DEBUG_REQUEST_DIGITAL_INPUT:
        /* Request for IOB digital input */
        if (mIOBDigitalInputRequestFlag == true)
        {
          mDebugAddrHiByte = (U8)( IOB_DIGITAL_INPUT_ADDR >> 8 );
          mDebugAddrLoByte = (U8)( IOB_DIGITAL_INPUT_ADDR );
          mDebugAddrHiByteReady = true;
          mDebugAddrLoByteReady = true;
          mIOBDigitalInputRequestFlag = false;
        }
        break;

      case IOB_DEBUG_REQUEST_AI_PRESSURE:
        /* Request for IOB analog input - pressure */
        if (mIOBAnalogInputPressureRequestFlag == true)
        {
          switch ( mRequestedIOBAnalogInputPressureByte )
          {
            case IOB_AI_PRESSURE_HI_BYTE:
              mDebugAddrHiByte = (U8)( IOB_AI_PRESSURE_HI_BYTE_ADDR >> 8 );
              mDebugAddrLoByte = (U8)( IOB_AI_PRESSURE_HI_BYTE_ADDR );
              mDebugAddrHiByteReady = true;
              mDebugAddrLoByteReady = true;
              mIOBAnalogInputPressureRequestFlag = false;
              break;
            case IOB_AI_PRESSURE_LO_BYTE:
              mDebugAddrHiByte = (U8)( IOB_AI_PRESSURE_LO_BYTE_ADDR >> 8 );
              mDebugAddrLoByte = (U8)( IOB_AI_PRESSURE_LO_BYTE_ADDR );
              mDebugAddrHiByteReady = true;
              mDebugAddrLoByteReady = true;
              mIOBAnalogInputPressureRequestFlag = false;
              break;
            default:
              /* Not a relevant request - stop IOB analog input - pressure requesting */
              mIOBAnalogInputPressureRequestFlag = false;
              break;
          }
        }
        break;

      default:
        break;
    }
  }

  /* Insert debug request - if any */
  if ( mDebugAddrHiByteReady == true )
  {
    /* The high byte of a debug address is waiting for transmission */
    mTxTgm[DEBUG_ADDR7_1_TXPOS] = mDebugAddrHiByte>>1;
    mTxTgm[IOCTRL_STATUS_TXPOS] &= b11111101;
    mTxTgm[IOCTRL_STATUS_TXPOS] |= ( (mDebugAddrHiByte & b00000001) << 1);
    mTxTgm[IOCTRL_STATUS_TXPOS] |= HI_BYTE;
    mDebugAddrHiByteReady = false;
  }
  else
  {
    if ( mDebugAddrLoByteReady == true )
    {
      /* The low byte of a debug address is waiting for transmission */
      mTxTgm[DEBUG_ADDR7_1_TXPOS] = mDebugAddrLoByte>>1;
      mTxTgm[IOCTRL_STATUS_TXPOS] &= b11111101;
      mTxTgm[IOCTRL_STATUS_TXPOS] |= ( (mDebugAddrLoByte & b00000001) << 1 );
      mTxTgm[IOCTRL_STATUS_TXPOS] |= LO_BYTE;
      mDebugAddrLoByteReady = false;
      mWaitForDebugReply = DEBUG_REPLY_TIMEOUT;
    }
  }


  /* 8-bits add checksum with bit7 clear */
  add8_temp = 0;
  for (i=0; i<(TXTGM_LEN-1); i++)
  {
    add8_temp += mTxTgm[i];
  }
  mTxTgm[ADD8_TXPOS] = add8_temp & b01111111;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

