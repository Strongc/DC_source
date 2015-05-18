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
/* CLASS NAME       : RelayFuncHandler                                      */
/*                                                                          */
/* FILE NAME        : RelayFuncHandler.cpp                                  */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <microp.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <FactoryTypes.h>
#include <IobComDrv.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <RelayFuncHandler.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define NO_OF_RELAYS_IOB      2 // IOB == I/O relays on board

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

RelayFuncHandler* RelayFuncHandler::mpInstance = 0;

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 ****************************************************************************/
RelayFuncHandler* RelayFuncHandler::GetInstance()
{
  if (!mpInstance)
  {
    mpInstance = new RelayFuncHandler();
  }
  return mpInstance;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION: Initialise RelayFuncHandler sub task
 *****************************************************************************/
void RelayFuncHandler::InitSubTask(void)
{
  int i;

  /* Update configuration at first run */
  for (i = 0; i < NO_OF_RELAYS; i++)
  {
    mpConfRelayFunc[i].SetUpdated();
  }

  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void RelayFuncHandler::RunSubTask(void)
{
  int i, j;
  bool relay_state;
  bool relay_configuration_changed = false;
  bool relay_func_found = false;
  bool io351_relay_status_changed = false;

  // check for new configuration(s) of relays
  for (i = 0; i < NO_OF_RELAYS; i++)
  {
    if (mpConfRelayFunc[i].IsUpdated())
    {
      for (j = 0; j < NO_OF_RELAY_FUNC; j++)
      {
        if (mpConfRelayFunc[i]->GetValue() == (RELAY_FUNC_TYPE)j)
        {
          // force a change at the relay from the newly configured function
          mpRelayStatus[j].SetUpdated();
        }
      }
      relay_configuration_changed = true;
    }
  }

  if (relay_configuration_changed)
  {
    if (mpRelayConfigurationChanged.IsValid())
    {
      mpRelayConfigurationChanged->SetEvent();
    }
    for (j = 0; j < NO_OF_RELAY_FUNC; j++)
    {
      relay_func_found = false;
      for (i = 0; i < NO_OF_RELAYS; i++)
      {
        if (mpConfRelayFunc[i]->GetValue() == (RELAY_FUNC_TYPE)j)
        {
          // Reconnect the relay numbers to relay functions
          mpRelayFuncOutput[j]->SetValue(i+1); // output numbers begins at '1' while index begins at '0'
          relay_func_found = true;
        }
      }
      if ( relay_func_found == false)
      {
        // Relay func is not connected to an output - set it to "not used" (0)
        mpRelayFuncOutput[j]->SetValue(0);
      }
    }
  }

  // set relays to output from the relay funcs
  for (i = 0; i < NO_OF_RELAY_FUNC; i++)
  {
    if (mpRelayStatus[i].IsUpdated())
    {
      relay_state = mpRelayStatus[i]->GetValue();
      for (j = 0; j < NO_OF_RELAYS; j++)
      {
        if (mpConfRelayFunc[j]->GetValue() == (RELAY_FUNC_TYPE)i)
        {
          if (relay_state)
          {
            SET_BIT_HIGH(mRelayBuffer, j);
          }
          else
          {
            SET_BIT_LOW(mRelayBuffer, j);
          }

          if (j < NO_OF_RELAYS_IOB)
          {
            SetIOBRelayState(j, relay_state);
          }
          else
          {
            io351_relay_status_changed = true;
          }
        }
      }
    }
  }

  // update IO351 IO module digital output status if relay status has changed
  if (io351_relay_status_changed)
  {
    mpIO351DigOutStatusBits[0]->SetValue((U16)((mRelayBuffer >> 2) & 0x007F));  // we know there are 7 digital outputs on IO351
    mpIO351DigOutStatusBits[1]->SetValue((U16)((mRelayBuffer >> 9) & 0x007F));
    mpIO351DigOutStatusBits[2]->SetValue((U16)((mRelayBuffer >> 16) & 0x007F));
  }

  // update digital output status object
  mpDigOutStatusBits->SetValue(mRelayBuffer);

  for (i=0; i<NO_OF_RELAYS; i++)
  {
    mpDigOutState[i]->SetValue(TEST_BIT_HIGH(mRelayBuffer, i));
  }
}

/*****************************************************************************
 * Function - IsFuncConfiged
 * DESCRIPTION: Checks if relayFunc is configed
 *
 *****************************************************************************/
bool RelayFuncHandler::IsFuncConfiged(RELAY_FUNC_TYPE relayFunc)
{
  for (int i = 0; i < NO_OF_RELAYS; i++)
  {
    if (mpConfRelayFunc[i]->GetValue() == relayFunc)
    {
      return true;
    }
  }

  return false;
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void RelayFuncHandler::Update(Subject* pSubject)
{
  switch (pSubject->GetSubjectId())
  {
    // relay status
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_NO_FUNCTION:
      mpRelayStatus[RELAY_FUNC_NO_FUNCTION].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_SYSTEM_READY:
      mpRelayStatus[RELAY_FUNC_SYSTEM_READY].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_SYSTEM_OPERATION:
      mpRelayStatus[RELAY_FUNC_SYSTEM_OPERATION].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_SYSTEM_WARNING:
      mpRelayStatus[RELAY_FUNC_SYSTEM_WARNING].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_SYSTEM_ALARM:
      mpRelayStatus[RELAY_FUNC_SYSTEM_ALARM].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_PUMP_1:
      mpRelayStatus[RELAY_FUNC_PUMP_1].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_PUMP_2:
      mpRelayStatus[RELAY_FUNC_PUMP_2].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_PUMP_3:
      mpRelayStatus[RELAY_FUNC_PUMP_3].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_PUMP_4:
      mpRelayStatus[RELAY_FUNC_PUMP_4].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_PUMP_5:
      mpRelayStatus[RELAY_FUNC_PUMP_5].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_PUMP_6:
      mpRelayStatus[RELAY_FUNC_PUMP_6].Update(pSubject);
      break;
    case SUBJECT_ID_MIXER_RELAY:
      mpRelayStatus[RELAY_FUNC_MIXER].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_RELAY_CUSTOM:
      mpRelayStatus[RELAY_FUNC_RELAY_CUSTOM].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_ALARM_RELAY_CUSTOM:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_CUSTOM].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_ALARM_RELAY_CRITICAL:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_CRITICAL].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_ALARM_RELAY_ALL_ALARMS:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_ALL_ALARMS].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_1:
      mpRelayStatus[RELAY_FUNC_USER_IO_1].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_2:
      mpRelayStatus[RELAY_FUNC_USER_IO_2].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_3:
      mpRelayStatus[RELAY_FUNC_USER_IO_3].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_4:
      mpRelayStatus[RELAY_FUNC_USER_IO_4].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_5:
      mpRelayStatus[RELAY_FUNC_USER_IO_5].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_6:
      mpRelayStatus[RELAY_FUNC_USER_IO_6].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_7:
      mpRelayStatus[RELAY_FUNC_USER_IO_7].Update(pSubject);
      break;
    case SUBJECT_ID_RELAY_STATUS_RELAY_FUNC_USER_IO_8:
      mpRelayStatus[RELAY_FUNC_USER_IO_8].Update(pSubject);
      break;
    case SUBJECT_ID_VFD_1_RELAY_STATUS_RELAY_FUNC_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_1_REVERSE].Update(pSubject);
      break;
    case SUBJECT_ID_VFD_2_RELAY_STATUS_RELAY_FUNC_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_2_REVERSE].Update(pSubject);
      break;
    case SUBJECT_ID_VFD_3_RELAY_STATUS_RELAY_FUNC_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_3_REVERSE].Update(pSubject);
      break;
    case SUBJECT_ID_VFD_4_RELAY_STATUS_RELAY_FUNC_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_4_REVERSE].Update(pSubject);
      break;
    case SUBJECT_ID_VFD_5_RELAY_STATUS_RELAY_FUNC_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_5_REVERSE].Update(pSubject);
      break;
    case SUBJECT_ID_VFD_6_RELAY_STATUS_RELAY_FUNC_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_6_REVERSE].Update(pSubject);
      break;

      // relay func conf
    case SUBJECT_ID_DIG_OUT_1_CONF_RELAY_FUNC:
      mpConfRelayFunc[0].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_2_CONF_RELAY_FUNC:
      mpConfRelayFunc[1].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_3_CONF_RELAY_FUNC:
      mpConfRelayFunc[2].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_4_CONF_RELAY_FUNC:
      mpConfRelayFunc[3].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_5_CONF_RELAY_FUNC:
      mpConfRelayFunc[4].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_6_CONF_RELAY_FUNC:
      mpConfRelayFunc[5].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_7_CONF_RELAY_FUNC:
      mpConfRelayFunc[6].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_8_CONF_RELAY_FUNC:
      mpConfRelayFunc[7].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_9_CONF_RELAY_FUNC:
      mpConfRelayFunc[8].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_10_CONF_RELAY_FUNC:
      mpConfRelayFunc[9].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_11_CONF_RELAY_FUNC:
      mpConfRelayFunc[10].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_12_CONF_RELAY_FUNC:
      mpConfRelayFunc[11].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_13_CONF_RELAY_FUNC:
      mpConfRelayFunc[12].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_14_CONF_RELAY_FUNC:
      mpConfRelayFunc[13].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_15_CONF_RELAY_FUNC:
      mpConfRelayFunc[14].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_16_CONF_RELAY_FUNC:
      mpConfRelayFunc[15].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_17_CONF_RELAY_FUNC:
      mpConfRelayFunc[16].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_18_CONF_RELAY_FUNC:
      mpConfRelayFunc[17].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_19_CONF_RELAY_FUNC:
      mpConfRelayFunc[18].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_20_CONF_RELAY_FUNC:
      mpConfRelayFunc[19].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_21_CONF_RELAY_FUNC:
      mpConfRelayFunc[20].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_22_CONF_RELAY_FUNC:
      mpConfRelayFunc[21].Update(pSubject);
      break;
    case SUBJECT_ID_DIG_OUT_23_CONF_RELAY_FUNC:
      mpConfRelayFunc[22].Update(pSubject);
      break;

      // IO351 IO module digital out status bits
    case SUBJECT_ID_IO351_IO_MODULE_1_DIG_OUT_STATUS_BITS:
      mpIO351DigOutStatusBits[0].Update(pSubject);
      break;
    case SUBJECT_ID_IO351_IO_MODULE_2_DIG_OUT_STATUS_BITS:
      mpIO351DigOutStatusBits[1].Update(pSubject);
      break;
    case SUBJECT_ID_IO351_IO_MODULE_3_DIG_OUT_STATUS_BITS:
      mpIO351DigOutStatusBits[2].Update(pSubject);
      break;
  }

  ReqTaskTime();
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void RelayFuncHandler::SubscribtionCancelled(Subject* pSubject)
{
  /* Nothing to do because subjects are never destroyed */
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void RelayFuncHandler::ConnectToSubjects(void)
{
  int i;

  for (i = 0; i < NO_OF_RELAYS; i++)
  {
     mpConfRelayFunc[i]->Subscribe(this);
  }

  for (i = 0; i < NO_OF_RELAY_FUNC; i++)
  {
     mpRelayStatus[i]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void RelayFuncHandler::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    /* Relay Function State and output Objects, one of each per relay function */
    case SP_RFH_RELAY_FUNC_NO_FUNCTION:
      mpRelayStatus[RELAY_FUNC_NO_FUNCTION].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_NO_FUNCTION:
      mpRelayFuncOutput[RELAY_FUNC_NO_FUNCTION].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_SYSTEM_READY:
      mpRelayStatus[RELAY_FUNC_SYSTEM_READY].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_SYSTEM_READY:
      mpRelayFuncOutput[RELAY_FUNC_SYSTEM_READY].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_SYSTEM_OPERATION:
      mpRelayStatus[RELAY_FUNC_SYSTEM_OPERATION].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_SYSTEM_OPERATION:
      mpRelayFuncOutput[RELAY_FUNC_SYSTEM_OPERATION].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_SYSTEM_WARNING:
      mpRelayStatus[RELAY_FUNC_SYSTEM_WARNING].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_SYSTEM_WARNING:
      mpRelayFuncOutput[RELAY_FUNC_SYSTEM_WARNING].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_SYSTEM_ALARM:
      mpRelayStatus[RELAY_FUNC_SYSTEM_ALARM].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_SYSTEM_ALARM:
      mpRelayFuncOutput[RELAY_FUNC_SYSTEM_ALARM].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_PUMP_1:
      mpRelayStatus[RELAY_FUNC_PUMP_1].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_PUMP_1:
      mpRelayFuncOutput[RELAY_FUNC_PUMP_1].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_PUMP_2:
      mpRelayStatus[RELAY_FUNC_PUMP_2].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_PUMP_2:
      mpRelayFuncOutput[RELAY_FUNC_PUMP_2].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_PUMP_3:
      mpRelayStatus[RELAY_FUNC_PUMP_3].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_PUMP_3:
      mpRelayFuncOutput[RELAY_FUNC_PUMP_3].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_PUMP_4:
      mpRelayStatus[RELAY_FUNC_PUMP_4].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_PUMP_4:
      mpRelayFuncOutput[RELAY_FUNC_PUMP_4].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_PUMP_5:
      mpRelayStatus[RELAY_FUNC_PUMP_5].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_PUMP_5:
      mpRelayFuncOutput[RELAY_FUNC_PUMP_5].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_PUMP_6:
      mpRelayStatus[RELAY_FUNC_PUMP_6].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_PUMP_6:
      mpRelayFuncOutput[RELAY_FUNC_PUMP_6].Attach(pSubject);
      break;

    case SP_RFH_RELAY_FUNC_MIXER:
      mpRelayStatus[RELAY_FUNC_MIXER].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_MIXER:
      mpRelayFuncOutput[RELAY_FUNC_MIXER].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_CUSTOM:
      mpRelayStatus[RELAY_FUNC_RELAY_CUSTOM].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_CUSTOM:
      mpRelayFuncOutput[RELAY_FUNC_RELAY_CUSTOM].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_ALARM_RELAY_CUSTOM:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_CUSTOM].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_ALARM_RELAY_CUSTOM:
      mpRelayFuncOutput[RELAY_FUNC_ALARM_RELAY_CUSTOM].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_ALARM_RELAY_HIGH_LEVEL:
      mpRelayFuncOutput[RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_ALARM_RELAY_CRITICAL:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_CRITICAL].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_ALARM_RELAY_CRITICAL:
      mpRelayFuncOutput[RELAY_FUNC_ALARM_RELAY_CRITICAL].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_ALARM_RELAY_ALL_ALARMS:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_ALL_ALARMS].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_ALARM_RELAY_ALL_ALARMS:
      mpRelayFuncOutput[RELAY_FUNC_ALARM_RELAY_ALL_ALARMS].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS:
      mpRelayStatus[RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS:
      mpRelayFuncOutput[RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_1:
      mpRelayStatus[RELAY_FUNC_USER_IO_1].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_1:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_1].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_2:
      mpRelayStatus[RELAY_FUNC_USER_IO_2].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_2:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_2].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_3:
      mpRelayStatus[RELAY_FUNC_USER_IO_3].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_3:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_3].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_4:
      mpRelayStatus[RELAY_FUNC_USER_IO_4].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_4:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_4].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_5:
      mpRelayStatus[RELAY_FUNC_USER_IO_5].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_5:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_5].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_6:
      mpRelayStatus[RELAY_FUNC_USER_IO_6].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_6:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_6].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_7:
      mpRelayStatus[RELAY_FUNC_USER_IO_7].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_7:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_7].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_RELAY_USER_IO_8:
      mpRelayStatus[RELAY_FUNC_USER_IO_8].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_RELAY_USER_IO_8:
      mpRelayFuncOutput[RELAY_FUNC_USER_IO_8].Attach(pSubject);
      break;

    case SP_RFH_RELAY_FUNC_VFD_1_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_1_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_VFD_1_REVERSE:
      mpRelayFuncOutput[RELAY_FUNC_VFD_1_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_VFD_2_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_2_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_VFD_2_REVERSE:
      mpRelayFuncOutput[RELAY_FUNC_VFD_2_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_VFD_3_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_3_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_VFD_3_REVERSE:
      mpRelayFuncOutput[RELAY_FUNC_VFD_3_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_VFD_4_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_4_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_VFD_4_REVERSE:
      mpRelayFuncOutput[RELAY_FUNC_VFD_4_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_VFD_5_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_5_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_VFD_5_REVERSE:
      mpRelayFuncOutput[RELAY_FUNC_VFD_5_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_VFD_6_REVERSE:
      mpRelayStatus[RELAY_FUNC_VFD_6_REVERSE].Attach(pSubject);
      break;
    case SP_RFH_RELAY_FUNC_OUTPUT_VFD_6_REVERSE:
      mpRelayFuncOutput[RELAY_FUNC_VFD_6_REVERSE].Attach(pSubject);
      break;


      /* Add Relay Function State and output Objects above */


      /* Relay Configuration Objects, one per relay */
    case SP_RFH_DIG_OUT_1_CONF_RELAY_FUNC:
      mpConfRelayFunc[0].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_2_CONF_RELAY_FUNC:
      mpConfRelayFunc[1].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_3_CONF_RELAY_FUNC:
      mpConfRelayFunc[2].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_4_CONF_RELAY_FUNC:
      mpConfRelayFunc[3].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_5_CONF_RELAY_FUNC:
      mpConfRelayFunc[4].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_6_CONF_RELAY_FUNC:
      mpConfRelayFunc[5].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_7_CONF_RELAY_FUNC:
      mpConfRelayFunc[6].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_8_CONF_RELAY_FUNC:
      mpConfRelayFunc[7].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_9_CONF_RELAY_FUNC:
      mpConfRelayFunc[8].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_10_CONF_RELAY_FUNC:
      mpConfRelayFunc[9].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_11_CONF_RELAY_FUNC:
      mpConfRelayFunc[10].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_12_CONF_RELAY_FUNC:
      mpConfRelayFunc[11].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_13_CONF_RELAY_FUNC:
      mpConfRelayFunc[12].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_14_CONF_RELAY_FUNC:
      mpConfRelayFunc[13].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_15_CONF_RELAY_FUNC:
      mpConfRelayFunc[14].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_16_CONF_RELAY_FUNC:
      mpConfRelayFunc[15].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_17_CONF_RELAY_FUNC:
      mpConfRelayFunc[16].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_18_CONF_RELAY_FUNC:
      mpConfRelayFunc[17].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_19_CONF_RELAY_FUNC:
      mpConfRelayFunc[18].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_20_CONF_RELAY_FUNC:
      mpConfRelayFunc[19].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_21_CONF_RELAY_FUNC:
      mpConfRelayFunc[20].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_22_CONF_RELAY_FUNC:
      mpConfRelayFunc[21].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_23_CONF_RELAY_FUNC:
      mpConfRelayFunc[22].Attach(pSubject);
      break;

      // IO351 IO module digital out status bits
    case SP_RFH_IO351_IO_MODULE_1_DIG_OUT_STATUS_BITS:
      mpIO351DigOutStatusBits[0].Attach(pSubject);
      break;
    case SP_RFH_IO351_IO_MODULE_2_DIG_OUT_STATUS_BITS:
      mpIO351DigOutStatusBits[1].Attach(pSubject);
      break;
    case SP_RFH_IO351_IO_MODULE_3_DIG_OUT_STATUS_BITS:
      mpIO351DigOutStatusBits[2].Attach(pSubject);
      break;

      /* misc */
    case SP_RFH_DIG_OUT_STATUS_BITS:
      mpDigOutStatusBits.Attach(pSubject);
      break;

      /* Relay state objects, one per relay */
    case SP_RFH_DIG_OUT_1_STATE:
      mpDigOutState[0].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_2_STATE:
      mpDigOutState[1].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_3_STATE:
      mpDigOutState[2].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_4_STATE:
      mpDigOutState[3].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_5_STATE:
      mpDigOutState[4].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_6_STATE:
      mpDigOutState[5].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_7_STATE:
      mpDigOutState[6].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_8_STATE:
      mpDigOutState[7].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_9_STATE:
      mpDigOutState[8].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_10_STATE:
      mpDigOutState[9].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_11_STATE:
      mpDigOutState[10].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_12_STATE:
      mpDigOutState[11].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_13_STATE:
      mpDigOutState[12].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_14_STATE:
      mpDigOutState[13].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_15_STATE:
      mpDigOutState[14].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_16_STATE:
      mpDigOutState[15].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_17_STATE:
      mpDigOutState[16].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_18_STATE:
      mpDigOutState[17].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_19_STATE:
      mpDigOutState[18].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_20_STATE:
      mpDigOutState[19].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_21_STATE:
      mpDigOutState[20].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_22_STATE:
      mpDigOutState[21].Attach(pSubject);
      break;
    case SP_RFH_DIG_OUT_23_STATE:
      mpDigOutState[22].Attach(pSubject);
      break;


    case SP_RFH_RELAY_CONFIGURATION_CHANGED:
      mpRelayConfigurationChanged.Attach(pSubject);
      break;

    default:
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
RelayFuncHandler::RelayFuncHandler()
{
  mRelayBuffer = 0;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
RelayFuncHandler::~RelayFuncHandler()
{
}

/*****************************************************************************
 * Function - SetIOBRelayState
 * DESCRIPTION:
 *
 *****************************************************************************/
bool RelayFuncHandler::SetIOBRelayState(int relayIdx, bool relayState)
{
  bool ret_value = false;
  IOB_DIG_OUT_NO_TYPE relay_location;

  if (relayIdx < NO_OF_RELAYS_IOB)
  {
    // relay located on CU351 (IOB)
    if (relayIdx == 0)
    {
      relay_location = IOB_DIG_OUT_NO_3;
    }
    else
    {
      relay_location = IOB_DIG_OUT_NO_4;
    }

    if (relayState)
    {
      IobComDrv::GetInstance()->SetDigitalOutput(relay_location);
    }
    else
    {
      IobComDrv::GetInstance()->ClearDigitalOutput(relay_location);
    }

    ret_value = true;
  }

  return ret_value;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
