/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : CombiAlarmSourceListView                              */
/*                                                                          */
/* FILE NAME        : CombiAlarmSourceListView.CPP                          */
/*                                                                          */
/* CREATED DATE     : 2007-10-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class implements the combi alarm ListView                           */
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
#include "CombiAlarmSourceListView.h"
#include "ModeCheckBox.h"
#include "Label.h"
#include <MPCFonts.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define mpc1ST_COLUMN_WIDTH 192
#define mpc2ND_COLUMN_WIDTH 34

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
namespace mpc
{
  namespace display
  {
    const MODE_CHECK_BOX_LABEL_VALUE LIST_VIEW_DATA[] =
    {
      { SID_NOT_USED,                ALARM_NOT_USED              },
      { SID_HIGH_LEVEL,              ALARM_HIGH_LEVEL            },
      { SID_SYS_ALARM_LEVEL,         ALARM_ALARM_LEVEL           },
      { SID_OVERFLOW,                ALARM_OVERFLOW              },
      { SID_ALL_PUMPS_IN_ALARM,      ALARM_ALL_PUMPS_IN_ALARM    },
      { SID_PUMP_1_IN_ALARM,         ALARM_PUMP_1_IN_ALARM       },
      { SID_PUMP_2_IN_ALARM,         ALARM_PUMP_2_IN_ALARM       },
      { SID_GENI_ERROR_PUMP_1,       ALARM_GENI_ERROR_PUMP_1     },
      { SID_GENI_ERROR_PUMP_2,       ALARM_GENI_ERROR_PUMP_2     }
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
    CombiAlarmSourceListView::CombiAlarmSourceListView(Component* pParent /*= NULL*/) : ListView(pParent)
    {
      Label*        label;
      ModeCheckBox* mode_check_box;

      InsertColumn(0);
      SetColumnWidth(0,mpc1ST_COLUMN_WIDTH);
      InsertColumn(1);
      SetColumnWidth(1,mpc2ND_COLUMN_WIDTH);

      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        InsertItem(i, (mpc::display::Component*)NULL);
      }

      for (int i = 0; i < LIST_VIEW_DATA_CNT; i++)
      {
        label = new Label;
        label->SetStringId(LIST_VIEW_DATA[i].StringId);
        label->SetFont(DEFAULT_FONT_13_LANGUAGE_DEP);
        label->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
        label->SetLeftMargin(8);
        label->SetRightMargin(0);
        label->SetWordWrap(false);
        label->SetVisible(true);
        label->SetReadOnly(true);
        label->Invalidate();
        SetItem(i,0,label);

        mode_check_box = new ModeCheckBox;
        mode_check_box->SetCheckState(LIST_VIEW_DATA[i].CheckValue);
        mode_check_box->SetVisible(true);
        mode_check_box->SetReadOnly(false);
        mode_check_box->Invalidate();
        SetItem(i,1,mode_check_box);
      }
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    CombiAlarmSourceListView::~CombiAlarmSourceListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void CombiAlarmSourceListView::Update(Subject* Object)
    {

    }
    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void CombiAlarmSourceListView::SubscribtionCancelled(Subject* pSubject)
    {
      for ( int i = 0; i < LIST_VIEW_DATA_CNT; i++ )
      {
        ((ModeCheckBox*)(GetItem(i,1)))->SubscribtionCancelled(pSubject);
      }
    }
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void CombiAlarmSourceListView::SetSubjectPointer(int Id,Subject* pSubject)
    {
      for ( int i = 0; i < LIST_VIEW_DATA_CNT; i++ )
      {
        ((ModeCheckBox*)(GetItem(i,1)))->SetSubjectPointer(Id,pSubject);
      }
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void CombiAlarmSourceListView::ConnectToSubjects(void)
    {
      for ( int i = 0; i < LIST_VIEW_DATA_CNT; i++ )
      {
        ((ModeCheckBox*)(GetItem(i,1)))->ConnectToSubjects();
      }
    }
    /* --------------------------------------------------
    * Sets the font of this text element
    * --------------------------------------------------*/
    void CombiAlarmSourceListView::SetFont(const GUI_FONT** Font)
    {
      for ( int i = 0; i < LIST_VIEW_DATA_CNT; i++ )
      {
        ((Label*)(GetItem(i,0)))->SetFont(Font);
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


