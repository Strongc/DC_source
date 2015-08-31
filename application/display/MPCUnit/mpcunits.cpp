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
/* CLASS NAME       : MpcUnits                                              */
/*                                                                          */
/* FILE NAME        : MpcUnits.cpp                                          */
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
#include <MpcUnits.h>
#include <mpcunits.conf.h>
#include <FactoryTypes.h>
#include <DataPoint.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

 /*****************************************************************************
  CREATS AN OBJECT.
 ******************************************************************************/
MpcUnits* MpcUnits::mInstance = 0;


QUANTITY_TYPE& operator++(QUANTITY_TYPE& quantity, int)  // postfix++
{
    int i = quantity;
    return quantity = static_cast<QUANTITY_TYPE> (++i);
}


 /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
  *****************************************************************************/
MpcUnits* MpcUnits::GetInstance()
{
  if (!mInstance)
  {
    mInstance = new MpcUnits();
  }
  return mInstance;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
float MpcUnits::GetFromStandardToActualUnit(float value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return (value * MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].ActualUnit].ScaleFactor)
      + MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].ActualUnit].Offset;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
double MpcUnits::GetFromStandardToActualUnit(double value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return (value * (double)MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].ActualUnit].ScaleFactor)
      + (double)MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].ActualUnit].Offset;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
float MpcUnits::GetFromActualUnitToStandard(float value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return  (value + MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].ActualUnit].Offset) *
     MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].ActualUnit].ScaleFactor;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
double MpcUnits::GetFromActualUnitToStandard(double value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return  (value + (double)MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].ActualUnit].Offset) *
     (double)MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].ActualUnit].ScaleFactor;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
float MpcUnits::GetFromStandardToDefaultSiUnit(float value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return (value * MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].DefaultSiUnit].ScaleFactor) +
     MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].DefaultSiUnit].Offset;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
float MpcUnits::GetFromDefaultSiUnitToStandard(float value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return  (value + MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].DefaultSiUnit].Offset) *
     MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].DefaultSiUnit].ScaleFactor;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
float MpcUnits::GetFromStandardToDefaultUsUnit(float value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return (value * MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].DefaultUsUnit].ScaleFactor) +
     MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].DefaultUsUnit].Offset;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
float MpcUnits::GetFromDefaultUsUnitToStandard(float value, QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  else
  {
    return  (value + MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].DefaultUsUnit].Offset) *
     MpcUnitTabel[quantity].FromUnitToStandard[MpcUnitTabel[quantity].DefaultUsUnit].ScaleFactor;
  }
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
STRING_ID MpcUnits::GetActualUnitString(QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    quantity = Q_NO_UNIT;
  }
  return MpcUnitTabel[quantity].FromStandardToUnit[MpcUnitTabel[quantity].ActualUnit].UnitTextId;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
STRING_ID  MpcUnits::GetStandardUnitString(QUANTITY_TYPE quantity)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    quantity = Q_NO_UNIT;
  }
  return MpcUnitTabel[quantity].FromStandardToUnit[0].UnitTextId;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void MpcUnits::SetDefaultSiUnit()
{
  QUANTITY_TYPE i;

  for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i = QUANTITY_TYPE((int)i + 1))
  {
    MpcUnitTabel[i].ActualUnit = MpcUnitTabel[i].DefaultSiUnit;
    if ( mUnitsDataPoints[i].IsValid() )
    {
      mUnitsDataPoints[i]->SetValue(MpcUnitTabel[i].ActualUnit);
    }
  }
  mDefaultSI->SetValue(GetDefaultSiUnit());
  mDefaultUS->SetValue(GetDefaultUsUnit());
  NotifyObservers();
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void MpcUnits::SetDefaultUsUnit()
{
  QUANTITY_TYPE i;

  for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i++)
  {
    MpcUnitTabel[i].ActualUnit = MpcUnitTabel[i].DefaultUsUnit;
    if ( mUnitsDataPoints[i].IsValid() )
    {
      mUnitsDataPoints[i]->SetValue(MpcUnitTabel[i].ActualUnit);
    }
  }
  mDefaultSI->SetValue(GetDefaultSiUnit());
  mDefaultUS->SetValue(GetDefaultUsUnit());
  NotifyObservers();
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool MpcUnits::GetDefaultSiUnit()
{
  QUANTITY_TYPE i;
  bool a_b;

  a_b = true;
  for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i++)
  {
    if ( MpcUnitTabel[i].ActualUnit != MpcUnitTabel[i].DefaultSiUnit )
    {
      a_b = false;
      break;
    }
  }

  return a_b;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool MpcUnits::GetDefaultUsUnit()
{
  QUANTITY_TYPE i;
  bool a_b;

  a_b = true;
  for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i++)
  {
    if ( MpcUnitTabel[i].ActualUnit != MpcUnitTabel[i].DefaultUsUnit )
    {
      a_b = false;
      break;
    }
  }

  return a_b;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void MpcUnits::SetTempDefaultSiUnit()
{
  mTempDefaultSiUnitMode = true;
  NotifyObservers();
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
void MpcUnits::ReleaseTempDefaultSiUnit()
{
  mTempDefaultSiUnitMode = false;
  NotifyObservers();
}


/*****************************************************************************
* Function
* DESCRIPTION:
* Returns the value of 'value' converted from the standard SI unit to a unit
* given by 'no'
*****************************************************************************/
float MpcUnits::GetFromStandardToUnitNo(float value, QUANTITY_TYPE quantity, int no)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  if ((MpcUnitTabel[quantity].LastUnit < no) || (no < 0))
  {
    return value;
  }
  else
  {
    return (value * MpcUnitTabel[quantity].FromStandardToUnit[no].ScaleFactor)
      + MpcUnitTabel[quantity].FromStandardToUnit[no].Offset;
  }
}

/*****************************************************************************
* Function
* DESCRIPTION:
* Returns the value of 'value' converted from the unit given by 'no' to the
* standard SI unit
*****************************************************************************/
float MpcUnits::GetFromUnitNoToStandard(float value, QUANTITY_TYPE quantity, int no)
{
  if ((quantity >= Q_LAST_UNIT) || (quantity <= Q_NO_UNIT))
  {
    return value;
  }
  if ((MpcUnitTabel[quantity].LastUnit < no) || (no < 0))
  {
    return value;
  }
  else
  {
    return  (value + MpcUnitTabel[quantity].FromUnitToStandard[no].Offset) *
     MpcUnitTabel[quantity].FromUnitToStandard[no].ScaleFactor;
  }
}


  void MpcUnits::SubscribtionCancelled(Subject* pSubject)
  {
    QUANTITY_TYPE i;

    for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i = QUANTITY_TYPE((int)i + 1))
    {
      if (mUnitsDataPoints[i].Detach(pSubject))
      {
        return;
      }
    }

    mDefaultSI.Detach(pSubject);
    mDefaultUS.Detach(pSubject);
  }

  void MpcUnits::Update(Subject* pSubject)
  {
    int a_new_unit_index;
    QUANTITY_TYPE i;
		
		switch (pSubject->GetSubjectId())
		{
		case SUBJECT_ID_UNIT_DEFAULT_SI_ACTUAL:
			if (mDefaultSI->GetValue())
			{
				SetDefaultSiUnit();
      }
			break;
			
		case SUBJECT_ID_UNIT_DEFAULT_US_ACTUAL:
			if (mDefaultUS->GetValue())
			{
				SetDefaultUsUnit();
			}
			break;

		default:			
      for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i = QUANTITY_TYPE((int)i + 1))
      {
        if (mUnitsDataPoints[i].Update(pSubject))
        {
          a_new_unit_index = mUnitsDataPoints[i]->GetValue();
          if ( a_new_unit_index <= MpcUnitTabel[i].LastUnit )
          {
            MpcUnitTabel[i].ActualUnit = a_new_unit_index;
            NotifyObservers();
          }
          break;
        }
      }
      mDefaultSI->SetValue(GetDefaultSiUnit());
      mDefaultUS->SetValue(GetDefaultUsUnit());
			break;	
		}
  }

  void MpcUnits::SetSubjectPointer(int Id,Subject* pSubject)
  {
    switch ( Id )
    {
      case SP_UNITS_Q_NO_UNIT                  : mUnitsDataPoints[Q_NO_UNIT].Attach(pSubject); break;
      case SP_UNITS_Q_FLOW                     : mUnitsDataPoints[Q_FLOW].Attach(pSubject); break;
      case SP_UNITS_Q_ENERGY                   : mUnitsDataPoints[Q_ENERGY].Attach(pSubject); break;
      case SP_UNITS_Q_TEMPERATURE              : mUnitsDataPoints[Q_TEMPERATURE].Attach(pSubject); break;
      case SP_UNITS_Q_DIFFERENCIAL_TEMPERATURE : mUnitsDataPoints[Q_DIFFERENCIAL_TEMPERATURE].Attach(pSubject); break;
      case SP_UNITS_Q_HEIGHT                   : mUnitsDataPoints[Q_HEIGHT].Attach(pSubject); break;
      case SP_UNITS_Q_DEPTH                    : mUnitsDataPoints[Q_DEPTH].Attach(pSubject); break;
      case SP_UNITS_Q_HEAD                     : mUnitsDataPoints[Q_HEAD].Attach(pSubject); break;
      case SP_UNITS_Q_PRESSURE                 : mUnitsDataPoints[Q_PRESSURE].Attach(pSubject); break;
      case SP_UNITS_Q_DIFFERENCIAL_PRESSURE    : mUnitsDataPoints[Q_DIFFERENCIAL_PRESSURE].Attach(pSubject); break;
      case SP_UNITS_Q_VOLTAGE                  : mUnitsDataPoints[Q_VOLTAGE].Attach(pSubject); break;
      case SP_UNITS_Q_LOW_CURRENT              : mUnitsDataPoints[Q_LOW_CURRENT].Attach(pSubject); break;
      case SP_UNITS_Q_SPECIFIC_ENERGY          : mUnitsDataPoints[Q_SPECIFIC_ENERGY].Attach(pSubject); break;
      case SP_UNITS_Q_POWER                    : mUnitsDataPoints[Q_POWER].Attach(pSubject); break;
      case SP_UNITS_Q_COS_PHI                  : mUnitsDataPoints[Q_COS_PHI].Attach(pSubject); break;
      case SP_UNITS_Q_VOLUME                   : mUnitsDataPoints[Q_VOLUME].Attach(pSubject); break;
      case SP_UNITS_Q_PERCENT                  : mUnitsDataPoints[Q_PERCENT].Attach(pSubject); break;
      case SP_UNITS_Q_TIME                     : mUnitsDataPoints[Q_TIME].Attach(pSubject); break;
      case SP_UNITS_Q_PH_VALUE                 : mUnitsDataPoints[Q_PH_VALUE].Attach(pSubject); break;
      case SP_UNITS_Q_FREQUENCY                : mUnitsDataPoints[Q_FREQUENCY].Attach(pSubject); break;
      case SP_UNITS_Q_REVOLUTION               : mUnitsDataPoints[Q_REVOLUTION].Attach(pSubject); break;
      case SP_UNITS_Q_PERFORMANCE              : mUnitsDataPoints[Q_PERFORMANCE].Attach(pSubject); break;
      case SP_UNITS_Q_RESISTANCE               : mUnitsDataPoints[Q_RESISTANCE].Attach(pSubject); break;
      case SP_UNITS_Q_AREA                     : mUnitsDataPoints[Q_AREA].Attach(pSubject); break;
      case SP_UNITS_Q_CONDUCTIVITY             : mUnitsDataPoints[Q_CONDUCTIVITY].Attach(pSubject); break;
      case SP_UNITS_Q_FORCE                    : mUnitsDataPoints[Q_FORCE].Attach(pSubject); break;
      case SP_UNITS_Q_TORQUE                   : mUnitsDataPoints[Q_TORQUE].Attach(pSubject); break;
      case SP_UNITS_Q_VELOCITY                 : mUnitsDataPoints[Q_VELOCITY].Attach(pSubject); break;
      case SP_UNITS_Q_MASS                     : mUnitsDataPoints[Q_MASS].Attach(pSubject); break;
      case SP_UNITS_Q_ACCELERATION             : mUnitsDataPoints[Q_ACCELERATION].Attach(pSubject); break;
      case SP_UNITS_Q_MASS_FLOW                : mUnitsDataPoints[Q_MASS_FLOW].Attach(pSubject); break;
      case SP_UNITS_Q_ANGULAR_VELOCITY         : mUnitsDataPoints[Q_ANGULAR_VELOCITY].Attach(pSubject); break;
      case SP_UNITS_Q_ANGULAR_ACCELERATION     : mUnitsDataPoints[Q_ANGULAR_ACCELERATION].Attach(pSubject); break;
      case SP_UNITS_Q_LUMINOUS_INTENSITY       : mUnitsDataPoints[Q_LUMINOUS_INTENSITY].Attach(pSubject); break;
      case SP_UNITS_Q_CLOCK_HOUR               : mUnitsDataPoints[Q_CLOCK_HOUR].Attach(pSubject); break;
      case SP_UNITS_Q_CLOCK_MINUTE             : mUnitsDataPoints[Q_CLOCK_MINUTE].Attach(pSubject); break;
      case SP_UNITS_Q_CLOCK_DAY                : mUnitsDataPoints[Q_CLOCK_DAY].Attach(pSubject); break;
      case SP_UNITS_Q_CLOCK_MONTH              : mUnitsDataPoints[Q_CLOCK_MONTH].Attach(pSubject); break;
      case SP_UNITS_Q_CLOCK_YEAR               : mUnitsDataPoints[Q_CLOCK_YEAR].Attach(pSubject); break;
      case SP_UNITS_Q_TIME_SUM                 : mUnitsDataPoints[Q_TIME_SUM].Attach(pSubject); break;
      case SP_UNITS_Q_RATIO                    : mUnitsDataPoints[Q_RATIO].Attach(pSubject); break;
      case SP_UNITS_Q_MAC_ADDRESS              : mUnitsDataPoints[Q_MAC_ADDRESS].Attach(pSubject); break;
      case SP_UNITS_Q_DATA_SIZE                : mUnitsDataPoints[Q_DATA_SIZE].Attach(pSubject); break;
      case SP_UNITS_Q_DATETIME                 : mUnitsDataPoints[Q_DATETIME].Attach(pSubject); break;
      case SP_UNITS_Q_HIGH_CURRENT             : mUnitsDataPoints[Q_HIGH_CURRENT].Attach(pSubject); break;
      case SP_UNITS_Q_SMALL_AREA               : mUnitsDataPoints[Q_SMALL_AREA].Attach(pSubject); break;
      case SP_UNITS_Q_HIGH_VELOCITY            : mUnitsDataPoints[Q_HIGH_VELOCITY].Attach(pSubject); break;
      case SP_UNITS_Q_PARTS_PER_MILLION        : mUnitsDataPoints[Q_PARTS_PER_MILLION].Attach(pSubject); break;
      case SP_UNITS_Q_SMALL_FLOW               : mUnitsDataPoints[Q_SMALL_FLOW].Attach(pSubject); break;
      case SP_UNITS_DEFAULT_SI_SYSTEM          : mDefaultSI.Attach(pSubject); break;
      case SP_UNITS_DEFAULT_US_SYSTEM          : mDefaultUS.Attach(pSubject); break;
    }
  }

  void MpcUnits::ConnectToSubjects(void)
  {
	  QUANTITY_TYPE i;

    for (i = Q_NO_UNIT; i < Q_LAST_UNIT; i = QUANTITY_TYPE((int)i + 1))
    {
      if (mUnitsDataPoints[i].IsValid())
      {
        mUnitsDataPoints[i]->SetMinValue(0);
        mUnitsDataPoints[i]->SetMaxValue(MpcUnitTabel[i].LastUnit);
        MpcUnitTabel[i].ActualUnit = mUnitsDataPoints[i]->GetValue();
        mUnitsDataPoints[i]->Subscribe(this);
      }
    }

    mDefaultSI->SetValue(GetDefaultSiUnit());
    mDefaultUS->SetValue(GetDefaultUsUnit());

    mDefaultSI->Subscribe(this);
    mDefaultUS->Subscribe(this);
  }


 /*****************************************************************************
  *
  *
  *              PRIVATE FUNCTIONS
  *
  *
  ****************************************************************************/
MpcUnits::MpcUnits()
{
  mTempDefaultSiUnitMode = false;
}

MpcUnits::~MpcUnits()
{
}

 /*****************************************************************************
  *
  *
  *              PROTECTED FUNCTIONS
  *                 - RARE USED -
  *
  ****************************************************************************/


