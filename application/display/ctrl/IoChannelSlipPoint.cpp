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
/* CLASS NAME       : IoChannelSlipPoint                                    */
/*                                                                          */
/* FILE NAME        : IoChannelSlipPoint.cpp                                */
/*                                                                          */
/* CREATED DATE     : 16-12-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the IoChannelConfigs into common                         */
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
#include "IoChannelSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_IO_CHANNEL_CONFIG_ID 124

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
IoChannelSlipPoint::IoChannelSlipPoint()
{
  mCurrentlyUpdating = false;
  mpIoChannelHeadlineState = new IoChannelHeadlineState();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 ****************************************************************************/
IoChannelSlipPoint::~IoChannelSlipPoint()
{
  delete mpIoChannelHeadlineState;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::InitSubTask(void)
{
  mpCurrentChannelNumber->SetValue(FIRST_USER_FUNC_SOURCE);

  mCurrentlyUpdating = true;
  
  UpdateVirtualChannel();
  UpdateUpperStatusLine();
  InitializeIndexOfSource(false);
  
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if (mpCurrentChannelNumber.IsUpdated())
  {
    UpdateVirtualChannel();
    UpdateUpperStatusLine();
  }

  if (mpVirtualSourceType.IsUpdated())
  {
    InitializeSourceFromType();
  }

  if (mpVirtualSource.IsUpdated(false))
  {
    InitializeIndexOfSource(true);

    USER_FUNC_SOURCE_TYPE index = mpCurrentChannelNumber->GetValue();
    mpChannelDP[(int)index].SetUpdated();      
  }

  if (mpVirtualConstantValue.IsUpdated(false))
  {
    mpVirtualSourceIndex->SetValue(mpVirtualConstantValue->GetValue() ? 1 : 0);
  }

  if ( mpVirtualSource.IsUpdated()
    || mpVirtualSourceIndex.IsUpdated()
    || mpVirtualAiLimit.IsUpdated()
    || mpVirtualInvert.IsUpdated()
    || mpVirtualResponseTime.IsUpdated()
    || mpVirtualTimerHigh.IsUpdated()
    || mpVirtualTimerLow.IsUpdated()
    || mpVirtualConstantValue.IsUpdated())
  {
    UpdateCurrentChannel();
  }
  
  USER_FUNC_SOURCE_TYPE index = mpCurrentChannelNumber->GetValue();

  if ((index >= FIRST_USER_FUNC_SOURCE) && (index <= LAST_USER_FUNC_SOURCE))
  {
    if (mpChannelDP[(int)index].IsUpdated())
    {
      UpdateVirtualChannel();
    }
  }
  else
  {
    FatalErrorOccured("IOCSP: OOR error"); // index out of range
  }
  

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reacting on updates

  for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
  {
    if (mpChannelDP[i].Detach(pSubject))
    {
      return;
    }
  }

  mpCurrentChannelNumber.Detach(pSubject);
  mpVirtualSourceType.Detach(pSubject);
  mpVirtualSource.Detach(pSubject);
  mpVirtualSourceIndex.Detach(pSubject);
  mpVirtualAiLimit.Detach(pSubject);
  mpVirtualInvert.Detach(pSubject);
  mpVirtualResponseTime.Detach(pSubject);
  mpVirtualTimerHigh.Detach(pSubject);
  mpVirtualTimerLow.Detach(pSubject);
  mpVirtualConstantValue.Detach(pSubject);
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::Update(Subject* pSubject)
{
  if (!mCurrentlyUpdating)
  {
    if (mpCurrentChannelNumber.Update(pSubject)){}
    else if (mpVirtualSourceType.Update(pSubject)){}
    else if (mpVirtualSource.Update(pSubject)){}
    else if (mpVirtualSourceIndex.Update(pSubject)){}
    else if (mpVirtualAiLimit.Update(pSubject)){}
    else if (mpVirtualInvert.Update(pSubject)){}
    else if (mpVirtualResponseTime.Update(pSubject)){}
    else if (mpVirtualTimerHigh.Update(pSubject)){}
    else if (mpVirtualTimerLow.Update(pSubject)){}
    else if (mpVirtualConstantValue.Update(pSubject)){}
    else
    {
      for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
      {
        if (mpChannelDP[i].Update(pSubject))
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
void IoChannelSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
  case SP_IOCSP_UDF_1_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_1_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_1_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_1_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_2_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_2_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_2_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_2_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_3_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_3_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_3_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_3_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_4_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_4_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_4_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_4_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_5_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_5_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_5_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_5_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_6_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_6_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_6_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_6_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_7_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_7_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_7_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_7_2].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_8_CH_1:
    mpChannelDP[USER_FUNC_SOURCE_8_1].Attach(pSubject);
    break;
  case SP_IOCSP_UDF_8_CH_2:
    mpChannelDP[USER_FUNC_SOURCE_8_2].Attach(pSubject);
    break;
  case SP_IOCSP_CURRENT_NUMBER:
    mpCurrentChannelNumber.Attach(pSubject);      
    break;
  case SP_IOCSP_VIRTUAL_SOURCE_TYPE:
    mpVirtualSourceType.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_SOURCE:
    mpVirtualSource.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_SOURCE_INDEX:
    mpVirtualSourceIndex.Attach(pSubject);  
    break;
  case SP_IOCSP_VIRTUAL_AI_LIMIT:
    mpVirtualAiLimit.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_INVERT:
    mpVirtualInvert.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_RESPONSE_TIME:
    mpVirtualResponseTime.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_TIMER_HIGH:
    mpVirtualTimerHigh.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_TIMER_LOW:
    mpVirtualTimerLow.Attach(pSubject);
    break;
  case SP_IOCSP_VIRTUAL_CONSTANT_VALUE:
    mpVirtualConstantValue.Attach(pSubject);
    break;
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::ConnectToSubjects(void)
{
  for (int i = FIRST_USER_FUNC_SOURCE; i <= LAST_USER_FUNC_SOURCE; i++)
  {
    if (mpChannelDP[i].IsValid())
    {
      mpChannelDP[i]->Subscribe(this);
    }
  }

  mpCurrentChannelNumber->Subscribe(this);
  mpVirtualSourceType->Subscribe(this);
  mpVirtualSource->Subscribe(this);
  mpVirtualSourceIndex->Subscribe(this);
  mpVirtualAiLimit->Subscribe(this);
  mpVirtualInvert->Subscribe(this);
  mpVirtualResponseTime->Subscribe(this);
  mpVirtualTimerHigh->Subscribe(this);
  mpVirtualTimerLow->Subscribe(this);
  mpVirtualConstantValue->Subscribe(this);
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 * Function - UpdateVirtualChannel
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::UpdateVirtualChannel()
{
  USER_FUNC_SOURCE_TYPE index = mpCurrentChannelNumber->GetValue();

  if ((index >= FIRST_USER_FUNC_SOURCE) && (index <= LAST_USER_FUNC_SOURCE))
  {
    IoChannelConfig* p_cfg = mpChannelDP[(int)index].GetSubject();

    mpVirtualSourceType->SetValue(p_cfg->GetSourceType());
    mpVirtualSource->SetValue(p_cfg->GetSource());
    // update min/max values of index, when source has been set
    InitializeIndexOfSource(false);
    mpVirtualSourceIndex->SetValue(p_cfg->GetSourceIndex());
    mpVirtualAiLimit->CopyValues(p_cfg->GetAiLimit());
    mpVirtualInvert->SetValue(p_cfg->GetInverted());
    mpVirtualResponseTime->SetValue(p_cfg->GetResponseTime());
    mpVirtualTimerHigh->SetValue(p_cfg->GetTimerHighPeriod());
    mpVirtualTimerLow->SetValue(p_cfg->GetTimerLowPeriod());
    mpVirtualConstantValue->SetValue(p_cfg->GetConstantValue());

    Display* p_display;
    p_display = GetDisplay(DISPLAY_IO_CHANNEL_CONFIG_ID);
    p_display->GetRoot()->Invalidate();
  }
  else
  {
    FatalErrorOccured("IOCSP: OOR error"); // index out of range
  }
}

/*****************************************************************************
 * Function - UpdateCurrentChannel
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::UpdateCurrentChannel()
{
  USER_FUNC_SOURCE_TYPE index = mpCurrentChannelNumber->GetValue();

  if ((index >= FIRST_USER_FUNC_SOURCE) && (index <= LAST_USER_FUNC_SOURCE))
  {
    IoChannelConfig* p_cfg = mpChannelDP[(int)index].GetSubject();

    p_cfg->SetSourceType(mpVirtualSourceType->GetValue());
    p_cfg->SetSource(mpVirtualSource->GetValue());
    p_cfg->SetSourceIndex(mpVirtualSourceIndex->GetValue());
    p_cfg->SetAiLimitRange(mpVirtualAiLimit.GetSubject());
    p_cfg->SetAiLimitValue(mpVirtualAiLimit->GetValue());
    p_cfg->SetInverted(mpVirtualInvert->GetValue());
    p_cfg->SetResponseTime(mpVirtualResponseTime->GetValue());
    p_cfg->SetTimerHighPeriod(mpVirtualTimerHigh->GetValue());
    p_cfg->SetTimerLowPeriod(mpVirtualTimerLow->GetValue());
    p_cfg->SetConstantValue(mpVirtualConstantValue->GetValue());
  }
  else
  {
    FatalErrorOccured("IOCSP: OOR error"); // index out of range
  }
}

/*****************************************************************************
 * Function - InitializeSourceFromType
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::InitializeSourceFromType()
{
  switch (mpVirtualSourceType->GetValue())
  {
    case CHANNEL_SOURCE_DATATYPE_AI:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_AI_INDEX_CU_361);
      break;
    case CHANNEL_SOURCE_DATATYPE_DI:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_DI_INDEX_CU_361);
      break;  
    case CHANNEL_SOURCE_DATATYPE_ALARM:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_COMBI_ALARM);
      break;
    case CHANNEL_SOURCE_DATATYPE_TIMER:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_TIMER_FUNC);
      break;
    case CHANNEL_SOURCE_DATATYPE_CONSTANT:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_CONSTANT_VALUE);
      break;
    case CHANNEL_SOURCE_DATATYPE_USERIO:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_USER_IO);
      break;
    case CHANNEL_SOURCE_DATATYPE_SYSTEM_STATES:
      mpVirtualSource->SetValue(CHANNEL_SOURCE_SYSTEM_STATES);
      break;

  }
  mpVirtualSource.SetUpdated();
}

/*****************************************************************************
 * Function - InitializeIndexOfSource
 * DESCRIPTION: select a default source-index based on source type
 ****************************************************************************/
void IoChannelSlipPoint::InitializeIndexOfSource(bool resetValue)
{
  switch (mpVirtualSource->GetValue())
  {
    case CHANNEL_SOURCE_AI_INDEX_CU_361:
      mpVirtualSourceIndex->SetMinValue(1);
      mpVirtualSourceIndex->SetMaxValue(3);
      break;
    case CHANNEL_SOURCE_AI_INDEX_IO_351_1:
    case CHANNEL_SOURCE_AI_INDEX_IO_351_2:
    case CHANNEL_SOURCE_AI_INDEX_IO_351_3:
      mpVirtualSourceIndex->SetMinValue(1);
      mpVirtualSourceIndex->SetMaxValue(2);
      break;
    case CHANNEL_SOURCE_DI_INDEX_CU_361:
      mpVirtualSourceIndex->SetMinValue(1);
      mpVirtualSourceIndex->SetMaxValue(3);
      break;
    case CHANNEL_SOURCE_DI_INDEX_IO_351_1:
    case CHANNEL_SOURCE_DI_INDEX_IO_351_2:
    case CHANNEL_SOURCE_DI_INDEX_IO_351_3:
      mpVirtualSourceIndex->SetMinValue(1);
      mpVirtualSourceIndex->SetMaxValue(9);
      break;
    case CHANNEL_SOURCE_COMBI_ALARM:
      mpVirtualSourceIndex->SetMinValue(1);
      mpVirtualSourceIndex->SetMaxValue(4);
      break;
    case CHANNEL_SOURCE_USER_IO:
      mpVirtualSourceIndex->SetMinValue(FIRST_USER_IO);
      mpVirtualSourceIndex->SetMaxValue(LAST_USER_IO);
      break;
    case CHANNEL_SOURCE_CONSTANT_VALUE:
      mpVirtualSourceIndex->SetMinValue(0);
      mpVirtualSourceIndex->SetMaxValue(1);
      break;
    case CHANNEL_SOURCE_TIMER_FUNC:
      mpVirtualSourceIndex->SetMinValue(0);
      mpVirtualSourceIndex->SetMaxValue(0);
      break;
    default:
      mpVirtualSourceIndex->SetMinValue(U8_MIN_VALUE);
      mpVirtualSourceIndex->SetMaxValue(U8_MAX_VALUE);
      break;
  }

  if (resetValue)
  {
    switch (mpVirtualSource->GetValue())
    {
      case CHANNEL_SOURCE_AI_INDEX_CU_361:
      case CHANNEL_SOURCE_AI_INDEX_IO_351_1:
      case CHANNEL_SOURCE_AI_INDEX_IO_351_2:
      case CHANNEL_SOURCE_AI_INDEX_IO_351_3:
      case CHANNEL_SOURCE_DI_INDEX_CU_361:
      case CHANNEL_SOURCE_DI_INDEX_IO_351_1:
      case CHANNEL_SOURCE_DI_INDEX_IO_351_2:
      case CHANNEL_SOURCE_DI_INDEX_IO_351_3:
      case CHANNEL_SOURCE_COMBI_ALARM:
        mpVirtualSourceIndex->SetValue(1);
        break;
      case CHANNEL_SOURCE_USER_IO:
        mpVirtualSourceIndex->SetValue(FIRST_USER_IO);
        break;
      case CHANNEL_SOURCE_CONSTANT_VALUE:
        mpVirtualSourceIndex->SetValue(mpVirtualConstantValue->GetValue() ? 1 : 0);
        break;
      default:
        mpVirtualSourceIndex->SetValue(0);
        break;
    }
  }
}

/*****************************************************************************
 * Function - UpdateUpperStatusLine
 * DESCRIPTION:
 ****************************************************************************/
void IoChannelSlipPoint::UpdateUpperStatusLine()
{
  Display* p_display = NULL;

  int index = (int) mpCurrentChannelNumber->GetValue();
  int title_string_id = mpIoChannelHeadlineState->GetStateStringId(index);

  p_display = GetDisplay(DISPLAY_IO_CHANNEL_CONFIG_ID); 
  p_display->SetName(title_string_id);

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
