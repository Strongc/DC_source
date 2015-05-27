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
/* FILE NAME        : AnalogInputConfListView.CPP                           */
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
#include <AppTypeDefs.h>
#include <DisplayTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "AnalogInputConfListView.h"
#include "ModeCheckBox.h"
#include "Label.h"
#include "AvalibleIfSet.h"
#include <MPCFonts.h>


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
    #define mpc1ST_COLUMN_WIDTH 192
    #define mpc2ND_COLUMN_WIDTH 34
    #define mpc3TH_COLUMN_WIDTH 0


    MODE_CHECK_BOX_LABEL_VALUE LIST_VIEW_DATA[] =
    {
      { SID_AI_NOT_USED_DISABLED,                 MEASURED_VALUE_NOT_SELECTED             },
      { SID_AI_FLOW,                              MEASURED_VALUE_FLOW,                    },
      { SID_AI_LEVEL_ULTRA_SONIC,                 MEASURED_VALUE_LEVEL_ULTRA_SOUND        },
      { SID_AI_LEVEL_PRESSURE,                    MEASURED_VALUE_LEVEL_PRESSURE           },
      { SID_AI_PRESSURE_SENSOR_OUTLET_PIPELINE,   MEASURED_VALUE_OUTLET_PRESSURE          },
      { SID_AI_MOTOR_CURRENT_PUMP_1,              MEASURED_VALUE_MOTOR_CURRENT_PUMP_1     },
      { SID_AI_MOTOR_CURRENT_PUMP_2,              MEASURED_VALUE_MOTOR_CURRENT_PUMP_2     },
      { SID_MOTOR_CURRENT_PUMP_3,                 MEASURED_VALUE_MOTOR_CURRENT_PUMP_3     },
      { SID_MOTOR_CURRENT_PUMP_4,                 MEASURED_VALUE_MOTOR_CURRENT_PUMP_4     },
      { SID_MOTOR_CURRENT_PUMP_5,                 MEASURED_VALUE_MOTOR_CURRENT_PUMP_5     },
      { SID_MOTOR_CURRENT_PUMP_6,                 MEASURED_VALUE_MOTOR_CURRENT_PUMP_6     },
      { SID_AI_WATER_IN_OIL_PUMP_1,               MEASURED_VALUE_WATER_IN_OIL_PUMP_1      },
      { SID_AI_WATER_IN_OIL_PUMP_2,               MEASURED_VALUE_WATER_IN_OIL_PUMP_2      },
      { SID_WATER_IN_OIL_PUMP_3,                  MEASURED_VALUE_WATER_IN_OIL_PUMP_3      },
      { SID_WATER_IN_OIL_PUMP_4,                  MEASURED_VALUE_WATER_IN_OIL_PUMP_4      },
      { SID_WATER_IN_OIL_PUMP_5,                  MEASURED_VALUE_WATER_IN_OIL_PUMP_5      },
      { SID_WATER_IN_OIL_PUMP_6,                  MEASURED_VALUE_WATER_IN_OIL_PUMP_6      },
      { SID_AI_POWER,                             MEASURED_VALUE_POWER                    },
      { SID_AI_POWER_PUMP_1,                      MEASURED_VALUE_POWER_PUMP_1             },
      { SID_AI_POWER_PUMP_2,                      MEASURED_VALUE_POWER_PUMP_2             },
      { SID_AI_POWER_PUMP_3,                      MEASURED_VALUE_POWER_PUMP_3             },
      { SID_AI_POWER_PUMP_4,                      MEASURED_VALUE_POWER_PUMP_4             },
      { SID_AI_POWER_PUMP_5,                      MEASURED_VALUE_POWER_PUMP_5             },
      { SID_AI_POWER_PUMP_6,                      MEASURED_VALUE_POWER_PUMP_6             },      
      { SID_AI_OPTIONAL_SENSOR_1,                 MEASURED_VALUE_USER_DEFINED_SOURCE_1    },
      { SID_AI_OPTIONAL_SENSOR_2,                 MEASURED_VALUE_USER_DEFINED_SOURCE_2    },
      { SID_AI_OPTIONAL_SENSOR_3,                 MEASURED_VALUE_USER_DEFINED_SOURCE_3    },
      { SID_AI_LEVEL_CHEMICAL_CONTAINER,          MEASURED_VALUE_CHEMICAL_CONTAINER       }
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
    AnalogInputConfListView::AnalogInputConfListView(Component* pParent /*= NULL*/) : ListView(pParent)
    {
      Label*         p_label;
      ModeCheckBox*  p_mode_check_box;
      AvalibleIfSet* p_available_if_set;
      int check_value;

      InsertColumn(COLUMN_LABEL);
      InsertColumn(COLUMN_CHECK_BOX);
      InsertColumn(COLUMN_AVAILABLE_PUMPS); // availability check against number of pumps for pump related inputs

      SetColumnWidth(COLUMN_LABEL, mpc1ST_COLUMN_WIDTH);
      SetColumnWidth(COLUMN_CHECK_BOX, mpc2ND_COLUMN_WIDTH);      
      SetColumnWidth(COLUMN_AVAILABLE_PUMPS, mpc3TH_COLUMN_WIDTH);

      for (int i = 0; i < LIST_VIEW_DATA_CNT ; i++)
      {
        InsertItem(i, (mpc::display::Component*)NULL);
      }

      for (int i = 0; i < LIST_VIEW_DATA_CNT ; i++)
      {
        check_value = LIST_VIEW_DATA[i].CheckValue;

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

        if (check_value == MEASURED_VALUE_MOTOR_CURRENT_PUMP_2
          || check_value == MEASURED_VALUE_WATER_IN_OIL_PUMP_2
          || check_value == MEASURED_VALUE_POWER_PUMP_2)
        {
          p_available_if_set = new AvalibleIfSet();
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          SetItem(i, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (check_value == MEASURED_VALUE_MOTOR_CURRENT_PUMP_3
          || check_value == MEASURED_VALUE_WATER_IN_OIL_PUMP_3
          || check_value == MEASURED_VALUE_POWER_PUMP_3)
        {
          p_available_if_set = new AvalibleIfSet();
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          p_available_if_set->AddCheckState(2);
          SetItem(i, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (check_value == MEASURED_VALUE_MOTOR_CURRENT_PUMP_4
          || check_value == MEASURED_VALUE_WATER_IN_OIL_PUMP_4
          || check_value == MEASURED_VALUE_POWER_PUMP_4)
        {
          p_available_if_set = new AvalibleIfSet();
          p_available_if_set->AddCheckState(4);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(i, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (check_value == MEASURED_VALUE_MOTOR_CURRENT_PUMP_5
          || check_value == MEASURED_VALUE_WATER_IN_OIL_PUMP_5
          || check_value == MEASURED_VALUE_POWER_PUMP_5)
        {
          p_available_if_set = new AvalibleIfSet();
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(i, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if (check_value == MEASURED_VALUE_MOTOR_CURRENT_PUMP_6
          || check_value == MEASURED_VALUE_WATER_IN_OIL_PUMP_6
          || check_value == MEASURED_VALUE_POWER_PUMP_6)
        {
          p_available_if_set = new AvalibleIfSet();
          p_available_if_set->AddCheckState(6);
          SetItem(i, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }

      }
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    AnalogInputConfListView::~AnalogInputConfListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void AnalogInputConfListView::Update(Subject* Object)
    {

    }
    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void AnalogInputConfListView::SubscribtionCancelled(Subject* pSubject)
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
    void AnalogInputConfListView::SetSubjectPointer(int Id, Subject* pSubject)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        switch(Id)
        {
          case SP_AICLV_FUNCTION:
            ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->SetSubjectPointer(Id, pSubject);
            break;

          case SP_AICLV_NO_OF_PUMPS:
            if (GetItem(i, COLUMN_AVAILABLE_PUMPS))
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS)))->SetSubjectPointer(Id, pSubject);
            }
            break;
        }

        
      }
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void AnalogInputConfListView::ConnectToSubjects(void)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->ConnectToSubjects();

        if (GetItem(i, COLUMN_AVAILABLE_PUMPS))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS)))->ConnectToSubjects();
        }
      }
    }
    /* --------------------------------------------------
    * Sets the font of this text element
    * --------------------------------------------------*/
    void AnalogInputConfListView::SetFont(const GUI_FONT** Font)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ((Label*)(GetItem(i, COLUMN_LABEL)))->SetFont(Font);
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


