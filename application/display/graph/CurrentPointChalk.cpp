/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Plaform                                   */
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
/* CLASS NAME       : CurrentPointChalk                                     */
/*                                                                          */
/* FILE NAME        : CurrentPointChalk.cpp                                 */
/*                                                                          */
/* CREATED DATE     : 29-01-2008   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* See header file                                                          */
/****************************************************************************/

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <GUI.h>
#include <MPCFonts.h>
#include <FloatDataPointToString.h>
#include <FloatDataPointQuantity.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <CurrentPointChalk.h>
#include <BlackBoard.h>

namespace mpc
{
  namespace display
  {

    #define CPC_NQ_WIDTH 74

    /*****************************************************************************
    * LIFECYCLE - Constructor
    *****************************************************************************/
    CurrentPointChalk::CurrentPointChalk(BlackBoard* pBlackBoard) : Chalk(pBlackBoard)
    {
      SetTextPosition(QUADRANT_POSITION_TOP_RIGHT);
      mpCurrentXNQPosX      = 1;
      mpCurrentXNQPosY      = 1;
      mpCurrentYNQPosX      = 1;
      mpCurrentYNQPosY      = 1 + 15;
    }

    /*****************************************************************************
    * LIFECYCLE - Destructor
    *****************************************************************************/
    CurrentPointChalk::~CurrentPointChalk()
    {

    }

    /*****************************************************************************
    * FUNCTION - Redraw
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointChalk::Redraw()
    {
      if ((mpBlackBoard != NULL) && (mpCurrentXDP != NULL) && (mpCurrentYDP != NULL))
      {
        int width = mpBlackBoard->GetWidth() - 2 * mpBlackBoard->GetPointMargin();
        int height = mpBlackBoard->GetHeight();
        char str[81];

        CalculateInternals(width, height);

        if (mpCurrentXDP->IsAvailable() && mpCurrentYDP->IsAvailable())
        {
          float val = mpCurrentYDP->GetValueAsPercent();

          int x = mpBlackBoard->GetPointMargin() + (int)(mpCurrentXDP->GetValueAsPercent()*(width) / 100.0f);
          int y = height - 1 - (int)(val * (height - 1) / 100.0f);

          GUI_SetLineStyle(GUI_LS_DOT);
          GUI_SetColor(mpBlackBoard->GetColour());
          GUI_DrawLine(x,height,x,y);
          GUI_DrawLine(0,y,x,y);
          GUI_SetLineStyle(GUI_LS_SOLID);
          GUI_DrawLine(x-2,y,x+2,y);
          GUI_DrawLine(x,y-2,x,y+2);
        }

        GUI_SetFont(*DEFAULT_FONT_13_LANGUAGE_INDEP);
        GUI_SetColor(GUI_COLOUR_GRAPH_DEFAULT);
        GUI_RECT rect;

        rect.x0 = mpCurrentXNQPosX;
        rect.y0 = mpCurrentXNQPosY;
        rect.x1 = mpCurrentXNQPosX + 32;
        rect.y1 = mpCurrentXNQPosY + 15;
        if (mpCurrentXDP->IsAvailable())
        {
          FloatToString(str,mpCurrentXDP->GetValue(),mpCurrentXDP->GetMaxValue(),3,mpCurrentXDP->GetQuantity(),false);
          GUI_DispStringInRect(str, &rect, GUI_TA_VCENTER | GUI_TA_RIGHT);
        }
        else if (mpCurrentXDP->IsNotAvailable())
        {
          GUI_DispStringInRect("--", &rect, GUI_TA_VCENTER | GUI_TA_RIGHT);
        }

        if (!mpCurrentXDP->IsNeverAvailable())
        {
          rect.x0 = mpCurrentXNQPosX + 34;
          rect.y0 = mpCurrentXNQPosY;
          rect.x1 = mpCurrentXNQPosX + 72;
          rect.y1 = mpCurrentXNQPosY + 15;
          FloatDataPointQuantity(str,mpCurrentXDP);
          GUI_DispStringInRect(str,&rect,GUI_TA_VCENTER | GUI_TA_LEFT);
        }

        rect.x0 = mpCurrentYNQPosX;
        rect.y0 = mpCurrentYNQPosY;
        rect.x1 = mpCurrentYNQPosX + 32;
        rect.y1 = mpCurrentYNQPosY + 15;
        if (mpCurrentYDP->IsAvailable())
        {
          FloatToString(str, mpCurrentYDP->GetValue(), mpCurrentYDP->GetMaxValue(), 4, mpCurrentYDP->GetQuantity(), false);
          GUI_DispStringInRect(str,&rect,GUI_TA_VCENTER | GUI_TA_RIGHT);
        }
        else if (mpCurrentYDP->IsNotAvailable())
        {
          GUI_DispStringInRect("--", &rect, GUI_TA_VCENTER | GUI_TA_RIGHT);
        }

        if (!mpCurrentYDP->IsNeverAvailable())
        {
          rect.x0 = mpCurrentYNQPosX + 34;
          rect.y0 = mpCurrentYNQPosY;
          rect.x1 = mpCurrentYNQPosX + 72;
          rect.y1 = mpCurrentYNQPosY + 15;
          FloatDataPointQuantity(str,mpCurrentYDP);
          GUI_DispStringInRect(str,&rect,GUI_TA_VCENTER | GUI_TA_LEFT);
        }
      }
    }

    /*****************************************************************************
    * FUNCTION - SetDataPoints
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointChalk::SetDataPoints(FloatDataPoint* pCurrentXDP, FloatDataPoint* pCurrentYDP)
    {
      mpCurrentXDP = pCurrentXDP;
      mpCurrentYDP = pCurrentYDP;
    }

    /*****************************************************************************
    * FUNCTION - SetTextPosition
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointChalk::SetTextPosition(QUADRANT_POSITION_TYPE textPosition)
    {
      mTextPosition = textPosition;
    }

    /*****************************************************************************
    * FUNCTION - CalculateInternals
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurrentPointChalk::CalculateInternals(int width, int height)
    {
      switch (mTextPosition)
      {
        case QUADRANT_POSITION_TOP_LEFT:
          mpCurrentXNQPosX = 1;
          mpCurrentXNQPosY = 1;
          mpCurrentYNQPosX = 1;
          mpCurrentYNQPosY = 1 + 15;
          break;
        case QUADRANT_POSITION_TOP_RIGHT:
          mpCurrentXNQPosX = width - 2 - CPC_NQ_WIDTH;
          mpCurrentXNQPosY = 1;
          mpCurrentYNQPosX = width - 2 - CPC_NQ_WIDTH;
          mpCurrentYNQPosY = 1 + 15;
          break;
        case QUADRANT_POSITION_TOP_CENTRE:
          mpCurrentXNQPosX = (width - 2 - CPC_NQ_WIDTH)/2 + 1;
          mpCurrentXNQPosY = 1;
          mpCurrentYNQPosX = (width - 2 - CPC_NQ_WIDTH)/2 + 1;
          mpCurrentYNQPosY = 1 + 15;
          break;

        case QUADRANT_POSITION_BOTTOM_LEFT:
          mpCurrentXNQPosX = 1;
          mpCurrentXNQPosY = height - 1 - 30;
          mpCurrentYNQPosX = 1;
          mpCurrentYNQPosY = height - 1 - 15;
          break;
        case QUADRANT_POSITION_BOTTOM_RIGHT:
          mpCurrentXNQPosX = width - 2 - CPC_NQ_WIDTH;
          mpCurrentXNQPosY = height - 1 - 30;
          mpCurrentYNQPosX = width - 2 - CPC_NQ_WIDTH;
          mpCurrentYNQPosY = height - 1 - 15;
          break;
        case QUADRANT_POSITION_BOTTOM_CENTRE:
          mpCurrentXNQPosX = (width - 2 - CPC_NQ_WIDTH)/2 + 1;
          mpCurrentXNQPosY = height - 1 - 30;
          mpCurrentYNQPosX = (width - 2 - CPC_NQ_WIDTH)/2 + 1;
          mpCurrentYNQPosY = height - 1 - 15;
          break;

        case QUADRANT_POSITION_CENTRE_LEFT:
          mpCurrentXNQPosX = 1;
          mpCurrentXNQPosY = (height - 1)/2 - 7;
          mpCurrentYNQPosX = 1;
          mpCurrentYNQPosY = (height - 1)/2 + 8 ;
          break;
        case QUADRANT_POSITION_CENTRE_RIGHT:
          mpCurrentXNQPosX = width - 2 - CPC_NQ_WIDTH;
          mpCurrentXNQPosY = (height - 1)/2 - 7;
          mpCurrentYNQPosX = width - 2 - CPC_NQ_WIDTH;
          mpCurrentYNQPosY = (height - 1)/2 + 8 ;
          break;
        case QUADRANT_POSITION_CENTRE_CENTRE:
          mpCurrentXNQPosX = (width - 2 - CPC_NQ_WIDTH)/2 + 1;
          mpCurrentXNQPosY = (height - 1)/2 - 7;
          mpCurrentYNQPosX = (width - 2 - CPC_NQ_WIDTH)/2 + 1;
          mpCurrentYNQPosY = (height - 1)/2 + 8 ;
          break;
      }
    }

  } // namespace mpc
} // namespace display
