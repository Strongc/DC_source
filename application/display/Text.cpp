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
/* FILE NAME        : Text.cpp                                              */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a text.                        */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include "StringWidthCalculator.h"
#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Text.h"
#include <mpcfonts.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define MAX_WORD_WRAP_LINES 50

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
    Text::Text(Component* pParent) : Component(pParent)
    {
      mText = NULL;
      mTextWrapped = NULL;
      mConstant = false;
      mWordWrap = false;
      mWordWrapRecalc = true;
      mpFont = DEFAULT_FONT_13_LANGUAGE_DEP;
      mLeftMargin = 0;
      mRightMargin = 0;
      mAlign = GUI_TA_LEFT | GUI_TA_VCENTER;
      mColour = GUI_COLOUR_TEXT_DEFAULT_FOREGROUND;
      mTextIsOutOfBounds = false;
      mShowIfTextContinues = false;
      mIsInvalidated = true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    Text::~Text()
    {
      if (mText != NULL && !mConstant)
      {
        delete[] mText;
      }
      if (mTextWrapped != NULL)
      {
        delete[] mTextWrapped;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool Text::Redraw()
    {
      bool return_value = false;
      if (IsVisible() && mIsInvalidated)
      {
        mIsInvalidated = false;

        // use GetText() rather than mText to use any subclass implementations of GetText (TimeText in particular)  
        const char* pText = GetText();

        Component::Redraw();
        if (pText != NULL)
        {
          GUI_SetFont(GetFont());

          GUI_RECT rect; // = (GUI_RECT)(GetClientArea());
          rect.x0 = mLeftMargin;
          rect.y0 = 0;
          rect.x1 = GetWidth() - mRightMargin - 1;
          rect.y1 = GetHeight() - 1;

          // Find the max length of a line.
          const int max_line_length = rect.x1 - rect.x0 + 1;  // we can draw on both the first and the last pixel (FKA)
          const int max_line_count = (rect.y1 - rect.y0 + 1) / GetFont()->YSize;

          if (mWordWrap)
          {
            if (mWordWrapRecalc)
            {
              int line_count = WrapText(max_line_length);
              
              mTextIsOutOfBounds = (line_count > max_line_count);

              mWordWrapRecalc = false;
            }

            DispStringInRect(mTextWrapped,&rect,mAlign);

            return_value = true;
          }
          else // No word wrapping
          {
            DispStringInRect(pText, &rect, mAlign);

            int line_length = GUI_GetStringDistX( pText );

            mTextIsOutOfBounds = (line_length > max_line_length);

            if (mTextIsOutOfBounds && mShowIfTextContinues)
            {
              int continues_length = GUI_GetStringDistX("...") + 1;

              GUI_RECT rectContinues;
              rectContinues.y0 = 0;
              rectContinues.x1 = GetWidth() - mRightMargin - 1;
              rectContinues.y1 = GetHeight() - 1;

              rectContinues.x0 = rectContinues.x1 - continues_length;
              rect.x1 -= continues_length;

              Component::Redraw();
              DispStringInRect(pText, &rect, mAlign);
              DispStringInRect("...", &rectContinues, (mAlign & GUI_TA_VERTICAL) | GUI_TA_RIGHT);
            }
          }
          
        }
      }
      Validate();

      return return_value;
    }

    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void Text::Invalidate()
    {
      Component::Invalidate();
      mIsInvalidated = true;
    }
    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Calculates if the whole string can be show in the client area
    *****************************************************************************/
    bool Text::IsBeyondClientArea()
    {
      return mTextIsOutOfBounds;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Returns the font of this text element
    *****************************************************************************/
    const GUI_FONT* Text::GetFont()
    {
      return *mpFont;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the font of this text element
    *****************************************************************************/
    void Text::SetFont(const GUI_FONT** Font)
    {
      mpFont = Font;
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the alignment of the element
    *****************************************************************************/
    void Text::SetAlign(int Align)
    {
      mAlign = Align;
      Invalidate();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the alignment of the element
    *****************************************************************************/
    int Text::GetAlign()
    {
      return mAlign;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Sets the text to be displayed and marks the component for update.
    * The parameter pText is always cloned, so changing the content of pText after a call
    * to this function will not affect the contens of the Text component.
    *****************************************************************************/
    void Text::SetText(const char* pText)
    {
      mWordWrapRecalc = true;

      bool is_changed = (mText == NULL || strcmp(mText, pText) != 0);
      
      // Return if the old string matches the new string
      if (!is_changed)
      {
        return;
      }

      if (mText != NULL)
      {
        delete[] mText;
        mText = NULL;
      }

      mText = new char[strlen(pText) + 3];
      strcpy(mText, pText);
      

      if (mWordWrap)
      {
        if (mTextWrapped != NULL)
        {
          delete[] mTextWrapped;
          mTextWrapped = NULL;
        }

        int len = strlen(mText) + MAX_WORD_WRAP_LINES;
        mTextWrapped = new char[len];
      }

      if (is_changed)
      {
        Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Gets the current text.
    *****************************************************************************/
    const char* Text::GetText()
    {
      return mText;
    }

    void Text::SetLeftMargin(U16 px)
    {
      mLeftMargin = px;
    }

    void Text::SetRightMargin(U16 px)
    {
      mRightMargin = px;
    }

    U16 Text::GetLeftMargin()
    {
      return mLeftMargin;
    }

    U16 Text::GetRightMargin()
    {
      return mRightMargin;
    }

    void Text::SetWordWrap(bool on /*= true*/)
    {
      mWordWrap = on;
      if (mWordWrap)
      {
        if(mTextWrapped != NULL)
        {
          delete[] mTextWrapped;
          mTextWrapped = NULL;
        }

        if (mText != NULL)
        {
          mTextWrapped = new char[strlen(mText)+MAX_WORD_WRAP_LINES];
        }
      }
      Invalidate();
    }

    /* --------------------------------------------------
    * Determins if the word wrap has to be recalculated
    * --------------------------------------------------*/
    void Text::SetSize(int width, int height)
    {
      if (width != GetWidth()
        || height != GetHeight())
      {
        mWordWrapRecalc = true;
      }
      Component::SetSize(width, height);
    }

      /* --------------------------------------------------
      * Determins if the word wrap has to be recalculated
      * --------------------------------------------------*/
    void Text::SetWidth(int width)
    {
      if (width != GetWidth())
      {
        mWordWrapRecalc = true;
      }
      Component::SetWidth(width);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    int Text::WrapText(int maxLineLength)
    {
      char end_of_wrapped_word[1024];

      strcpy(mTextWrapped, mText);

      char* p_last_valid_word = mTextWrapped;
      char* p_current_word = mTextWrapped;

      int line_count = 0;
      int line_length = 0;
      int bytes_in_char = 0;
      GUI_RECT rect;

      int cur_word_len = GUI_GetStringDistX(p_current_word);
      if (maxLineLength > 4 && strlen(mTextWrapped) > 2 && cur_word_len >= maxLineLength)
      {
        // decrypt the line.
        while (strlen(p_last_valid_word) > 0)
        {
          cur_word_len = 0;

          // Find next word length.
          while (*p_current_word != '\n' && *p_current_word != ' ' && strlen(p_current_word) > 0)
          {
            bytes_in_char = GUI_UC_GetCharSize(p_current_word);
            GUI_GetTextExtend(&rect, p_current_word, 1);
            cur_word_len += (rect.x1 - rect.x0) + 1;

            // The entire word is longer than the line, so break the word.
            if (cur_word_len >= maxLineLength)
            {
              
              // Find the point to break the word at
              bytes_in_char = GUI_UC_GetCharSize(p_last_valid_word);
              p_current_word = p_last_valid_word;
              GUI_GetTextExtend(&rect, p_current_word, 1);
              cur_word_len = line_length + (rect.x1 - rect.x0) + 1;

              while (cur_word_len < maxLineLength)
              {
                p_current_word += bytes_in_char;
                bytes_in_char = GUI_UC_GetCharSize(p_current_word);
                GUI_GetTextExtend(&rect, p_current_word, 1);
                cur_word_len += (rect.x1 - rect.x0) + 1;
              }

              // break the word
              strcpy(end_of_wrapped_word, p_current_word);
              *(p_current_word) = '\n';
              *(p_current_word + 1) = '\0';
              strcat(p_current_word, end_of_wrapped_word);
              
              cur_word_len = 0;
              line_length = 0;
            }
            else
            {
              p_current_word += bytes_in_char;
            }
          }

          // If the line length of the current line exceeds the max line length
          // split the line.
          if (line_length + cur_word_len >= maxLineLength)
          {
            // replace the space before the current word with a new line
            (*p_last_valid_word) = '\n';
            line_length = 0;
            line_count++;
          }
          else
          {
            GUI_GetTextExtend(&rect, p_current_word, 1);
            line_length += cur_word_len + (rect.x1 - rect.x0) + 1;
            p_last_valid_word = p_current_word;
            if ((*p_last_valid_word) == '\n')
            {
              line_length = 0;
              line_count++;
            }
          }
          bytes_in_char = GUI_UC_GetCharSize(p_last_valid_word);
          p_current_word = p_last_valid_word + bytes_in_char;

        } 
      }

      return ++line_count;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void Text::DispStringInRect(const char* pText, GUI_RECT* pRect, int textAlign)
    {
      GUI_DispStringInRect(pText,pRect,textAlign);
    }


#ifdef __PC__
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/
    void Text::CalculateStringWidths(bool forceVisible, int stringId)
    {
      //Handle cvs file
      GUI_SetFont(GetFont());
      Component::CalculateStringWidths(false);
      int component_width = GetWidth() - mLeftMargin - mRightMargin;
      int component_height = GetHeight();
      int string_width_pixels = -1;
      int string_height_pixels = -1;
      bool fits = true;

      GUI_RECT rect;
      rect.x0 = 0;
      rect.x1 = 0;
      rect.y0 = 0;
      rect.y1 = 0;

      if (!mWordWrap)
      {
        if (mText!=NULL && strlen(mText)>0 && mText[strlen(mText)-1] != '}')
        {
          GUI_GetTextExtend(&rect, mText, strlen(mText));

          string_width_pixels  = abs(rect.x1 - rect.x0) + 1;
          string_height_pixels = abs(rect.y1 - rect.y0) + 1;

          fits = (string_width_pixels <= component_width);
        }
      }
      else
      {
        if (mText!=NULL && mTextWrapped!=NULL && strlen(mTextWrapped)>0 && mText[strlen(mText)-1] != '}')
        {
          GUI_GetTextExtend(&rect, mTextWrapped, strlen(mTextWrapped));

          string_width_pixels = abs(rect.x1 - rect.x0) + 1;

          int max_line_length = GetWidth() - mRightMargin - mLeftMargin;
         
          int no_of_lines = WrapText(max_line_length);

          string_height_pixels = (abs(rect.y1 - rect.y0) + 1);

          fits = (string_height_pixels <= component_height);
        }
      }


      CSV_ENTRY entry;
      entry.componentId = mComponentId;
      entry.stringId = stringId;
      entry.componentWidth = component_width;
      entry.stringWidth = string_width_pixels;
      entry.componentHeight = component_height;
      entry.stringHeight = string_height_pixels;
      entry.wordwrap = mWordWrap;
      entry.fits = fits;
      entry.visible = IsVisible();
      entry.forcedVisible = forceVisible;

      StringWidthCalculator::GetInstance()->WriteToCSV(entry);
    }
#endif

  } // namespace display
} // namespace mpc
