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
/* CLASS NAME       : common_info_tab                                       */
/*                                                                          */
/* FILE NAME        : common_info_tab.c                                     */
/*                                                                          */
/* CREATED DATE     : 03-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Info table used for GeniPro interface                 */
/*                                                                          */
/****************************************************************************/

/***************************** Include files *******************************/
#include "geni_cnf.h"
#include "profiles.h"
#include "InfoDef.h"


/*****************************    Defines    *******************************/

/*****************************   Constants   *******************************/

// IMPORTANT NOTE:
// This table must be in sync with the enum defines in InfoDef.h
const INFO_DATA common_info_tab[COM_INFO_LEN] =
{
//Extended precision
// HEADER  ,  UNIT index,          ZERO(H), ZERO(L)
  {0xA3    ,  GENI_UNIT_1X,        0,       0},     //  COM_INDEX_EXT_DIMLESS_255
  {0x83    ,  GENI_UNIT_1X,        0,       0},     //  COM_INDEX_EXT_DIMLESS_254
  {0xA3    ,  GENI_UNIT_1X,        0,       0},     //  COM_INDEX_EXT_GAIN_1
  {0xA3    ,  GENI_UNIT_DOT1X,     0,       0},     //  COM_INDEX_EXT_GAIN_DOT1
  {0xA3    ,  GENI_UNIT_DOT01X,    0,       0},     //  COM_INDEX_EXT_GAIN_DOT01
  {0x83    ,  GENI_UNIT_1PCT,      0,       0},     //  COM_INDEX_EXT_PERCENTAGE_1PCT
  {0x83    ,  GENI_UNIT_DOT1PCT,   0,       0},     //  COM_INDEX_EXT_PERCENTAGE_DOT1PCT
  {0x83    ,  GENI_UNIT_1M,        0,       0},     //  COM_INDEX_EXT_HEAD_1M
  {0x83    ,  GENI_UNIT_DOT01M,    0,       0},     //  COM_INDEX_EXT_HEAD_DOT01M
  {0x83    ,  GENI_UNIT_1M3H,      0,       0},     //  COM_INDEX_EXT_FLOW_1M3H
  {0x83    ,  GENI_UNIT_1LS,       0,       0},     //  COM_INDEX_EXT_FLOW_1LS
  {0x83    ,  GENI_UNIT_DOT1LS,    0,       0},     //  COM_INDEX_EXT_FLOW_DOT1LS
  {0x83    ,  GENI_UNIT_1W,        0,       0},     //  COM_INDEX_EXT_POWER_1W
  {0x83    ,  GENI_UNIT_1KWH ,     0,       0},     //  COM_INDEX_EXT_ENERGY_1KWH
  {0x83    ,  GENI_UNIT_DOT1KWH,   0,       0},     //  COM_INDEX_EXT_ENERGY_DOT1KWH
  {0x83    ,  GENI_UNIT_1WHM3,     0,       0},     //  COM_INDEX_EXT_EFFICIENCY_1WHM3
  {0x83    ,  GENI_UNIT_1M3,       0,       0},     //  COM_INDEX_EXT_VOLUME_1M3
  {0x83    ,  GENI_UNIT_DOT1M3,    0,       0},     //  COM_INDEX_EXT_VOLUME_DOT1M3
  {0x83    ,  GENI_UNIT_1S,        0,       0},     //  COM_INDEX_EXT_TIME_1SEC
  {0x83    ,  GENI_UNIT_1MIN,      0,       0},     //  COM_INDEX_EXT_TIME_1MIN
  {0x83    ,  GENI_UNIT_DOT1A,     0,       0},     //  COM_INDEX_EXT_CURRENT_DOT1A
  {0x83    ,  GENI_UNIT_1HZ,       0,       0},     //  COM_INDEX_EXT_FREQUENCY_1HZ
  {0x83    ,  GENI_UNIT_05HZ,      0,       0},     //  COM_INDEX_EXT_FREQUENCY_05HZ
  {0x83    ,  GENI_UNIT_1OHM,      0,       0},     //  COM_INDEX_EXT_RESISTANCE_1OHM
  {0x83    ,  GENI_UNIT_10KOHM,    0,       0},     //  COM_INDEX_EXT_RESISTANCE_10KHM
  {0x83    ,  GENI_UNIT_1C,        0,       0},     //  COM_INDEX_EXT_TEMPERATURE_1C
  {0x83    ,  GENI_UNIT_DOT1C,     0,       0},     //  COM_INDEX_EXT_TEMPERATURE_DOT1C
  {0x83    ,  GENI_UNIT_DOT01K,    0,       0},     //  COM_INDEX_EXT_TEMPERATURE_DOT01K
  {0x83    ,  GENI_UNIT_DOT1NM,    0,       0},     //  COM_INDEX_EXT_TORQUE_DOT1NM
  {0x83    ,  GENI_UNIT_100MV,     0,       0},     //  COM_INDEX_EXT_VOLTAGE_DOT1V
  {0x83    ,  GENI_UNIT_001HZ,     0,       0},     //  COM_INDEX_EXT_FREQUENCY_001HZ
  {0x83    ,  GENI_UNIT_1MBAR,     0,       0},     //  COM_INDEX_EXT_PRESSURE_1MBAR
  {0x83    ,  GENI_UNIT_DOT1M3,    0,       0},     //  COM_INDEX_EXT_TOTAL_PUMPED_VOLUME_DOT1M3
  {0x83    ,  GENI_UNIT_DOT1M3,    0,       0},     //  COM_INDEX_EXT_TOTAL_OVERFLOW_VOLUME_DOT1M3
  {0x82	   ,  GENI_UNIT_2HZ	  ,    GENI_2HZ_ZERO_VALUE,    	GENI_2HZ_RANGE}

};
