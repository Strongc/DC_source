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
/* CLASS NAME       : bu_tab_string                                         */
/*                                                                          */
/* FILE NAME        : bu_tab_string.c                                       */
/*                                                                          */
/* CREATED DATE     : 03-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Bus unit table for GeniPro class 7 interface (ascii)  */
/*                                                                          */
/****************************************************************************/

/***************************** Include files *******************************/
#include "geni_cnf.h"
#include "profiles.h"
#include "GeniTestModeData.h"
#include "AppTypeDefs.h"

/*****************************    Defines    *******************************/

/*****************************   Constants   *******************************/


const ID_PTR  ascii_tab[HIGH_ASCII_ID+1] =
{
  0,                                                /* ID 0                 */
  (UCHAR *)HW_NAME,                                 /* ID 1 product name    */
  (UCHAR *)PRODUCT_NAME,                            /* ID 2 project name    */
  (UCHAR *)&G_ge_cpu_software_version,              /* ID 3 software_name   */
  __DATE__,                                         /* ID 4 compile_date    */
  (UCHAR *) GENI_VERSION_STR,                       /* ID 5 protocol_ver    */
  (UCHAR *)"N/A",                                   /* ID 6 grundfos_no     */
  (UCHAR *)"JSM, ESL, JSZ, XFK, XHE, HRG, DAU",     /* ID 7 developers      */
  __TIME__,                                         /* ID 8 compile_time    */
  (UCHAR *)"N/A",                                   /* ID 9                 */
  (UCHAR *)"N/A",                                   /* ID 10                */
  (UCHAR *)"N/A",                                   /* ID 11                */
  (UCHAR *)"N/A",                                   /* ID 12                */
  (UCHAR *)"N/A",                                   /* ID 13                */
  (UCHAR *)&G_ge_io_software_version,               /* ID 14                */
  (UCHAR *)&G_ge_boot_load_software_version,        /* ID 15                */
  (UCHAR *)"N/A",                                   /* ID 16                */
};
