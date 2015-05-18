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
/* CLASS NAME       : CurveChalk                                            */
/*                                                                          */
/* FILE NAME        : CurveChalk.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 29-01-2008   (dd-mm-yyyy)                             */
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
#include <CurveChalk.h>

namespace mpc
{
  namespace display
  {

    /*****************************************************************************
    * LIFECYCLE - Constructor
    *****************************************************************************/
    CurveChalk::CurveChalk(BlackBoard* pBlackBoard) : Chalk(pBlackBoard)
    {
      mpData = NULL;
    }

    /*****************************************************************************
    * LIFECYCLE - Destructor
    *****************************************************************************/
    CurveChalk::~CurveChalk()
    {
    }

    /*****************************************************************************
    * FUNCTION - SetData
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurveChalk::SetData(CurrentPointCurveData*  pData)
    {
      mpData = pData;
    }

    /*****************************************************************************
    * FUNCTION - Redraw
    * DESCRIPTION: See header file
    *****************************************************************************/
    void CurveChalk::Redraw()
    {
      if ((mpBlackBoard != NULL) && (mpData != NULL))
      {
        int width = mpBlackBoard->GetWidth();
        int height = mpBlackBoard->GetHeight();
        int length = mpData->GetSize();
        bool first_to_draw = true;

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
            int a = height - 1 - (int)(mpData->GetCurveValueAsPercent(i) * (height - 1) / 100.0f);

            if (first_to_draw)
            {
              GUI_MoveTo(i,a);
              GUI_DrawLineTo(i,a);
              first_to_draw = false;
            }
            else
            {
              GUI_DrawLineTo(i,a);
            }
          }
          else
          {
            first_to_draw = true;
          }
        }
        
        mpData->UnLock();
      }
    }
  } // namespace mpc
} // namespace display
