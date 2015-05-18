/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME      : AnalogOutputConfigSlipPoint                            */
/*                                                                          */
/* FILE NAME       : AnalogOutputConfigSlipPoint.CPP                        */
/*                                                                          */
/* CREATED DATE    :                                                        */
/*                                                                          */
/* SHORT FILE DESCRIPTION:                                                  */
/* This class maps the analog output configuration DataPoints into one      */
/* virtual DataPoint for the display to look at...                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <DisplayController.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "AnalogOutputConfigSlipPoint.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define DISPLAY_AO_CONFIG_ID 144

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/
namespace mpc
{
  namespace display
  {
    namespace ctrl
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
  AnalogOutputConfigSlipPoint::AnalogOutputConfigSlipPoint()
  {
    mCurrentlyUpdating = false;
  }

  /*****************************************************************************
  * Function - Destructor
  * DESCRIPTION:
  *
  ****************************************************************************/
  AnalogOutputConfigSlipPoint::~AnalogOutputConfigSlipPoint()
  {
  }

  /*****************************************************************************
  * Function - InitSubTask
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::InitSubTask(void)
  {
    if (mpCurrentAnalogOutputNumber.IsValid())
    {
      mpCurrentAnalogOutputNumber->SetAsInt(1);
    }

    UpdateVirtualAnalogOutputConfig();
    UpdateUpperStatusLine();
  }

  /*****************************************************************************
  * Function - RunSubTask
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::RunSubTask(void)
  {
    mCurrentlyUpdating = true;  // Guard the SubTask

    const int ao_index = (mpCurrentAnalogOutputNumber->GetAsInt() - 1);
    bool update_current_config = false;
    bool update_virtual_config = false;

    if (mpCurrentAnalogOutputNumber.IsUpdated())
    {
      UpdateVirtualAnalogOutputConfig();
      UpdateUpperStatusLine();
    }

    if (mpVirtualAnalogOutputFunc.IsUpdated())
    {
      update_current_config = true;
    }

    if (mpVirtualAnalogOutputMin.IsUpdated())
    {
      update_current_config = true;
    }

    if (mpVirtualAnalogOutputMax.IsUpdated())
    {
      update_current_config = true;
    }

    if (update_current_config)
    {
      UpdateCurrentAnalogOutputConfig();  
    }

    if ((ao_index >= 0) && (ao_index < MAX_NO_OF_ANA_OUTPUTS))
    {
      if (mpAnalogOutputFunc[ao_index].IsUpdated())
      {
        update_virtual_config = true;
      }

      if (mpAnalogOutputMin[ao_index].IsUpdated())
      {
        update_virtual_config = true;
      }

      if (mpAnalogOutputMax[ao_index].IsUpdated())
      {
        update_virtual_config = true;
      }

      if (update_virtual_config)
      {
        UpdateVirtualAnalogOutputConfig();
      }
    }
    else
    {
      FatalErrorOccured("AOCSP index out of range!");
    }
    

    mCurrentlyUpdating = false; // End of: Guard the SubTask
  }

  /*****************************************************************************
  * Function - SubscribtionCancelled
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::SubscribtionCancelled(Subject* pSubject)
  {
    mCurrentlyUpdating = true;  // stop reaction on updates

    mpCurrentAnalogOutputNumber.Detach(pSubject);

    mpVirtualAnalogOutputFunc.Detach(pSubject);
    mpVirtualAnalogOutputMin.Detach(pSubject);
    mpVirtualAnalogOutputMax.Detach(pSubject);

    for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
    {
      mpAnalogOutputFunc[i].Detach(pSubject);
      mpAnalogOutputMin[i].Detach(pSubject);
      mpAnalogOutputMax[i].Detach(pSubject);
    }
  }

  /*****************************************************************************
  * Function - Update
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::Update(Subject* pSubject)
  {
    if (!mCurrentlyUpdating)
    {
      if (mpCurrentAnalogOutputNumber.Update(pSubject)){}
      else if (mpVirtualAnalogOutputFunc.Update(pSubject)){}
      else if (mpVirtualAnalogOutputMin.Update(pSubject)){}
      else if (mpVirtualAnalogOutputMax.Update(pSubject)){}
      else
      {
        for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
        {
          if (mpAnalogOutputFunc[i].Update(pSubject)) break;
          else if (mpAnalogOutputMin[i].Update(pSubject)) break;
          else if (mpAnalogOutputMax[i].Update(pSubject)) break;
        }
      }

      ReqTaskTime();
    }
  }

  /*****************************************************************************
  * Function - SetSubjectPointer
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
  {
    switch (Id)
    {
    case SP_AOCSP_AO_1_FUNC:
      mpAnalogOutputFunc[0].Attach(pSubject);
      break;
    case SP_AOCSP_AO_2_FUNC:
      mpAnalogOutputFunc[1].Attach(pSubject);
      break;
    case SP_AOCSP_AO_3_FUNC:
      mpAnalogOutputFunc[2].Attach(pSubject);
      break;
    case SP_AOCSP_AO_4_FUNC:
      mpAnalogOutputFunc[3].Attach(pSubject);
      break;
    case SP_AOCSP_AO_5_FUNC:
      mpAnalogOutputFunc[4].Attach(pSubject);
      break;
    case SP_AOCSP_AO_6_FUNC:
      mpAnalogOutputFunc[5].Attach(pSubject);
      break;
    case SP_AOCSP_AO_7_FUNC:
      mpAnalogOutputFunc[6].Attach(pSubject);
      break;
    case SP_AOCSP_AO_8_FUNC:
      mpAnalogOutputFunc[7].Attach(pSubject);
      break;
    case SP_AOCSP_AO_9_FUNC:
      mpAnalogOutputFunc[8].Attach(pSubject);
      break;

    case SP_AOCSP_AO_1_MIN:
      mpAnalogOutputMin[0].Attach(pSubject);
      break;
    case SP_AOCSP_AO_2_MIN:
      mpAnalogOutputMin[1].Attach(pSubject);
      break;
    case SP_AOCSP_AO_3_MIN:
      mpAnalogOutputMin[2].Attach(pSubject);
      break;
    case SP_AOCSP_AO_4_MIN:
      mpAnalogOutputMin[3].Attach(pSubject);
      break;
    case SP_AOCSP_AO_5_MIN:
      mpAnalogOutputMin[4].Attach(pSubject);
      break;
    case SP_AOCSP_AO_6_MIN:
      mpAnalogOutputMin[5].Attach(pSubject);
      break;
    case SP_AOCSP_AO_7_MIN:
      mpAnalogOutputMin[6].Attach(pSubject);
      break;
    case SP_AOCSP_AO_8_MIN:
      mpAnalogOutputMin[7].Attach(pSubject);
      break;
    case SP_AOCSP_AO_9_MIN:
      mpAnalogOutputMin[8].Attach(pSubject);
      break;

    case SP_AOCSP_AO_1_MAX:
      mpAnalogOutputMax[0].Attach(pSubject);
      break;
    case SP_AOCSP_AO_2_MAX:
      mpAnalogOutputMax[1].Attach(pSubject);
      break;
    case SP_AOCSP_AO_3_MAX:
      mpAnalogOutputMax[2].Attach(pSubject);
      break;
    case SP_AOCSP_AO_4_MAX:
      mpAnalogOutputMax[3].Attach(pSubject);
      break;
    case SP_AOCSP_AO_5_MAX:
      mpAnalogOutputMax[4].Attach(pSubject);
      break;
    case SP_AOCSP_AO_6_MAX:
      mpAnalogOutputMax[5].Attach(pSubject);
      break;
    case SP_AOCSP_AO_7_MAX:
      mpAnalogOutputMax[6].Attach(pSubject);
      break;
    case SP_AOCSP_AO_8_MAX:
      mpAnalogOutputMax[7].Attach(pSubject);
      break;
    case SP_AOCSP_AO_9_MAX:
      mpAnalogOutputMax[8].Attach(pSubject);
      break;

    case SP_AOCSP_CURRENT_NO:
      mpCurrentAnalogOutputNumber.Attach(pSubject);
      break;

    case SP_AOCSP_VIRTUAL_FUNC:
      mpVirtualAnalogOutputFunc.Attach(pSubject);
      break;
    case SP_AOCSP_VIRTUAL_MIN:
      mpVirtualAnalogOutputMin.Attach(pSubject);
      break;
    case SP_AOCSP_VIRTUAL_MAX:
      mpVirtualAnalogOutputMax.Attach(pSubject);
      break;
    }
  }

  /*****************************************************************************
  * Function - ConnectToSubjects
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::ConnectToSubjects(void)
  {
    mpCurrentAnalogOutputNumber->Subscribe(this);

    mpVirtualAnalogOutputFunc->Subscribe(this);
    mpVirtualAnalogOutputMin->Subscribe(this);
    mpVirtualAnalogOutputMax->Subscribe(this);

    for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
    {
      mpAnalogOutputFunc[i]->Subscribe(this);
      mpAnalogOutputMin[i]->Subscribe(this);
      mpAnalogOutputMax[i]->Subscribe(this);
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
  * Function - UpdateVirtualAnalogOutputConfig
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::UpdateVirtualAnalogOutputConfig()
  {
    int i = (mpCurrentAnalogOutputNumber->GetAsInt() - 1);

    if ((i >= 0) && (i < MAX_NO_OF_ANA_OUTPUTS))
    {
      mpVirtualAnalogOutputFunc->SetValue(mpAnalogOutputFunc[i]->GetValue());
      mpVirtualAnalogOutputMin->CopyValues(mpAnalogOutputMin[i].GetSubject());
      mpVirtualAnalogOutputMax->CopyValues(mpAnalogOutputMax[i].GetSubject());
    }
  }

  /*****************************************************************************
  * Function - UpdateCurrentAnalogOutputConfig
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::UpdateCurrentAnalogOutputConfig()
  {
    int i = (mpCurrentAnalogOutputNumber->GetAsInt() - 1);

    if ((i >= 0) && (i < MAX_NO_OF_ANA_OUTPUTS))
    {
      mpAnalogOutputFunc[i]->SetValue(mpVirtualAnalogOutputFunc->GetValue());
      mpAnalogOutputMin[i]->SetValue(mpVirtualAnalogOutputMin->GetValue());
      mpAnalogOutputMax[i]->SetValue(mpVirtualAnalogOutputMax->GetValue());
    }
  }

  /*****************************************************************************
  * Function - UpdateUpperStatusLine
  * DESCRIPTION:
  *
  ****************************************************************************/
  void AnalogOutputConfigSlipPoint::UpdateUpperStatusLine()
  {
    Display* p_display = NULL;
    char display_number[10];

    int index = mpCurrentAnalogOutputNumber->GetAsInt();

    sprintf(display_number, "4.4.3.%i", index);

    p_display = GetDisplay(DISPLAY_AO_CONFIG_ID);
    p_display->SetDisplayNumber(display_number);

    DisplayController::GetInstance()->RequestTitleUpdate();
  }
/*****************************************************************************
*
*
*              PROTECTED FUNCTIONS
*                 - RARE USED -
*
****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc
