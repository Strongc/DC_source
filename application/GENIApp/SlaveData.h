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
/* MODULE NAME      : slaveData                                             */
/*                                                                          */
/* FILE NAME        : slaveData.h                                           */
/*                                                                          */
/* CREATED DATE     : 01-04-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :  Gives access to global geni slave data         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __SLAVE_DATA_H__
#define __SLAVE_DATA_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <common.h>
#include <binary.h>
#include <InfoDef.h> //Access to GENI_INFO_SIZE

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
//rare in the .h file

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_NUMBER_OF_IO_MODULES   3
#define MAX_NUMBER_OF_IO_111       6
#define MAX_NUMBER_OF_CUE          6
#define MAX_NUMBER_OF_UNITS        MAX_NUMBER_OF_IO_MODULES + MAX_NUMBER_OF_IO_111 + MAX_NUMBER_OF_CUE

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
#define NO_DEVICE                 NO_DEVICES
#define DEVICE_E_PUMP             DEVICE1
#define DEVICE_MP204              DEVICE2
#define DEVICE_IO351_IO_MODULE    DEVICE3
#define DEVICE_IO351_PUMP_MODULE  DEVICE4
#define DEVICE_IO111              DEVICE5

/*****************************************************************************
 * Prototypes:
 * DESCRIPTION: Gives access to global geni slave variables. Only to be used by
 * GENIpro sw
 *
 ******************************************************************************/
                                            //CUE Parameter      IO 351 Parameter       IO111 Parameter   MP204 Parameter
                                            //--------------     ----------------       ----------------  ----------------
EXTERN U8 s_cl2_id020[MAX_NUMBER_OF_UNITS]; //                   s_ana_in_1_hi
EXTERN U8 s_cl2_id021[MAX_NUMBER_OF_UNITS]; //                   s_ana_in_1_lo          s_t_support_bear
EXTERN U8 s_cl2_id022[MAX_NUMBER_OF_UNITS]; //                   s_ana_in_2_hi          s_t_main_bear
EXTERN U8 s_cl2_id023[MAX_NUMBER_OF_UNITS]; //                   s_ana_in_2_lo
EXTERN U8 s_cl2_id024[MAX_NUMBER_OF_UNITS]; //                   s_dig_in_1
EXTERN U8 s_cl2_id025[MAX_NUMBER_OF_UNITS]; //                   s_dig_in_2
EXTERN U8 s_cl2_id026[MAX_NUMBER_OF_UNITS]; //                   s_dig_out
EXTERN U8 s_cl2_id027[MAX_NUMBER_OF_UNITS]; //s_v_dc                                    s_t_pt100
EXTERN U8 s_cl2_id029[MAX_NUMBER_OF_UNITS]; //                                          s_t_pt1000
EXTERN U8 s_cl2_id030[MAX_NUMBER_OF_UNITS]; //s_i_mo                                    s_r_insulate
EXTERN U8 s_cl2_id032[MAX_NUMBER_OF_UNITS]; //s_f_act
EXTERN U8 s_cl2_id033[MAX_NUMBER_OF_UNITS]; //s_p_hi                                    speed_hi (IO113)
EXTERN U8 s_cl2_id034[MAX_NUMBER_OF_UNITS]; //s_p_lo                                    speed_lo (IO113)
EXTERN U8 s_cl2_id035[MAX_NUMBER_OF_UNITS]; //s_speed_hi                                                  v_line_hi
EXTERN U8 s_cl2_id036[MAX_NUMBER_OF_UNITS]; //s_speed_lo                                                  v_line_lo
EXTERN U8 s_cl2_id043[MAX_NUMBER_OF_UNITS]; //                                                            i_line_hi
EXTERN U8 s_cl2_id044[MAX_NUMBER_OF_UNITS]; //                                                            i_line_lo
EXTERN U8 s_cl2_id045[MAX_NUMBER_OF_UNITS]; //s_torque_hi
EXTERN U8 s_cl2_id046[MAX_NUMBER_OF_UNITS]; //s_torque_lo
EXTERN U8 s_cl2_id047[MAX_NUMBER_OF_UNITS]; //                                                            t_mo1 (Tempcon)
EXTERN U8 s_cl2_id048[MAX_NUMBER_OF_UNITS]; //                                                            t_mo2 (PT resis)
EXTERN U8 s_cl2_id049[MAX_NUMBER_OF_UNITS]; //                                                            i_asym
EXTERN U8 s_cl2_id050[MAX_NUMBER_OF_UNITS]; //                                          s_vibration
EXTERN U8 s_cl2_id060[MAX_NUMBER_OF_UNITS]; //                                          s_water_in_oil
EXTERN U8 s_cl2_id064[MAX_NUMBER_OF_UNITS]; //                                                            cos phi
EXTERN U8 s_cl2_id065[MAX_NUMBER_OF_UNITS]; //                                                            p_hi
EXTERN U8 s_cl2_id066[MAX_NUMBER_OF_UNITS]; //                                                            p_lo1
EXTERN U8 s_cl2_id067[MAX_NUMBER_OF_UNITS]; //                                                            p_lo2
EXTERN U8 s_cl2_id068[MAX_NUMBER_OF_UNITS]; //                                                            p_lo3
EXTERN U8 s_cl2_id069[MAX_NUMBER_OF_UNITS]; //                                                            energy_hi
EXTERN U8 s_cl2_id070[MAX_NUMBER_OF_UNITS]; //                                                            energy_lo1
EXTERN U8 s_cl2_id071[MAX_NUMBER_OF_UNITS]; //                                                            energy_lo2
EXTERN U8 s_cl2_id072[MAX_NUMBER_OF_UNITS]; //                                                            energy_lo3
EXTERN U8 s_cl2_id081[MAX_NUMBER_OF_UNITS]; //s_act_mode1
EXTERN U8 s_cl2_id083[MAX_NUMBER_OF_UNITS]; //s_act_mode3
EXTERN U8 s_cl2_id086[MAX_NUMBER_OF_UNITS]; //s_t_remote_2
EXTERN U8 s_cl2_id094[MAX_NUMBER_OF_UNITS]; //                                                            r_insulation
EXTERN U8 s_cl2_id095[MAX_NUMBER_OF_UNITS]; //drive_modes1
EXTERN U8 s_cl2_id099[MAX_NUMBER_OF_UNITS]; //                                                            dig_in
EXTERN U8 s_cl2_id141[MAX_NUMBER_OF_UNITS]; //                                          s_alarm_code
EXTERN U8 s_cl2_id142[MAX_NUMBER_OF_UNITS]; //                                          s_warning_code
EXTERN U8 s_cl2_id144[MAX_NUMBER_OF_UNITS]; //                                          digital_sensors   alarm_code
EXTERN U8 s_cl2_id145[MAX_NUMBER_OF_UNITS]; //                                                            warnings 1
EXTERN U8 s_cl2_id146[MAX_NUMBER_OF_UNITS]; //                                                            warnings 2
EXTERN U8 s_cl2_id147[MAX_NUMBER_OF_UNITS]; //                                                            warnings 3
EXTERN U8 s_cl2_id148[MAX_NUMBER_OF_UNITS]; //unit_family                               unit_family       unit_family
EXTERN U8 s_cl2_id149[MAX_NUMBER_OF_UNITS]; //                                          unit_type (IO11x)

EXTERN U8 s_cl2_id151[MAX_NUMBER_OF_UNITS]; //s_energy_hi
EXTERN U8 s_cl2_id152[MAX_NUMBER_OF_UNITS]; //s_energy_lo1
EXTERN U8 s_cl2_id153[MAX_NUMBER_OF_UNITS]; //s_energy_lo2
EXTERN U8 s_cl2_id156[MAX_NUMBER_OF_UNITS]; //s_warning_code     s_warnings
EXTERN U8 s_cl2_id158[MAX_NUMBER_OF_UNITS]; //s_alarm_code
EXTERN U8 s_cl2_id195[MAX_NUMBER_OF_UNITS]; //                   s_power_on_cnt_hi
EXTERN U8 s_cl2_id196[MAX_NUMBER_OF_UNITS]; //                   s_power_on_cnt_lo

EXTERN U8 s_cl4_id030[MAX_NUMBER_OF_UNITS]; //f_upper

EXTERN U8 s_cl5_id001[MAX_NUMBER_OF_UNITS]; //s_motor_reference  ana_out_1_set - IO351
EXTERN U8 s_cl5_id002[MAX_NUMBER_OF_UNITS]; //                   ana_out_2_set - IO351
EXTERN U8 s_cl5_id003[MAX_NUMBER_OF_UNITS]; //                   ana_out_3_set - IO351
EXTERN U8 s_cl5_id007[MAX_NUMBER_OF_UNITS]; //                   dig_in_1_cnt_hi
EXTERN U8 s_cl5_id008[MAX_NUMBER_OF_UNITS]; //                   dig_in_1_cnt_lo1
EXTERN U8 s_cl5_id009[MAX_NUMBER_OF_UNITS]; //                   dig_in_1_cnt_lo2
EXTERN U8 s_cl5_id010[MAX_NUMBER_OF_UNITS]; //                   dig_in_1_cnt_lo3
EXTERN U8 s_cl5_id011[MAX_NUMBER_OF_UNITS]; //                   dig_in_2_cnt_hi
EXTERN U8 s_cl5_id012[MAX_NUMBER_OF_UNITS]; //                   dig_in_2_cnt_lo1
EXTERN U8 s_cl5_id013[MAX_NUMBER_OF_UNITS]; //                   dig_in_2_cnt_lo2
EXTERN U8 s_cl5_id014[MAX_NUMBER_OF_UNITS]; //                   dig_in_2_cnt_lo3


                                                                 //CUE Parameter
                                                                 //--------------
EXTERN U8 s_cl2_id027_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_v_dc
EXTERN U8 s_cl2_id030_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_i_mo
EXTERN U8 s_cl2_id032_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_f_act
EXTERN U8 s_cl2_id033_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_p_hi
            /* id034 - s_p_lo - no info here, info for id033 hi byte is used */
EXTERN U8 s_cl2_id035_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_speed_hi
            /* id036 - s_speed_lo - no info here, info for id035 hi byte is used */
EXTERN U8 s_cl2_id045_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_torque_hi
            /* id046 - s_torque_lo - no info here, info for id045 hi byte is used */
EXTERN U8 s_cl2_id086_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_t_remote_2
EXTERN U8 s_cl2_id151_info[MAX_NUMBER_OF_UNITS][GENI_INFO_SIZE]; //s_energy_hi
            /*  id152 - s_energy_lo1 - no info here, info for id151 is used */
            /*  id153 - s_energy_lo2 - no info here, info for id151 is used */

#endif
