/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: Platform                                         */
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
/* CLASS NAME       : StringWidthCalculator                                 */
/*                                                                          */
/* FILE NAME        : StringWidthCalculator.cpp                             */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#include <direct.h>
#include <windows.h>
#include <atlconv.h>

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <StringWidthCalculator.h>
#include <ConfirmWrite.h>
#include <HelpMessageBox.h>
#include <MenuBar.h>
#include <UpperStatusLine.h>
#include <LowerStatusLine.h>
#include <UTF16-UTF8.h>


/*****************************************************************************
DEFINES
*****************************************************************************/
#define UNREFERENCED_HELPTEXT "Unreferenced Helptext"

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    static std::map<U16, U16> sStringIdCompWidth;

    static std::map<U16, bool> sHelpStringIdExported;

    StringWidthCalculator* StringWidthCalculator::mInstance = 0;

    /*****************************************************************************
    *
    *
    *              PUBLIC FUNCTIONS
    *
    *
    *****************************************************************************/

    /*****************************************************************************
    * Function - GetInstance
    * DESCRIPTION:
    *
    ****************************************************************************/
    StringWidthCalculator* StringWidthCalculator::GetInstance()
    {
      if (!mInstance)
      {
        mInstance = new StringWidthCalculator();
      }
      return mInstance;
    }

    
    /*****************************************************************************
    * Function - ExportStringWidths
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::ExportStringWidths(const char* filename)
    {
      StringWidthParameters sw_params;
      strcpy(sw_params.filename, filename);
      sw_params.firstcolumn[0] = 0;
      sw_params.includeHeader = true;
      sw_params.onlyRelationsInCurrentDisplay = false;

      ExportStringWidthsAdv(&sw_params);
    }

    /*****************************************************************************
    * Function - ExportStringWidths
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::ExportStringWidthsAdv(StringWidthParameters* parameters)
    {
      if (parameters->includeHeader)
      {//create new file
        mpStringLengthFile = fopen(parameters->filename, "wb");
      }
      else
      {//append to file
        mpStringLengthFile = fopen(parameters->filename, "ab");
      }

      if (mpStringLengthFile == NULL)
      {
        TCHAR sz_msg[512];
        _stprintf(sz_msg, TEXT("Unable to open file: %s"), parameters->filename);
        ::MessageBox(NULL,sz_msg,TEXT("Unable to open file."),MB_OK|MB_ICONHAND);
        return;
      }

      strncpy(mpFirstColumnContents, parameters->firstcolumn, MAX_PATH);
      
      if (parameters->includeHeader)
      {
        Languages* p_lang = Languages::GetInstance();
        LANGUAGE_TYPE orig_lang = p_lang->GetLanguage();
        p_lang->SetLanguage(DEV_LANGUAGE);

        // write a simple header
        DataPointTime* pdt = new DataPointTime();
        fprintf(mpStringLengthFile, "\"%s\";\"%s\"\r\n",
          p_lang->GetLanguageName( orig_lang ),
          pdt->GetText()
          );

        p_lang->SetLanguage(orig_lang);

        delete pdt;


        if (parameters->onlyRelationsInCurrentDisplay)
        {
          sHelpStringIdExported.clear();

          // write column names
          fprintf(mpStringLengthFile, "\"%s\";\"%s\";\"%s\";\"%s\";\"%s\"\r\n",
            "DisplayFilename","DisplayId","Display","StringId","Visible");
        }
        else
        {
          // write column names
          fprintf(mpStringLengthFile, "\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\"\r\n",
            "DisplayId","Display",
            "StringId","Visible","String Width","String Height",
            "ComponentId","Component Width","Component Height", "Max. no of lines",
            "Wrapped","Fits");
        }
      }

      sStringIdCompWidth.clear();

      if (parameters->onlyRelationsInCurrentDisplay)
      {
        // special export for first display
        if (parameters->includeHeader)
        {
          mpDisplayCtrl->GetMenuBar()->CalculateStringWidths(false);
        }

        // special export of unreferenced helptexts
        if (strcmp(mpFirstColumnContents, UNREFERENCED_HELPTEXT) == 0)
        {
          for (int i = 0; i < LANG_GEN_HELP_STRING_COUNT; ++i)
          {
            int id = Languages::GetInstance()->GetHelpStingId(i);

            std::map<U16, bool>::iterator itr = sHelpStringIdExported.find(id);
            if (itr == sHelpStringIdExported.end())
            {
              fprintf(mpStringLengthFile, "\"%s\";\"%d\";\"[%s]\";\"%d\";\"%d\"\r\n",
                mpFirstColumnContents, 0, "", id, 2);
            }
          }

        }
        else // normal export
        {

        // don't set g_is_calculating_strings, only visible listview rows should be exported
        PopupBox* popup = mpDisplayCtrl->GetCurrentPopupBox();
        if (popup != NULL)
        {
          popup->CalculateStringWidths(false);
        }
        else
        {
          Display* disp = mpDisplayCtrl->GetCurrentDisplay();
         
          g_is_calculating_strings = true;
          disp->GetRoot()->Invalidate();
          disp->GetRoot()->Run();
          g_is_calculating_strings = false;

          disp->CalculateStringWidths(false);

          //special export for display 2.1 Pump Control. (on/off/auto switch)
          if (disp->GetId() == 37)
          {
            CancelOrContinuePopUp* pConfirm = new CancelOrContinuePopUp();
            pConfirm->SetQuestionStringId( SID_YOU_ARE_ABOUT_TO_CHANGE_PUMP_OPERATION_MODE_ );
            pConfirm->SetVisible(false);
            pConfirm->CalculateStringWidths(false);
            delete pConfirm;
          }

          //special export for display 4.5 Alarm settings
          if (disp->GetId() == 58)
          {
            Label* pLabel = new Label(disp->GetRoot());
            pLabel->SetSize(230, 15);
            pLabel->SetStringId(SID_PUMP_ALARMS_GROUP_1);
            pLabel->CalculateStringWidths(false);
            pLabel->SetStringId(SID_PUMP_ALARMS_GROUP_2);
            pLabel->CalculateStringWidths(false);
            delete pLabel;
          }

          //special export for display 4.5.2 Pump Alarms
          if (disp->GetId() == 60)
          {
            Label* pLabel = new Label(disp->GetRoot());
            pLabel->SetSize(230, 15);
            pLabel->SetStringId(SID_PUMP_ALARM_GENIBUS_COM_ERROR_IO111);
            pLabel->CalculateStringWidths(false);
            delete pLabel;
          }


          //special export for 4.3.4.8 work/off/sleep schedule
          if (disp->GetId() == 76)
          {
            Label* pLabel = new Label(disp->GetRoot());
            pLabel->SetSize(78, 15);
            pLabel->SetVisible();
            pLabel->SetStringId( SID_WORK_PERIOD );
            pLabel->CalculateStringWidths(false);
            pLabel->SetStringId( SID_OFF_PERIOD );
            pLabel->CalculateStringWidths(false);
            pLabel->SetStringId( SID_SLEEP_PERIOD );
            pLabel->CalculateStringWidths(false);
            pLabel->SetVisible(false);
            delete pLabel;
          }
        }
      
        }
      }
      else // calculate widths of all strings
      {
        g_is_calculating_strings = true;

        // calculate menu bars strings
        mpDisplayCtrl->GetMenuBar()->CalculateStringWidths(true);

        // calculate strings of all displays
        int i = 0;
        while (i++ < MAX_DISPLAY_ID)
        {
          Display* p_disp = GetDisplay(i);
          if(p_disp && p_disp->GetAbleToShow())
          {
            mpDisplayCtrl->Push(p_disp);
            p_disp->CalculateStringWidths(true);

            p_disp->Hide();
          }
        }

        // calculate all alarm strings 
        CalculateAlarmStringWidths();

        // calculate all help texts
        CalculateHelpStringWidths();

        // calculate all unit texts
        CalculateUnitStringWidths();

        // calculate all remaining strings
        CalculateSpecialStringWidths();

        i = 0;
        std::map<U16, U16>::iterator itr;

        // ensure all string id's are exported
        while (i++ <= LANG_GEN_STRING_COUNT)
        {
          itr = sStringIdCompWidth.find(i);
          if (itr == sStringIdCompWidth.end())
          {
            fprintf(mpStringLengthFile, "\"%d\";\"%s\";\"%d\";\"%d\";\"%s\";\"%s\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\"\r\n",
              0,"",i,
              0,"","",
              0,0,0,
              0,0,0);
          }
        }

        g_is_calculating_strings = false;
        mpDisplayCtrl->ResetToHome();

      }

      fclose(mpStringLengthFile);
      
    }

    
    /*****************************************************************************
    * Function - WriteToCSV
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::WriteToCSV(CSV_ENTRY entry)
    {
      if ( mpStringLengthFile == NULL 
        || (entry.componentWidth <= 1 && entry.forcedVisible)
        || entry.stringWidth < 0
        || entry.stringId == 0)
      {
        return;
      }

      bool more_restrictive_location_already_found = false;
      bool export_relations_only = (mpFirstColumnContents[0] > 0);

      std::map<U16, U16>::iterator itr = sStringIdCompWidth.find(entry.stringId);
      bool string_id_previously_exported = (itr != sStringIdCompWidth.end());

      if (!string_id_previously_exported)
      {
        sStringIdCompWidth[entry.stringId] = entry.componentWidth;
      }
      else
      {
        if (sStringIdCompWidth[entry.stringId] <= entry.componentWidth)
        {
          more_restrictive_location_already_found = true;
        }
        else
        {
          sStringIdCompWidth[entry.stringId] = entry.componentWidth;
        }
      }

      if (more_restrictive_location_already_found && !export_relations_only)
      {
        return;
      }

      if (export_relations_only && string_id_previously_exported && !entry.visible)
      {
        return;
      }

      int maxLineCount = entry.componentHeight / 13;

      const char* display_number = mpDisplayCtrl->GetCurrentDisplay()->GetDisplayNumber();
      bool show_display_id = strlen(display_number) > 0;

      
      if (export_relations_only)
      {
        fprintf(mpStringLengthFile, "\"%s\";\"%d\";\"[%s]\";\"%d\";\"%d\"\r\n", //regional settings = da
          mpFirstColumnContents,
          (show_display_id ? mpDisplayCtrl->GetCurrentDisplay()->GetId() : 0), 
          display_number,
          entry.stringId, 
          (int)entry.visible);
      }
      else
      {

        fprintf(mpStringLengthFile, "\"%d\";\"[%s]\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\";\"%d\"\r\n", //regional settings = da
          (show_display_id ? mpDisplayCtrl->GetCurrentDisplay()->GetId() : 0), 
          display_number,
          entry.stringId, 
          (int)entry.visible,
          entry.stringWidth, 
          entry.stringHeight,
          entry.componentId, 
          entry.componentWidth,
          entry.componentHeight,
          maxLineCount,
          (int)entry.wordwrap,
          (int)entry.fits);
      }
      
    }

    /*****************************************************************************
    * Function - WriteHelpRefToCSV
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::WriteHelpRefToCSV(STRING_ID id)
    {
      if (mpFirstColumnContents[0] != '\0')
      {
        std::map<U16, U16>::iterator itr = sStringIdCompWidth.find(id);
        bool string_id_previously_exported = (itr != sStringIdCompWidth.end());

        if ( mpStringLengthFile != NULL && id > 0 && !string_id_previously_exported)
        {
          fprintf(mpStringLengthFile, "\"%s\";\"%d\";\"[%s]\";\"%d\";\"%d\"\r\n",
            mpFirstColumnContents,
            mpDisplayCtrl->GetCurrentDisplay()->GetId(), 
            mpDisplayCtrl->GetCurrentDisplay()->GetDisplayNumber(),
            id, 
            2);

          sHelpStringIdExported[id] = true;
          sStringIdCompWidth[id] = 0;
        }
      }
    }


    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/

    /*****************************************************************************
    * Function - CalculateAlarmStringWidths
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::CalculateAlarmStringWidths()
    {
      Label* pLabel = new Label();

      // use the same size as mLabelErrorString in AlarmListItem
      pLabel->SetSize(225-5+1, 15);

      mpDisplayCtrl->GetCurrentDisplay()->SetDisplayNumber("");

      for(int i = 0; i < DISPLAY_ALARM_STRINGS_CNT; ++i)
      {
        pLabel->SetStringId( DISPLAY_ALARM_STRINGS[i].StringId );
        pLabel->CalculateStringWidths(true);
      }

      pLabel->SetStringId( SID_ALARM_UNKNOWN );
      pLabel->CalculateStringWidths(true);
      pLabel->SetVisible(false);

      delete pLabel;
    }

    /*****************************************************************************
    * Function - CalculateHelpStringWidths
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::CalculateHelpStringWidths()
    {
      mpDisplayCtrl->GetCurrentDisplay()->SetDisplayNumber("");

      HelpMessageBox* pHelp = new HelpMessageBox();
      
      for (int i = 0; i < LANG_GEN_HELP_STRING_COUNT; ++i)
      {
        pHelp->SetMessage( Languages::GetInstance()->GetHelpStingId(i) );
        pHelp->CalculateStringWidths(true);        
      }

      pHelp->SetVisible(false);
      delete pHelp;

    }

    /*****************************************************************************
    * Function - CalculateUnitStringWidths
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::CalculateUnitStringWidths()
    {
      mpDisplayCtrl->GetCurrentDisplay()->SetDisplayNumber("");

      Label* pLabel = new Label();

      // use the same size as mLabelErrorUnit in AlarmListItem
      pLabel->SetSize(225-19+1, 15);
      pLabel->SetChildPos(240, 0);
      pLabel->SetVisible();

      for(int i = 0; i < DISPLAY_UNIT_STRINGS_CNT; ++i)
      {
        pLabel->SetStringId( DISPLAY_UNIT_STRINGS[i].StringId );
        //ignore duplicates
        if (!pLabel->IsValid())
          pLabel->CalculateStringWidths(true);
      }

      pLabel->SetStringId( SID_UNIT_UNKNOWN );
      pLabel->CalculateStringWidths(true);
      pLabel->SetVisible(false);

      delete pLabel;
    }


    
    /*****************************************************************************
    * Function - CalculateSpecialStringWidths
    * DESCRIPTION:
    *
    ****************************************************************************/
    void StringWidthCalculator::CalculateSpecialStringWidths()
    {      
      mpDisplayCtrl->GetCurrentDisplay()->SetDisplayNumber("");

      Label* pLabel = new Label();
      pLabel->SetSize(239+1, 15);
      pLabel->SetStringId( SID_STEP_BY_STEP_INSTALLATION_GUIDE );
      pLabel->CalculateStringWidths(true);

      pLabel->SetSize(78, 15);
      pLabel->SetStringId( SID_WORK_PERIOD );
      pLabel->CalculateStringWidths(true);
      pLabel->SetStringId( SID_OFF_PERIOD );
      pLabel->CalculateStringWidths(true);
      pLabel->SetStringId( SID_SLEEP_PERIOD );
      pLabel->CalculateStringWidths(true);
      pLabel->SetVisible(false);
      delete pLabel;

      CancelOrContinuePopUp* pConfirm = new CancelOrContinuePopUp();
      pConfirm->SetQuestionStringId( SID_PASSWORD_TEXT );
      pConfirm->CalculateStringWidths(true);
      pConfirm->SetQuestionStringId( SID_YOU_ARE_ABOUT_TO_CHANGE_PUMP_OPERATION_MODE_ );
      pConfirm->CalculateStringWidths(true);
      pConfirm->SetVisible(false);
      delete pConfirm;

    }
    

    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    *****************************************************************************/
     /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/
    StringWidthCalculator::StringWidthCalculator()
    {
      mpDisplayCtrl = DisplayController::GetInstance();
      mpStringLengthFile = NULL;

    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    StringWidthCalculator::~StringWidthCalculator()
    {

    }


  } // namespace display
} // namespace mpc


