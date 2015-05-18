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
/* CLASS NAME       : Frame                                                 */
/*                                                                          */
/* FILE NAME        : Frame.cpp                                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include "WM.h"
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Frame.h"

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
    * Constructor
    * DESCRIPTION:
    *****************************************************************************/
    Frame::Frame(bool fillBackground, bool drawFrame, Component* pParent) : Group( pParent )
    {
      mFillBackground = fillBackground;
      mDrawFrame = drawFrame;
      
      mFillingColour = GUI_COLOUR_DEFAULT_BACKGROUND;
      mIsFillingColourUsed = false;
    }

    /*****************************************************************************
    * Destructor
    * DESCRIPTION:
    *****************************************************************************/
    Frame::~Frame()
    {

    }

    /*****************************************************************************
    * Function - SetFrame
    * DESCRIPTION:
    *****************************************************************************/
    void Frame::SetFrame(bool enableFrame)
    {
      mDrawFrame = enableFrame;
      Invalidate();
    }

    /*****************************************************************************
    * Function - SetFillBackground
    * DESCRIPTION:
    *****************************************************************************/
    void Frame::SetFillBackground(bool fillBackground)
    {
      mFillBackground = fillBackground;
      Invalidate();
    }

    
    /*****************************************************************************
    * Function - SetFillingColour
    * DESCRIPTION:
    *****************************************************************************/
    void Frame::SetFillingColour(U32 fillingColour)
    {
      mIsFillingColourUsed = true;
      mFillingColour = fillingColour;
      Invalidate();
    }

    /*****************************************************************************
    * Function - GetFillingColour
    * DESCRIPTION:
    *****************************************************************************/
    U32 Frame::GetFillingColour(void)
    {
      return mFillingColour;
    }
    

    /*****************************************************************************
    * Function - ClearArea
    * DESCRIPTION:
    *****************************************************************************/
    void Frame::ClearArea()
    {
      if (!IsVisible())
      {
        Group::ClearArea();
      }
      else
      {
        int x1 = GetWidth() - 1;
        int y1 = GetHeight() - 1;
        if (mFillBackground)
        {
          GUI_SetBkColor(GetBackgroundColour());
          Group::ClearArea();
          if (mIsFillingColourUsed)
          {
            GUI_SetColor(mFillingColour);
            GUI_FillRect(1, 1, x1 - 1, y1 - 1);
          }
        }
        else
        {
          SelectWindow();
        }

        if (mDrawFrame)
        {
          GUI_SetColor(GetColour());
          GUI_DrawRect(0, 0, x1, y1);

          GUI_SetColor(GetBackgroundColour());
          GUI_DrawPoint(0, 0);
          GUI_DrawPoint(x1, 0);
          GUI_DrawPoint(x1, y1);
          GUI_DrawPoint(0, y1);
        }
      }
    }



  } // namespace display
} // namespace mpc
