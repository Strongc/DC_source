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
/* CLASS NAME       : DisplayDumper                                         */
/*                                                                          */
/* FILE NAME        : DisplayDumper.cpp                                     */
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
#include <PcDevToolService.h>
#include <U8DataPoint.h>
#include <EnumDataPoint.h>
#include <BaseDirectory.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include <DisplayDumper.h>
#include <ListView.h>
#include <Group.h>
#include <MenuBar.h>
#include <UpperStatusLine.h>
#include <LowerStatusLine.h>
#include <UTF16-UTF8.h>


/*****************************************************************************
DEFINES
*****************************************************************************/
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define DISPLAY_RELOAD_TIME_IN_MS 20

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
extern "C" void SIM_GUI_GetLCDPos           (int * px, int * py);

namespace mpc
{
  namespace display
  {
    static std::map<U16, mpc::display::Display*> sDumpedDisplayCol;
    DisplayDumper* DisplayDumper::mInstance = 0;

    /*****************************************************************************
    *
    *
    *              PUBLIC FUNCTIONS
    *
    *
    *****************************************************************************/

    DisplayDumper* DisplayDumper::GetInstance()
    {
      if (!mInstance)
      {
        mInstance = new DisplayDumper();
      }
      return mInstance;
    }

   
    /*****************************************************************************
    * Function - DumpScreen
    * DESCRIPTION:
    *
    ****************************************************************************/
    bool DisplayDumper::DumpScreen( bool recursive/*=true*/, bool lcdonly/*=true*/, int bmpnumber /*=0*/, bool useTitleAsFilename/*=false*/)
    {
      USES_CONVERSION;

      char sz_path_text[MAX_PATH * 3 + 1];
      char sz_file_path[MAX_PATH * 3 + 1];
      char sz_title[MAX_PATH * 3 + 1];
      
      bool previously_dumped = false;

      std::map<U16, mpc::display::Display*>::iterator itr = sDumpedDisplayCol.find(mpDisplayCtrl->GetCurrentDisplay()->GetId());
      if (itr == sDumpedDisplayCol.end())
      {
        sDumpedDisplayCol[mpDisplayCtrl->GetCurrentDisplay()->GetId()] = mpDisplayCtrl->GetCurrentDisplay();
      }
      else
      {
        previously_dumped = true;
      }

      if (useTitleAsFilename)
      {
        sprintf(sz_title, "%-10s %s", mpDisplayCtrl->GetCurrentDisplay()->GetDisplayNumber(), Languages::GetInstance()->GetString(mpDisplayCtrl->GetCurrentDisplay()->GetName()));

        if (strchr(mpDisplayCtrl->GetCurrentDisplay()->GetDisplayNumber(),'/') != NULL )
        {
          char sz_wizard_title[255];

          *(strchr(sz_title,'/')) = '-';

          strcpy(sz_wizard_title, sz_title);
          sprintf(sz_title, "wizard_%s", sz_wizard_title);
        }

        while (strchr(sz_title,'/') != NULL)
          *(strchr(sz_title,'/')) = '_';

        while (strchr(sz_title,':') != NULL)
          *(strchr(sz_title,':')) = '#';

        sprintf(sz_file_path,"%s_%0.3d.bmp", sz_title, bmpnumber);
      }
      else
      {
        sprintf(sz_path_text,"%s%0.4d", BaseDirectory::GetInstance()->Get(), bmpnumber);
        sprintf(sz_file_path,"%s.bmp",sz_path_text);
      }

      // Run to update screen. (Run twice just to make sure :) - more....

      for (int run=0; run < 5;++run)
      {
        mpDisplayCtrl->Run();
      }
      WM_Exec();


      /******************************
      Use the win32 api to screen dump
      *******************************/

      //GUI_Delay(2);
      //Sleep(2);

      bool variants_was_dumped = false;

      if (recursive && !previously_dumped)
        variants_was_dumped = DumpDisplayVariants();

      if (!variants_was_dumped)
      {
        int width = 600;
        int height = 561;
        int x_off = 0;
        int y_off = 0;
        HDC  compatible_main_wnd_dc;
        HDC  main_wnd_dc;
        HBITMAP hbmScreen;
        PBITMAPINFO pBmpInfo;

        GUI_LOCK();

        mMainWndHandle = PcDevToolService::GetInstance()->GetControllerWindow();
        main_wnd_dc = GetWindowDC(mMainWndHandle);
        compatible_main_wnd_dc = CreateCompatibleDC(main_wnd_dc);

        if (lcdonly)
        {
          width = 242;
          height = 320;
          SIM_GUI_GetLCDPos(&x_off, &y_off);
          x_off+=6;

        }
        else
        {
          WINDOWPLACEMENT wplacement;
          wplacement.length = sizeof(WINDOWPLACEMENT);
          GetWindowPlacement(mMainWndHandle, &wplacement);
          width = wplacement.rcNormalPosition.right - wplacement.rcNormalPosition.left;
          width -= 2*GetSystemMetrics(SM_CXFIXEDFRAME);
          height = wplacement.rcNormalPosition.bottom - wplacement.rcNormalPosition.top;
          height -= 2*GetSystemMetrics(SM_CYFIXEDFRAME);
          height -= GetSystemMetrics(SM_CYCAPTION);
        }
        x_off += GetSystemMetrics(SM_CXFIXEDFRAME) + 2;
        y_off += GetSystemMetrics(SM_CYFIXEDFRAME) + 2;
        y_off += GetSystemMetrics(SM_CYCAPTION); // Caption bar
        hbmScreen = CreateCompatibleBitmap(main_wnd_dc, width,height);

        pBmpInfo = CreateBitmapInfoStruct(hbmScreen);

        if (!SelectObject(compatible_main_wnd_dc, hbmScreen))
          Beep(2000,100);
        SetForegroundWindow(mMainWndHandle);
        RedrawWindow(mMainWndHandle, NULL, NULL,RDW_INVALIDATE| RDW_UPDATENOW|RDW_ALLCHILDREN);

        //Copy color data for the entire display into a
        //bitmap that is selected into a compatible DC.
        if (BitBlt(compatible_main_wnd_dc,
          0,0,
          pBmpInfo->bmiHeader.biWidth, pBmpInfo->bmiHeader.biHeight,
          main_wnd_dc,
          x_off,y_off,
          SRCCOPY) == 0)
        {
          ::MessageBox(NULL, TEXT("BitBlt failed"), TEXT("Save image error"), MB_ICONERROR|MB_OK);
        }


        CreateBMPFile(A2T(sz_file_path),pBmpInfo,hbmScreen,compatible_main_wnd_dc);

        LocalFree((HLOCAL)pBmpInfo);
        DeleteObject(hbmScreen);

        GUI_UNLOCK();
      }


      /******************************
      END Use the win32 api
      *******************************/

      if (recursive && !previously_dumped)
      {
        Group*      p_root_group = mpDisplayCtrl->GetCurrentDisplay()->GetRoot();
        Display*    p_disp = NULL;

        // Create a list of refered displays on the root group.
        std::vector<Display*>   refered_displays;
        CreateDisplayList(p_root_group, &refered_displays);

        // Links for the index.html file
        std::vector<Display*>::iterator iterEnd = refered_displays.end();
        std::vector<Display*>::iterator iter = refered_displays.begin();


        // Step into and dump reffered displays.
        for (iter = refered_displays.begin(); iter != iterEnd; ++iter)
        {
          p_disp = dynamic_cast<Display*>(*iter);
          sprintf(sz_path_text,"%s", p_disp->GetDisplayNumber());
          if (strchr(sz_path_text,'/') != NULL)
            *(strchr(sz_path_text,'/')) = '-';
          mkdir(sz_path_text);
          chdir(sz_path_text);

          mpDisplayCtrl->Push(p_disp);

          if (!DumpScreen())
          {
            chdir("..");
            return false;
          }
          mpDisplayCtrl->Pop();

          chdir("..");
          
        }

        refered_displays.clear();
      }

      return true;
    }


    /*****************************************************************************
    * Function - DumpScreens
    * DESCRIPTION:
    * Asks the user if all displays and languages shopuld be dumped. 
    * Calls DumpScreen for each menu/language combination.
    *****************************************************************************/
    void DisplayDumper::DumpScreens(bool lcdonly /*=true*/, bool developeronly /*=false*/)
    {
      USES_CONVERSION;
      TCHAR sz_msg[200];
      char current_working_dir[1025];

      _stprintf( sz_msg, TEXT("Screen dump all displays in all languages?"));
      if (developeronly || ::MessageBox(NULL, sz_msg, TEXT("Screen dump"), MB_YESNO|MB_ICONINFORMATION) != IDYES)
      {
        // not dumping all screens in all languages. 
        // Ask if display should be dumped in developer language  

        _stprintf( sz_msg, TEXT("Screen dump all displays in developer language?"));
        if (developeronly || ::MessageBox(NULL, sz_msg, TEXT("Screen dump"), MB_YESNO|MB_ICONINFORMATION) == IDYES)
        {
          DumpAllDevScreensInSingleFolder();
          return;
        }

        // Ask if display should be dumped in current language 

        _stprintf( sz_msg, TEXT("Screen dump all displays in current language?"));
        if ( ::MessageBox(NULL, sz_msg, TEXT("Screen dump"), MB_YESNO|MB_ICONINFORMATION) == IDYES)
        {
          mkdir("Screen_dumps");
          chdir("Screen_dumps");

          DumpAllScreensWithStructure( Languages::GetInstance()->GetLanguage(), lcdonly);
          
          chdir("..");
          return;
        }
        //else not dumping anything

        return;
      }
      // else continue to dump all screens in all languages

      if (mpDisplayCtrl->GetMenuBar() == NULL)
      {
        throw TEXT("Please set menubar before dumping screens.");
        return;
      }

      int current_drive = _getdrive();
      _getdcwd(current_drive, current_working_dir, sizeof(current_working_dir)-1);
      

      HWND wnd = PcDevToolService::GetInstance()->GetControllerWindow();
      SetWindowPos( wnd,
        HWND_TOPMOST,
        0, // X ignored
        0, // Y ignored
        0, // width ignored
        0, // height ignored
        SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

      DWORD dw_start = GetTickCount();
      mkdir("Screen_dumps");
      chdir("Screen_dumps");

      LANGUAGE_TYPE org_lang = Languages::GetInstance()->GetLanguage();

      OS_IncDI();
      OS_EnterRegion();
      try
      {
        
        LANGUAGE_TYPE language = FIRST_LANGUAGE;
        for (;language <= LAST_LANGUAGE; language = (LANGUAGE_TYPE)(((int)language) + 1))
        {
          DumpAllScreensWithStructure(language, lcdonly);
        } // for
      }
      catch(const TCHAR* sz_msg)
      {
        ::MessageBox(NULL,sz_msg,TEXT("Error dumping screens"), MB_OK|MB_ICONERROR);
      }
      catch(const char* sz_msg)
      {
        ::MessageBox(NULL,(TCHAR*)sz_msg,TEXT("Error dumping screens"), MB_OK|MB_ICONERROR);
      }
      catch(...)
      {
        ::MessageBox(NULL,TEXT("..Exception caught"),TEXT("Error dumping screens"), MB_OK|MB_ICONERROR);
      }
      OS_LeaveRegion();
      OS_DecRI();

      chdir("..");  // Move back from Screen_dumps

      Languages::GetInstance()->SetLanguage(org_lang);
      mpDisplayCtrl->ResetToHome();

      chdir(current_working_dir);

      SetWindowPos( wnd,
        HWND_NOTOPMOST,
        0, // X ignored
        0, // Y ignored
        0, // width ignored
        0, // height ignored
        SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

      DWORD time = labs(GetTickCount() - dw_start);
      _stprintf( sz_msg, TEXT("Screen dump completed in %ldms"),time);
      ::MessageBox(NULL, sz_msg, TEXT("Screen dump"), MB_OK|MB_ICONINFORMATION);


    }


    /*****************************************************************************
    * Function - 
    * DESCRIPTION:
    *
    ****************************************************************************/
    bool DisplayDumper::DumpHelpBoxes()
    {
      // Dummy component used to call ShowHelp
      Component dummy_comp;

      // Hide everything else but the HelpBox
      mpDisplayCtrl->GetMenuBar()->SetVisible(false);
      mpDisplayCtrl->GetLowerStatusLine()->SetVisible(false);
      mpDisplayCtrl->GetUpperStatusLine()->SetVisible(false);
      mpDisplayCtrl->GetCurrentDisplay()->GetRoot()->SetVisible(false);

      // Just some runtime to the hidden components
      for (int j = 0; j < 10;++j)
      {
        if (mpDisplayCtrl->GetMenuBar()!=NULL)
        {
          mpDisplayCtrl->GetMenuBar()->Run();
        }
        mpDisplayCtrl->GetUpperStatusLine()->Run();
        mpDisplayCtrl->GetLowerStatusLine()->Run();
        mpDisplayCtrl->GetCurrentDisplay()->Run();
      }

      // Clear the Desktop window for hanging graphics from hidden components
      WM_HWIN desk_win = WM_GetDesktopWindow();
      WM_SelectWindow(desk_win);
      GUI_SetColor(GUI_WHITE);
      GUI_SetBkColor(GUI_WHITE);
      GUI_Clear();

      // Create and change to dir for screen dumps
      mkdir("Screen_dumps");
      chdir("Screen_dumps");
      mkdir("HelpMessages");
      chdir("HelpMessages");

      // Loop through the help messages
      for (int i = 0; i < LANG_GEN_HELP_STRING_COUNT; ++i)
      {
        STRING_ID sid_tmp = Languages::GetInstance()->GetHelpStingId(i);		
        // Set the help string on the dummy_component.
        dummy_comp.SetHelpString(sid_tmp);

        // Show the help popup
        dummy_comp.ShowHelp();
        
        // Give the popup some runtime
        mpDisplayCtrl->Run();

        // Dump LCD to BMP file        
        DumpScreen(false, true, i);

        // Destroy the messagebox
        mpDisplayCtrl->KeyEvent(MPC_OK_KEY); 
      }

      // Back to normal oper
      chdir("..");
      chdir("..");
      mpDisplayCtrl->GetMenuBar()->SetVisible(true);
      mpDisplayCtrl->GetLowerStatusLine()->SetVisible(true);
      mpDisplayCtrl->GetUpperStatusLine()->SetVisible(true);
      mpDisplayCtrl->GetCurrentDisplay()->GetRoot()->SetVisible(true);
      return true;
    }


    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/

    /*****************************************************************************
    * Function - DumpDisplayVariants
    * DESCRIPTION:
    * returns true if something was dumped
    ****************************************************************************/
    bool DisplayDumper::DumpDisplayVariants()
    {
      int display_id = mpDisplayCtrl->GetCurrentDisplay()->GetId();

      if (display_id == 39) // fsw config selection
      {
        EnumDataPoint<SENSOR_TYPE_TYPE>* pDp_level_control_type = dynamic_cast<EnumDataPoint<SENSOR_TYPE_TYPE>*>( GetSubject(SUBJECT_ID_PIT_LEVEL_CTRL_TYPE) );
        U8DataPoint* pDp_no_of_fsw = dynamic_cast<U8DataPoint*>( GetSubject(SUBJECT_ID_NO_OF_FLOAT_SWITCHES) );
        U8DataPoint* pDp_no_of_pumps = dynamic_cast<U8DataPoint*>( GetSubject(SUBJECT_ID_NO_OF_PUMPS) );

        SENSOR_TYPE_TYPE orig_ctrl = pDp_level_control_type->GetValue();
        U8 orig_no_of_fsw = pDp_no_of_fsw->GetValue();
        U8 orig_no_of_pumps = pDp_no_of_pumps->GetValue();

        pDp_level_control_type->SetValue(SENSOR_TYPE_FLOAT_SWITCHES);

        pDp_no_of_pumps->SetValue((U8)1);
        pDp_no_of_fsw->SetValue((U8)2);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 1, false);
        pDp_no_of_fsw->SetValue((U8)3);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 2, false);
        pDp_no_of_fsw->SetValue((U8)4);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 3, false);

        pDp_no_of_pumps->SetValue((U8)2);
        pDp_no_of_fsw->SetValue((U8)3);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 4, false);
        pDp_no_of_fsw->SetValue((U8)4);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 5, false);
        pDp_no_of_fsw->SetValue((U8)5);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 6, false);

        pDp_level_control_type->SetValue(orig_ctrl);
        pDp_no_of_fsw->SetValue(orig_no_of_fsw);
        pDp_no_of_pumps->SetValue(orig_no_of_pumps);

        return true;
      }
      else if (display_id == 1) // pit status
      {
        EnumDataPoint<SENSOR_TYPE_TYPE>* pDp_level_control_type = dynamic_cast<EnumDataPoint<SENSOR_TYPE_TYPE>*>( GetSubject(SUBJECT_ID_PIT_LEVEL_CTRL_TYPE) );
        U8DataPoint* pDp_no_of_fsw = dynamic_cast<U8DataPoint*>( GetSubject(SUBJECT_ID_NO_OF_FLOAT_SWITCHES) );
        U8DataPoint* pDp_no_of_pumps = dynamic_cast<U8DataPoint*>( GetSubject(SUBJECT_ID_NO_OF_PUMPS) );
        U8DataPoint* pDp_fsw_conf = dynamic_cast<U8DataPoint*>( GetSubject(SUBJECT_ID_FLOAT_SWITCH_CONFIG_NUMBER) );

        pDp_no_of_pumps->SetValue((U8)2);

        pDp_level_control_type->SetValue(SENSOR_TYPE_PRESSURE);
        pDp_fsw_conf->SetValue((U8)3);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 1, false);

        pDp_level_control_type->SetValue(SENSOR_TYPE_FLOAT_SWITCHES);
        pDp_no_of_fsw->SetValue((U8)5);
        pDp_fsw_conf->SetValue((U8)1);
        Sleep(DISPLAY_RELOAD_TIME_IN_MS);
        DumpScreen(false, true, 2, false);

        return true;
      }

      return false;
    }


    /*****************************************************************************
    * Function - DumpAllDevScreensInSingleFolder
    * DESCRIPTION:
    *
    ****************************************************************************/
    void DisplayDumper::DumpAllDevScreensInSingleFolder()
    {
      LANGUAGE_TYPE org_lang = Languages::GetInstance()->GetLanguage();
      mkdir("DEV_Screen_dumps");
      chdir("DEV_Screen_dumps");

      OS_IncDI();
      OS_EnterRegion();

      Languages::GetInstance()->SetLanguage(DEV_LANGUAGE);

      for (int i=0; i<MAX_DISPLAY_ID; i++)
      { 
        try
        {
          if ( PcDevToolService::GetInstance()->LoadDisplay(i)==0 && mpDisplayCtrl->GetCurrentDisplay()->GetAbleToShow() ){

            switch(mpDisplayCtrl->GetCurrentDisplay()->GetDisplayNumber()[0])
            {
            case '2':
              mpDisplayCtrl->GetMenuBar()->SetActiveTab(1);
              break;
            case '3':
              mpDisplayCtrl->GetMenuBar()->SetActiveTab(2);
              break;
            case '4':
              mpDisplayCtrl->GetMenuBar()->SetActiveTab(3);
              break;
            default:
              mpDisplayCtrl->GetMenuBar()->SetActiveTab(0);
            }
            mpDisplayCtrl->GetMenuBar()->Run();

            //load display again, due to stack reset at SetActiveTab
            PcDevToolService::GetInstance()->LoadDisplay(i);

            DumpScreen(false,true,i,true);
          }

        }
        catch(...)
        {
          //ignore
        }
      }

      Languages::GetInstance()->SetLanguage(org_lang);
      mpDisplayCtrl->ResetToHome();

      OS_LeaveRegion();
      OS_DecRI();
      chdir("..");

    }


    
    /*****************************************************************************
    * Function - DumpAllScreensWithStructure
    * DESCRIPTION:
    *
    ****************************************************************************/
    void DisplayDumper::DumpAllScreensWithStructure(LANGUAGE_TYPE language, bool lcdonly)
    {
      sDumpedDisplayCol.clear();

      Languages::GetInstance()->SetLanguage(DEV_LANGUAGE);
      const char* p_sz_language = Languages::GetInstance()->GetLanguageName( language );
    
      if ( !MkUnicodeDirFromUTF8String(p_sz_language) )
      {
        return;
      }

      Languages::GetInstance()->SetLanguage(language);
      mpDisplayCtrl->Run();

      // Loop through the menues.
      char sz_path_text[MAX_PATH * 3 + 1];
      for(int menu = 0; menu < 4; ++menu)
      {
        // SetActive tab resets displayControlles display stack.
        mpDisplayCtrl->GetMenuBar()->SetActiveTab(menu);
        mpDisplayCtrl->GetMenuBar()->Run();
        sprintf(sz_path_text,"%d", menu + 1);

        mkdir(sz_path_text);
        chdir(sz_path_text);
        
        // DumpScreen is recursive, meaning it goes into all displays
        // within the current display.
        if ( !DumpScreen(true,lcdonly) )
        {
          chdir("..");
        }

        chdir("..");

      }

      ///////////////////////////////////////////////////////////////////////////////

      // dump wizard
      sprintf(sz_path_text,"Wizard");

      mkdir(sz_path_text);
      chdir(sz_path_text);

      mpDisplayCtrl->EnableWizard( true );

      // display id from DisplayFactory.Display
      int wizard_displays[18] = {85, 86, 87, 88, 89, 90, 92, 93, 94, 95, 110, 96, 97, 111, 98, 99, 102, 100};

      for(int i=0; i<18; i++)
      {
        PcDevToolService::GetInstance()->LoadDisplay(wizard_displays[i]);

        DumpScreen(false, lcdonly, i);
      }

      chdir("..");

      mpDisplayCtrl->EnableWizard( false );

      chdir(".."); // Move back from language
      mpDisplayCtrl->ResetToHome();
    }
    


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    PBITMAPINFO DisplayDumper::CreateBitmapInfoStruct(HBITMAP hBmp)
    {
      BITMAP bmp;
      PBITMAPINFO pbmi;
      WORD    cClrBits;

      // Retrieve the bitmap color format, width, and height.
      if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
        throw( TEXT("GetObject") );

      // Convert the color format to a count of bits.
      cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
      if (cClrBits == 1)
        cClrBits = 1;
      else if (cClrBits <= 4)
        cClrBits = 4;
      else if (cClrBits <= 8)
        cClrBits = 8;
      else if (cClrBits <= 16)
        cClrBits = 16;
      else if (cClrBits <= 24)
        cClrBits = 24;
      else cClrBits = 32;

      // Allocate memory for the BITMAPINFO structure. (This structure
      // contains a BITMAPINFOHEADER structure and an array of RGBQUAD
      // data structures.)

      if (cClrBits != 24)
      {
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits));
      }
      // There is no RGBQUAD array for the 24-bit-per-pixel format.
      else
      {
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
      }

      // Initialize the fields in the BITMAPINFO structure.
      pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      pbmi->bmiHeader.biWidth = bmp.bmWidth;
      pbmi->bmiHeader.biHeight = bmp.bmHeight;
      pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
      pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
      if (cClrBits < 24)
      {
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits);
      }

      // If the bitmap is not compressed, set the BI_RGB flag.
      pbmi->bmiHeader.biCompression = BI_RGB;

      // Compute the number of bytes in the array of color
      // indices and store the result in biSizeImage.
      // For Windows NT, the width must be DWORD aligned unless
      // the bitmap is RLE compressed. This example shows this.
      // For Windows 95/98/Me, the width must be WORD aligned unless the
      // bitmap is RLE compressed.
      pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) / 8 * pbmi->bmiHeader.biHeight;

      // Set biClrImportant to 0, indicating that all of the
      // device colors are important.
      pbmi->bmiHeader.biClrImportant = 0;

      return pbmi;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void DisplayDumper::CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
    {
      HANDLE hf;                 // file handle
      BITMAPFILEHEADER hdr;       // bitmap file-header
      PBITMAPINFOHEADER pbih;     // bitmap info-header
      LPBYTE lpBits;              // memory pointer
      DWORD dwTotal;              // total count of bytes
      DWORD cb;                   // incremental count of bytes
      BYTE *hp;                   // byte pointer
      DWORD dwTmp;

      pbih = (PBITMAPINFOHEADER) pbi;
      lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

      if (!lpBits)
        throw(TEXT("GlobalAlloc"));

      // Retrieve the color table (RGBQUAD array) and the bits
      // (array of palette indices) from the DIB.
      if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
      {
        throw(TEXT("GetDIBits"));
      }

      // Create the .BMP file.
      hf = CreateFile(  pszFile,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD) 0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE) NULL);

      if (hf == INVALID_HANDLE_VALUE)
        throw(TEXT("CreateFile 1"));

      hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"

      // Compute the size of the entire file.
      hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
      hdr.bfReserved1 = 0;
      hdr.bfReserved2 = 0;

      // Compute the offset to the array of color indices.
      hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD);

      // Copy the BITMAPFILEHEADER into the .BMP file.
      if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp,  NULL))
      {
        throw(TEXT("WriteFile 1"));
      }

      // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
      if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, ( NULL)) )
        throw(TEXT("WriteFile 2"));

      // Copy the array of color indices into the .BMP file.
      dwTotal = cb = pbih->biSizeImage;
      hp = lpBits;
      if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))
        throw(TEXT("WriteFile 3"));

      // Close the .BMP file.
      if (!CloseHandle(hf))
        throw(TEXT("CloseHandle 1"));

      // Free memory.
      GlobalFree((HGLOBAL)lpBits);
    }


    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    void  DisplayDumper::CreateDisplayList(Group* rootGroup, std::vector<Display*>*  refDisplays)
    {
      Group*      p_root_group = dynamic_cast<Group*>(rootGroup);
      if (p_root_group == NULL)
      {
        return;
      }

      // Find all displays refered to in the group.
      Component*  p_comp;
      Component*  p_child = p_root_group->GetFirstChild();
      ListViewItem* lst_view_item = NULL;
      ListView* l_view = NULL;
      Group*    p_child_group = NULL;
      Display*  p_disp = NULL;

      while (p_child != NULL)
      {
        p_child_group = dynamic_cast<Group*>(p_child);
        l_view = dynamic_cast<ListView*>(p_child);
        if (p_child_group != NULL)
        {
          CreateDisplayList(p_child_group, refDisplays);
        }
        // If the child is a list loop through rows in that list.
        else if (l_view != NULL)
        {
          int lst_view_item_no = 0;
          int col_count = l_view->GetColCount();
          // Loop all items (rows) in the ListView
          while ( (lst_view_item = l_view->GetListViewItem(lst_view_item_no)) != NULL)
          {
            // Search for a column with a display attached in the row.
            for (int column = 0;column<col_count;++column)
            {
              if ( (p_comp = lst_view_item->GetItem(column)) != 0)
              {
                p_disp = p_comp->GetDisplay();
                if ( p_disp != NULL )
                {
                  refDisplays->push_back(p_disp);
                }
              }
            }
            ++lst_view_item_no;
          }
        }

        p_child = p_root_group->GetNextChild(p_child);
      } // while(p_child != NULL)
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    *
    *****************************************************************************/
    bool DisplayDumper::MkUnicodeDirFromUTF8String(const char* p_u8_string)
    {

      UTF16 sz_unicode[MAX_PATH * 3 +1];
      ConversionResult rc = ::UTF82UTF16(sz_unicode,(UTF8*)p_u8_string,MAX_PATH);
      if (rc != conversionOK)
      {
        Beep(4000, 300);
        Sleep(100);
        Beep(4000, 300);
        Sleep(100);
        Beep(4000, 300);
        Sleep(100);
        Beep(4000, 300);
        Sleep(100);
        TCHAR sz_msg[200];
        _stprintf( sz_msg, _T("UTF82UTF16 returned %ld"), rc );
        ::MessageBox(NULL, sz_msg, _T("Error converting UTF8 -> UTF16"), MB_OK|MB_ICONSTOP);
        return false;
      }

      // Create and switch to a dir with the name of the language.
      _tmkdir((TCHAR*)sz_unicode);
      _tchdir((TCHAR*)sz_unicode);

      return true;
    }


     /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/
    DisplayDumper::DisplayDumper()
    {
      mpDisplayCtrl = DisplayController::GetInstance();
      mMainWndHandle = NULL;
    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    DisplayDumper::~DisplayDumper()
    {

    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Shows help boxes and dumps em to a BMP file 
    *****************************************************************************/
      } // namespace display
} // namespace mpc


