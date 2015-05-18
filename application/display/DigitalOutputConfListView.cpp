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
/* CLASS NAME       : DigitalOutputConfListView                             */
/*                                                                          */
/* FILE NAME        : DigitalOutputConfListView.CPP                         */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class implements the digital output configuration ListView          */
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
#include "DigitalOutputConfListView.h"
#include <MPCFonts.h>
#include "ModeCheckBox.h"
#include "Label.h"
#include "DataPointText.h"
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
    #define mpc1ST_COLUMN_WIDTH 175
    #define mpc2ND_COLUMN_WIDTH 64
    #define mpcNO_WIDTH 0

    #define FIRST_USER_IO_INDEX 19

    const MODE_CHECK_BOX_LABEL_VALUE LIST_VIEW_DATA[] =
    {
      { SID_DO_RELAY_NO_FUNC,              RELAY_FUNC_NO_FUNCTION               },
      { SID_DO_START_DOSING_PUMP,               RELAY_FUNC_PUMP_1                    },
      { SID_DO_PUMP_2_START,               RELAY_FUNC_PUMP_2                    },
      { SID_PUMP_3_START,                  RELAY_FUNC_PUMP_3                    },
      { SID_PUMP_4_START,                  RELAY_FUNC_PUMP_4                    },
      { SID_PUMP_5_START,                  RELAY_FUNC_PUMP_5                    },
      { SID_PUMP_6_START,                  RELAY_FUNC_PUMP_6                    },
      { SID_DO_MIXER_START,                RELAY_FUNC_MIXER                     },
      { SID_DO_CUSTOM_RELAY,               RELAY_FUNC_RELAY_CUSTOM              },
      { SID_DO_HIGH_LEVEL_ALARM,           RELAY_FUNC_ALARM_RELAY_HIGH_LEVEL    },
      { SID_DO_URGENT_ALARMS,              RELAY_FUNC_ALARM_RELAY_CRITICAL      },
      { SID_DO_ALL_ALARMS,                 RELAY_FUNC_ALARM_RELAY_ALL_ALARMS    },
      { SID_DO_ALL_ALARMS_AND_WARNINGS,    RELAY_FUNC_ALARM_RELAY_ALL_ALARMS_AND_WARNINGS },
      { SID_DO_CUSTOM_ALARM_RELAY,         RELAY_FUNC_ALARM_RELAY_CUSTOM        },
      { SID_DO_VFD_1_REVERSE,              RELAY_FUNC_VFD_1_REVERSE             },
      { SID_DO_VFD_2_REVERSE,              RELAY_FUNC_VFD_2_REVERSE             },
      { SID_DO_VFD_3_REVERSE,              RELAY_FUNC_VFD_3_REVERSE             },
      { SID_DO_VFD_4_REVERSE,              RELAY_FUNC_VFD_4_REVERSE             },
      { SID_DO_VFD_5_REVERSE,              RELAY_FUNC_VFD_5_REVERSE             },
      { SID_DO_VFD_6_REVERSE,              RELAY_FUNC_VFD_6_REVERSE             },
      { SID_USERDEFINED_FUNCTION_1,        RELAY_FUNC_USER_IO_1                 },
      { SID_USERDEFINED_FUNCTION_2,        RELAY_FUNC_USER_IO_2                 },
      { SID_USERDEFINED_FUNCTION_3,        RELAY_FUNC_USER_IO_3                 },
      { SID_USERDEFINED_FUNCTION_4,        RELAY_FUNC_USER_IO_4                 },
      { SID_USERDEFINED_FUNCTION_5,        RELAY_FUNC_USER_IO_5                 },
      { SID_USERDEFINED_FUNCTION_6,        RELAY_FUNC_USER_IO_6                 },
      { SID_USERDEFINED_FUNCTION_7,        RELAY_FUNC_USER_IO_7                 },
      { SID_USERDEFINED_FUNCTION_8,        RELAY_FUNC_USER_IO_8                 }
      //{ SID_DO_START_DOSING_PUMP,          RELAY_FUNC_DOSING_PUMP               }
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
    DigitalOutputConfListView::DigitalOutputConfListView(Component* pParent /*= NULL*/) : ListView(pParent)
    {
      Text*          p_label;
      ModeCheckBox*  p_mode_check_box;
      AvalibleIfSet* p_available_if_set;

      InsertColumn(COLUMN_LABEL);
      InsertColumn(COLUMN_CHECK_BOX);
      InsertColumn(COLUMN_AVAILABLE);
      InsertColumn(COLUMN_HIGH_END_AVAILABLE);

      SetColumnWidth(COLUMN_LABEL, mpc1ST_COLUMN_WIDTH);
      SetColumnWidth(COLUMN_CHECK_BOX, mpc2ND_COLUMN_WIDTH);
      SetColumnWidth(COLUMN_AVAILABLE, mpcNO_WIDTH);
      SetColumnWidth(COLUMN_HIGH_END_AVAILABLE, mpcNO_WIDTH);

      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        InsertItem(i, (mpc::display::Component*)NULL);

        int check_value = LIST_VIEW_DATA[i].CheckValue;

        if (i < LIST_VIEW_DATA_CNT - NO_OF_USER_IO)
        {
          p_label = new Label();
          ((Label*)p_label)->SetStringId(LIST_VIEW_DATA[i].StringId);
        }
        else
        {
          p_label = new DataPointText();
        }

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

        if (check_value == RELAY_FUNC_PUMP_2 || check_value == RELAY_FUNC_VFD_2_REVERSE)
        {
          p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          SetItem(i, COLUMN_AVAILABLE, p_available_if_set);
        }
        else if (check_value == RELAY_FUNC_PUMP_3 || check_value == RELAY_FUNC_VFD_3_REVERSE)
        {
          p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->Invert();
          p_available_if_set->AddCheckState(1);
          p_available_if_set->AddCheckState(2);
          SetItem(i, COLUMN_AVAILABLE, p_available_if_set);
        }
        else if (check_value == RELAY_FUNC_PUMP_4 || check_value == RELAY_FUNC_VFD_4_REVERSE)
        {
          p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(4);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(i, COLUMN_AVAILABLE, p_available_if_set);
        }
        else if (check_value == RELAY_FUNC_PUMP_5 || check_value == RELAY_FUNC_VFD_5_REVERSE)
        {
          p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(5);
          p_available_if_set->AddCheckState(6);
          SetItem(i, COLUMN_AVAILABLE, p_available_if_set);
        }
        else if (check_value == RELAY_FUNC_PUMP_6 || check_value == RELAY_FUNC_VFD_6_REVERSE)
        {
          p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(6);
          SetItem(i, COLUMN_AVAILABLE, p_available_if_set);
        }

        if (i>=FIRST_USER_IO_INDEX + USER_IO_1
          && i <= FIRST_USER_IO_INDEX + USER_IO_8)
        {
          p_available_if_set = new AvalibleIfSet(this);
          p_available_if_set->AddCheckState(1);
          SetItem(i, COLUMN_HIGH_END_AVAILABLE, p_available_if_set);
        }

      }
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    DigitalOutputConfListView::~DigitalOutputConfListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void DigitalOutputConfListView::Update(Subject* Object)
    {

    }
    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void DigitalOutputConfListView::SubscribtionCancelled(Subject* pSubject)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->SubscribtionCancelled(pSubject);

        if (i >= (FIRST_USER_IO_INDEX + USER_IO_1) && i <= (FIRST_USER_IO_INDEX + USER_IO_8))
        {
          ((DataPointText*)(GetItem(i, COLUMN_LABEL)))->SubscribtionCancelled(pSubject);
        }

        if (GetItem(i, COLUMN_AVAILABLE))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE)))->SubscribtionCancelled(pSubject);
        }

        if (GetItem(i, COLUMN_HIGH_END_AVAILABLE))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_HIGH_END_AVAILABLE)))->SubscribtionCancelled(pSubject);
        }
      }
    }
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void DigitalOutputConfListView::SetSubjectPointer(int Id,Subject* pSubject)
    {
      switch(Id)
      {
        case SP_DOCLV_FUNCTION:
          for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
          {
            ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->SetSubjectPointer(Id,pSubject);
          }
          break;
        case SP_DOCLV_NO_OF_PUMPS:
          for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
          {
            if (GetItem(i, COLUMN_AVAILABLE))
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE)))->SetSubjectPointer(Id,pSubject);
            }
          }
          break;
        case SP_DOCLV_USER_IO_1:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_1, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_2:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_2, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_3:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_3, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_4:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_4, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_5:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_5, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_6:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_6, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_7:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_7, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_USER_IO_8:
          ((DataPointText*)(GetItem(FIRST_USER_IO_INDEX + USER_IO_8, COLUMN_LABEL)))->SetSubjectPointer(Id,pSubject);
          break;
        case SP_DOCLV_IS_HIGH_END:
          for (int i = FIRST_USER_IO_INDEX + USER_IO_1; i <=  FIRST_USER_IO_INDEX + USER_IO_8; i++)
          {
            if (GetItem(i, COLUMN_HIGH_END_AVAILABLE))
            {
              ((AvalibleIfSet*)(GetItem(i, COLUMN_HIGH_END_AVAILABLE)))->SetSubjectPointer(Id, pSubject);
            }
          }
          break;
      }
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void DigitalOutputConfListView::ConnectToSubjects(void)
    {
      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        ((ModeCheckBox*)(GetItem(i, COLUMN_CHECK_BOX)))->ConnectToSubjects();

        if (i >= (FIRST_USER_IO_INDEX + USER_IO_1)
          && i <= (FIRST_USER_IO_INDEX + USER_IO_8))
        {
          ((DataPointText*)(GetItem(i, COLUMN_LABEL)))->ConnectToSubjects();
        }

        if (GetItem(i, COLUMN_AVAILABLE))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_AVAILABLE)))->ConnectToSubjects();
        }

        if (GetItem(i, COLUMN_HIGH_END_AVAILABLE))
        {
          ((AvalibleIfSet*)(GetItem(i, COLUMN_HIGH_END_AVAILABLE)))->ConnectToSubjects();
        }
      }
    }
    /* --------------------------------------------------
    * Sets the font of this text element
    * --------------------------------------------------*/
    void DigitalOutputConfListView::SetFont(const GUI_FONT** Font)
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


