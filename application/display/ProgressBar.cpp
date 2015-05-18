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
/* CLASS NAME       : ProgressBar                                                  */
/*                                                                          */
/* FILE NAME        : ProgressBar.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a ProgressBar.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include "ProgressBar.h"
#include <MpcFonts.h>
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

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
    ProgressBar::ProgressBar(Component* pParent) : Frame(true, true, pParent)
    {
      mMin = 0;
      mMax = 100;
      mBarLength = 0;
      mPos = 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ProgressBar::~ProgressBar()
    {
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool ProgressBar::Redraw()
    {
      Frame::Redraw();

      GUI_SetColor(GUI_COLOUR_BAR_DEFAULT);
      GUI_FillRect(1,1,mBarLength,GetHeight()-2);

      char sz_percent[40];
      GUI_RECT r;
      r.x0=0;
      r.y0=0;
      r.x1=GetWidth()-1;
      r.y1=GetHeight()-1;

      float percent = (mPos*100.0 / (float)(mMax - mMin));
      sprintf(sz_percent, "%0.0f%%",percent);
      GUI_SetColor(GetColour());
      GUI_SetTextMode(GUI_TEXTMODE_TRANS);
      GUI_SetFont(*DEFAULT_FONT_13_LANGUAGE_INDEP);
      GUI_DispStringInRect(sz_percent, &r, GUI_TA_VCENTER|GUI_TA_HCENTER);
      return true;
    }

    void ProgressBar::SetMinMaxPos(int min, int max)
    {
      if( mMin != min || mMax != max)
      {
        mMin = min;
        mMax = max;
        Invalidate();
      }
    }

    void ProgressBar::SetPos(int newPos)
    {
      int width = GetWidth() - 2;
      float percent = (newPos / (float)(mMax - mMin));
      int bar_length = width * percent;
      if( bar_length != mBarLength )
      {
        mPos = newPos;
        if(newPos == mMax)
        {
          mBarLength = width;
        }
        else
        {
          mBarLength = bar_length;
        }

        Invalidate();
      }
    }

    void ProgressBar::SetPercentVisible(bool visible /*= true*/)
    {
      
    }
  } // namespace display
} // namespace mpc
