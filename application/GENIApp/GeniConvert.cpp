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
/* FILE NAME        : GeniConvert.cpp                                       */
/*                                                                          */
/* CREATED DATE     : 29-02-2008 dd-mm-yyyy                                 */
/* SHORT FILE DESCRIPTION : See .h file                                     */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <GeniConvert.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
extern U16							  mcounter;
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  PRIVATE FUNCTIONS
 *****************************************************************************/

/*****************************************************************************
 * Function - ConvertToGeni
 * DESCRIPTION: Template for conversion from data point value to Geni value
 *
 *****************************************************************************/
template<typename IN_TYPE, class OUT_TYPE>
static OUT_TYPE ConvertToGeni(IN_TYPE value, GENI_CONVERT_ID_TYPE convertId, U32 maxValue)
{
  U32 convertedValue = 0;

  switch (convertId)
  {
    case GENI_CONVERT_ID_DIMLESS_254:
    case GENI_CONVERT_ID_DIMLESS_255:
    case GENI_CONVERT_ID_BITVAR_254:
    case GENI_CONVERT_ID_BITVAR_255:
    case GENI_CONVERT_ID_PERCENTAGE_1PCT:
    case GENI_CONVERT_ID_HEAD_DIST_1M:
    case GENI_CONVERT_ID_POWER_1W:
    case GENI_CONVERT_ID_ENERGY_1KWH:
    case GENI_CONVERT_ID_VOLUME_1M3:
    case GENI_CONVERT_ID_TIME_1SEC:
    case GENI_CONVERT_ID_FREQUENCY_1HZ:
    case GENI_CONVERT_ID_RESISTANCE_1OHM:
    case GENI_CONVERT_ID_PERCENTAGE_1PPM:
      convertedValue = (U32)value;
      break;

	case GENI_CONVERT_ID_FREQUENCY_2HZ:			
			convertedValue = (U32)(254.0f*(value-(15.0f*2)))/(10*2) + 0.5f;			    
			break;    

    case GENI_CONVERT_ID_EFFICIENCY_1WHM3:
      convertedValue = (U32)(value / 3600.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_SCALE_DOT1:
    case GENI_CONVERT_ID_PERCENTAGE_DOT1PCT:
    case GENI_CONVERT_ID_ENERGY_KWH_DOT1KWH:
    case GENI_CONVERT_ID_VOLUME_DOT1M3:
    case GENI_CONVERT_ID_CURRENT_DOT1A:
    case GENI_CONVERT_ID_TORQUE_DOT1NM:
    case GENI_CONVERT_ID_VOLTAGE_DOT1V:
      convertedValue = (U32)(value * 10.0f + 0.5f);
      break;

 /*  case GENI_CONVERT_ID_VOLUME_DOT1M3:	 
	   convertedValue = (U32)(value / 100.0f + 0.5f);
      break;
*/
	case GENI_CONVERT_ID_TOTAL_VOLUME_DOT1M3:
	  convertedValue = (U32)(value / 100.0f + 0.5f);
      break;
    case GENI_CONVERT_ID_OVER_VOLUME_DOT1M3:
	  convertedValue = (U32)(value / 100.0f + 0.5f);
      break;
    case GENI_CONVERT_ID_SCALE_DOT01:
    case GENI_CONVERT_ID_HEAD_DIST_DOT01M:
    case GENI_CONVERT_ID_TEMPERATURE_DOT01K:
    case GENI_CONVERT_ID_FREQUENCY_001HZ:
      convertedValue = (U32)(value * 100.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_SCALE_DOT001:
      convertedValue = (U32)(value * 1000.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_SCALE_X10:
      convertedValue = (U32)(value / 10.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_SCALE_X100:
      convertedValue = (U32)(value / 100.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_SCALE_X1000:
      convertedValue = (U32)(value / 1000.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_FLOW_1M3H:
      convertedValue = (U32)(value * 3600.0f + 0.5f);
      break;
    case GENI_CONVERT_ID_FLOW_1LS:
      convertedValue = (U32)(value * 1000.0f + 0.5f);
      break;
    case GENI_CONVERT_ID_FLOW_DOT1LS:
      convertedValue = (U32)(value * 10000.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_TIME_1MIN:
      convertedValue = (U32)(value / 60);
      break;

    case GENI_CONVERT_ID_TEMPERATURE_1C:
      if ( value > 273.15f)
      {
        convertedValue = (OUT_TYPE)(value - 273.15f);
      }
      break;
    case GENI_CONVERT_ID_TEMPERATURE_DOT1C:
      if ( value > 273.15f)
      {
        convertedValue = (OUT_TYPE)((value - 273.15f) * 10.0f);
      }
      break;

    case GENI_CONVERT_ID_FREQUENCY_05HZ:
      convertedValue = (U32)(value * 2);
      break;

    case GENI_CONVERT_ID_RESISTANCE_10KOHM:
      convertedValue = (U32)(value / 10000 + 0.5f);
      break;

    case GENI_CONVERT_ID_ENERGY_JOULE_DOT1KWH:
      convertedValue = (U32)(value/(3600000.0f/10.0f) + 0.5f);
      break;

    case GENI_CONVERT_ID_PRESSURE_1MBAR:
      convertedValue = (U32)(value * 0.01f + 0.5f);
      break;

    case GENI_CONVERT_ID_FLOW_DOT1LH:
      convertedValue = (U32)(value * 10.0f + 0.5f);
      break;

    case GENI_CONVERT_ID_VOLUME_1ML:
      convertedValue = (U32)(value * 1000.0f + 0.5f);
      break;

    default:
      convertedValue = (U32)0; // unhandled convert id
      break;
  }

  // check max value range
  if (convertedValue > maxValue)
  {
    convertedValue = maxValue;
  }

  return (OUT_TYPE)convertedValue;
}


/*****************************************************************************
 * Function - ConvertFromGeni
 * DESCRIPTION: Template for conversion from Geni value to data point value
 *
 *****************************************************************************/
template<typename IN_TYPE, class OUT_TYPE>
static OUT_TYPE ConvertFromGeni(IN_TYPE value, GENI_CONVERT_ID_TYPE convertId)
{
  OUT_TYPE convertedValue;

  switch (convertId)
  {
    case GENI_CONVERT_ID_DIMLESS_254:
    case GENI_CONVERT_ID_DIMLESS_255:
    case GENI_CONVERT_ID_BITVAR_254:
    case GENI_CONVERT_ID_BITVAR_255:
    case GENI_CONVERT_ID_PERCENTAGE_1PCT:
    case GENI_CONVERT_ID_HEAD_DIST_1M:
    case GENI_CONVERT_ID_POWER_1W:
    case GENI_CONVERT_ID_ENERGY_1KWH:
    case GENI_CONVERT_ID_VOLUME_1M3:
    case GENI_CONVERT_ID_TIME_1SEC:
    case GENI_CONVERT_ID_FREQUENCY_1HZ:
    case GENI_CONVERT_ID_RESISTANCE_1OHM:	
    case GENI_CONVERT_ID_PERCENTAGE_1PPM:
      convertedValue = (OUT_TYPE)value;
    break;
	

	case GENI_CONVERT_ID_FREQUENCY_2HZ:
	   convertedValue = (OUT_TYPE) (15.0f+ (value*10/254.0f))*2.0f;	
    break;

    case GENI_CONVERT_ID_EFFICIENCY_1WHM3:
      convertedValue = (OUT_TYPE)value * 3600;
    break;


    case GENI_CONVERT_ID_SCALE_DOT1:
    case GENI_CONVERT_ID_PERCENTAGE_DOT1PCT:
    case GENI_CONVERT_ID_ENERGY_KWH_DOT1KWH:
    case GENI_CONVERT_ID_VOLUME_DOT1M3:
    case GENI_CONVERT_ID_CURRENT_DOT1A:
    case GENI_CONVERT_ID_TORQUE_DOT1NM:
    case GENI_CONVERT_ID_VOLTAGE_DOT1V:
      convertedValue = (OUT_TYPE)value / 10;
      break;

    case GENI_CONVERT_ID_SCALE_DOT01:
    case GENI_CONVERT_ID_HEAD_DIST_DOT01M:
    case GENI_CONVERT_ID_TEMPERATURE_DOT01K:
    case GENI_CONVERT_ID_FREQUENCY_001HZ:
      convertedValue = (OUT_TYPE)value / 100;
      break;

    case GENI_CONVERT_ID_SCALE_DOT001:
      convertedValue = (OUT_TYPE)value / 1000;
      break;

    case GENI_CONVERT_ID_SCALE_X10:
      convertedValue = (OUT_TYPE)value * 10;
      break;

    case GENI_CONVERT_ID_SCALE_X100:
      convertedValue = (OUT_TYPE)value * 100;
      break;

    case GENI_CONVERT_ID_SCALE_X1000:
      convertedValue = (OUT_TYPE)value * 1000;
      break;

    case GENI_CONVERT_ID_FLOW_1M3H:
      convertedValue = (OUT_TYPE)value / 3600;
      break;
    case GENI_CONVERT_ID_FLOW_1LS:
      convertedValue = (OUT_TYPE)value / 1000;
      break;
    case GENI_CONVERT_ID_FLOW_DOT1LS:
      convertedValue = (OUT_TYPE)value / 10000;
      break;

    case GENI_CONVERT_ID_TIME_1MIN:
      convertedValue = (OUT_TYPE)value * 60;
      break;

    case GENI_CONVERT_ID_TEMPERATURE_1C:
      convertedValue = (OUT_TYPE)value + (OUT_TYPE)273.15f;
      break;
    case GENI_CONVERT_ID_TEMPERATURE_DOT1C:
      convertedValue = (OUT_TYPE)value / 10 + (OUT_TYPE)273.15f;
      break;

    case GENI_CONVERT_ID_FREQUENCY_05HZ:
      convertedValue = (OUT_TYPE)value / 2;
      break;

    case GENI_CONVERT_ID_RESISTANCE_10KOHM:
      convertedValue = (OUT_TYPE)value * 10000;
      break;

    case GENI_CONVERT_ID_ENERGY_JOULE_DOT1KWH:
      convertedValue = (OUT_TYPE)value*3600000.0f / 10.0f;
      break;
      
    case GENI_CONVERT_ID_FLOW_DOT1LH:
      convertedValue = (OUT_TYPE)value / 10.0f ;
      break;

    case GENI_CONVERT_ID_VOLUME_1ML:
      convertedValue = (OUT_TYPE)value / 1000.0f ;
      break;
      
    default:
      convertedValue = 0; // unhandled convert id
      break;
  }

  return convertedValue;
}

/*****************************************************************************
 * PUBLIC FUNCTIONS:
 * DESCRIPTION: To Geni from direct value
 *
 *****************************************************************************/
U8 ValueToGeni8bitValue(float fValue, DP_QUALITY_TYPE dpQuality, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (dpQuality != DP_AVAILABLE))
  {
    return 0xFF;
  }
  else
  {
    return ConvertToGeni<float, U8>(fValue, convertId, 0xFF);
  }
}

/*****************************************************************************/
U8 ValueToGeni8bitValue(int iValue, DP_QUALITY_TYPE dpQuality, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (dpQuality != DP_AVAILABLE))
  {
    return 0xFF;
  }
  else
  {
    return ConvertToGeni<int, U8>(iValue, convertId, 0xFF);
  }
}

/*****************************************************************************
 * PUBLIC FUNCTIONS:
 * DESCRIPTION: To Geni from data point
 *
 *****************************************************************************/
U8 ToGeni8bitValue(FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (pDP->GetQuality() != DP_AVAILABLE))
  {
    return 0xFF;
  }
  else
  {
		return ConvertToGeni<float, U8>(pDP->GetValue(), convertId, 0xFF);
  }
}

/*****************************************************************************/
U8 ToGeni8bitValue(IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (pDP->GetQuality() != DP_AVAILABLE))
  {
    return 0xFF;
  }
  else
  {
    return ConvertToGeni<int, U8>(pDP->GetAsInt(), convertId, 0xFF);
  }
}

/*****************************************************************************/
U16 ToGeni16bitValue(FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (pDP->GetQuality() != DP_AVAILABLE))
  {
    return 0xFFFF;
  }
  else
  {
    return ConvertToGeni<float, U16>(pDP->GetValue(), convertId, 0xFFFF);
  }
}

/*****************************************************************************/
U16 ToGeni16bitValue(IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (pDP->GetQuality() != DP_AVAILABLE))
  {
    return 0xFFFF;
  }
  else
  {
    return ConvertToGeni<int, U16>(pDP->GetAsInt(), convertId, 0xFFFF);
  }
}

/*****************************************************************************/
U16 ToGeni16bitValue(AlarmConfig* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (pDP->GetLimitTypeIsFloat() == true)
  {
    return ConvertToGeni<float, U16>(pDP->GetAlarmLimit()->GetAsFloat(), convertId, 0xFFFF);
  }
  else
  {
    return ConvertToGeni<int, U16>(pDP->GetAlarmLimit()->GetAsInt(), convertId, 0xFFFF);
  }
}


/*****************************************************************************/
U32 ToGeni32bitValue(FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (pDP->GetQuality() != DP_AVAILABLE))
  {
    return 0xFFFFFFFF;
  }
  else
  {
    return ConvertToGeni<float, U32>(pDP->GetValue(), convertId, 0xFFFFFFFF);
  }
}

/*****************************************************************************/
U32 ToGeni32bitValue(IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (pDP->GetQuality() != DP_AVAILABLE))
  {
    return 0xFFFFFFFF;
  }
  else
  {
	//  if(convertId != GENI_CONVERT_ID_VOLUME_DOT1M3)
	  if(convertId != GENI_CONVERT_ID_TOTAL_VOLUME_DOT1M3 && convertId != GENI_CONVERT_ID_OVER_VOLUME_DOT1M3)
	  {
		   return ConvertToGeni<int, U32>(pDP->GetAsInt(), convertId, 0xFFFFFFFF);
	  }
	  else
	  {
		   return geni_convert_dot1m3((U32)pDP->GetAsInt(), convertId, 0xFFFFFFFF);
	  }
	
  }
}



/*****************************************************************************
 * PUBLIC FUNCTIONS:
 * DESCRIPTION: From Geni to data point
 *
 *****************************************************************************/
void GeniToDataPoint(U8 newValue, FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    pDP->SetValue(ConvertFromGeni<U8, float>(newValue, convertId));
  }
}

/*****************************************************************************/
void GeniToDataPoint(U8 newValue, IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    pDP->SetAsInt(ConvertFromGeni<U8, int>(newValue, convertId));
  }
}

/*****************************************************************************/
void GeniToDataPoint(U16 newValue, FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFFFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    pDP->SetValue(ConvertFromGeni<U16, float>(newValue, convertId));
  }
}

/*****************************************************************************/
void GeniToDataPoint(U16 newValue, IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFFFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    pDP->SetAsInt(ConvertFromGeni<U16, int>(newValue, convertId));
  }
}

/*****************************************************************************/
void GeniToDataPoint(U16 newValue, AlarmConfig* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFFFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    if (pDP->GetLimitTypeIsFloat() == true)
    {
      pDP->SetAlarmLimit(ConvertFromGeni<U16, float>(newValue, convertId));
    }
    else
    {
      pDP->SetAlarmLimit(ConvertFromGeni<U16, int>(newValue, convertId));
    }
  }
}

/*****************************************************************************/
void GeniToDataPoint(U16 newValue, AlarmConfig* pDP, ALARM_MODE alarmMode, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFFFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
	  if(alarmMode == ALARM)
	  {
	    GeniToDataPoint(newValue, pDP, convertId);
	  }
	  else if(alarmMode == WARNING)
	  {
	    if (pDP->GetLimitTypeIsFloat() == true)
	    {
	      pDP->SetWarningLimit(ConvertFromGeni<U16, float>(newValue, convertId));
	    }
	    else
	    {
	      pDP->SetWarningLimit(ConvertFromGeni<U16, int>(newValue, convertId));
	    }
    }
  }
}
/*****************************************************************************/
void GeniToDataPoint(U32 newValue, FloatDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFFFFFFFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    pDP->SetValue(ConvertFromGeni<U32, float>(newValue, convertId));
  }
}

/*****************************************************************************/
void GeniToDataPoint(U32 newValue, IIntegerDataPoint* pDP, GENI_CONVERT_ID_TYPE convertId)
{
  if (HasGeniNASupport(convertId) && (newValue == 0xFFFFFFFF))
  {
    ; // Don't use the NA-value
  }
  else
  {
    pDP->SetAsInt(ConvertFromGeni<U32, int>(newValue, convertId));
  }
}


//Dot1m3 conversion

U32 geni_convert_dot1m3(U32 value, GENI_CONVERT_ID_TYPE convertId, U32 maxValue)
{
	  U32 convertedValue = 0;
	  U8 temp_value;
	  temp_value = mcounter;

  switch (convertId)
  {

  // case GENI_CONVERT_ID_VOLUME_DOT1M3:
  case GENI_CONVERT_ID_TOTAL_VOLUME_DOT1M3:
       U32 temp_val;
       temp_val= value;
	   convertedValue = (U32)(value / 100.0f + 0.5f);

	   if(mcounter > 0 && mcounter < 100)
	   {
		   U32 temp_val1;
		   temp_val1 = convertedValue + (mcounter * 42949670);
		   temp_val = temp_val1;
		   convertedValue = temp_val1;
	   }
      break;
  case GENI_CONVERT_ID_OVER_VOLUME_DOT1M3:	   
	   convertedValue = (U32)(value / 100.0f + 0.5f);
	  break;

    default:
      convertedValue = (U32)0; // unhandled convert id
      break;
  }

  // check max value range
  if (convertedValue > maxValue)
  {
    convertedValue = maxValue;
  }

  return convertedValue;
}
