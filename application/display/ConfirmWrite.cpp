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
/* FILE NAME        : ConfirmWrite.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 2008-7-4                                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
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
#include "ConfirmWrite.h"

#include <MPCFonts.h>
#include <DisplayController.h>
#include <ListViewItem.h>

/*****************************************************************************
DEFINES
*****************************************************************************/
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmGoBack;
extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmGoOn;

#define ROW_NUMBER_CANCEL   0
#define ROW_NUMBER_CONTINUE 1

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {
  /*****************************************************************************
  *
  *
  *              PUBLIC FUNCTIONS
  *
  *
    *****************************************************************************/

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    ConfirmWrite::ConfirmWrite(Component* pParent) : Component( pParent )
    {
      mValue = -1;
      this->SetVisible(false);
      this->SetClientArea(0,0,0,0);
      
      mpCancelOrContinuePopUp = new CancelOrContinuePopUp();

      mLedsStatus = PLUS_LED | MINUS_LED | OK_LED;
      mLegalKeys  = MPC_OK_KEY |MPC_PLUS_KEY | MPC_PLUS_KEY_REP | MPC_MINUS_KEY | MPC_MINUS_KEY_REP;

    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    ConfirmWrite::~ConfirmWrite()
    {
      delete mpCancelOrContinuePopUp;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    void ConfirmWrite::SetQuestionStringId(STRING_ID question)
    {
      mpCancelOrContinuePopUp->SetQuestionStringId( question );
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    void ConfirmWrite::SetValueToSet(int value)
    {
      mValue = value;
      mpCancelOrContinuePopUp->SetValueToSet(value);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    Leds ConfirmWrite::GetLedsStatus()
    {
      if (mValue != mDpDestination->GetAsInt())
      {
        return PLUS_LED | MINUS_LED | OK_LED;
      }
      else
      {
        return NO_LED;
      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    bool ConfirmWrite::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_PLUS_KEY:
        case MPC_PLUS_KEY_REP:
        case MPC_MINUS_KEY:
        case MPC_MINUS_KEY_REP:
        case MPC_OK_KEY :
          if ( mValue != mDpDestination->GetAsInt() )
          {
            ShowPopup();
          }
          break;
      }
      
      return true;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to set the subject pointer (used by class
    * factory)
    *****************************************************************************/
    void ConfirmWrite::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mDpDestination.Attach(pSubject);
      mpCancelOrContinuePopUp->SetDestinationSubject(pSubject);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    void ConfirmWrite::ShowPopup()
    {
      mpCancelOrContinuePopUp->SetVisible();
      DisplayController::GetInstance()->PushPopupBox(mpCancelOrContinuePopUp);

      // need to call wm_bringToTop to ensure the popup is show every time ShowPopup is called
      WM_BringToTop(mpCancelOrContinuePopUp->GetWMHandle());
    }



    /*****************************************************************************
    * 
    * 
    * CLASS      : CancelOrContinuePopUp
    * 
    * 
    *****************************************************************************/   



    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    CancelOrContinuePopUp::CancelOrContinuePopUp() : PopupBox()
    {
      this->SetClientArea(2,44,237,232);

      mpFrame = new Frame(true, false, this);
      mpFrame->SetClientArea(2, 2, 233, 124);
      mpFrame->SetVisible();

      mpLabelQuestion = new Label(mpFrame);
      mpLabelQuestion->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpLabelQuestion->SetAlign( GUI_TA_TOP | GUI_TA_LEFT );
      mpLabelQuestion->SetClientArea(4, 15, 230, 120);
      mpLabelQuestion->SetWordWrap(true);
      mpLabelQuestion->SetStringId(SID_CONTINUE);
      mpLabelQuestion->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpLabelQuestion->SetVisible();
      mpFrame->AddChild(mpLabelQuestion);
      
      mpLabelCancel = new Label();
      mpLabelCancel->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpLabelCancel->SetLeftMargin(4);
      mpLabelCancel->SetAlign( GUI_TA_LEFT | GUI_TA_VCENTER );
      mpLabelCancel->SetStringId( SID_CANCEL );
      mpLabelCancel->SetVisible();

      mpLabelContinue = new Label();
      mpLabelContinue->SetLeftMargin(4);
      mpLabelContinue->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpLabelContinue->SetAlign( GUI_TA_LEFT | GUI_TA_VCENTER );
      mpLabelContinue->SetStringId( SID_CONTINUE );
      mpLabelContinue->SetVisible();

      mpImgGoForward = new Image();
      mpImgGoForward->SetClientArea(0,0,bmGoOn.XSize-1, bmGoOn.YSize-1);
      mpImgGoForward->SetBitmap(&bmGoOn);
      mpImgGoForward->SetVisible();

      mpImgGoBack = new Image();
      mpImgGoBack->SetClientArea(0,0,bmGoBack.XSize-1, bmGoBack.YSize-1);
      mpImgGoBack->SetBitmap(&bmGoBack);
      mpImgGoBack->SetVisible();

      mpListOptions = new ListView(this);
      mpListOptions->SetRowHeight(30);
      mpListOptions->SetVisible();
      mpListOptions->SetClientArea(2, 126, 233, 186);
      mpListOptions->InsertColumn(0);
      mpListOptions->InsertColumn(1);
      mpListOptions->SetColumnWidth(0,162);
      mpListOptions->SetColumnWidth(1,64);
      
      ListViewItem* pCancel_row = new ListViewItem();
      pCancel_row->InsertItem(0, mpLabelCancel);
      pCancel_row->InsertItem(1, mpImgGoBack);
      mpListOptions->InsertItem(ROW_NUMBER_CANCEL, pCancel_row);

      ListViewItem* pContinue_row = new ListViewItem();
      pContinue_row->InsertItem(0, mpLabelContinue);
      pContinue_row->InsertItem(1, mpImgGoForward);
      mpListOptions->InsertItem(ROW_NUMBER_CONTINUE, pContinue_row);

      mpListOptions->SelectFirstVisibleItem();

      mLegalKeys = MPC_UP_KEY | MPC_DOWN_KEY | MPC_OK_KEY | MPC_ESC_KEY;
      mLedsStatus = UP_LED | DOWN_LED | OK_LED | ESC_LED;
      
      this->AddChild(mpFrame);
      this->AddChild(mpListOptions);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * NB: never called... never tested 
    *****************************************************************************/   
    CancelOrContinuePopUp::~CancelOrContinuePopUp()
    {
      delete mpFrame;
      delete mpLabelQuestion;
      delete mpListOptions->GetListViewItem(0);
      delete mpListOptions->GetListViewItem(1);
      delete mpListOptions;
      delete mpLabelCancel;
      delete mpLabelContinue;
      delete mpImgGoForward;
      delete mpImgGoBack;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    void CancelOrContinuePopUp::SetQuestionStringId(STRING_ID question)
    {
      mpLabelQuestion->SetStringId(question);
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    void CancelOrContinuePopUp::SetValueToSet(int value)
    {
      mValue = value;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    bool CancelOrContinuePopUp::HandleKeyEvent(Keys KeyID)
    {
      switch(KeyID)
      {
        case MPC_UP_KEY:
        case MPC_DOWN_KEY:
          mpListOptions->HandleKeyEvent(KeyID);
          break;
        case MPC_ESC_KEY:
          HidePopup();
          break;
        case MPC_OK_KEY :

          if (mpListOptions->GetSelection() == ROW_NUMBER_CONTINUE)
          {
            if ( mDpDestination.IsValid())
            {
              mDpDestination->SetAsInt( mValue );             
            }
            else
            {
              FatalErrorOccured("Continue MsgBow: destination DP not set");
            } 
          }
          HidePopup();
          break;
      }

      return true;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * 
    *****************************************************************************/   
    void CancelOrContinuePopUp::HidePopup()
    {
      mpListOptions->SelectFirstVisibleItem();
      this->SetVisible(false);
      DisplayController::GetInstance()->PopPopupBox();
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to set the subject pointer (used by class
    * factory)
    *****************************************************************************/
    void CancelOrContinuePopUp::SetDestinationSubject(Subject* pSubject)
    {
      mDpDestination.Attach(pSubject);
    }

  } // namespace display
} // namespace mpc
