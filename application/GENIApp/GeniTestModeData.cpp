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
/* MODULE NAME      : GeniTestModeData                                      */
/*                                                                          */
/* FILE NAME        : GeniTestModeData.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Global variables used to hold geni test data    */
/* Only used by GENI sw                                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <GeniTestModeData.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  C functions declarations
 *****************************************************************************/

/*****************************************************************************
  Global variables
 *****************************************************************************/

U8 G_ge_firmware_update_state;

U8 G_ge_error_log_control;

U8 G_ge_test_config;
U8 G_ge_dig_out_ref;

U8 G_ge_hw_info;
U8 G_ge_eth_loop_back_test;
U8 G_ge_uart0_loop_back_test;
U8 G_ge_sd_ram_test;
U16 G_ge_keyboard_status;

U16 G_ge_max_controller_event_queue_size;
U16 G_ge_max_display_event_queue_size;

U8 G_ge_display_backlight;
U8 G_ge_display_contrast;
U8 G_ge_test_loop_displays;
U8 G_ge_display_test_picture;
U16 G_ge_emwin_key_input;

// Data for IO simulation
U8 G_ge_io_simulation_status;
U8 G_ge_io_simulation_mode_enable;
U8 G_ge_digital_input_simulation_mode;
U8 G_ge_sim_digital_inputs;
U8 G_ge_analog_input_simulation_mode;
U16 G_ge_sim_value_ad_0;
U16 G_ge_sim_value_ad_1;
U16 G_ge_sim_value_ad_2;
U16 G_ge_sim_value_ad_3;
U16 G_ge_sim_value_ad_4;
U16 G_ge_sim_value_ad_5;

char G_ge_cpu_software_version[30];
char G_ge_io_software_version[30];
char G_ge_boot_load_software_version[30];

U8 G_ge_iob_board_id;
U16 G_ge_iob_temperature;
U16 G_ge_iob_pressure;
U16 G_ge_iob_battery_voltage;
U8 G_ge_iob_bus_module_pressent;
U8 G_ge_iob_supply_status;
U8 G_ge_iob_battery_status;

U32 G_ge_dig_in;
U16 G_ge_ana_in_raw_0;
U16 G_ge_ana_in_raw_1;
U16 G_ge_ana_in_raw_2;
U16 G_ge_ana_in_raw_3;
U16 G_ge_ana_in_raw_4;
U16 G_ge_ana_in_raw_5;


/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/
