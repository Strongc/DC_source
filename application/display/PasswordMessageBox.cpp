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
/* CLASS NAME       : PasswordMessageBox                                    */
/*                                                                          */
/* FILE NAME        : PasswordMessageBox.cpp                                */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "PasswordMessageBox.h"
#include <MpcFonts.h>
#include <DisplayController.h>
#include <DataPoint.h>
#include <Image.h>
#include <Label.h>
#include <MenuBar.h>
#include <MultiNumber.h>
/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmPasswordIcon;


namespace mpc
{
  namespace display
  {

  const int MasterStoredPw = 6814;

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
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    PasswordMessageBox::PasswordMessageBox(Component* pParent) : PopupBox( pParent )
    {
      mTestPw = false;
      SetClientArea(2,34,237,304);

      mpTitle = new Label(this);
      mpTitle->SetStringId(SID_PASSWORD_TITLE);
      mpTitle->SetClientArea(3,3,195,39);
      mpTitle->SetFont(DEFAULT_FONT_18_LANGUAGE_DEP);
      mpTitle->SetVisible();
      mpTitle->Invalidate();
      AddChild(mpTitle);

      mpIconImage = new Image(this);
      mpIconImage->SetClientArea(200,2,231,33);
      mpIconImage->SetBitmap(&bmPasswordIcon);
      mpIconImage->SetVisible();
      AddChild(mpIconImage);

      mpMessage = new Label(this);
      mpMessage->SetStringId(SID_PASSWORD_TEXT);
      mpMessage->SetClientArea(3,46,234,148);
      mpMessage->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      mpMessage->SetWordWrap();
      mpMessage->SetVisible();
      AddChild(mpMessage);
      mPw = 0;
      mPwType = 0;

      mpMultiNumber = new MultiNumber();
      mpMultiNumber->SetClientArea((237/2) - (2 * 16), 170, (237/2)+(2*16), 187);
      mpMultiNumber->SetFieldCount(4, 5);
      mpMultiNumber->SetVisible();
      mpMultiNumber->SetReadOnly(false);
      mpMultiNumber->SetFocus();
      AddChild(mpMultiNumber);

      mpDpMultiNumber = new U32DataPoint();
      mpDpMultiNumber->SetMinValue(0);
      mpDpMultiNumber->SetMaxValue(9999);
      mpDpMultiNumber->SetValue(5555);
      mpDpMultiNumber->SetQuality(DP_AVAILABLE);
      mpDpMultiNumber->SetQuantity(Q_NO_UNIT);

      mpDpMultiNumber->SubscribeE(this);

      mpMultiNumber->SetSubjectPointer(0, mpDpMultiNumber);
      mpMultiNumber->ConnectToSubjects();
      mpMultiNumber->SetFieldMinValue(0);
      mpMultiNumber->SetFieldMaxValue(9);
      
      
      SetReadOnly(false);

      SetCurrentChild(mpMultiNumber);
      SetAllowFlags(mLegalKeys, MPC_HOME_KEY|MPC_MENU_KEY);
      mLedsStatus = HOME_LED|MENU_LED;

      Invalidate();
    }

    PasswordMessageBox::~PasswordMessageBox()
    {

      delete mpIconImage;
      delete mpMessage;
      delete mpTitle;
      
      delete mpDpMultiNumber;
      delete mpMultiNumber;
    }

    bool PasswordMessageBox::HandleKeyEvent(Keys KeyID)
    {
      if(mpMultiNumber->HandleKeyEvent(KeyID))
        return true;

      switch(KeyID)
      {
        case MPC_HOME_KEY:
            DisplayController::GetInstance()->ResetToHome();
            DeleteMessageBox();
            return true;

        case MPC_MENU_KEY:
            DisplayController::GetInstance()->GetMenuBar()->HandleKeyEvent(KeyID);
            DeleteMessageBox();
            return true;
      }

      return false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * The Run method
    *****************************************************************************/
    void PasswordMessageBox::Run()
    {
      PopupBox::Run();
      if(mTestPw)
      {
        mTestPw = false;
        ValidatePassword();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * The Run method
    *****************************************************************************/
    void PasswordMessageBox::ValidatePassword()
    {
      int pw = 0;
      pw = mpDpMultiNumber->GetValue();
      if ( pw == mPw )
      {
        if ( mPwType == 1 )
        {
          DisplayController::GetInstance()->SettingsMenuPasswordPass(true);
        }
        DisplayController::GetInstance()->OperationMenuPasswordPass(true);
        DeleteMessageBox();
      }
      else if ( pw == MasterStoredPw )
      {
        DisplayController::GetInstance()->OperationMenuPasswordPass(true);
        DisplayController::GetInstance()->SettingsMenuPasswordPass(true);
        DeleteMessageBox();
      }
    }
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * The Run method
    *****************************************************************************/
    void PasswordMessageBox::DeleteMessageBox()
    {
      DisplayController::GetInstance()->PopPopupBox();
      delete this;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * The Run method
    *****************************************************************************/
    void PasswordMessageBox::SetPw(int pw)
    {
      mPw = pw;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * The Run method
    *****************************************************************************/
    void PasswordMessageBox::SetPwType(int type)
    {
      mPwType = type;
    }

    void PasswordMessageBox::SubscribtionCancelled(Subject* pSubject)
    {
    }
    
    void PasswordMessageBox::Update(Subject* pSubject)
    {
      mTestPw = true;
    }
    
    void PasswordMessageBox::SetSubjectPointer(int Id,Subject* pSubject)
    {
    }

    void PasswordMessageBox::ConnectToSubjects(void)
    {
    }
  } // namespace display
} // namespace mpc
