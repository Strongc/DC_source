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
/* CLASS NAME       : CrossChalk                                            */
/*                                                                          */
/* FILE NAME        : CrossChalk.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 26-11-2009   (dd-mm-yyyy)                             */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* See header file                                                          */
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
#include <CrossChalk.h>

namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    * LIFECYCLE - Constructor
    *****************************************************************************/
    CrossChalk::CrossChalk(BlackBoard* pBlackBoard) : Chalk(pBlackBoard)
    {
      mpData = NULL;
    }

    /*****************************************************************************
    * LIFECYCLE - Destructor
    *****************************************************************************/
    CrossChalk::~CrossChalk()
    {
    }

    /*****************************************************************************
    * FUNCTION - SetData
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CrossChalk::SetData(CurrentPointCurveData*  pData)
    {
      mpData = pData;
    }

    /*****************************************************************************
    * FUNCTION - Redraw
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CrossChalk::Redraw()
    {
      if ((mpBlackBoard != NULL) && (mpData != NULL))
      {
        int width = mpBlackBoard->GetWidth();
        int height = mpBlackBoard->GetHeight();
        int length = mpData->GetSize();

        if (length > width)
        {
          length = width;
        }

        GUI_SetLineStyle(GUI_LS_SOLID);
        GUI_SetColor(mpBlackBoard->GetColour());

        GUI_RECT rect;
        GUI_GetClientRect(&rect);

        mpData->Lock();
        
        for (int i = 0; i < length; i++)
        {
          if (mpData->IsCurveValueValid(i))
          {
            float val = mpData->GetCurveValueAsPercent(i);
            
            int y = height - 1 - (int)(val * (height - 1) / 100.0f);
 
            DrawCross(i, y);
          }
        }
        
        mpData->UnLock();
      }
    }

    /*****************************************************************************
    * FUNCTION - DrawCross
    * DESCRIPTION: 
    *****************************************************************************/
    void CrossChalk::DrawCross(int xCenter, int yCenter)
    {
      GUI_MoveTo(xCenter - 2, yCenter + 2);
      GUI_DrawLineTo(xCenter + 2, yCenter - 2);

      GUI_MoveTo(xCenter - 2, yCenter - 2);
      GUI_DrawLineTo(xCenter + 2, yCenter + 2);
    }
  } // namespace mpc
} // namespace display
