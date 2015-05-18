

#include <math.h>
#include <UnitTypes.h>
#include <mpcunits.h>
#include <floatdatapointtostring.h>

namespace mpc
{
  namespace display
  {

    int Log10Int(float a)
    {
      return a == 0.0 ? 0 : (int)log10(abs(a));
    }

    float Pow10Int(int n)
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

    void FloatToString(char* string, float number, float max, int numberOfDigits, QUANTITY_TYPE quantityOfTheValue, bool directValue /* = true */)
    {
      if (!directValue)
      {
        number = MpcUnits::GetInstance()->GetFromStandardToActualUnit(number, quantityOfTheValue);
        max = MpcUnits::GetInstance()->GetFromStandardToActualUnit(max, quantityOfTheValue);
      }

      int decimals = numberOfDigits - Log10Int(max) - 1;
      if( decimals < 0 )
        decimals = 0;
      sprintf(string,"%0.*f",decimals,number);
    }

  }
}




