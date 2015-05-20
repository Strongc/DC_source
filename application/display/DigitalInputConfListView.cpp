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
/* CLASS NAME       : DigitalInputConfListView                              */
/*                                                                          */
/* FILE NAME        : DigitalInputConfListView.CPP                          */
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
#include "DigitalInputConfListView.h"
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
      { SID_FLOAT_SWITCH_1                , DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1              },
      { SID_FLOAT_SWITCH_2                , DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2              },
      { SID_FLOAT_SWITCH_3                , DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3              },
      { SID_FLOAT_SWITCH_4                , DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4              },
      { SID_FLOAT_SWITCH_5                , DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5              },
      { SID_DI_OVERFLOW_SWITCH            , DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH             },
      { SID_DI_NOT_USED                   , DIGITAL_INPUT_FUNC_NO_FUNCTION                 },
      { SID_DI_AUTO_MAN_PUMP_1            , DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_1             },
      { SID_DI_MAN_START_PUMP_1           , DIGITAL_INPUT_FUNC_ON_OFF_PUMP_1               },
      { SID_DI_AUTO_MAN_PUMP_2            , DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2             },
      { SID_DI_MAN_START_PUMP_2           , DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2               },
      { SID_AUTOMATIC_MANUAL_PUMP_3       , DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3             },
      { SID_MANUAL_START_PUMP_3           , DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3               },
      { SID_AUTOMATIC_MANUAL_PUMP_4       , DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4             },
      { SID_MANUAL_START_PUMP_4           , DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4               },
      { SID_AUTOMATIC_MANUAL_PUMP_5       , DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5             },
      { SID_MANUAL_START_PUMP_5           , DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5               },
      { SID_AUTOMATIC_MANUAL_PUMP_6       , DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6             },
      { SID_MANUAL_START_PUMP_6           , DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6               },
      { SID_DI_EXTERNAL_FAULT             , DIGITAL_INPUT_FUNC_EXTERNAL_FAULT              },
      { SID_DI_ALARM_RESET                , DIGITAL_INPUT_FUNC_ALARM_RESET                 },
      { SID_DI_ALARM_RELAY_RESET          , DIGITAL_INPUT_FUNC_ALARM_RELAY_RESET           },
      { SID_DI_CONTACTOR_FEEDBACK_PUMP_1  , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_1   },
      { SID_DI_CONTACTOR_FEEDBACK_PUMP_2  , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2   },
      { SID_CONTACTOR_FEEDBACK_PUMP_3     , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3   },
      { SID_CONTACTOR_FEEDBACK_PUMP_4     , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4   },
      { SID_CONTACTOR_FEEDBACK_PUMP_5     , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5   },
      { SID_CONTACTOR_FEEDBACK_PUMP_6     , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6   },
      { SID_DI_CONTACTOR_FEEDBACK_MIXER   , DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_MIXER    },
      { SID_DI_FAIL_PHASE                 , DIGITAL_INPUT_FUNC_FAIL_PHASE                  },
      { SID_DI_ENERGY_COUNTER             , DIGITAL_INPUT_FUNC_ENERGY_CNT                  },
      { SID_DI_VOLUME_COUNTER             , DIGITAL_INPUT_FUNC_VOLUME_CNT                  },
      { SID_DI_MOTOR_PROTECTION_PUMP_1    , DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_1     },
      { SID_DI_MOTOR_PROTECTION_PUMP_2    , DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2     },
      { SID_MOTOR_PROTECTION_PUMP_3       , DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3     },
      { SID_MOTOR_PROTECTION_PUMP_4       , DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4     },
      { SID_MOTOR_PROTECTION_PUMP_5       , DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5     },
      { SID_MOTOR_PROTECTION_PUMP_6       , DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6     },
      { SID_DI_INTERLOCK                  , DIGITAL_INPUT_FUNC_INTERLOCK                   },
      { SID_DI_VFD_READY_PUMP_1           , DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1            },
      { SID_DI_VFD_READY_PUMP_2           , DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2            },
      { SID_DI_VFD_READY_PUMP_3           , DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3            },
      { SID_DI_VFD_READY_PUMP_4           , DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4            },
      { SID_DI_VFD_READY_PUMP_5           , DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5            },
      { SID_DI_VFD_READY_PUMP_6           , DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6            },
      { SID_DI_GAS_DETECTOR               , DIGITAL_INPUT_FUNC_GAS_DETECTOR                },
      { SID_DI_WATER_ON_PIT_FLOOR         , DIGITAL_INPUT_FUNC_WATER_ON_PIT_FLOOR          },
      { SID_DI_SERVICE_MODE               , DIGITAL_INPUT_FUNC_SERVICE_MODE                },
      { SID_DOSING_PUMP_READY             , DIGITAL_INPUT_FUNC_DOSING_PUMP                 },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_EXTRA_FAULT_1               }, // The last 8 rows uses datapointtext and multistring
      { SID_NAME_OF_UDF                   , DIGITAL_INPUT_FUNC_EXTRA_FAULT_1               },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_EXTRA_FAULT_2               },
      { SID_NAME_OF_UDF                   , DIGITAL_INPUT_FUNC_EXTRA_FAULT_2               },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_EXTRA_FAULT_3               },
      { SID_NAME_OF_UDF                   , DIGITAL_INPUT_FUNC_EXTRA_FAULT_3               },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_EXTRA_FAULT_4               },
      { SID_NAME_OF_UDF                   , DIGITAL_INPUT_FUNC_EXTRA_FAULT_4               },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1            },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2            },
      { SID_NONE                          , DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3            }      
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
    DigitalInputConfListView::DigitalInputConfListView(Component* pParent /*= NULL*/) : ListView(pParent)
    {
      Label*         p_label;
      ModeCheckBox*  p_mode_check_box;

      InsertColumn(COLUMN_LABEL);
      InsertColumn(COLUMN_EDIT_NAME);
      InsertColumn(COLUMN_CHECK_BOX);
      InsertColumn(COLUMN_AVAILABLE_FSW);// availability check against selected function
      InsertColumn(COLUMN_AVAILABLE_PUMPS);// availability check against selected DI for counter inputs,
                                           // or availability check against number of pumps for pump related inputs
      InsertColumn(COLUMN_VFD_INSTALLED);

      SetColumnWidth(COLUMN_LABEL, 75);
      SetColumnWidth(COLUMN_EDIT_NAME, 100);
      SetColumnWidth(COLUMN_CHECK_BOX, 64);
      SetColumnWidth(COLUMN_AVAILABLE_FSW, 0);
      SetColumnWidth(COLUMN_AVAILABLE_PUMPS, 0);
      SetColumnWidth(COLUMN_VFD_INSTALLED, 0);

      int number_of_label_and_check_box_rows = LIST_VIEW_DATA_CNT - ((NO_OF_EXTRA_FAULTS * 2)+ NO_OF_USD_COUNTERS);

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

      CreateExtraFaultRows(number_of_label_and_check_box_rows); 
      CreateUserDefinedCounterRows(number_of_label_and_check_box_rows + (NO_OF_EXTRA_FAULTS * 2));      
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    DigitalInputConfListView::~DigitalInputConfListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void DigitalInputConfListView::Update(Subject* Object)
    {

    }

    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void DigitalInputConfListView::SubscribtionCancelled(Subject* pSubject)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->SubscribtionCancelled(pSubject);
        ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_FSW)))->SubscribtionCancelled(pSubject);

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
    void DigitalInputConfListView::SetSubjectPointer(int id, Subject* pSubject)
    {
      switch (id)
      {
        case SP_DICLV_NAME_EXTRA_FAULT_1:
          mpExtraEditName[0]->SetSubjectPointer(id, pSubject);
          mpExtraShowName[0]->SetSubjectPointer(id, pSubject);
          return;
        case SP_DICLV_NAME_EXTRA_FAULT_2:
          mpExtraEditName[1]->SetSubjectPointer(id, pSubject);
          mpExtraShowName[1]->SetSubjectPointer(id, pSubject);
          return;
        case SP_DICLV_NAME_EXTRA_FAULT_3:
          mpExtraEditName[2]->SetSubjectPointer(id, pSubject);
          mpExtraShowName[2]->SetSubjectPointer(id, pSubject);
          return;
        case SP_DICLV_NAME_EXTRA_FAULT_4:
          mpExtraEditName[3]->SetSubjectPointer(id, pSubject);
          mpExtraShowName[3]->SetSubjectPointer(id, pSubject);
          return;
        case SP_DICLV_NAME_USD_CNT_1:
          mpUSDCounterName[0]->SetSubjectPointer(id, pSubject);
          return;
        case SP_DICLV_NAME_USD_CNT_2:
          mpUSDCounterName[1]->SetSubjectPointer(id, pSubject);
          return;
        case SP_DICLV_NAME_USD_CNT_3:
          mpUSDCounterName[2]->SetSubjectPointer(id, pSubject);
          return;
      }

      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        int check_value = LIST_VIEW_DATA[i].CheckValue;

        switch (id)
        {
          case SP_DICLV_FUNCTION:
            {
              ModeCheckBox* p_check_box = (ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX));
              if (p_check_box != NULL)
              {
                p_check_box->SetSubjectPointer(id, pSubject);
              }
              ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_FSW)))->SetSubjectPointer(id, pSubject);
            }
            break;

          case SP_DICLV_SLIP_POINT_NO:
            {
              AvalibleIfSet* p_available = (AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS));
              if (p_available != NULL
                && (check_value == DIGITAL_INPUT_FUNC_ENERGY_CNT
                || check_value == DIGITAL_INPUT_FUNC_VOLUME_CNT
                || check_value == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
                || check_value == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
                || check_value == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3))
              {
                p_available->SetSubjectPointer(id, pSubject);
              }
            }
            break;

          case SP_DICLV_NO_OF_PUMPS:
            {
              AvalibleIfSet* p_available = (AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS));
              if (p_available != NULL
                && (check_value == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2
                || check_value == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3
                || check_value == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4
                || check_value == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5
                || check_value == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6
                || check_value == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2
                || check_value == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3
                || check_value == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4
                || check_value == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5
                || check_value == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6
                || check_value == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2
                || check_value == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3
                || check_value == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4
                || check_value == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5
                || check_value == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6
                || check_value == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2
                || check_value == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3
                || check_value == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4
                || check_value == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5
                || check_value == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6
                || check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2
                || check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3
                || check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4
                || check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5
                || check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6))
              {
                p_available->SetSubjectPointer(id,pSubject);
              }
            }
            break;

          case SP_DICLV_VFD_1_INSTALLED:
            if (GetItem(i, COLUMN_VFD_INSTALLED) && check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1)
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_VFD_INSTALLED)))->SetSubjectPointer(id, pSubject);
            }
            break;

          case SP_DICLV_VFD_2_INSTALLED:
            if (GetItem(i, COLUMN_VFD_INSTALLED) && check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2)
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_VFD_INSTALLED)))->SetSubjectPointer(id, pSubject);
            }
            break;

          case SP_DICLV_VFD_3_INSTALLED:
            if (GetItem(i, COLUMN_VFD_INSTALLED) && check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3)
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_VFD_INSTALLED)))->SetSubjectPointer(id, pSubject);
            }
            break;

          case SP_DICLV_VFD_4_INSTALLED:
            if (GetItem(i, COLUMN_VFD_INSTALLED) && check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4)
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_VFD_INSTALLED)))->SetSubjectPointer(id, pSubject);
            }
            break;

          case SP_DICLV_VFD_5_INSTALLED:
            if (GetItem(i, COLUMN_VFD_INSTALLED) && check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5)
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_VFD_INSTALLED)))->SetSubjectPointer(id, pSubject);
            }
            break;

          case SP_DICLV_VFD_6_INSTALLED:
            if (GetItem(i, COLUMN_VFD_INSTALLED) && check_value == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6)
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_VFD_INSTALLED)))->SetSubjectPointer(id, pSubject);
            }
            break;
        }
      }
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void DigitalInputConfListView::ConnectToSubjects(void)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ModeCheckBox* p_mode_check_box = (ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX));

        if (p_mode_check_box != NULL)
        {
          p_mode_check_box->ConnectToSubjects();
        }

        ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_FSW)))->ConnectToSubjects();

        if (GetItem(i, COLUMN_AVAILABLE_PUMPS))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE_PUMPS)))->ConnectToSubjects();
        }
      }

      for (int i = 0; i < NO_OF_EXTRA_FAULTS; i++)
      {
        mpExtraShowName[i]->ConnectToSubjects();
        mpExtraEditName[i]->ConnectToSubjects();
      }
      for (int i = 0; i < NO_OF_USD_COUNTERS; i++)
      {
        mpUSDCounterName[i]->ConnectToSubjects();       
      }
    }

    /* --------------------------------------------------
    * Sets the font of this text element
    * --------------------------------------------------*/
    void DigitalInputConfListView::SetFont(const GUI_FONT** Font)
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

    
    void DigitalInputConfListView::AddAvailableIfSets(int rowIndex, int checkValue)
    {
      // compare selected function with current selected funtion
        AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
        p_available_if_set->SetReadOnly(false);

        // if current function is a float switch make sure that float switch is shown when selected
        if      (checkValue == DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1)
        {
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1);
        }
        else if (checkValue == DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2)
        {
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2);
        }
        else if (checkValue == DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3)
        {
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3);
        }
        else if (checkValue == DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4)
        {
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4);
        }
        else if (checkValue == DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5)
        {
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5);
        }
        else if (checkValue == DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH)
        {
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH);
        }
        else
        {
          //    if current function is a float switch hide all other rows
          // (= if current function is not a float switch show all other rows)
          p_available_if_set->Invert();

          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_1);
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_2);
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_3);
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_4);
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_FLOAT_SWITCH_5);
          p_available_if_set->AddCheckState(DIGITAL_INPUT_FUNC_OVERFLOW_SWITCH);
        }

        SetItem(rowIndex, COLUMN_AVAILABLE_FSW, p_available_if_set);

        // compare selected DI with state id of DI 1, DI 2 (IO351)
        if (checkValue == DIGITAL_INPUT_FUNC_ENERGY_CNT
          || checkValue == DIGITAL_INPUT_FUNC_VOLUME_CNT
          || checkValue == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1
          || checkValue == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_2
          || checkValue == DIGITAL_INPUT_FUNC_USERDEFINE_CNT_3 )
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(4);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(13);
          p_available_if_set->AddCheckState(14);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if(checkValue == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_2
          || checkValue == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_2
          || checkValue == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_2
          || checkValue == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_2
          || checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if(checkValue == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_3
          || checkValue == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_3
          || checkValue == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_3
          || checkValue == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_3
          || checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          p_available_if_set->AddCheckState(2);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if(checkValue == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_4
          || checkValue == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_4
          || checkValue == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_4
          || checkValue == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_4
          || checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(4);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if(checkValue == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_5
          || checkValue == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_5
          || checkValue == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_5
          || checkValue == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_5
          || checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }
        else if(checkValue == DIGITAL_INPUT_FUNC_AUTO_MAN_PUMP_6
          || checkValue == DIGITAL_INPUT_FUNC_ON_OFF_PUMP_6
          || checkValue == DIGITAL_INPUT_FUNC_CONTACTOR_FEEDBACK_PUMP_6
          || checkValue == DIGITAL_INPUT_FUNC_MOTOR_PROTECTION_PUMP_6
          || checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(6);
          SetItem(rowIndex, COLUMN_AVAILABLE_PUMPS, p_available_if_set);
        }


        if (checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_1 ||
            checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_2 ||
            checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_3 ||
            checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_4 ||
            checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_5 ||
            checkValue == DIGITAL_INPUT_FUNC_VFD_READY_PUMP_6)
        {
          AvalibleIfSet* p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(1);
          SetItem(rowIndex, COLUMN_VFD_INSTALLED, p_available_if_set);
        }

    }

    void DigitalInputConfListView::CreateExtraFaultRows(int firstRowIndex)
    {    
      int index = firstRowIndex;

      for (int extra = 0; extra < NO_OF_EXTRA_FAULTS; extra++)
      {
        mpExtraShowName[extra] = new DataPointText();
        mpExtraShowName[extra]->SetReadOnly();
        mpExtraShowName[extra]->SetLeftMargin(8);
        mpExtraShowName[extra]->SetVisible(true);

        ModeCheckBox* p_mode_check_box = new ModeCheckBox();
        p_mode_check_box->SetCheckState(DIGITAL_INPUT_FUNC_EXTRA_FAULT_1 + extra);
        p_mode_check_box->SetReadOnly(false);
        p_mode_check_box->SetVisible(true);

        
        SetItem(index, COLUMN_LABEL, mpExtraShowName[extra]);
        SetItem(index, COLUMN_CHECK_BOX, p_mode_check_box);

        
        Label* p_label = new Label();
        p_label->SetStringId(SID_NAME_OF_UDF);
        p_label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
        p_label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
        p_label->SetLeftMargin(16);
        p_label->SetReadOnly(true);
        p_label->SetVisible(true);

        mpExtraEditName[extra] = new MultiString(this);
        mpExtraEditName[extra]->SetReadOnly(false);
        //mpExtraEditName[extra]->
        mpExtraEditName[extra]->SetVisible();

        SetItem(index + 1, COLUMN_LABEL, p_label);
        SetItem(index + 1, COLUMN_EDIT_NAME, mpExtraEditName[extra]);

        index += 2;
      }
      

    }

    void DigitalInputConfListView::CreateUserDefinedCounterRows(int firstRowIndex)
    {    
      int index = firstRowIndex;

      for (int extra = 0; extra < NO_OF_USD_COUNTERS; extra++)
      {
        mpUSDCounterName[extra] = new DataPointText();
        mpUSDCounterName[extra]->SetReadOnly();
        mpUSDCounterName[extra]->SetLeftMargin(8);
        mpUSDCounterName[extra]->SetVisible(true);

        ModeCheckBox* p_mode_check_box = new ModeCheckBox();
        p_mode_check_box->SetCheckState(DIGITAL_INPUT_FUNC_USERDEFINE_CNT_1 + extra);
        p_mode_check_box->SetReadOnly(false);
        p_mode_check_box->SetVisible(true);

        
        SetItem(index, COLUMN_LABEL, mpUSDCounterName[extra]);
        SetItem(index, COLUMN_CHECK_BOX, p_mode_check_box);
        index++;
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


