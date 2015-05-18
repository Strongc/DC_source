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
/* CLASS NAME       : DisplayController                                     */
/*                                                                          */
/* FILE NAME        : DisplayController.H                                   */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcDisplayController_h
#define mpcDisplayController_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>
#ifdef __PC__
  #include <stdio.h>
  extern  bool  g_is_calculating_strings;
#endif

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <BoolDataPoint.h>
#include <U8VectorDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "gui.h"
#include <observer.h>
#include <GUI_Utility\leds.h>
#include <GUI_Utility\keys.h>
#include <ContrastMessageBox.h>
#include <PasswordMessageBox.h>
#include <Languages.h>

#ifdef __PC__
  #include <OdbcInterface.h>
  #include <DisplayDumper.h>
  #include <StringWidthCalculator.h>
#endif
/*****************************************************************************
  DEFINES
 *****************************************************************************/
 #define MAX_DISPLAY_ID 200
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
* FOWARD DECLARATIONS
*****************************************************************************/
class Subject;

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * FOWARD DECLARATIONS
    *****************************************************************************/
    class Display;
    class PopupBox;
    class MenuBar;
    class LowerStatusLine;
    class UpperStatusLine;

#ifdef __PC__
    class DisplayDumper;
#endif

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class DisplayController : public Observer
    {
      public:

        typedef  enum {NORMAL = 0, TEST_SCREEN_1, TEST_SCREEN_2} TEST_SCREEN_TYPE;
        
        static DisplayController* GetInstance();

        /********************************************************************
        ASSIGNMENT OPERATOR
        ********************************************************************/
        /********************************************************************
        OPERATIONS
        ********************************************************************/        
        /* --------------------------------------------------
        * Used by DisplayDumper and PasswordMessageBox
        * --------------------------------------------------*/
        virtual MenuBar* GetMenuBar();

        /* --------------------------------------------------
        * Used by DisplayDumper
        * --------------------------------------------------*/
        virtual LowerStatusLine*  GetLowerStatusLine();
        
        /* --------------------------------------------------
        * Used by DisplayDumper
        * --------------------------------------------------*/
        virtual UpperStatusLine*  GetUpperStatusLine();

        /* --------------------------------------------------
        * Used by Factory
        * --------------------------------------------------*/
        virtual bool SetMenuBar(MenuBar*  MenuBar);

        /* --------------------------------------------------
        * Used by Factory
        * --------------------------------------------------*/
        virtual bool SetWizardDisplay(Display*  pWizardDisplay);

        /* --------------------------------------------------
        * Resets the stack.
        * --------------------------------------------------*/
        virtual bool ResetTo(Display* Disp);

        /* --------------------------------------------------
        * Steps into the display given, pushing the current
        * display onto the stack.
        * --------------------------------------------------*/
        virtual bool Push(Display* NextDisplay);

        /* --------------------------------------------------
        * Steps out of the current display, pops the stack
        * --------------------------------------------------*/
        virtual bool Pop();

        /* --------------------------------------------------
        * Used by DisplayDumper
        * --------------------------------------------------*/
        virtual Display*  GetCurrentDisplay();

        /* --------------------------------------------------
        * Used by StringWidthCalculator
        * --------------------------------------------------*/
        virtual PopupBox* GetCurrentPopupBox();

        /* --------------------------------------------------
        * Adds a new PopupBox, at the top of the display and
        * other PopupBoxes.
        * --------------------------------------------------*/
        virtual bool PushPopupBox(PopupBox* pPopupBox);

        /* --------------------------------------------------
        * Removes the popup box at the top. Returning the
        * control to the next PopupBox in the PopupBox stack
        * or the Display at the Display stack.
        * --------------------------------------------------*/
        virtual bool PopPopupBox();

        /* --------------------------------------------------
        * Gets the LEDs to turn on
        * --------------------------------------------------*/
        Leds GetLedsStatus(void);

        /* --------------------------------------------------
        * Gets the legal eys
        * --------------------------------------------------*/
        Keys GetLegalkeys(void);

        /* --------------------------------------------------
        * A key (with a key id) has been pressed
        * --------------------------------------------------*/
        void KeyEvent(KeyId Key);

        /* --------------------------------------------------
        * Update is part of the observer pattern
        * --------------------------------------------------*/
        virtual void Update(Subject* pSubject);
        /* --------------------------------------------------
        * Called if subscription shall be canceled
        * --------------------------------------------------*/
        virtual void SubscribtionCancelled(Subject* pSubject);
        /* --------------------------------------------------
        * Called to set the subject pointer (used by class
        * factory)
        * --------------------------------------------------*/
        virtual void SetSubjectPointer(int Id, Subject* pSubject);
        /* --------------------------------------------------
        * Called to indicate that subscription kan be made
        * --------------------------------------------------*/
        virtual void ConnectToSubjects(void);

        /* --------------------------------------------------
        * Called by operating system to give time to do things
        * --------------------------------------------------*/
        virtual void Run(void);

        /* --------------------------------------------------
        * Called by GENI to show test screen 1, 2 or normal
        * --------------------------------------------------*/
        virtual void TestScreen(TEST_SCREEN_TYPE screen);
        virtual void TestLoopDisplays();

        /* --------------------------------------------------
        * Called to jump to the Status picture
        * --------------------------------------------------*/
        bool ResetToHome(void);

        void OperationMenuPasswordPass(bool pass = true);
        void SettingsMenuPasswordPass(bool pass = true);

        virtual void RequestTitleUpdate();
        virtual void RequestDisplayChange(U16 displayId);

        /* --------------------------------------------------
        * Used by DisplayDumper to force controller in and out of Wizard mode
        * --------------------------------------------------*/
        virtual void EnableWizard(bool enabled);

      private:
        typedef  enum {WIZARD_NOT_RUNNING = 0, WIZARD_STARTING, WIZARD_RUNNING, WIZARD_ENDING} WIZARD_STATE;
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        DisplayController();
        virtual ~DisplayController();

        virtual int GetTitleText(char* szText, int maxLen);

        virtual void UpdateTitle();
        
        /* --------------------------------------------------
        * Change the current display to the display given
        * --------------------------------------------------*/
        bool ChangeTo(Display* NewDisplay);

        void HandleProductionTestScreens(void);
        void ShowTestScreen(TEST_SCREEN_TYPE number);

        void ShowContrastScreen(void);
        void HideContrastScreen(void);

        void ShowPasswordMessageBoxNo(int pw_no);

        void ApplyContrastAndBacklightValues();
        void RunWizard();

  #ifdef __KEY__SIMULATION__
        void RunKeySimulation();
        void HandleSimulatedKey(char key);
  #endif
        void HandlePcKeyPress(char key);
        
        void SetLeds(Leds leds);

        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        static DisplayController* mInstance;

        SubjectPtr<U32DataPoint*> mpContrastPwmValue;           // The contrast value of the LCD-display
        SubjectPtr<U32DataPoint*> mpBacklightPwmValue;          // The value of the backlight
        SubjectPtr<U16DataPoint*> mpOperationMenuPasswordValue;
        SubjectPtr<U16DataPoint*> mpSettingsMenuPasswordValue;
        SubjectPtr<BoolDataPoint*> mpOperationMenuPasswordEnabled;
        SubjectPtr<BoolDataPoint*> mpSettingsMenuPasswordEnabled;
        SubjectPtr<Languages*> mpCurrentLanguage;
        SubjectPtr<BoolDataPoint*> mpWizardEnabled;
        SubjectPtr<U8VectorDataPoint*> mpWizardIdValues;
        SubjectPtr<BoolDataPoint*> mpIsPoweredByBattery;

        bool      mShowOperationMenuPassword;
        bool      mShowSettingsMenuPassword;
        PasswordMessageBox* mpPasswordMessageBox;
        
        bool      mTestScreenEvent;
        TEST_SCREEN_TYPE mTestScreen;
        PopupBox* mpTestScreen;

        bool      mWizardIsRunning;
        WIZARD_STATE mWizardState;
        Label*    mpWizardLabel;
        Display*  mpWizardDisplay;

        ContrastMessageBox* mpContrastScreen;
        int       mContrastCounter;

        int       mBacklightCounter;
		    bool      mBacklightCounterIsOver;

        bool      mScreenSaverActive;
        bool      mHomeReset;

        MenuBar*  mpMenuBar;
        LowerStatusLine* mpLowerStatusLine;
        UpperStatusLine* mpUpperStatusLine;
                
        Display*  mpStatusMenuOverviewPicture;
        Display*  mpAlarmMenuOverviewPicture;
        Display*  mpOperationMenuOverviewPicture;
        Display*  mpSettingsMenuOverviewPicture;
        Display*  mpCurrentDisplay;
        PopupBox* mpCurrentPopupBox;
        std::vector<Display*> mDisplayStack;
        std::vector<PopupBox*> mPopupBoxStack;
        
        bool      mTitleUpdateRequested;
        U16       mDisplayIdRequested;

      protected:
        /********************************************************************
        OPERATIONS
        ********************************************************************/

        /********************************************************************
        ATTRIBUTE
        ********************************************************************/

    };
  }
}

#endif
