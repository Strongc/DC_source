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
#ifdef TO_RUN_ON_PC
 
#else

#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <IobComDrv.h>
#include <IobHwIf.h>
#include <microp.h>
#include <binary.h>
#include <mailbox.h>

extern "C"                              // TEST JUA: Scope extern "C" can be removed
{                                       // when new GENIpro version is available.
#include <rs232_drv_if.h>
}


/*****************************************************************************
  DEFINES
 *****************************************************************************/

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
#ifndef __PC__
extern "C" void InterComTask(void)
{
  IobComDrv::GetInstance()->Run();
}

#else

extern "C" void InterComReceiver(void)
{
  IobComDrv::GetInstance()->RunReceiver();
}

extern "C" void InterComTransmitter(void)
{
  IobComDrv::GetInstance()->RunTransmitter();
}

#endif // __PC__


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
#ifndef __PC__
  OS_CREATETASK(&mTCBInterComTask, "InterCom Task", InterComTask, 200, mStackInterComTask);
#else
  OS_CREATETASK(&mTCBInterComReceiver, "InterCom Receiver", InterComReceiver, 200, mStackInterComReceiver);
  OS_CREATETASK(&mTCBInterComTransmitter, "InterCom Transmitter", InterComTransmitter, 200, mStackInterComTransmitter);
#endif
}


/*****************************************************************************
 * Function - Run
 * DESCRIPTION: Main function of IobComDrv
 *
 *****************************************************************************/
void IobComDrv::RunTransmitter(void)
{   
    mNoOfTxTgmsToSend = MB_IOB_TRANSMIT_SIZE / TXTGM_LEN;
    /* Fill IOB tx mailbox with mNoOfTxTgmsToSend elements */
    int i = 0;
    while (i<mNoOfTxTgmsToSend)
    {
        // Build next IOB tx tgm 
        BuildTxTgm();
       
        OS_PutMail(&MBIobTransmit,&mTxTgm);
        i++;
    } 


    mTaskRunPhase = TRUE;

    InitInterComChannel();

    while (1)
    {    
        i = 0;     
        while (i<mNoOfTxTgmsToSend)
        {
            // Build next IOB tx tgm 
            BuildTxTgm();
       
            OS_PutMail(&MBIobTransmit,&mTxTgm);
            i++;
        }    
    }
}

void IobComDrv::RunReceiver(void)
{
    char rx_byte; 
    unsigned char msg[RXTGM_LEN];
    /* Get first byte from mailbox */    
    while (1)
    {
        OS_GetMail(&MBIobReceive, msg);       
        for (int j=0; j<RXTGM_LEN; j++) {
            rx_byte = msg[j];
            switch (mRxTgmState)
            {
            case IOB_TGM_STATE_SD_EXPECTED :
                // Check for start delimiter bit (SD). No bytes is stored before
                //SD is found 
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
                // We are in the middle of a tgm where no SD should occur. If SD found
                //    anyway mTgmIndx must be reset 
                mRxTgmIndx = 0;
                }
                mRxTgm[mRxTgmIndx] = rx_byte;
                if (mRxTgmIndx >= RXTGM_LEN-1)
                {
                // mRxTgm now holds a complete IOB tgm. Process tgm and search for next SD 
                InterpreteRxTgm();
                mRxTgmState = IOB_TGM_STATE_SD_EXPECTED;
                }
                else
                {
                mRxTgmIndx++;
                }
                break;
            default:
                // no action 
                break;
            }
        }      
    }
}

/*****************************************************************************
 * Function - StoreIobRxTgm
 * DESCRIPTION: Puts one rx tgm in IOB receive mailbox
 *
 *****************************************************************************/
U8 IobComDrv::StoreIobRxTgm(U8* rxTgm)
{
  U8 ret_val = 1;

  if ( mTaskRunPhase == TRUE )
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
#ifndef __PC__
void IobComDrv::SignalIobDataRxed(void)
{
  if ( mTaskRunPhase == TRUE )
  {
    OS_SignalEvent(TASK_EVENT_IOB_DATA_RXED, &mTCBInterComTask);
  }
}
#endif

/*****************************************************************************
 * Function - RetrieveIobTxTgm
 * DESCRIPTION: Gets one tx tgm from IOB transmit mailbox
 *
 *****************************************************************************/
U8 IobComDrv::RetrieveIobTxTgm(U8* txTgm)
{
  U8 ret_val = 1;

  if ( mTaskRunPhase == TRUE )
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
#ifndef __PC__
void IobComDrv::SignalIobDataTxed(void)
{
  if ( mTaskRunPhase == TRUE )
  {
    OS_SignalEvent(TASK_EVENT_IOB_DATA_TXED, &mTCBInterComTask);
  }
}
#endif

/*****************************************************************************
 * Function - GetIobTxTgmCnt
 * DESCRIPTION:
 *
 *****************************************************************************/
U8 IobComDrv::GetIobTxTgmCnt(void)
{
  U8 ret_val = 0;

  if ( mTaskRunPhase == TRUE )
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
  return mAnaInBuf[no];
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
  mBusTransmit = TRUE;
  mBusReset = FALSE;
}

/*****************************************************************************
 * Function - StoreTxByte
 * DESCRIPTION: Store bus byte delivered via argument in buffer. If more bytes
 *              can be stored TRUE is returned.
 *****************************************************************************/
bool IobComDrv::StoreTxByte(U8 txByte)
{
  bool request_more_bytes = FALSE;

  if ( mBus1TxFlag == FALSE )
  {
    mBus1TxByte = txByte;
    mBus1TxFlag = TRUE;
    request_more_bytes = TRUE;
  }
  else if ( mBus2TxFlag == FALSE )
  {
    mBus2TxByte = txByte;
    mBus2TxFlag = TRUE;
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
  mBusReset = TRUE;
}

/*****************************************************************************
 * Function - TestDigitalInput
 * DESCRIPTION: Returns state of digital input specified by argument
 *              TRUE: High
 *              FALSE: Low
 *****************************************************************************/
bool IobComDrv::TestDigitalInput(IOB_DIG_IN_NO_TYPE no)
{
  return (bool)(TEST_BIT_HIGH(mDigInBuf, no));
}

/*****************************************************************************
 * Function - GetDigitalInputs
 * DESCRIPTION: Deliver digital inputs via pointer argument
 *
 *****************************************************************************/
void IobComDrv::GetDigitalInputs(U8* pDigInBuf)
{
  *pDigInBuf = mDigInBuf;
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

  mBusTransmit = FALSE;
  mBusReset = FALSE;
  mBus1TxFlag = FALSE;
  mBus2TxFlag = FALSE;
  mBusBusyFlag = FALSE;
  mDigOutBuf = 0;
  mAdd8ErrorCnt = 0;
  mAdd8OkCnt = 0;
  mTaskRunPhase = FALSE;
  mRxTgmState = IOB_TGM_STATE_SD_EXPECTED;
  mTestModeActiveFlag = false;
  for ( i=0; i<TXTGM_LEN; i++)
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
      if ( mBusBusyFlag == FALSE )
      {
        mBusBusyFlag = TRUE;
        GeniChannelBusy(RS232_CH_INDX, TRUE);
      }
    }
    else
    {
      if ( mBusBusyFlag == TRUE )
      {
        mBusBusyFlag = FALSE;
        GeniChannelBusy(RS232_CH_INDX, FALSE);
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
  if ( mBusTransmit == TRUE )
  {
    if ( mBus1TxFlag == TRUE )
    {
      mTxTgm[BUS_TX1_7_1_TXPOS] = mBus1TxByte>>1;
      mTxTgm[BUS_TX1_0_TXPOS] &= b11111110;
      mTxTgm[BUS_TX1_0_TXPOS] |= mBus1TxByte & b00000001;
      mTxTgm[IOCTRL_STATUS_TXPOS] |= BUS1_OUTPUT_DATA_VALID;
      mBus1TxFlag = FALSE;
      if ( mBus2TxFlag == TRUE )
      {
        mTxTgm[BUS_TX2_7_1_TXPOS] = mBus2TxByte>>1;
        mTxTgm[BUS_TX2_0_TXPOS] &= b11111101;
        mTxTgm[BUS_TX2_0_TXPOS] |= (mBus2TxByte & b00000001)<<1;
        mTxTgm[IOCTRL_STATUS_TXPOS] |= BUS2_OUTPUT_DATA_VALID;
        mBus2TxFlag = FALSE;
      }
    }
    if ( mBusReset == TRUE )
    {
      mBusTransmit = FALSE;
      mBusReset = FALSE;
    }
    else
    {
      SendTxByte(RS232_CH_INDX);
    }
  }
  else if ( mBusReset == TRUE )
  {
    mBusTransmit = FALSE;
    mBusReset = FALSE;
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

