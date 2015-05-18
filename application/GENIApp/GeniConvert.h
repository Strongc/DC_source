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
/* CLASS NAME       : GeniConvert                                           */
/*                                                                          */
/* FILE NAME        : GeniConvert.h                                         */
/*                                                                          */
/* CREATED DATE     : 29-02-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Conversion between data points to Geni value    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <GeniConvertId.h>
#include <FloatDataPoint.h>
#include <IIntegerDataPoint.h>
#include <EnumDataPoint.h>
#include <AlarmConfig.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  FUNCTION PROTOTYPES
 *****************************************************************************/
U8 ToGeni8bitValue(FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
U8 ToGeni8bitValue(IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);

U8 ValueToGeni8bitValue(float fValue, DP_QUALITY_TYPE dpQuality, GENI_CONVERT_ID_TYPE convertId);
U8 ValueToGeni8bitValue(int iValue, DP_QUALITY_TYPE dpQuality, GENI_CONVERT_ID_TYPE convertId);

U16 ToGeni16bitValue(FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
U16 ToGeni16bitValue(IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
U16 ToGeni16bitValue(AlarmConfig* pDP, GENI_CONVERT_ID_TYPE convertId);

U32 ToGeni32bitValue(FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
U32 ToGeni32bitValue(IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);


void GeniToDataPoint(U8 newValue, FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
void GeniToDataPoint(U8 newValue, IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);

void GeniToDataPoint(U16 newValue, FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
void GeniToDataPoint(U16 newValue, IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
void GeniToDataPoint(U16 newValue, AlarmConfig* pDP, GENI_CONVERT_ID_TYPE convertId);
void GeniToDataPoint(U16 newValue, AlarmConfig* pDP, ALARM_MODE alarmMode, GENI_CONVERT_ID_TYPE convertId);

void GeniToDataPoint(U32 newValue, FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
void GeniToDataPoint(U32 newValue, IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId);
U32 geni_convert_dot1m3(U32 value, GENI_CONVERT_ID_TYPE convertId, U32 maxValue);
