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
/* CLASS NAME       : GeniAppIf                                             */
/*                                                                          */
/* FILE NAME        : GeniAppIf.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 29-02-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <PowerDown.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <GeniAppIf.h>
#include <GeniAppIfData.h>
#include <ConfigControl.h>
#include <AppTypeDefs.h>
extern "C"
{
  #include <geni_if.h>         /* Access to GENI buffers */
}
#include <ErrorLog.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

#define BIT0    (1<<0)
#define BIT1    (1<<1)
#define BIT2    (1<<2)
#define BIT3    (1<<3)
#define BIT4    (1<<4)
#define BIT5    (1<<5)
#define BIT6    (1<<6)
#define BIT7    (1<<7)
#define BIT8    (1<<8)
#define BIT9    (1<<9)
#define BIT10   (1<<10)
#define BIT11   (1<<11)
#define BIT12   (1<<12)
#define BIT13   (1<<13)
#define BIT14   (1<<14)
#define BIT15   (1<<15)

#define EXTRACT_BYTE_0(dp)      ((dp->GetValue() & 0x000000FF) >> 0)
#define EXTRACT_BYTE_1(dp)      ((dp->GetValue() & 0x0000FF00) >> 8)
#define EXTRACT_BYTE_2(dp)      ((dp->GetValue() & 0x00FF0000) >> 16)
#define EXTRACT_BYTE_3(dp)      ((dp->GetValue() & 0xFF000000) >> 24)

#define INSERT_BYTE_0(byte,dp)  dp->SetValue((dp->GetValue() & 0xFFFFFF00) | (byte << 0))
#define INSERT_BYTE_1(byte,dp)  dp->SetValue((dp->GetValue() & 0xFFFF00FF) | (byte << 8))
#define INSERT_BYTE_2(byte,dp)  dp->SetValue((dp->GetValue() & 0xFF00FFFF) | (byte << 16))
#define INSERT_BYTE_3(byte,dp)  dp->SetValue((dp->GetValue() & 0x00FFFFFF) | (byte << 24))

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  CREATES AN OBJECT.
 ******************************************************************************/
GeniAppIf* GeniAppIf::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
/*****************************************************************************
 * Function - GeniBusDataReceived
 * DESCRIPTION: GENIpro user function Bus_post_user_fct
 *
 *****************************************************************************/
extern "C" void GeniBusDataReceived(void)
{
  GeniAppIf::GetInstance()->DataFromBus();
}


/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
GeniAppIf* GeniAppIf::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new GeniAppIf();
  }
  return mInstance;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniAppIf::DataFromBus()
{
  mDataFromBusFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool GeniAppIf::RunCommand(U8 cmd)
{
  if (mExternalCommandPending)
  {
    return false;
  }
  mExternalCommand = cmd;
  mExternalCommandPending = true;
  ReqTaskTime();
  return true;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *****************************************************************************/
void GeniAppIf::InitSubTask(void)
{
  gai_11_32_pit_pump_mode = 0xFFFF; // pumps not present

  // request task time to empty mSubjectQueue initially
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
*****************************************************************************/
void GeniAppIf::RunSubTask()
{
  Subject* pSubject;

  // reset task flag
  mReqTaskTimeFlag = false;

  // handle data from bus
  if (mDataFromBusFlag)
  {
    GENI_USE_CLASS_DATA;
    HandleGeniBuffers();
    GENI_UNUSE_CLASS_DATA;
    mDataFromBusFlag = false;
  }

  // handle external commands (currently used for acceping GENI commands via SMS)
  if (mExternalCommandPending)
  {
    if (!BaseGeniAppIf::HandleCommand((GAI_CMD_TYPE)mExternalCommand))
    {
      HandleCommand((GAI_CMD_TYPE)mExternalCommand);
    }
    mExternalCommandPending = false;
  }

  // handle changed subjects
  OS_Use(&mSemaSubjectQueue);
  pSubject = mSubjectQueue.read();
  while (pSubject != NULL)
  {
    OS_Unuse(&mSemaSubjectQueue);
    if (!BaseGeniAppIf::SubjectToGeni(pSubject))
    {
      SubjectToGeni(pSubject);
    }
    OS_Use(&mSemaSubjectQueue);
    pSubject = mSubjectQueue.read();
  }
  OS_Unuse(&mSemaSubjectQueue);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
*****************************************************************************/
void GeniAppIf::SetSubjectPointer(int id, Subject* pSubject)
{
  // handled in base class
  BaseGeniAppIf::SetSubjectPointer(id, pSubject);

  // add to subject queue initially to update all external geni variables
  mSubjectQueue.write(pSubject);
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
*****************************************************************************/
void GeniAppIf::ConnectToSubjects(void)
{
  // handled in base class
  BaseGeniAppIf::ConnectToSubjects();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
*****************************************************************************/
void GeniAppIf::Update(Subject* pSubject)
{
  OS_Use(&mSemaSubjectQueue);
  mSubjectQueue.write(pSubject);
  OS_Unuse(&mSemaSubjectQueue);

  if (mReqTaskTimeFlag == false)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
*****************************************************************************/
void GeniAppIf::SubscribtionCancelled(Subject* pSubject)
{
  // handled in base class
  BaseGeniAppIf::SubscribtionCancelled(pSubject);
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
GeniAppIf::GeniAppIf()
{
  OS_CREATERSEMA(&mSemaSubjectQueue);
  mReqTaskTimeFlag = false;
  mDataFromBusFlag = false;
  mExternalCommandPending = false;
  mExternalCommand = 0;
  mpActTime = ActTime::GetInstance();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GeniAppIf::~GeniAppIf()
{
}

/*****************************************************************************
 * Function - HandleGeniBuffers
 * DESCRIPTION: Extract Geni commands and values from buffers and call some
 *              geni class specific funtions for further processing.
 *****************************************************************************/
void GeniAppIf::HandleGeniBuffers()
{
  GAI_VAR_TYPE geniVar;
  U8 u8Value;
  U16 u16Value;
  U32 u32Value;

  // look for new commands in GENI command buffer
  while (cmd_buf[WR_INDX] > cmd_buf[RD_INDX])
  {
    // get channel (not used)
    cmd_buf[RD_INDX]++;
    // get geni cmd
    GAI_CMD_TYPE geniCmd = (GAI_CMD_TYPE)(cmd_buf[cmd_buf[RD_INDX]++]&= 0x7F);

    // handle command
    if (!BaseGeniAppIf::HandleCommand(geniCmd))
    {
      HandleCommand(geniCmd);
    }
  }
  // reset GENI command buffer
  cmd_buf[WR_INDX] = BUF_START;
  cmd_buf[RD_INDX] = BUF_START;


  // look for new 8 bit configuration data in GENI configuration buffer
  while (conf_buf[WR_INDX] > conf_buf[RD_INDX])
  {
    conf_buf[RD_INDX]++;  // channel not used

    // get geni var and value
    geniVar = (GAI_VAR_TYPE)((4 << 8) | conf_buf[conf_buf[RD_INDX]++]);
    u8Value = conf_buf[conf_buf[RD_INDX]++];

    // handle the new value
    if (!BaseGeniAppIf::GeniToSubject(geniVar, u8Value))
    {
      HandleConfiguration8bit(geniVar, u8Value);
    }
  }
  // reset GENI configuration buffer
  conf_buf[WR_INDX] = BUF_START;
  conf_buf[RD_INDX] = BUF_START;


  // look for new 8 bit reference data in GENI reference buffer
  while (ref_buf[WR_INDX] > ref_buf[RD_INDX])
  {
    ref_buf[RD_INDX]++;  // channel not used

    // get geni var and value
    geniVar = (GAI_VAR_TYPE)((5 << 8) | ref_buf[ref_buf[RD_INDX]++]);
    u8Value = ref_buf[ref_buf[RD_INDX]++];

    // handle the new value
    if (!BaseGeniAppIf::GeniToSubject(geniVar, u8Value))
    {
      HandleReference8bit(geniVar, u8Value);
    }
  }
  // reset GENI reference buffer
  ref_buf[WR_INDX] = BUF_START;
  ref_buf[RD_INDX] = BUF_START;


  // look for new 16 bit configuration data in GENI configuration buffer
  while (conf16_buf[WR_INDX] > conf16_buf[RD_INDX])
  {
    conf16_buf[RD_INDX]++;  // channel not used

    // get geni var and value
    geniVar = (GAI_VAR_TYPE)((12 << 8) | conf16_buf[conf16_buf[RD_INDX]++]);
    u16Value = conf16_buf[conf16_buf[RD_INDX]++];

    // handle the new value
    if (!BaseGeniAppIf::GeniToSubject(geniVar, u16Value))
    {
      HandleConfiguration16Bit(geniVar, u16Value);
    }
  }
  // reset GENI 16 bit configuration buffer
  conf16_buf[WR_INDX] = BUF_START;
  conf16_buf[RD_INDX] = BUF_START;


  // look for new 16 bit reference data in GENI reference buffer
  while (ref16_buf[WR_INDX] > ref16_buf[RD_INDX])
  {
    ref16_buf[RD_INDX]++;  // channel not used

    // get geni var and value
    geniVar = (GAI_VAR_TYPE)((13 << 8) | ref16_buf[ref16_buf[RD_INDX]++]);
    u16Value = ref16_buf[ref16_buf[RD_INDX]++];

    // handle the new value
    if (!BaseGeniAppIf::GeniToSubject(geniVar, u16Value))
    {
      HandleReference16Bit(geniVar, u16Value);
    }
  }
  // reset GENI 16 bit reference buffer
  ref16_buf[WR_INDX] = BUF_START;
  ref16_buf[RD_INDX] = BUF_START;


  // look for new 32 bit configuration data in GENI configuration buffer
  while (conf32_buf[WR_INDX] > conf32_buf[RD_INDX])
  {
    conf32_buf[RD_INDX]++;  // channel not used

    // get geni var and value
    geniVar = (GAI_VAR_TYPE)((15 << 8) | conf32_buf[conf32_buf[RD_INDX]++]);
    u32Value = conf32_buf[conf32_buf[RD_INDX]++];

    // handle the new value
    if (!BaseGeniAppIf::GeniToSubject(geniVar, u32Value))
    {
      HandleConfiguration32Bit(geniVar, u32Value);
    }
  }
  // reset GENI 32 bit configuration buffer
  conf32_buf[WR_INDX] = BUF_START;
  conf32_buf[RD_INDX] = BUF_START;


  // look for new 32 bit reference data in GENI reference buffer
  while (ref32_buf[WR_INDX] > ref32_buf[RD_INDX])
  {
    ref32_buf[RD_INDX]++;  // channel not used

    // get geni var and value
    geniVar = (GAI_VAR_TYPE)((16 << 8) | ref32_buf[ref32_buf[RD_INDX]++]);
    u32Value = ref32_buf[ref32_buf[RD_INDX]++];

    // handle the new value
    if (!BaseGeniAppIf::GeniToSubject(geniVar, u32Value))
    {
      HandleReference32Bit(geniVar, u32Value);
    }
  }
  // reset GENI 32 bit reference buffer
  ref32_buf[WR_INDX] = BUF_START;
  ref32_buf[RD_INDX] = BUF_START;
}

/*****************************************************************************
 * Function - HandleCommand
 * DESCRIPTION: Handling of NOT auto generated Geni commands
 *****************************************************************************/
void GeniAppIf::HandleCommand(GAI_CMD_TYPE geniCmd)
{
  switch (geniCmd)
  {
    case GAI_CMD_RESET:
      SignalEventToPowerDown(POWER_DOWN_EVENT);
      break;
    case GAI_CMD_RESET_HIST:
	  if(gai_4_21_mac_addr_0 == 0 && gai_4_22_mac_addr_1 == 0 && gai_4_23_mac_addr_2 ==0 && gai_4_24_mac_addr_3 == 0 &&
	     gai_4_25_mac_addr_4 == 0 && gai_4_26_mac_addr_5 == 0)
	  {
		ConfigControl::GetInstance()->Boot(LOG_BLOCK);
	  }
      break;
    case GAI_CMD_RESET_EVENT_LOG:
      mp_alarm_clear_alarm_log_event->SetEvent();
      mp_reset_event_log->SetEvent();
      break;

    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function - HandleConfiguration8bit
 * DESCRIPTION: Handling of NOT auto generated Geni data
 *****************************************************************************/
void GeniAppIf::HandleConfiguration8bit(GAI_VAR_TYPE geniVar, U8 newValue)
{
  switch (geniVar)
  {
    case GAI_VAR_PRODUCTION_CODE_1:
      INSERT_BYTE_3(newValue, mp_production_type_code);
      break;
    case GAI_VAR_PRODUCTION_CODE_2:
      INSERT_BYTE_2(newValue, mp_production_type_code);
      break;
    case GAI_VAR_PRODUCTION_CODE_3:
      INSERT_BYTE_1(newValue, mp_production_type_code);
      break;
    case GAI_VAR_PRODUCTION_CODE_4:
      INSERT_BYTE_0(newValue, mp_production_type_code);
      break;

    case GAI_VAR_PRODUCTION_CODE_5:
      INSERT_BYTE_2(newValue, mp_production_rev_date_code);
      break;
    case GAI_VAR_PRODUCTION_CODE_6:
      INSERT_BYTE_1(newValue, mp_production_rev_date_code);
      break;
    case GAI_VAR_PRODUCTION_CODE_7:
      INSERT_BYTE_0(newValue, mp_production_rev_date_code);
      break;

    case GAI_VAR_PRODUCTION_CODE_8:
      INSERT_BYTE_2(newValue, mp_production_serial_no);
      break;
    case GAI_VAR_PRODUCTION_CODE_9:
      INSERT_BYTE_1(newValue, mp_production_serial_no);
      break;
    case GAI_VAR_PRODUCTION_CODE_10:
      INSERT_BYTE_0(newValue, mp_production_serial_no);
      break;

    case GAI_VAR_MAC_ADDR_5:
      INSERT_BYTE_2(newValue, mp_ethernet_mac_address_hi);
      break;
    case GAI_VAR_MAC_ADDR_4:
      INSERT_BYTE_1(newValue, mp_ethernet_mac_address_hi);
      break;
    case GAI_VAR_MAC_ADDR_3:
      INSERT_BYTE_0(newValue, mp_ethernet_mac_address_hi);
      break;
    case GAI_VAR_MAC_ADDR_2:
      INSERT_BYTE_2(newValue, mp_ethernet_mac_address_lo);
      break;
    case GAI_VAR_MAC_ADDR_1:
      INSERT_BYTE_1(newValue, mp_ethernet_mac_address_lo);
      break;
    case GAI_VAR_MAC_ADDR_0:
      INSERT_BYTE_0(newValue, mp_ethernet_mac_address_lo);
      break;

    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function HandleReference8bit
 * DESCRIPTION: Handling of NOT auto generated Geni data
 *****************************************************************************/
void GeniAppIf::HandleReference8bit(GAI_VAR_TYPE geniVar, U8 newValue)
{
  switch (geniVar)
  {
    case 0:
      break;
    case GAI_VAR_RTC_YEAR :     	  
	  mpActTime->SetDate(YEAR, 2000+newValue);
      break;
	case GAI_VAR_RTC_MONTH_OF_YEAR:
	  mpActTime->SetDate(MONTH, newValue);
	  break;
    case GAI_VAR_RTC_DAY_OF_MONTH:
	  mpActTime->SetDate(DAY, newValue);
      break;
    case GAI_VAR_RTC_HOUR:
	  mpActTime->SetTime(HOURS, newValue);
      break;
    case GAI_VAR_RTC_MINUTE:
	  mpActTime->SetTime(MINUTES, newValue);
      break;
	case GAI_VAR_RTC_SECOND:
	  mpActTime->SetTime(SECONDS, newValue);
	  break;

    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function HandleReference16Bit
 * DESCRIPTION: Handling of NOT auto generated Geni data
 *****************************************************************************/
void GeniAppIf::HandleReference16Bit(GAI_VAR_TYPE geniVar, U16 newValue)
{
  switch (geniVar)
  {
    case 0 :
      break;

    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function HandleConfiguration16Bit
 * DESCRIPTION: Handling of NOT auto generated Geni data
 *****************************************************************************/
void GeniAppIf::HandleConfiguration16Bit(GAI_VAR_TYPE geniVar, U16 newValue)
{
  switch (geniVar)
  {
    case GAI_VAR_CURRENT_MAX_ALARM_GROUP1:		
      GeniToDataPoint(newValue,
                      mp_pump_group_1_alarm_motor_current_overload_alarm_conf.GetSubject(),
                      ALARM,
                      GENI_CONVERT_ID_CURRENT_DOT1A);		
      break;

    case GAI_VAR_CURRENT_MIN_ALARM_GROUP1:
      GeniToDataPoint(newValue,
                      mp_pump_group_1_alarm_motor_current_underload_alarm_conf.GetSubject(),
                      ALARM,
                      GENI_CONVERT_ID_CURRENT_DOT1A);	
      break;

    case GAI_VAR_CURRENT_MAX_WARN_GROUP1:
      GeniToDataPoint(newValue,
                      mp_pump_group_1_alarm_motor_current_overload_alarm_conf.GetSubject(),
                      WARNING,
                      GENI_CONVERT_ID_CURRENT_DOT1A);	
      break;

    case GAI_VAR_CURRENT_MIN_WARN_GROUP1:
      GeniToDataPoint(newValue,
                      mp_pump_group_1_alarm_motor_current_underload_alarm_conf.GetSubject(),
                      WARNING,
                      GENI_CONVERT_ID_CURRENT_DOT1A);	
      break;

    case GAI_VAR_CURRENT_MAX_ALARM_GROUP2:
      GeniToDataPoint(newValue,
                      mp_pump_group_2_alarm_motor_current_overload_alarm_conf.GetSubject(),
                      ALARM,
                      GENI_CONVERT_ID_CURRENT_DOT1A);		
      break;

    case GAI_VAR_CURRENT_MIN_ALARM_GROUP2:
      GeniToDataPoint(newValue,
                      mp_pump_group_2_alarm_motor_current_underload_alarm_conf.GetSubject(),
                      ALARM,
                      GENI_CONVERT_ID_CURRENT_DOT1A);	
      break;

    case GAI_VAR_CURRENT_MAX_WARN_GROUP2:
      GeniToDataPoint(newValue,
                      mp_pump_group_2_alarm_motor_current_overload_alarm_conf.GetSubject(),
                      WARNING,
                      GENI_CONVERT_ID_CURRENT_DOT1A);	
      break;

    case GAI_VAR_CURRENT_MIN_WARN_GROUP2:
      GeniToDataPoint(newValue,
                      mp_pump_group_2_alarm_motor_current_underload_alarm_conf.GetSubject(),
                      WARNING,
                      GENI_CONVERT_ID_CURRENT_DOT1A);	
      break;
    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function HandleConfiguration32Bit
 * DESCRIPTION: Handling of NOT auto generated Geni data
 *****************************************************************************/
void GeniAppIf::HandleConfiguration32Bit(GAI_VAR_TYPE geniVar, U32 newValue)
{
  switch (geniVar)
  {
    case 0 :
      break;

    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function HandleReference32Bit
 * DESCRIPTION: Handling of NOT auto generated Geni data
 *****************************************************************************/
void GeniAppIf::HandleReference32Bit(GAI_VAR_TYPE geniVar, U32 newValue)
{
  switch (geniVar)
  {
    case 0 :
      break;

    default:
      // unhandled!!
      break;
  }
}

/*****************************************************************************
 * Function - SubjectToGeni
 * DESCRIPTION: Update Geni values for NOT auto generated subjects
 *****************************************************************************/
void GeniAppIf::SubjectToGeni(Subject* pSubject)
{
  if (HandleSpecialBitPacking(pSubject) == false)
  {
    switch (pSubject->GetSubjectId())
    {
      case SUBJECT_ID_SCADA_PIN_CODE:
        gai_2_9_scada_pin_code_hi  = EXTRACT_BYTE_1(mp_scada_pin_code);
        gai_2_10_scada_pin_code_lo = EXTRACT_BYTE_0(mp_scada_pin_code);
        break;

      case SUBJECT_ID_STATUS_LED_GREEN_STATE:
      case SUBJECT_ID_STATUS_LED_RED_STATE:
        gai_2_73_led_status = mp_status_led_green_state->GetValue();
        gai_2_73_led_status |= (mp_status_led_red_state->GetValue() << 2);
        break;

      case SUBJECT_ID_PRODUCTION_TYPE_CODE:
        gai_4_0_production_code_1 = EXTRACT_BYTE_3(mp_production_type_code);
        gai_4_1_production_code_2 = EXTRACT_BYTE_2(mp_production_type_code);
        gai_4_2_production_code_3 = EXTRACT_BYTE_1(mp_production_type_code);
        gai_4_3_production_code_4 = EXTRACT_BYTE_0(mp_production_type_code);
        break;

      case SUBJECT_ID_PRODUCTION_REV_DATE_CODE:
        gai_4_4_production_code_5 = EXTRACT_BYTE_2(mp_production_rev_date_code);
        gai_4_5_production_code_6 = EXTRACT_BYTE_1(mp_production_rev_date_code);
        gai_4_6_production_code_7 = EXTRACT_BYTE_0(mp_production_rev_date_code);
        break;

      case SUBJECT_ID_PRODUCTION_SERIAL_NO:
        gai_4_7_production_code_8 = EXTRACT_BYTE_2(mp_production_serial_no);
        gai_4_8_production_code_9 = EXTRACT_BYTE_1(mp_production_serial_no);
        gai_4_9_production_code_10= EXTRACT_BYTE_0(mp_production_serial_no);
        break;

      case SUBJECT_ID_ETHERNET_MAC_ADDRESS_HI:
        gai_4_26_mac_addr_5 = EXTRACT_BYTE_2(mp_ethernet_mac_address_hi);
        gai_4_25_mac_addr_4 = EXTRACT_BYTE_1(mp_ethernet_mac_address_hi);
        gai_4_24_mac_addr_3 = EXTRACT_BYTE_0(mp_ethernet_mac_address_hi);
        break;
      case SUBJECT_ID_ETHERNET_MAC_ADDRESS_LO:
        gai_4_23_mac_addr_2 = EXTRACT_BYTE_2(mp_ethernet_mac_address_lo);
        gai_4_22_mac_addr_1 = EXTRACT_BYTE_1(mp_ethernet_mac_address_lo);
        gai_4_21_mac_addr_0 = EXTRACT_BYTE_0(mp_ethernet_mac_address_lo);
        break;

      default:
        // unhandled!!
        break;
    }
  }
}

/*****************************************************************************
 * Function HandleSpecialBitPacking
 * DESCRIPTION: Bit packing/mapping for misc status status parameters like:
 *              pump_alarm status
 *              pump_mode_and_connection status
 *
 *****************************************************************************/
bool GeniAppIf::HandleSpecialBitPacking(Subject* pSubject)
{
  bool subject_handled = true;

  switch (pSubject->GetSubjectId())
  {
    case SUBJECT_ID_ACTUAL_ALARM:
      gai_2_74_alarm_code = mp_actual_alarm->GetValue();
      gai_2_72_pit_main_status &= ~BIT1;
      gai_2_72_pit_main_status |=  BIT1*(gai_2_74_alarm_code != 0);
      break;

    case SUBJECT_ID_ACTUAL_WARNING:
      gai_2_75_warning_code = mp_actual_warning->GetValue();
      gai_2_72_pit_main_status &= ~BIT0;
      gai_2_72_pit_main_status |=  BIT0*(gai_2_75_warning_code != 0);
      break;

    case SUBJECT_ID_PUMP_FAULT_STATUS:
      gai_11_33_pit_pump_fault = mp_pump_fault_status->GetValue();
      gai_11_76_p1_mode_and_connect &= ~BIT7;
      gai_11_76_p1_mode_and_connect |=  BIT7*((gai_11_33_pit_pump_fault & BIT1) != 0); // Pump 1 warning
      gai_11_76_p1_mode_and_connect &= ~BIT8;
      gai_11_76_p1_mode_and_connect |=  BIT8*((gai_11_33_pit_pump_fault & BIT0) != 0); // Pump 1 alarm
      gai_11_106_p2_mode_and_connect &= ~BIT7;
      gai_11_106_p2_mode_and_connect |=  BIT7*((gai_11_33_pit_pump_fault & BIT3) != 0); // Pump 2 warning
      gai_11_106_p2_mode_and_connect &= ~BIT8;
      gai_11_106_p2_mode_and_connect |=  BIT8*((gai_11_33_pit_pump_fault & BIT2) != 0); // Pump 2 alarm
      gai_11_136_p3_mode_and_connect &= ~BIT7;
      gai_11_136_p3_mode_and_connect |=  BIT7*((gai_11_33_pit_pump_fault & BIT5) != 0); // Pump 3 warning
      gai_11_136_p3_mode_and_connect &= ~BIT8;
      gai_11_136_p3_mode_and_connect |=  BIT8*((gai_11_33_pit_pump_fault & BIT4) != 0); // Pump 3 alarm
      gai_11_166_p4_mode_and_connect &= ~BIT7;
      gai_11_166_p4_mode_and_connect |=  BIT7*((gai_11_33_pit_pump_fault & BIT7) != 0); // Pump 4 warning
      gai_11_166_p4_mode_and_connect &= ~BIT8;
      gai_11_166_p4_mode_and_connect |=  BIT8*((gai_11_33_pit_pump_fault & BIT6) != 0); // Pump 4 alarm
      gai_11_196_p5_mode_and_connect &= ~BIT7;
      gai_11_196_p5_mode_and_connect |=  BIT7*((gai_11_33_pit_pump_fault & BIT9) != 0); // Pump 5 warning
      gai_11_196_p5_mode_and_connect &= ~BIT8;
      gai_11_196_p5_mode_and_connect |=  BIT8*((gai_11_33_pit_pump_fault & BIT8) != 0); // Pump 5 alarm
      gai_11_226_p6_mode_and_connect &= ~BIT7;
      gai_11_226_p6_mode_and_connect |=  BIT7*((gai_11_33_pit_pump_fault & BIT11) != 0); // Pump 6 warning
      gai_11_226_p6_mode_and_connect &= ~BIT8;
      gai_11_226_p6_mode_and_connect |=  BIT8*((gai_11_33_pit_pump_fault & BIT10) != 0); // Pump 6 alarm
      break;

    case SUBJECT_ID_PUMP_MONITORING_FAULT:
      gai_11_34_pit_pump_mon_fault = mp_pump_monitoring_fault->GetValue();
      gai_11_76_p1_mode_and_connect &= ~BIT6;
      gai_11_76_p1_mode_and_connect |=  BIT6*((gai_11_34_pit_pump_mon_fault & BIT0) != 0);
      gai_11_106_p2_mode_and_connect &= ~BIT6;
      gai_11_106_p2_mode_and_connect |=  BIT6*((gai_11_34_pit_pump_mon_fault & BIT1) != 0);
      gai_11_136_p3_mode_and_connect &= ~BIT6;
      gai_11_136_p3_mode_and_connect |=  BIT6*((gai_11_34_pit_pump_mon_fault & BIT2) != 0);
      gai_11_166_p4_mode_and_connect &= ~BIT6;
      gai_11_166_p4_mode_and_connect |=  BIT6*((gai_11_34_pit_pump_mon_fault & BIT3) != 0);
      gai_11_196_p5_mode_and_connect &= ~BIT6;
      gai_11_196_p5_mode_and_connect |=  BIT6*((gai_11_34_pit_pump_mon_fault & BIT4) != 0);
      gai_11_226_p6_mode_and_connect &= ~BIT6;
      gai_11_226_p6_mode_and_connect |=  BIT6*((gai_11_34_pit_pump_mon_fault & BIT5) != 0);
      break;

    case SUBJECT_ID_OPERATION_MODE_REQ_PUMP_1:
    case SUBJECT_ID_OPERATION_MODE_REQ_PUMP_2:
    case SUBJECT_ID_OPERATION_MODE_REQ_PUMP_3:
    case SUBJECT_ID_OPERATION_MODE_REQ_PUMP_4:
    case SUBJECT_ID_OPERATION_MODE_REQ_PUMP_5:
    case SUBJECT_ID_OPERATION_MODE_REQ_PUMP_6:
    case SUBJECT_ID_PUMP_1_CONTROL_SOURCE:
    case SUBJECT_ID_PUMP_2_CONTROL_SOURCE:
    case SUBJECT_ID_PUMP_3_CONTROL_SOURCE:
    case SUBJECT_ID_PUMP_4_CONTROL_SOURCE:
    case SUBJECT_ID_PUMP_5_CONTROL_SOURCE:
    case SUBJECT_ID_PUMP_6_CONTROL_SOURCE:
      gai_2_72_pit_main_status &= ~BIT3;
      gai_2_72_pit_main_status |=  BIT3*(mp_operation_mode_req_pump_1->GetValue() != REQ_OPERATION_MODE_AUTO);
      gai_2_72_pit_main_status |=  BIT3*(mp_operation_mode_req_pump_2->GetValue() != REQ_OPERATION_MODE_AUTO); // Same bit !
      gai_2_72_pit_main_status |=  BIT3*(mp_operation_mode_req_pump_3->GetValue() != REQ_OPERATION_MODE_AUTO); // Same bit !
      gai_2_72_pit_main_status |=  BIT3*(mp_operation_mode_req_pump_4->GetValue() != REQ_OPERATION_MODE_AUTO); // Same bit !
      gai_2_72_pit_main_status |=  BIT3*(mp_operation_mode_req_pump_5->GetValue() != REQ_OPERATION_MODE_AUTO); // Same bit !
      gai_2_72_pit_main_status |=  BIT3*(mp_operation_mode_req_pump_6->GetValue() != REQ_OPERATION_MODE_AUTO); // Same bit !

      // Pump 1 bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT0+BIT1);
      if (mp_operation_mode_req_pump_1->GetValue() != REQ_OPERATION_MODE_AUTO)
      {
        switch (mp_pump_1_control_source->GetValue())
        {
          case CONTROL_SOURCE_DI:       gai_11_36_pit_pump_ctr_source |= BIT0;  break;
          case CONTROL_SOURCE_HMI:      gai_11_36_pit_pump_ctr_source |= BIT1;  break;
          case CONTROL_SOURCE_GENI_BUS: gai_11_36_pit_pump_ctr_source |= (BIT0+BIT1);  break;
          default:                      gai_11_36_pit_pump_ctr_source |= (BIT0+BIT1);  break;
        }
      }
      gai_11_76_p1_mode_and_connect &= ~(BIT2+BIT3);
      gai_11_76_p1_mode_and_connect |= ((gai_11_36_pit_pump_ctr_source & (BIT0+BIT1)) << 2);

      // Pump 2 bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT2+BIT3);
      if (mp_operation_mode_req_pump_2->GetValue() != REQ_OPERATION_MODE_AUTO)
      {
        switch (mp_pump_2_control_source->GetValue())
        {
          case CONTROL_SOURCE_DI:       gai_11_36_pit_pump_ctr_source |= BIT2;  break;
          case CONTROL_SOURCE_HMI:      gai_11_36_pit_pump_ctr_source |= BIT3;  break;
          case CONTROL_SOURCE_GENI_BUS: gai_11_36_pit_pump_ctr_source |= (BIT2+BIT3);  break;
          default:                      gai_11_36_pit_pump_ctr_source |= (BIT2+BIT3);  break;
        }
      }
      gai_11_106_p2_mode_and_connect &= ~(BIT2+BIT3);
      gai_11_106_p2_mode_and_connect |= ((gai_11_36_pit_pump_ctr_source & (BIT2+BIT3)) << 0);

      // Pump 3 bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT4+BIT5);
      if (mp_operation_mode_req_pump_3->GetValue() != REQ_OPERATION_MODE_AUTO)
      {
        switch (mp_pump_3_control_source->GetValue())
        {
          case CONTROL_SOURCE_DI:       gai_11_36_pit_pump_ctr_source |= BIT4;  break;
          case CONTROL_SOURCE_HMI:      gai_11_36_pit_pump_ctr_source |= BIT5;  break;
          case CONTROL_SOURCE_GENI_BUS: gai_11_36_pit_pump_ctr_source |= (BIT4+BIT5);  break;
          default:                      gai_11_36_pit_pump_ctr_source |= (BIT4+BIT5);  break;
        }
      }
      gai_11_136_p3_mode_and_connect &= ~(BIT2+BIT3);
      gai_11_136_p3_mode_and_connect |= ((gai_11_36_pit_pump_ctr_source & (BIT4+BIT5)) >> 2);

      // Pump 4 bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT6+BIT7);
      if (mp_operation_mode_req_pump_4->GetValue() != REQ_OPERATION_MODE_AUTO)
      {
        switch (mp_pump_4_control_source->GetValue())
        {
          case CONTROL_SOURCE_DI:       gai_11_36_pit_pump_ctr_source |= BIT6;  break;
          case CONTROL_SOURCE_HMI:      gai_11_36_pit_pump_ctr_source |= BIT7;  break;
          case CONTROL_SOURCE_GENI_BUS: gai_11_36_pit_pump_ctr_source |= (BIT6+BIT7);  break;
          default:                      gai_11_36_pit_pump_ctr_source |= (BIT6+BIT7);  break;
        }
      }
      gai_11_166_p4_mode_and_connect &= ~(BIT2+BIT3);
      gai_11_166_p4_mode_and_connect |= ((gai_11_36_pit_pump_ctr_source & (BIT6+BIT7)) >> 4);

      // Pump 5 bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT8+BIT9);
      if (mp_operation_mode_req_pump_5->GetValue() != REQ_OPERATION_MODE_AUTO)
      {
        switch (mp_pump_5_control_source->GetValue())
        {
          case CONTROL_SOURCE_DI:       gai_11_36_pit_pump_ctr_source |= BIT8;  break;
          case CONTROL_SOURCE_HMI:      gai_11_36_pit_pump_ctr_source |= BIT9;  break;
          case CONTROL_SOURCE_GENI_BUS: gai_11_36_pit_pump_ctr_source |= (BIT8+BIT9);  break;
          default:                      gai_11_36_pit_pump_ctr_source |= (BIT8+BIT9);  break;
        }
      }
      gai_11_196_p5_mode_and_connect &= ~(BIT2+BIT3);
      gai_11_196_p5_mode_and_connect |= ((gai_11_36_pit_pump_ctr_source & (BIT8+BIT9)) >> 6);

      // Pump 6 bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT10+BIT11);
      if (mp_operation_mode_req_pump_6->GetValue() != REQ_OPERATION_MODE_AUTO)
      {
        switch (mp_pump_6_control_source->GetValue())
        {
          case CONTROL_SOURCE_DI:       gai_11_36_pit_pump_ctr_source |= BIT10;  break;
          case CONTROL_SOURCE_HMI:      gai_11_36_pit_pump_ctr_source |= BIT11;  break;
          case CONTROL_SOURCE_GENI_BUS: gai_11_36_pit_pump_ctr_source |= (BIT10+BIT11);  break;
          default:                      gai_11_36_pit_pump_ctr_source |= (BIT10+BIT11);  break;
        }
      }
      gai_11_226_p6_mode_and_connect &= ~(BIT2+BIT3);
      gai_11_226_p6_mode_and_connect |= ((gai_11_36_pit_pump_ctr_source & (BIT10+BIT11)) >> 8);

      // Mixer bits
      gai_11_36_pit_pump_ctr_source &= ~(BIT12+BIT13);


      // Dosing pump
      gai_11_36_pit_pump_ctr_source &= ~(BIT14+BIT15);

      break;

    case SUBJECT_ID_PIT_LEVEL_CTRL_TYPE:
      gai_2_7_pit_sensors &= ~(BIT0+BIT1);
      switch (mp_pit_level_ctrl_type->GetValue())
      {
        case SENSOR_TYPE_ULTRA_SONIC:     gai_2_7_pit_sensors |= 0; break;
        case SENSOR_TYPE_PRESSURE:        gai_2_7_pit_sensors |= BIT0; break;
        case SENSOR_TYPE_FLOAT_SWITCHES:  gai_2_7_pit_sensors |= BIT1; break;
        default:                          gai_2_7_pit_sensors |= (BIT0+BIT1); break;
      }
      break;

    case SUBJECT_ID_FLOW_QUALITY:
      gai_2_7_pit_sensors &= ~BIT2;
      gai_2_7_pit_sensors |=  BIT2*(mp_flow_quality->GetValue() == FLOW_QUALITY_FLOW_METER);
      gai_2_7_pit_sensors |=  BIT2*(mp_flow_quality->GetValue() == FLOW_QUALITY_VOLUME_METER);
      break;

    case SUBJECT_ID_DIG_IN_FUNC_INPUT_ENERGY_CNT:
    case SUBJECT_ID_ANA_IN_1_CONF_MEASURED_VALUE:
    case SUBJECT_ID_ANA_IN_2_CONF_MEASURED_VALUE:
    case SUBJECT_ID_ANA_IN_3_CONF_MEASURED_VALUE:
    case SUBJECT_ID_ANA_IN_4_CONF_MEASURED_VALUE:
    case SUBJECT_ID_ANA_IN_5_CONF_MEASURED_VALUE:
    case SUBJECT_ID_ANA_IN_6_CONF_MEASURED_VALUE:
    case SUBJECT_ID_ANA_IN_7_CONF_MEASURED_VALUE:
      gai_2_7_pit_sensors &= ~BIT3;
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_1_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_2_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_3_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_4_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_5_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_6_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_ana_in_7_conf_measured_value->GetValue() == MEASURED_VALUE_POWER);
      gai_2_7_pit_sensors |=  BIT3*(mp_dig_in_func_input_energy_cnt->GetValue() > 0);

      gai_2_7_pit_sensors &= ~BIT4;
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_1_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_2_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_3_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_4_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_5_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_6_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);
      gai_2_7_pit_sensors |=  BIT4*(mp_ana_in_7_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_1);

      gai_2_7_pit_sensors &= ~BIT5;
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_1_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_2_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_3_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_4_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_5_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_6_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);
      gai_2_7_pit_sensors |=  BIT5*(mp_ana_in_7_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_2);

      gai_2_7_pit_sensors &= ~BIT6;
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_1_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_2_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_3_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_4_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_5_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_6_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      gai_2_7_pit_sensors |=  BIT6*(mp_ana_in_7_conf_measured_value->GetValue() == MEASURED_VALUE_USER_DEFINED_SOURCE_3);
      break;

    case SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_1:
      gai_11_32_pit_pump_mode &= ~(BIT0+BIT1);
      switch (mp_operation_mode_actual_pump_1->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT0; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT1; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT0+BIT1); break;
      }
      gai_11_76_p1_mode_and_connect &= ~(BIT0+BIT1);
      gai_11_76_p1_mode_and_connect |= (gai_11_32_pit_pump_mode & (BIT0+BIT1));
      break;

    case SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_2:
      gai_11_32_pit_pump_mode &= ~(BIT2+BIT3);
      switch (mp_operation_mode_actual_pump_2->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT2; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT3; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT2+BIT3); break;
      }
      gai_11_106_p2_mode_and_connect &= ~(BIT0+BIT1);
      gai_11_106_p2_mode_and_connect |= ((gai_11_32_pit_pump_mode & (BIT2+BIT3)) >> 2);
      break;

    case SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_3:
      gai_11_32_pit_pump_mode &= ~(BIT4+BIT5);
      switch (mp_operation_mode_actual_pump_3->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT4; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT5; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT4+BIT5); break;
      }
      gai_11_136_p3_mode_and_connect &= ~(BIT0+BIT1);
      gai_11_136_p3_mode_and_connect |= ((gai_11_32_pit_pump_mode & (BIT4+BIT5)) >> 4);
      break;

    case SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_4:
      gai_11_32_pit_pump_mode &= ~(BIT6+BIT7);
      switch (mp_operation_mode_actual_pump_4->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT6; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT7; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT6+BIT7); break;
      }
      gai_11_166_p4_mode_and_connect &= ~(BIT0+BIT1);
      gai_11_166_p4_mode_and_connect |= ((gai_11_32_pit_pump_mode & (BIT6+BIT7)) >> 6);
      break;

    case SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_5:
      gai_11_32_pit_pump_mode &= ~(BIT8+BIT9);
      switch (mp_operation_mode_actual_pump_5->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT8; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT9; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT8+BIT9); break;
      }
      gai_11_196_p5_mode_and_connect &= ~(BIT0+BIT1);
      gai_11_196_p5_mode_and_connect |= ((gai_11_32_pit_pump_mode & (BIT8+BIT9)) >> 8);
      break;

    case SUBJECT_ID_OPERATION_MODE_ACTUAL_PUMP_6:
      gai_11_32_pit_pump_mode &= ~(BIT10+BIT11);
      switch (mp_operation_mode_actual_pump_6->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT10; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT11; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT10+BIT11); break;
      }
      gai_11_226_p6_mode_and_connect &= ~(BIT0+BIT1);
      gai_11_226_p6_mode_and_connect |= ((gai_11_32_pit_pump_mode & (BIT10+BIT11)) >> 10);
      break;

    case SUBJECT_ID_MIXER_OPERATING_MODE:
      gai_11_32_pit_pump_mode &= ~(BIT12+BIT13);
      switch (mp_mixer_operating_mode->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT12; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT13; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT12+BIT13); break;
      }
      break;

    case SUBJECT_ID_DOSING_PUMP_OPERATING_MODE:
      gai_11_32_pit_pump_mode &= ~(BIT14+BIT15);
      switch (mp_dosing_pump_operating_mode->GetValue())
      {
        case ACTUAL_OPERATION_MODE_STARTED:       gai_11_32_pit_pump_mode |= 0; break;
        case ACTUAL_OPERATION_MODE_STOPPED:       gai_11_32_pit_pump_mode |= BIT14; break;
        case ACTUAL_OPERATION_MODE_DISABLED:      gai_11_32_pit_pump_mode |= BIT15; break;
        case ACTUAL_OPERATION_MODE_NOT_INSTALLED: gai_11_32_pit_pump_mode |= (BIT14+BIT15); break;
      }
      break;

    case SUBJECT_ID_RELAY_FUNC_OUTPUT_PUMP_1:
    case SUBJECT_ID_VFD_1_INSTALLED:
      gai_11_35_pit_pump_conn_type &= ~(BIT0+BIT1);
      gai_11_35_pit_pump_conn_type |= BIT0*(mp_relay_func_output_pump_1->GetValue() > 2); // Relay number > 2 is on IO351
      gai_11_35_pit_pump_conn_type |= BIT1*(mp_vfd_1_installed->GetValue() );
      gai_11_76_p1_mode_and_connect &= ~(BIT4+BIT5);
      gai_11_76_p1_mode_and_connect |= ((gai_11_35_pit_pump_conn_type&(BIT0+BIT1))<<0);
      break;
    case SUBJECT_ID_RELAY_FUNC_OUTPUT_PUMP_2:
    case SUBJECT_ID_VFD_2_INSTALLED:
      gai_11_35_pit_pump_conn_type &= ~(BIT2+BIT3);
      gai_11_35_pit_pump_conn_type |= BIT2*(mp_relay_func_output_pump_2->GetValue() > 2); // Relay number > 2 is on IO351
      gai_11_35_pit_pump_conn_type |= BIT3*(mp_vfd_2_installed->GetValue() );
      gai_11_106_p2_mode_and_connect &= ~(BIT4+BIT5);
      gai_11_106_p2_mode_and_connect |= ((gai_11_35_pit_pump_conn_type&(BIT2+BIT3))<<2);
      break;

    case SUBJECT_ID_RELAY_FUNC_OUTPUT_PUMP_3:
    case SUBJECT_ID_VFD_3_INSTALLED:
      gai_11_35_pit_pump_conn_type &= ~(BIT4+BIT5);
      gai_11_35_pit_pump_conn_type |= BIT4*(mp_relay_func_output_pump_3->GetValue() > 2); // Relay number > 2 is on IO351
      gai_11_35_pit_pump_conn_type |= BIT5*(mp_vfd_3_installed->GetValue() );
      gai_11_136_p3_mode_and_connect &= ~(BIT4+BIT5);
      gai_11_136_p3_mode_and_connect |= ((gai_11_35_pit_pump_conn_type&(BIT4+BIT5))<<4);
      break;

    case SUBJECT_ID_RELAY_FUNC_OUTPUT_PUMP_4:
    case SUBJECT_ID_VFD_4_INSTALLED:
      gai_11_35_pit_pump_conn_type &= ~(BIT6+BIT7);
      gai_11_35_pit_pump_conn_type |= BIT6*(mp_relay_func_output_pump_4->GetValue() > 2); // Relay number > 2 is on IO351
      gai_11_35_pit_pump_conn_type |= BIT7*(mp_vfd_4_installed->GetValue() );
      gai_11_166_p4_mode_and_connect &= ~(BIT4+BIT5);
      gai_11_166_p4_mode_and_connect |= ((gai_11_35_pit_pump_conn_type&(BIT6+BIT7))<<6);
      break;

    case SUBJECT_ID_RELAY_FUNC_OUTPUT_PUMP_5:
    case SUBJECT_ID_VFD_5_INSTALLED:
      gai_11_35_pit_pump_conn_type &= ~(BIT8+BIT9);
      gai_11_35_pit_pump_conn_type |= BIT8*(mp_relay_func_output_pump_5->GetValue() > 2); // Relay number > 2 is on IO351
      gai_11_35_pit_pump_conn_type |= BIT9*(mp_vfd_5_installed->GetValue() );
      gai_11_196_p5_mode_and_connect &= ~(BIT4+BIT5);
      gai_11_196_p5_mode_and_connect |= ((gai_11_35_pit_pump_conn_type&(BIT8+BIT9))<<8);
      break;

    case SUBJECT_ID_RELAY_FUNC_OUTPUT_PUMP_6:
    case SUBJECT_ID_VFD_6_INSTALLED:
      gai_11_35_pit_pump_conn_type &= ~(BIT10+BIT11);
      gai_11_35_pit_pump_conn_type |= BIT10*(mp_relay_func_output_pump_6->GetValue() > 2); // Relay number > 2 is on IO351
      gai_11_35_pit_pump_conn_type |= BIT11*(mp_vfd_6_installed->GetValue() );
      gai_11_226_p6_mode_and_connect &= ~(BIT4+BIT5);
      gai_11_226_p6_mode_and_connect |= ((gai_11_35_pit_pump_conn_type&(BIT10+BIT11))<<10);
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_MIXER:
      gai_11_35_pit_pump_conn_type &= ~(BIT12+BIT13);
      gai_11_35_pit_pump_conn_type |= BIT12*(mp_relay_status_relay_func_mixer->GetValue() > 2); // Relay number > 2 is on IO351

    case SUBJECT_ID_DOSING_PUMP_TYPE:
      gai_11_35_pit_pump_conn_type &= ~(BIT14+BIT15);
      gai_11_35_pit_pump_conn_type |= BIT14*(mp_dosing_pump_type->GetValue() == DOSING_PUMP_TYPE_ANALOG);
      gai_11_35_pit_pump_conn_type |= (BIT14+BIT15)*(mp_dosing_pump_type->GetValue() == DOSING_PUMP_TYPE_DDA);

    case SUBJECT_ID_PUMP_1_IO111_DEVICE_STATUS:
    case SUBJECT_ID_PUMP_2_IO111_DEVICE_STATUS:
    case SUBJECT_ID_PUMP_3_IO111_DEVICE_STATUS:
    case SUBJECT_ID_PUMP_4_IO111_DEVICE_STATUS: 
    case SUBJECT_ID_PUMP_5_IO111_DEVICE_STATUS:
    case SUBJECT_ID_PUMP_6_IO111_DEVICE_STATUS: 
      if (mp_service_mode_enabled->GetValue())      //Service mode enabled, clear alarms
      {
        UpdateIo111DeviceStatus(true);
      }
      else
      {
        UpdateIo111DeviceStatus(false);
      }
      break;

    case SUBJECT_ID_MP204_1_DEVICE_STATUS:
    case SUBJECT_ID_MP204_2_DEVICE_STATUS:
    case SUBJECT_ID_MP204_3_DEVICE_STATUS:
    case SUBJECT_ID_MP204_4_DEVICE_STATUS:
    case SUBJECT_ID_MP204_5_DEVICE_STATUS:
    case SUBJECT_ID_MP204_6_DEVICE_STATUS:  
      if (mp_service_mode_enabled->GetValue())      //Service mode enabled, clear alarms
      {
        UpdateMp204DeviceStatus(true);
      }
      else
      {
        UpdateMp204DeviceStatus(false);
      }
      break;

    case SUBJECT_ID_CUE_PUMP_1_DEVICE_STATUS:
    case SUBJECT_ID_CUE_PUMP_2_DEVICE_STATUS:
    case SUBJECT_ID_CUE_PUMP_3_DEVICE_STATUS:
    case SUBJECT_ID_CUE_PUMP_4_DEVICE_STATUS:
    case SUBJECT_ID_CUE_PUMP_5_DEVICE_STATUS:
    case SUBJECT_ID_CUE_PUMP_6_DEVICE_STATUS:
      if (mp_service_mode_enabled->GetValue())      //Service mode enabled, clear alarms
      {
        UpdateCueDeviceStatus(true);
      }
      else
      {
        UpdateCueDeviceStatus(false);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_1:
      if (mp_relay_status_relay_func_user_io_1->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_1->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT0;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT0);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_2:
      if (mp_relay_status_relay_func_user_io_2->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_2->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT1;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT1);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_3:
      if (mp_relay_status_relay_func_user_io_3->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_3->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT2;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT2);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_4:
      if (mp_relay_status_relay_func_user_io_4->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_4->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT3;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT3);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_5:
      if (mp_relay_status_relay_func_user_io_5->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_5->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT4;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT4);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_6:
      if (mp_relay_status_relay_func_user_io_6->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_6->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT5;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT5);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_7:
      if (mp_relay_status_relay_func_user_io_7->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_7->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT6;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT6);
      }
      break;

    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_8:
      if (mp_relay_status_relay_func_user_io_8->GetQuality() == DP_AVAILABLE && mp_relay_status_relay_func_user_io_8->GetAsBool())
      {
        gai_2_77_io_logic_outputs |=   BIT7;
      }
      else
      {
        gai_2_77_io_logic_outputs &= ~(BIT7);
      }
      break;

    case SUBJECT_ID_LOWEST_START_LEVEL_VARIATION_ENABLED:
    case SUBJECT_ID_LOWEST_START_LEVEL_VARIATION:
      if (mp_lowest_start_level_variation_enabled->GetAsBool())
      {
        gai_11_48_pit_rand_start_level_band = ToGeni16bitValue(mp_lowest_start_level_variation.GetSubject(), GENI_CONVERT_ID_HEAD_DIST_DOT01M);
      }
      else
      {
        gai_11_48_pit_rand_start_level_band = 0;
      }
      break;

    case SUBJECT_ID_PUMP_GROUP_1_ALARM_MOTOR_CURRENT_OVERLOAD_ALARM_CONF:      
        gai_12_21_current_max_alarm_group1 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_1_alarm_motor_current_overload_alarm_conf.GetSubject()->GetAlarmLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);
        gai_12_23_current_max_warn_group1 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_1_alarm_motor_current_overload_alarm_conf.GetSubject()->GetWarningLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);
		break;

    case SUBJECT_ID_PUMP_GROUP_1_ALARM_MOTOR_CURRENT_UNDERLOAD_ALARM_CONF:      
        gai_12_22_current_min_alarm_group1 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_1_alarm_motor_current_underload_alarm_conf.GetSubject()->GetAlarmLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);
		    gai_12_24_current_min_warn_group1 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_1_alarm_motor_current_underload_alarm_conf.GetSubject()->GetWarningLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);

		break;

    case SUBJECT_ID_PUMP_GROUP_2_ALARM_MOTOR_CURRENT_OVERLOAD_ALARM_CONF:      
        gai_12_25_current_max_alarm_group2 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_2_alarm_motor_current_overload_alarm_conf.GetSubject()->GetAlarmLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);
        gai_12_27_current_max_warn_group2 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_2_alarm_motor_current_overload_alarm_conf.GetSubject()->GetWarningLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);
		break;

    case SUBJECT_ID_PUMP_GROUP_2_ALARM_MOTOR_CURRENT_UNDERLOAD_ALARM_CONF:      
        gai_12_26_current_min_alarm_group2 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_2_alarm_motor_current_underload_alarm_conf.GetSubject()->GetAlarmLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);
     		gai_12_28_current_min_warn_group2 = ToGeni16bitValue(dynamic_cast<FloatDataPoint*>(mp_pump_group_2_alarm_motor_current_underload_alarm_conf.GetSubject()->GetWarningLimit()),
                                                              GENI_CONVERT_ID_CURRENT_DOT1A);

		break;

    case SUBJECT_ID_SERVICE_MODE_ENABLED:  
      UpdateServiceMode();
      break;

    default:
      subject_handled = false;
      break;
  }

  return subject_handled;
}

/*****************************************************************************
 * Function UpdateServiceMode
 * DESCRIPTION: 
 *****************************************************************************/
void GeniAppIf::UpdateServiceMode()
{
  if (mp_service_mode_enabled->GetValue())      //Service mode enabled, clear alarms
  {
    UpdateMp204DeviceStatus(true);
    UpdateCueDeviceStatus(true);
    UpdateIo111DeviceStatus(true);
  }
  else                                          //Service mode disabled, update alarms
  {
    UpdateMp204DeviceStatus(false);
    UpdateCueDeviceStatus(false);
    UpdateIo111DeviceStatus(false);    
  }
}


/*****************************************************************************
 * Function UpdateMp204DeviceStatus
 * DESCRIPTION: 
 *****************************************************************************/
void GeniAppIf::UpdateMp204DeviceStatus(bool bServiceMode)
{
  if (bServiceMode)
  {
    //Service mode enabled, clear alarms
    gai_11_76_p1_mode_and_connect  &= ~(BIT11+BIT12);
    if (mp_mp204_1_device_status->GetValue())
      gai_11_76_p1_mode_and_connect  |= (BIT11* BIT0);

    gai_11_106_p2_mode_and_connect  &= ~(BIT11+BIT12);
    if (mp_mp204_2_device_status->GetValue())
      gai_11_106_p2_mode_and_connect  |= (BIT11* BIT0);

    gai_11_136_p3_mode_and_connect  &= ~(BIT11+BIT12);
    if (mp_mp204_3_device_status->GetValue())
      gai_11_136_p3_mode_and_connect  |= (BIT11* BIT0);

    gai_11_166_p4_mode_and_connect  &= ~(BIT11+BIT12);
    if (mp_mp204_4_device_status->GetValue())
      gai_11_166_p4_mode_and_connect  |= (BIT11* BIT0);

    gai_11_196_p5_mode_and_connect  &= ~(BIT11+BIT12);
    if (mp_mp204_5_device_status->GetValue())
      gai_11_196_p5_mode_and_connect  |= (BIT11* BIT0);

    gai_11_226_p6_mode_and_connect  &= ~(BIT11+BIT12);
    if (mp_mp204_6_device_status->GetValue())
      gai_11_226_p6_mode_and_connect  |= (BIT11* BIT0);
  }
  else
  {
    //Service mode disabled, update alarms
    gai_11_76_p1_mode_and_connect  &= ~(BIT11+BIT12);
    gai_11_76_p1_mode_and_connect  |= BIT11*(mp_mp204_1_device_status->GetValue() & (BIT0+BIT1));

    gai_11_106_p2_mode_and_connect &= ~(BIT11+BIT12);
    gai_11_106_p2_mode_and_connect |= BIT11*(mp_mp204_2_device_status->GetValue() & (BIT0+BIT1));

    gai_11_136_p3_mode_and_connect &= ~(BIT11+BIT12);
    gai_11_136_p3_mode_and_connect |= BIT11*(mp_mp204_3_device_status->GetValue() & (BIT0+BIT1));
  
    gai_11_166_p4_mode_and_connect &= ~(BIT11+BIT12);
    gai_11_166_p4_mode_and_connect |= BIT11*(mp_mp204_4_device_status->GetValue() & (BIT0+BIT1));
 
    gai_11_196_p5_mode_and_connect &= ~(BIT11+BIT12);
    gai_11_196_p5_mode_and_connect |= BIT11*(mp_mp204_5_device_status->GetValue() & (BIT0+BIT1));
  
    gai_11_226_p6_mode_and_connect &= ~(BIT11+BIT12);
    gai_11_226_p6_mode_and_connect |= BIT11*(mp_mp204_6_device_status->GetValue() & (BIT0+BIT1));    
  }
}

/*****************************************************************************
 * Function UpdateCueDeviceStatus
 * DESCRIPTION: 
 *****************************************************************************/
void GeniAppIf::UpdateCueDeviceStatus(bool bServiceMode)
{
  if (bServiceMode)
  {
    //Service mode enabled, clear alarms
    gai_11_76_p1_mode_and_connect  &= ~(BIT13+BIT14);
    if (mp_cue_pump_1_device_status->GetValue())
      gai_11_76_p1_mode_and_connect  |= (BIT13* BIT0);

    gai_11_106_p2_mode_and_connect  &= ~(BIT13+BIT14);
    if (mp_cue_pump_2_device_status->GetValue())
      gai_11_106_p2_mode_and_connect  |= (BIT13* BIT0);

    gai_11_136_p3_mode_and_connect  &= ~(BIT13+BIT14);
    if (mp_cue_pump_3_device_status->GetValue())
      gai_11_136_p3_mode_and_connect  |= (BIT13* BIT0);

    gai_11_166_p4_mode_and_connect  &= ~(BIT13+BIT14);
    if (mp_cue_pump_4_device_status->GetValue())
      gai_11_166_p4_mode_and_connect  |= (BIT13* BIT0);

    gai_11_196_p5_mode_and_connect  &= ~(BIT13+BIT14);
    if (mp_cue_pump_5_device_status->GetValue())
      gai_11_196_p5_mode_and_connect  |= (BIT13* BIT0);

    gai_11_226_p6_mode_and_connect  &= ~(BIT13+BIT14);
    if (mp_cue_pump_6_device_status->GetValue())
      gai_11_226_p6_mode_and_connect  |= (BIT13* BIT0);
  }
  else
  {
    //Service mode disabled, update alarms
    gai_11_76_p1_mode_and_connect &= ~(BIT13+BIT14);
    gai_11_76_p1_mode_and_connect |= BIT13*(mp_cue_pump_1_device_status->GetValue() & (BIT0+BIT1));

    gai_11_106_p2_mode_and_connect &= ~(BIT13+BIT14);
    gai_11_106_p2_mode_and_connect |= BIT13*(mp_cue_pump_2_device_status->GetValue() & (BIT0+BIT1));

    gai_11_136_p3_mode_and_connect &= ~(BIT13+BIT14);
    gai_11_136_p3_mode_and_connect |= BIT13*(mp_cue_pump_3_device_status->GetValue() & (BIT0+BIT1));
  
    gai_11_166_p4_mode_and_connect &= ~(BIT13+BIT14);
    gai_11_166_p4_mode_and_connect |= BIT13*(mp_cue_pump_4_device_status->GetValue() & (BIT0+BIT1));
  
    gai_11_196_p5_mode_and_connect &= ~(BIT13+BIT14);
    gai_11_196_p5_mode_and_connect |= BIT13*(mp_cue_pump_5_device_status->GetValue() & (BIT0+BIT1));
  
    gai_11_226_p6_mode_and_connect &= ~(BIT13+BIT14);
    gai_11_226_p6_mode_and_connect |= BIT13*(mp_cue_pump_6_device_status->GetValue() & (BIT0+BIT1));
  }
}

/*****************************************************************************
 * Function UpdateIo111DeviceStatus
 * DESCRIPTION: 
 *****************************************************************************/
void GeniAppIf::UpdateIo111DeviceStatus(bool bServiceMode)
{
  if (bServiceMode)
  {
    //Service mode enabled, clear alarms
    gai_11_76_p1_mode_and_connect  &= ~(BIT9+BIT10);
    if (mp_pump_1_io111_device_status->GetValue())
      gai_11_76_p1_mode_and_connect  |= (BIT9* BIT0);

    gai_11_106_p2_mode_and_connect  &= ~(BIT9+BIT10);
    if (mp_pump_2_io111_device_status->GetValue())
      gai_11_106_p2_mode_and_connect  |= (BIT9* BIT0);

    gai_11_136_p3_mode_and_connect  &= ~(BIT9+BIT10);
    if (mp_pump_3_io111_device_status->GetValue())
      gai_11_136_p3_mode_and_connect  |= (BIT9* BIT0);

    gai_11_166_p4_mode_and_connect  &= ~(BIT9+BIT10);
    if (mp_pump_4_io111_device_status->GetValue())
      gai_11_166_p4_mode_and_connect  |= (BIT9* BIT0);

    gai_11_196_p5_mode_and_connect  &= ~(BIT9+BIT10);
    if (mp_pump_5_io111_device_status->GetValue())
      gai_11_196_p5_mode_and_connect  |= (BIT9* BIT0);

    gai_11_226_p6_mode_and_connect  &= ~(BIT9+BIT10);
    if (mp_pump_6_io111_device_status->GetValue())
      gai_11_226_p6_mode_and_connect  |= (BIT9* BIT0);
  }
  else
  {
    //Service mode disabled, update alarms
    gai_11_76_p1_mode_and_connect &= ~(BIT9+BIT10);
    gai_11_76_p1_mode_and_connect |= BIT9*(mp_pump_1_io111_device_status->GetValue() & (BIT0+BIT1));

    gai_11_106_p2_mode_and_connect &= ~(BIT9+BIT10);
    gai_11_106_p2_mode_and_connect |= BIT9*(mp_pump_2_io111_device_status->GetValue() & (BIT0+BIT1));

    gai_11_136_p3_mode_and_connect &= ~(BIT9+BIT10);
    gai_11_136_p3_mode_and_connect |= BIT9*(mp_pump_3_io111_device_status->GetValue() & (BIT0+BIT1));

    gai_11_166_p4_mode_and_connect &= ~(BIT9+BIT10);
    gai_11_166_p4_mode_and_connect |= BIT9*(mp_pump_4_io111_device_status->GetValue() & (BIT0+BIT1));

    gai_11_196_p5_mode_and_connect &= ~(BIT9+BIT10);
    gai_11_196_p5_mode_and_connect |= BIT9*(mp_pump_5_io111_device_status->GetValue() & (BIT0+BIT1));

    gai_11_226_p6_mode_and_connect &= ~(BIT9+BIT10);
    gai_11_226_p6_mode_and_connect |= BIT9*(mp_pump_6_io111_device_status->GetValue() & (BIT0+BIT1));
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
