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
/* CLASS NAME       : SmsScheduleSlipPoint                                  */
/*                                                                          */
/* FILE NAME        : SmsScheduleSlipPoint.cpp                              */
/*                                                                          */
/* CREATED DATE     : 2007-11-27                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the sms schedule Vector DataPoints into                  */
/* virtual (non-vector) DataPoints for the display to look at...            */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Factory.h>
#include <DataPoint.h>
#include <DisplayController.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "SmsScheduleSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_EDIT_SMS_SCHEDULE_ID 73
#define DISPLAY_SELECT_PHONE_NO_ID   74
#define DISPLAY_VIEW_SMS_SCHEDULE_ID 76

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_PHONE_NO_SOURCE = 0,
  PHONE_NO_SOURCE_1ST_PRIMARY = FIRST_PHONE_NO_SOURCE,
  PHONE_NO_SOURCE_1ST_SECONDARY,
  PHONE_NO_SOURCE_2ND_PRIMARY,
  PHONE_NO_SOURCE_2ND_SECONDARY,
  PHONE_NO_SOURCE_3TH_PRIMARY,
  PHONE_NO_SOURCE_3TH_SECONDARY,
  NO_OF_PHONE_NO_SOURCE,
  LAST_PHONE_NO_SOURCE = NO_OF_PHONE_NO_SOURCE - 1
} PHONE_NO_SOURCE_TYPE;

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
SmsScheduleSlipPoint::SmsScheduleSlipPoint()
{
  mCurrentlyUpdating = false;
  mpWeekdayHeadlineState = new WeekdayHeadlineState();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
SmsScheduleSlipPoint::~SmsScheduleSlipPoint()
{
  delete mpWeekdayHeadlineState;
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::InitSubTask(void)
{
  if (mDpCurrentWeekday.IsValid())
    mDpCurrentWeekday->SetAsInt(1);

  UpdateVirtualSmsSchedule();
  UpdateUpperStatusLineOfEditDisplay();
  UpdateUpperStatusLineOfPhoneNoSelectionDisplay();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask
  bool update_flag;

  if (mDpCurrentScheduleViewed.IsUpdated() )
  {
    UpdateUpperStatusLineOfViewDisplay();
  }

  update_flag  = mDpWorkEnabled.IsUpdated();
  update_flag |= mDpOffEnabled.IsUpdated();
  update_flag |= mDpSleepEnabled.IsUpdated();
  update_flag |= mDpWorkStarttime.IsUpdated();
  update_flag |= mDpOffStarttime.IsUpdated();
  update_flag |= mDpSleepStarttime.IsUpdated();
  update_flag |= mDp1stStarttime.IsUpdated();
  update_flag |= mDp2ndStarttime.IsUpdated();
  update_flag |= mDp3thStarttime.IsUpdated();
  update_flag |= mDp1stPriNoId.IsUpdated();
  update_flag |= mDp1stSecNoId.IsUpdated();
  update_flag |= mDp2ndPriNoId.IsUpdated();
  update_flag |= mDp2ndSecNoId.IsUpdated();
  update_flag |= mDp3thPriNoId.IsUpdated();
  update_flag |= mDp3thSecNoId.IsUpdated();
  if ( mDpCurrentWeekday.IsUpdated() )
  {
    UpdateUpperStatusLineOfEditDisplay();
    UpdateVirtualSmsSchedule();
  }
  else if(update_flag)
  {
    UpdateVirtualSmsSchedule();
  }

  if ( mDpCurrentPhoneNoSource.IsUpdated())
  {
    UpdateVirtualSelectedPhoneNo();
    UpdateUpperStatusLineOfPhoneNoSelectionDisplay();
  }
  if ( mDpVirtualSelectedPhoneNo.IsUpdated() )
  {
    UpdateCurrentPhoneNoSelection();
  }

  update_flag  = mDpVirtualWorkEnabled.IsUpdated();
  update_flag |= mDpVirtualOffEnabled.IsUpdated();
  update_flag |= mDpVirtualSleepEnabled.IsUpdated();
  update_flag |= mDpVirtualWorkStarttime.IsUpdated();
  update_flag |= mDpVirtualOffStarttime.IsUpdated();
  update_flag |= mDpVirtualSleepStarttime.IsUpdated();
  update_flag |= mDpVirtual1stStarttime.IsUpdated();
  update_flag |= mDpVirtual2ndStarttime.IsUpdated();
  update_flag |= mDpVirtual3thStarttime.IsUpdated();
  update_flag |= mDpVirtual1stPriNo.IsUpdated();
  update_flag |= mDpVirtual1stSecNo.IsUpdated();
  update_flag |= mDpVirtual2ndPriNo.IsUpdated();
  update_flag |= mDpVirtual2ndSecNo.IsUpdated();
  update_flag |= mDpVirtual3thPriNo.IsUpdated();
  update_flag |= mDpVirtual3thSecNo.IsUpdated();
  if (update_flag)
  {
    UpdateCurrentSmsSchedule();
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reacting on updates

  if (mDpWorkEnabled.Detach(pSubject))
  {
    return;
  }

  mDpCurrentScheduleViewed.Detach(pSubject);
  mDpCurrentWeekday.Detach(pSubject);
  mDpWorkEnabled.Detach(pSubject);
  mDpOffEnabled.Detach(pSubject);
  mDpSleepEnabled.Detach(pSubject);
  mDpWorkStarttime.Detach(pSubject);
  mDpOffStarttime.Detach(pSubject);
  mDpSleepStarttime.Detach(pSubject);
  mDp1stStarttime.Detach(pSubject);
  mDp2ndStarttime.Detach(pSubject);
  mDp3thStarttime.Detach(pSubject);
  mDp1stPriNoId.Detach(pSubject);
  mDp1stSecNoId.Detach(pSubject);
  mDp2ndPriNoId.Detach(pSubject);
  mDp2ndSecNoId.Detach(pSubject);
  mDp3thPriNoId.Detach(pSubject);
  mDp3thSecNoId.Detach(pSubject);
  mDpVirtualWorkEnabled.Detach(pSubject);
  mDpVirtualOffEnabled.Detach(pSubject);
  mDpVirtualSleepEnabled.Detach(pSubject);
  mDpVirtualWorkStarttime.Detach(pSubject);
  mDpVirtualOffStarttime.Detach(pSubject);
  mDpVirtualSleepStarttime.Detach(pSubject);
  mDpVirtual1stStarttime.Detach(pSubject);
  mDpVirtual2ndStarttime.Detach(pSubject);
  mDpVirtual3thStarttime.Detach(pSubject);
  mDpVirtual1stPriNo.Detach(pSubject);
  mDpVirtual1stSecNo.Detach(pSubject);
  mDpVirtual2ndPriNo.Detach(pSubject);
  mDpVirtual2ndSecNo.Detach(pSubject);
  mDpVirtual3thPriNo.Detach(pSubject);
  mDpVirtual3thSecNo.Detach(pSubject);
  mDpCurrentPhoneNoSource.Detach(pSubject);
  mDpVirtualSelectedPhoneNo.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    mDpCurrentScheduleViewed.Update(pSubject);

    if (mDpCurrentWeekday.Update(pSubject)){}
    else if (mDpWorkEnabled.Update(pSubject)){}
    else if (mDpOffEnabled.Update(pSubject)){}
    else if (mDpSleepEnabled.Update(pSubject)){}
    else if (mDpWorkStarttime.Update(pSubject)){}
    else if (mDpOffStarttime.Update(pSubject)){}
    else if (mDpSleepStarttime.Update(pSubject)){}
    else if (mDp1stStarttime.Update(pSubject)){}
    else if (mDp2ndStarttime.Update(pSubject)){}
    else if (mDp3thStarttime.Update(pSubject)){}
    else if (mDp1stPriNoId.Update(pSubject)){}
    else if (mDp1stSecNoId.Update(pSubject)){}
    else if (mDp2ndPriNoId.Update(pSubject)){}
    else if (mDp2ndSecNoId.Update(pSubject)){}
    else if (mDp3thPriNoId.Update(pSubject)){}
    else if (mDp3thSecNoId.Update(pSubject)){}
    else if (mDpVirtualWorkEnabled.Update(pSubject)){}
    else if (mDpVirtualOffEnabled.Update(pSubject)){}
    else if (mDpVirtualSleepEnabled.Update(pSubject)){}
    else if (mDpVirtualWorkStarttime.Update(pSubject)){}
    else if (mDpVirtualOffStarttime.Update(pSubject)){}
    else if (mDpVirtualSleepStarttime.Update(pSubject)){}
    else if (mDpVirtual1stStarttime.Update(pSubject)){}
    else if (mDpVirtual2ndStarttime.Update(pSubject)){}
    else if (mDpVirtual3thStarttime.Update(pSubject)){}
    else if (mDpVirtual1stPriNo.Update(pSubject)){}
    else if (mDpVirtual1stSecNo.Update(pSubject)){}
    else if (mDpVirtual2ndPriNo.Update(pSubject)){}
    else if (mDpVirtual2ndSecNo.Update(pSubject)){}
    else if (mDpVirtual3thPriNo.Update(pSubject)){}
    else if (mDpVirtual3thSecNo.Update(pSubject)){}
    else if (mDpCurrentPhoneNoSource.Update(pSubject)){}
    else if (mDpVirtualSelectedPhoneNo.Update(pSubject)){}


    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
  case SP_SSSP_CURRENT_SCHEDULE_VIEWED :
    mDpCurrentScheduleViewed.Attach(pSubject);
    break;
  case SP_SSSP_CURRENT_WEEKDAY :
    mDpCurrentWeekday.Attach(pSubject);
    break;
  case SP_SSSP_WORK_ENABLED :
    mDpWorkEnabled.Attach(pSubject);
    break;
  case SP_SSSP_OFF_ENABLED :
    mDpOffEnabled.Attach(pSubject);
    break;
  case SP_SSSP_SLEEP_ENABLED :
    mDpSleepEnabled.Attach(pSubject);
    break;
  case SP_SSSP_WORK_STARTTIME :
    mDpWorkStarttime.Attach(pSubject);
    break;
  case SP_SSSP_OFF_STARTTIME :
    mDpOffStarttime.Attach(pSubject);
    break;
  case SP_SSSP_SLEEP_STARTTIME :
    mDpSleepStarttime.Attach(pSubject);
    break;
  case SP_SSSP_1ST_STARTTIME :
    mDp1stStarttime.Attach(pSubject);
    break;
  case SP_SSSP_2ND_STARTTIME :
    mDp2ndStarttime.Attach(pSubject);
    break;
  case SP_SSSP_3TH_STARTTIME :
    mDp3thStarttime.Attach(pSubject);
    break;
  case SP_SSSP_1P_NO :
    mDp1stPriNoId.Attach(pSubject);
    break;
  case SP_SSSP_1S_NO :
    mDp1stSecNoId.Attach(pSubject);
    break;
  case SP_SSSP_2P_NO :
    mDp2ndPriNoId.Attach(pSubject);
    break;
  case SP_SSSP_2S_NO :
    mDp2ndSecNoId.Attach(pSubject);
    break;
  case SP_SSSP_3P_NO :
    mDp3thPriNoId.Attach(pSubject);
    break;
  case SP_SSSP_3S_NO :
    mDp3thSecNoId.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_WORK_ENABLED :
    mDpVirtualWorkEnabled.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_OFF_ENABLED :
    mDpVirtualOffEnabled.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_SLEEP_ENABLED :
    mDpVirtualSleepEnabled.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_WORK_STARTTIME :
    mDpVirtualWorkStarttime.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_OFF_STARTTIME :
    mDpVirtualOffStarttime.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_SLEEP_STARTTIME :
    mDpVirtualSleepStarttime.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_1ST_STARTTIME :
    mDpVirtual1stStarttime.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_2ND_STARTTIME :
    mDpVirtual2ndStarttime.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_3TH_STARTTIME :
    mDpVirtual3thStarttime.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_1P_NO :
    mDpVirtual1stPriNo.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_1S_NO :
    mDpVirtual1stSecNo.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_2P_NO :
    mDpVirtual2ndPriNo.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_2S_NO :
    mDpVirtual2ndSecNo.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_3P_NO :
    mDpVirtual3thPriNo.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_3S_NO :
    mDpVirtual3thSecNo.Attach(pSubject);
    break;
  case SP_SSSP_CURRENT_PHONE_NO_SOURCE:
    mDpCurrentPhoneNoSource.Attach(pSubject);
    break;
  case SP_SSSP_VIRTUAL_SELECTED_PHONE_NO:
    mDpVirtualSelectedPhoneNo.Attach(pSubject);
    break;
  case SP_SSSP_PHONE_NO_1:
    mDpPhoneNo1.Attach(pSubject);
    break;
  case SP_SSSP_PHONE_NO_2:
    mDpPhoneNo2.Attach(pSubject);
    break;
  case SP_SSSP_PHONE_NO_3:
    mDpPhoneNo3.Attach(pSubject);
    break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::ConnectToSubjects(void)
{
  mDpCurrentScheduleViewed->Subscribe(this);
  mDpCurrentWeekday->Subscribe(this);
  mDpWorkEnabled->Subscribe(this);
  mDpOffEnabled->Subscribe(this);
  mDpSleepEnabled->Subscribe(this);
  mDpWorkStarttime->Subscribe(this);
  mDpOffStarttime->Subscribe(this);
  mDpSleepStarttime->Subscribe(this);
  mDp1stStarttime->Subscribe(this);
  mDp2ndStarttime->Subscribe(this);
  mDp3thStarttime->Subscribe(this);
  mDp1stPriNoId->Subscribe(this);
  mDp1stSecNoId->Subscribe(this);
  mDp2ndPriNoId->Subscribe(this);
  mDp2ndSecNoId->Subscribe(this);
  mDp3thPriNoId->Subscribe(this);
  mDp3thSecNoId->Subscribe(this);
  mDpVirtualWorkEnabled->Subscribe(this);
  mDpVirtualOffEnabled->Subscribe(this);
  mDpVirtualSleepEnabled->Subscribe(this);
  mDpVirtualWorkStarttime->Subscribe(this);
  mDpVirtualOffStarttime->Subscribe(this);
  mDpVirtualSleepStarttime->Subscribe(this);
  mDpVirtual1stStarttime->Subscribe(this);
  mDpVirtual2ndStarttime->Subscribe(this);
  mDpVirtual3thStarttime->Subscribe(this);
  mDpVirtual1stPriNo->Subscribe(this);
  mDpVirtual1stSecNo->Subscribe(this);
  mDpVirtual2ndPriNo->Subscribe(this);
  mDpVirtual2ndSecNo->Subscribe(this);
  mDpVirtual3thPriNo->Subscribe(this);
  mDpVirtual3thSecNo->Subscribe(this);
  mDpCurrentPhoneNoSource->Subscribe(this);
  mDpVirtualSelectedPhoneNo->Subscribe(this);
}


/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateVirtualSmsSchedule()
{
  const int index = mDpCurrentWeekday->GetAsInt() ;

  if ((index >= 0) && (index < NO_OF_WEEKDAY))
  {

    mDpVirtualWorkEnabled->SetValue( mDpWorkEnabled->GetValue(index) );
    mDpVirtualOffEnabled->SetValue( mDpOffEnabled->GetValue(index) );
    mDpVirtualSleepEnabled->SetValue( mDpSleepEnabled->GetValue(index) );

    mDpVirtualWorkStarttime->SetValue( mDpWorkStarttime->GetValue(index) );
    mDpVirtualOffStarttime->SetValue( mDpOffStarttime->GetValue(index) );
    mDpVirtualSleepStarttime->SetValue( mDpSleepStarttime->GetValue(index) );
    mDpVirtual1stStarttime->SetValue( mDp1stStarttime->GetValue(index) );
    mDpVirtual2ndStarttime->SetValue( mDp2ndStarttime->GetValue(index) );
    mDpVirtual3thStarttime->SetValue( mDp3thStarttime->GetValue(index) );

    mDpVirtual1stPriNo->SetValue( GetPhoneNo( mDp1stPriNoId->GetValue(index) ) );
    mDpVirtual1stSecNo->SetValue( GetPhoneNo( mDp1stSecNoId->GetValue(index) ) );
    mDpVirtual2ndPriNo->SetValue( GetPhoneNo( mDp2ndPriNoId->GetValue(index) ) );
    mDpVirtual2ndSecNo->SetValue( GetPhoneNo( mDp2ndSecNoId->GetValue(index) ) );
    mDpVirtual3thPriNo->SetValue( GetPhoneNo( mDp3thPriNoId->GetValue(index) ) );
    mDpVirtual3thSecNo->SetValue( GetPhoneNo( mDp3thSecNoId->GetValue(index) ) );
  }
  else
  {
    FatalErrorOccured("UpdateVirtualSmsSchedule OOR");
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateCurrentSmsSchedule()
{
  const int index = mDpCurrentWeekday->GetAsInt();

  if ((index >= 0) && (index < NO_OF_WEEKDAY))
  {
    mDpWorkEnabled->SetValue( index, mDpVirtualWorkEnabled->GetValue() );
    mDpOffEnabled->SetValue( index, mDpVirtualOffEnabled->GetValue() );
    mDpSleepEnabled->SetValue( index, mDpVirtualSleepEnabled->GetValue() );
    mDpWorkStarttime->SetValue( index, mDpVirtualWorkStarttime->GetValue() );
    mDpOffStarttime->SetValue( index, mDpVirtualOffStarttime->GetValue() );
    mDpSleepStarttime->SetValue( index, mDpVirtualSleepStarttime->GetValue() );
    mDp1stStarttime->SetValue( index, mDpVirtual1stStarttime->GetValue() );
    mDp2ndStarttime->SetValue( index, mDpVirtual2ndStarttime->GetValue() );
    mDp3thStarttime->SetValue( index, mDpVirtual3thStarttime->GetValue() );
  }
  else
  {
    FatalErrorOccured("UpdateCurrentSmsSchedule OOR");
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateVirtualSelectedPhoneNo()
{
  const int index = mDpCurrentWeekday->GetAsInt();
  U8 phone_no_id;

  if ((index >= 0) && (index < NO_OF_WEEKDAY))
  {
    switch ( mDpCurrentPhoneNoSource->GetValue() )
    {
    case PHONE_NO_SOURCE_1ST_PRIMARY:
      phone_no_id = mDp1stPriNoId->GetValue(index);
      break;
    case PHONE_NO_SOURCE_1ST_SECONDARY:
      phone_no_id = mDp1stSecNoId->GetValue(index);
      break;
    case PHONE_NO_SOURCE_2ND_PRIMARY:
      phone_no_id = mDp2ndPriNoId->GetValue(index);
      break;
    case PHONE_NO_SOURCE_2ND_SECONDARY:
      phone_no_id = mDp2ndSecNoId->GetValue(index);
      break;
    case PHONE_NO_SOURCE_3TH_PRIMARY:
      phone_no_id = mDp3thPriNoId->GetValue(index);
      break;
    case PHONE_NO_SOURCE_3TH_SECONDARY:
      phone_no_id = mDp3thSecNoId->GetValue(index);
      break;
    default:
      FatalErrorOccured("UpdateVirtualSelectedPhoneNo source OOR");
      return;
    }

    mDpVirtualSelectedPhoneNo->SetValue( phone_no_id );
  }
  else
  {
    FatalErrorOccured("UpdateVirtualSelectedPhoneNo day OOR");
  }

}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateCurrentPhoneNoSelection()
{
  const int index = mDpCurrentWeekday->GetAsInt();

  if ((index >= 0) && (index < NO_OF_WEEKDAY))
  {
    U8 phone_no_id = mDpVirtualSelectedPhoneNo->GetValue();

    switch ( mDpCurrentPhoneNoSource->GetValue() )
    {
    case PHONE_NO_SOURCE_1ST_PRIMARY:
      mDp1stPriNoId->SetValue( index, phone_no_id );
      mDpVirtual1stPriNo->SetValue( GetPhoneNo( phone_no_id ) );
      break;
    case PHONE_NO_SOURCE_1ST_SECONDARY:
      mDp1stSecNoId->SetValue( index, phone_no_id );
      mDpVirtual1stSecNo->SetValue( GetPhoneNo( phone_no_id ) );
      break;
    case PHONE_NO_SOURCE_2ND_PRIMARY:
      mDp2ndPriNoId->SetValue( index, phone_no_id );
      mDpVirtual2ndPriNo->SetValue( GetPhoneNo( phone_no_id ) );
      break;
    case PHONE_NO_SOURCE_2ND_SECONDARY:
      mDp2ndSecNoId->SetValue( index, phone_no_id );
      mDpVirtual2ndSecNo->SetValue( GetPhoneNo( phone_no_id ) );
      break;
    case PHONE_NO_SOURCE_3TH_PRIMARY:
      mDp3thPriNoId->SetValue( index, phone_no_id );
      mDpVirtual3thPriNo->SetValue( GetPhoneNo( phone_no_id ) );
      break;
    case PHONE_NO_SOURCE_3TH_SECONDARY:
      mDp3thSecNoId->SetValue( index, phone_no_id );
      mDpVirtual3thSecNo->SetValue( GetPhoneNo( phone_no_id ) );
      break;
    default:
      FatalErrorOccured("UpdateCurrentPhoneNoSelection source OOR");
      return;
    }

  }
  else
  {
    FatalErrorOccured("UpdateCurrentPhoneNoSelection day OOR");
  }
}


/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateUpperStatusLineOfEditDisplay()
{
  Display* p_display = NULL;
  char display_number[10];

  int index = mDpCurrentWeekday->GetAsInt();
  int title_string_id = mpWeekdayHeadlineState->GetStateStringId(index);

  mpWeekdayHeadlineState->GetStateDisplayNumber(index, display_number);

  p_display = GetDisplay( DISPLAY_EDIT_SMS_SCHEDULE_ID );

  p_display->SetDisplayNumber( display_number );
  p_display->SetName( title_string_id );

  DisplayController::GetInstance()->RequestTitleUpdate();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateUpperStatusLineOfPhoneNoSelectionDisplay()
{
  Display* p_display = NULL;
  char display_number[12];

  int index = mDpCurrentWeekday->GetAsInt();
  int title_string_id = mpWeekdayHeadlineState->GetStateStringId(index);

  mpWeekdayHeadlineState->GetStateDisplayNumber(index, display_number);

  sprintf(display_number, "%s.%i", display_number, (mDpCurrentPhoneNoSource->GetValue()+1));

  p_display = GetDisplay( DISPLAY_SELECT_PHONE_NO_ID );

  p_display->SetDisplayNumber( display_number );

  DisplayController::GetInstance()->RequestTitleUpdate();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void SmsScheduleSlipPoint::UpdateUpperStatusLineOfViewDisplay()
{
  Display* p_display = GetDisplay( DISPLAY_VIEW_SMS_SCHEDULE_ID );
  switch( mDpCurrentScheduleViewed->GetValue() )
  {
  case SMS_SCHEDULE_TYPE_WORK_OFF_SLEEP:
    p_display->SetName(SID_WORK_OFF_SLEEP);
    p_display->SetDisplayNumber("4.3.4.8");
    break;
  case SMS_SCHEDULE_TYPE_PRIMARY_NO:
    p_display->SetName(SID_PRIMARY_RECIPIENTS);
    p_display->SetDisplayNumber("4.3.4.9");
    break;
  case SMS_SCHEDULE_TYPE_SECONDARY_NO:
    p_display->SetName(SID_SECONDARY_RECIPIENTS);
    p_display->SetDisplayNumber("4.3.4.10");
    break;
  }
  DisplayController::GetInstance()->RequestTitleUpdate();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
const char* SmsScheduleSlipPoint::GetPhoneNo(U8 phoneNoId)
{
  switch (phoneNoId)
  {
    case PHONE_NO_1:
      return mDpPhoneNo1->GetValue();
    case PHONE_NO_2:
      return mDpPhoneNo2->GetValue();
    case PHONE_NO_3:
      return mDpPhoneNo3->GetValue();
    default:
      {
        //There is a small risk that this might happen, so return something valid. 
        // E.g. if a MPC is transformed into a DC and thus have an invalid DC configuration.
       return mDpPhoneNo1->GetValue();
      }
  }
  return "!";
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
