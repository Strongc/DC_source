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
/* FILE NAME        : DisplayDumper.H                                       */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_DisplayDumper_h
#define mrc_DisplayDumper_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>
#include <map>
#include <stdio.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Languages.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Display.h>
#include <DisplayController.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
 
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
* EXTERN DECLARATIONS
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * FOWARD DECLARATIONS
    *****************************************************************************/
    class DisplayController;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class DisplayDumper
    {
      public:
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        static DisplayDumper* GetInstance(void);

        void    DumpScreens(bool lcdonly = true, bool developeronly = false);
        bool    DumpScreen(bool recursive = true, bool lcdonly = true, int bmpnumber = 0, bool useTitleAsFilename = false);
        bool    DumpHelpBoxes(void);

      protected:
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        bool    DumpDisplayVariants(void);
        void    DumpAllDevScreensInSingleFolder(void);
        void    DumpAllScreensWithStructure(LANGUAGE_TYPE language, bool lcdonly);

        void    CreateDisplayList(Group*  rootGroup, std::vector<Display*>*  refDisplays);
        
        PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp);
        void    CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
        bool    MkUnicodeDirFromUTF8String(const char* p_u8_string);

        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        DisplayController* mpDisplayCtrl;
        HWND mMainWndHandle;
        
    private:
        /********************************************************************
        OPERATIONS
        ********************************************************************/
        DisplayDumper();
        ~DisplayDumper();
        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        static DisplayDumper* mInstance;


    };
  }
}

#endif
