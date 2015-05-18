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
/* CLASS NAME       : PTCInputConfListView                                  */
/*                                                                          */
/* FILE NAME        : PTCInputConfListView.CPP                              */
/*                                                                          */
/* CREATED DATE     : 2012-02-16                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class implements the ptc input configuration ListView               */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <DisplayTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "PTCInputConfListView.h"
#include <MPCFonts.h>
#include "ModeCheckBox.h"
#include "Label.h"
#include "AvalibleIfSet.h"

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
      DEFINES
    *****************************************************************************/

    const MODE_CHECK_BOX_LABEL_VALUE LIST_VIEW_DATA[] =
    {
      { SID_PTC_NOT_USED         ,    PTC_INPUT_FUNC_NO_FUNCTION      },
      { SID_PTC_1_PUMP_1         ,    PTC_INPUT_FUNC_PTC_1_PUMP_1     },
      { SID_PTC_2_PUMP_1         ,    PTC_INPUT_FUNC_PTC_2_PUMP_1     },
      { SID_PTC_MOISTURE_PUMP_1  ,    PTC_INPUT_FUNC_MOISTURE_PUMP_1  },
      { SID_PTC_1_PUMP_2         ,    PTC_INPUT_FUNC_PTC_1_PUMP_2     },
      { SID_PTC_2_PUMP_2         ,    PTC_INPUT_FUNC_PTC_2_PUMP_2     },
      { SID_PTC_MOISTURE_PUMP_2  ,    PTC_INPUT_FUNC_MOISTURE_PUMP_2  },
      { SID_PTC_1_PUMP_3         ,    PTC_INPUT_FUNC_PTC_1_PUMP_3     },
      { SID_PTC_2_PUMP_3         ,    PTC_INPUT_FUNC_PTC_2_PUMP_3     },
      { SID_PTC_MOISTURE_PUMP_3  ,    PTC_INPUT_FUNC_MOISTURE_PUMP_3  },
      { SID_PTC_1_PUMP_4         ,    PTC_INPUT_FUNC_PTC_1_PUMP_4     },
      { SID_PTC_2_PUMP_4         ,    PTC_INPUT_FUNC_PTC_2_PUMP_4     },
      { SID_PTC_MOISTURE_PUMP_4  ,    PTC_INPUT_FUNC_MOISTURE_PUMP_4  },      
      { SID_PTC_1_PUMP_5         ,    PTC_INPUT_FUNC_PTC_1_PUMP_5     },
      { SID_PTC_2_PUMP_5         ,    PTC_INPUT_FUNC_PTC_2_PUMP_5     },
      { SID_PTC_MOISTURE_PUMP_5  ,    PTC_INPUT_FUNC_MOISTURE_PUMP_5  },
      { SID_PTC_1_PUMP_6         ,    PTC_INPUT_FUNC_PTC_1_PUMP_6     },
      { SID_PTC_2_PUMP_6         ,    PTC_INPUT_FUNC_PTC_2_PUMP_6     },
      { SID_PTC_MOISTURE_PUMP_6  ,    PTC_INPUT_FUNC_MOISTURE_PUMP_6  }
    };

    const int LIST_VIEW_DATA_CNT = sizeof(LIST_VIEW_DATA) / sizeof(LIST_VIEW_DATA[0]);

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
    PTCInputConfListView::PTCInputConfListView(Component* pParent /*= NULL*/) : ListView(pParent)
    {
      Label*         p_label;
      ModeCheckBox*  p_mode_check_box;

      InsertColumn(COLUMN_LABEL);
      InsertColumn(COLUMN_CHECK_BOX);
      InsertColumn(COLUMN_AVAILABLE_PUMPS); // availability check against number of pumps for pump related inputs

      SetColumnWidth(COLUMN_LABEL, 175);
      SetColumnWidth(COLUMN_CHECK_BOX, 64);
      SetColumnWidth(COLUMN_AVAILABLE_PUMPS, 0);

      int number_of_label_and_check_box_rows = LIST_VIEW_DATA_CNT;

      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        InsertItem(i, (mpc::display::Component*)NULL);

        int check_value = LIST_VIEW_DATA[i].CheckValue;

        if (i < number_of_label_and_check_box_rows)
        {
          p_label = new Label;
          p_label->SetStringId(LIST_VIEW_DATA[i].StringId);
          p_label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
          p_label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
          p_label->SetLeftMargin(8);
          p_label->SetRightMargin(0);
          p_label->SetWordWrap(false);
          p_label->SetVisible(true);
          p_label->SetReadOnly(true);
          p_label->Invalidate();
          SetItem(i, COLUMN_LABEL, p_label);

          p_mode_check_box = new ModeCheckBox;
          p_mode_check_box->SetCheckState(check_value);
          p_mode_check_box->SetVisible(true);
          p_mode_check_box->SetReadOnly(false);
          p_mode_check_box->Invalidate();
          SetItem(i, COLUMN_CHECK_BOX, p_mode_check_box);
        }
        AddAvailableIfSets(i, check_value);
      }    
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    PTCInputConfListView::~PTCInputConfListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void PTCInputConfListView::Update(Subject* Object)
    {

    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void PTCInputConfListView::SubscribtionCancelled(Subject* pSubject)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->SubscribtionCancelled(pSubject);        

        if (GetItem(i, COLUMN_AVAILABLE_PUMPS))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS)))->SubscribtionCancelled(pSubject);
        }
      }
    }
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void PTCInputConfListView::SetSubjectPointer(int id, Subject* pSubject)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        int check_value = LIST_VIEW_DATA[i].CheckValue;

        switch (id)
        {
          case SP_PTCCLV_FUNCTION:
            {
              ModeCheckBox* p_check_box = (ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX));
              if (p_check_box != NULL)
              {
                p_check_box->SetSubjectPointer(id, pSubject);
              }             
            }
            break;

          case SP_PTCCLV_NO_OF_PUMPS:
            {
              AvalibleIfSet* p_available = (AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS));
              if (p_available != NULL && 
                  (check_value == PTC_INPUT_FUNC_PTC_1_PUMP_1    ||
                   check_value == PTC_INPUT_FUNC_PTC_2_PUMP_1    ||
                   check_value == PTC_INPUT_FUNC_MOISTURE_PUMP_1 ||
                   check_value == PTC_INPUT_FUNC_PTC_1_PUMP_2    ||
                   check_value == PTC_INPUT_FUNC_PTC_2_PUMP_2    ||
                   check_value == PTC_INPUT_FUNC_MOISTURE_PUMP_2 ||
                   check_value == PTC_INPUT_FUNC_PTC_1_PUMP_3    ||
                   check_value == PTC_INPUT_FUNC_PTC_2_PUMP_3    ||
                   check_value == PTC_INPUT_FUNC_MOISTURE_PUMP_3 ||
                   check_value == PTC_INPUT_FUNC_PTC_1_PUMP_4    ||
                   check_value == PTC_INPUT_FUNC_PTC_2_PUMP_4    ||
                   check_value == PTC_INPUT_FUNC_MOISTURE_PUMP_4 ||
                   check_value == PTC_INPUT_FUNC_PTC_1_PUMP_5    ||
                   check_value == PTC_INPUT_FUNC_PTC_2_PUMP_5    ||
                   check_value == PTC_INPUT_FUNC_MOISTURE_PUMP_5 ||
                   check_value == PTC_INPUT_FUNC_PTC_1_PUMP_6    ||
                   check_value == PTC_INPUT_FUNC_PTC_2_PUMP_6    ||
                   check_value == PTC_INPUT_FUNC_MOISTURE_PUMP_6))
              {
                p_available->SetSubjectPointer(id,pSubject);
              }
            }
            break;
        }
      }
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void PTCInputConfListView::ConnectToSubjects(void)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ModeCheckBox* p_mode_check_box = (ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX));

        if (p_mode_check_box != NULL)
        {
          p_mode_check_box->ConnectToSubjects();
        }

        if (GetItem(i, COLUMN_AVAILABLE_PUMPS))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS)))->ConnectToSubjects();
        }
      }
    }

    /* --------------------------------------------------
    * Sets the font of this text element
    * --------------------------------------------------*/
    void PTCInputConfListView::SetFont(const GUI_FONT** Font)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        Label* p_label = (Label*) GetItem(i, COLUMN_LABEL);

        if (p_label != NULL)
        {
          p_label->SetFont(Font);
        }
      }
    }

    
    void PTCInputConfListView::AddAvailableIfSets(int rowIndex, int checkValue)
    {
        if (checkValue == PTC_INPUT_FUNC_PTC_1_PUMP_2 || 
            checkValue == PTC_INPUT_FUNC_PTC_2_PUMP_2 || 
            checkValue == PTC_INPUT_FUNC_MOISTURE_PUMP_2)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (checkValue == PTC_INPUT_FUNC_PTC_1_PUMP_3 || 
                 checkValue == PTC_INPUT_FUNC_PTC_2_PUMP_3 || 
                 checkValue == PTC_INPUT_FUNC_MOISTURE_PUMP_3)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          p_available_if_set->AddCheckState(2);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (checkValue == PTC_INPUT_FUNC_PTC_1_PUMP_4 ||
                 checkValue == PTC_INPUT_FUNC_PTC_2_PUMP_4 || 
                 checkValue == PTC_INPUT_FUNC_MOISTURE_PUMP_4)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(4);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (checkValue == PTC_INPUT_FUNC_PTC_1_PUMP_5 || 
                 checkValue == PTC_INPUT_FUNC_PTC_2_PUMP_5 ||
                 checkValue == PTC_INPUT_FUNC_MOISTURE_PUMP_5)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (checkValue == PTC_INPUT_FUNC_PTC_1_PUMP_6 ||
                 checkValue == PTC_INPUT_FUNC_PTC_2_PUMP_6 ||
                 checkValue == PTC_INPUT_FUNC_MOISTURE_PUMP_6)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(6);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
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
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/

  } // namespace display
} // namespace mpc


