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
/* CLASS NAME       : SpecificEnergyListView                                */
/*                                                                          */
/* FILE NAME        : SpecificEnergyListView.CPP                            */
/*                                                                          */
/* CREATED DATE     : 2007-10-02                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* see header file                                                          */
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
#include "SpecificEnergyListView.h"
#include "Text.h"
#include "NumberQuantity.h"
#include <MPCFonts.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MIN_SPECIFIC_ENERGY_VALUE 0
#define MAX_SPECIFIC_ENERGY_VALUE 9999999 // [J/m3]

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
namespace mpc
{
  namespace display
  {
    
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
    SpecificEnergyListView::SpecificEnergyListView(Component* pParent /*= NULL*/) : ListView(pParent)
    {
      char label_text[] = "XX Hz";
      Text*           p_text;
      NumberQuantity* p_nq;

      InsertColumn(0);
      SetColumnWidth(0, 140);
      InsertColumn(1);
      SetColumnWidth(1, 78);

      for (int i = 0; i < MAX_NO_OF_GRAPH_SAMPLES ; i++)
      {
        InsertItem(i, (mpc::display::Component*)NULL);

        mpVirtualYValues[i] = new FloatDataPoint();
        mpVirtualYValues[i]->SetQuality(DP_NEVER_AVAILABLE);
      }

      for (int i = 0; i < MAX_NO_OF_GRAPH_SAMPLES ; i++)
      {
        sprintf(label_text, "%i Hz", 60 - 2*i);

        p_text = new Text();
        p_text->SetText(label_text);
        p_text->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
        p_text->SetAlign(GUI_TA_LEFT + GUI_TA_VCENTER);
        p_text->SetLeftMargin(2);
        p_text->SetRightMargin(0);
        p_text->SetWordWrap(false);
        p_text->SetVisible(true);
        p_text->SetReadOnly(true);
        p_text->Invalidate();
        SetItem(i,0,p_text);

        p_nq = new NumberQuantity();
        p_nq->SetFont(DEFAULT_FONT_13_LANGUAGE_INDEP);
        p_nq->SetNumberOfDigits(4);
        p_nq->SetQuantityType(Q_SPECIFIC_ENERGY);
        p_nq->SetVisible(true);
        p_nq->SetReadOnly(true);
        SetItem(i,1,p_nq);
      }
    }


    /*****************************************************************************
    * Function - Destructor
    * DESCRIPTION:
    *
    ****************************************************************************/
    SpecificEnergyListView::~SpecificEnergyListView()
    {
    }


    /* --------------------------------------------------
    * Update is part of the observer pattern
    * --------------------------------------------------*/
    void SpecificEnergyListView::Update(Subject* Object)
    {
      UpdateList();
    }
    /* --------------------------------------------------
    * Called if subscription shall be canceled
    * --------------------------------------------------*/
    void SpecificEnergyListView::SubscribtionCancelled(Subject* pSubject)
    {
      
    }
    /* --------------------------------------------------
    * Called to set the subject pointer (used by class
    * factory)
    * --------------------------------------------------*/
    void SpecificEnergyListView::SetSubjectPointer(int Id, Subject* pSubject)
    {
      mYValues.Attach(pSubject);
    }
    /* --------------------------------------------------
    * Called to indicate that subscription kan be made
    * --------------------------------------------------*/
    void SpecificEnergyListView::ConnectToSubjects(void)
    {
      mYValues.Subscribe(this);

      InitializeList();
    }


    /*****************************************************************************
    *
    *
    *              PRIVATE FUNCTIONS
    *
    *
    ****************************************************************************/

    /*****************************************************************************
    * Function - InitializeList
    * DESCRIPTION:
    *
    ****************************************************************************/
    void SpecificEnergyListView::InitializeList()
    {     
      for (int i = 0; i < MAX_NO_OF_GRAPH_SAMPLES ; i++)
      {
        mpVirtualYValues[i]->SetQuantity(Q_SPECIFIC_ENERGY);
        mpVirtualYValues[i]->SetMinValue(MIN_SPECIFIC_ENERGY_VALUE);
        mpVirtualYValues[i]->SetMaxValue(MAX_SPECIFIC_ENERGY_VALUE);

        NumberQuantity* p_nq = (NumberQuantity*) GetItem(i,1);
        p_nq->SetSubjectPointer(0, mpVirtualYValues[i]);
        p_nq->ConnectToSubjects();
      }
    }

    /*****************************************************************************
    * Function - UpdateList
    * DESCRIPTION:
    *
    ****************************************************************************/
    void SpecificEnergyListView::UpdateList()
    {
      int no_of_samples = mYValues->GetSize();

      for (int i = 0; i < MAX_NO_OF_GRAPH_SAMPLES; i++)
      {
        float value = -1;

        if (i < no_of_samples)
        {
          value = mYValues->GetValue(i);
        }

        int nq_index = MAX_NO_OF_GRAPH_SAMPLES - i - 1;

        if (value > 0)
        {
          mpVirtualYValues[nq_index]->SetValue(value);
        }
        else if (value == 0)
        {
          mpVirtualYValues[nq_index]->SetQuality(DP_NOT_AVAILABLE);
        }
        else
        {
          mpVirtualYValues[nq_index]->SetQuality(DP_NEVER_AVAILABLE);
        }
        
      }
    }

    /*****************************************************************************
    *
    *
    *              PROTECTED FUNCTIONS
    *                 - RARE USED -
    *
    ****************************************************************************/

  } // namespace display
} // namespace mpc


