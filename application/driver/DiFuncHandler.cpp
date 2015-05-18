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
/* CLASS NAME       : DiFuncHandler                                         */
/*                                                                          */
/* FILE NAME        : DiFuncHandler.cpp                                     */
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

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <DiFuncHandler.h>
#include <IobComDrv.h>


/*****************************************************************************
  DEFINES
 *****************************************************************************/

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

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
DiFuncHandler::DiFuncHandler()
{
  mDigInBuffer = 0;
  mOldDigInBuffer = 0;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
DiFuncHandler::~DiFuncHandler()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION: Initialise DiFuncHandler sub task
 *****************************************************************************/
void DiFuncHandler::InitSubTask(void)
{
  // update configuration at first run
  mDiConfChanged = true;

  // update digital input status bits from IO 351 IO modules at first run
  mpIO351DigInStatusBits[0].SetUpdated();
  mpIO351DigInStatusBits[1].SetUpdated();
  mpIO351DigInStatusBits[2].SetUpdated();

  // transfer the logic configuration from the integer variable to each input at first run
  // make sure that each input is set as not updated to avoid tranfer in both directions
  mpConfLogic.SetUpdated();
  mInputConfLogicChanged = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void DiFuncHandler::RunSubTask(void)
{
  U32 temp_conf_logic;
  int i, j;
  DIGITAL_INPUT_FUNC_TYPE di_func;
  bool di_func_used;
  bool di_func_set_active[NO_OF_DIGITAL_INPUT_FUNC];
  DIGITAL_INPUT_FUNC_STATE_TYPE di_func_state[NO_OF_DIGITAL_INPUT_FUNC];

  // Syncronize the InputConfLogic datapoints with the integer containing bit information
  // about all the InputConfLogic - in both directions
  if (mpConfLogic.IsUpdated())
  {
    temp_conf_logic = mpConfLogic->GetValue();
    for (i=0; i<MAX_NO_OF_DIG_INPUTS; i++)
    {
      mpInputConfLogic[i]->SetValue(TEST_BIT_HIGH(temp_conf_logic, i));
    }
  }
  if (mInputConfLogicChanged == true)
  {
    mInputConfLogicChanged = false;
    temp_conf_logic = 0;
    for (i=0; i<MAX_NO_OF_DIG_INPUTS; i++)
    {
      if (mpInputConfLogic[i]->GetValue() == true)
      {
        SET_BIT_HIGH(temp_conf_logic, i);
      }
    }
    mpConfLogic->SetValue(temp_conf_logic);
  }


  /* Initialise temporary digital input functions array of DIGITAL_INPUT_FUNC_STATE_TYPE.
   * Initialise digital input functions array of bool.
   * Used to perform OR function at multiple inputs using the same function
   * Indes 0 is the function "Not Used" and is a 0 pointer and may not be used */
  for (i = (FIRST_DIGITAL_INPUT_FUNC); i <= LAST_DIGITAL_INPUT_FUNC; i++)
  {
    di_func_state[i] = DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED;
    di_func_set_active[i] = false;
  }

  // get digital input status from CU351 (IOB)
  for (i = 0; i < NO_OF_DIG_INPUTS_IOB; i++)
  {
    if (IobComDrv::GetInstance()->TestDigitalInput((IOB_DIG_IN_NO_TYPE)i))
    {
      SET_BIT_HIGH(mDigInBuffer, i);
      mpDigInLevel[i]->SetValue(true);
    }
    else
    {
      SET_BIT_LOW(mDigInBuffer, i);
      mpDigInLevel[i]->SetValue(false);
    }
  }

  // get digital input status from I0351 IO Module 1
  if (mpIO351DigInStatusBits[0].IsUpdated())
  {
    mDigInBuffer &= ~0x00000FF8;
    if (mpIO351DigInStatusBits[0]->GetQuality() == DP_AVAILABLE)
    {
      mDigInBuffer |= (mpIO351DigInStatusBits[0]->GetValue() << 3);
      for (i = NO_OF_DIG_INPUTS_IOB; i < (NO_OF_DIG_INPUTS_IOB + NO_OF_DIG_INPUTS_IO351); i++ )
      {
        mpDigInLevel[i]->SetValue(TEST_BIT_HIGH(mDigInBuffer, i));
      }
    }
    else
    {
      for (i = NO_OF_DIG_INPUTS_IOB; i < (NO_OF_DIG_INPUTS_IOB + NO_OF_DIG_INPUTS_IO351); i++ )
      {
        mpDigInLevel[i]->SetQuality(mpIO351DigInStatusBits[0]->GetQuality());
      }
    }
  }

  // get digital input status from I0351 IO Module 2
  if (mpIO351DigInStatusBits[1].IsUpdated())
  {
    mDigInBuffer &= ~0x001FF000;
    if (mpIO351DigInStatusBits[1]->GetQuality() == DP_AVAILABLE)
    {
      mDigInBuffer |= (mpIO351DigInStatusBits[1]->GetValue() << 12);
      for (i = (NO_OF_DIG_INPUTS_IOB + NO_OF_DIG_INPUTS_IO351); i < MAX_NO_OF_DIG_INPUTS; i++ )
      {
        mpDigInLevel[i]->SetValue(TEST_BIT_HIGH(mDigInBuffer, i));
      }
    }
    else
    {
      for (i = (NO_OF_DIG_INPUTS_IOB + NO_OF_DIG_INPUTS_IO351); i < MAX_NO_OF_DIG_INPUTS; i++ )
      {
        mpDigInLevel[i]->SetQuality(mpIO351DigInStatusBits[1]->GetQuality());
      }
    }
  }

 // get digital input status from I0351 IO Module 3
  if (mpIO351DigInStatusBits[2].IsUpdated())
  {
    mDigInBuffer &= ~0xFFE00000;
    if (mpIO351DigInStatusBits[2]->GetQuality() == DP_AVAILABLE)
    {
      mDigInBuffer |= (mpIO351DigInStatusBits[2]->GetValue() << 21);
      for (i = (NO_OF_DIG_INPUTS_IOB + (NO_OF_DIG_INPUTS_IO351 * 2)); i < MAX_NO_OF_DIG_INPUTS; i++ )
      {
        mpDigInLevel[i]->SetValue(TEST_BIT_HIGH(mDigInBuffer, i));
      }
    }
    else
    {
      for (i = (NO_OF_DIG_INPUTS_IOB + (NO_OF_DIG_INPUTS_IO351 * 2)); i < MAX_NO_OF_DIG_INPUTS; i++ )
      {
        mpDigInLevel[i]->SetQuality(mpIO351DigInStatusBits[2]->GetQuality());
      }
    }
  }

  // make sure unused bits are set to binary 0
  mDigInBuffer &= 0x3FFFFFFF;

  // handle used digital input functions
  for (i = 0; i < MAX_NO_OF_DIG_INPUTS; i++)
  {
    bool is_active = (TEST_BIT_HIGH(mDigInBuffer, i) && TEST_BIT_LOW(mpConfLogic->GetValue(), i) ||  // Active at high level
                      TEST_BIT_LOW(mDigInBuffer, i) && TEST_BIT_HIGH(mpConfLogic->GetValue(), i));    // Active at low level

    mpDigInLogicState[i]->SetValue(is_active ? DIGITAL_INPUT_FUNC_STATE_ACTIVE : DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE);

    di_func = mpDiConf[i]->GetValue();

    if ( mpDigInLevel[i]->GetQuality() == DP_AVAILABLE)
    {
      if (di_func != DIGITAL_INPUT_FUNC_NO_FUNCTION)
      {
        if (TEST_BIT_HIGH(mDigInBuffer, i) && TEST_BIT_LOW(mpConfLogic->GetValue(), i) ||  // Active at high level
            TEST_BIT_LOW(mDigInBuffer, i) && TEST_BIT_HIGH(mpConfLogic->GetValue(), i))    // Active at low level
        {
          di_func_state[di_func] = DIGITAL_INPUT_FUNC_STATE_ACTIVE;
          di_func_set_active[di_func] = true;
        }
        else if (di_func_set_active[di_func] == false)
        {
          di_func_state[di_func] = DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE;
        }
      }
    }
  }

  // check for unused digital functions and set DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED
  if (mDiConfChanged == true)
  {
    for (i = (FIRST_DIGITAL_INPUT_FUNC+1); i <= LAST_DIGITAL_INPUT_FUNC; i++)
    {
      di_func_used = false;

      for (j = 0; j < MAX_NO_OF_DIG_INPUTS; j++)
      {
        if (mpDiConf[j]->GetValue() == (DIGITAL_INPUT_FUNC_TYPE)i)
        {
          // Connect the digital input number to the digital function
          mpDiFuncInput[i]->SetValue(j+1);  // input numbers begins at '1' while index begins at '0'
          di_func_used = true;
          break;
        }
      }

      if (di_func_used == false)
      {
        mpDiFuncInput[i]->SetValue(0);  // '0' indicates the function is not connected to an input
        mpDiFuncState[i]->SetValue(DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED);
      }
    }
    mDiConfChanged = false;
  }

  // update used digital input func state objects
  for (i = (FIRST_DIGITAL_INPUT_FUNC+1); i <= LAST_DIGITAL_INPUT_FUNC; i++)
  {
    mpDiFuncState[i]->SetValue(di_func_state[i]);
  }

  // Transfer digital input func states to the state variable for each input
  for (i = 0; i < MAX_NO_OF_DIG_INPUTS; i++)
  {
    di_func = mpDiConf[i]->GetValue();
    mpDigInState[i]->SetValue(di_func_state[di_func]);
  }

  // update digital input status bits object
  mpDigInStatusBits->SetValue(mDigInBuffer);

  UpdateInputCounters();

  // Task has been run - any movement of float switches has been handled.
  mpFloatSwitchInputMoved->SetValue(false);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void DiFuncHandler::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    // digital input function state and input objects, one of each per digital input function
    case SP_DIFH_DIG_IN_FUNC_STATE_EXTERNAL_FAULT:
      mpDiFuncState[DIGITAL_INPUT_FUNC_EXTERNAL_FAULT].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_EXTERNAL_FAULT:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_EXTERNAL_FAULT].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ALARM_RESET:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ALARM_RESET].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ALARM_RESET:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ALARM_RESET].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ALARM_RELAY_RESET:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ALARM_RELAY_RESET].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ALARM_RELAY_RESET:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ALARM_RELAY_RESET].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ON_OFF_PUMP_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ON_OFF_PUMP_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ON_OFF_PUMP_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ON_OFF_PUMP_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ON_OFF_PUMP_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ON_OFF_PUMP_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ON_OFF_PUMP_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ON_OFF_PUMP_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ON_OFF_PUMP_5:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ON_OFF_PUMP_5:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ON_OFF_PUMP_6:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ON_OFF_PUMP_6:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_AUTO_MAN_PUMP_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_AUTO_MAN_PUMP_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_AUTO_MAN_PUMP_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_AUTO_MAN_PUMP_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP_5:
      mpDiFuncState[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_AUTO_MAN_PUMP_5:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_AUTO_MAN_PUMP_6:
      mpDiFuncState[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_AUTO_MAN_PUMP_6:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6].Attach(pSubject);
      break;

    case SP_DIFH_DIG_IN_FUNC_STATE_FLOAT_SWITCH_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FLOAT_SWITCH_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_FLOAT_SWITCH_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FLOAT_SWITCH_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_FLOAT_SWITCH_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FLOAT_SWITCH_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_FLOAT_SWITCH_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FLOAT_SWITCH_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_FLOAT_SWITCH_5:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FLOAT_SWITCH_5:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_OVERFLOW_SWITCH:
      mpDiFuncState[DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_OVERFLOW_SWITCH:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_CONTACTOR_FEEDBACK_PUMP_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_CONTACTOR_FEEDBACK_PUMP_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_CONTACTOR_FEEDBACK_PUMP_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_CONTACTOR_FEEDBACK_PUMP_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_CONTACTOR_FEEDBACK_PUMP_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_CONTACTOR_FEEDBACK_PUMP_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_CONTACTOR_FEEDBACK_PUMP_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_CONTACTOR_FEEDBACK_PUMP_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_CONTACTOR_FEEDBACK_PUMP_5:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_CONTACTOR_FEEDBACK_PUMP_5:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_CONTACTOR_FEEDBACK_PUMP_6:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_CONTACTOR_FEEDBACK_PUMP_6:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MIXER_CONTACTOR_FEEDBACK:
      mpDiFuncState[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MIXER_CONTACTOR_FEEDBACK:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_FAIL_PHASE:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FAIL_PHASE].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FAIL_PHASE:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FAIL_PHASE].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ENERGY_CNT:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ENERGY_CNT].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ENERGY_CNT:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ENERGY_CNT].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VOLUME_CNT:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VOLUME_CNT].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VOLUME_CNT:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VOLUME_CNT].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MOTOR_PROTECTION_PUMP_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MOTOR_PROTECTION_PUMP_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MOTOR_PROTECTION_PUMP_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MOTOR_PROTECTION_PUMP_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MOTOR_PROTECTION_PUMP_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MOTOR_PROTECTION_PUMP_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MOTOR_PROTECTION_PUMP_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MOTOR_PROTECTION_PUMP_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MOTOR_PROTECTION_PUMP_5:
      mpDiFuncState[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MOTOR_PROTECTION_PUMP_5:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_MOTOR_PROTECTION_PUMP_6:
      mpDiFuncState[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_MOTOR_PROTECTION_PUMP_6:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_ANTI_SEIZING:
      mpDiFuncState[DIGITAL_INPUT_FUNC_ANTI_SEIZING].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_ANTI_SEIZING:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_ANTI_SEIZING].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_DAILY_EMPTYING:
      mpDiFuncState[DIGITAL_INPUT_FUNC_DAILY_EMPTYING].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_DAILY_EMPTYING:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_DAILY_EMPTYING].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_FOAM_DRAINING:
      mpDiFuncState[DIGITAL_INPUT_FUNC_FOAM_DRAINING].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_FOAM_DRAINING:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_FOAM_DRAINING].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_INTERLOCK:
      mpDiFuncState[DIGITAL_INPUT_FUNC_INTERLOCK].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_INTERLOCK:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_INTERLOCK].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VFD_READY_PUMP_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VFD_READY_PUMP_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VFD_READY_PUMP_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VFD_READY_PUMP_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VFD_READY_PUMP_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VFD_READY_PUMP_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VFD_READY_PUMP_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VFD_READY_PUMP_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VFD_READY_PUMP_5:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VFD_READY_PUMP_5:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_VFD_READY_PUMP_6:
      mpDiFuncState[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_VFD_READY_PUMP_6:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_USER_DEFINED_CNT_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_USER_DEFINED_CNT_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_USER_DEFINED_CNT_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_USER_DEFINED_CNT_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_USER_DEFINED_CNT_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_USER_DEFINED_CNT_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_GAS_DETECTOR:
      mpDiFuncState[DIGITAL_INPUT_FUNC_GAS_DETECTOR].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_GAS_DETECTOR:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_GAS_DETECTOR].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_WATER_ON_PIT_FLOOR:
      mpDiFuncState[DIGITAL_INPUT_FUNC_WATER_ON_PIT_FLOOR].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_WATER_ON_PIT_FLOOR:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_WATER_ON_PIT_FLOOR].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_EXTRA_FAULT_1:
      mpDiFuncState[DIGITAL_INPUT_FUNC_EXTRA_FAULT_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_EXTRA_FAULT_1:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_EXTRA_FAULT_1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_EXTRA_FAULT_2:
      mpDiFuncState[DIGITAL_INPUT_FUNC_EXTRA_FAULT_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_EXTRA_FAULT_2:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_EXTRA_FAULT_2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_EXTRA_FAULT_3:
      mpDiFuncState[DIGITAL_INPUT_FUNC_EXTRA_FAULT_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_EXTRA_FAULT_3:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_EXTRA_FAULT_3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_EXTRA_FAULT_4:
      mpDiFuncState[DIGITAL_INPUT_FUNC_EXTRA_FAULT_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_EXTRA_FAULT_4:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_EXTRA_FAULT_4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_STATE_SERVICE_MODE:
      mpDiFuncState[DIGITAL_INPUT_FUNC_SERVICE_MODE].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_FUNC_INPUT_SERVICE_MODE:
      mpDiFuncInput[DIGITAL_INPUT_FUNC_SERVICE_MODE].Attach(pSubject);
      break;
    /* Add digital input Function State and input Objects above */

    // digital input configuration objects, one per digital input
    case SP_DIFH_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[0].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_2_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_3_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_4_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_5_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_6_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_7_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_8_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[7].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_9_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[8].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_10_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[9].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_11_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[10].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_12_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[11].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_13_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[12].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_14_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[13].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_15_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[14].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_16_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[15].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_17_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[16].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_18_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[17].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_19_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[18].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_20_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[19].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_21_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[20].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_22_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[21].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_23_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[22].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_24_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[23].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_25_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[24].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_26_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[25].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_27_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[26].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_28_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[27].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_29_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[28].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_30_CONF_DIGITAL_INPUT_FUNC:
      mpDiConf[29].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_CONF_LOGIC:
      mpConfLogic.Attach(pSubject);
      break;

    case SP_DIFH_DIG_IN_1_CONF_LOGIC:
      mpInputConfLogic[0].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_2_CONF_LOGIC:
      mpInputConfLogic[1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_3_CONF_LOGIC:
      mpInputConfLogic[2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_4_CONF_LOGIC:
      mpInputConfLogic[3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_5_CONF_LOGIC:
      mpInputConfLogic[4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_6_CONF_LOGIC:
      mpInputConfLogic[5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_7_CONF_LOGIC:
      mpInputConfLogic[6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_8_CONF_LOGIC:
      mpInputConfLogic[7].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_9_CONF_LOGIC:
      mpInputConfLogic[8].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_10_CONF_LOGIC:
      mpInputConfLogic[9].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_11_CONF_LOGIC:
      mpInputConfLogic[10].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_12_CONF_LOGIC:
      mpInputConfLogic[11].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_13_CONF_LOGIC:
      mpInputConfLogic[12].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_14_CONF_LOGIC:
      mpInputConfLogic[13].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_15_CONF_LOGIC:
      mpInputConfLogic[14].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_16_CONF_LOGIC:
      mpInputConfLogic[15].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_17_CONF_LOGIC:
      mpInputConfLogic[16].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_18_CONF_LOGIC:
      mpInputConfLogic[17].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_19_CONF_LOGIC:
      mpInputConfLogic[18].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_20_CONF_LOGIC:
      mpInputConfLogic[19].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_21_CONF_LOGIC:
      mpInputConfLogic[20].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_22_CONF_LOGIC:
      mpInputConfLogic[21].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_23_CONF_LOGIC:
      mpInputConfLogic[22].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_24_CONF_LOGIC:
      mpInputConfLogic[23].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_25_CONF_LOGIC:
      mpInputConfLogic[24].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_26_CONF_LOGIC:
      mpInputConfLogic[25].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_27_CONF_LOGIC:
      mpInputConfLogic[26].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_28_CONF_LOGIC:
      mpInputConfLogic[27].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_29_CONF_LOGIC:
      mpInputConfLogic[28].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_30_CONF_LOGIC:
      mpInputConfLogic[29].Attach(pSubject);
      break;

    case SP_DIFH_DIG_IN_1_LOGIC_STATE:
      mpDigInLogicState[0].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_2_LOGIC_STATE:
      mpDigInLogicState[1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_3_LOGIC_STATE:
      mpDigInLogicState[2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_4_LOGIC_STATE:
      mpDigInLogicState[3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_5_LOGIC_STATE:
      mpDigInLogicState[4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_6_LOGIC_STATE:
      mpDigInLogicState[5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_7_LOGIC_STATE:
      mpDigInLogicState[6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_8_LOGIC_STATE:
      mpDigInLogicState[7].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_9_LOGIC_STATE:
      mpDigInLogicState[8].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_10_LOGIC_STATE:
      mpDigInLogicState[9].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_11_LOGIC_STATE:
      mpDigInLogicState[10].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_12_LOGIC_STATE:
      mpDigInLogicState[11].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_13_LOGIC_STATE:
      mpDigInLogicState[12].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_14_LOGIC_STATE:
      mpDigInLogicState[13].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_15_LOGIC_STATE:
      mpDigInLogicState[14].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_16_LOGIC_STATE:
      mpDigInLogicState[15].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_17_LOGIC_STATE:
      mpDigInLogicState[16].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_18_LOGIC_STATE:
      mpDigInLogicState[17].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_19_LOGIC_STATE:
      mpDigInLogicState[18].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_20_LOGIC_STATE:
      mpDigInLogicState[19].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_21_LOGIC_STATE:
      mpDigInLogicState[20].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_22_LOGIC_STATE:
      mpDigInLogicState[21].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_23_LOGIC_STATE:
      mpDigInLogicState[22].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_24_LOGIC_STATE:
      mpDigInLogicState[23].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_25_LOGIC_STATE:
      mpDigInLogicState[24].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_26_LOGIC_STATE:
      mpDigInLogicState[25].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_27_LOGIC_STATE:
      mpDigInLogicState[26].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_28_LOGIC_STATE:
      mpDigInLogicState[27].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_29_LOGIC_STATE:
      mpDigInLogicState[28].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_30_LOGIC_STATE:
      mpDigInLogicState[29].Attach(pSubject);
      break;

    case SP_DIFH_IO351_IO_MODULE_1_DIG_IN_STATUS_BITS:
      mpIO351DigInStatusBits[0].Attach(pSubject);
      break;
    case SP_DIFH_IO351_IO_MODULE_2_DIG_IN_STATUS_BITS:
      mpIO351DigInStatusBits[1].Attach(pSubject);
      break;
    case SP_DIFH_IO351_IO_MODULE_3_DIG_IN_STATUS_BITS:
      mpIO351DigInStatusBits[2].Attach(pSubject);
      break;

    case SP_DIFH_DIG_IN_STATUS_BITS:
      mpDigInStatusBits.Attach(pSubject);
      break;

    case SP_DIFH_DIG_IN_1_LEVEL:
      mpDigInLevel[0].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_2_LEVEL:
      mpDigInLevel[1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_3_LEVEL:
      mpDigInLevel[2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_4_LEVEL:
      mpDigInLevel[3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_5_LEVEL:
      mpDigInLevel[4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_6_LEVEL:
      mpDigInLevel[5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_7_LEVEL:
      mpDigInLevel[6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_8_LEVEL:
      mpDigInLevel[7].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_9_LEVEL:
      mpDigInLevel[8].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_10_LEVEL:
      mpDigInLevel[9].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_11_LEVEL:
      mpDigInLevel[10].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_12_LEVEL:
      mpDigInLevel[11].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_13_LEVEL:
      mpDigInLevel[12].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_14_LEVEL:
      mpDigInLevel[13].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_15_LEVEL:
      mpDigInLevel[14].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_16_LEVEL:
      mpDigInLevel[15].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_17_LEVEL:
      mpDigInLevel[16].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_18_LEVEL:
      mpDigInLevel[17].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_19_LEVEL:
      mpDigInLevel[18].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_20_LEVEL:
      mpDigInLevel[19].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_21_LEVEL:
      mpDigInLevel[20].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_22_LEVEL:
      mpDigInLevel[21].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_23_LEVEL:
      mpDigInLevel[22].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_24_LEVEL:
      mpDigInLevel[23].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_25_LEVEL:
      mpDigInLevel[24].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_26_LEVEL:
      mpDigInLevel[25].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_27_LEVEL:
      mpDigInLevel[26].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_28_LEVEL:
      mpDigInLevel[27].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_29_LEVEL:
      mpDigInLevel[28].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_30_LEVEL:
      mpDigInLevel[29].Attach(pSubject);
      break;

    case SP_DIFH_DIG_IN_1_STATE:
      mpDigInState[0].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_2_STATE:
      mpDigInState[1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_3_STATE:
      mpDigInState[2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_4_STATE:
      mpDigInState[3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_5_STATE:
      mpDigInState[4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_6_STATE:
      mpDigInState[5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_7_STATE:
      mpDigInState[6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_8_STATE:
      mpDigInState[7].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_9_STATE:
      mpDigInState[8].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_10_STATE:
      mpDigInState[9].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_11_STATE:
      mpDigInState[10].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_12_STATE:
      mpDigInState[11].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_13_STATE:
      mpDigInState[12].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_14_STATE:
      mpDigInState[13].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_15_STATE:
      mpDigInState[14].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_16_STATE:
      mpDigInState[15].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_17_STATE:
      mpDigInState[16].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_18_STATE:
      mpDigInState[17].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_19_STATE:
      mpDigInState[18].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_20_STATE:
      mpDigInState[19].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_21_STATE:
      mpDigInState[20].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_22_STATE:
      mpDigInState[21].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_23_STATE:
      mpDigInState[22].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_24_STATE:
      mpDigInState[23].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_25_STATE:
      mpDigInState[24].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_26_STATE:
      mpDigInState[25].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_27_STATE:
      mpDigInState[26].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_28_STATE:
      mpDigInState[27].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_29_STATE:
      mpDigInState[28].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_30_STATE:
      mpDigInState[29].Attach(pSubject);
      break;

    // Counter inputs
    case SP_DIFH_DIG_IN_1_COUNT:
      mpDigInCount[0].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_2_COUNT:
      mpDigInCount[1].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_3_COUNT:
      mpDigInCount[2].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_4_COUNT:
      mpDigInCount[3].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_5_COUNT:
      mpDigInCount[4].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_6_COUNT:
      mpDigInCount[5].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_7_COUNT:
      mpDigInCount[6].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_8_COUNT:
      mpDigInCount[7].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_9_COUNT:
      mpDigInCount[8].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_10_COUNT:
      mpDigInCount[9].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_11_COUNT:
      mpDigInCount[10].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_12_COUNT:
      mpDigInCount[11].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_13_COUNT:
      mpDigInCount[12].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_14_COUNT:
      mpDigInCount[13].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_15_COUNT:
      mpDigInCount[14].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_16_COUNT:
      mpDigInCount[15].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_17_COUNT:
      mpDigInCount[16].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_18_COUNT:
      mpDigInCount[17].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_19_COUNT:
      mpDigInCount[18].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_20_COUNT:
      mpDigInCount[19].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_21_COUNT:
      mpDigInCount[20].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_22_COUNT:
      mpDigInCount[21].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_23_COUNT:
      mpDigInCount[22].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_24_COUNT:
      mpDigInCount[23].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_25_COUNT:
      mpDigInCount[24].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_26_COUNT:
      mpDigInCount[25].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_27_COUNT:
      mpDigInCount[26].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_28_COUNT:
      mpDigInCount[27].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_29_COUNT:
      mpDigInCount[28].Attach(pSubject);
      break;
    case SP_DIFH_DIG_IN_30_COUNT:
      mpDigInCount[29].Attach(pSubject);
      break;
    case SP_DIFH_RAW_VOLUME_PULSES:
      mpDiFuncCount[COUNTER_INPUT_FUNC_VOLUME_PULSES].Attach(pSubject);
      break;
    case SP_DIFH_RAW_ENERGY_PULSES:
      mpDiFuncCount[COUNTER_INPUT_FUNC_ENERGY_PULSES].Attach(pSubject);
      break;
    case SP_DIFH_RAW_USD_CNT_1_PULSES:
      mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_1_PULSES].Attach(pSubject);
      break;
    case SP_DIFH_RAW_USD_CNT_2_PULSES:
      mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_2_PULSES].Attach(pSubject);
      break;
    case SP_DIFH_RAW_USD_CNT_3_PULSES:
      mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_3_PULSES].Attach(pSubject);
      break;

    case SP_DIFH_FSW_INPUT_MOVED:
      mpFloatSwitchInputMoved.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void DiFuncHandler::ConnectToSubjects(void)
{
  for (int i = 0; i < MAX_NO_OF_DIG_INPUTS; i++)
  {
    mpDiConf[i]->Subscribe(this);
    mpInputConfLogic[i]->Subscribe(this);
  }

  mpConfLogic->Subscribe(this);
  mpIO351DigInStatusBits[0]->Subscribe(this);
  mpIO351DigInStatusBits[1]->Subscribe(this);
  mpIO351DigInStatusBits[2]->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void DiFuncHandler::Update(Subject* pSubject)
{
  if (mpIO351DigInStatusBits[0].Update(pSubject)) return;
  if (mpIO351DigInStatusBits[1].Update(pSubject)) return;
  if (mpIO351DigInStatusBits[2].Update(pSubject)) return;

  if (mpConfLogic.Update(pSubject)) return;

  for (int i = 0; i < MAX_NO_OF_DIG_INPUTS; i++)
  {
    if (mpDiConf[i].Update(pSubject))
    {
      mpDiConf[i].ResetUpdated();
      mDiConfChanged = true;
      return;
    }
    if (mpInputConfLogic[i].Update(pSubject))
    {
      mpInputConfLogic[i].ResetUpdated();
      mInputConfLogicChanged = true;
      return;
    }
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void DiFuncHandler::SubscribtionCancelled(Subject* pSubject)
{
  /* Nothing to do because subjects are never destroyed */
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - UpdateInputCounters
 * DESCRIPTION: Update input counters and
 *              transfer the value to configured counter functions
 *
 * NOTE:
 *
 *****************************************************************************/
void DiFuncHandler::UpdateInputCounters()
{
  int i, bit_value;

  // Simple increment of input counters when changed from high to low
  for (i = 0; i < MAX_NO_OF_DIG_INPUTS; i++)
  {
    switch(i)
    {
      case 0 + NO_OF_DIG_INPUTS_IOB:
      case 1 + NO_OF_DIG_INPUTS_IOB:
      case 0 + NO_OF_DIG_INPUTS_IOB + NO_OF_DIG_INPUTS_IO351:
      case 1 + NO_OF_DIG_INPUTS_IOB + NO_OF_DIG_INPUTS_IO351:
         break; // Nothing, these counters are updated from IO351 module (input 1 and 2 on module 1 and 2)
      default:
        bit_value = 1 << i;
        if ((mDigInBuffer & bit_value) < (mOldDigInBuffer & bit_value))
        {
          mpDigInCount[i]->SetValue(1+mpDigInCount[i]->GetValue());
        }
        break;
    }
  }
  mOldDigInBuffer = mDigInBuffer;

  // Transfer input counters to their related functions (this could be made more generic)
  bool volume_counter_configured = false;
  bool energy_counter_configured = false;
  bool user_defined_counter_1_configured = false;
  bool user_defined_counter_2_configured = false;
  bool user_defined_counter_3_configured = false;
  for (i = 0; i < MAX_NO_OF_DIG_INPUTS; i++)
  {
    switch (mpDiConf[i]->GetValue())
    {
      case DIGITAL_INPUT_FUNC_VOLUME_CNT:
        if (volume_counter_configured == false) // Use the first found in case of more
        {
          mpDiFuncCount[COUNTER_INPUT_FUNC_VOLUME_PULSES]->SetValue(mpDigInCount[i]->GetValue());
          volume_counter_configured = true;
        }
        break;
      case DIGITAL_INPUT_FUNC_ENERGY_CNT:
        if (energy_counter_configured == false) // Use the first found in case of more
        {
          mpDiFuncCount[COUNTER_INPUT_FUNC_ENERGY_PULSES]->SetValue(mpDigInCount[i]->GetValue());
          energy_counter_configured = true;
        }
        break;
      case DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1:
        if (user_defined_counter_1_configured == false) // Use the first found in case of more
        {
          mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_1_PULSES]->SetValue(mpDigInCount[i]->GetValue());
          user_defined_counter_1_configured = true;
        }
        break;
      case DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2:
        if (user_defined_counter_2_configured == false) // Use the first found in case of more
        {
          mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_2_PULSES]->SetValue(mpDigInCount[i]->GetValue());
          user_defined_counter_2_configured = true;
        }
        break;
      case DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3:
        if (user_defined_counter_3_configured == false) // Use the first found in case of more
        {
          mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_3_PULSES]->SetValue(mpDigInCount[i]->GetValue());
          user_defined_counter_3_configured = true;
        }
        break;
    }
  }
  if (volume_counter_configured == false)
  {
    mpDiFuncCount[COUNTER_INPUT_FUNC_VOLUME_PULSES]->SetQuality(DP_NOT_AVAILABLE);
  }
  if (energy_counter_configured == false)
  {
    mpDiFuncCount[COUNTER_INPUT_FUNC_ENERGY_PULSES]->SetQuality(DP_NOT_AVAILABLE);
  }
  if (user_defined_counter_1_configured == false)
  {
    mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_1_PULSES]->SetQuality(DP_NOT_AVAILABLE);
  }
  if (user_defined_counter_2_configured == false)
  {
    mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_2_PULSES]->SetQuality(DP_NOT_AVAILABLE);
  }
  if (user_defined_counter_3_configured == false)
  {
    mpDiFuncCount[COUNTER_INPUT_FUNC_USER_DEFINED_3_PULSES]->SetQuality(DP_NOT_AVAILABLE);
  }
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
