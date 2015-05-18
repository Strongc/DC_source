/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange Controller                           */
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
/* CLASS NAME       : NumericalDataPointInterface                           */
/*                                                                          */
/* FILE NAME        : NumericalDataPointInterface.cpp                       */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "NumericalDataPointInterface.h"
#include <math.h>
#include <TimeFormatDataPoint.h>
#include <FloatDataPoint.h>
#include <AlarmConfig.h>
#include <TimeText.h>
#include <MpcTime.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define STR_DP_NOT_AVAILABLE "--"
#define NO_OF_REQUIRED_ACCELERATION_REQUESTS 5

#define ONE_MINUTE_IN_SEC        60
#define TEN_MINUTES_IN_SEC      600
#define ONE_HOUR_IN_SEC        3600
#define TEN_HOURS_IN_SEC      36000
#define TWENTYFOUR_HOURS_IN_SEC (24*60*60)

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    *
    *
    *              PUBLIC FUNCTIONS
    *
    *
    *****************************************************************************/

    /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *  we can only handle subjects derived from NumberDataPoint + (alarm limits of) AlarmConfigDatapoints
    *****************************************************************************/
    NumericalDataPointInterface::NumericalDataPointInterface(Subject* pSubject)
    {
      mNumber = dynamic_cast<INumberDataPoint*>(pSubject);
      mCapturedNumber = NULL;
      mNumberOfAccelerationRequests = 0;

      if (mNumber == NULL)
      {	
        AlarmConfig* ac = dynamic_cast<AlarmConfig*>(pSubject);
        if (ac != NULL)
        {
          mNumber = ac->GetAlarmLimit();
        }
        else
        {
          FatalErrorOccured("NDPI: not a NumberDataPoint!");
        }
      }
    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    NumericalDataPointInterface::~NumericalDataPointInterface()
    {
      // make sure captured data point is released to avoid memory leak
      ReleaseDataPoint();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *  string points to a char array allocated by caller, which must be large enough to hold the max value of the datapoint 
    *****************************************************************************/
    void NumericalDataPointInterface::GetDataPointAsString(char* string, int numberOfDigits)
    {
      GetAsString(string, numberOfDigits, mNumber);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * string points to a char array allocated by caller, which must be large enough to hold the max value of the datapoint 
    *****************************************************************************/
    void NumericalDataPointInterface::GetCapturedDataPointAsString(char* string, int numberOfDigits)
    {
      GetAsString(string, numberOfDigits, mCapturedNumber);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    float NumericalDataPointInterface::GetDataPointAsFloat(void)
    {
      if (mNumber)
      {
        return mNumber->GetAsFloat();
      }
      else
      {
        return 0.0;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    int NumericalDataPointInterface::GetValueAsInt(void)
    {
      if (mNumber)
      {
        return mNumber->GetAsInt();
      }
      else
      {
        return 0;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Subject* NumericalDataPointInterface::CaptureDataPoint(void)
    {
      FloatDataPoint* dp_f  = dynamic_cast<FloatDataPoint*>(mNumber);

      if (IsCaptured())
      {
        ReleaseDataPoint(); // error - memory leak !!!
      }

      if (mNumber->IsFloat())
      {
        mCapturedNumber = new FloatDataPoint();

        ((FloatDataPoint*)(mCapturedNumber))->SetQuality(dp_f->GetQuality());
        ((FloatDataPoint*)(mCapturedNumber))->SetQuantity(dp_f->GetQuantity());
        ((FloatDataPoint*)(mCapturedNumber))->SetWritable(dp_f->GetWritable());
        ((FloatDataPoint*)(mCapturedNumber))->SetMaxValue(dp_f->GetMaxValue());
        ((FloatDataPoint*)(mCapturedNumber))->SetMinValue(dp_f->GetMinValue());
        ((FloatDataPoint*)(mCapturedNumber))->SetValue(dp_f->GetValue());
      }
      else
      {

        U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(mNumber);
        if (dp_u32 != NULL)
        {
          mCapturedNumber = new U32DataPoint();
          ((U32DataPoint*)(mCapturedNumber))->SetQuality(dp_u32->GetQuality());
          ((U32DataPoint*)(mCapturedNumber))->SetQuantity(dp_u32->GetQuantity());
          ((U32DataPoint*)(mCapturedNumber))->SetWritable(dp_u32->GetWritable());
          ((U32DataPoint*)(mCapturedNumber))->SetMaxValue(dp_u32->GetMaxValue());
          ((U32DataPoint*)(mCapturedNumber))->SetMinValue(dp_u32->GetMinValue());
          ((U32DataPoint*)(mCapturedNumber))->SetValue(dp_u32->GetValue());
        }
        else
        {
          mCapturedNumber = new I32DataPoint();
          mCapturedNumber->SetQuality(mNumber->GetQuality());
          mCapturedNumber->SetQuantity(mNumber->GetQuantity());
          mCapturedNumber->SetWritable(mNumber->GetWritable());
          mCapturedNumber->SetMaxAsInt(mNumber->GetMaxAsInt());
          mCapturedNumber->SetMinAsInt(mNumber->GetMinAsInt());
          mCapturedNumber->SetAsInt(mNumber->GetAsInt());
        }

      }

      return dynamic_cast<Subject*>(mCapturedNumber);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool NumericalDataPointInterface::IsCaptured(void)
    {
      return (mCapturedNumber != NULL) ? true : false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void NumericalDataPointInterface::ReleaseDataPoint(void)
    {
      if (mCapturedNumber)
      {
        delete mCapturedNumber;
        mCapturedNumber = NULL;
      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * returns true if increments/decrements with specified changeValue is allowed
    *****************************************************************************/
    bool NumericalDataPointInterface::IsReadyForAcceleration(int changeValue, int numberOfDigits)
    {
      bool ready_for_acceleration = false;

      FloatDataPoint* dp_f    = dynamic_cast<FloatDataPoint*>(mCapturedNumber);
      IIntegerDataPoint* dp_i = dynamic_cast<IIntegerDataPoint*>(mCapturedNumber);

      if ( dp_i && mCapturedNumber->GetQuantity() == Q_TIME_SUM)
      {
        FatalErrorOccured("Missuse of NumericalDataPointInterface.");
        return false;

      }
      else if ( dp_f )
      {
        float f_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(dp_f->GetAsFloat(), dp_f->GetQuantity());

        if (f_value == 0.0)
        {
          ready_for_acceleration = false;
        }
        else
        {
          float remaining = GetRemaining( dp_f, numberOfDigits, changeValue );

          ready_for_acceleration = (remaining == 0.0);
        }

      }
      else if ( dp_i )
      {
        U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(dp_i);
        if (dp_u32)
        {
          if (dp_u32->GetValue() == 0)
            ready_for_acceleration = false;
          else
            ready_for_acceleration =  ((dp_u32->GetValue() % abs(changeValue)) == 0);
        }
        else
        {
          if (dp_i->GetAsInt() == 0)
            ready_for_acceleration = false;
          else
            ready_for_acceleration =  ((dp_i->GetAsInt() % changeValue) == 0);
        }
      }

      mNumberOfAccelerationRequests++;

      if (ready_for_acceleration && mNumberOfAccelerationRequests > NO_OF_REQUIRED_ACCELERATION_REQUESTS)
      {
        mNumberOfAccelerationRequests = 0;
      }
      else
      {
        ready_for_acceleration = false;
      }

      return ready_for_acceleration;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * clears acceleration request counter
    *****************************************************************************/
    void NumericalDataPointInterface::ResetAccelerationCounter()
    {
      mNumberOfAccelerationRequests = 0;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * returns true if acceleration is done by decade
    *****************************************************************************/
    bool NumericalDataPointInterface::ChangeAccelerationOnDecades()
    {
      return (mCapturedNumber && mCapturedNumber->GetQuantity() != Q_TIME_SUM);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * return current "timesum change value" in seconds (always a positive amount),
    * based on previous value and acceleration conditions
    *****************************************************************************/
    int NumericalDataPointInterface::GetCurrentTimesumChange(int previousTimesumChange, int numberOfDigits)
    {
      int currentValue = mCapturedNumber->GetAsInt();

      bool is_minutes_visible = (numberOfDigits > 2 
        && ((currentValue / ONE_HOUR_IN_SEC) < pow((double)10, (numberOfDigits - 2))));

      int increasedTimesumChange = previousTimesumChange;

      switch(previousTimesumChange)
      {
      case 0: //initial situation
        if (is_minutes_visible)
          return ONE_MINUTE_IN_SEC;
        else
          return ONE_HOUR_IN_SEC;

      case ONE_MINUTE_IN_SEC:
        if (is_minutes_visible)
        {
          if(currentValue % TEN_MINUTES_IN_SEC == 0)
            increasedTimesumChange = TEN_MINUTES_IN_SEC;
        }
        else
        {
          increasedTimesumChange = ONE_HOUR_IN_SEC;
        }
        break;

      case TEN_MINUTES_IN_SEC:
        if(currentValue % ONE_HOUR_IN_SEC == 0)
          increasedTimesumChange = ONE_HOUR_IN_SEC;
        break;

      default:
        if(currentValue % (previousTimesumChange*10) == 0)
          increasedTimesumChange = (previousTimesumChange*10);

      }

      // set max timesum change to 1 hour if DP max is below 24 hours
      if (increasedTimesumChange > ONE_HOUR_IN_SEC && mCapturedNumber->GetMaxAsInt() <= TWENTYFOUR_HOURS_IN_SEC)
        increasedTimesumChange = ONE_HOUR_IN_SEC;

      mNumberOfAccelerationRequests++;

      if (increasedTimesumChange != previousTimesumChange && mNumberOfAccelerationRequests > NO_OF_REQUIRED_ACCELERATION_REQUESTS)
      {
        mNumberOfAccelerationRequests = 0;
        return increasedTimesumChange;
      }
      else
      {
        return previousTimesumChange;
      }

    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *              -? .. -1, 0 , 1, ?
    *****************************************************************************/
    void NumericalDataPointInterface::ChangeCapturedDataPoint(int changeValue, int numberOfDigits)
    {
      char tmp_str[81];
      float f_value;
      int digitsBelowDecimalPoint;
      float f_max_value;
      float f_max_value_act;
      float tmp_float;
      QUANTITY_TYPE the_quantity_of_the_value;
      FloatDataPoint* dp_f = dynamic_cast<FloatDataPoint*>(mCapturedNumber);
      IIntegerDataPoint* dp_i = dynamic_cast<IIntegerDataPoint*>(mCapturedNumber);

      if( dp_i && mCapturedNumber->GetQuantity() == Q_TIME_SUM)
      {
        long currentValue;
        U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(dp_i);
        if (dp_u32)
        {
          currentValue = dp_u32->GetValue();
        }
        else
        {
          currentValue = dp_i->GetAsInt();
        }

        if (changeValue != 0 && (currentValue % changeValue != 0))
        {
          if(changeValue > 0)
            changeValue = changeValue - (currentValue % changeValue);
          else
            changeValue = - (currentValue % changeValue);
        }

        if (dp_u32)
        {
          dp_u32->SetValue(currentValue + changeValue);
        }
        else
        {
          dp_i->SetAsInt(currentValue + changeValue);
        }

      }
      else if ( dp_f )
      {
        f_value = dp_f->GetValue();
        f_max_value = dp_f->GetMaxValue();

        the_quantity_of_the_value = dp_f->GetQuantity();

        f_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_value,the_quantity_of_the_value);
        f_max_value_act = ::MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_max_value, the_quantity_of_the_value);

        FormatStr(tmp_str,f_value,f_max_value_act,numberOfDigits);

        tmp_float = (float)(atof(tmp_str)); // cast since VS atof returns a double

        digitsBelowDecimalPoint = numberOfDigits - Log10(f_max_value_act) - 1;

        if (digitsBelowDecimalPoint < 0)
          digitsBelowDecimalPoint = 0;

        float f_changeValue = ((float)changeValue) / Pow10(digitsBelowDecimalPoint);

        float remaining = GetRemaining( dp_f, numberOfDigits, changeValue );

        if ( remaining != 0 )
        {
          if( tmp_float > 0 )
          {
            f_changeValue = f_changeValue > 0 ? f_changeValue - remaining : -remaining;
          }
          else
          {
            f_changeValue = f_changeValue > 0 ? remaining : f_changeValue + remaining;
          }          
        }

        f_value = tmp_float + (float)(f_changeValue);

        f_value = ::MpcUnits::GetInstance()->GetFromActualUnitToStandard(f_value, the_quantity_of_the_value);

        dp_f->SetValue(f_value);
      }
      else if ( dp_i )
      {

        U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(dp_i);
        if (dp_u32)
        {
          U32 currentValue = dp_u32->GetValue();

          U32 remaining = currentValue % abs(changeValue);

          if (remaining != 0)
          {
            if(abs(changeValue) == 1)
            {
              changeValue = changeValue > 0 ? 1 : -1;
            }
            else
            {
              if( currentValue > 0 )
              {
                changeValue = changeValue > 0 ? changeValue - remaining : -remaining;
              }
              else
              {
                changeValue = changeValue > 0 ? remaining : changeValue + remaining;
              }
            }
          }

          if (changeValue > 0)
          {
            if ((dp_u32->GetMaxValue() - currentValue) < changeValue)
            {
              dp_u32->SetValue(dp_u32->GetMaxValue());
            }
            else
            {
              dp_u32->SetValue(currentValue + changeValue);              
            }
          }
          else
          {
            if ((currentValue - dp_u32->GetMinValue()) < abs(changeValue))
            {
              dp_u32->SetValue(dp_u32->GetMinValue());
            }
            else
            {
              dp_u32->SetValue(currentValue + changeValue);              
            }
          }

        }
        else
        {
          I32 currentValue = dp_i->GetAsInt();

          I32 remaining = currentValue % changeValue;

          if (remaining != 0)
          {
            if(abs(changeValue) == 1)
            {
              changeValue = changeValue > 0 ? 1 : -1;
            }
            else
            {
              if( currentValue > 0 )
              {
                changeValue = changeValue > 0 ? changeValue - remaining : -remaining;
              }
              else
              {
                changeValue = changeValue > 0 ? remaining : changeValue + remaining;
              }
            }
          }

          dp_i->SetAsInt(currentValue + changeValue);
        }


      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool NumericalDataPointInterface::SetCapturedDataPoint(void)
    {
      if (mNumber && mCapturedNumber)
      {
        if (mNumber->IsFloat())
          return mNumber->SetAsFloat(mCapturedNumber->GetAsFloat());
        else
          return mNumber->SetAsInt(mCapturedNumber->GetAsInt());
      }

      return false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool NumericalDataPointInterface::IsDataPointMax(void)
    {
      if (mNumber)
      {
        return mNumber->IsAtMax();
      }
      else
      {
        return false;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool NumericalDataPointInterface::IsDataPointMin(void)
    {
      if (mNumber)
      {
        return mNumber->IsAtMin();
      }
      else
      {
        return false;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool NumericalDataPointInterface::IsCapturedDataPointMax(void)
    {
      if (mCapturedNumber)
      {
        return mCapturedNumber->IsAtMax();
      }
      else
      {
        return false;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool NumericalDataPointInterface::IsCapturedDataPointMin(void)
    {
      if (mCapturedNumber)
      {
        return mCapturedNumber->IsAtMin();
      }
      else
      {
        return false;
      }
    }


    /* --------------------------------------------------
    * If the subject is a DataPoint and the quality is
    * DP_NEVER_AVAILABLE this function shall return true
    * --------------------------------------------------*/
    bool NumericalDataPointInterface::IsNeverAvailable()
    {
      if (mNumber)
      {
        return mNumber->GetQuality() == DP_NEVER_AVAILABLE;
      }
      else
      {
        return true;
      }
    }


    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    ****************************************************************************/

    void NumericalDataPointInterface::GetAsString(char* string, int numberOfDigits, INumberDataPoint* number)
    {
      if (number->IsFloat())
      {
        FloatDataPoint* dp_f  = dynamic_cast<FloatDataPoint*>(number);

        if (dp_f->GetQuality() == DP_AVAILABLE)
        {
          float f_value;
          float f_max_value;
          QUANTITY_TYPE the_quantity_of_the_value;

          f_value = dp_f->GetValue();
          f_max_value = dp_f->GetMaxValue();
          the_quantity_of_the_value = dp_f->GetQuantity();

          f_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_value, the_quantity_of_the_value);
          f_max_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_max_value, the_quantity_of_the_value);

          FormatStr(string,f_value,f_max_value, numberOfDigits);
        }
        else
        {
          strcpy(string, STR_DP_NOT_AVAILABLE);
        }

      }
      else
      {
        if (number->GetQuality() == DP_AVAILABLE)
        {
          int temp_i;
          switch ( number->GetQuantity() )
          {
          case Q_CLOCK_MINUTE :
          case Q_CLOCK_DAY    :
          case Q_CLOCK_MONTH  :
          case Q_CLOCK_YEAR   :
            sprintf(string,"%0*d",numberOfDigits, number->GetAsInt());
            break;

          case Q_CLOCK_HOUR:
            if (TimeFormatDataPoint::GetInstance()->GetAsInt() == 2)  // US
            {
              int hours = number->GetAsInt();
              bool am = false;

              if ( hours == 0 )                         // 00:00 -> 00:59
              {
                hours = 12;
                am = true;
              }
              else if ( (hours > 0) && (hours <= 11) )  // 01:00 -> 11:59
              {
                am = true;
              }
              else if ( hours == 12 )                  // 12:00 -> 12:59
              {
                am = false;
              }
              else if ( hours >= 13 )                  // 13:00 -> 23:59
              {
                hours -= 12;
                am = false;
              }
              sprintf(string,"%d%s",hours, am ? "am" : "pm");
            }
            else
            {
              sprintf(string,"%0*d", numberOfDigits, number->GetAsInt());
            }
            break;

          case Q_MAC_ADDRESS:
            temp_i = number->GetAsInt();
            sprintf(string,"%02X %02X %02X", (temp_i & 0x00FF0000) >> 16, (temp_i & 0x0000FF00) >> 8, (temp_i & 0x000000FF));
            break;

          case Q_TIME:
          case Q_TIME_SUM:
            {
              FormatDateStr(number, string, numberOfDigits);
              break;
            }
          case Q_RATIO:
            {
              sprintf(string,"%s%d", "1:", number->GetAsInt());
              break;
            }
          case Q_DATETIME:
            {
              U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(number);

              if (dp_u32 != NULL)
              {
                MpcTime* mpc_time = new MpcTime(dp_u32->GetValue());
                TimeText* formatted_time = new TimeText();
                formatted_time->SetSubjectPointer(0, TimeFormatDataPoint::GetInstance());
                formatted_time->Update(TimeFormatDataPoint::GetInstance());
                formatted_time->SetTime(*mpc_time);
              
                sprintf(string,"%s", formatted_time->GetText());

                delete formatted_time;
                delete mpc_time;
              }
              //else
              //  FatalErrorOccured("using q_datetime with non-U32 type!");

              break;
            }

          default:
            U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(number);
            if (dp_u32 != NULL)
            {
              sprintf(string,"%lu", dp_u32->GetValue());
            }
            else
            {
              sprintf(string,"%d", number->GetAsInt());
            }
            break;
          }
        }
        else
        {
          strcpy(string, STR_DP_NOT_AVAILABLE);
        }
      }
    }

    float NumericalDataPointInterface::GetRemaining( FloatDataPoint* pfDp, int numberOfDigits, float changeValue )
    {
      char tmp_str[81];
      int digitsBelowDecimalPoint;
      float f_value;
      float f_max_value;
      float f_max_value_act;
      QUANTITY_TYPE the_quantity_of_the_value;

      f_value = pfDp->GetValue();
      f_max_value = pfDp->GetMaxValue();

      the_quantity_of_the_value = pfDp->GetQuantity();
      f_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_value,the_quantity_of_the_value);
      f_max_value_act = ::MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_max_value,the_quantity_of_the_value);

      FormatStr(tmp_str,f_value,f_max_value_act,numberOfDigits);

      digitsBelowDecimalPoint = numberOfDigits - Log10(f_max_value_act) - 1;

      if (digitsBelowDecimalPoint < 0)
        digitsBelowDecimalPoint = 0;

      int decadePosition = Log10((float)changeValue);

      int changing_digit = strlen(tmp_str) - decadePosition - 1;

      if ( digitsBelowDecimalPoint > 0)
      {// a decimal point is shown
        if ( decadePosition <= digitsBelowDecimalPoint  )
        {// is changing a digit below decimal point
          changing_digit++;
        }
      }
      else
      {
        changing_digit++;
      }

      if(changing_digit < 0)
        changing_digit = 0;
      else if(changing_digit > strlen(tmp_str))
        changing_digit = strlen(tmp_str);

      float remaining = (float)atof(&tmp_str[changing_digit]);

      remaining /= Pow10(digitsBelowDecimalPoint - decadePosition);

      return remaining;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * formats time_sum quantity as hh:mm 
    *****************************************************************************/
    void NumericalDataPointInterface::FormatDateStr(INumberDataPoint* number, char* string, int numberOfDigits)
    {
      float f_value;
      float f_max_value;
      QUANTITY_TYPE the_quantity_of_the_value;

      f_value = number->GetAsFloat();
      f_max_value = number->GetMaxAsFloat();
      the_quantity_of_the_value = number->GetQuantity();

      if (the_quantity_of_the_value == Q_TIME_SUM)
      {
        U32DataPoint* dp_u32 = dynamic_cast<U32DataPoint*>(number);

        U32 ui_value;
        if (dp_u32 != NULL)
        {
          ui_value = dp_u32->GetValue();
        }
        else
        {
          ui_value = (U32) number->GetAsInt();
        }

        U32 ui_hours = (int) ui_value / 3600;
        int i_minutes = (int) (ui_value / 60) - (60 * ui_hours);

        // show minutes part if the space is adequate
        if (numberOfDigits > 2 && ui_hours < pow((double)10, (numberOfDigits - 2)))
        {
          sprintf(string,"%0.1u:%0.2i", ui_hours, i_minutes);
        }
        else
        {
          if (i_minutes >= 30)
          {
            ui_hours++;
          }

          sprintf(string,"%0.1u", ui_hours);
        }
      }
      else
      {
        f_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_value, the_quantity_of_the_value);

        f_max_value = MpcUnits::GetInstance()->GetFromStandardToActualUnit(f_max_value, the_quantity_of_the_value);

        FormatStr(string,f_value,f_max_value, numberOfDigits);
      }
    }

    char* NumericalDataPointInterface::FormatStr(char* numberStr ,float number, float max, int numberOfDigits)
    {
      int decimals = numberOfDigits - Log10(max) - 1;
      if( decimals < 0 )
        decimals = 0;
      sprintf(numberStr,"%0.*f",decimals,number);

      return numberStr;
    }

    int NumericalDataPointInterface::Log10(float a)
    {
      return a == 0.0 ? 0 : (int)log10(abs(a));
    }


    float NumericalDataPointInterface::Pow10(int n)
    {
      float a_number = 1.0;

      if ( n > 0)
      {
        for (int i=0; i < n; ++i)
        {
          a_number *= 10.0;
        }
      }
      else if (n < 0)
      {
        for (int i=0; i > n; --i)
        {
          a_number *= 0.1f;
        }
      }
      return a_number;;
    }

    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/

  }

}

