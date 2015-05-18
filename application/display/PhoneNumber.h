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
/* CLASS NAME       : PhoneNumber                                           */
/*                                                                          */
/* FILE NAME        : PhoneNumber.h                                         */
/*                                                                          */
/* CREATED DATE     : 2007-11-21                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for editing a phone number                     */
/* Implementation has similarities to MultiNumber.                          */
/****************************************************************************/
/*****************************************************************************
Protect against Phoneple inclusion through the use of guards:
****************************************************************************/
#ifndef mpc_displayPhoneNumber_h
#define mpc_displayPhoneNumber_h

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
#include <Group.h>
#include <PhoneChar.h>
#include <Frame.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
#define PHONENUMBER_MAX_LEN 16

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    // FORWARD DECLARATIONS
    class Frame;
    class PhoneChar;

    class PhoneNumber : public Group, public Observer
    {
    public:
      /********************************************************************
      LIFECYCLE - Default constructor.
      ********************************************************************/
      PhoneNumber(Component* pParent = NULL);
      /********************************************************************
      LIFECYCLE - Destructor.
      ********************************************************************/
      virtual ~PhoneNumber();
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

      //virtual void SetColour(U32 colour, bool forced = false);

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
      virtual void PadAndCaptureSubject(bool inEditMode);

      virtual void StripCommitAndRecapture();
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      I8 mCurrentField;

      SubjectPtr<StringDataPoint*> mpDpSubject;
      StringDataPoint* mpDpCapturedString;

      Frame*          mpFrames;
      PhoneChar*      mpPhoneChars;

    };
  } // namespace display
} // namespace mpc

#endif


