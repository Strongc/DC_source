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
/* CLASS NAME       : CombiAlarmSlipPoint                                   */
/*                                                                          */
/* FILE NAME        : CombiAlarmSlipPoint.cpp                               */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the combi alarm DataPoints into one                      */
/* virtual DataPoint for the display to look at...                          */
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
#include "CombiAlarmSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_COMBI_ALARM_SELECT_SOURCE_ID 57

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
CombiAlarmSlipPoint::CombiAlarmSlipPoint()
{
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
CombiAlarmSlipPoint::~CombiAlarmSlipPoint()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::InitSubTask(void)
{
  if (mpCurrentCombiAlarmNumber.IsValid())
  {
    mpCurrentCombiAlarmNumber->SetAsInt(1);
  }

  UpdateVirtualCombiAlarmSource();
  UpdateUpperStatusLine();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if ( mpCurrentCombiAlarmNumber.IsUpdated() )
  {
    UpdateVirtualCombiAlarmSource();
    UpdateUpperStatusLine();
  }

  if ( mpVirtualCombiAlarmSource.IsUpdated() )
  {
    UpdateCurrentCombiAlarmSource();
  }


  {
    const int index = mpCurrentCombiAlarmNumber->GetAsInt() - 1;

    if ((index >= 0) && (index < NO_OF_COMBI_ALARM_SOURCES))
    {
      if (mpCombiAlarmDP[index].IsUpdated())
      {
        UpdateVirtualCombiAlarmSource();
      }
    }
    else
    {
      FatalErrorOccured("CASP1 index out of range!");
    }
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
  mCurrentlyUpdating = true;  // stop reacting on updates

  for (int i = 0; i < NO_OF_COMBI_ALARM_SOURCES; ++i )
  {
    if (mpCombiAlarmDP[i].Detach(pSubject))
    {
      return;
    }
  }

  mpCurrentCombiAlarmNumber.Detach(pSubject);
  mpVirtualCombiAlarmSource.Detach(pSubject);
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    if (mpVirtualCombiAlarmSource.Update(pSubject))
    {
      // nop
    }
    else if (mpCurrentCombiAlarmNumber.Update(pSubject))
    {
      // nop
    }
    else
    {
      for (int i = 0; i < NO_OF_COMBI_ALARM_SOURCES; ++i )
      {
        if (mpCombiAlarmDP[i].Update(pSubject))
        {
          break;
        }
      }
    }
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
    case SP_CASP_COMBI_11_ALARM_SOURCE:
      mpCombiAlarmDP[0].Attach(pSubject);
      break;
    case SP_CASP_COMBI_12_ALARM_SOURCE:
      mpCombiAlarmDP[1].Attach(pSubject);
      break;
    case SP_CASP_COMBI_21_ALARM_SOURCE:
      mpCombiAlarmDP[2].Attach(pSubject);
      break;
    case SP_CASP_COMBI_22_ALARM_SOURCE:
      mpCombiAlarmDP[3].Attach(pSubject);
      break;
    case SP_CASP_COMBI_31_ALARM_SOURCE:
      mpCombiAlarmDP[4].Attach(pSubject);
      break;
    case SP_CASP_COMBI_32_ALARM_SOURCE:
      mpCombiAlarmDP[5].Attach(pSubject);
      break;
    case SP_CASP_COMBI_41_ALARM_SOURCE:
      mpCombiAlarmDP[6].Attach(pSubject);
      break;
    case SP_CASP_COMBI_42_ALARM_SOURCE:
      mpCombiAlarmDP[7].Attach(pSubject);
      break;

    case SP_CASP_CURRENT_NO:
      mpCurrentCombiAlarmNumber.Attach(pSubject);
      break;
    case SP_CASP_VIRTUAL_ALARM_SOURCE:
      mpVirtualCombiAlarmSource.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::ConnectToSubjects(void)
{
  for ( int i = 0; i < NO_OF_COMBI_ALARM_SOURCES; ++i )
  {
    mpCombiAlarmDP[i]->Subscribe(this);
  }

  mpCurrentCombiAlarmNumber->Subscribe(this);
  mpVirtualCombiAlarmSource->Subscribe(this);
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void CombiAlarmSlipPoint::UpdateVirtualCombiAlarmSource()
{
  const int index = (mpCurrentCombiAlarmNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < NO_OF_COMBI_ALARM_SOURCES))
  {
    mpVirtualCombiAlarmSource->SetValue(mpCombiAlarmDP[index]->GetValue());
  }
  else
  {
    FatalErrorOccured("CASP2 index out of range!");
  }
}

void CombiAlarmSlipPoint::UpdateCurrentCombiAlarmSource()
{
  const int index = (mpCurrentCombiAlarmNumber->GetAsInt() - 1);

  if ((index >= 0) && (index < NO_OF_COMBI_ALARM_SOURCES))
  {
    mpCombiAlarmDP[index]->SetValue(mpVirtualCombiAlarmSource->GetValue());
  }
  else
  {
    FatalErrorOccured("CASP3 index out of range!");
  }
}

void CombiAlarmSlipPoint::UpdateUpperStatusLine()
{
    Display* p_display = NULL;
    char display_number[10];

    p_display = GetDisplay( DISPLAY_COMBI_ALARM_SELECT_SOURCE_ID );

    int index = mpCurrentCombiAlarmNumber->GetAsInt();

    if (((index+1)%2)==0)
    {
      p_display->GetRoot()->SetHelpString(SID_HELP_4_5_4_1);
    }
    else
    {
      p_display->GetRoot()->SetHelpString(SID_HELP_4_5_4_2);
    }


    index = index + (index-1)/2;

    sprintf(display_number, "4.5.4.%i", index);
    p_display->SetDisplayNumber( display_number );

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
