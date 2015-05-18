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
/* CLASS NAME       : bu_tab_test                                           */
/*                                                                          */
/* FILE NAME        : bu_tab_test.c                                         */
/*                                                                          */
/* CREATED DATE     : 03-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Bus unit table for GeniPro class 6 interface (test)   */
/*                                                                          */
/****************************************************************************/

/***************************** Include files *******************************/
#include "geni_cnf.h"
#include "profiles.h"
#include "GeniTestModeData.h"

/*****************************    Defines    *******************************/

#define UC_ADDR_3_LI(X) ((UCHAR *)&X+3) /* Insert address of byte 3 of a long int, including UCHAR typecast */
#define UC_ADDR_2_LI(X) ((UCHAR *)&X+2) /* Insert address of byte 2 of a long int, including UCHAR typecast */
#define UC_ADDR_1_LI(X) ((UCHAR *)&X+1) /* Insert address of byte 1 of a long int, including UCHAR typecast */
#define UC_ADDR_0_LI(X) ((UCHAR *)&X)   /* Insert address of byte 0 of a long int, including UCHAR typecast */

/*****************************   Constants   *******************************/

const ID_PTR test_tab[HIGH_TEST_ID+1] =
{
  0,                                                /*  ID   0              */
  &G_ge_sd_ram_test,                                /*  ID   1              */
  &G_ge_eth_loop_back_test,                         /*  ID   2              */
  &G_ge_hw_info,                                    /*  ID   3              */
  &G_ge_display_test_picture,                       /*  ID   4              */
  &G_ge_test_config,                                /*  ID   5              */
  &G_ge_dig_out_ref,                                /*  ID   6              */
  NA,                                               /*  ID   7              */
  &G_ge_firmware_update_state,                      /*  ID   8              */
  NA,                                               /*  ID   9              */
  UC_ADDR_0_LI(G_ge_keyboard_status),               /*  ID   10             */
  UC_ADDR_1_LI(G_ge_keyboard_status),               /*  ID   11             */
  &G_ge_test_loop_displays,                         /*  ID   12             */
  &G_ge_uart0_loop_back_test,                       /*  ID   13             */
  &G_ge_iob_board_id,                               /*  ID   14             */
  UC_ADDR_1_LI(G_ge_iob_temperature),               /*  ID   15             */
  UC_ADDR_0_LI(G_ge_iob_temperature),               /*  ID   16             */
  UC_ADDR_1_LI(G_ge_iob_pressure),                  /*  ID   17             */
  UC_ADDR_0_LI(G_ge_iob_pressure),                  /*  ID   18             */
  UC_ADDR_1_LI(G_ge_iob_battery_voltage),           /*  ID   19             */
  UC_ADDR_0_LI(G_ge_iob_battery_voltage),           /*  ID   20             */
  &G_ge_iob_bus_module_pressent,                    /*  ID   21             */
  &G_ge_iob_supply_status,                          /*  ID   22             */
  &G_ge_iob_battery_status,                         /*  ID   23             */
  NA,                                               /*  ID   24             */
  NA,                                               /*  ID   25             */
  NA,                                               /*  ID   26             */
  NA,                                               /*  ID   27             */
  &G_ge_display_backlight,                          /*  ID   28             */
  &G_ge_display_contrast,                           /*  ID   29             */
  &G_ge_error_log_control,                          /*  ID   30             */
  UC_ADDR_1_LI(G_ge_max_controller_event_queue_size),/* ID   31             */
  UC_ADDR_0_LI(G_ge_max_controller_event_queue_size),/* ID   32             */
  UC_ADDR_1_LI(G_ge_max_display_event_queue_size),  /*  ID   33             */
  UC_ADDR_0_LI(G_ge_max_display_event_queue_size),  /*  ID   34             */
  NA,                                               /*  ID   35             */
  NA,                                               /*  ID   36             */
  NA,                                               /*  ID   37             */
  NA,                                               /*  ID   38             */
  NA,                                               /*  ID   39             */
  NA,                                               /*  ID   40             */
  NA,                                               /*  ID   41             */
  UC_ADDR_0_LI(G_ge_emwin_key_input),               /*  ID   42             */
  &G_ge_io_simulation_status,                       /*  ID   43             */
  &G_ge_io_simulation_mode_enable,                  /*  ID   44             */
  &G_ge_digital_input_simulation_mode,              /*  ID   45             */
  &G_ge_sim_digital_inputs,                         /*  ID   46             */
  &G_ge_analog_input_simulation_mode,               /*  ID   47             */
  UC_ADDR_1_LI(G_ge_sim_value_ad_0),                /*  ID   48             */
  UC_ADDR_0_LI(G_ge_sim_value_ad_0),                /*  ID   49             */
  UC_ADDR_1_LI(G_ge_sim_value_ad_1),                /*  ID   50             */
  UC_ADDR_0_LI(G_ge_sim_value_ad_1),                /*  ID   51             */
  UC_ADDR_1_LI(G_ge_sim_value_ad_2),                /*  ID   52             */
  UC_ADDR_0_LI(G_ge_sim_value_ad_2),                /*  ID   53             */
  UC_ADDR_1_LI(G_ge_sim_value_ad_3),                /*  ID   54             */
  UC_ADDR_0_LI(G_ge_sim_value_ad_3),                /*  ID   55             */
  UC_ADDR_1_LI(G_ge_sim_value_ad_4),                /*  ID   56             */
  UC_ADDR_0_LI(G_ge_sim_value_ad_4),                /*  ID   57             */
  UC_ADDR_1_LI(G_ge_sim_value_ad_5),                /*  ID   58             */
  UC_ADDR_0_LI(G_ge_sim_value_ad_5),                /*  ID   59             */
  UC_ADDR_0_LI(G_ge_dig_in),                        /*  ID   60             */
  UC_ADDR_1_LI(G_ge_dig_in),                        /*  ID   61             */
  UC_ADDR_2_LI(G_ge_dig_in),                        /*  ID   62             */
  UC_ADDR_1_LI(G_ge_ana_in_raw_0),                  /*  ID   63             */
  UC_ADDR_0_LI(G_ge_ana_in_raw_0),                  /*  ID   64             */
  UC_ADDR_1_LI(G_ge_ana_in_raw_1),                  /*  ID   65             */
  UC_ADDR_0_LI(G_ge_ana_in_raw_1),                  /*  ID   66             */
  UC_ADDR_1_LI(G_ge_ana_in_raw_2),                  /*  ID   67             */
  UC_ADDR_0_LI(G_ge_ana_in_raw_2),                  /*  ID   68             */
  UC_ADDR_1_LI(G_ge_ana_in_raw_3),                  /*  ID   69             */
  UC_ADDR_0_LI(G_ge_ana_in_raw_3),                  /*  ID   70             */
  UC_ADDR_1_LI(G_ge_ana_in_raw_4),                  /*  ID   71             */
  UC_ADDR_0_LI(G_ge_ana_in_raw_4),                  /*  ID   72             */
  UC_ADDR_1_LI(G_ge_ana_in_raw_5),                  /*  ID   73             */
  UC_ADDR_0_LI(G_ge_ana_in_raw_5)                   /*  ID   74             */
};
