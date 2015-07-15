/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:    GENIpro                                       */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      : vir_tab_master.c                                      */
/*                                                                          */
/* FILE NAME        : vir_tab_master.c                                      */
/*                                                                          */
/* FILE DESCRIPTION : Table module for master application                   */
/*                                                                          */
/****************************************************************************/
#include "typedef.h"
#include "geni_cnf.h"                       /* Access to configuration      */
#include "common.h"                         /* Access to common definitions */
#include "profiles.h"                       /* Access to channel profiles   */
#include "geni_sys.h"                       /* Access to system             */
#include "vir_dia_master.h"                 /* Access to dialog layer       */
#include "vir_ctr_master.h"                 /* Access to control            */
#include "vir_tab_master.h"                 /* Acces to it self             */

#include "slavedata.h"                      /* Access to DEVICE names       */

const UCHAR io_module_status_fast[] = // IO351, class 2
{
  20,     // ana_in_1_hi
  21,     // ana_in_1_lo
  22,     // ana_in_2_hi
  23,     // ana_in_2_lo
  24,     // dig_in_1
  25,     // dig_in_2
  26,     // dig_out
  156,    // warnings
  195,    // power_on_cnt_hi
  196     // power_on_cnt_lo
};

const UCHAR io_module_counters[] = // IO351, class 5
{
  10,     // dig_in_1_cnt_lo3
  14,     // dig_in_2_cnt_lo3
};

const UCHAR io111_status_fast[] = // IO111
{
  21,     // t_support_bear
  22,     // t_main_bear
  27,     // t_pt100
  29,     // t_pt1000
  30,     // r_insulate
  50,     // vibration
  60,     // water_in_oil
  141,    // alarm_code
  142,    // warning_code
  144,    // alarms2 (Moistere, Thermal)
  33,     // speed_hi (IO113)
  34,     // speed_lo (IO113)
  149     // unit_type
};

const UCHAR io_module_ref[] = // IO351, class 5
{
  1,      // ana_out_4_set
  2,      // ana_out_5_set
  3       // ana_out_6_set
};

const UCHAR cue_module_status_fast[] = // CUE, class 2
{
  81,     // act_mode1
  83,     // act_mode3
  95,     // drive_modes1
  156,    // warning_code
  158     // alarm_code
};

const UCHAR cue_module_measurements[] = // CUE, class 2
{
  27,     // v_dc
  30,     // i_mo
  32,     // f_act
  33,     // p_hi
  34,     // p_lo
  35,     // speed_hi
  36,     // speed_lo
  45,     // torque_hi
  46,     // torque_lo
  86,     // t_remote_2
  148,    // unit family
  151,    // energy_hi
  152,    // energy_lo1
  153     // energy_lo2
};

const UCHAR cue_module_f_upper[] = // CUE, class 4
{
  30      // f_upper
};

const UCHAR cue_module_ref[] = // CUE, class 5
{
  1       // ref_rem
};

const UCHAR mp204_status_fast[] = // MP204 class 2
{
  144,    // alarm code
  145,    // warning 1
  146,    // warning 1
  147     // warning 1
};

const UCHAR mp204_status_slow[] = // MP204 class 2
{
  35,     // voltage hi
  36,     // voltage lo
  43,     // current hi
  44,     // current hi
  47,     // temperature tempcon
  48,     // temperature pt
  49,     // current asymmetry
  64,     // cos phi
  65,     // power hi
  66,     // power lo1
  67,     // power lo2
  68,     // power lo3
  69,     // energy hi
  70,     // energy lo1
  71,     // energy lo2
  72,     // energy lo3
  94,     // insulation resistance
  99,     // temperature ptc
  148,    // unit family
};

const UCHAR DDA_module_capability[] = // DDA, class 2
{
    0,	// pressure_max 
    1,	// dosing_cap_max_hi 
    2,	// dosing_cap_max_lo1
    3,	// dosing_cap_max_lo2
    8,	// dosing_cap_ref_hi
    9,	// dosing_cap_ref_lo1
    10,	// dosing_cap_ref_lo2
    11,	// dosing_cap_ref_lo3
};

const UCHAR DDA_module_status[] = // DDA, class 2
{
    12,	// flow_mon_dosing_cap_hi
    13,	// flow_mon_dosing_cap_lo1
    14,	// flow_mon_dosing_cap_lo2
    15,	// flow_mon_dosing_cap_lo3
    16,	// flow_mon_press
    71, // volume_total_hi
    72, // volume_total_lo1
    73, // volume_total_lo2
    74, // volume_total_lo3
    80, // system_mode
    81, // operating_mode
    82, // control_mode
    85, // stop_ctr_state
    86, // ctr_source
    87, // pumping_state
    148, // unit family
    232, // warn_bits1
    233, // warn_bits2
    234, // alarm_code
    235, // warn_code   
};

const UCHAR DDA_module_ref[] = // DDA, class 5
{
    1, // bus_ctr_dosing_cap_hi
    2, // bus_ctr_dosing_cap_lo1
    3, // bus_ctr_dosing_cap_lo2
    4, // bus_ctr_dosing_cap_lo3   
};






//
//  allocate an group table for four ID code list
//
//  Format of table:
//  Class, Operation, Request-time, number of ID's, pointer to list, devicetype
//

const GROUP_TAB_TYPE group_table[MAS_MAX_NO_GROUPS] = {
  { MEAS_APDU, GET, 1,   sizeof(io_module_status_fast),   (UCHAR *)io_module_status_fast,   DEVICE_IO351_IO_MODULE },
  { MEAS_APDU, GET, 1,   sizeof(cue_module_status_fast),  (UCHAR *)cue_module_status_fast,  DEVICE_E_PUMP },
  { MEAS_APDU, GET, 1,   sizeof(mp204_status_fast),       (UCHAR *)mp204_status_fast,       DEVICE_MP204 },
  { REF_APDU,  SET, 1,   sizeof(io_module_ref),           (UCHAR *)io_module_ref,           DEVICE_IO351_IO_MODULE },
  { MEAS_APDU, GET, 2,   sizeof(cue_module_measurements), (UCHAR *)cue_module_measurements, DEVICE_E_PUMP },
  { MEAS_APDU, GET, 2,   sizeof(mp204_status_slow),       (UCHAR *)mp204_status_slow,       DEVICE_MP204 },
  { MEAS_APDU, GET, 1,   sizeof(io111_status_fast),       (UCHAR *)io111_status_fast,       DEVICE_IO111 },
  { CONF_APDU, GET, 20,  sizeof(cue_module_f_upper),      (UCHAR *)cue_module_f_upper,      DEVICE_E_PUMP },
  { MEAS_APDU, GET, 1,   sizeof(DDA_module_capability),   (UCHAR *)DDA_module_capability,   DEVICE_DDA },
  { MEAS_APDU, GET, 1,   sizeof(DDA_module_status),       (UCHAR *)DDA_module_status,       DEVICE_DDA },
  { REF_APDU,  SET, 1,   sizeof(DDA_module_ref),          (UCHAR *)DDA_module_ref,          DEVICE_DDA }
	  
};

//  if static_apdu_tgm has a size > 0 this apdu will be included in every telegram
//  together with the active group apdu
//  const GROUP_TAB_TYPE static_apdu_tgm = { 0, 0, 0, 0, 0 }; will deactivate this
//  function.
//
const GROUP_TAB_TYPE static_apdu_tgm[MAS_MAX_NO_GROUPS] ={
  { REF_APDU,   GET, 1,  sizeof(io_module_counters),      (UCHAR *)io_module_counters },  // DEVICE_IO351_IO_MODULE, fast
  { REF_APDU,   SET, 1,  sizeof(cue_module_ref),      (UCHAR *)cue_module_ref },          // DEVICE_E_PUMP (speed ref)
  { 0, 0, 0, 0, 0 },                                                                      // DEVICE_MP204
  { 0, 0, 0, 0, 0 },                                                                      // DEVICE_IO351_IO_MODULE
  { REF_APDU,   SET, 1,  sizeof(cue_module_ref),      (UCHAR *)cue_module_ref },          // DEVICE_E_PUMP (speed ref)
  { 0, 0, 0, 0, 0 },                                                                      // DEVICE_MP204
  { 0, 0, 0, 0, 0 },                                                                      // DEVICE_IO111
  { REF_APDU,   SET, 1,  sizeof(cue_module_ref),      (UCHAR *)cue_module_ref }           // DEVICE_E_PUMP (speed ref)
};

//void voidfunc(void)
//{}
extern void SlaveInfoFromUnit(void);
extern void ConfigFromIO351(void);
extern void DummyReply(void);


const DIR_RESP_FNC dir_resp_table[MAS_LAST_RESP_FNC+1] = {
  0, // 0 reserved index to send messages (no response)
  SlaveInfoFromUnit,
  ConfigFromIO351,
  DummyReply
};


/* dir_resp_table could look like this:
const DIR_RESP_FNC dir_resp_table[MAS_LAST_RESP_FNC+1] = {               0,    // 0 reserved index to send messages(no response)
                                                         CheckApduResp,    // 1 Check for ok acknowledge for Cmd or Set
                                                           SaveDirInfo,    // 2 Handle response from info request
                                                          SaveDirAscii,    // 3 Handle response from ASCII request
                                                        HandleUserResp,    // 4 Handle response on an adress change in pump
                                                          PumpResponse,    // 5 Check response and update pump_eep_resp_ok flag
                                                     GetPumpEEpromData,    // 6 Save the read response data
                                                          SaveTestData};   // 7 Save special SQE test data
*/
//
// fill out these tables to specify the location of ID's used in the grouptable
// there is no check for NA, remember to fill out mas_tab
//
#if(MAS_EXTRA_ITEM_POINTER_TAB == Enable)
/*****************************************************************************/
/* CLASS 2, MEASURED DATA  (Extra itempointer table)                         */
/*****************************************************************************/
UCHAR dummy;
UINT d16;
ULONG d32;
const ID_PTR mas_meas_tab[] = {
   s_cl2_id000,     				                         /*  ID   0             */
   s_cl2_id001,                                      /*  ID   1             */
   s_cl2_id002,                                      /*  ID   2             */
   s_cl2_id003,                                      /*  ID   3             */
   NA, NA, NA, NA,                                   /*   4 -  7            */
   s_cl2_id008,                                      /*  ID   8             */
   s_cl2_id009,                                      /*  ID   9             */
   s_cl2_id010,                                      /*  ID   10            */
   s_cl2_id011,                                      /*  ID   11            */
   s_cl2_id012,                                      /*  ID   12            */
   s_cl2_id013,                                      /*  ID   13            */
   s_cl2_id014,                                      /*  ID   14            */
   s_cl2_id015,                                      /*  ID   15            */
   s_cl2_id016,                                      /*  ID   16            */  
   NA, NA, NA,                                       /*  17 - 19            */
   s_cl2_id020,                                      /*  ID  20             */
   s_cl2_id021,                                      /*  ID  21             */
   s_cl2_id022,                                      /*  ID  22             */
   s_cl2_id023,                                      /*  ID  23             */
   s_cl2_id024,                                      /*  ID  24             */
   s_cl2_id025,                                      /*  ID  25             */
   s_cl2_id026,                                      /*  ID  26             */
   s_cl2_id027,                                      /*  ID  27             */
   NA,                                               /*  ID  28             */
   s_cl2_id029,                                      /*  ID  29             */
   s_cl2_id030,                                      /*  ID  30             */
   NA,                                               /*  ID  31             */
   s_cl2_id032,                                      /*  ID  32             */
   s_cl2_id033,                                      /*  ID  33             */
   s_cl2_id034,                                      /*  ID  34             */
   s_cl2_id035,                                      /*  ID  35             */
   s_cl2_id036,                                      /*  ID  36             */
   NA, NA, NA,                                       /*  37 - 39            */
   NA, NA, NA,                                       /*  40 - 42            */
   s_cl2_id043,                                      /*  ID  43             */
   s_cl2_id044,                                      /*  ID  44             */
   s_cl2_id045,                                      /*  ID  45             */
   s_cl2_id046,                                      /*  ID  46             */
   s_cl2_id047,                                      /*  ID  47             */
   s_cl2_id048,                                      /*  ID  48             */
   s_cl2_id049,                                      /*  ID  49             */
   s_cl2_id050,                                      /*  ID  50             */
   NA, NA, NA, NA, NA, NA, NA, NA, NA,               /*  51 - 59            */
   s_cl2_id060,                                      /*  ID  60             */
   NA, NA, NA,                                       /*  61 - 63            */
   s_cl2_id064,                                      /*  ID  64             */
   s_cl2_id065,                                      /*  ID  65             */
   s_cl2_id066,                                      /*  ID  66             */
   s_cl2_id067,                                      /*  ID  67             */
   s_cl2_id068,                                      /*  ID  68             */
   s_cl2_id069,                                      /*  ID  69             */
   s_cl2_id070,                                      /*  ID  70             */
   s_cl2_id071,                                      /*  ID  71             */
   s_cl2_id072,                                      /*  ID  72             */
   s_cl2_id073,                                      /*  ID  73             */
   s_cl2_id074,                                      /*  ID  74             */  
   NA, NA, NA, NA, NA,                               /*  75 - 79            */
   s_cl2_id080,                                      /*  ID  80             */
   s_cl2_id081,                                      /*  ID  81             */
   s_cl2_id082,                                      /*  ID  82             */
   s_cl2_id083,                                      /*  ID  83             */
   NA,                                               /*  84                 */
   s_cl2_id085,                                      /*  ID  85             */   
   s_cl2_id086,                                      /*  ID  86             */
   s_cl2_id087,                                      /*  ID  87             */
   NA, NA,                                           /*  88 - 89            */
   NA,                                               /*  ID  90             */
   NA,                                               /*  ID  91             */
   NA,                                               /*  ID  92             */
   NA,                                               /*  ID  93             */
   s_cl2_id094,                                      /*  ID  94             */
   s_cl2_id095,                                      /*  ID  95             */
   NA, NA, NA,                                       /*  96 - 98            */
   s_cl2_id099,                                      /*  ID  99             */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA,                                               /*  ID 140             */
   s_cl2_id141,                                      /*  ID 141             */
   s_cl2_id142,                                      /*  ID 142             */
   NA,                                               /*  ID 143             */
   s_cl2_id144,                                      /*  ID 144             */
   s_cl2_id145,                                      /*  ID 145             */
   s_cl2_id146,                                      /*  ID 146             */
   s_cl2_id147,                                      /*  ID 147             */
   s_cl2_id148,                                      /*  ID 148             */
   s_cl2_id149,                                      /*  149                */
   NA,                                               /*  ID 150             */
   s_cl2_id151,                                      /*  ID 151             */
   s_cl2_id152,                                      /*  ID 152             */
   s_cl2_id153,                                      /*  ID 153             */
   NA,                                               /*  ID 154             */
   NA,                                               /*  ID 155             */
   s_cl2_id156,                                      /*  ID 156             */
   NA,                                               /*  ID 157             */
   s_cl2_id158,                                      /*  ID 158             */
   NA,                                               /*  ID 159             */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA,                                               /*  ID 190             */
   NA,                                               /*  ID 191             */
   NA,                                               /*  ID 192             */
   NA,                                               /*  ID 193             */
   NA,                                               /*  ID 194             */
   s_cl2_id195,                                      /*  ID 195             */
   s_cl2_id196,                                      /*  ID 196             */
   NA,                                               /*  ID 197             */
   NA,                                               /*  ID 198             */
   NA,                                               /*  ID 199             */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA,                                           /*  230 - 231          */
   s_cl2_id232,                                      /*  ID 232             */   
   s_cl2_id233,                                      /*  ID 233             */
   s_cl2_id234,                                      /*  ID 234             */
   s_cl2_id235,                                      /*  ID 235             */ 
   NA, NA, NA, NA,                                   /*  236 - 239          */   
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA};                          /*  250 - 255          */

/*****************************************************************************/
/* Class 4, Configuration Parameters (Extra itempointer table)               */
/*****************************************************************************/
const ID_PTR mas_conf_tab[] = {
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  0 -  9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
   s_cl4_id030,                                      /* ID 30                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA,               /* 31 - 39              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
   NA, NA, NA, NA, NA, NA };                         /* 250 - 255            */

/*****************************************************************************/
/* Class 5, Reference Values (Extra itempointer table)                       */
/*****************************************************************************/

const ID_PTR   mas_ref_tab[]   = {
   NA,                                               /*  00                  */
   s_cl5_id001,                                      /*  01                  */
   s_cl5_id002,                                      /*  02                  */
   s_cl5_id003,                                      /*  03                  */
   s_cl5_id004,                                      /*  04                  */
   NA,                                               /*  05                  */
   NA,                                               /*  06                  */
   s_cl5_id007,                                      /*  07                  */
   s_cl5_id008,                                      /*  08                  */
   s_cl5_id009,                                      /*  09                  */
   s_cl5_id010,                                      /*  10                  */
   s_cl5_id011,                                      /*  11                  */
   s_cl5_id012,                                      /*  12                  */
   s_cl5_id013,                                      /*  13                  */
   s_cl5_id014,                                      /*  14                  */
   NA, NA, NA, NA, NA,                               /* 15 - 19              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
   NA, NA, NA, NA, NA, NA };                         /* 250 - 255            */

const ID_PTR mas_meas16_tab[] = {
(ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16,         NA,         NA,         NA,         NA,         /*  0 -  9      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 10 - 19      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 20 - 29      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 30 - 39      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 40 - 49      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 50 - 59      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 60 - 69      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 70 - 79      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 80 - 89      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 90 - 99      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 100 - 109    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 110 - 119    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 120 - 129    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 130 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 140 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 150 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 160 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 170 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 180 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 190 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 200 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 210 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 220 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 230 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 240 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA                                                          /* 250 - 255    */
};

const ID_PTR mas_conf16_tab[] = {
(ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16,         NA,         NA,         NA,         NA,         /*  0 -  9      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 10 - 19      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 20 - 29      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 30 - 39      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 40 - 49      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 50 - 59      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 60 - 69      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 70 - 79      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 80 - 89      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 90 - 99      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 100 - 109    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 110 - 119    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 120 - 129    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 130 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 140 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 150 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 160 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 170 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 180 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 190 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 200 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 210 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 220 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 230 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 240 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA                                                          /* 250 - 255    */
};

const ID_PTR mas_ref16_tab[] = {
(ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16, (ID_PTR)&d16,         NA,         NA,         NA,         NA,         /*  0 -  9      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 10 - 19      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 20 - 29      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 30 - 39      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 40 - 49      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 50 - 59      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 60 - 69      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 70 - 79      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 80 - 89      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 90 - 99      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 100 - 109    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 110 - 119    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 120 - 129    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 130 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 140 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 150 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 160 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 170 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 180 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 190 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 200 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 210 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 220 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 230 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 240 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA                                                          /* 250 - 255    */
};

const ID_PTR mas_meas32_tab[] = {
 (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32,         NA,         NA,         NA,         NA,         /*  0 -  9      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 10 - 19      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 20 - 29      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 30 - 39      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 40 - 49      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 50 - 59      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 60 - 69      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 70 - 79      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 80 - 89      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 90 - 99      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 100 - 109    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 110 - 119    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 120 - 129    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 130 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 140 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 150 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 160 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 170 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 180 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 190 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 200 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 210 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 220 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 230 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 240 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA                                                          /* 250 - 255    */
};

const ID_PTR mas_conf32_tab[] = {
 (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32,         NA,         NA,         NA,         NA,         /*  0 -  9      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 10 - 19      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 20 - 29      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 30 - 39      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 40 - 49      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 50 - 59      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 60 - 69      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 70 - 79      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 80 - 89      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 90 - 99      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 100 - 109    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 110 - 119    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 120 - 129    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 130 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 140 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 150 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 160 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 170 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 180 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 190 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 200 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 210 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 220 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 230 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 240 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA                                                          /* 250 - 255    */
};

const ID_PTR mas_ref32_tab[] = {
 (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32, (ID_PTR)&d32,         NA,         NA,         NA,         NA,         /*  0 -  9      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 10 - 19      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 20 - 29      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 30 - 39      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 40 - 49      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 50 - 59      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 60 - 69      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 70 - 79      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 80 - 89      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 90 - 99      */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 100 - 109    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 110 - 119    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 120 - 129    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 130 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 140 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 150 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 160 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 170 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 180 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 190 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 200 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 210 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 220 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 230 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         NA,         /* 240 - 139    */
   NA,          NA,         NA,         NA,         NA,         NA                                                          /* 250 - 255    */
};

#endif   // end MAS_EXTRA_ITEM_POINTER_TAB


//
// This table descipes which itemtables to use if the busmaster should use
// a common butab, then insert the names from butab.c
//
const ID_PTR *mas_tab[] = {
  0,                    // 0      Protocol itemtable
  0,                    // 1      channel itemtable
  mas_meas_tab,         // 2      Measurements itemtable
  0,                    // 3      Commands
  mas_conf_tab,         // 4      Configurations itemtable
  mas_ref_tab,          // 5      Reference itemtable
  0,                    // cl6
  0,                    // cl7
  0,                    // cl8
  0,                    // cl9
  0,                    // cl10
  mas_meas16_tab,       // cl11     Measurements itemtable
  mas_conf16_tab,       // cl12     Configurations itemtable
  mas_ref16_tab,        // cl13     Reference itemtable
  mas_meas32_tab,       // cl14     Measurements itemtable
  mas_conf32_tab,       // cl15     Configurations itemtable
  mas_ref32_tab         // cl16     Reference itemtable
};


const UCHAR nof_bytes_class[] = {
  sizeof(GF_UINT8),     // cl0
  0,
  sizeof(GF_UINT8),     // cl2
  0,
  sizeof(GF_UINT8),     // cl4
  sizeof(GF_UINT8),     // cl5
  sizeof(GF_UINT8),     // cl6
  0,                    // cl7
  0,                    // cl8
  0,                    // cl9
  0,                    // cl10
  sizeof(GF_UINT16),    // cl11
  sizeof(GF_UINT16),    // cl12
  sizeof(GF_UINT16),    // cl13
  sizeof(GF_UINT32),    // cl14
  sizeof(GF_UINT32),    // cl15
  sizeof(GF_UINT32)     // cl16
};

