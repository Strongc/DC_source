/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRange                                      */
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
/* CLASS NAME       : PumpGroupGraphic                                      */
/*                                                                          */
/* FILE NAME        : PumpGroupGraphic.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 2009-05-19                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
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
#include "PumpGroupGraphic.h"
#include "MPCFonts.h"

namespace mpc
{
  namespace display
  {
  /*****************************************************************************
  DEFINES
  *****************************************************************************/
  #define PGG_LABEL_HEIGHT     15 // height of group label
  #define PGG_PUMP_WIDTH       17 // width of a pump 
  #define PGG_SPACE_WIDTH      24 // width of the space between pumps
  #define PGG_PUMP_INTERVAL    (PGG_PUMP_WIDTH + PGG_SPACE_WIDTH)
  #define PGG_DIVIDER_X_OFFSET (-9)

  /*****************************************************************************
  EXTERNS
  *****************************************************************************/
  // width,height
  extern "C"  GUI_CONST_STORAGE GUI_BITMAP bmPumpGroups;    // 221, 24

  /*****************************************************************************
  * Function...: PumpGroupGraphic
  * DESCRIPTION: Constructor
  *****************************************************************************/
  PumpGroupGraphic::PumpGroupGraphic(Component* pParent) : ObserverGroup(pParent)
  {
    mGroupDividerPosition = 0;
    mVisibleImageWidth = 0;

    mpImgPumps = new Image(this);
    mpImgPumps->SetBitmap(&bmPumpGroups);
    mpImgPumps->SetVisible();
    mpImgPumps->SetAlign(GUI_TA_LEFT);
    mpImgPumps->SetTransparent();
    AddChild(mpImgPumps);

    for (int i = FIRST_PUMP_GROUP; i <= LAST_PUMP_GROUP; i++)
    {
      mpLabelGroupName[i] = new Label(this);
      mpLabelGroupName[i]->SetAlign(GUI_TA_VCENTER+GUI_TA_HCENTER);
      mpLabelGroupName[i]->SetFont(DEFAULT_FONT_11_LANGUAGE_DEP);
      mpLabelGroupName[i]->SetLeftMargin(0);
      mpLabelGroupName[i]->SetRightMargin(0);
      mpLabelGroupName[i]->SetReadOnly(true);
      mpLabelGroupName[i]->SetVisible(false);
      mpLabelGroupName[i]->SetWordWrap(false);
      mpLabelGroupName[i]->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      AddChild(mpLabelGroupName[i]);
    }

    mpLabelGroupName[PUMP_GROUP_1]->SetStringId(SID_GROUP_1);
    mpLabelGroupName[PUMP_GROUP_2]->SetStringId(SID_GROUP_2);

    mpDpNoOfPumps.SetUpdated();
    mpDpGroupsEnabled.SetUpdated();
    mpDpFirstPumpInGroupTwo.SetUpdated();

  }

  /*****************************************************************************
  * Function...: ~PumpGroupGraphic
  * DESCRIPTION: Dectructor
  *****************************************************************************/
  PumpGroupGraphic::~PumpGroupGraphic()
  {
    // not implemented - never called
  }


  /*****************************************************************************
  * Function...: CalculateClientAreas
  * DESCRIPTION: 
  *****************************************************************************/
  void PumpGroupGraphic::CalculateClientAreas()
  {
    int x1, y1, x2, y2;

    mVisibleImageWidth = (mpDpNoOfPumps->GetValue() * PGG_PUMP_INTERVAL);

    if (mpDpNoOfPumps->IsAtMax())
    {
      mVisibleImageWidth = GetWidth();
    }

    x1 = 0;
    y1 = PGG_LABEL_HEIGHT;
    x2 = x1 + mVisibleImageWidth - 1;
    y2 = y1 + bmPumpGroups.YSize - 1;
    mpImgPumps->SetClientArea(x1, y1, x2, y2);

    if (mpDpGroupsEnabled->GetAsBool())
    {
      mGroupDividerPosition = (mpDpFirstPumpInGroupTwo->GetAsInt() - 1) * PGG_PUMP_INTERVAL + PGG_DIVIDER_X_OFFSET;

      if (mGroupDividerPosition < mVisibleImageWidth)
      {
        mpLabelGroupName[PUMP_GROUP_1]->SetClientArea(0, 0, mGroupDividerPosition - 1, PGG_LABEL_HEIGHT - 1);
        mpLabelGroupName[PUMP_GROUP_2]->SetClientArea(mGroupDividerPosition + 2, 0, mVisibleImageWidth, PGG_LABEL_HEIGHT - 1);
      }
    }
    else
    {
      mGroupDividerPosition = 0;
    }

    bool show_labels = (mGroupDividerPosition > 0 && mGroupDividerPosition < mVisibleImageWidth);

    for (int i = FIRST_PUMP_GROUP; i <= LAST_PUMP_GROUP; i++)
    {
      mpLabelGroupName[i]->SetVisible(show_labels);
    }

    Invalidate();
  }



  /*****************************************************************************
  * Function...: Run
  * DESCRIPTION: 
  *****************************************************************************/
  void PumpGroupGraphic::Run()
  {
    bool update_graphics = false;

    if (mpDpNoOfPumps.IsUpdated())
    {
      update_graphics = true;
    }
    if (mpDpGroupsEnabled.IsUpdated())
    {
      update_graphics = true;
    }
    if (mpDpFirstPumpInGroupTwo.IsUpdated())
    {
      update_graphics = true;
    }

    if (update_graphics)
    {
      CalculateClientAreas();
    }

    mpLabelGroupName[PUMP_GROUP_1]->Run();
    mpLabelGroupName[PUMP_GROUP_2]->Run();

    if (!IsValid())
    {
      Redraw();
    }
  }

  /*****************************************************************************
  * Function...: Redraw
  * DESCRIPTION: Redraws this element
  *****************************************************************************/
  bool PumpGroupGraphic::Redraw()
  {
    Component::Redraw();

    mpLabelGroupName[PUMP_GROUP_1]->Redraw();
    mpLabelGroupName[PUMP_GROUP_2]->Redraw();
    mpImgPumps->Redraw();

    if (mGroupDividerPosition > 0 
     && mGroupDividerPosition < mVisibleImageWidth)
    {
      SelectWindow();
      GUI_SetColor(GetColour());
      GUI_SetLineStyle(GUI_LS_DOT);
      GUI_DrawLine(mGroupDividerPosition, 0, mGroupDividerPosition, GetHeight());
      GUI_SetLineStyle(GUI_LS_SOLID);
    }
    Validate();

    return true;
  }

  /* --------------------------------------------------
  * Update is part of the observer pattern
  * --------------------------------------------------*/
  void PumpGroupGraphic::Update(Subject* pSubject)
  {
    mpDpNoOfPumps.Update(pSubject);
    mpDpGroupsEnabled.Update(pSubject);
    mpDpFirstPumpInGroupTwo.Update(pSubject);
  }

  /* --------------------------------------------------
  * Called if subscription shall be cancelled
  * --------------------------------------------------*/
  void PumpGroupGraphic::SubscribtionCancelled(Subject* pSubject)
  {
    // not implemented - never called
  }

  /* --------------------------------------------------
  * Called to set the subject pointer (used by class
  * factory)
  * --------------------------------------------------*/
  void PumpGroupGraphic::SetSubjectPointer(int Id, Subject* pSubject)
  {
    switch (Id)
    {
      case SP_PGG_NO_OF_PUMPS:
      mpDpNoOfPumps.Attach(pSubject);
      break; 
      case SP_PGG_PUMP_GROUPS_ENABLED:
      mpDpGroupsEnabled.Attach(pSubject);
      break; 
      case SP_PGG_FIRST_PUMP_IN_GROUP_2:
      mpDpFirstPumpInGroupTwo.Attach(pSubject);
      break;
    }

  }

  /* --------------------------------------------------
  * Called to indicate that subscription kan be made
  * --------------------------------------------------*/
  void PumpGroupGraphic::ConnectToSubjects(void)
  {      
    mpDpNoOfPumps.Subscribe(this);
    mpDpGroupsEnabled.Subscribe(this);
    mpDpFirstPumpInGroupTwo.Subscribe(this);
  }

  } // namespace display
} // namespace mpc


