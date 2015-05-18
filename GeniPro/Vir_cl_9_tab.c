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
/* MODULE NAME      :    vir_cl_9_tab.c                                     */
/*                                                                          */
/* FILE NAME        :    vir_cl_9_tab.c                                     */
/*                                                                          */
/* FILE DESCRIPTION :    Virtual class 9 tables for class's                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include "common.h"
#include "geni_if.h"
#include "vir_cl_9_tab.h"


UCHAR debug=0;
ULONG bit32_var = 0x11223344;
/*****************************************************************************/
/* HIGH ID                                                                   */
/*****************************************************************************/
#define HIGH_VIR_MEAS_ID    sizeof(vir_meas_tab)     / sizeof(*vir_meas_tab) - 1
#define HIGH_VIR_CMD_ID     sizeof(vir_cmd_info_tab) / sizeof(*vir_cmd_info_tab) - 1
#define HIGH_VIR_CONF_ID    sizeof(vir_conf_tab)     / sizeof(*vir_conf_tab) - 1
#define HIGH_VIR_REF_ID     sizeof(vir_ref_tab)      / sizeof(*vir_ref_tab) - 1
#define HIGH_VIR_TEST_ID    sizeof(vir_test_tab)     / sizeof(*vir_test_tab) - 1
#define HIGH_VIR_ASCII_ID   sizeof(vir_ascii_tab)    / sizeof(*vir_ascii_tab) - 1
#if ( CTO_VIR_CLASS_8 == Enable )
  #define HIGH_VIR_MEMORY_ID  1
#else
  #define HIGH_VIR_MEMORY_ID  0
#endif
#if ( CTO_VIR_CLASS_10 == Enable )
  #define HIGH_VIR_OBJECT_ID  1
#else
  #define HIGH_VIR_OBJECT_ID  0
#endif
#define HIGH_VIR_MEAS16_ID  sizeof(vir_meas16_tab) / sizeof(*vir_meas16_tab) -1
#define HIGH_VIR_CONF16_ID  sizeof(vir_conf16_tab) / sizeof(*vir_conf16_tab) -1
#define HIGH_VIR_REF16_ID   sizeof(vir_ref16_tab) / sizeof(*vir_ref16_tab)  -1

#define HIGH_VIR_MEAS32_ID  sizeof(vir_meas32_tab) / sizeof(*vir_meas32_tab) -1
#define HIGH_VIR_CONF32_ID  sizeof(vir_conf32_tab) / sizeof(*vir_conf32_tab) -1
#define HIGH_VIR_REF32_ID   sizeof(vir_ref32_tab) / sizeof(*vir_ref32_tab)  -1

/*****************************************************************************/
/* DATA and INFO TABLES                                                      */
/*****************************************************************************/
//
//CLASS 2, MEASURED DATA
//

const ID_PTR  vir_meas_tab[] = {
   0,  &debug, NA, NA, NA, NA, NA, NA, NA, NA,           /*  0 - 9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA};                          /*  250 - 255          */


const ID_INFO  vir_meas_info_tab[] = {
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
   NI, NI, NI, NI, NI, NI};                          /* 250 - 255            */

//
// Class 3, Commands
//
const ID_INFO  vir_cmd_info_tab[] = {
   Unexecutable,                                     /*   0                  */
   Executable,                                       /*   1  RESET           */
   Executable,                                       /*   2  RESET_ALARM     */
   Executable,                                       /*   3  FACTORY_BOOT    */
   Executable,                                       /*   4  USER_BOOT       */
   Executable,                                       /*   5  STOP            */
   Executable,                                       /*   6  START           */
   Unexecutable,                                     /*   7                  */
   Unexecutable,                                     /*   8                  */
   Unexecutable,                                     /*   9                  */
   Unexecutable,                                     /*  10                  */
   Unexecutable,                                     /*  11                  */
   Unexecutable,                                     /*  12                  */
   Unexecutable,                                     /*  13                  */
   Unexecutable,                                     /*  14                  */
   Unexecutable,                                     /*  15                  */
   Unexecutable,                                     /*  16                  */
   Unexecutable,                                     /*  17                  */
   Unexecutable,                                     /*  18                  */
   Executable,                                       /*  19  USE             */
   Executable,                                       /*  20  TEST            */
   Unexecutable,                                     /*  21                  */
   Executable,                                       /*  22                  */
   Unexecutable,                                     /*  23                  */
   Unexecutable,                                     /*  24                  */
   Executable,                                       /*  25  MIN             */
   Executable,                                       /*  26  MAX             */
   Unexecutable,                                     /*  27                  */
   Unexecutable,                                     /*  28                  */
   Unexecutable,                                     /*  29                  */
   Executable,                                       /*  30  LOCK_KEYS       */
   Executable,                                       /*  31  UNLOCK_KEYS     */
   Unexecutable,                                     /*  32                  */
   Unexecutable,                                     /*  33                  */
   Unexecutable,                                     /*  34                  */
   Unexecutable,                                     /*  35                  */
   Unexecutable,                                     /*  36                  */
   Unexecutable,                                     /*  37                  */
   Unexecutable,                                     /*  38                  */
   Unexecutable,                                     /*  39                  */
   Unexecutable,                                     /*  40                  */
   Executable,                                       /*  41 AUTO_RESTART-ENABLE  */
   Executable,                                       /*  42 AUTO-RESTART-DISABLE */
   Unexecutable,                                     /*  43                  */
   Unexecutable,                                     /*  44                  */
   Unexecutable,                                     /*  45                  */
   Unexecutable,                                     /*  46                  */
   Unexecutable,                                     /*  47                  */
   Unexecutable,                                     /*  48                  */
   Unexecutable,                                     /*  49                  */
   Unexecutable,                                     /*  50                  */
   Unexecutable,                                     /*  51                  */
   Unexecutable,                                     /*  52                  */
   Unexecutable,                                     /*  53                  */
   Unexecutable,                                     /*  54                  */
   Unexecutable,                                     /*  55                  */
   Unexecutable,                                     /*  56                  */
   Unexecutable,                                     /*  57                  */
   Unexecutable,                                     /*  58                  */
   Unexecutable,                                     /*  59                  */
   Unexecutable,                                     /*  60                  */
   Unexecutable,                                     /*  61                  */
   Unexecutable,                                     /*  62                  */
   Unexecutable,                                     /*  63                  */
   Unexecutable,                                     /*  64                  */
   Unexecutable,                                     /*  65                  */
   Unexecutable,                                     /*  66                  */
   Executable,                                       /*  67 DOUBLE_STB_ENABLE*/
   Executable,                                       /*  68 DOUBLE_STB_DISABLE*/
   Unexecutable,                                     /*  69                  */
   Unexecutable,                                     /*  70                  */
   Unexecutable,                                     /*  71                  */
   Unexecutable,                                     /*  72                  */
   Unexecutable,                                     /*  73                  */
   Unexecutable,                                     /*  74                  */
   Unexecutable,                                     /*  75                  */
   Unexecutable,                                     /*  76                  */
   Unexecutable,                                     /*  77                  */
   Unexecutable};                                    /*  78                  */

//
// Class 4, Configuration Parameters
//
const ID_PTR   vir_conf_tab[] = {
   0,  &debug, NA, NA, NA, NA, NA, NA, NA, NA,           /* 0 - 9                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
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

const ID_INFO  vir_conf_info_tab[] = {
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
   NI, NI, NI, NI, NI, NI };                         /* 250 - 255            */


//
// Class 5, Reference Values
//

const ID_PTR   vir_ref_tab[]   = {
   0,                                                /*  0                   */
   NA,                                               /*  1                   */
   NA };                                             /*  2                   */

const ID_INFO  vir_ref_info_tab[]   = {
   0,                                                /*  0                   */
   NI,                                               /*  1                   */
   NI };                                             /*  2                   */


//
// Class 6: Test data
//

const ID_PTR vir_test_tab[] = {
   0,                                                /*  ID   0              */
   NA,                                               /*  ID   1              */
   NA,                                               /*  ID   2              */
   NA,                                               /*  ID   3              */
   NA,                                               /*  ID   4              */
   NA };                                             /*  ID   5              */


//
// Class 7, Ascii Strings
//

CHAR  vcl7_id10[ASCII_SIZE] = "0123456789";

const ID_PTR  vir_ascii_tab[] = {
   0,                                                /* ID 0                 */
   (UCHAR *)"product name",                          /* ID 1 product name    */
   (UCHAR *)"project name",                          /* ID 2 project name    */
   (UCHAR *)"software name",                         /* ID 3 software_name   */
   __DATE__,                                         /* ID 4 compile_date    */
   (UCHAR *) GENI_VERSION_STR,                       /* ID 5 protocol_ver    */
   (UCHAR *)"grundfos no",                           /* ID 6 grundfos_no     */
   (UCHAR *)"developers",                            /* ID 7 developers      */
   (UCHAR *)"extra",                                 /* ID 8                 */
   (UCHAR *)"extra",                                 /* ID 9                 */
   (UCHAR *) &vcl7_id10
   };



//
// CLASS 11, 16 BIT MEASURED DATA
//

const ID_PTR  vir_meas16_tab[] = {
   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA};                          /*  250 - 255          */

const ID_INFO  vir_meas16_info_tab[] = {
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
   NI, NI, NI, NI, NI, NI};                          /* 250 - 255            */

  //
// CLASS 12, 16 BIT Configuration Parameters
//

const ID_PTR   vir_conf16_tab[] = {
   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
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


const ID_INFO  vir_conf16_info_tab[] = {
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
   NI, NI, NI, NI, NI, NI };                         /* 250 - 255            */



//
// Class 13, 16 BIT Reference Values
//

const ID_PTR   vir_ref16_tab[]   = {
   0,                                                /*  0                   */
   NA,                                               /*  1                   */
   NA };                                             /*  2                   */

const ID_INFO  vir_ref16_info_tab[]   = {
   0,                                                /*  0                   */
   NI,                                               /*  1                   */
   NI };                                             /*  2                   */

//
// CLASS 14, 32 BIT MEASURED DATA
//

const ID_PTR  vir_meas32_tab[] = {
   0, (UCHAR*)&bit32_var, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA};                          /*  250 - 255          */

const ID_INFO  vir_meas32_info_tab[] = {
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
   NI, NI, NI, NI, NI, NI};                          /* 250 - 255            */

  //
// CLASS 15, 32 BIT Configuration Parameters
//

const ID_PTR   vir_conf32_tab[] = {
   0, (UCHAR*)&bit32_var, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
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


const ID_INFO  vir_conf32_info_tab[] = {
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
   NI, NI, NI, NI, NI, NI };                         /* 250 - 255            */



//
// Class 16, 32 BIT Reference Values
//

const ID_PTR   vir_ref32_tab[]   = {
   0,                                                /*  0                   */
   (UCHAR*)&bit32_var,                                               /*  1                   */
   NA };                                             /*  2                   */

const ID_INFO  vir_ref32_info_tab[]   = {
   0,                                                /*  0                   */
   NI,                                               /*  1                   */
   NI };                                             /*  2                   */


/*****************************************************************************/
/* Common Tables                                                             */
/*****************************************************************************/

const INFO_DATA vir_common_info_tab[] =
   {
   // HEADER,  UNIT index, ZERO, RANGE
   {User_acc,         149,   16,   154  },           /*  0:    1.0 [øC]    */
   {User_acc,          38,    0,   183  },           /*  1:    2.0 [Hz]    */
   {User_acc,          39,    0,   127  },           /*  2:   1024 [h]     */
   {User_acc,          37,    0,   254  },           /*  3:    1.0 [s]     */
   {User_acc,          30,    0,   100  },           /*  4:    1.0 [%]     */
   {User_acc,           5,    0,   100  },           /*  5:    5.0 [V]     */
   {User_acc,          36,    0,   127  },           /*  6:    2.0 [min]   */
   {User_acc,          40,    0,   254  },           /*  7:    512 [kWh]   */
   {User_acc,           8,    0,   250  },           /*  8:     10 [W]     */
   {User_acc,          19,    0,   110  },           /*  9:    100 [rpm]   */
   {User_acc,          22,    0,   254  },           /* 10:    0.1 [m3/h]  */
   {User_acc,          65,    0,     1  },           /* 11:   1000 [m3]    */
   {User_acc,          66,    0,   100  },           /* 12:     10 [kWh/m3]*/
   {User_acc,          50,    0,   254  },           /* 13:      1 [ ]     */
   {User_acc,          64,    0,    65  },           /* 14:    0.1 [m3]    */
   {User_acc,          67,    0,   254  },           /* 15:    256 [m3]    */
   {User_acc,          19,    0,   109  },           /* 16:    100 [rpm]   */

   };


const INFO_DATA_PTR vir_common_ptr_tab[] =
   {
   NA,                                            /* 0                     */
   NA,                                            /* 1                     */
   NA,                                            /* 2                     */
   NA,                                            /* 3                     */
   NA,                                            /* 4                     */
   NA,                                            /* 5                     */
   NA,                                            /* 6                     */
   NA,                                            /* 7                     */
   NA,                                            /* 8                     */
   NA,                                            /* 9                     */
   };

/*****************************************************************************/
/* BUFFER                                                                    */
/*****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

#if ( VIR_CMD_BUF_LEN != 0 )
  BUFFER vir_cmd_buf[VIR_CMD_BUF_LEN+2];            // command buffer
#else
  #define vir_cmd_buf       0
#endif

#if ( VIR_CONF_BUF_LEN != 0 )
  BUFFER vir_conf_buf[VIR_CONF_BUF_LEN+2];          // configuration buffer
#else
  #define vir_conf_buf      0
#endif

#if ( VIR_REF_BUF_LEN != 0 )
  BUFFER vir_ref_buf[VIR_REF_BUF_LEN+2];            // reference buffer
#else
  #define vir_ref_buf       0
#endif

#if ( VIR_ASCII_BUF_LEN != 0 )
  BUFFER vir_ascii_buf[VIR_ASCII_BUF_LEN+2];          // ascii buffer
#else
  #define vir_ascii_buf       0
#endif


#if ( CTO_VIR_CLASS_10 == Enable )
  UCHAR vir_object_buf[VIR_OBJECT_DF_buf_len+1];    // object PDU's
#else
  #define vir_object_buf    0
#endif

#if ( VIR_CONF16_BUF_LEN != 0 )
  BUFFER16 vir_conf16_buf[VIR_CONF16_BUF_LEN+2];    // configuration buffer
#else
  #define vir_conf16_buf      0
#endif

#if ( VIR_REF16_BUF_LEN != 0 )
  BUFFER16 vir_ref16_buf[VIR_REF16_BUF_LEN+2];      // reference buffer
#else
  #define vir_ref16_buf       0
#endif

#if ( VIR_CONF32_BUF_LEN != 0 )
  BUFFER32 vir_conf32_buf[VIR_CONF32_BUF_LEN+2];    // configuration buffer
#else
  #define vir_conf32_buf      0
#endif

#if ( VIR_REF32_BUF_LEN != 0 )
  BUFFER32 vir_ref32_buf[VIR_REF32_BUF_LEN+2];      // reference buffer
#else
  #define vir_ref32_buf       0
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif


/*****************************************************************************/
/* vir_pre_tab                                                               */
/*****************************************************************************/
  const PROTAB vir_pre_tab[] = {
{ HIGH_PRO_ID        ,                     0,           pro_tab,                0,                      0},    // 0      Protocol table
{ HIGH_CH_ID         ,                     0,            ch_tab,                0,                      0},    // 1      channel table
{ HIGH_VIR_MEAS_ID   ,  vir_meas_info_tab   ,  vir_meas_tab    ,                0,                      0},    // 2      Measurements
{ HIGH_VIR_CMD_ID    ,  vir_cmd_info_tab    ,                 0,  vir_cmd_buf    ,  VIR_CMD_BUF_LEN      },    // 3      Commands
{ HIGH_VIR_CONF_ID   ,  vir_conf_info_tab   ,  vir_conf_tab    ,  vir_conf_buf   ,  VIR_CONF_BUF_LEN     },    // 4      Configurations
{ HIGH_VIR_REF_ID    ,  vir_ref_info_tab    ,  vir_ref_tab     ,  vir_ref_buf    ,  VIR_REF_BUF_LEN      },    // 5      Reference
{ HIGH_VIR_TEST_ID   ,                     0,  vir_test_tab    ,                0,                      0},    // 6      Parameters for test
{ HIGH_VIR_ASCII_ID  ,                     0,  vir_ascii_tab   ,    vir_ascii_buf,      VIR_ASCII_BUF_LEN},    // 7      Ascii informations
{ HIGH_VIR_MEMORY_ID ,                     0,                 0,                0,                      0},    // 8      Memory blocks       PGH
{                   0,                     0,                 0,                0,                      0},    // 9      Routing class
{ HIGH_VIR_OBJECT_ID ,                     0,                 0,  vir_object_buf ,  VIR_OBJECT_DF_buf_len},    // 10     be implemented in Genipro 3.51
{ HIGH_VIR_MEAS16_ID , vir_meas16_info_tab  , vir_meas16_tab   ,                0,                      0},    // 11     16 bit measurements
{ HIGH_VIR_CONF16_ID , vir_conf16_info_tab  , vir_conf16_tab   ,  vir_conf16_buf ,  VIR_CONF16_BUF_LEN   },    // 12     16 bit configurations
{ HIGH_VIR_REF16_ID  , vir_ref16_info_tab   , vir_ref16_tab    ,  vir_ref16_buf  ,  VIR_REF16_BUF_LEN    },    // 13     16 bit reference
{ HIGH_VIR_MEAS32_ID , vir_meas32_info_tab  , vir_meas32_tab   ,                0,                      0},    // 14     32 bit measurements
{ HIGH_VIR_CONF32_ID , vir_conf32_info_tab  , vir_conf32_tab   ,  vir_conf32_buf ,  VIR_CONF32_BUF_LEN   },    // 15     32 bit configurations
{ HIGH_VIR_REF32_ID  , vir_ref32_info_tab   , vir_ref32_tab    ,  vir_ref32_buf  ,  VIR_REF32_BUF_LEN    }};   // 16     32 bit reference
