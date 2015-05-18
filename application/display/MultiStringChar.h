/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : MultiStringChar                                       */
/*                                                                          */
/* FILE NAME        : MultiStringChar.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 2008-02-11                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for showing and editing a MultiStringChar      */
/* (a-zA-Z0-9.,-/# ). Implementation has similarities to Number.            */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __MultiStringChar_H__
#define __MultiStringChar_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "ObserverText.h"
#include "MultiString.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define MULTISTRING_FLASH_VALUE 3
#define MULTISTRING_NO_LETTER_ENTERED_CHAR "."
#define MULTISTRING_REPLACE_ILLEGAL_CHAR '*'

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {

    /*
     * first char (@) is reserved for index 0 and is never shown
     * last char (*) is used for chars not entered and is replaced with NO_LETTER_ENTERED_CHAR
     */
    static const char sMultiStringChars[] = {'@',
                                             'a','b','c','d','e','f','g','h','i','j',
                                             'k','l','m','n','o','p','q','r','s','t',
                                             'u','v','w','x','y','z','A','B','C','D',
                                             'E','F','G','H','I','J','K','L','M','N',
                                             'O','P','Q','R','S','T','U','V','W','X',
                                             'Y','Z','0','1','2','3','4','5','6','7',
                                             '8','9','.',':','/','#','+','-',' ',
                                             '*','\0'};

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    * This Class is responsible for showing and editing a MultiStringChar given by a StringDataPoint
    *
    *****************************************************************************/
    class MultiStringChar : public ObserverText
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      MultiStringChar(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~MultiStringChar();
      /********************************************************************
      ASSIGNMENT OPERATOR
      ********************************************************************/

      /********************************************************************
      OPERATIONS
      ********************************************************************/

      virtual void Init(U8 position, U8 noOfChars);

      virtual void Run();

      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      /* --------------------------------------------------
      * Handle this key event. Return true if it is taken care of
      * --------------------------------------------------*/
      virtual bool HandleKeyEvent(Keys KeyID);
      
      /* --------------------------------------------------
      * Gets the LEDs which this element and the ones below, wants to be on
      * or off. Normaly this is also the keys to react, but it doesn't have
      * to be this way ...
      * --------------------------------------------------*/
      virtual Leds GetLedsStatus();
      
      /* --------------------------------------------------
      * Gets the keys to which we should react in this element and the
      * elements below. Thereby this is the element to send key messages
      * to for the given keys.
      * --------------------------------------------------*/
      virtual Keys GetLegalKeys();

      /* --------------------------------------------------
      * Puts the component in edit mode.
      * --------------------------------------------------*/
      virtual bool BeginEdit();
      
      /* --------------------------------------------------
      * Cancels the edit mode started by a Call to BeginEdit.
      * The value being edited should return to the original.
      * --------------------------------------------------*/
      virtual bool CancelEdit();
      
      /* --------------------------------------------------
      * Ends edit mode and stores the new value.
      * --------------------------------------------------*/
      virtual bool EndEdit();

      virtual void Update(Subject* pSubject);

      virtual void UpdateCapturedValue(bool isInEditMode);

    private:
      typedef  enum 
      { 
        NORMAL,
        FLASHING
      } NumberEditState;

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void UpdateSubjectValue();              

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      U8  mPosition; // char position in string (0-max)
      U8  mNoOfChars;

      const U8 mMinCharIdx;  
      const U8 mMaxCharIdx;
      
      I8  mOriginalCharIdx; 
      I8  mCapturedCharIdx; 

      bool mToggle;
      NumberEditState  mFlashingState;
      U8  mFlashingCounter;
      bool mIsEdited;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
    };
  } // namespace display
} // namespace mpc

#endif
