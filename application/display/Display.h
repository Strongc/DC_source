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
/* CLASS NAME       : Display                                               */
/*                                                                          */
/* FILE NAME        : Display.h                                             */
/*                                                                          */
/* CREATED DATE     : 2004-10-18                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for a single line in a listbox.                */
/*                                                                          */
/****************************************************************************/

#ifndef mpcDisplay
#define mpcDisplay

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "leds.h"
#include "keys.h"
#include "Group.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define DISPLAY_NUMBER_STR_LEN 30
/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

/*****************************************************************************
* CLASS:
* DESCRIPTION:
*
* This Class is responsible for grouping and drawing displaycomponents.
*
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    // FOWARD declarations

    class Display
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      Display( Group* pRoot, STRING_ID Name, U16 Id, bool AbleToShow = true, bool Show = false );

      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~Display();

      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual int GetId();
      STRING_ID GetName();
      void SetName(STRING_ID DisplayTitleId);
      void SetTitleText(char* pText);
      const char* GetTitleText(void);
      bool Show();
      bool Hide();
      bool Redraw();
      void SetVisible(bool Visible = true);
      
      void SetAbleToShow(bool AbleToShow = true);
      bool GetAbleToShow();


      Group*  GetRoot();
      int  GetPasswordId();
      void SetPasswordId(int pw = 0);

      /* --------------------------------------------------
      * Gets the LEDs to turn on
      * --------------------------------------------------*/
      Leds GetLedsStatus(void);

      /* --------------------------------------------------
      * Gets the legal eys
      * --------------------------------------------------*/
      Keys GetLegalKeys(void);

      /* --------------------------------------------------
      * A key (with a key id) has been pressed
      * --------------------------------------------------*/
      void HandleKeyEvent(KeyId Key);

      /* --------------------------------------------------
      * Called by operating system to give time to redraw
      * --------------------------------------------------*/
      virtual void Run(void);

      virtual void SetDisplayNumber(const char* Number);
      virtual const char* GetDisplayNumber(void);
#ifdef __FADE__
      virtual void FadeIn(int startx = 240, int starty = 0);
      virtual void FadeOut(Display* to_display);
#endif // __FADE__

#ifdef __PC__
      virtual void CalculateStringWidths(bool forceVisible);
#endif

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      int mFadeX;
      int mFadeY;
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      bool          mVisible;
      bool          mAbleToShow;
      display::Group* mpRoot;
      STRING_ID     mName;
      U16           mId;
      int           mPasswordId;
      char          mDisplayNumber[DISPLAY_NUMBER_STR_LEN];
      char*         mpTitleText;
    };
  }
} // namespace mpc


#endif // mpcDisplay
