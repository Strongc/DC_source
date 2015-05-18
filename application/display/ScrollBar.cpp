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
/* CLASS NAME       : ScrollBar                                             */
/*                                                                          */
/* FILE NAME        : ScrollBar.CPP                                         */
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
#include <math.h>
#include <GUI.h>
#include <GUIConf.h>
//#include <UnitList.h>
#include "ScrollBar.h"

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
    ScrollBar::ScrollBar(Component* pParent /*= NULL*/) : Frame(true,true,pParent)
    {
      mMinPos = 0;
      mMaxPos = 1;
      mCurPos = 0;
      mSliderSize = 5;
      mShowArrows = true;

      mColour = GUI_COLOUR_SLIDER_INACTIVE;
      mBackgroundColour = GUI_COLOUR_DEFAULT_BACKGROUND;

      mpSliderFrame = new Frame(true, true);
      mpSliderFrame->SetColour(GUI_COLOUR_SLIDER_INACTIVE);
      mpSliderFrame->SetFillingColour(GUI_COLOUR_SLIDER_INACTIVE);
      mpSliderFrame->SetBackgroundColour(GUI_COLOUR_DEFAULT_BACKGROUND);
      AddChild(mpSliderFrame);
      mpSliderFrame->SetVisible(true);
    }

    /*****************************************************************************
    * Destructor
    * DESCRIPTION:
    *****************************************************************************/
    ScrollBar::~ScrollBar()
    {
    }

    /*****************************************************************************
    * ShowArrows
    * DESCRIPTION:
    *****************************************************************************/
    void ScrollBar::ShowArrows(bool showArrows /* = true */)
    {
      mShowArrows = showArrows;
    }

    /*****************************************************************************
    * Function - SetRange
    * DESCRIPTION:
    *****************************************************************************/
    void ScrollBar::SetRange(U16 minPos, U16 maxPos)
    {
      if (mMinPos != minPos || mMaxPos != maxPos)
      {
        if (mMinPos < mMaxPos)
        {
          mMinPos = minPos;
          mMaxPos = maxPos;
        }
        Invalidate();
      }
    }

    /*****************************************************************************
    * Function - SetScrollPos
    * DESCRIPTION:
    *****************************************************************************/
    bool ScrollBar::SetScrollPos(U16 pos)
    {
      if( pos > mMaxPos || pos < mMinPos )
      {
        return false;
      }

      if(mCurPos != pos)
      {
        mCurPos = pos;
        Invalidate();
      }
      return true;
    }

    U16 ScrollBar::GetScrollPos()
    {
      return mCurPos;
    }


    /*****************************************************************************
    * Function - Redraw
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool ScrollBar::Redraw()
    {
      bool return_value;
      if (IsVisible())
      {
        // Draw the scrollbar
        const U16 y2 = (mShowArrows ? GetWidth() - 1 : 1);
        const U16 slider_area_height = GetSliderAreaSize() - mSliderSize + (mShowArrows ? - 2 : -4);

        U16 pos = 0;
        if ((mMaxPos - mMinPos) > 0)
        {
          pos = y2 + 1 + (((U32)slider_area_height) * mCurPos / (mMaxPos - mMinPos));
          mpSliderFrame->SetClientArea(2, pos, GetWidth() - 3, pos + mSliderSize - 1);
          mpSliderFrame->SetVisible();
        }
        else
        {
          mpSliderFrame->SetVisible(false);
        }

        Frame::Redraw();

        if (mShowArrows)
        {
          int width = GetWidth() - 1;
          int height = width - 2;
          DrawArrow(1, height + 1, height, width, true);
          DrawArrow(1, GetHeight() - 2 - height, height, width, false);
        }
        return_value = true;
      }
      else
      {
        return_value = false;
        Frame::Redraw();
      }
      return return_value;
    }


    /*****************************************************************************
    * Function - DrawArrow
    * DESCRIPTION:
    *****************************************************************************/
    void ScrollBar::DrawArrow(int xpos, int ypos, int heigh, int width, bool upwards)
    {
      GUI_SetColor(mpSliderFrame->GetFillingColour());

      // Up arrow frame
      U8 j = xpos;
      for (U8 i = 0; i < heigh; i++)
      {
        if (i % 2 == 0)
        {
          j++;
        }
        GUI_DrawHLine((upwards ? ypos - i : ypos + i), j, width - j );
      }

    }
    
    /*****************************************************************************
    * Function - SetSliderSize
    * DESCRIPTION:
    *****************************************************************************/
    void ScrollBar::SetSliderSize(U16 size)
    {
      mSliderSize = size;
      // Fix slider size
      if (mSliderSize < 5)
      {
        mSliderSize = 5;
      }
    }
    
    /*****************************************************************************
    * Function - GetSliderAreaSize
    * DESCRIPTION:
    *****************************************************************************/
    U16 ScrollBar::GetSliderAreaSize()
    {
      int height = GetHeight();
      int arrows_height = 2 * (GetWidth() - 1);

      return mShowArrows ? height - arrows_height : height;
    }

    
    /*****************************************************************************
    * Function - SetFocus
    * DESCRIPTION:
    *****************************************************************************/
    void ScrollBar::SetFocus(bool focus)
    {
      Component::SetFocus(focus);

      U32 colour = (focus ? GUI_COLOUR_SLIDER_ACTIVE : GUI_COLOUR_SLIDER_INACTIVE);

      mpSliderFrame->SetColour(colour);
      mpSliderFrame->SetFillingColour(colour);
    }

  } // namespace display
} // namespace mpc
