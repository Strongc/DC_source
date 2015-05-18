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
/* CLASS NAME       : MultiString                                           */
/*                                                                          */
/* FILE NAME        : MultiString.h                                         */
/*                                                                          */
/* CREATED DATE     : 2008-02-11                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for editing a string datapoint                 */
/* Implementation has similarities to MultiNumber.                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayMultiString_h
#define mpc_displayMultiString_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <StringDataPoint.h>
#include <Observer.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "Group.h"
#include "MultiStringChar.h"
#include "Frame.h"
#include "Image.h"
#include "Label.h"
#include "DataPointText.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define MULTISTRING_MAX_STRING_LEN 100

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    // FORWARD DECLARATIONS
    class Frame;
    class MultiStringChar;


    class MultiString : public Group, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      MultiString(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~MultiString();
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
      
      virtual void Run();

      virtual bool IsValid();

      virtual bool HandleKeyEvent(Keys KeyID);
      
      virtual Leds GetLedsStatus();


      virtual void SubscribtionCancelled(Subject* pSubject);
      virtual void Update(Subject* pSubject);
      virtual void SetSubjectPointer(int Id,Subject* pSubject);
      virtual void ConnectToSubjects(void);

      virtual bool SetReadOnly(bool readOnly = true);

      virtual void SetClientArea(int x1, int y1, int x2, int y2);

      virtual void SetColour(U32 colour, bool forced = false);

    private:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      
    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void PadAndCaptureSubject(bool isInEditMode);

      virtual void StripCommitAndRecapture();

      virtual void Init(U8 noOfChars);

      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      I8 mCurrentField;// 0 - mNoOfChars
      U8 mNoOfChars;   // 0 - MULTISTRING_MAX_STRING_LEN

      SubjectPtr<StringDataPoint*> mpDpSubject;
      StringDataPoint*    mpDpCapturedString;

      Frame*              mpFrames;
      MultiStringChar*    mpMultiStringChars;

      Image*              mpImgMoreCharsToTheLeft;
      Image*              mpImgMoreCharsToTheRight;

      DataPointText*      mpNonEditableText;

    };
  } // namespace display
} // namespace mpc

#endif


