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
/* MODULE NAME      :  Profiles.c                                           */
/*                                                                          */
/* FILE NAME        :  Profiles.c                                           */
/*                                                                          */
/* FILE DESCRIPTION :  Holds the presentation table                         */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "common.H"
#include "vir_dia_slave.h"
#include "vir_common.h"
#include "profiles.h"
/****************************************************************************/
/*                                                                          */
/* G L O B A L   C O N S T A N T S                                          */
/*                                                                          */
/****************************************************************************/
const ULONG na=0xFFFFFFFF;   // general not avaiable parameter

const PROTAB pre_tab[] = {
{ HIGH_PRO_ID      ,               0,   pro_tab,            0,                 0},    // 0      Protocol table
{ HIGH_CH_ID      ,               0,     ch_tab,            0,                 0},    // 1      channel table
{ HIGH_MEAS_ID     ,   meas_info_tab,   meas_tab,           0,                 0},    // 2      Measurements
{ HIGH_CMD_ID      ,   cmd_info_tab ,        0,       cmd_buf,      CMD_BUF_LEN },    // 3      Commands
{ HIGH_CONF_ID     ,   conf_info_tab,   conf_tab,    conf_buf,     CONF_BUF_LEN },    // 4      Configurations
{ HIGH_REF_ID      ,   ref_info_tab ,   ref_tab,      ref_buf,      REF_BUF_LEN },    // 5      Reference
{ HIGH_TEST_ID     ,               0,   test_tab,           0,                 0},    // 6      Parameters for test
{ HIGH_ASCII_ID    ,               0,   ascii_tab,  ascii_buf,     ASCII_BUF_LEN},    // 7      Ascii informations
{ HIGH_MEMORY_ID   ,               0,           0,          0,                 0},    // 8      Memory blocks
{ HIGH_ROUTE_ID    ,               0,           0,routing_buf, ROUTER_DF_buf_len},    // 9      Routing class
{ HIGH_OBJECT_ID   ,               0,           0, object_buf, OBJECT_DF_buf_len},    // 10     Object class
{ HIGH_MEAS16_ID   , meas16_info_tab,  meas16_tab,          0,                 0},    // 11    16 bit measurements
{ HIGH_CONF16_ID   , conf16_info_tab,  conf16_tab, conf16_buf,    CONF16_BUF_LEN},    // 12    16 bit configurations
{ HIGH_REF16_ID    ,  ref16_info_tab,   ref16_tab,  ref16_buf,     REF16_BUF_LEN},    // 13    16 bit reference
{ HIGH_MEAS32_ID   , meas32_info_tab,  meas32_tab,          0,                 0},    // 14    32 bit measurements
{ HIGH_CONF32_ID   , conf32_info_tab,  conf32_tab, conf32_buf,    CONF32_BUF_LEN},    // 15    32 bit configurations
{ HIGH_REF32_ID    ,  ref32_info_tab,   ref32_tab,  ref32_buf,     REF32_BUF_LEN}};   // 16    32 bit reference

#if (COM_INFO_LEN == 0)
  const INFO_DATA  common_info_tab[1];
#endif

#if (COM_PTR_LEN == 0)
  const INFO_DATA_PTR common_ptr_tab[1];
#endif
/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/

