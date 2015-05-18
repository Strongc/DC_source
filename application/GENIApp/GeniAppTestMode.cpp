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
/* CLASS NAME       : GeniAppTestMode                                       */
/*                                                                          */
/* FILE NAME        : GeniAppTestMode.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 04-03-2008 dd-mm-yyyy                                 */
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
#include <GeniAppTestMode.h>
#include <GeniTestModeData.h>
#include <SoftwareVersion.h>
#include <IobComDrv.h>
#include <microp.h>
#ifndef __PC__
  #include <EthernetCtrl.h>
#else
  #include <PC/EthernetCtrl.h>
#endif // __PC__
#include <keyboard.h>
#include <gpio.h>
#include <iob_h_irq.h>
#include <ErrorLog.h>
#include <DisplayController.h> /* Access to TestScreen */
#include <GUI.h> /* Access to GUI_StoreKey */

extern "C"
{
  #include <geni_if.h>         /* Access to GENI buffers */
}

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DIG_OUT_TEST_BIT      0
#define LEDS_ON_OFF_TEST_BIT  1
#define TEST_PICTURE_TEST_BIT 2
#define UART0_LB_TEST_BIT     3
#define SD_RAM_TEST_OK 0

#define HW_INFO (*(U8*)0xB1000000)

#define IO_SIMULATION_ENABLED 108

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  CREATES AN OBJECT.
 ******************************************************************************/
GeniAppTestMode* GeniAppTestMode::mInstance = 0;


/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

 /*****************************************************************************
  * Function - GetInstance
  * DESCRIPTION: Returns pointer to singleton object of this class.
  *
  *****************************************************************************/
GeniAppTestMode* GeniAppTestMode::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new GeniAppTestMode();
  }
  return mInstance;
}

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
GeniAppTestMode::GeniAppTestMode()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
GeniAppTestMode::~GeniAppTestMode()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniAppTestMode::InitSubTask()
{
  mTestModeEnabled = false;

  // Disable Class 6 Access
  ClassAcc(TEST_APDU, GET_ACC, Disable);
  ClassAcc(TEST_APDU, SET_ACC, Disable);

  // Initialize test status parameters
  G_ge_eth_loop_back_test = ETH_NOT_TESTED;
  G_ge_keyboard_status = 0;
  G_ge_emwin_key_input = 255;
  G_ge_sd_ram_test = SD_RAM_TEST_OK;
  G_ge_uart0_loop_back_test = UART_NOT_TESTED;
  G_ge_firmware_update_state = FIRMWARE_UPDATE_STATE_IDLE;
  G_ge_test_loop_displays = 0;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniAppTestMode::RunSubTask()
{
  static U32 run_counter = 0;

  run_counter++;

  if (mpTestModeEnable.IsUpdated())
  {
    EnableTestMode(true);
  }
  if (mpTestModeDisable.IsUpdated())
  {
    EnableTestMode(false);
  }

  /* Handle GENI Class 6 data received from bus*/
  if (mTestModeEnabled == true)
  {
    HandleGeniTestData();

  /* Get test status from internal tests and copy result to geni */
  #ifndef __PC__
    GetTestStatus();
  #endif
  }

  // Special handling for sw version strings since IO version may be a bit delayed (count = 100 ~~> 1 second)
  if (run_counter == 1 || run_counter == 100)
  {
    // Initialize software versions (could be in a separate class, not really test values)
    sprintf(G_ge_cpu_software_version,"%s CPU %s",HW_NAME, CpuSoftwareVersion::GetInstance()->GetValue());
    sprintf(G_ge_io_software_version,"CU 361 IO %s", IoSoftwareVersion::GetInstance()->GetValue());
    sprintf(G_ge_boot_load_software_version,"%s BOOT LOAD %s", HW_NAME, BootLoadSoftwareVersion::GetInstance()->GetValue());
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void GeniAppTestMode::ConnectToSubjects()
{
  mpTestModeEnable->Subscribe(this);
  mpTestModeDisable->Subscribe(this);
  mpIobBoardId->Subscribe(this);
  mpIobTemperature->Subscribe(this);
  mpIobPressure->Subscribe(this);
  mpIobBatteryVoltage->Subscribe(this);
  mpIobBusModulePressent->Subscribe(this);
  mpIobSupplyStatus->Subscribe(this);
  mpIobBatteryStatus->Subscribe(this);
  mpDigitalInput->Subscribe(this);
  mpAnalogInput0->Subscribe(this);
  mpAnalogInput1->Subscribe(this);
  mpAnalogInput2->Subscribe(this);
  mpAnalogInput3->Subscribe(this);
  mpAnalogInput4->Subscribe(this);
  mpAnalogInput5->Subscribe(this);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void GeniAppTestMode::Update(Subject* pSubject)
{
  mpTestModeEnable.Update(pSubject);
  mpTestModeDisable.Update(pSubject);
  if (mTestModeEnabled == true)
  {
    mpIobBoardId.Update(pSubject);
    mpIobTemperature.Update(pSubject);
    mpIobPressure.Update(pSubject);
    mpIobBatteryVoltage.Update(pSubject);
    mpIobBusModulePressent.Update(pSubject);
    mpIobSupplyStatus.Update(pSubject);
    mpIobBatteryStatus.Update(pSubject);
    mpDigitalInput.Update(pSubject);
    mpAnalogInput0.Update(pSubject);
    mpAnalogInput1.Update(pSubject);
    mpAnalogInput2.Update(pSubject);
    mpAnalogInput3.Update(pSubject);
    mpAnalogInput4.Update(pSubject);
    mpAnalogInput5.Update(pSubject);
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void GeniAppTestMode::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void GeniAppTestMode::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Test mode
    case SP_GATM_ENABLE_TEST_MODE:
      mpTestModeEnable.Attach(pSubject);
      break;
    case SP_GATM_DISABLE_TEST_MODE:
      mpTestModeDisable.Attach(pSubject);
      break;
    case SP_GATM_TEST_MODE_STATUS:
      mpTestModeStatus.Attach(pSubject);
      break;


    // Firmware Update
    case SP_GATM_FIRMWARE_UPDATE_STATE:
      mpFirmwareUpdateState.Attach(pSubject);
      break;

    // Display
    case SP_GATM_DISPLAY_BACKLIGHT:
      mpDisplayBacklight.Attach(pSubject);
      break;
    case SP_GATM_DISPLAY_CONTRAST:
      mpDisplayContrast.Attach(pSubject);
      break;

    // IO simulation
    case SP_GATM_IOB_SIM_ENABLE:
      mpIOSimulationEnable.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_DISABLE:
      mpIOSimulationDisable.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_MODE:
      mpIOSimulationMode.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_STATUS:
      mpIOSimulationStatus.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_DIG_IN:
      mpSimDigitalInputs.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_ANA_IN_0:
      mpSimValueAD0.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_ANA_IN_1:
      mpSimValueAD1.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_ANA_IN_2:
      mpSimValueAD2.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_ANA_IN_3:
      mpSimValueAD3.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_ANA_IN_4:
      mpSimValueAD4.Attach(pSubject);
      break;
    case SP_GATM_IOB_SIM_ANA_IN_5:
      mpSimValueAD5.Attach(pSubject);
      break;
    case SP_GATM_IOB_BOARD_ID:
      mpIobBoardId.Attach(pSubject);
      break;
    case SP_GATM_IOB_TEMPERATURE:
      mpIobTemperature.Attach(pSubject);
      break;
    case SP_GATM_IOB_PRESSURE:
      mpIobPressure.Attach(pSubject);
      break;
    case SP_GATM_IOB_BATTERY_VOLTAGE:
      mpIobBatteryVoltage.Attach(pSubject);
      break;
    case SP_GATM_IOB_BUS_MODULE_PRESSENT:
      mpIobBusModulePressent.Attach(pSubject);
      break;
    case SP_GATM_IOB_SUPPLY_STATUS:
      mpIobSupplyStatus.Attach(pSubject);
      break;
    case SP_GATM_IOB_BATTERY_STATUS:
      mpIobBatteryStatus.Attach(pSubject);
      break;

    case SP_GATM_ANALOG_INPUT_0:
      mpAnalogInput0.Attach(pSubject);
      break;
    case SP_GATM_ANALOG_INPUT_1:
      mpAnalogInput1.Attach(pSubject);
      break;
    case SP_GATM_ANALOG_INPUT_2:
      mpAnalogInput2.Attach(pSubject);
      break;
    case SP_GATM_ANALOG_INPUT_3:
      mpAnalogInput3.Attach(pSubject);
      break;
    case SP_GATM_ANALOG_INPUT_4:
      mpAnalogInput4.Attach(pSubject);
      break;
    case SP_GATM_ANALOG_INPUT_5:
      mpAnalogInput5.Attach(pSubject);
      break;
    case SP_GATM_DIGITAL_INPUT:
      mpDigitalInput.Attach(pSubject);
      break;

    default:
      break;
  }
}

/*****************************************************************************
 * Function - GetTestmode
 * DESCRIPTION: Return true if test mode is enabled
 *
 *****************************************************************************/
bool GeniAppTestMode::GetTestMode(void)
{
  return mTestModeEnabled;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function EnableTestMode
 * DESCRIPTION: TRUE: Enables GET and SET access to GENI Class 6
 *              FALSE: Disables GET and SET access to GENI Class 6
 *****************************************************************************/
void GeniAppTestMode::EnableTestMode(bool new_mode)
{
  if (mTestModeEnabled == false)
  {
    if (new_mode == true)
    {
      //Enable Class 6 Access
      ClassAcc(TEST_APDU, GET_ACC, Enable);
      ClassAcc(TEST_APDU, SET_ACC, Enable);

      G_ge_display_backlight = mpDisplayBacklight->GetValueAsPercent()*2.54f + 0.5f;
      G_ge_display_contrast = mpDisplayContrast->GetValueAsPercent()*2.54f + 0.5f;

      mTestModeEnabled = true;
    }
  }
  else
  {
    if (new_mode == false)
    {
      //Disable Class 6 Access
      ClassAcc(TEST_APDU, GET_ACC, Disable);
      ClassAcc(TEST_APDU, SET_ACC, Disable);
      ClearGeniTestData();
      mTestModeEnabled = false;
    }
  }
  mpTestModeStatus->SetValue(mTestModeEnabled);
}

/*****************************************************************************
 * Function HandleGeniTestData
 * DESCRIPTION:
 *****************************************************************************/
void GeniAppTestMode::HandleGeniTestData(void)
{
  static bool firmware_update_started = false;
  static bool io_simulation_mode_enable = false;

  G_ge_io_simulation_status = mpIOSimulationStatus->GetValue();

  /* Handle G_ge_test_config */
  HandleTestConfig(G_ge_test_config);

  if (G_ge_io_simulation_mode_enable != IO_SIMULATION_ENABLED)
  {
    if (io_simulation_mode_enable == true)
    {
      mpIOSimulationDisable->SetEvent();
      HandleIOSimulation();  // To clear simulation data
      io_simulation_mode_enable = false;
    }
    // Normal test functions
    /* Handle G_ge_dig_out */
    IobComDrv::GetInstance()->SetDigitalOutputTestMode(G_ge_dig_out_ref);
    // Display
    mpDisplayContrast->SetValueAsPercent(G_ge_display_contrast/2.54f);
    mpDisplayBacklight->SetValueAsPercent(G_ge_display_backlight/2.54f);
  }
  else
  {
    // Handle IO simulation
    if (io_simulation_mode_enable == false)
    {
      mpIOSimulationEnable->SetEvent();
      io_simulation_mode_enable = true;
    }
    HandleIOSimulation();
  }


  /* Handle G_ge_error_log_control */
  if (TEST_BIT_HIGH(G_ge_error_log_control, 0)) //Reset ErrorLog
  {
    SET_BIT_LOW(G_ge_error_log_control, 0);
    ErrorLog::GetInstance()->ResetErrorLog();
  }

  /* Handle G_ge_firmware_update_state */
  if (G_ge_firmware_update_state == FIRMWARE_UPDATE_STATE_START && firmware_update_started == false)
  {
    mpFirmwareUpdateState->SetValue(FIRMWARE_UPDATE_STATE_START);
    firmware_update_started = true;
  }
  else if (G_ge_firmware_update_state == FIRMWARE_UPDATE_STATE_STARTBL && firmware_update_started == false)
  {
    mpFirmwareUpdateState->SetValue(FIRMWARE_UPDATE_STATE_STARTBL);
    firmware_update_started = true;
  }
  G_ge_firmware_update_state = mpFirmwareUpdateState->GetValue();

  if (G_ge_firmware_update_state != FIRMWARE_UPDATE_STATE_START && G_ge_firmware_update_state != FIRMWARE_UPDATE_STATE_STARTBL)
  {
    firmware_update_started = false;
  }

  /* Handle G_ge_test_loop_displays */
  if (G_ge_test_loop_displays == 1)

  {
    G_ge_test_loop_displays = 0;

    mpc::display::DisplayController::GetInstance()->TestLoopDisplays();
  }
  /* Handle G_ge_emwin_key_input */
  else if (G_ge_emwin_key_input != 255)
  {
    GUI_StoreKey(G_ge_emwin_key_input);
    G_ge_emwin_key_input = 255;
  }

  if (mpIobBoardId.IsUpdated())
  {
    G_ge_iob_board_id = mpIobBoardId->GetValue();
  }
  if (mpIobTemperature.IsUpdated())
  {
    G_ge_iob_temperature = (U16)(mpIobTemperature->GetValue() + 273.15f + 0.5f); // K
  }
  if (mpIobPressure.IsUpdated())
  {
    G_ge_iob_pressure = (U16)(mpIobPressure->GetValue()*0.01f + 0.5f); // Scale from Pa to mbar;
  }
  if (mpIobBatteryVoltage.IsUpdated())
  {
    G_ge_iob_battery_voltage = (U16)(mpIobBatteryVoltage->GetValue()*1000.0f + 0.5f); // Scale from V to mV
  }
  if (mpIobBusModulePressent.IsUpdated())
  {
    G_ge_iob_bus_module_pressent = mpIobBusModulePressent->GetValue();
  }
  if (mpIobSupplyStatus.IsUpdated())
  {
    G_ge_iob_supply_status = mpIobSupplyStatus->GetValue();
  }
  if (mpIobBatteryStatus.IsUpdated())
  {
    G_ge_iob_battery_status = mpIobBatteryStatus->GetValue();
  }
  if (mpAnalogInput0.IsUpdated())
  {
    G_ge_ana_in_raw_0 = mpAnalogInput0->GetValue();
  }
  if (mpAnalogInput1.IsUpdated())
  {
    G_ge_ana_in_raw_1 = mpAnalogInput1->GetValue();
  }
  if (mpAnalogInput2.IsUpdated())
  {
    G_ge_ana_in_raw_2 = mpAnalogInput2->GetValue();
  }
  if (mpAnalogInput3.IsUpdated())
  {
    G_ge_ana_in_raw_3 = mpAnalogInput3->GetValue();
  }
  if (mpAnalogInput4.IsUpdated())
  {
    G_ge_ana_in_raw_4 = mpAnalogInput4->GetValue();
  }
  if (mpAnalogInput5.IsUpdated())
  {
    G_ge_ana_in_raw_5 = mpAnalogInput5->GetValue();
  }
  if (mpDigitalInput.IsUpdated())
  {
    G_ge_dig_in = mpDigitalInput->GetValue();
  }
}

/*****************************************************************************
 * Function ClearGeniTestData
 * DESCRIPTION:
 *****************************************************************************/
void GeniAppTestMode::ClearGeniTestData(void)
{
  G_ge_test_config = 0;
  G_ge_dig_out_ref = 0;
  HandleGeniTestData();

  G_ge_io_simulation_mode_enable = 0;
  HandleIOSimulation();
}

/*****************************************************************************
 * Function HandleIOSimulation
 * DESCRIPTION:
 *****************************************************************************/
void GeniAppTestMode::HandleIOSimulation(void)
{
  if ( G_ge_io_simulation_mode_enable != IO_SIMULATION_ENABLED)
  {
    mpIOSimulationDisable->SetEvent();
    G_ge_digital_input_simulation_mode = 0;
    G_ge_analog_input_simulation_mode = 0;
  }

  mpIOSimulationMode->SetValue((G_ge_digital_input_simulation_mode<<8) + G_ge_analog_input_simulation_mode);
  mpSimDigitalInputs->SetValue(G_ge_sim_digital_inputs);
  mpSimValueAD0->SetValue(G_ge_sim_value_ad_0);
  mpSimValueAD1->SetValue(G_ge_sim_value_ad_1);
  mpSimValueAD2->SetValue(G_ge_sim_value_ad_2);
  mpSimValueAD3->SetValue(G_ge_sim_value_ad_3);
  mpSimValueAD4->SetValue(G_ge_sim_value_ad_4);
  mpSimValueAD5->SetValue(G_ge_sim_value_ad_5);
}

/*****************************************************************************
 * Function HandleTestConfig
 * DESCRIPTION:
 *****************************************************************************/
void GeniAppTestMode::HandleTestConfig(U8 new_config)
{
  static mpc::display::DisplayController::TEST_SCREEN_TYPE display_test_picture_old = mpc::display::DisplayController::NORMAL;

  if (TEST_BIT_HIGH(new_config, DIG_OUT_TEST_BIT))
  {
    IobComDrv::GetInstance()->SetTestModeFlag();
  }
  else
  {
    IobComDrv::GetInstance()->ClearTestModeFlag();
  }
  if (TEST_BIT_HIGH(new_config, LEDS_ON_OFF_TEST_BIT))
  {
    GPio::GetInstance()->SetLedTest(true);
  }
  else
  {
    GPio::GetInstance()->SetLedTest(false);
  }
  if (TEST_BIT_HIGH(new_config, TEST_PICTURE_TEST_BIT))
  {
    if (display_test_picture_old != G_ge_display_test_picture)
    {
      mpc::display::DisplayController::GetInstance()->TestScreen((mpc::display::DisplayController::TEST_SCREEN_TYPE)G_ge_display_test_picture);
      display_test_picture_old = (mpc::display::DisplayController::TEST_SCREEN_TYPE)G_ge_display_test_picture;
    }
  }
  else
  {
    if (display_test_picture_old != mpc::display::DisplayController::NORMAL)
    {
      mpc::display::DisplayController::GetInstance()->TestScreen(mpc::display::DisplayController::NORMAL);
      display_test_picture_old = mpc::display::DisplayController::NORMAL;
    }
  }
  if (TEST_BIT_HIGH(new_config, UART0_LB_TEST_BIT))
  {
    StartUartLoopBackTest();
  }
  else
  {
    StopUartLoopBackTest();
  }
}

/*****************************************************************************
 * Function GetTestStatus
 * DESCRIPTION:
 *****************************************************************************/
void GeniAppTestMode::GetTestStatus(void)
{
  /* Get EthLoppBackTestStatus*/
  if (G_ge_eth_loop_back_test == ETH_NOT_TESTED || G_ge_eth_loop_back_test == ETH_TESTING)
  {
    G_ge_eth_loop_back_test = (U8)EthernetCtrl::GetInstance()->GetEthernetLoopBackTestStatus();
  }


  /* Get KeyboardStatus */
  G_ge_keyboard_status = GetKeyStates();

  /*Get SD RAM test status */
  //G_ge_sd_ram_test = GetSdRamTestStatus();

  G_ge_uart0_loop_back_test = GetUartLoopBackTestStatus();

  /* Get HW-Info */
  G_ge_hw_info = HW_INFO;


  if(G_ge_iob_supply_status == 1)
  {
	G_ge_iob_battery_status = 1;
  }
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
