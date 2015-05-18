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
/* CLASS NAME       : -                                                     */
/*                                                                          */
/* FILE NAME        : AlarmDef.h                                            */
/*                                                                          */
/* CREATED DATE     : 17-09-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Defines used for handling alarms.               */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ALARM_DEF_H__
#define __ALARM_DEF_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <FactoryTypes.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

// alarm ID enumeration
typedef enum
{
  FIRST_ALARM_ID = 0,
  ALARM_ID_NO_ALARM                                                      = FIRST_ALARM_ID,
    ALARM_ID_1 = 1,
    ALARM_ID_2 = 2,
    ALARM_ID_3 = 3,
  ALARM_ID_EXTERNAL_FAULT_SIGNAL                                         = 3,
  ALARM_ID_MAINS_FAULT                                                   = 6,
  ALARM_ID_COMMUNICATION_FAULT                                           = 10,
  ALARM_ID_WATER_IN_MOTOR_OIL_FAULT                                      = 11,
  ALARM_ID_TIME_FOR_SERVICE                                              = 12,
  ALARM_ID_MAIN_SYSTEM_COMMUNICATION_FAULT                               = 15,
    ALARM_ID_16 = 16,
  ALARM_ID_OTHER                                                         = 16,
  ALARM_ID_INSULATION_RESISTANCE_LOW                                     = 20,
  ALARM_ID_TOO_MANY_STARTS_PER_H                                         = 21,
  ALARM_ID_HUMIDITY_SWITCH_ALARM                                         = 22,
  ALARM_ID_VIBRATION                                                     = 24,
  ALARM_ID_SETUP_CONFLICT                                                = 25,
  ALARM_ID_MOTOR_PROTECTION_TRIPPED                                      = 27,
    ALARM_ID_30 = 30,
    ALARM_ID_32 = 32,
    ALARM_ID_40 = 40,
    ALARM_ID_48 = 48,
  ALARM_ID_OVERLOAD                                                      = 48,
    ALARM_ID_49 = 49,
  ALARM_ID_PUMP_MOTOR_BLOCKED                                            = 51,
    ALARM_ID_55 = 55,
  ALARM_ID_UNDERLOAD                                                     = 56,
    ALARM_ID_57 = 57,
  ALARM_ID_DRY_RUNNING                                                   = 57,
  ALARM_ID_LOW_FLOW                                                      = 58,
    ALARM_ID_64 = 64,
  ALARM_ID_OVERTEMPERATURE                                               = 64,
  ALARM_ID_TERMO_RELAY_1_IN_MOTOR                                        = 69,
    ALARM_ID_70 = 70,
  ALARM_ID_TERMO_RELAY_2_IN_MOTOR                                        = 70,
  ALARM_ID_HARDWARE_FAULT_TYPE_1                                         = 72,
    ALARM_ID_77 = 77,
  ALARM_ID_HARDWARE_FAULT_TYPE_2                                         = 80,
  ALARM_ID_FE_PARAMETER_AREA_VERIFICATION_ERROR                          = 83,    // FE = Front End
    ALARM_ID_85 = 85,
  ALARM_ID_BE_PARAMETER_AREA_VERIFICATION_ERROR                          = 85,  // BE = Back End
  ALARM_ID_SENSOR_FAULT                                                  = 88,
    ALARM_ID_89 = 89,
    ALARM_ID_91 = 91,
    ALARM_ID_93 = 93,
  ALARM_ID_UPPER_BEARING_TEMPERATURE                                     = 145,
  ALARM_ID_LOWER_BEARING_TEMPERATURE                                     = 146,
    ALARM_ID_148 = 148,
    ALARM_ID_149 = 149,
    ALARM_ID_155 = 155,
  ALARM_ID_CIU_CARD_FAULT                                                = 159,
  ALARM_ID_SIM_CARD_FAULT                                                = 160,
  ALARM_ID_PRESSURE_SENSOR_SIGNAL_FAULT                                  = 168,
  ALARM_ID_FLOW_SENSOR_SIGNAL_FAULT                                      = 169,
  ALARM_ID_WATER_IN_GLYCOL_SENSOR_SIGNAL_FAULT                           = 170,
    ALARM_ID_175 = 175,
    ALARM_ID_176 = 176,
  ALARM_ID_UPPER_BEARING_TEMPERATURE_SIGNAL_FAULT                        = 179,
  ALARM_ID_LOWER_BEARING_TEMPERATURE_SIGNAL_FAULT                        = 180,
  ALARM_ID_TERMO_RELAY_1_IN_MOTOR_SIGNAL_FAULT                           = 181,
  ALARM_ID_POWER_METER                                                   = 186,
  ALARM_ID_USER_DEFINED_SENSOR_FAULT                                     = 188,
  ALARM_ID_LEVEL_SENSOR_SIGNAL_FAULT                                     = 189,
    ALARM_ID_190 = 190,
  ALARM_ID_ALARM_LEVEL                                                   = 190,
    ALARM_ID_191 = 191,
  ALARM_ID_HIGH_LEVEL                                                    = 191,
  ALARM_ID_OVERFLOW                                                      = 192,
  ALARM_ID_CONFLICTING_LEVELS                                            = 204,
  ALARM_ID_LEVEL_FLOAT_SWITCH_INCONSISTENCY                              = 205,
  ALARM_ID_CONTACTOR                                                     = 220,
  ALARM_ID_MIXER_CONTACTOR                                               = 221,
  ALARM_ID_MIXER_SERVICE_TIME                                            = 222,
  ALARM_ID_MIXER_STARTS_PR_HOUR                                          = 223,
  ALARM_ID_GENIBUS_PUMP_MODULE                                           = 225,
  ALARM_ID_GENIBUS_IO_MODULE                                             = 226,
  ALARM_ID_COMBI_ALARM                                                   = 227,
    ALARM_ID_228 = 228,
  ALARM_ID_WATER_ON_FLOOR                                                = 229,
  ALARM_ID_ETHERNET_NO_IP_ADDRESS_FROM_DHCP_SERVER                       = 231,
  ALARM_ID_ETHERNET_AUTO_DISABLED_DUE_TO_MISUSE                          = 232,
  ALARM_ID_GAS_DETECTOR                                                  = 235,
    ALARM_ID_240 = 240,
    ALARM_ID_241 = 241,
  ALARM_ID_COMMON_PHASE                                                  = 241,
    ALARM_ID_242 = 242,
  ALARM_ID_PUMP_MANUAL                                                   = 243,
  ALARM_ID_ON_OFF_AUTO_SWITCH                                            = 244,
  ALARM_ID_LATEST_RUNTIME                                                = 245,
  ALARM_ID_CUSTOM_RELAY_ACTIVATED                                        = 246,
  ALARM_ID_POWER_ON_OCCURED_WARNING                                      = 247,
  ALARM_ID_UNINTERRUPTABLE_POWER_SUPPLY                                  = 248,
  ALARM_ID_EXTRA_FAULT_1                                                 = 249,
  ALARM_ID_EXTRA_FAULT_2                                                 = 250,
  ALARM_ID_EXTRA_FAULT_3                                                 = 251,
  ALARM_ID_EXTRA_FAULT_4                                                 = 252,
  ALARM_ID_SCADA_CALLBACK_TEST                                           = 253,


  // Insert alarm id's above
  NO_OF_ALARM_ID,
  LAST_ALARM_ID = NO_OF_ALARM_ID - 1

} ALARM_ID_TYPE;

// alarm state table
typedef struct
{
  SYSTEM_MODE_TYPE    systemMode;
  OPERATION_MODE_TYPE operationMode;
} ALARM_STATE_TABLE_TYPE;

// alarm priority enumeration
typedef enum
{
  FIRST_ALARM_PRIORITY,
  ALARM_PRIORITY_1 = FIRST_ALARM_PRIORITY,
  ALARM_PRIORITY_2,
  ALARM_PRIORITY_3,
  ALARM_PRIORITY_4,
  ALARM_PRIORITY_5,
  ALARM_PRIORITY_6,
  ALARM_PRIORITY_7,
  ALARM_PRIORITY_8,
  // insert alarm priority above
  NO_OF_ALARM_PRIORITY,
  LAST_ALARM_PRIORITY = NO_OF_ALARM_PRIORITY - 1
} ALARM_PRIORITY_TYPE;


// alarm relay enumeration
typedef enum
{
  FIRST_ALARM_RELAY,
  ALARM_RELAY_CUSTOM = FIRST_ALARM_RELAY,
  ALARM_RELAY_HIGH_LEVEL,
  ALARM_RELAY_CRITICAL,
  ALARM_RELAY_ALL_ALARMS,
  ALARM_RELAY_ALL_ALARMS_AND_WARNINGS,
  // insert alarm relays above
  NO_OF_ALARM_RELAYS,
  LAST_ALARM_RELAY = NO_OF_ALARM_RELAYS - 1
} ALARM_RELAY_TYPE;


#endif
