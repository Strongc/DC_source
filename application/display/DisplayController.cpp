/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: DC                                               */
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
/* CLASS NAME       : DisplayController                                     */
/*                                                                          */
/* FILE NAME        : DisplayController.cpp                                 */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef __PC__
#include <windows.h>
#include <atlconv.h>
#endif

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Factory.h>
#include <AppTypeDefs.h>
#include <ctrl\ServiceModeEnabled.h>

 /*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <DisplayController.h>
#include "Languages.h"
#include "pwm_drv.h"
#include "gpio.h"
#include "keyboard.h"
#include <UTF16-UTF8.h>
#include <MenuBar.h>
#include <Display.h>
#include <PopupBox.h>
#include <LowerStatusLine.h>
#include <UpperStatusLine.h>


/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define BACKLIGHT_COUNTER_TIMEOUT_VALUE (15*60*10)  // 15 minutes

#define BACKLIGHT_PWM_VALUE_CHANGE 24
#define BACKLIGHT_ON_PWM_VALUE 0
#define BACKLIGHT_OFF_PWM_VALUE 0xffff
#define BACKLIGHT_REDUCED_PWM_VALUE 3500

#define CONTRAST_COUNTER_RESET_VALUE 0
#define CONTRAST_COUNTER_TIMEOUT_VALUE 20

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmTestScreen1;
extern "C" GUI_CONST_STORAGE GUI_BITMAP bmTestScreen2;


/* --------------------------------------------------
* Memory leakage test -
* Define __MEMORY_LEAKAGE_TEST___ in the project
* to enable this test
* --------------------------------------------------*/
#ifdef __MEMORY_LEAKAGE_TEST___
#define __KEY__SIMULATION__
#endif

#ifdef __KEY__SIMULATION__
#define KEY_SIMULATION_FACTOR 5 // How often the display task should simulate a key press
#ifdef __MEMORY_LEAKAGE_TEST___
int key_simulation_data[] =
{ // Key array for memory test
  GUI_KEY_RIGHT
};
/*{ // Key array for MPC memory test
  23,13,19,13,27,19,19,19,13,27,27,63,27,18,13,43,43,43,43,43,13,18,13,13,27,18,17,13,19,
  19,13,19,13,27,17,17,27,19,23,18,13,45,45,45,45,45,13,63,13,23,18,18,13,13,27,18,23
};*/
#else
int key_simulation_data[] =
{
  GUI_KEY_RIGHT
};
#endif

bool key_simulation_enabled = false;
int key_simulation_counter = 0;
int key_simulation_index = 0;
int home_key_count = 0;
int forced_display_id = 0;
#endif // __KEY__SIMULATION__

#ifdef __PC__
bool g_is_calculating_strings = false;
bool g_show_string_ids = false;
#endif // __PC__

static bool s_contrast_key_pressed = false;

namespace mpc
{
  namespace display
  {
    using namespace ctrl;

    DisplayController* DisplayController::mInstance = 0;

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
    *****************************************************************************/
    DisplayController* DisplayController::GetInstance()
    {
      if (!mInstance)
      {
        mInstance = new DisplayController();
      }
      return mInstance;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    bool DisplayController::SetMenuBar(MenuBar*  MenuBar)
    {
      mpMenuBar = MenuBar;
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    MenuBar* DisplayController::GetMenuBar()
    {
      return mpMenuBar;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    LowerStatusLine*  DisplayController::GetLowerStatusLine()
    {
      return mpLowerStatusLine;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    UpperStatusLine*  DisplayController::GetUpperStatusLine()
    {
      return mpUpperStatusLine;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called once by factory.
    *****************************************************************************/
    bool DisplayController::SetWizardDisplay(Display*  pWizardDisplay)
    {
      mpWizardDisplay = pWizardDisplay;
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Resets the stack and activates the display given.
    *****************************************************************************/
    bool DisplayController::ResetTo(Display* pDisp)
    {
      #ifdef  __PC__
      if (mpCurrentDisplay == pDisp)
      {
        // return to avoid recursive calls from menubar (menutab)
        return false;
      }
      #endif

      mDisplayStack.erase(mDisplayStack.begin(),mDisplayStack.end()); // clear without resize

      #ifdef  __PC__
      bool tab_changed = (mpCurrentDisplay != NULL
        && mpCurrentDisplay->GetDisplayNumber()[0] != pDisp->GetDisplayNumber()[0]);
      #endif

      ChangeTo(pDisp);

      #ifdef  __PC__
      if(tab_changed)
      {
        // update menubar
        switch (pDisp->GetDisplayNumber()[0])
        {
          case '1': mpMenuBar->SetActiveTab(0); break;
          case '2': mpMenuBar->SetActiveTab(1); break;
          case '3': mpMenuBar->SetActiveTab(2); break;
          case '4': mpMenuBar->SetActiveTab(3); break;
          default : break; //ignore
        }
        // need call ChangeTo again, because MenuBar resets current display
        ChangeTo(pDisp);
      }
      #endif

      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Resets the stack and activates the display given.
    *****************************************************************************/
    bool DisplayController::ResetToHome(void)
    {
      if (mpCurrentPopupBox != NULL)
      {
         mpCurrentPopupBox->HandleKeyEvent(MPC_ESC_KEY);
      }

      Component* p_comp = mpCurrentDisplay->GetRoot()->GetCurrentChild();
      if(p_comp != NULL)
      {
        p_comp->CancelEdit();
      }

      mDisplayStack.erase(mDisplayStack.begin(),mDisplayStack.end()); // clear without resize

      if (mpMenuBar != NULL)
      {
        mpMenuBar->SetActiveTab(0);
        ResetTo(mpMenuBar->GetCurrentChild()->GetDisplay());
      }
      mShowOperationMenuPassword = true;
      mShowSettingsMenuPassword = true;

      return true;
    }

	/*****************************************************************************
	* Function - Push
	* DESCRIPTION:
	* Steps into the display given, pushing the current
	* display onto the stack.
	*****************************************************************************/
    bool DisplayController::Push(Display* nextDisplay)
    {
      if (nextDisplay != NULL)
      {
        // If a display pushed on the stack already exists in the stack, the
        // stack should be thruncated at the first instance of the display.
        std::vector<Display*>::iterator iter = std::find(mDisplayStack.begin(), mDisplayStack.end(), nextDisplay);
        if(iter < mDisplayStack.end())
        {
          mDisplayStack.erase(iter, mDisplayStack.end());
        }
        else
        {
          mDisplayStack.push_back(mpCurrentDisplay);
        }
        ChangeTo(nextDisplay);
        return true;
      }
      return false;
    }

    /* --------------------------------------------------
    * Steps out of the current display, pops the stack
    * --------------------------------------------------*/
    bool DisplayController::Pop()
    {
      if(mDisplayStack.size() > 0)
      {
        Display* p_prev = mDisplayStack.back();
        mDisplayStack.pop_back();
        ChangeTo(p_prev);
        return true;
      }
      return false;
    }

    /* --------------------------------------------------
    * Adds a new PopupBox, at the top of the display and
    * other PopupBoxes.
    * --------------------------------------------------*/
    bool DisplayController::PushPopupBox(PopupBox* pPopupBox)
    {
      mPopupBoxStack.push_back(pPopupBox);
      mpCurrentPopupBox = mPopupBoxStack.back();
      return mpCurrentPopupBox == pPopupBox;
    }

    /* --------------------------------------------------
    * Removes the popup box at the top. Returning the
    * control to the next PopupBox in the PopupBox stack
    * or the Display at the Display stack.
    * --------------------------------------------------*/
    bool DisplayController::PopPopupBox()
    {
      if (mPopupBoxStack.size() > 0)
      {
        mPopupBoxStack.pop_back();
      }
      if (mPopupBoxStack.size() == 0)
      {
        mpCurrentPopupBox = NULL;
        mpPasswordMessageBox = NULL;
        mpCurrentDisplay->GetRoot()->Invalidate();
        mpUpperStatusLine->Invalidate();
        mpLowerStatusLine->Invalidate();
        mpMenuBar->Invalidate();
        WM_SelectWindow(WM_GetDesktopWindow());

        return false;
      }
      else
      {
        mpCurrentPopupBox = mPopupBoxStack.back();
        mpCurrentPopupBox->Invalidate();
      }
      return true;
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Update is part of the observer pattern
    *****************************************************************************/
    void DisplayController::Update(Subject* pSubject)
    {
      mpContrastPwmValue.Update(pSubject);
      mpBacklightPwmValue.Update(pSubject);
      mpOperationMenuPasswordValue.Update(pSubject);
      mpSettingsMenuPasswordValue.Update(pSubject);
      mpOperationMenuPasswordEnabled.Update(pSubject);
      mpSettingsMenuPasswordEnabled.Update(pSubject);
      mpCurrentLanguage.Update(pSubject);
      mpWizardEnabled.Update(pSubject);
   }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called if subscription shall be canceled
    *****************************************************************************/
    void  DisplayController::SubscribtionCancelled(Subject* pSubject)
    {
      mpBacklightPwmValue.Detach(pSubject);
      mpContrastPwmValue.Detach(pSubject);
      mpOperationMenuPasswordValue.Detach(pSubject);
      mpSettingsMenuPasswordValue.Detach(pSubject);
      mpSettingsMenuPasswordEnabled.Detach(pSubject);
      mpOperationMenuPasswordEnabled.Detach(pSubject);
      mpCurrentLanguage.Detach(pSubject);
      mpWizardEnabled.Detach(pSubject);
      mWizardIsRunning = false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to set the subject pointer (used by class factory)
    *****************************************************************************/
    void DisplayController::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch (id)
      {
        case SP_DC_CONTRAST:
          mpContrastPwmValue.Attach(pSubject);
          break;
        case SP_DC_BACKLIGHT:
          mpBacklightPwmValue.Attach(pSubject);
          break;
        case SP_DC_PASSWORD_OPERATION_MENU_ENABLE:
          mpOperationMenuPasswordEnabled.Attach(pSubject);
          break;
        case SP_DC_PASSWORD_OPERATION_MENU_VALUE:
          mpOperationMenuPasswordValue.Attach(pSubject);
          break;
        case SP_DC_PASSWORD_SETTINGS_MENU_ENABLE:
          mpSettingsMenuPasswordEnabled.Attach(pSubject);
          break;
        case SP_DC_PASSWORD_SETTINGS_MENU_VALUE:
          mpSettingsMenuPasswordValue.Attach(pSubject);
          break;
        case SP_DC_RUN_WIZARD:
          mpWizardEnabled.Attach(pSubject);
          break;
        case SP_DC_WIZARD_STACK:
          mpWizardIdValues.Attach(pSubject);
          break;
        case SP_DC_IS_POWERED_BY_BATTERY:
          mpIsPoweredByBattery.Attach(pSubject);
          break;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called to indicate that subscription kan be made
    *****************************************************************************/
    void DisplayController::ConnectToSubjects(void)
    {
      mpContrastPwmValue.Subscribe(this);
      mpContrastPwmValue.SetUpdated();

      mpBacklightPwmValue.Subscribe(this);
      mpBacklightPwmValue.SetUpdated();

      mpOperationMenuPasswordValue.Subscribe(this);
      mpOperationMenuPasswordValue.SetUpdated();

      mpSettingsMenuPasswordValue.Subscribe(this);
      mpSettingsMenuPasswordValue.SetUpdated();

      mpOperationMenuPasswordEnabled.Subscribe(this);
      mpOperationMenuPasswordEnabled.SetUpdated();

      mpSettingsMenuPasswordEnabled.Subscribe(this);
      mpSettingsMenuPasswordEnabled.SetUpdated();

      mpCurrentLanguage.Attach(Languages::GetInstance());
      mpCurrentLanguage.Subscribe(this);

      mpWizardEnabled.Subscribe(this);
      mpWizardEnabled.SetUpdated();

      mpLowerStatusLine->ConnectToSubjects();
      mpUpperStatusLine->ConnectToSubjects();
    }

    
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Leds DisplayController::GetLedsStatus(void)
    {
      Leds leds = 0;

      if (mScreenSaverActive)
      {
        leds = NO_LED;
      }
      else if(mpCurrentPopupBox != NULL)
      {
        leds = mpCurrentPopupBox->GetLedsStatus();
      }
      else
      {
        leds = mpCurrentDisplay->GetLedsStatus();
        if (mWizardIsRunning)  // when running the Wizard you can't get out
        {
          SetDenyFlags(leds,MENU_LED | HOME_LED);
        }
        else
        {
          SetAllowFlags(leds,MENU_LED | HOME_LED);
        }

        if(mDisplayStack.size() > 0)
        {
          SetAllowFlags(leds, ESC_LED);
        }
      }
      return leds;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Keys DisplayController::GetLegalkeys(void)
    {
      if(mpCurrentPopupBox != NULL)
      {
        return mpCurrentPopupBox->GetLegalKeys();
      }
      else
      {
        Keys  legal_keys = mpCurrentDisplay->GetLegalKeys();
        if(mDisplayStack.size() > 0)
          SetAllowFlags(legal_keys,MPC_ESC_KEY);
        return legal_keys;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * A key (with a key id) has been pressed
    *****************************************************************************/
    void DisplayController::KeyEvent(KeyId Key)
    {
      Keys legal_keys;

      if (mScreenSaverActive)
      {
        return;
      }
      else if(mpCurrentPopupBox != NULL)
      {
        legal_keys = mpCurrentPopupBox->GetLegalKeys();
        if (IsFlagInSet(legal_keys,Key))
        {
          mpCurrentPopupBox->HandleKeyEvent(Key);
        }
      }
      else
      {
        if(Key == MPC_ESC_KEY && mDisplayStack.size() > 0 && !mpCurrentDisplay->GetRoot()->IsInEditMode())
        {
          Pop();
          return;
        }
        legal_keys = mpCurrentDisplay->GetLegalKeys();

        if (IsFlagInSet(legal_keys,Key))
        {
          mpCurrentDisplay->HandleKeyEvent(Key);
        }

        if (!mWizardIsRunning)  // when running the wizard you can't get out
        {
          legal_keys |= mpMenuBar->GetLegalKeys();
          if (IsFlagInSet(legal_keys,Key))
          {
            mpMenuBar->HandleKeyEvent(Key);
          }
        }
      }

      if (Key == MPC_CONTRAST_KEY
        || Key == MPC_EX_CONTRAST_KEY)
      {
        if (mpContrastScreen != NULL)
        {
          mContrastCounter = CONTRAST_COUNTER_TIMEOUT_VALUE;
          s_contrast_key_pressed = false;
        }
        else
        {
          s_contrast_key_pressed = true;
        }
      }
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called by operating system to give time to do things
    *****************************************************************************/
    void DisplayController::Run(void)
    {
      int key;

      GUI_LOCK();

      RunWizard();

      ApplyContrastAndBacklightValues();

      if (mpCurrentLanguage.IsUpdated())
      {
        UpdateTitle();
      }

      if (mBacklightCounter <= BACKLIGHT_COUNTER_TIMEOUT_VALUE)
      {
        mBacklightCounter++;
      }

      HandleProductionTestScreens();

      #ifdef __KEY__SIMULATION__
      RunKeySimulation();
      #endif

      // Get first key from key buffer.
      if ((key = GUI_GetKey()) != 0)
      {
        #ifdef __KEY__SIMULATION__
        HandleSimulatedKey(key);
        #endif

        mContrastCounter = CONTRAST_COUNTER_RESET_VALUE;
        mBacklightCounter = 0;

        HandlePcKeyPress((char) (key & 0xff));


        KeyEvent(Int2KeyId(key));

        // Empty key buffer
        while ((key = GUI_GetKey()) != 0);
      }

      if (s_contrast_key_pressed)
      {
        s_contrast_key_pressed = false;

        mContrastCounter = CONTRAST_COUNTER_RESET_VALUE;
        if (mpContrastScreen == NULL)
        {
          ShowContrastScreen();
        }
      }
      else if ((mContrastCounter < CONTRAST_COUNTER_TIMEOUT_VALUE) && (mpContrastScreen))
      {
        mContrastCounter++;
      }
      else if ((mContrastCounter == CONTRAST_COUNTER_TIMEOUT_VALUE) && (mpContrastScreen))
      {
        mContrastCounter++;
        HideContrastScreen();
      }

      if (mBacklightCounter > BACKLIGHT_COUNTER_TIMEOUT_VALUE)
      {
        mBacklightCounterIsOver = true;

        if (mpIsPoweredByBattery->GetAsBool())
        {
          mScreenSaverActive = true;
        }

        if (mpBacklightPwmValue->GetValue() < BACKLIGHT_REDUCED_PWM_VALUE)
        {
          mpBacklightPwmValue->SetValue(mpBacklightPwmValue->GetValue() + BACKLIGHT_PWM_VALUE_CHANGE);
        }
        else if (mScreenSaverActive)
        {
          mpBacklightPwmValue->SetValue(BACKLIGHT_OFF_PWM_VALUE);
        }

        mBacklightCounter = BACKLIGHT_COUNTER_TIMEOUT_VALUE;

        if (mHomeReset && mWizardState != WIZARD_RUNNING)
        {
          ServiceModeEnabled::GetInstance()->SetValue(false);
          ResetToHome();
          mHomeReset = false;
        }
      }
      else if (mBacklightCounter == 0)
      {
        mScreenSaverActive = false;

#ifdef TFT_16_BIT_LCD
        if (mBacklightCounterIsOver)
	      {
			    // turn back light on
          mpBacklightPwmValue->SetValue(BACKLIGHT_ON_PWM_VALUE);
			    mBacklightCounterIsOver = false;
	      }
#else
		    mpBacklightPwmValue->SetValue(BACKLIGHT_ON_PWM_VALUE);
#endif
        mHomeReset = true;
      }

      
      if (mTitleUpdateRequested)
      {
        UpdateTitle();
      }

      mpMenuBar->Run();
      mpUpperStatusLine->Run();
      mpLowerStatusLine->Run();
      mpCurrentDisplay->Run();

      if (mpCurrentPopupBox)
      {
        mpCurrentPopupBox->Run();
      }

      SetLeds(GetLedsStatus());

      if (mDisplayIdRequested > 0)
      {
        Push(GetDisplay(mDisplayIdRequested));

        mDisplayIdRequested = 0;
      }
      GUI_UNLOCK();
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    Display*  DisplayController::GetCurrentDisplay()
    {
      return mpCurrentDisplay;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    PopupBox*  DisplayController::GetCurrentPopupBox()
    {
      return mpCurrentPopupBox;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::RequestTitleUpdate()
    {
      mTitleUpdateRequested = true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::RequestDisplayChange(U16 displayId)
    {
      mDisplayIdRequested = displayId;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::EnableWizard(bool enabled)
    {
      if (enabled)
      {
        mpWizardEnabled->SetValue(true);
      }
      else
      {
        mWizardState = WIZARD_ENDING;
      }
    }


    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    ****************************************************************************/

    /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/
    DisplayController::DisplayController()
    {
      mBacklightCounter = 0;
      mScreenSaverActive = false;
      mpCurrentDisplay = 0;
      mpUpperStatusLine = new UpperStatusLine();
      mpLowerStatusLine = new LowerStatusLine();
      mpMenuBar = NULL;
      mpCurrentPopupBox = NULL;
      mTestScreen = NORMAL;
      mTestScreenEvent = false;
      mWizardIsRunning = false;
      mWizardState = WIZARD_NOT_RUNNING;
      mpWizardDisplay = NULL;
      mpTestScreen = NULL;

      mpContrastScreen = NULL;
      mContrastCounter = CONTRAST_COUNTER_TIMEOUT_VALUE;
      mHomeReset = true;

      mShowOperationMenuPassword = true;
      mShowSettingsMenuPassword = true;

      mpPasswordMessageBox = NULL;

      mpWizardLabel = new Label();
      mpWizardLabel->SetAlign(GUI_TA_HCENTER + GUI_TA_VCENTER);
      mpWizardLabel->SetLeftMargin(0);
      mpWizardLabel->SetRightMargin(0);
      mpWizardLabel->SetClientArea(0,0,239,17);
      mpWizardLabel->SetStringId(SID_STEP_BY_STEP_INSTALLATION_GUIDE);
      mpWizardLabel->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpWizardLabel->SetVisible(false);

      mTitleUpdateRequested = false;
      mDisplayIdRequested = 0;

      mDisplayStack.reserve(20);  // To avoid alloc/dealloc

      mBacklightCounterIsOver = false;
    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    DisplayController::~DisplayController()
    {
    }

    
    /*****************************************************************************
    * Function - ApplyContrastAndBacklightValues
    * DESCRIPTION:
    *****************************************************************************/
    void DisplayController::ApplyContrastAndBacklightValues()
    {
#ifndef __PC__
      if (mpContrastPwmValue.IsUpdated())
      {
        SetDutyCycleHW_2(mpContrastPwmValue->GetValue());
      }
      if (mpBacklightPwmValue.IsUpdated())
      {
        SetDutyCycleHW_1(mpBacklightPwmValue->GetValue());
      }
#endif
    }
    
    /*****************************************************************************
    * Function - RunWizard
    * DESCRIPTION:
    *****************************************************************************/
    void DisplayController::RunWizard()
    {
      if (mpWizardEnabled.IsUpdated())
      {
        mWizardIsRunning = true;
        if (mpWizardEnabled->GetValue())
        {
          mWizardState = WIZARD_STARTING;
        }
        else
        {
          mWizardState = WIZARD_ENDING;
        }
      }

      if (mWizardIsRunning)
      {
        switch (mWizardState)
        {
          case WIZARD_NOT_RUNNING :               // Nothing
            break;

          case WIZARD_STARTING :
            mpMenuBar->SetVisible(false);
            mpWizardLabel->Invalidate();
            mpWizardLabel->SetVisible(true);
            WM_BringToTop(mpWizardLabel->GetWMHandle());
            mpWizardLabel->Run();

            if (mpWizardDisplay != NULL)
            {
              // Restore wizard display stack if any
              if(mpWizardIdValues->GetValue(0) > 0)
              {
                mDisplayStack.erase(mDisplayStack.begin(),mDisplayStack.end());
                const int count = (int)(mpWizardIdValues->GetSize() - 1);
                int i=0;
                for(;i < count;++i)
                {
                  if(mpWizardIdValues->GetValue(i+1) == 0)
                  {
                    ChangeTo(GetDisplay(mpWizardIdValues->GetValue(i)));
                    break;
                  }
                  mDisplayStack.push_back(GetDisplay(mpWizardIdValues->GetValue(i)));
                }
              }
              else
              {
                mpWizardIdValues->SetValue(0, mpWizardDisplay->GetId());
                mpWizardIdValues->SetValue(1, 0);
                ResetTo(mpWizardDisplay);
              }

            }
            mWizardState = WIZARD_RUNNING;
            break;
          case WIZARD_RUNNING  :
            mpWizardLabel->Run();
            break;
          case WIZARD_ENDING   :

            mpWizardIdValues->SetValue(0, 0);
            mpWizardLabel->SetVisible(false);
            mpMenuBar->SetVisible();
            mpMenuBar->SetActiveTab(0);
            ResetTo(mpMenuBar->GetCurrentChild()->GetDisplay());

            mWizardState = WIZARD_NOT_RUNNING;
            mWizardIsRunning = false;

            ResetToHome();
        }
      }
    }

    #ifdef __KEY__SIMULATION__
    /*****************************************************************************
    * Function - RunKeySimulation
    * DESCRIPTION:
    *****************************************************************************/
    void DisplayController::RunKeySimulation()
    {
      key_simulation_counter++;
      if (key_simulation_enabled && key_simulation_counter >= KEY_SIMULATION_FACTOR)
      {
        key_simulation_counter = 0;
        if (forced_display_id > 0) // Force a walk through all displays
        {
          Display* p_disp = NULL;
          while (p_disp == NULL && forced_display_id > 0)
          {
            forced_display_id--;
            p_disp = GetDisplay(forced_display_id);
          }
          if(p_disp != NULL)
          {
            ResetTo(p_disp);
            key_simulation_index = 100; // Skip key simulation while running through displays
          }
        }
        if (key_simulation_index >= sizeof(key_simulation_data)/sizeof(key_simulation_data[0]))
        {
          key_simulation_index = 0;
        }
        else
        {
          GUI_StoreKey(key_simulation_data[key_simulation_index]);
          key_simulation_index++;
        }
      }
    }
    /*****************************************************************************
    * Function - HandleSimulatedKey
    * DESCRIPTION:
    *****************************************************************************/
    void DisplayController::HandleSimulatedKey(char key)
    {
      // Handle enable/disable of key simulation
      if (key == GUI_KEY_HOME && key_simulation_counter <= 4)
      {
        home_key_count++;
        if (home_key_count == 3) // 3 x home key fast, enable key simulation
        {
          key_simulation_enabled = true;
          forced_display_id = MAX_DISPLAY_ID;
        }
      }
      else
      {
        home_key_count = 0;
        if (key == GUI_KEY_HOME)
        {
          home_key_count++;
        }
        if (key == 99) // contrast key, disable key simulation
        {
          key_simulation_enabled = false;
        }
      }
      key_simulation_counter = 0;
    }
    #endif //__KEY__SIMULATION__

    /*****************************************************************************
    * Function - HandlePcKeyPress
    * DESCRIPTION:
    *****************************************************************************/
    void DisplayController::HandlePcKeyPress(char key)
    {
#ifdef __PC__
      switch(key)
      {
      case 'a': // show [a]ll
        // toogle Care/don't care of available
        g_is_calculating_strings = !g_is_calculating_strings;
        break;
      case 'p': // dump lcd screen (single)
        DisplayDumper::GetInstance()->DumpScreen(false, true, 0, true);
        break;
      case 'P': // dump lcd screen (all)
        DisplayDumper::GetInstance()->DumpScreens(true, false);
        break;
      case 'f': // dump [f]ull screen (single)
        DisplayDumper::GetInstance()->DumpScreen(false, false);
        break;
      case 'F': // dump [F]ull screen (all)
        DisplayDumper::GetInstance()->DumpScreens(false, false);
        break;
      case 'H': // dump [H]elp boxes
        DisplayDumper::GetInstance()->DumpHelpBoxes();
        break;
      case 'L': // dump [L]engths of strings of current language
        StringWidthCalculator::GetInstance()->ExportStringWidths("StringLengths.csv");
        break;
      case 'r': // [r]efresh current display
      case 'R':
        mpMenuBar->Invalidate();
        mpUpperStatusLine->Invalidate();
        mpCurrentDisplay->GetRoot()->Invalidate();
        break;
      case 's': // toggle [s]tring id's with strings
        g_show_string_ids = !g_show_string_ids;
        mpMenuBar->Invalidate();
        mpUpperStatusLine->Invalidate();
        mpCurrentDisplay->GetRoot()->Invalidate();
        break;
      case 'm': // [m]emory check 1
         _CrtDumpMemoryLeaks();
        break;
      case 'M': // [M]emory check 2
        {
          char *buffer;

          // Allocate and deallocate some memory
          if((buffer = (char *)malloc(100)) != NULL)
              free(buffer);

          int heapstatus = _heapchk();
          switch(heapstatus)
          {
          case _HEAPOK:
            OutputDebugStringA(" OK - heap is fine\n");
            break;
          case _HEAPEMPTY:
            OutputDebugStringA(" OK - heap is empty\n");
            break;
          case _HEAPBADBEGIN:
            Beep(1000, 400);
            FatalErrorOccured("ERROR - bad start of heap\n");
            break;
          case _HEAPBADNODE:
            Beep(1000, 400);
            FatalErrorOccured("ERROR - bad node in heap\n");
            break;
          }
        }
      break;
      }
#endif // __PC__
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::HandleProductionTestScreens(void)
    {
      if (mTestScreenEvent)
      {
        ShowTestScreen(mTestScreen);

        mTestScreenEvent = false;
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::ShowTestScreen(TEST_SCREEN_TYPE number)
    {
      PopPopupBox();

      if (mpTestScreen != NULL)
      {
        Component* p_image = mpTestScreen->GetCurrentChild();
        mpTestScreen->RemoveChild(p_image);
        delete p_image;
        delete mpTestScreen;
        mpTestScreen = NULL;
      }

      if (mTestScreen != NORMAL)
      {
        mpTestScreen = new PopupBox();
        mpTestScreen->SetClientArea(0,0,239,319);
        Image* test_screen_image = new Image();

        if (number == TEST_SCREEN_1)
          test_screen_image->SetBitmap(&bmTestScreen1);
        else
          test_screen_image->SetBitmap(&bmTestScreen2);

        test_screen_image->SetClientArea(0,0,239,319);
        test_screen_image->SetVisible();
        test_screen_image->Invalidate();
        test_screen_image->SetParent(mpTestScreen);
        mpTestScreen->AddChild(test_screen_image);

        PushPopupBox(mpTestScreen);
        mpTestScreen->Invalidate();
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::ShowContrastScreen(void)
    {
       if (mpContrastScreen == NULL)
       {
         mpContrastScreen = new ContrastMessageBox();
         /*Changes for display of brightness screen i.s.o. contrast*/
#ifndef TFT_16_BIT_LCD
	       mpContrastScreen->SetSubjectPointer(-1, mpContrastPwmValue.GetSubject());
#else
	       mpContrastScreen->SetSubjectPointer(-1, mpBacklightPwmValue.GetSubject());
#endif
         mpContrastScreen->ConnectToSubjects();
         PushPopupBox(mpContrastScreen);
       }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::HideContrastScreen(void)
    {
       PopPopupBox();
       delete mpContrastScreen;
       mpContrastScreen = NULL;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::ShowPasswordMessageBoxNo(int pw_no)
    {
      if (mpPasswordMessageBox == NULL)
      {
        mpPasswordMessageBox = new PasswordMessageBox();
        if (pw_no == 1)
        {
          mpPasswordMessageBox->SetPw(mpOperationMenuPasswordValue->GetValue());
          mpPasswordMessageBox->SetPwType(0);
        }
        else if (pw_no == 2)
        {
          mpPasswordMessageBox->SetPw(mpSettingsMenuPasswordValue->GetValue());
          mpPasswordMessageBox->SetPwType(1);
        }

        PushPopupBox(mpPasswordMessageBox);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayController::UpdateTitle()
    {
      const int len = GetTitleText(NULL, 0);
      char* szText = new char[len];
      GetTitleText(szText, len);
      mpUpperStatusLine->SetText(szText);
      delete[] szText;

      mTitleUpdateRequested = false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *****************************************************************************/
    int DisplayController::GetTitleText(char* szText, int maxLen)
    {
      bool show_string_id = false;

      #ifdef __PC__
      show_string_id = g_show_string_ids;
      #endif

      int needed = 4; // 4 is length of " - " + zero termination

      if (szText != NULL && maxLen > 0)
      {
        szText[0] = '\0';
      }

      const char* display_number = mpCurrentDisplay->GetDisplayNumber();
      STRING_ID title_sid = mpCurrentDisplay->GetName();

      needed += (int) strlen(display_number);

      if (show_string_id)
      {
        needed += 4; // need chars for string id of up to 4 digits
      }
      else
      {
        if (title_sid != SID_NONE)
        {
          needed += (int) strlen(Languages::GetInstance()->GetString(title_sid));
        }
        else
        {
          needed += (int) strlen(mpCurrentDisplay->GetTitleText());
        }
      }

      if (needed <= maxLen && szText != NULL)
      {
        if (show_string_id)
        {
          sprintf(szText, "%s - %d", display_number, title_sid);
        }
        else
        {
          const char* p_title;

          if (title_sid != SID_NONE)
          {
            p_title = Languages::GetInstance()->GetString(title_sid);
          }
          else
          {
            p_title = mpCurrentDisplay->GetTitleText();
          }
          
          sprintf(szText, "%s - %s", display_number, p_title);
        }
      }
      if (needed > maxLen || szText == NULL)
      {
        return needed;
      }
      return 0;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Change the current display to the display given
    *****************************************************************************/
    bool DisplayController::ChangeTo(Display* NewDisplay)
    {
      bool is_calculating_string_lengths = false;
      #ifdef __PC__
      is_calculating_string_lengths = g_is_calculating_strings;
      #endif

      if (mpCurrentDisplay != NULL)
      {
        mpCurrentDisplay->Hide();
      }
      mpCurrentDisplay = NewDisplay;

      /*** Update flash with new wizard ids ***/
      if (mWizardState == WIZARD_RUNNING)
      {
        if (mpWizardIdValues.IsValid())
        {
          mpWizardIdValues->SetValue((int)(mDisplayStack.size()), mpCurrentDisplay->GetId());
          mpWizardIdValues->SetValue((int)(mDisplayStack.size() + 1), 0);
        }
      }

      /*** Test for password ***/
      int pw_id = mpCurrentDisplay->GetPasswordId();

      if (is_calculating_string_lengths)
      { // avoid all password protection when calculating strings widths on pc
        pw_id = 0;
      }

      if (pw_id != 0)
      {
        if ((pw_id == 1) && (mpOperationMenuPasswordEnabled->GetValue()) && (mShowOperationMenuPassword))
        {
          ShowPasswordMessageBoxNo(pw_id);
        }
        else if ((pw_id == 2) && (mpSettingsMenuPasswordEnabled->GetValue()) && (mShowSettingsMenuPassword))
        {
          ShowPasswordMessageBoxNo(pw_id);
        }
      }

      /*** Display the display ***/
      UpdateTitle();
      mpCurrentDisplay->Show();
      return true;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called by GENI to show test screen 1, 2 or normal
    *****************************************************************************/
    void DisplayController::TestScreen(TEST_SCREEN_TYPE screen)
    {
      mTestScreenEvent = true;
      mTestScreen = screen;
    }

    void DisplayController::TestLoopDisplays()
    {
      //not used anymore
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called by GENI to show test screen 1, 2 or normal
    *****************************************************************************/
    void DisplayController::OperationMenuPasswordPass(bool pass /* = true */)
    {
      mShowOperationMenuPassword = !pass;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Called by GENI to show test screen 1, 2 or normal
    *****************************************************************************/
    void DisplayController::SettingsMenuPasswordPass(bool pass /* = true */)
    {
      mShowSettingsMenuPassword = !pass;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Change the current display to the display given
    *****************************************************************************/
    void DisplayController::SetLeds(Leds leds)
    {
      U32 on  = leds & 0x0000ffff;
      U32 off = ((leds >> 16)) & 0x0000ffff;
      leds = on & ~off;

      GPio* p_gpio = GPio::GetInstance();

      p_gpio->SetAs(GP_IO_LED_MENU,     OUTPUT, (leds & MENU_LED) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_QUESTION, OUTPUT, (leds & HELP_LED) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_UP,       OUTPUT, (leds & UP_LED  ) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_DOWN,     OUTPUT, (leds & DOWN_LED) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_PLUS,     OUTPUT, (leds & PLUS_LED) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_MINUS,    OUTPUT, (leds & MINUS_LED) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_ESC,      OUTPUT, (leds & ESC_LED ) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_OK,       OUTPUT, (leds & OK_LED  ) ? 1 : 0);
      p_gpio->SetAs(GP_IO_LED_HOME,     OUTPUT, (leds & HOME_LED) ? 1 : 0);

    }

    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/


  } // namespace display
} // namespace mpc
