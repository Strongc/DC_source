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
/* CLASS NAME       : ConfirmWrite                                          */
/*                                                                          */
/* FILE NAME        : ConfirmWrite.h                                        */
/*                                                                          */
/* CREATED DATE     : 2008-7-4                                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* The ConfirmWrite controls when to show a CancelOrContinuePopUp,          */
/* which is able to set the value of datapoint implementing                 */
/* IintegerDatapoint.                                                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef mrc_displayConfirmWrite_h
#define mrc_displayConfirmWrite_h

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Observer.h>
#include <Subject.h>
#include <IIntegerDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
*****************************************************************************/
#include <PopupBox.h>
#include <Frame.h>
#include <Label.h>
#include <ListView.h>
#include <Image.h>


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

    //forward declaration
    class CancelOrContinuePopUp;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class ConfirmWrite : public Component, public Observer
    {
    public:
      ConfirmWrite(Component* pParent = NULL);
      virtual ~ConfirmWrite();

      /********************************************************************
      OPERATIONS
      ********************************************************************/
      virtual void SetQuestionStringId(STRING_ID question);
      virtual void SetValueToSet(int value);

      virtual Leds GetLedsStatus(void);
      virtual bool HandleKeyEvent(Keys KeyID);

      /* --------------------------------------------------
      * Update is part of the observer pattern
      * --------------------------------------------------*/
      virtual void Update(Subject* Object){}
      /* --------------------------------------------------
      * Called if subscription shall be canceled
      * --------------------------------------------------*/
      virtual void SubscribtionCancelled(Subject* pSubject){}
      /* --------------------------------------------------
      * Called to set the subject pointer
      * --------------------------------------------------*/
      virtual void SetSubjectPointer(int Id, Subject* pSubject);
      /* --------------------------------------------------
      * Called to indicate that subscription kan be made
      * --------------------------------------------------*/
      virtual void ConnectToSubjects(void){}

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
      virtual void ShowPopup(void);
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      int    mValue;
      SubjectPtr<IIntegerDataPoint*> mDpDestination;

      CancelOrContinuePopUp* mpCancelOrContinuePopUp;
    };


    class CancelOrContinuePopUp : public PopupBox
    {
    public:
      CancelOrContinuePopUp();
      ~CancelOrContinuePopUp();
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      void SetQuestionStringId(STRING_ID question);
      void SetValueToSet(int value);
      void SetDestinationSubject(Subject* pSubject);

      bool HandleKeyEvent(Keys KeyID);

      void HidePopup(void);

    protected:
      /********************************************************************
      OPERATIONS
      ********************************************************************/
      /********************************************************************
      ATTRIBUTE
      ********************************************************************/
      int        mValue;
      SubjectPtr<IIntegerDataPoint*> mDpDestination;
      Frame*     mpFrame;
      Label*     mpLabelQuestion;
      ListView*  mpListOptions;
      Label*     mpLabelCancel;
      Label*     mpLabelContinue;
      Image*     mpImgGoForward;
      Image*     mpImgGoBack;
    };
      


  } // namespace display
} // namespace mpc

#endif
