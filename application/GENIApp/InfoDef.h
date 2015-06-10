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
/* CLASS NAME       : InfoDef                                               */
/*                                                                          */
/* FILE NAME        : InfoDef.h                                             */
/*                                                                          */
/* CREATED DATE     : 14-05-2005                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : This file contains various definitions that are */
/*                          useful when using the geni INFO scaling system. */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcInfoDef_h
#define mpcInfoDef_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <binary.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define GENI_INFO_HEAD           0
#define GENI_INFO_UNIT           1
#define GENI_INFO_ZERO           2
#define GENI_INFO_RANGE          3
#define GENI_INFO_SIZE           4

/* INFO CODES                                                               */
#define NO_INFO           b10000000 /* No info available                    */
#define BITVAR_254        b10000001 /* Bitwise, 255 = data not available    */
#define BITVAR_255        b10100001 /* Bitwise, 255 = valid data            */
#define COMMON_INFO       b01000000 /* Value with constant scale information*/
#define COMMON_PTR        b11000000 /* Value with scale information         */
#define EXECUTABLE        b10100000 /* Command exists                       */
#define UNEXECUTABLE      b10101100 /* Command does not exist               */
#define NOT_CONFIGURABLE  b10001100 /* Class 4 parameter - not configurable */
#define LOWER_BYTE        b10110000 /* Low order byte to 16 bit value       */

/* Masks for extracting information from INFO HEAD */
#define GENI_INFO_SZ_MASK    b10000000 /* Isolates Sign of Zero             */
#define GENI_INFO_SIF_MASK   b00000011 /* Isolates Scale Information Format */
#define GENI_INFO_VI_MASK    b00100000 /* Isolates Value Interpretation     */

#define GENI_INFO_SZ_NEGATIVE        b10000000 /* Zero is negative           */
#define GENI_INFO_SIF_SCALED_VALUE   b00000010 /* UNIT, ZERO and RANGE exist */
#define GENI_INFO_EXTENDED_PRECISION b00000011 /* Extended precision used    */
#define GENI_INFO_VI_0_255           b00100000 /* 0-255 is valid             */

/* INFO UNITS definitions */
/* Percentage */
#define GENI_UNIT_DOT1PCT        12
#define GENI_UNIT_1PCT           30
#define GENI_UNIT_10PCT          76
#define GENI_UNIT_1PPM           113

/* Pressure */
#define GENI_UNIT_1MBAR          51
#define GENI_UNIT_DOT01BAR       27
#define GENI_UNIT_DOT1BAR        28
#define GENI_UNIT_1BAR           29
/* Flow */
#define GENI_UNIT_DOT01M3H       95
#define GENI_UNIT_DOT1M3H        22
#define GENI_UNIT_1M3H           23
#define GENI_UNIT_10M3H          92
#define GENI_UNIT_100M3H         93
#define GENI_UNIT_1LS            52
#define GENI_UNIT_DOT1LS         63
#define GENI_UNIT_DOT1LH         114
/* Volume */
#define GENI_UNIT_1M3            86
#define GENI_UNIT_DOT1M3         64
#define GENI_UNIT_PEDDOT1M3		   64
#define GENI_UNIT_1ML            88
/* Level */
#define GENI_UNIT_1M             25
#define GENI_UNIT_DOT01M         83
/* Current */
#define GENI_UNIT_1UA            15
#define GENI_UNIT_100MA           1
#define GENI_UNIT_DOT1A           1
#define GENI_UNIT_200MA          42
#define GENI_UNIT_02A            42
#define GENI_UNIT_500MA          62
#define GENI_UNIT_05A            62
#define GENI_UNIT_5A              2
/* Voltage */
#define GENI_UNIT_100MV           3
#define GENI_UNIT_1V              4
#define GENI_UNIT_2V            104
#define GENI_UNIT_5V              5
/* Power */
#define GENI_UNIT_1W              7
#define GENI_UNIT_10W             8
#define GENI_UNIT_100W            9
#define GENI_UNIT_1KW            44
#define GENI_UNIT_10KW           45
/* Temperature */
#define GENI_UNIT_DOT1C          20
#define GENI_UNIT_1C             21
#define GENI_UNIT_DOT01K         84
#define GENI_UNIT_1F             57
/* Time */
#define GENI_UNIT_1024H          39
#define GENI_UNIT_100H           81
#define GENI_UNIT_1024MIN        72
#define GENI_UNIT_2H             13
#define GENI_UNIT_1H             35
#define GENI_UNIT_2MIN           36
#define GENI_UNIT_1MIN           80
#define GENI_UNIT_30S            14
#define GENI_UNIT_10S            78
#define GENI_UNIT_1S             37
#define GENI_UNIT_DOT1S          79
/* Energy */
#define GENI_UNIT_1WS            87
#define GENI_UNIT_1WH            94
#define GENI_UNIT_DOT1KWH       103
#define GENI_UNIT_1KWH           31
#define GENI_UNIT_2KWH           85
#define GENI_UNIT_10KWH          32
#define GENI_UNIT_100KWH         33
#define GENI_UNIT_512KWH         40
#define GENI_UNIT_1MWH           46
#define GENI_UNIT_10MWH          47
#define GENI_UNIT_100MWH         48
/* Energy per volume */
#define GENI_UNIT_1WHM3          74
/* Gain */
#define GENI_UNIT_1X             50
#define GENI_UNIT_DOT1X          96
#define GENI_UNIT_DOT01X         77
/* Frequency */
#define GENI_UNIT_001HZ          105
#define GENI_UNIT_05HZ           11
#define GENI_UNIT_1HZ            16
#define GENI_UNIT_2HZ            38
#define GENI_UNIT_2DOT5HZ        17
/* Resistance */
#define GENI_UNIT_1OHM            6
#define GENI_UNIT_10KOHM         43
/* Rot. velocity */
#define GENI_UNIT_1RPM           98
#define GENI_UNIT_12RPM          18
#define GENI_UNIT_100RPM         19
/* Torque */
#define GENI_UNIT_1NM            75
#define GENI_UNIT_DOT1NM         97


#define GENI_2HZ_ZERO_VALUE      15
#define	GENI_2HZ_RANGE			 10

/* Indexes to common info table.
 * Do not change sequence without changing common_info_tab */
typedef enum
{
  COM_INDEX_EXT_DIMLESS_255 = 0,
  COM_INDEX_EXT_DIMLESS_254,
  COM_INDEX_EXT_GAIN_1,
  COM_INDEX_EXT_GAIN_DOT1,
  COM_INDEX_EXT_GAIN_DOT01,
  COM_INDEX_EXT_PERCENTAGE_1PCT,
  COM_INDEX_EXT_PERCENTAGE_DOT1PCT,
  COM_INDEX_EXT_HEAD_1M,
  COM_INDEX_EXT_HEAD_DOT01M,
  COM_INDEX_EXT_FLOW_1M3H,
  COM_INDEX_EXT_FLOW_1LS,
  COM_INDEX_EXT_FLOW_DOT1LS,
  COM_INDEX_EXT_POWER_1W,
  COM_INDEX_EXT_ENERGY_1KWH,
  COM_INDEX_EXT_ENERGY_DOT1KWH,
  COM_INDEX_EXT_EFFICIENCY_1WHM3,
  COM_INDEX_EXT_VOLUME_1M3,
  COM_INDEX_EXT_VOLUME_DOT1M3,
  COM_INDEX_EXT_TIME_1SEC,
  COM_INDEX_EXT_TIME_1MIN,
  COM_INDEX_EXT_CURRENT_DOT1A,
  COM_INDEX_EXT_FREQUENCY_1HZ,
  COM_INDEX_EXT_FREQUENCY_05HZ,
  COM_INDEX_EXT_RESISTANCE_1OHM,
  COM_INDEX_EXT_RESISTANCE_10KOHM,
  COM_INDEX_EXT_TEMPERATURE_1C,
  COM_INDEX_EXT_TEMPERATURE_DOT1C,
  COM_INDEX_EXT_TEMPERATURE_DOT01K,
  COM_INDEX_EXT_TORQUE_DOT1NM,
  COM_INDEX_EXT_VOLTAGE_DOT1V,
  COM_INDEX_EXT_FREQUENCY_001HZ,
  COM_INDEX_EXT_PRESSURE_1MBAR,
  COM_INDEX_EXT_TOTAL_VOLUME_DOT1M3,
  COM_INDEX_EXT_OVER_VOLUME_DOT1M3,
  COM_INDEX_EXT_FREQUENCY_2HZ,
  COM_INDEX_EXT_PERCENTAGE_1PPM,
  COM_INDEX_EXT_FLOW_DOT1LH,
  COM_INDEX_EXT_VOLUME_1ML,
  /****************/
  LAST_COM_INDEX_EXT,
  NO_OF_COM_INDEX_EXT = LAST_COM_INDEX_EXT - 1
} COM_INDEX_EXT_TYPE;


typedef enum
{
  COM_PTR_INFO_REF = 0,

  /****************/
  LAST_COM_PTR_INFO,
  NO_OF_COM_PTR_INFO = LAST_COM_PTR_INFO - 1
} COM_PTR_INFO_TYPE;

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/


#endif


