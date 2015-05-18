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
/* CLASS NAME       : Image                                                 */
/*                                                                          */
/* FILE NAME        : Image.cpp                                             */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a Image.                       */
/*                                                                          */
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
#include "Image.h"
#include <Factory.h>
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
    Image::Image(Component* pParent) : Component(pParent)
    {
      mpBitmap = NULL;
      mAlign = GUI_TA_HCENTER | GUI_TA_VCENTER;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Image::~Image()
    {
    }

    /*****************************************************************************
    * Function - SetBitmap
    * DESCRIPTION:
    *****************************************************************************/
    void Image::SetBitmap(GUI_CONST_STORAGE GUI_BITMAP* pBitmap)
    {
      mpBitmap = pBitmap;

      if (mClientRect.x0 == mClientRect.x1 == mClientRect.y0 == mClientRect.y1)
      {
        SetHeight(pBitmap->YSize);
        SetWidth(pBitmap->XSize);
      }

      Invalidate();
    }

    /*****************************************************************************
    * Function - SetAlign
    * DESCRIPTION:
    *****************************************************************************/
    void Image::SetAlign(int Align)
    {
      mAlign = Align;
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool Image::Redraw()
    {
      Component::Redraw();

      if (IsVisible() && mpBitmap != NULL)
      {
        int width = GetWidth();
        int height = GetHeight();
        int x;
        int y;
        
        if ((mAlign & GUI_TA_HCENTER) == GUI_TA_HCENTER)
        {
          x = (width / 2.0) - (mpBitmap->XSize / 2.0);
        }
        else if ((mAlign & GUI_TA_RIGHT) == GUI_TA_RIGHT)
        {
          x = width - mpBitmap->XSize;
        }
        else // GUI_TA_LEFT
        {
          x = 0;
        }

        if ((mAlign & GUI_TA_VCENTER) == GUI_TA_VCENTER)
        {
          y = (height / 2.0) - (mpBitmap->YSize / 2.0);
        }
        else if ((mAlign & GUI_TA_BOTTOM) == GUI_TA_BOTTOM)
        {
          y = height - mpBitmap->YSize;
        }
        else // GUI_TA_TOP
        {
          y = 0;
        }

        GUI_DrawBitmap(mpBitmap,x,y);          
        Validate();
        return true;
      }
      return false;
    }

  } // namespace display
} // namespace mpc


