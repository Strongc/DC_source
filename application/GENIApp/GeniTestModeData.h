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
/* CLASS NAME       : GeniTestModeData                                      */
/*                                                                          */
/* FILE NAME        : GeniTestModeData.h                                    */
/*                                                                          */
/* CREATED DATE     : 01-06-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Access to Geni Test Mode Bus Data               */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcGeniTestModeData_h
#define mpcGeniTestModeData_h

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

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  Global variables
 *****************************************************************************/
EXTERN U8 G_ge_firmware_update_state;

EXTERN U8 G_ge_error_log_control;

EXTERN U8 G_ge_test_config;
EXTERN U8 G_ge_dig_out_ref;

EXTERN U8 G_ge_hw_info;
EXTERN U8 G_ge_eth_loop_back_test;
EXTERN U8 G_ge_uart0_loop_back_test;
EXTERN U8 G_ge_sd_ram_test;
EXTERN U16 G_ge_keyboard_status;

EXTERN U16 G_ge_max_controller_event_queue_size;
EXTERN U16 G_ge_max_display_event_queue_size;

EXTERN U8 G_ge_display_backlight;
EXTERN U8 G_ge_display_contrast;
EXTERN U8 G_ge_test_loop_displays;
EXTERN U8 G_ge_display_test_picture;
EXTERN U16 G_ge_emwin_key_input;

// Data for IO simulation
EXTERN U8 G_ge_io_simulation_status;
EXTERN U8 G_ge_io_simulation_mode_enable;
EXTERN U8 G_ge_digital_input_simulation_mode;
EXTERN U8 G_ge_sim_digital_inputs;
EXTERN U8 G_ge_analog_input_simulation_mode;
EXTERN U16 G_ge_sim_value_ad_0;
EXTERN U16 G_ge_sim_value_ad_1;
EXTERN U16 G_ge_sim_value_ad_2;
EXTERN U16 G_ge_sim_value_ad_3;
EXTERN U16 G_ge_sim_value_ad_4;
EXTERN U16 G_ge_sim_value_ad_5;

EXTERN char G_ge_cpu_software_version[30];
EXTERN char G_ge_io_software_version[30];
EXTERN char G_ge_boot_load_software_version[30];

EXTERN U8 G_ge_iob_board_id;
EXTERN U16 G_ge_iob_temperature;
EXTERN U16 G_ge_iob_pressure;
EXTERN U16 G_ge_iob_battery_voltage;
EXTERN U8 G_ge_iob_bus_module_pressent;
EXTERN U8 G_ge_iob_supply_status;
EXTERN U8 G_ge_iob_battery_status;

EXTERN U32 G_ge_dig_in;
EXTERN U16 G_ge_ana_in_raw_0;
EXTERN U16 G_ge_ana_in_raw_1;
EXTERN U16 G_ge_ana_in_raw_2;
EXTERN U16 G_ge_ana_in_raw_3;
EXTERN U16 G_ge_ana_in_raw_4;
EXTERN U16 G_ge_ana_in_raw_5;



/*****************************************************************************
* CLASS:
* DESCRIPTION:
*
*****************************************************************************/
#endif
