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
/* FILE NAME        : ScrollBar.h                                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayScrollBar_h
#define mpc_displayScrollBar_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Group.h"
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
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class ScrollBar : public Frame
    {
    public:
      ScrollBar(Component* pParent = NULL);
      virtual ~ScrollBar();

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      virtual void ShowArrows(bool showArrows = true);
      
      virtual void SetRange(U16 minPos, U16 maxPos);
      virtual bool SetScrollPos(U16 pos);
      virtual U16  GetScrollPos();
      
      virtual void SetSliderSize(U16 size);
      virtual U16 GetSliderAreaSize();
      virtual void SetFocus(bool focus = true);
      
    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void DrawArrow(int xpos, int ypos, int heigh, int width, bool upwards);
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      Frame*  mpSliderFrame;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      U16 mMinPos;
      U16 mMaxPos;
      U16 mCurPos;
      U16 mSliderSize;
      bool mShowArrows;
    };
  }
}

#endif
