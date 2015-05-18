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
/* CLASS NAME       : UpperStatusLine                                       */
/*                                                                          */
/* FILE NAME        : UpperStatusLine.cpp                                   */
/*                                                                          */
/* CREATED DATE     :                                                       */
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
#include <UpperStatusLine.h>
#include "DataPointTime.h"
#include "Subject.h"
#include <AlarmInLog.h>
#include <MPCFonts.h>
#include <ctrl\ServiceModeEnabled.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/
namespace mpc
{
  namespace display
  {
    using namespace ctrl;
    /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/
    UpperStatusLine::UpperStatusLine() : Frame(true,false), Observer()
    {
      mpPath  = new Text();
      mpPath->SetClientArea(1,0,239-13-1-13-1-1,14);
      mpPath->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpPath->SetBackgroundColour(GUI_COLOUR_UPPER_STATUSLINE_BACKGROUND);
      mpPath->SetColour(GUI_COLOUR_UPPER_STATUSLINE_FOREGROUND);
      mpPath->SetAlign(GUI_TA_LEFT | GUI_TA_VCENTER);
      mpPath->SetText("");
      AddChild(mpPath);

      mpAlarmIconState = new AlarmIconState();
      mpAlarmIconState->SetClientArea(239-13,1,238,13);
      mpAlarmIconState->SetSubjectPointer(0,AlarmInLog::GetInstance());
      mpAlarmIconState->ConnectToSubjects();
      AddChild(mpAlarmIconState);

      SetBackgroundColour(GUI_COLOUR_UPPER_STATUSLINE_BACKGROUND);
      SetColour(GUI_COLOUR_UPPER_STATUSLINE_FOREGROUND);
      SetClientArea(0,18,239,32);

      mpServiceBitmap = new ServiceIconState();
      mpServiceBitmap->SetClientArea(239-13-1-13,1,238-13-1,13);
      mpServiceBitmap->SetSubjectPointer(0,ServiceModeEnabled::GetInstance());
      mpServiceBitmap->ConnectToSubjects();
      mpServiceBitmap->Update(ServiceModeEnabled::GetInstance());
      mpServiceBitmap->SetVisible();
      AddChild(mpServiceBitmap);

      SetVisible();
      mpPath->SetVisible();
      mpAlarmIconState->SetVisible();
    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    UpperStatusLine::~UpperStatusLine()
    {
      delete mpPath;
      delete mpAlarmIconState;
    }

    void UpperStatusLine::SetText(const char* string)
    {
      int no_of_chars = strlen(string) + 4;
      char* sz_clone = new char[no_of_chars];
      char* string_to_show = new char[no_of_chars];
      char* p_text = sz_clone;

      no_of_chars--;      
      strncpy(sz_clone, string, no_of_chars);
      sz_clone[no_of_chars] = '\0';

      SelectWindow();
      GUI_SetFont(mpPath->GetFont());
      int txt_len = GUI_GetStringDistX(sz_clone);
      const int max_width = mpPath->GetWidth() - mpPath->GetLeftMargin() - mpPath->GetRightMargin();
      if(txt_len >= max_width)
      {
        const int prefix_len = GUI_GetStringDistX("...");
        while( txt_len >= max_width)
        {
          p_text += GUI_UC_GetCharSize(p_text);
          txt_len = GUI_GetStringDistX(p_text) + prefix_len;
        }

        sprintf(string_to_show, "...%s", p_text);
      }
      else
      {
        sprintf(string_to_show,"%s",sz_clone);
      }
      mpPath->SetText(string_to_show);
      delete[] sz_clone;
      delete[] string_to_show;
      Invalidate();
    }

    void UpperStatusLine::Update(Subject* pSubject)
    {
      // hide the alarm icon state when wizard is running
      if (mDpWizardEnabled.IsValid())
      {
        mpAlarmIconState->SetVisible( !mDpWizardEnabled->GetAsBool() );
      }
    }
    
    void UpperStatusLine::SubscribtionCancelled(Subject* pSubject)
    {
      mDpWizardEnabled.Detach();
    }
    
    void UpperStatusLine::SetSubjectPointer(int Id, Subject* pSubject)
    {
      //ignore
    }
    
    void UpperStatusLine::ConnectToSubjects(void)
    {
      mDpWizardEnabled.Attach( GetSubject(SUBJECT_ID_DISPLAY_WIZARD_ENABLE) );
      mDpWizardEnabled.Subscribe(this);
    }

  #ifdef __PC__
    /*****************************************************************************
    * Function - CalculateStringWidths
    * DESCRIPTION:
    ****************************************************************************/
    void UpperStatusLine::CalculateStringWidths(int stringId)
    {
      mpPath->CalculateStringWidths(false, stringId);
    }
  #endif

    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    ****************************************************************************/

    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/
  }

}

