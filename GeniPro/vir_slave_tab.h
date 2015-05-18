/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:  GENIpro                                         */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2003               */
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
/* MODULE NAME      :  vir_slave_tab                                        */
/*                                                                          */
/* FILE NAME        :  vir_slave_tab.h                                      */
/*                                                                          */
/* FILE DESCRIPTION :  This header file gives the interface to              */
/*                     vir_slave_tab.c  which implements the virtual        */
/*                     slave functionality.                                 */
/*                                                                          */
/****************************************************************************/

#ifndef _VIR_SLAVE_TAB_H
  #define _VIR_SLAVE_TAB_H
#include <typedef.h>            /*definements for the Grundfos common types */
#include "geni_if.h"            /* API interface */

/****************************************************************************/
/*                                                                          */
/* G L O B A L   D E F I N E M E N T S                                      */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************/
/* Specification of Virtual Slave Unit Tables and Buffers                    */
/*****************************************************************************/
#define   VIR_SLAVE_HIGH_MEAS_ID      255             // [1; 255] Highest ID in Class 2, Measured Data
#define   VIR_SLAVE_HIGH_CMD_ID       78              // [1; 255] Highest ID in Class 3, Commands
#define   VIR_SLAVE_HIGH_CONF_ID      255             // [1; 255] Highest ID in Class 4, Config. Param.
#define   VIR_SLAVE_HIGH_REF_ID       2               // [1; 255] Highest ID in Class 5, Ref. Values
#define   VIR_SLAVE_HIGH_TEST_ID      6              // [1; 255] Highest ID in Class 6, Test Data
#define   VIR_SLAVE_HIGH_ASCII_ID     10              // [1; 255] Highest ID in Class 7, ASCII Strings

#define   VIR_SLAVE_HIGH_MEAS16_ID    1             // [1; 255] Highest ID in Class 11, 16 bit measured Data
#define   VIR_SLAVE_HIGH_CONF16_ID    1             // [1; 255] Highest ID in Class 12, 16 bit Config. Data
#define   VIR_SLAVE_HIGH_REF16_ID     1               // [1; 255] Highest ID in Class 13, 16 bit Reference Data

#define   VIR_SLAVE_HIGH_MEAS32_ID    1             // [1; 255] Highest ID in Class 14, 32 bit measured Data
#define   VIR_SLAVE_HIGH_CONF32_ID    1             // [1; 255] Highest ID in Class 15, 32 bit Config. Data
#define   VIR_SLAVE_HIGH_REF32_ID     1               // [1; 255] Highest ID in Class 16, 32 bit Reference Data

#define   VIR_SLAVE_CMD_BUF_LEN       6               // [6; 254],   Length of Command buffer
#define   VIR_SLAVE_CONF_BUF_LEN      1              // [0,6; 254], Length of Conf. Parameter buffer
#define   VIR_SLAVE_REF_BUF_LEN       1               // [0,6; 254], Length of Reference Value buffer
#define   VIR_SLAVE_ASCII_BUF_LEN     24               // [0,6; 254], Length of ascii buffer

#define   VIR_SLAVE_CONF16_BUF_LEN    1               // [0; 254], Length of Conf16 Parameter buffer
#define   VIR_SLAVE_REF16_BUF_LEN     1               // [0; 254], Length of Reference16 Value buffer

#define   VIR_SLAVE_CONF32_BUF_LEN    1               // [0; 254], Length of Conf32 Parameter buffer
#define   VIR_SLAVE_REF32_BUF_LEN     1               // [0; 254], Length of Reference32 Value buffer

/****************************************************************************/
/*                                                                          */
/*    Dummy definements of tables and buffers not used                      */
/*    and external tabel referenses.                                        */
/*                                                                          */
/****************************************************************************/
#if ( CTO_CLASS_8 == Enable )
  #define VIR_SLAVE_HIGH_MEMORY_ID  1
#else
  #define VIR_SLAVE_HIGH_MEMORY_ID  0
#endif

#if ( VIR_SLAVE_HIGH_MEAS_ID == 0 )
  #define slave_meas_tab      0
  #define slave_meas_info_tab 0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_meas_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1];
  extern ID_INFO slave_meas_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1];
  #else
  extern const ID_PTR  slave_meas_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1];
  extern const ID_INFO slave_meas_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_CMD_ID == 0 )
  #define slave_cmd_info_tab  0
#else
  #if defined( __PC__ )
  extern ID_INFO slave_cmd_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CMD_ID+1];
  #else
  extern const ID_INFO slave_cmd_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CMD_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_CONF_ID == 0)
  #define conf_tab      0
  #define conf_info_tab 0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_conf_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1];
  extern ID_INFO slave_conf_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1];
  #else
  extern const ID_PTR  slave_conf_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1];
  extern const ID_INFO slave_conf_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_REF_ID  == 0)
  #define slave_ref_tab       0
  #define slave_ref_info_tab  0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_ref_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1];
  extern ID_INFO slave_ref_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1];
  #else
  extern const ID_PTR  slave_ref_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1];
  extern const ID_INFO slave_ref_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_TEST_ID == 0 )
  #define slave_test_tab      0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_test_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_TEST_ID+1];
  #else
  extern const ID_PTR  slave_test_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_TEST_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_ASCII_ID == 0 )
  #define slave_ascii_tab     0
#else
  #if defined( __PC__ )
  extern ID_PTR slave_ascii_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_ASCII_ID+1];
  #else
  extern const ID_PTR slave_ascii_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_ASCII_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_MEAS16_ID == 0 )
  #define slave_meas16_tab      0
  #define slave_meas16_info_tab 0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_meas16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1];
  extern ID_INFO slave_meas16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1];
  #else
  extern const ID_PTR  slave_meas16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1];
  extern const ID_INFO slave_meas16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_CONF16_ID == 0)
  #define slave_conf16_tab      0
  #define slave_conf16_info_tab 0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_conf16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1];
  extern ID_INFO slave_conf16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1];
  #else
  extern const ID_PTR  slave_conf16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1];
  extern const ID_INFO slave_conf16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_REF16_ID  == 0)
  #define slave_ref16_tab       0
  #define slave_ref16_info_tab  0
#else
  #if defined( __PC__ )
  extern ID_PTR   slave_ref16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1];
  extern ID_INFO  slave_ref16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1];
  #else
  extern const ID_PTR   slave_ref16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1];
  extern const ID_INFO  slave_ref16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_MEAS32_ID == 0 )
  #define slave_meas32_tab      0
  #define slave_meas32_info_tab 0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_meas32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1];
  extern ID_INFO slave_meas32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1];
  #else
  extern const ID_PTR  slave_meas32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1];
  extern const ID_INFO slave_meas32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_CONF32_ID == 0)
  #define slave_conf32_tab      0
  #define slave_conf32_info_tab 0
#else
  #if defined( __PC__ )
  extern ID_PTR  slave_conf32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1];
  extern ID_INFO slave_conf32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1];
  #else
  extern const ID_PTR  slave_conf32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1];
  extern const ID_INFO slave_conf32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1];
  #endif
#endif

#if ( VIR_SLAVE_HIGH_REF32_ID  == 0)
  #define slave_ref32_tab       0
  #define slave_ref32_info_tab  0
#else
  #if defined( __PC__ )
  extern ID_PTR   slave_ref32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1];
  extern ID_INFO  slave_ref32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1];
  #else
  extern const ID_PTR   slave_ref32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1];
  extern const ID_INFO  slave_ref32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1];
  #endif
#endif


#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

#if ( VIR_SLAVE_CONF_BUF_LEN == 0 )
  #define slave_conf_buf      0
#endif

#if ( VIR_SLAVE_CMD_BUF_LEN == 0 )
  #define slave_cmd_buf       0
#endif

#if ( VIR_SLAVE_REF_BUF_LEN == 0 )
  #define slave_ref_buf       0
#endif

#if ( VIR_SLAVE_ASCII_BUF_LEN == 0 )
  #define slave_ascii_buf       0
#endif

#if ( VIR_SLAVE_CONF16_BUF_LEN == 0 )
  #define slave_conf16_buf      0
#endif

#if ( VIR_SLAVE_REF16_BUF_LEN == 0 )
  #define slave_ref16_buf       0
#endif

#if ( VIR_SLAVE_CONF32_BUF_LEN == 0 )
  #define slave_conf32_buf      0
#endif

#if ( VIR_SLAVE_REF32_BUF_LEN == 0 )
  #define slave_ref32_buf       0
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
#if defined( __PC__ )
extern PROTAB vir_slave_pre_tab[MAX_VIR_SLAVE_COUNT][17];
#else
extern const PROTAB vir_slave_pre_tab[MAX_VIR_SLAVE_COUNT][17];
#endif

#endif
/****************************************************************************/
/*                                                                          */
/* E N D   O F   S P E C I F I C A T I O N   F I L E                        */
/*                                                                          */
/****************************************************************************/
