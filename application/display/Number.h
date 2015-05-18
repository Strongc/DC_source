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
/* CLASS NAME       : Number                                                */
/*                                                                          */
/* FILE NAME        : Number.h                                              */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayNumber_h
#define mpc_displayNumber_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DataPoint.h"
#include "ObserverText.h"
#include "NumericalDataPointInterface.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define MAX_NUMBER_STR_LEN  51

#define FLASH_VALUE 3

#define NUMBER_SAMPLE_TIME  333

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    // FORWARD DECLARATIONS
    class NumberGraphics;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class Number : public ObserverText
    {
    public:
      Number(Component* pParent = NULL);
      virtual ~Number();
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /* --------------------------------------------------
      * Redraws this element. If it fails (for some reason) the
      * method returns false.
      * --------------------------------------------------*/
      virtual bool Redraw();

      virtual void Run(void);
      virtual void SetNumberOfDigits(int numberOfDigits);
      virtual int GetNumberOfDigits(void);
      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject);
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void);
      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* pSubject);
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

      /* --------------------------------------------------
      * If the subject is a DataPoint and the quality is
      * DP_NEVER_AVAILABLE this function shall return true
      * --------------------------------------------------*/
      virtual bool IsNeverAvailable();

      /* --------------------------------------------------
      * Indicate that the DataPoint connected to this Number is captured
      * --------------------------------------------------*/
      virtual bool IsCaptured(void);

      virtual U16 SetUpdateDelay(U16 newUpdateDelay);
    private:
      typedef  enum { START,
                      CHANGE,
                      ESC,
                      VALIDATE
                    } NumberEditState;
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      U8 mNumberOfDigits;
      bool mToggle;
      NumberEditState  mState;
      int mChangeTimer;
      NumericalDataPointInterface* mNumericalDpIf;

      U16 mUpdateDelay;
      U16 mUpdateTimer;

      U8  mDecadeAccelerationLevel;
      U32 mTimesumChangeValue;

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      bool  mUpdate;
    };
  } // namespace display
} // namespace display

#endif
