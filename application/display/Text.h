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
/* CLASS NAME       : Text                                                  */
/*                                                                          */
/* FILE NAME        : Text.h                                                */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayText_h
#define mpc_displayText_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Component.h"

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
    * This Class is responsible for how to show a text string given by a DataPoint
    *
    *****************************************************************************/
    class Text : public Component
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      Text(Component* pParent = NULL);

      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Text();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      virtual void Invalidate();

      /* --------------------------------------------------
      * Determins if the word wrap has to be recalculated
      * --------------------------------------------------*/
      virtual void SetSize(int width, int height);

      /* --------------------------------------------------
      * Determins if the word wrap has to be recalculated
      * --------------------------------------------------*/
      virtual void SetWidth(int width);

      /* --------------------------------------------------
      * Sets the text, and marks the component for update.
      * --------------------------------------------------*/
      virtual void SetText(const char* pText);
      /* --------------------------------------------------
      * Gets the current text.
      * --------------------------------------------------*/
      virtual const char* GetText();

      /* --------------------------------------------------
      * Returns the font of this text element
      * --------------------------------------------------*/
      virtual const GUI_FONT* GetFont();
      /* --------------------------------------------------
      * Sets the font of this text element
      * --------------------------------------------------*/
      virtual void SetFont(const GUI_FONT** Font);
      /* --------------------------------------------------
      * Sets the alignment of the element
      * --------------------------------------------------*/
      virtual void SetAlign(int Align);
      /* --------------------------------------------------
      * Gets the alignment of the element
      * --------------------------------------------------*/
      virtual int GetAlign();

      virtual void SetLeftMargin(U16 px);
      virtual void SetRightMargin(U16 px);
      virtual U16 GetLeftMargin();
      virtual U16 GetRightMargin();
      virtual void SetWordWrap(bool on = true);

      virtual bool IsBeyondClientArea();

    #ifdef __PC__
      virtual void CalculateStringWidths(bool forceVisible, int stringId);
    #endif


    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    protected:
      const GUI_FONT** mpFont;
      int mAlign;
      bool  mConstant;
      U16   mLeftMargin;
      U16   mRightMargin;
      char* mText;  // The text to be displayed.
      bool  mWordWrap;
      bool  mWordWrapRecalc;
      char* mTextWrapped;
      bool  mTextIsOutOfBounds;
      bool  mShowIfTextContinues;
      bool  mIsInvalidated;

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual int WrapText(int maxLineLength);
      virtual void DispStringInRect(const char* pText, GUI_RECT* pRect, int textAlign);
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/

    };
  } // namespace display
} // namespace mpc

#endif
