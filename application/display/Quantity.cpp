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
/* CLASS NAME       : Quantity                                              */
/*                                                                          */
/* FILE NAME        : Quantity.cpp                                          */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include "WM.h"
#ifdef __PC__
#include "StringWidthCalculator.h"
#endif
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Quantity.h"
#include "Languages.h"
#include <DataPoint.h>
#include <DisplayController.h>

/*****************************************************************************
DEFINES
*****************************************************************************/

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
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    Quantity::Quantity(Component* pParent) : ObserverText( pParent )
    {
      mQuantityType = Q_NO_UNIT;
      mUpdate = true;
      MpcUnits::GetInstance()->Subscribe(this);
      Languages::GetInstance()->Subscribe(this);
    }

    Quantity::~Quantity()
    {
      MpcUnits::GetInstance()->Unsubscribe(this);
    }

    void Quantity::SetQuantityType(QUANTITY_TYPE quantity)
    {
      if(mQuantityType != quantity)
      {
        mQuantityType= quantity;
      }
    }

    QUANTITY_TYPE Quantity::GetQuantityType()
    {
      return mQuantityType;
    }

    void Quantity::Run()
    {
      if(!IsVisible())
        return;
      if(mUpdate)
      {
	      IDataPoint* pDP = dynamic_cast<IDataPoint*>(GetSubject());
	      if (pDP != NULL)
	      {
	        SetQuantityType(pDP->GetQuantity());	      
	      }
	  
        mUpdate = false;
        int string_id = MpcUnits::GetInstance()->GetActualUnitString(mQuantityType);
        const char* the_string_to_show = Languages::GetInstance()->GetString(string_id);
        ObserverText::SetText(the_string_to_show);
      }
      ObserverText::Run();
    }

    void Quantity::Update(Subject* Object)
    {
      mUpdate = true;
    }

#ifdef __PC__
    void Quantity::CalculateStringWidths(bool forceVisible)
    {
      Component::CalculateStringWidths(forceVisible);
      int q_type = Q_NO_UNIT;
      GUI_SetFont(GetFont());
      int component_width = GetWidth() - mLeftMargin - mRightMargin;
      int string_width_pixels = 0;
      int string_height_pixels = 0;

      // makes sure mText is set
      int string_id = MpcUnits::GetInstance()->GetActualUnitString(mQuantityType);
      const char* the_string_to_show = Languages::GetInstance()->GetString(string_id);
      ObserverText::SetText(the_string_to_show);

      for(int i = 0; i < NO_OF_UNITS; i++)
      {
        string_width_pixels = 0;
        string_height_pixels = 0;

        STRING_ID string_id = MpcUnitTabel[mQuantityType].FromStandardToUnit[i].UnitTextId;

        if (string_id == 0)
          break;

        const char* p_text= Languages::GetInstance()->GetString(string_id);

        bool fits = false;

        if (strlen(p_text) > 0 && p_text[0] != '{')
        {
          GUI_RECT rect;
          GUI_GetTextExtend(&rect,p_text,strlen(p_text));
          string_width_pixels = abs(rect.x1 - rect.x0) + 1;
          string_height_pixels = abs(rect.y1 - rect.y0) + 1;

          fits = (string_width_pixels <= component_width);
        }
        
        CSV_ENTRY entry;
        entry.componentId = mComponentId;
        entry.stringId = string_id;
        entry.componentWidth = component_width;
        entry.stringWidth = string_width_pixels;
        entry.componentHeight = GetHeight();
        entry.stringHeight = string_height_pixels;
        entry.wordwrap = mWordWrap;
        entry.fits = fits;
        entry.visible = (strcmp(p_text, mText) == 0);
        entry.forcedVisible = forceVisible;

        StringWidthCalculator::GetInstance()->WriteToCSV(entry);

      }
    }
#endif

  } // namespace display
} // namespace mpc
