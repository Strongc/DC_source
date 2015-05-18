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
/* CLASS NAME       : State                                                  */
/*                                                                          */
/* FILE NAME        : State.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a State.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include "StringWidthCalculator.h"
extern bool g_show_string_ids;
#endif

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "State.h"
#include <DisplayController.h>
#include <Display.h>
#include "DataPoint.h"
#include "Languages.h"

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
  * Function
  * DESCRIPTION:
  * Constructor
  *****************************************************************************/
  State::State(Component* pParent) : ObserverText(pParent)
  {
    mpStateStringIds = NULL;
    mStringIdCount = 0;
    SetAlign(GUI_TA_LEFT|GUI_TA_HCENTER);
    this->SetWordWrap(false);
    Languages::GetInstance()->Subscribe(this);
  }

  /*****************************************************************************
  * Function
  * DESCRIPTION:
  * Dectructor
  *****************************************************************************/
  State::~State()
  {
    Languages::GetInstance()->Unsubscribe(this);
  }

  /*****************************************************************************
  * Function - Redraw
  * DESCRIPTION:
  *****************************************************************************/
  bool State::Redraw()
  {
    IIntegerDataPoint* pDP  = dynamic_cast<IIntegerDataPoint*>(GetSubject());  
    
    if (pDP != NULL)
    {
      if (pDP->GetQuality() != DP_AVAILABLE)
      {
        SetText("--");
      }
      else
      {
        int state = pDP->GetAsInt();
        
#ifdef __PC__
        if (g_show_string_ids)
        {
          char string_id_as_string[10];
          U16 string_id = GetStateStringId(state);
          sprintf(string_id_as_string, "%d", string_id);
          SetText(string_id_as_string);
          ObserverText::Redraw();
          return true;
        }
#endif
        
        const char* the_string_to_show = GetStateAsString(state);
        SetText(the_string_to_show);
      }
    }
    else 
    {
      SetText("--");
    }
    
    ObserverText::Redraw();
    
    return true;
  }

  /*****************************************************************************
  * Function - GetStateStringId
  * DESCRIPTION:
  *****************************************************************************/
  U16 State::GetStateStringId(int state)
  {
    if (mpStateStringIds != NULL)
    {
      for (int i = 0; i < mStringIdCount; i++)
      {
        if (mpStateStringIds[i].state == state)
        {
          return mpStateStringIds[i].stringId;
        }
      }
    }
    
    return SID_NONE;
  }

  /*****************************************************************************
  * Function - GetStateAsString
  * DESCRIPTION:
  *****************************************************************************/
  const char* State::GetStateAsString(int state)
  {
    IIntegerDataPoint* pDP  = dynamic_cast<IIntegerDataPoint*>(GetSubject());  
    
    if (pDP != NULL)
    {
      U16 string_id = GetStateStringId(pDP->GetAsInt());
      return Languages::GetInstance()->GetString(string_id);
    }

    return "";
  }


  /*****************************************************************************
  * Function - IsNeverAvailable
  * DESCRIPTION:
  *****************************************************************************/
  bool State::IsNeverAvailable()
  {
    IDataPoint* dp  = dynamic_cast<IDataPoint*>(GetSubject());
    if (dp == NULL)
    {
      return false;
    }

    return dp->GetQuality() == DP_NEVER_AVAILABLE;
  }


#ifdef __PC__
  /*****************************************************************************
  * Function - CalculateStringWidths
  * DESCRIPTION:
  *****************************************************************************/
  void State::CalculateStringWidths(bool forceVisible)
  {
    Component::CalculateStringWidths(forceVisible);
    GUI_SetFont(GetFont());
    int component_width = GetWidth() - mLeftMargin - mRightMargin;
    int component_height = GetHeight();
    int string_width_pixels = 0;
    int string_height_pixels = 0;

    int state_id = -1;
    IIntegerDataPoint* pDP  = dynamic_cast<IIntegerDataPoint*>(GetSubject());  
    if (pDP != NULL)
    {
      state_id = pDP->GetAsInt();
    }

    for (int i = 0; i < mStringIdCount; i++)
    {
      string_width_pixels = 0;
      string_height_pixels = 0;

      bool fits = false;

      const char* p_text = Languages::GetInstance()->GetString(mpStateStringIds[i].stringId);
      if (strlen(p_text)>0 && p_text[0] != '{')
      {
        GUI_RECT rect;

        if (mWordWrap)
        {
          SetText(p_text);
          int line_count = WrapText(component_width);
          fits = (line_count <= (component_height/GetFont()->YSize));

          GUI_GetTextExtend(&rect,mTextWrapped,strlen(mTextWrapped));
          string_width_pixels = abs(rect.x1 - rect.x0) + 1;
          string_height_pixels = (abs(rect.y1 - rect.y0) + 1) * line_count;
        }
        else
        {
          GUI_GetTextExtend(&rect,p_text,strlen(p_text));
          string_width_pixels = abs(rect.x1 - rect.x0) + 1;
          string_height_pixels = abs(rect.y1 - rect.y0) + 1;

          fits = (string_width_pixels <= component_width);
        }
      }


      CSV_ENTRY entry;
      entry.componentId = mComponentId;
      entry.stringId = mpStateStringIds[i].stringId;
      entry.componentWidth = component_width;
      entry.stringWidth = string_width_pixels;
      entry.componentHeight = component_height;
      entry.stringHeight = string_height_pixels;
      entry.wordwrap = mWordWrap;
      entry.fits = fits;
      entry.visible = (mpStateStringIds[i].state == state_id);
      entry.forcedVisible = forceVisible;

      StringWidthCalculator::GetInstance()->WriteToCSV(entry);
    }
  }
#endif // __PC__

  } // namespace display
} // namespace mpc


