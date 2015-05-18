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
/* CLASS NAME       : LowerStatusLine                                       */
/*                                                                          */
/* FILE NAME        : LowerStatusLine.cpp                                   */
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
#include <AppTypeDefs.h>
#include <DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "LowerStatusLine.h"
#include "DataPointTime.h"
#include <TimeFormatDataPoint.h>
#include <ctrl\ServiceModeEnabled.h>
#include <Text.h>
#include <mpcfonts.h>

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
    /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/
    LowerStatusLine::LowerStatusLine() : Frame(true, false), Observer()
    {
    /* --------------------------------------------------
    * Memory leakage test -
    * Define __MEMORY_LEAKAGE_TEST___ in the project
    * to enable this test
    * --------------------------------------------------*/
    #ifdef __MEMORY_LEAKAGE_TEST___
      mpHeapSize = new HeapSize();
      AddChild(mpHeapSize);
      mpHeapSize->SetColour(GUI_COLOUR_LOWER_STATUSLINE_FOREGROUND);
      mpHeapSize->SetBackgroundColour(GUI_COLOUR_LOWER_STATUSLINE_BACKGROUND);
      mpHeapSize->SetClientArea(0,0,129,14);
      mpHeapSize->SetAlign(GUI_TA_LEFT | GUI_TA_VCENTER);
      mpHeapSize->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);
    #endif // __MEMORY_LEAKAGE_TEST___

      mpInstallationName = new DataPointText();
      mpInstallationName->SetAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
      mpInstallationName->SetColour(GUI_COLOUR_LOWER_STATUSLINE_FOREGROUND);
      mpInstallationName->SetBackgroundColour(GUI_COLOUR_LOWER_STATUSLINE_BACKGROUND);
      mpInstallationName->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpInstallationName->SetClientArea(2,0,129,14);
      
      AddChild(mpInstallationName);

      mpCurrentTime = new DataPointTime;
      AddChild(mpCurrentTime);

      TimeFormatDataPoint* pDpTimePreference = TimeFormatDataPoint::GetInstance();
      pDpTimePreference->SetMaxValue(2);
      pDpTimePreference->SetMinValue(0);
      pDpTimePreference->SetValue(0);

      mpCurrentTime->SetSubjectPointer(0,pDpTimePreference);
      mpCurrentTime->ConnectToSubjects();
      mpCurrentTime->SetColour(GUI_COLOUR_LOWER_STATUSLINE_FOREGROUND);
      mpCurrentTime->SetBackgroundColour(GUI_COLOUR_LOWER_STATUSLINE_BACKGROUND);
      mpCurrentTime->SetClientArea(130,0,239,14);
      mpCurrentTime->SetAlign(GUI_TA_RIGHT | GUI_TA_VCENTER);
      mpCurrentTime->SetFont(DEFAULT_FONT_11_LANGUAGE_INDEP);

      SetBackgroundColour(GUI_COLOUR_LOWER_STATUSLINE_BACKGROUND);
      SetColour(GUI_COLOUR_LOWER_STATUSLINE_FOREGROUND);

      SetClientArea(0,306,239,319);
      SetVisible();

    /* --------------------------------------------------
    * Memory leakage test -
    * Define __MEMORY_LEAKAGE_TEST___ in the project
    * to enable this test
    * --------------------------------------------------*/
    #ifdef __MEMORY_LEAKAGE_TEST___
      mpHeapSize->SetVisible();
			mpInstallationName->SetVisible(false);
		#else
			mpInstallationName->SetVisible();
    #endif // __MEMORY_LEAKAGE_TEST___

      mpCurrentTime->SetVisible();
    }

    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    LowerStatusLine::~LowerStatusLine()
    {

    }

    void LowerStatusLine::Update(Subject* pSubject)
    {
      //ignore
    }
    
    void LowerStatusLine::SubscribtionCancelled(Subject* pSubject)
    {
      //ignore
    }
    
    void LowerStatusLine::SetSubjectPointer(int Id, Subject* pSubject)
    {
      //ignore
    }
    
    void LowerStatusLine::ConnectToSubjects(void)
    {
      mpInstallationName->SetSubjectPointer(0, GetSubject(SUBJECT_ID_INSTALLATION_NAME));
      mpInstallationName->ConnectToSubjects();
    }

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

