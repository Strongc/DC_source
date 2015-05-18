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
/* CLASS NAME       : IconState                                             */
/*                                                                          */
/* FILE NAME        : IconState.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a IconState.                   */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "IIntegerDataPoint.h"
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "IconState.h"

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
    * Constructor
    *****************************************************************************/
    IconState::IconState(Component* pParent) : ObserverText(pParent)
    {
      mpStateIconIds = NULL;
      mIconIdCount = 0;
      SetAlign(GUI_TA_LEFT|GUI_TA_HCENTER);

      mpBackgroundBitmap = NULL;
      mBackgroundOffsetX = 0;
      mBackgroundOffsetY = 0;
    }

    /*****************************************************************************
    * Dectructor
    *****************************************************************************/
    IconState::~IconState()
    {
    }
    
    /*****************************************************************************
    * Function...: SetBackgroundBitmap
    * DESCRIPTION:
    *****************************************************************************/
    void IconState::SetBackgroundBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap)
    {
      mpBackgroundBitmap = pBitmap;
    }

    /*****************************************************************************
    * Function...: SetBackgroundOffset
    * DESCRIPTION:
    *****************************************************************************/
    void IconState::SetBackgroundOffset(int x, int y)
    {
      mBackgroundOffsetX = x;
      mBackgroundOffsetY = y;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool IconState::Redraw()
    {
      ObserverText::Redraw();
      Component::Redraw();

      if (mpBackgroundBitmap != NULL)
      {
        GUI_DrawBitmap(mpBackgroundBitmap, mBackgroundOffsetX, mBackgroundOffsetY);
      }

      if (IsVisible())
      {
        IIntegerDataPoint* pDP  = dynamic_cast<IIntegerDataPoint*>(GetSubject());
        if (pDP != NULL)
        {
          GUI_CONST_STORAGE GUI_BITMAP* icon_ptr = GetIconStateId(pDP->GetAsInt());

          if (icon_ptr == NULL)
          {
            return false;
          }

          int width = GetWidth();
          int height = GetHeight();
          int x;
          int y;

          x = (width / 2.0) - (icon_ptr->XSize / 2.0);
          y = (height / 2.0) - (icon_ptr->YSize / 2.0);

          GUI_DrawBitmap(icon_ptr, x, y);

          Validate();
          return true;
        }
        return true;
      }
      Validate();
      return false;
    }

    /*****************************************************************************
    * Function...: IsNeverAvailable
    * DESCRIPTION:
    *****************************************************************************/
    bool IconState::IsNeverAvailable()
    {
      IDataPoint* pDP  = dynamic_cast<IDataPoint*>(GetSubject());
      
      if (pDP == NULL)
      {
        return false;
      }

      return pDP->GetQuality() == DP_NEVER_AVAILABLE;
    }



    /*****************************************************************************
    * Function...: GetIconStateId
    * DESCRIPTION:
    *****************************************************************************/
    GUI_CONST_STORAGE GUI_BITMAP*  IconState::GetIconStateId(int state)
    {
      if (mpStateIconIds == NULL)
      {
        return NULL;
      }

      for (int i = 0; i < mIconIdCount; i++)
      {
        if (mpStateIconIds[i].state == state)
        {
          return mpStateIconIds[i].bmIcon;
        }
      }
      return NULL;
    }



  } // namespace display
} // namespace mpc


