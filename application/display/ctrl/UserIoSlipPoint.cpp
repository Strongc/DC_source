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
/* CLASS NAME       : UserIoSlipPoint                                       */
/*                                                                          */
/* FILE NAME        : UserIoSlipPoint.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 16-12-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the UserIoConfigs into common                            */
/* virtual DataPoints for the display to look at...                         */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Factory.h>
#include <DataPoint.h>
#include <UpperStatusLine.h>
#include <DisplayController.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "UserIoSlipPoint.h"
#include <IoChannelConfig.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_USER_IO_CONFIG_ID 121
#define DISPLAY_USER_IO_LOGIC_ID 123
#define DISPLAY_FIRST_USER_FUNC_SOURCE_ID 131
#define DISPLAY_SECOND_IO_CHANNEL_ID 132
#define DISPLAY_IO_CHANNEL_CONFIG_ID 124
#define DISPLAY_IO_CHANNEL_AI_SELECT_ID 133
#define DISPLAY_IO_CHANNEL_DI_SELECT_ID 134

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
 *****************************************************************************/
UserIoSlipPoint::UserIoSlipPoint()
{
  mCurrentlyUpdating = false;
  mpUserIoState = new UserIoState();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
UserIoSlipPoint::~UserIoSlipPoint()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::InitSubTask(void)
{
  mpCurrentUserIoNumber->SetValue(USER_IO_1);
  mpCurrentUserIoNumber.SetUpdated();

  mCurrentlyUpdating = true;
  
  UpdateVirtualUserIo();
  UpdateUpperStatusLine();
  
  mCurrentlyUpdating = false;

  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask
  bool update_user_io_channel_sources = false;

  if (mpInputNumber.IsUpdated())
  {
    if (mpInputNumber->GetValue() == 1)
    {
      mpCurrentChannelNumber->SetValue((USER_FUNC_SOURCE_TYPE)mpVirtualChannelIndex1->GetValue());
    }
    else if (mpInputNumber->GetValue() == 2)
    {
      mpCurrentChannelNumber->SetValue((USER_FUNC_SOURCE_TYPE)mpVirtualChannelIndex2->GetValue());
    }

    UpdateUpperStatusLine();
  }

  if (mpCurrentUserIoNumber.IsUpdated())
  {
    UpdateUpperStatusLine();

    mpUserIoDP[mpCurrentUserIoNumber->GetAsInt() - 1].SetUpdated();

    update_user_io_channel_sources = true;
  }

  if (mpVirtualChannelIndex1.IsUpdated(false)
    || mpVirtualChannelIndex2.IsUpdated(false))
  {
    update_user_io_channel_sources = true;
  }

  if (mpVirtualMaxHoldTime.IsUpdated(false) 
    && mpVirtualMaxHoldTimeEnabled->GetValue() == false)
  {
    mpVirtualMaxHoldTimeEnabled->SetValue(true);
  }

  if (mpVirtualMaxHoldTimeEnabled.IsUpdated(false))
  {
    DP_QUALITY_TYPE q = (mpVirtualMaxHoldTimeEnabled->GetValue() ? DP_AVAILABLE : DP_NOT_AVAILABLE);
    mpVirtualMaxHoldTime->SetQuality(q);
  }
  
  if ( mpVirtualEnabled.IsUpdated()
    || mpVirtualName.IsUpdated()
    || mpVirtualChannelIndex1.IsUpdated()
    || mpVirtualChannelIndex2.IsUpdated()
    || mpVirtualLogic.IsUpdated()
    || mpVirtualInvert.IsUpdated()
    || mpVirtualMinHoldTime.IsUpdated()
    || mpVirtualMaxHoldTime.IsUpdated()
    || mpVirtualMaxHoldTimeEnabled.IsUpdated())
  {
    UpdateCurrentUserIo();
  }

  
  const int index = mpCurrentUserIoNumber->GetAsInt();

  if ((index >= FIRST_USER_IO) && (index <= LAST_USER_IO))
  {
    if (mpUserIoDP[index - 1].IsUpdated() || mpNames[index].IsUpdated())
    {
      UpdateVirtualUserIo();
      update_user_io_channel_sources = true;
    }
  }
  else
  {
    FatalErrorOccured("UIOSP: OOR error"); // index out of range
  }

  for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
  {
    if (mpIoChConfigs[i].IsUpdated())
    {
      update_user_io_channel_sources = true;
    }
  }

  if (update_user_io_channel_sources)
  {
    UpdateUserIoChannelSources();    
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reacting on updates

  for (int i = 0; i < NO_OF_USER_IO; i++)
  {
    if (mpUserIoDP[i].Detach(pSubject))
    {
      return;
    }
  }
  mpInputNumber.Detach(pSubject);
  mpCurrentChannelNumber.Detach(pSubject);
  mpCurrentUserIoNumber.Detach(pSubject);
  mpVirtualEnabled.Detach(pSubject);
  mpVirtualName.Detach(pSubject);
  mpVirtualChannelIndex1.Detach(pSubject);
  mpVirtualChannelIndex2.Detach(pSubject);
  mpVirtualLogic.Detach(pSubject);
  mpVirtualInvert.Detach(pSubject);
  mpVirtualMinHoldTime.Detach(pSubject);
  mpVirtualMaxHoldTime.Detach(pSubject);
  mpVirtualMaxHoldTimeEnabled.Detach(pSubject);
  mpVirtualDestination.Detach(pSubject);

}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::Update(Subject* pSubject)
{
  if (!mCurrentlyUpdating)
  {
    if (mpInputNumber.Update(pSubject)){}
    else if (mpCurrentUserIoNumber.Update(pSubject)){}
    else if (mpVirtualEnabled.Update(pSubject)){}
    else if (mpVirtualName.Update(pSubject)){}
    else if (mpVirtualChannelIndex1.Update(pSubject)){}
    else if (mpVirtualChannelIndex2.Update(pSubject)){}
    else if (mpVirtualLogic.Update(pSubject)){}
    else if (mpVirtualInvert.Update(pSubject)){}
    else if (mpVirtualMinHoldTime.Update(pSubject)){}
    else if (mpVirtualMaxHoldTime.Update(pSubject)){}
    else if (mpVirtualMaxHoldTimeEnabled.Update(pSubject)){}
    else
    {
      for (int i = 0; i < NO_OF_USER_IO; i++)
      {
        if (mpUserIoDP[i].Update(pSubject))
        {
          break;
        }
      }

      for (int i = FIRST_USER_IO; i < LAST_USER_IO; i++)
      {
        if (mpNames[i].Update(pSubject))
        {
          break;
        }
      }

      for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
      {
        if (mpIoChConfigs[i].Update(pSubject))
        {
          break;
        }
      }
    }
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
  case SP_UIOSP_USER_IO1:
    mpUserIoDP[0].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO2:
    mpUserIoDP[1].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO3:
    mpUserIoDP[2].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO4:
    mpUserIoDP[3].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO5:
    mpUserIoDP[4].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO6:
    mpUserIoDP[5].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO7:
    mpUserIoDP[6].Attach(pSubject);
    break;
  case SP_UIOSP_USER_IO8:
    mpUserIoDP[7].Attach(pSubject);
    break;
  case SP_UIOSP_INPUT_NUMBER:
    mpInputNumber.Attach(pSubject);
    break;
  case SP_UIOSP_CURRENT_CHANNEL_NUMBER:
    mpCurrentChannelNumber.Attach(pSubject);
    break;
  case SP_UIOSP_CURRENT_NUMBER:
    mpCurrentUserIoNumber.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_ENABLED:
    mpVirtualEnabled.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_NAME:
    mpVirtualName.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_CH1:
    mpVirtualChannelIndex1.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_CH2:
    mpVirtualChannelIndex2.Attach(pSubject);
    break;
  case SP_UIOSP_CH1_CONFIG:
    mpCh1Config.Attach(pSubject);
    break;
  case SP_UIOSP_CH2_CONFIG:
    mpCh2Config.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_LOGIC:
    mpVirtualLogic.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_INVERT:
    mpVirtualInvert.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_MIN_HOLD_TIME:
    mpVirtualMinHoldTime.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_MAX_HOLD_TIME:
    mpVirtualMaxHoldTime.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_MAX_HOLD_TIME_ENABLED:
    mpVirtualMaxHoldTimeEnabled.Attach(pSubject);
    break;
  case SP_UIOSP_VIRTUAL_DESTINATION:
    mpVirtualDestination.Attach(pSubject);
    break;
  case SP_UIOSP_UDF_1_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_1_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_1_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_1_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_2_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_2_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_2_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_2_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_3_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_3_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_3_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_3_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_4_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_4_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_4_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_4_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_5_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_5_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_5_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_5_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_6_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_6_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_6_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_6_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_7_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_7_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_7_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_7_2].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_8_CH_1_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_8_1].Attach(pSubject);
    break;
  case SP_UIOSP_UDF_8_CH_2_CONFIG:
    mpIoChConfigs[USER_FUNC_SOURCE_8_2].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_1:
    mpNames[USER_IO_1].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_2:
    mpNames[USER_IO_2].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_3:
    mpNames[USER_IO_3].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_4:
    mpNames[USER_IO_4].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_5:
    mpNames[USER_IO_5].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_6:
    mpNames[USER_IO_6].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_7:
    mpNames[USER_IO_7].Attach(pSubject);
    break;
  case SP_UIOSP_NAME_8:
    mpNames[USER_IO_8].Attach(pSubject);
    break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::ConnectToSubjects(void)
{
  for (int i = 0; i < NO_OF_USER_IO; i++)
  {
    if (mpUserIoDP[i].IsValid())
    {
      mpUserIoDP[i]->Subscribe(this);
    }
  }

  for (int i = FIRST_USER_IO; i < LAST_USER_IO; i++)
  {
    if (mpNames[i].IsValid())
    {
      mpNames[i]->Subscribe(this);
    }
  }

  // need subscribeE on inputNumber, because it is reused for all user IOs
  // that is inputNumber 1 on User IO X may differ from inputNumber 1 on User IO Y
  mpInputNumber->SubscribeE(this);
  mpCurrentUserIoNumber->Subscribe(this);
  mpVirtualEnabled->Subscribe(this);
  mpVirtualName->Subscribe(this);
  mpVirtualChannelIndex1->Subscribe(this);
  mpVirtualChannelIndex2->Subscribe(this);
  mpVirtualLogic->Subscribe(this);
  mpVirtualInvert->Subscribe(this);
  mpVirtualMinHoldTime->Subscribe(this);
  mpVirtualMaxHoldTime->Subscribe(this);
  mpVirtualMaxHoldTimeEnabled->Subscribe(this);

  char prefix[MAX_CHANNEL_PREFIX_LEN];

  for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
  {
    if (mpIoChConfigs[i].IsValid())
    {
      mpIoChConfigs[i]->Subscribe(this);

      sprintf(prefix, "%d: ", i);
      mpIoChConfigs[i]->SetChannelPrefix(prefix);
    }
  }
  
}



/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 * Function - UpdateVirtualUserIo
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::UpdateVirtualUserIo()
{
  U8 index = mpCurrentUserIoNumber->GetValue();

  if ((index >= FIRST_USER_IO) && (index <= LAST_USER_IO))
  {
    UserIoConfig* p_cfg = mpUserIoDP[index - 1].GetSubject();

    mpVirtualEnabled->SetValue(p_cfg->GetEnabled());
    mpVirtualName->SetValue(mpNames[index]->GetValue());
    mpVirtualChannelIndex1->SetValue((U8)p_cfg->GetFirstSourceIndex());
    mpVirtualChannelIndex2->SetValue((U8)p_cfg->GetSecondSourceIndex());
    mpVirtualLogic->SetValue(p_cfg->GetLogic());
    mpVirtualInvert->SetValue(p_cfg->GetInverted());
    mpVirtualMinHoldTime->SetValue(p_cfg->GetMinHoldTime());
    mpVirtualMaxHoldTime->SetValue(p_cfg->GetMaxHoldTime());
    mpVirtualMaxHoldTimeEnabled->SetValue(p_cfg->GetMaxHoldTimeEnabled());
    mpVirtualDestination->SetValue(p_cfg->GetDestination());

    DP_QUALITY_TYPE q = (mpVirtualMaxHoldTimeEnabled->GetValue() ? DP_AVAILABLE : DP_NOT_AVAILABLE);
    mpVirtualMaxHoldTime->SetQuality(q);

    Display* p_display;
    p_display = GetDisplay(DISPLAY_USER_IO_CONFIG_ID);
    p_display->GetRoot()->Invalidate();   
    
  }
  else
  {
    FatalErrorOccured("UIOSP: OOR error"); // index out of range
  }
}


/*****************************************************************************
 * Function - UpdateCurrentUserIo
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::UpdateCurrentUserIo()
{
  U8 index = mpCurrentUserIoNumber->GetValue();

  if ((index >= FIRST_USER_IO) && (index <= LAST_USER_IO))
  {
    UserIoConfig* p_cfg = mpUserIoDP[index - 1].GetSubject();

    p_cfg->SetEnabled(mpVirtualEnabled->GetValue());
    mpNames[index]->SetValue(mpVirtualName->GetValue());
    p_cfg->SetLogic(mpVirtualLogic->GetValue());
    p_cfg->SetInverted(mpVirtualInvert->GetValue());
    p_cfg->SetMinHoldTime(mpVirtualMinHoldTime->GetValue());
    p_cfg->SetMaxHoldTime(mpVirtualMaxHoldTime->GetValue());
    p_cfg->SetMaxHoldTimeEnabled(mpVirtualMaxHoldTimeEnabled->GetValue());
    p_cfg->SetDestination(mpVirtualDestination->GetValue());
  }
  else
  {
    FatalErrorOccured("UIOSP: OOR error"); // index out of range
  }
}


/*****************************************************************************
 * Function - UpdateUserIoChannelSources
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::UpdateUserIoChannelSources()
{
  IoChannelConfig* p_channel_config;

  char prefix[MAX_CHANNEL_PREFIX_LEN];
  int index_1 = mpVirtualChannelIndex1->GetAsInt();
  int index_2 = mpVirtualChannelIndex2->GetAsInt();
  
  p_channel_config = mpIoChConfigs[index_1].GetSubject();
  mpCh1Config->CopyValues(p_channel_config);
  sprintf(prefix, "%d: ", index_1);
  mpCh1Config->SetChannelPrefix(prefix);
  
  p_channel_config = mpIoChConfigs[index_2].GetSubject();
  mpCh2Config->CopyValues(p_channel_config);
  sprintf(prefix, "%d: ", index_2);
  mpCh2Config->SetChannelPrefix(prefix);
}

/*****************************************************************************
 * Function - UpdateUpperStatusLine
 * DESCRIPTION:
 ****************************************************************************/
void UserIoSlipPoint::UpdateUpperStatusLine()
{
  Display* p_display = NULL;
  char display_number[20];
  char input_no = '0' + mpInputNumber->GetValue();
  int index = mpCurrentUserIoNumber->GetAsInt();
  int title_string_id = mpUserIoState->GetStateStringId(index);

  mpUserIoState->GetStateDisplayNumber(index, display_number);

  p_display = GetDisplay(DISPLAY_USER_IO_CONFIG_ID);
  p_display->SetDisplayNumber(display_number);
  p_display->SetName(title_string_id);


  strcat(display_number, ".3");
  p_display = GetDisplay(DISPLAY_USER_IO_LOGIC_ID);
  p_display->SetDisplayNumber(display_number);
  
  // replace last charactor with input number ('1' or '2')
  display_number[strlen(display_number)-1] = input_no;
  p_display = GetDisplay(DISPLAY_IO_CHANNEL_CONFIG_ID);
  p_display->SetDisplayNumber(display_number);
  
  strcat(display_number, ".1");
  p_display = GetDisplay(DISPLAY_IO_CHANNEL_AI_SELECT_ID);
  p_display->SetDisplayNumber(display_number);
  p_display = GetDisplay(DISPLAY_IO_CHANNEL_DI_SELECT_ID);
  p_display->SetDisplayNumber(display_number);

  DisplayController::GetInstance()->RequestTitleUpdate();

}
/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc
