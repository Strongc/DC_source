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
/* MODULE NAME      :   profiles.h                                          */
/*                                                                          */
/* FILE NAME        :   profiles.h                                          */
/*                                                                          */
/* FILE DESCRIPTION :  Profiles module for Geni                             */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _PROFILES_H_
#define _PROFILES_H_

#include "geni_if.h"                    // Access to Geni interface

extern const PROTAB pre_tab[17];
//
//
//    Dummy definements of tables and buffers not used
//    and external tabel referenses.
//

#if ( HIGH_MEAS_ID == 0 )
  #define meas_tab      0
  #define meas_info_tab 0
#endif

#if ( HIGH_CMD_ID == 0 )
  #define cmd_info_tab  0
#endif

#if ( HIGH_CONF_ID == 0)
  #define conf_tab      0
  #define conf_info_tab 0
#endif

#if ( HIGH_REF_ID  == 0)
  #define ref_tab       0
  #define ref_info_tab  0
#endif

#if ( HIGH_TEST_ID == 0 )
  #define test_tab      0
#endif

#if ( HIGH_ASCII_ID == 0 )
  #define ascii_tab     0
#endif

#if (COM_INFO_LEN == 0)
  extern const INFO_DATA  common_info_tab[1];
#endif

#if (COM_PTR_LEN == 0)
  extern const INFO_DATA_PTR common_ptr_tab[1];
#endif

#if ( HIGH_MEAS16_ID == 0 )
  #define meas16_tab      0
  #define meas16_info_tab 0
#endif

#if ( HIGH_CONF16_ID == 0)
  #define conf16_tab      0
  #define conf16_info_tab 0
#endif

#if ( HIGH_REF16_ID  == 0)
  #define ref16_tab       0
  #define ref16_info_tab  0
#endif

#if ( HIGH_MEAS32_ID == 0 )
  #define meas32_tab      0
  #define meas32_info_tab 0
#endif

#if ( HIGH_CONF32_ID == 0)
  #define conf32_tab      0
  #define conf32_info_tab 0
#endif

#if ( HIGH_REF32_ID  == 0)
  #define ref32_tab       0
  #define ref32_info_tab  0
#endif
#endif // _PROFILES_H_
