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
 /* FILE NAME        : UnitTypes.h                                           */
 /*                                                                          */
 /* CREATED DATE     : 2004-09-17                                            */
 /*                                                                          */
 /* SHORT FILE DESCRIPTION :                                                 */
 /* This is the definition of physical units                                 */
 /****************************************************************************/
 /*****************************************************************************
    Protect against multiple inclusion through the use of guards:
  ****************************************************************************/
 #ifndef __UNIT_TYPES_H__
 #define __UNIT_TYPES_H__

 /*****************************************************************************
   SYSTEM INCLUDES
  *****************************************************************************/

 /*****************************************************************************
   PROJECT INCLUDES
  *****************************************************************************/
  
 /*****************************************************************************
   LOCAL INCLUDES
  ****************************************************************************/

 /*****************************************************************************
   DEFINES
  *****************************************************************************/

 /*****************************************************************************
   TYPE DEFINES
  *****************************************************************************/

typedef enum 
{
  Q_NO_UNIT = 0,
  Q_FLOW,
  Q_ENERGY,
  Q_TEMPERATURE,
  Q_DIFFERENCIAL_TEMPERATURE,
  Q_HEIGHT,
  Q_DEPTH,
  Q_HEAD,
  Q_PRESSURE,
  Q_DIFFERENCIAL_PRESSURE,
  Q_VOLTAGE,
  Q_LOW_CURRENT,
  Q_SPECIFIC_ENERGY,
  Q_POWER,
  Q_COS_PHI,
  Q_VOLUME,
  Q_PERCENT,
  Q_TIME,
  Q_PH_VALUE,
  Q_FREQUENCY,
  Q_REVOLUTION,
  Q_PERFORMANCE,
  Q_RESISTANCE,
  Q_AREA,
  Q_CONDUCTIVITY,
  Q_FORCE,
  Q_TORQUE,
  Q_VELOCITY,
  Q_MASS,
  Q_ACCELERATION,
  Q_MASS_FLOW,
  Q_ANGULAR_VELOCITY,
  Q_ANGULAR_ACCELERATION,
  Q_LUMINOUS_INTENSITY,
  Q_CLOCK_HOUR,
  Q_CLOCK_MINUTE,
  Q_CLOCK_DAY,
  Q_CLOCK_MONTH,
  Q_CLOCK_YEAR,
  Q_TIME_SUM,
  Q_RATIO,
  Q_MAC_ADDRESS,
  Q_DATA_SIZE,
  Q_DATETIME,
  Q_HIGH_CURRENT,
  Q_SMALL_AREA,
  Q_HIGH_VELOCITY,
  Q_PARTS_PER_MILLION,
  Q_SMALL_FLOW,
  Q_LAST_UNIT
} QUANTITY_TYPE;


#endif
