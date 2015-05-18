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
/* CLASS NAME       : FloatSwitchCnfCtrl                                    */
/*                                                                          */
/* FILE NAME        : FloatSwitchCnfCtrl.cpp                                */
/*                                                                          */
/* CREATED DATE     : 24-07-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <FloatSwitchCnfCtrl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_FLOAT_SWITCH_CONFIG_NUMBER     13

#define CONFIGURATIONS_ANALOG               4
#define CONFIGURATIONS_2SWITCHES_1PUMP      3
#define CONFIGURATIONS_3SWITCHES_1PUMP      3
#define CONFIGURATIONS_3SWITCHES_2PUMPS     4
#define CONFIGURATIONS_4SWITCHES_1PUMP      1
#define CONFIGURATIONS_4SWITCHES_2PUMPS     8
#define CONFIGURATIONS_5SWITCHES_2PUMPS     13



/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

const U8 mcAnalogFloatSwitchNumberTable[CONFIGURATIONS_ANALOG+1] =
{
  0,                                    // Configuration 0 - illegal
  0,                                    // Configuration 1 - 0 floatswitches
  1,                                    // Configuration 2 - 1 floatswitch
  1,                                    // Configuration 3 - 1 floatswitch
  2                                     // Configuration 4 - 2 floatswitches
};


const U8 mcFloatSwitchConfigNumberTable[MAX_NO_OF_FLOAT_SWITCHES+1][MAX_NO_OF_PUMPS+1] =
{
  // 0 floatswitches
  {0, 0, 0},
  // 1 floatswitch
  {0, 0, 0},
  // 2 floatswitches
  {0, CONFIGURATIONS_2SWITCHES_1PUMP, 0},
  // 3 floatswitches
  {0, CONFIGURATIONS_3SWITCHES_1PUMP, CONFIGURATIONS_3SWITCHES_2PUMPS},
  // 4 floatswitches
  {0, CONFIGURATIONS_4SWITCHES_1PUMP, CONFIGURATIONS_4SWITCHES_2PUMPS},
  // 5 floatswitches
  {0, 0, CONFIGURATIONS_5SWITCHES_2PUMPS}
};


// Functiontables for FloatSwitchCnfCtrl

const FSW_TYPE mcFloatSwitchConfigTableAnalog[CONFIGURATIONS_ANALOG+1][2] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 2
  {FSW_HIGH_WATER, FSW_NOT_DEFINED},
  // Configuration 3
  {FSW_DRY_RUN, FSW_NOT_DEFINED},
  // Configuration 4
  {FSW_DRY_RUN, FSW_HIGH_WATER}
};

const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTableAnalog[CONFIGURATIONS_ANALOG+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 2
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 3
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 4
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION}
};



const FSW_TYPE mcFloatSwitchConfigTable2Switches1Pump[CONFIGURATIONS_2SWITCHES_1PUMP+1][2] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_STOP, FSW_START1},
  // Configuration 2
  {FSW_START1_STOP, FSW_HIGH_WATER},
  // Configuration 3
  {FSW_DRY_RUN, FSW_START1_STOP}
};

const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTable2Switches1Pump[CONFIGURATIONS_2SWITCHES_1PUMP+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 2
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 3
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION}
};



const FSW_TYPE mcFloatSwitchConfigTable3Switches1Pump[CONFIGURATIONS_3SWITCHES_1PUMP+1][3] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_STOP, FSW_START1, FSW_HIGH_WATER},
  // Configuration 2
  {FSW_DRY_RUN, FSW_START1_STOP, FSW_HIGH_WATER},
  // Configuration 3
  {FSW_DRY_RUN, FSW_STOP, FSW_START1}
};



const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTable3Switches1Pump[CONFIGURATIONS_3SWITCHES_1PUMP+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 2
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 3
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION}
};



const FSW_TYPE mcFloatSwitchConfigTable3Switches2Pumps[CONFIGURATIONS_3SWITCHES_2PUMPS+1][3] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_START1_STOP, FSW_START2, FSW_HIGH_WATER},
  // Configuration 2
  {FSW_DRY_RUN, FSW_START1_STOP, FSW_START2},
  // Configuration 3
  {FSW_START1_STOP, FSW_ALARM, FSW_START2},
  // Configuration 4
  {FSW_STOP, FSW_START1, FSW_START2}
};

const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTable3Switches2Pumps[CONFIGURATIONS_3SWITCHES_2PUMPS+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 2
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 3
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 4
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION}
};



const FSW_TYPE mcFloatSwitchConfigTable4Switches1Pump[CONFIGURATIONS_4SWITCHES_1PUMP+1][4] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_DRY_RUN, FSW_STOP, FSW_START1, FSW_HIGH_WATER}
};

const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTable4Switches1Pump[CONFIGURATIONS_4SWITCHES_1PUMP+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION}
};



const FSW_TYPE mcFloatSwitchConfigTable4Switches2Pumps[CONFIGURATIONS_4SWITCHES_2PUMPS+1][4] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_DRY_RUN, FSW_START1_STOP, FSW_START2, FSW_HIGH_WATER},
  // Configuration 2
  {FSW_STOP, FSW_START1, FSW_START2, FSW_HIGH_WATER},
  // Configuration 3
  {FSW_STOP, FSW_START1, FSW_ALARM, FSW_START2},
  // Configuration 4
  {FSW_DRY_RUN, FSW_START1_STOP, FSW_ALARM, FSW_START2},
  // Configuration 5
  {FSW_DRY_RUN, FSW_STOP, FSW_START1, FSW_START2},
  // Configuration 6
  {FSW_STOP1, FSW_STOP2, FSW_START1, FSW_START2},
  // Configuration 7
  {FSW_STOP2, FSW_STOP1, FSW_START1, FSW_START2},
  // Configuration 8
  {FSW_DRY_RUN, FSW_START1_STOP, FSW_STOP2, FSW_START2}
};

const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTable4Switches2Pumps[CONFIGURATIONS_4SWITCHES_2PUMPS+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 2
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 3
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 4
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 5
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 6
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 7
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 8
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_NO_FUNCTION},
};



const FSW_TYPE mcFloatSwitchConfigTable5Switches2Pumps[CONFIGURATIONS_5SWITCHES_2PUMPS+1][5] =
{
  // Configuration 0 - illegal situation
  {FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED, FSW_NOT_DEFINED},
  // Configuration 1
  {FSW_DRY_RUN, FSW_STOP, FSW_START1, FSW_START2, FSW_HIGH_WATER},
  // Configuration 2
  {FSW_DRY_RUN, FSW_START1_STOP, FSW_ALARM, FSW_START2, FSW_HIGH_WATER},
  // Configuration 3
  {FSW_STOP, FSW_START1, FSW_ALARM, FSW_START2, FSW_HIGH_WATER},
  // Configuration 4
  {FSW_DRY_RUN, FSW_STOP, FSW_START1, FSW_ALARM, FSW_START2},
  // Configuration 5
  {FSW_DRY_RUN, FSW_STOP1, FSW_STOP2, FSW_START1, FSW_START2},
  // Configuration 6
  {FSW_STOP1, FSW_STOP2, FSW_START1, FSW_START2, FSW_HIGH_WATER},
  // Configuration 7
  {FSW_STOP1, FSW_STOP2, FSW_START1, FSW_ALARM, FSW_START2},
  // Configuration 8
  {FSW_DRY_RUN, FSW_STOP2, FSW_STOP1, FSW_START1, FSW_START2},
  // Configuration 9
  {FSW_DRY_RUN, FSW_STOP1, FSW_START1, FSW_STOP2, FSW_START2},
  // Configuration 10
  {FSW_STOP2, FSW_STOP1, FSW_START1, FSW_START2, FSW_HIGH_WATER},
  // Configuration 11
  {FSW_STOP2, FSW_STOP1, FSW_START1, FSW_ALARM, FSW_START2},
  // Configuration 12
  {FSW_STOP1, FSW_START1, FSW_STOP2, FSW_START2, FSW_HIGH_WATER},
  // Configuration 13
  {FSW_STOP1, FSW_START1, FSW_STOP2, FSW_ALARM, FSW_START2}
};

const DIGITAL_INPUT_FUNC_TYPE mcDigitalInputConfigTable5Switches2Pumps[CONFIGURATIONS_5SWITCHES_2PUMPS+1][MAX_NO_OF_FLOAT_SWITCHES] =
{
  // Configuration 0 - illegal situation
  {DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION, DIGITAL_INPUT_FUNC_NO_FUNCTION},
  // Configuration 1
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4},
  // Configuration 2
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4},
  // Configuration 3
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4},
  // Configuration 4
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
  // Configuration 5
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
  // Configuration 6
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4},
  // Configuration 7
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
  // Configuration 8
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
  // Configuration 9
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
  // Configuration 10
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4},
  // Configuration 11
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
  // Configuration 12
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4},
  // Configuration 13
  {DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4, DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5},
};



/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

FloatSwitchCnfCtrl* FloatSwitchCnfCtrl::mpInstance = 0;

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 ****************************************************************************/
FloatSwitchCnfCtrl* FloatSwitchCnfCtrl::GetInstance()
{
  if (!mpInstance)
  {
    mpInstance = new FloatSwitchCnfCtrl();
  }
  return mpInstance;
}


/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::InitSubTask()
{
  mpFloatSwitchInputMoved->SetValue(false);

  // Check if the max value for config number is set correct.
  // In theory it could be wrong if a controller is powered down just when the pitLevelCtrlType has been changed.
  // This changed should cause the config number max limit to be recalculated, but if the power goes before the update
  // of the pitLevelCtrlType is recognized this recalculation could be missing.
  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
  {
    // Set min and max at number of float switches to the range for legal number of float switches (2-5)
    mpCurrentNoOfFloatSwitches->SetMaxValue(MAX_NO_OF_FLOAT_SWITCHES);
    if (mpNoOfPumps->GetValue() == 2)
    {
      mpCurrentNoOfFloatSwitches->SetMinValue(3);
      mpCurrentNoOfFloatSwitches->SetMaxValue(5);
    }
    else
    {
      mpCurrentNoOfFloatSwitches->SetMinValue(2);
      mpCurrentNoOfFloatSwitches->SetMaxValue(4);
    }
    // Set max value for config number to the value determined by switches and pumps
    // If an illegal combination of swithces and pumps are selected the max value will be set to 0
    mpCurrentFloatSwitchConfigNumber->SetMaxValue(mcFloatSwitchConfigNumberTable[mpCurrentNoOfFloatSwitches->GetValue()][mpNoOfPumps->GetValue()]);
  }
  else
  {
    // Set max value for config number to the value for analog sensors
    mpCurrentFloatSwitchConfigNumber->SetMaxValue(CONFIGURATIONS_ANALOG);
    // Set the number of floatswitches (and the max and min value of floatswitches) to the value matching configuration 1
    // Max and min are set to the same value to assure, that the value cannot be changed from the display.
    mpCurrentNoOfFloatSwitches->SetMaxValue(mcAnalogFloatSwitchNumberTable[mpCurrentFloatSwitchConfigNumber->GetValue()]);
    mpCurrentNoOfFloatSwitches->SetMinValue(mcAnalogFloatSwitchNumberTable[mpCurrentFloatSwitchConfigNumber->GetValue()]);
    mpCurrentNoOfFloatSwitches->SetValue(mcAnalogFloatSwitchNumberTable[mpCurrentFloatSwitchConfigNumber->GetValue()]);
  }

  // Syncronize the current no of float swithces and the current config number with the config number used for the configuration
  // to be selected before rolling through the procedures of setting the configuration
  mpNoOfFloatSwitches->CopyValues(mpCurrentNoOfFloatSwitches.GetSubject());
  mpFloatSwitchConfigNumber->CopyValues(mpCurrentFloatSwitchConfigNumber.GetSubject());
  mpFloatSwitchConfigNumber.ResetUpdated();  // A response to the any update of the config number caused by fiddling with the
                                             // max value is not wanted. This response will clear the execute config flag and
                                             // we want the execute config flag to be set as the initialization should end up
                                             // with the configuration saved at power down reestablished.

  SetTemporaryFloatSwitchConfiguration();
  TransferTemporaryFloatSwitchConfiguration();
  mpFloatSwitchExecuteConfig->SetValue(true);
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::RunSubTask()
{
  U8 configuration_number;

  mRunRequestedFlag = false;

  if (mpPitLevelCtrlType.IsUpdated())
  {
    if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
    {
      // Set min and max at number of float switches to the range for legal number of float switches (2-5)
      mpNoOfFloatSwitches->SetMaxValue(MAX_NO_OF_FLOAT_SWITCHES);
      // Adjust minimum number of float switches to the number of pumps.
      if (mpNoOfPumps->GetValue() == 2)
      {
        mpNoOfFloatSwitches->SetMinValue(3);
        mpNoOfFloatSwitches->SetMaxValue(5);
      }
      else
      {
        mpNoOfFloatSwitches->SetMinValue(2);
        mpNoOfFloatSwitches->SetMaxValue(4);
      }
      // Force an update of floatswitches to assure that a configuration is done
      // using the current number of floatswitches.
      mpNoOfFloatSwitches.SetUpdated();
    }
    else
    {
      // Set max value for config number to the value for analog sensors
      mpFloatSwitchConfigNumber->SetMaxValue(CONFIGURATIONS_ANALOG);
      // Set default configuration to the first configuration - this must be done after the correct max value has been set.
      mpFloatSwitchConfigNumber->SetValue(1);
      mpFloatSwitchConfigNumber.ResetUpdated();  // A response to the update of the config number is not wanted. This response would
                                                 // have cleared the execute config flag and we want the execute config flag to be
                                                 // set as a change to analog sensor should force the configuration 1 through.
      // Set the number of floatswitches (and the max and min value of floatswitches) to the value matching configuration 1
      // Max and min are set to the same value to assure, that the value cannot be changed from the display.
      mpNoOfFloatSwitches->SetMaxValue(mcAnalogFloatSwitchNumberTable[1]);
      mpNoOfFloatSwitches->SetMinValue(mcAnalogFloatSwitchNumberTable[1]);
      mpNoOfFloatSwitches->SetValue(mcAnalogFloatSwitchNumberTable[1]);

      SetTemporaryFloatSwitchConfiguration();
      TransferTemporaryFloatSwitchConfiguration();
      mpFloatSwitchExecuteConfig->SetValue(true);
    }
  }

  if (mpNoOfFloatSwitches.IsUpdated() || mpNoOfPumps.IsUpdated())
  {
    if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
    {
      // Adjust minimum number of float switches to the number of pumps.
      if (mpNoOfPumps->GetValue() == 2)
      {
        mpNoOfFloatSwitches->SetMinValue(3);
        mpNoOfFloatSwitches->SetMaxValue(5);
      }
      else
      {
        mpNoOfFloatSwitches->SetMinValue(2);
        mpNoOfFloatSwitches->SetMaxValue(4);
      }

      // Set max value for config number to the value determined by switches and pumps
      // If an illegal combination of swithces and pumps are selected the max value will be set to 0
      mpFloatSwitchConfigNumber->SetMaxValue(mcFloatSwitchConfigNumberTable[mpNoOfFloatSwitches->GetValue()][mpNoOfPumps->GetValue()]);
      // Set default configuration to the first configuration - this must be done after the correct max value has been set.
      mpFloatSwitchConfigNumber->SetValue(1);
      mpFloatSwitchConfigNumber.ResetUpdated();  // A response to the update of the config number is not wanted. This response would
                                                 // have cleared the execute config flag and we want the execute config flag to be
                                                 // set as a change of switches or pumps should force the configuration 1 through.
      SetTemporaryFloatSwitchConfiguration();
      TransferTemporaryFloatSwitchConfiguration();
      mpFloatSwitchExecuteConfig->SetValue(true);
    }
  }

  if (mpFloatSwitchConfigNumber.IsUpdated())
  {
    if (mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)
    {
      // Set the number of floatswitches (and the max and min value of floatswitches) to the value matching the configuration
      // Max and min are set to the same value to assure, that the value cannot be changed from the display.
      configuration_number = mpFloatSwitchConfigNumber->GetValue();
      mpNoOfFloatSwitches->SetMaxValue(mcAnalogFloatSwitchNumberTable[configuration_number]);
      mpNoOfFloatSwitches->SetMinValue(mcAnalogFloatSwitchNumberTable[configuration_number]);
      mpNoOfFloatSwitches->SetValue(mcAnalogFloatSwitchNumberTable[configuration_number]);
    }
    SetTemporaryFloatSwitchConfiguration();
    mpFloatSwitchExecuteConfig->SetValue(false);
  }

  if (mpFloatSwitchExecuteConfig.IsUpdated())
  {
    if (mpFloatSwitchExecuteConfig->GetValue() == true)
    {
      TransferTemporaryFloatSwitchConfiguration();
    }
  }

  if (mpOverflowSwitchInstalled.IsUpdated())
  {
    // Reapply the float switch configuration already selected 
    // to redo/undo configuration with/without overflow switch
    configuration_number = mpCurrentFloatSwitchConfigNumber->GetValue();
    mpFloatSwitchConfigNumber->SetValue(configuration_number);
    SetTemporaryFloatSwitchConfiguration();
    TransferTemporaryFloatSwitchConfiguration();
  }

}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::ConnectToSubjects()
{
  mpNoOfPumps->Subscribe(this);
  mpNoOfFloatSwitches->Subscribe(this);
  mpFloatSwitchConfigNumber->Subscribe(this);
  mpFloatSwitchExecuteConfig->Subscribe(this);
  mpPitLevelCtrlType->Subscribe(this);
  mpOverflowSwitchInstalled->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 * If it is then put the pointer in queue and request task time for sub task.
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::Update(Subject* pSubject)
{
  if (mpNoOfPumps.Update(pSubject))
  {
    // nop
  }
  else if (mpNoOfFloatSwitches.Update(pSubject))
  {
    // nop
  }
  else if (mpFloatSwitchConfigNumber.Update(pSubject))
  {
    // nop
  }
  else if (mpFloatSwitchExecuteConfig.Update(pSubject))
  {
    // nop
  }
  else if (mpPitLevelCtrlType.Update(pSubject))
  {
    // nop
  }
  else if (mpOverflowSwitchInstalled.Update(pSubject))
  {
    // nop
  }

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_FSCC_NO_OF_PUMPS:
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_FSCC_NO_OF_FLOAT_SWITCHES:
      mpNoOfFloatSwitches.Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_CONFIG_NUMBER:
      mpFloatSwitchConfigNumber.Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_EXECUTE_CONFIG:
      mpFloatSwitchExecuteConfig.Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_1_CNF:
      mpFloatSwitchCnf[0].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_2_CNF:
      mpFloatSwitchCnf[1].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_3_CNF:
      mpFloatSwitchCnf[2].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_4_CNF:
      mpFloatSwitchCnf[3].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_5_CNF:
      mpFloatSwitchCnf[4].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_1_TMP_CNF:
      mpFloatSwitchTmpCnf[0].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_2_TMP_CNF:
      mpFloatSwitchTmpCnf[1].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_3_TMP_CNF:
      mpFloatSwitchTmpCnf[2].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_4_TMP_CNF:
      mpFloatSwitchTmpCnf[3].Attach(pSubject);
      break;
    case SP_FSCC_FLOAT_SWITCH_5_TMP_CNF:
      mpFloatSwitchTmpCnf[4].Attach(pSubject);
      break;
    case SP_FSCC_DIG_IN_1_CONF_DIGITAL_INPUT_FUNC:
      mpDiConfFloatSwitch[0].Attach(pSubject);
      break;
    case SP_FSCC_DIG_IN_2_CONF_DIGITAL_INPUT_FUNC:
      mpDiConfFloatSwitch[1].Attach(pSubject);
      break;
    case SP_FSCC_DIG_IN_3_CONF_DIGITAL_INPUT_FUNC:
      mpDiConfFloatSwitch[2].Attach(pSubject);
      break;
    case SP_FSCC_DIG_IN_12_CONF_DIGITAL_INPUT_FUNC:  // Dig in 9 at first IO 351
      mpDiConfFloatSwitch[3].Attach(pSubject);
      break;
    case SP_FSCC_DIG_IN_11_CONF_DIGITAL_INPUT_FUNC:  // Dig in 8 at first IO 351
      mpDiConfFloatSwitch[4].Attach(pSubject);
      break;

    case SP_FSCC_PIT_LEVEL_CTRL_TYPE:
      mpPitLevelCtrlType.Attach(pSubject);
      break;
    case SP_FSCC_CURRENT_FLOAT_SWITCH_CONFIG_NUMBER:
      mpCurrentFloatSwitchConfigNumber.Attach(pSubject);
      break;
    case SP_FSCC_CURRENT_NO_OF_FLOAT_SWITCHES:
      mpCurrentNoOfFloatSwitches.Attach(pSubject);
      break;

    case SP_FSCC_FSW_INPUT_MOVED:
      mpFloatSwitchInputMoved.Attach(pSubject);
      break;
    case SP_FSCC_OVERFLOW_SWITCH_INSTALLED:
      mpOverflowSwitchInstalled.Attach(pSubject);
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
FloatSwitchCnfCtrl::FloatSwitchCnfCtrl()
{
  mRunRequestedFlag = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
FloatSwitchCnfCtrl::~FloatSwitchCnfCtrl()
{
}


/*****************************************************************************
 * Function - SetTemporaryFloatSwitchConfiguration
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::SetTemporaryFloatSwitchConfiguration(void)
{
  U8 i;
  U8 no_of_float_switches;
  U8 configuration_number;

  // Clear temporary configuration
  for (i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
  {
    mpFloatSwitchTmpCnf[i]->SetValue(FSW_NOT_DEFINED);
  }

  no_of_float_switches = mpNoOfFloatSwitches->GetValue();
  configuration_number = mpFloatSwitchConfigNumber->GetValue();

  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
  {
    switch (no_of_float_switches)
    {
      case 0:
        break;
      case 1:
        break;
      case 2:
        if (mpNoOfPumps->GetValue() == 1)
        {
          for (i=0; i<no_of_float_switches; i++)
          {
            mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTable2Switches1Pump[configuration_number][i]);
          }
        }
        break;
      case 3:
        if (mpNoOfPumps->GetValue() == 1)
        {
          for (i=0; i<no_of_float_switches; i++)
          {
            mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTable3Switches1Pump[configuration_number][i]);
          }
        }
        else if (mpNoOfPumps->GetValue() == 2)
        {
          for (i=0; i<no_of_float_switches; i++)
          {
            mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTable3Switches2Pumps[configuration_number][i]);
          }
        }
        break;
      case 4:
        if (mpNoOfPumps->GetValue() == 1)
        {
          for (i=0; i<no_of_float_switches; i++)
          {
            mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTable4Switches1Pump[configuration_number][i]);
          }
        }
        else if (mpNoOfPumps->GetValue() == 2)
        {
          for (i=0; i<no_of_float_switches; i++)
          {
            mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTable4Switches2Pumps[configuration_number][i]);
          }
        }
        break;
      case 5:
        if (mpNoOfPumps->GetValue() == 2)
        {
          for (i=0; i<no_of_float_switches; i++)
          {
            mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTable5Switches2Pumps[configuration_number][i]);
          }
        }
        break;
      default:
        break;
    }
  }
  else
  {
    for (i=0; i<no_of_float_switches; i++)
    {
      mpFloatSwitchTmpCnf[i]->SetValue(mcFloatSwitchConfigTableAnalog[configuration_number][i]);
    }
  }

  // Set not existing float switches as "never available" and not defined float switches
  // as "not available" to assure that they will not be shown in display
  for (i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
  {
    FSW_TYPE float_switch_cnf;

    if (i >= no_of_float_switches)
    {
      mpFloatSwitchTmpCnf[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
    else
    {
      float_switch_cnf = mpFloatSwitchTmpCnf[i]->GetValue();
      if (float_switch_cnf == FSW_NOT_DEFINED)
      {
        mpFloatSwitchTmpCnf[i]->SetQuality(DP_NOT_AVAILABLE);
      }
    }
  }

}


/*****************************************************************************
 * Function - TransferTemporaryFloatSwitchConfiguration
 * DESCRIPTION:
 *
 *****************************************************************************/
void FloatSwitchCnfCtrl::TransferTemporaryFloatSwitchConfiguration(void)
{
  U8 no_of_float_switches;
  U8 highest_used_dig_in;
  U8 configuration_number;
  U8 i;
  DP_QUALITY_TYPE float_switch_tmp_cnf_quality;
  DIGITAL_INPUT_FUNC_TYPE previous_di_conf_float_switch[MAX_NO_OF_FLOAT_SWITCHES];

  for ( i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
  {
    previous_di_conf_float_switch[i] = mpDiConfFloatSwitch[i]->GetValue();
  }

  // Configure the floatswitches to the correct digital input
  no_of_float_switches = mpNoOfFloatSwitches->GetValue();
  configuration_number = mpFloatSwitchConfigNumber->GetValue();

  // update current float switch number and current no of float switches
  mpCurrentNoOfFloatSwitches->CopyValues(mpNoOfFloatSwitches.GetSubject());
  mpCurrentFloatSwitchConfigNumber->CopyValues(mpFloatSwitchConfigNumber.GetSubject());

  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
  {
    for ( i=0; i<no_of_float_switches; i++)
    {
      switch (no_of_float_switches)
      {
        case 1:
          break;
        case 2:
          if (mpNoOfPumps->GetValue() == 1)
          {
            mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTable2Switches1Pump[configuration_number][i]);
          }
          break;
        case 3:
          if (mpNoOfPumps->GetValue() == 1)
          {
            mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTable3Switches1Pump[configuration_number][i]);
          }
          else if (mpNoOfPumps->GetValue() == 2)
          {
            mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTable3Switches2Pumps[configuration_number][i]);
          }
          break;
        case 4:
          if (mpNoOfPumps->GetValue() == 1)
          {
            mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTable4Switches1Pump[configuration_number][i]);
          }
          else if (mpNoOfPumps->GetValue() == 2)
          {
            mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTable4Switches2Pumps[configuration_number][i]);
          }
          break;
        case 5:
          if (mpNoOfPumps->GetValue() == 2)
          {
            mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTable5Switches2Pumps[configuration_number][i]);
          }
          break;
        default:
          mpDiConfFloatSwitch[i]->SetValue(DIGITAL_INPUT_FUNC_NO_FUNCTION);
          break;
      }
    }
  }
  else // not float switch controlled
  {
    if (configuration_number == 2)
    {
      // Special handling required for configuration 2 because the high water switch
      // must be placed at digital input 2 while there is nothing placed at input 1.
      mpDiConfFloatSwitch[1]->SetValue(mcDigitalInputConfigTableAnalog[configuration_number][1]);
    }
    else
    {
      for (i=0; i<no_of_float_switches; i++)
      {
        mpDiConfFloatSwitch[i]->SetValue(mcDigitalInputConfigTableAnalog[configuration_number][i]);
      }
    }
  }

  // Release inputs not used by float switches, if they have been used by a float switch previously. Otherwise leave the
  // input configuration alone.
  highest_used_dig_in = no_of_float_switches;
  if (configuration_number == 2 && mpPitLevelCtrlType->GetValue() != SENSOR_TYPE_FLOAT_SWITCHES)
  {
    // Again special handling required for configuration 2 using pressure sensor because the high water switch
    // must be placed at digital input 2 while there is nothing placed at input 1.
    // Thus the digital input 1 can be released for other purposes if it was previous used for a float switch
    switch (mpDiConfFloatSwitch[0]->GetValue())
    {
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5:
        // Input has been configured for a float switch, but is not used for this anymore. Set it to NO_FUNCTION.
        mpDiConfFloatSwitch[0]->SetValue(DIGITAL_INPUT_FUNC_NO_FUNCTION);
        break;
      default:
        break;
    }
    highest_used_dig_in++;
  }
  for ( i=highest_used_dig_in; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
  {
    switch (mpDiConfFloatSwitch[i]->GetValue())
    {
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4:
      case DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5:
      case DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH:
        // Input has been configured for a float switch, but is not used for this anymore. Set it to NO_FUNCTION.
        mpDiConfFloatSwitch[i]->SetValue(DIGITAL_INPUT_FUNC_NO_FUNCTION);
        break;
      default:
        break;
    }
  }

  HandleOverflowSwitch();

  // Set a flag for changing the float switch inputs. This means that the float switch controller must wait
  // for the DiFuncHandler to reconfigure the digital inputs before it can validate the float switch inputs
  for ( i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
  {
    if (previous_di_conf_float_switch[i] != mpDiConfFloatSwitch[i]->GetValue())
    {
      mpFloatSwitchInputMoved->SetValue(true);
    }
  }



  // Transfer temporary float switch configuration to the saved configuration.
  for ( i=0; i<MAX_NO_OF_FLOAT_SWITCHES; i++)
  {
    if (i >= no_of_float_switches)
    {
      mpFloatSwitchCnf[i]->SetQuality(DP_NEVER_AVAILABLE);
    }
    else
    {
      float_switch_tmp_cnf_quality = mpFloatSwitchTmpCnf[i]->GetQuality();
      if (float_switch_tmp_cnf_quality == DP_AVAILABLE)
      {
        mpFloatSwitchCnf[i]->SetValue(mpFloatSwitchTmpCnf[i]->GetValue());
      }
      else
      {
        mpFloatSwitchCnf[i]->SetQuality(DP_NOT_AVAILABLE);
      }
    }
  }
}

/*****************************************************************************
 * Function - HandleOverflowSwitch
 * DESCRIPTION: Assigns overflow switch to DI3-CU361 if selected and number of switches is below 5
 * If 5 float switches are installed then "high water switch" is used as overflow switch. 
 *****************************************************************************/
void FloatSwitchCnfCtrl::HandleOverflowSwitch()
{
  const int di3_index = 2;

  DIGITAL_INPUT_FUNC_TYPE function_of_di3 = mpDiConfFloatSwitch[di3_index]->GetValue();

  if (mpNoOfFloatSwitches->GetValue() == MAX_NO_OF_FLOAT_SWITCHES)
  {
    //don't allow overflow switch if 5 switches are installed
    mpOverflowSwitchInstalled->SetValue(false);
  }

  
  if (mpOverflowSwitchInstalled->GetValue())
  {
    // always use DI3 for the overflow switch. 
    // If it is already used for something else then move its function to next vacant input.
    if (function_of_di3 != DIGITAL_INPUT_FUNC_NO_FUNCTION
      && function_of_di3 != DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH)
    {
      for (int i = di3_index + 1; i < MAX_NO_OF_FLOAT_SWITCHES; i++)
      {
        if (mpDiConfFloatSwitch[i]->GetValue() == DIGITAL_INPUT_FUNC_NO_FUNCTION)
        {
          mpDiConfFloatSwitch[i]->SetValue(function_of_di3);
          break;
        }
      }
    }

    mpDiConfFloatSwitch[di3_index]->SetValue(DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH);
  }
  else
  {
    if (function_of_di3 == DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH)
    {
      mpDiConfFloatSwitch[di3_index]->SetValue(DIGITAL_INPUT_FUNC_NO_FUNCTION);
    }
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
