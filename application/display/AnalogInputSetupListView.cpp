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
/* CLASS NAME       : AnalogInputConfListView                               */
/*                                                                          */
/* FILE NAME        : AnalogInputConfListView.cpp                           */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class implements the digital input configuration ListView           */
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
#include "AnalogInputSetupListView.h"
#include "ListView.h"

#include <Factory.h>
#include "ModeCheckBox.h"
#include "label.h"
#include "AnalogInputFunctionState.h"
#include "NumberQuantity.h"
#include <AnaInOnIOCtrl.H>
#include "AvalibleIfNotSet.h"
#include <MPCFonts.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define mpc1ST_COLUMN_WIDTH 150
#define mpc2ND_COLUMN_WIDTH 87
#define mpc3RD_COLUMN_WIDTH 2
#define SUB_LIST_INDENT 0
#define mpcFONT *DEFAULT_FONT_13

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
namespace mpc
{
  namespace display
  {
    MODE_CHECK_BOX_LABEL_VALUE AnalogInputSetupListViewListData[] =
    {
      { SID_SENSOR_EL_TYPE_0_20mA,   SENSOR_ELECTRIC_TYPE_0_20mA },
      { SID_SENSOR_EL_TYPE_4_20mA,   SENSOR_ELECTRIC_TYPE_4_20mA },
      { SID_SENSOR_EL_TYPE_0_10V,    SENSOR_ELECTRIC_TYPE_0_10V },
      { SID_AI_NOT_USED_DISABLED,    SENSOR_ELECTRIC_DISABLED },
    };

    /*****************************************************************************
    *
    *
    *              PUBLIC FUNCTIONS
    *
    *
    *****************************************************************************/

    /*****************************************************************************
    * Function - Constructor
    * DESCRIPTION:
    *
    *****************************************************************************/

    AnalogInputSetupListView::AnalogInputSetupListView(Component* pParent /*= NULL*/) : ObserverGroup(pParent)
    {
      SetReadOnly(false);
      Label*                     label;
      ModeCheckBox*              mode_check_box;
      AnalogInputFunctionState*  analog_input_function_state;
      AvalibleIfNotSet*          avalible_if_not_set;
      NumberQuantity*            number_quantity;

      mpAnalogInputSetupListViewListData = AnalogInputSetupListViewListData;
      mAnalogInputSetupListViewListDataLength = sizeof(AnalogInputSetupListViewListData)/sizeof(MODE_CHECK_BOX_LABEL_VALUE);

      mpAnaInSetup = new ListView();
      mpAnaInSetup->SetReadOnly(false);
      mpAnaInSetup->SetRowHeight(15);
      mpAnaInSetup->SetVisible();
      mpAnaInSetup->SetClientArea(SUB_LIST_INDENT,0,239,15*mAnalogInputSetupListViewListDataLength-1);
      AddChild(mpAnaInSetup);
      SetCurrentChild(mpAnaInSetup);

      mpAnaInSetup->InsertColumn(0);
      mpAnaInSetup->SetColumnWidth(0,mpc1ST_COLUMN_WIDTH);
      mpAnaInSetup->InsertColumn(1);
      mpAnaInSetup->SetColumnWidth(1,mpc2ND_COLUMN_WIDTH);
      mpAnaInSetup->InsertColumn(2);
      mpAnaInSetup->SetColumnWidth(2,mpc3RD_COLUMN_WIDTH);

      for (int i = 0; i < mAnalogInputSetupListViewListDataLength; ++i)
      {
        mpAnaInSetup->InsertItem(i, (mpc::display::Component*)NULL);
      }

      for (int i = 0; i < mAnalogInputSetupListViewListDataLength ; ++i)
      {
        label = new Label;
        label->SetStringId(mpAnalogInputSetupListViewListData[i].StringId);
        label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
        label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
        label->SetLeftMargin(8);
        label->SetRightMargin(0);
        label->SetWordWrap(false);
        label->SetVisible(true);
        label->SetReadOnly(true);
        mpAnaInSetup->SetItem(i,0,label);

        mode_check_box = new ModeCheckBox;
        mode_check_box->SetCheckState(mpAnalogInputSetupListViewListData[i].CheckValue);
        mode_check_box->SetVisible(true);
        mode_check_box->SetReadOnly(false);
        mpAnaInSetup->SetItem(i,1,mode_check_box);
      }
      mpAnaInSetup->SetSelection(0);
      mpAnaInSetup->SetHelpString(SID_HELP_PIC_NO_4_3_8_1__);


      mpAnaInConfHeader = new ListView();
      mpAnaInConfHeader->SetId(12345);
      mpAnaInConfHeader->SetRowHeight(30);
      mpAnaInConfHeader->SetVisible();
      mpAnaInConfHeader->SetClientArea(0, 15*mAnalogInputSetupListViewListDataLength,239,15*mAnalogInputSetupListViewListDataLength+29);
      AddChild(mpAnaInConfHeader);

      mpAnaInConfHeader->InsertColumn(0);
      mpAnaInConfHeader->InsertColumn(1);
      mpAnaInConfHeader->SetColumnWidth(0,239);
      mpAnaInConfHeader->SetColumnWidth(1,0);

      label = new Label;
      label->SetStringId(SID_AI_MEASURED_VALUE_OF_THE_INPUT);
      label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      label->SetAlign(GUI_TA_LEFT | GUI_TA_BOTTOM);
      label->SetLeftMargin(0);
      label->SetRightMargin(0);
      label->SetWordWrap(false);
      label->SetVisible(true);
      label->SetReadOnly(true);
      label->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpAnaInConfHeader->InsertItem(0,label);

      avalible_if_not_set = new AvalibleIfNotSet;
      avalible_if_not_set->SetCheckState(SENSOR_ELECTRIC_DISABLED);
      avalible_if_not_set->SetId(12346);
      avalible_if_not_set->SetVisible();
      avalible_if_not_set->SetReadOnly(true);
      mpAnaInConfHeader->SetItem(0,1,avalible_if_not_set);


      mpAnaInConf = new ListView();
      mpAnaInConf->SetRowHeight(15);
      mpAnaInConf->SetVisible();
      mpAnaInConf->SetReadOnly(false);
      mpAnaInConf->SetClientArea(SUB_LIST_INDENT, 15*mAnalogInputSetupListViewListDataLength+30,239,15*mAnalogInputSetupListViewListDataLength+44);
      mpAnaInConf->SetHelpString(SID_HELP_PIC_NO_4_3_8_1_);
      AddChild(mpAnaInConf);

      mpAnaInConf->InsertColumn(0);
      mpAnaInConf->InsertColumn(1);
      mpAnaInConf->SetColumnWidth(0,239);
      mpAnaInConf->SetColumnWidth(1,0);
      analog_input_function_state = new AnalogInputFunctionState;
      analog_input_function_state->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      analog_input_function_state->SetAlign(GUI_TA_RIGHT | GUI_TA_VCENTER);
      analog_input_function_state->SetLeftMargin(2);
      analog_input_function_state->SetRightMargin(3);
      analog_input_function_state->SetWordWrap(false);
      analog_input_function_state->SetVisible(true);
      analog_input_function_state->SetReadOnly(true);
      analog_input_function_state->SetSelectable(true);
      analog_input_function_state->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);
      mpAnaInConf->InsertItem(0,analog_input_function_state);

      avalible_if_not_set = new AvalibleIfNotSet;
      avalible_if_not_set->SetCheckState(SENSOR_ELECTRIC_DISABLED);
      avalible_if_not_set->SetVisible(false);
      avalible_if_not_set->SetReadOnly(true);
      mpAnaInConf->SetItem(0,1,avalible_if_not_set);


      mpAnaInRangeHeader = new ListView();
      mpAnaInRangeHeader->SetRowHeight(15);
      mpAnaInRangeHeader->SetVisible();
      mpAnaInRangeHeader->SetClientArea(0, 15*mAnalogInputSetupListViewListDataLength+46,239,15*mAnalogInputSetupListViewListDataLength+60);
      AddChild(mpAnaInRangeHeader);

      mpAnaInRangeHeader->InsertColumn(0);
      mpAnaInRangeHeader->InsertColumn(1);
      mpAnaInRangeHeader->SetColumnWidth(0,239);
      mpAnaInRangeHeader->SetColumnWidth(1,0);

      label = new Label;
      label->SetStringId(SID_SENSOR_RANGE2);
      label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
      label->SetLeftMargin(0);
      label->SetRightMargin(0);
      label->SetWordWrap(false);
      label->SetVisible(true);
      label->SetReadOnly(true);
      label->SetColour(GUI_COLOUR_TEXT_HEADLINE_FOREGROUND);

      mpAnaInRangeHeader->InsertItem(0,label);
      avalible_if_not_set = new AvalibleIfNotSet;
      avalible_if_not_set->SetCheckState(MEASURED_VALUE_NOT_SELECTED);
      avalible_if_not_set->SetVisible(false);
      avalible_if_not_set->SetReadOnly(true);
      mpAnaInRangeHeader->SetItem(0,1,avalible_if_not_set);


      mpAnaInRange = new ListView();
      mpAnaInRange->SetRowHeight(15);
      mpAnaInRange->SetVisible();
      mpAnaInRange->SetClientArea(SUB_LIST_INDENT, 15*mAnalogInputSetupListViewListDataLength+61,239,15*mAnalogInputSetupListViewListDataLength+90);
      mpAnaInRange->InsertColumn(0);
      mpAnaInRange->InsertColumn(1);
      mpAnaInRange->InsertColumn(2);
      mpAnaInRange->SetColumnWidth(0,mpc1ST_COLUMN_WIDTH);
      mpAnaInRange->SetColumnWidth(1,mpc2ND_COLUMN_WIDTH);
      mpAnaInRange->SetColumnWidth(2,mpc3RD_COLUMN_WIDTH);
      mpAnaInRange->SetHelpString(SID_HELP_PIC_NO_4_3_8_1);
      AddChild(mpAnaInRange);

      label = new Label;
      label->SetStringId(SID_MINIMUM);
      label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
      label->SetLeftMargin(8);
      label->SetRightMargin(0);
      label->SetWordWrap(false);
      label->SetVisible(true);
      label->SetReadOnly(true);
      mpAnaInRange->InsertItem(0,label);

      number_quantity = new NumberQuantity;
      number_quantity->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      number_quantity->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
      number_quantity->SetLeftMargin(0);
      number_quantity->SetRightMargin(0);
      number_quantity->SetWordWrap(false);
      number_quantity->SetVisible(true);
      number_quantity->SetReadOnly(false);
      number_quantity->SetQuantityType(Q_PRESSURE);
      number_quantity->SetNumberOfDigits(4);
      mpAnaInRange->SetItem(0,1,number_quantity);

      avalible_if_not_set = new AvalibleIfNotSet;
      avalible_if_not_set->SetCheckState(MEASURED_VALUE_NOT_SELECTED);
      avalible_if_not_set->SetVisible(false);
      avalible_if_not_set->SetReadOnly(true);
      mpAnaInRange->SetItem(0,2,avalible_if_not_set);


      label = new Label;
      label->SetStringId(SID_MAXIMUM);
      label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
      label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
      label->SetLeftMargin(8);
      label->SetRightMargin(0);
      label->SetWordWrap(false);
      label->SetVisible(true);
      label->SetReadOnly(true);
      mpAnaInRange->InsertItem(1,label);

      number_quantity = new NumberQuantity;
      number_quantity->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
      number_quantity->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
      number_quantity->SetLeftMargin(0);
      number_quantity->SetRightMargin(0);
      number_quantity->SetWordWrap(false);
      number_quantity->SetVisible(true);
      number_quantity->SetReadOnly(false);
      number_quantity->SetQuantityType(Q_PRESSURE);
      number_quantity->SetNumberOfDigits(4);
      mpAnaInRange->SetItem(1,1,number_quantity);

      avalible_if_not_set = new AvalibleIfNotSet;
      avalible_if_not_set->SetCheckState(MEASURED_VALUE_NOT_SELECTED);
      avalible_if_not_set->SetVisible(false);
      avalible_if_not_set->SetReadOnly(true);
      mpAnaInRange->SetItem(1,2,avalible_if_not_set);


      // connect the list views
      mpAnaInSetup->SetPrevList(mpAnaInRange);
      mpAnaInSetup->SetNextList(mpAnaInConf);

      mpAnaInConf->SetPrevList(mpAnaInSetup);
      mpAnaInConf->SetNextList(mpAnaInRange);

      mpAnaInRange->SetPrevList(mpAnaInConf);
      mpAnaInRange->SetNextList(mpAnaInSetup);
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    AnalogInputSetupListView::~AnalogInputSetupListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void AnalogInputSetupListView::Update(Subject* Object)
    {
      Invalidate();
    }
    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void AnalogInputSetupListView::SubscribtionCancelled(Subject* pSubject)
    {
      for ( int i = 0; i < mAnalogInputSetupListViewListDataLength; ++i )
      {
        (dynamic_cast<ModeCheckBox*>(mpAnaInSetup->GetItem(i,1)))->SubscribtionCancelled(pSubject);
      }
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInConfHeader->GetItem(0,1)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInConf->GetItem(0,1)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<AnalogInputFunctionState*>(mpAnaInConf->GetItem(0,0)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRangeHeader->GetItem(0,1)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<NumberQuantity*>(mpAnaInRange->GetItem(0,1)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<NumberQuantity*>(mpAnaInRange->GetItem(1,1)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRange->GetItem(0,2)))->SubscribtionCancelled(pSubject);
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRange->GetItem(1,2)))->SubscribtionCancelled(pSubject);
    }
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void AnalogInputSetupListView::SetSubjectPointer(int Id,Subject* pSubject)
    {
      switch (Id)
      {
        case SP_AISLV_VIRTUAL_CONF_MEASURED_VALUE:
          (dynamic_cast<AnalogInputFunctionState*>(mpAnaInConf->GetItem(0,0)))->SetSubjectPointer(Id,pSubject);
          (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRangeHeader->GetItem(0,1)))->SetSubjectPointer(Id,pSubject);
          (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRange->GetItem(0,2)))->SetSubjectPointer(Id,pSubject);
          (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRange->GetItem(1,2)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_AISLV_VIRTUAL_CONF_SENSOR_ELECTRIC:
          if (mpSensorElectric.Attach(pSubject))
          {
            for ( int i = 0; i < mAnalogInputSetupListViewListDataLength; ++i )
            {
             (dynamic_cast<ModeCheckBox*>(mpAnaInSetup->GetItem(i,1)))->SetSubjectPointer(Id, mpSensorElectric.GetSubject());
            }
            (dynamic_cast<AvalibleIfNotSet*>(mpAnaInConfHeader->GetItem(0,1)))->SetSubjectPointer(Id, mpSensorElectric.GetSubject());
            (dynamic_cast<AvalibleIfNotSet*>(mpAnaInConf->GetItem(0,1)))->SetSubjectPointer(Id, mpSensorElectric.GetSubject());
          }
          break;
        case SP_AISLV_VIRTUAL_CONF_SENSOR_MIN_VALUE:
          if (mpSensorMin.Attach(pSubject))
          {
            (dynamic_cast<NumberQuantity*>(mpAnaInRange->GetItem(0,1)))->SetSubjectPointer(Id, mpSensorMin.GetSubject());
          }
          break;
        case SP_AISLV_VIRTUAL_CONF_SENSOR_MAX_VALUE:
          if (mpSensorMax.Attach(pSubject))
          {
            (dynamic_cast<NumberQuantity*>(mpAnaInRange->GetItem(1,1)))->SetSubjectPointer(Id, mpSensorMax.GetSubject());
          }
          break;
      }
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void AnalogInputSetupListView::ConnectToSubjects(void)
    {
      for ( int i = 0; i < mAnalogInputSetupListViewListDataLength; ++i )
      {
        (dynamic_cast<ModeCheckBox*>(mpAnaInSetup->GetItem(i,1)))->ConnectToSubjects();
      }
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInConfHeader->GetItem(0,1)))->ConnectToSubjects();
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInConf->GetItem(0,1)))->ConnectToSubjects();
      (dynamic_cast<AnalogInputFunctionState*>(mpAnaInConf->GetItem(0,0)))->ConnectToSubjects();
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRangeHeader->GetItem(0,1)))->ConnectToSubjects();
      (dynamic_cast<NumberQuantity*>(mpAnaInRange->GetItem(0,1)))->ConnectToSubjects();
      (dynamic_cast<NumberQuantity*>(mpAnaInRange->GetItem(1,1)))->ConnectToSubjects();
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRange->GetItem(0,2)))->ConnectToSubjects();
      (dynamic_cast<AvalibleIfNotSet*>(mpAnaInRange->GetItem(1,2)))->ConnectToSubjects();
    }
    /* --------------------------------------------------
    * Sets the font of this text element
    * --------------------------------------------------*/
    void AnalogInputSetupListView::SetFont(const GUI_FONT** Font)
    {
      for ( int i = 0; i < mAnalogInputSetupListViewListDataLength; i++ )
      {
        ((Label*)(mpAnaInSetup->GetItem(i,0)))->SetFont(Font);
      }
    }


    /* --------------------------------------------------
    * Sets the component to be displayed when this tab
    * is active.
    * --------------------------------------------------*/
    void AnalogInputSetupListView::SetDisplay(Display* pDisplay)
    {
      (dynamic_cast<AnalogInputFunctionState*>(mpAnaInConf->GetItem(0,0)))->SetDisplay(pDisplay);
    }

    void AnalogInputSetupListView::SetNextList(ListView* pList)
    {
      mpAnaInRange->SetNextList(pList);
    }

    void AnalogInputSetupListView::SetPrevList(ListView* pList)
    {
      mpAnaInSetup->SetPrevList(pList);
    }

    bool AnalogInputSetupListView::SetSelection(int itemIndex)
    {
      if (mCurrentChild == NULL && itemIndex != -1)
        SetCurrentChild(mpAnaInSetup);

      return mpAnaInSetup->SetSelection(itemIndex);
    }

    ListView* AnalogInputSetupListView::GetFirstListView()
    {
      return mpAnaInSetup;
    }

    ListView* AnalogInputSetupListView::GetLastListView()
    {
      return mpAnaInRange;
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

  } // namespace display
} // namespace mpc


