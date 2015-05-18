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
/* CLASS NAME       : DataPointText                                         */
/*                                                                          */
/* FILE NAME        : DataPointText.cpp                                     */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a StringDataPoint.             */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Languages.h>
#include <StringDataPoint.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "DataPointText.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    DataPointText::DataPointText(Component* pParent /*=NULL*/) : ObserverText(pParent)
    {
      mShowIfTextContinues = true;
      mIsSubscribedToChangeOfLanguage = false;
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    DataPointText::~DataPointText()
    {
    }

    void DataPointText::ConnectToSubjects()
    {
      ObserverText::ConnectToSubjects();

      if (!mIsSubscribedToChangeOfLanguage)
      {
        mIsSubscribedToChangeOfLanguage = true;
        // in case the font is language depended subscribe to language changes
        Languages::GetInstance()->Subscribe(this);
      }
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Redraws this element. If it fails (for some reason) the
    * method returns false.
    *****************************************************************************/
    bool DataPointText::Redraw()
    {
      bool return_value = false;

      if (IsVisible())
      {
        const char* the_string_to_show;
        StringDataPoint* dp  = (StringDataPoint*)GetSubject();
        if(dp)
        {
          the_string_to_show = dp->GetValue();
          SetText(the_string_to_show);
          ObserverText::Redraw();
          return_value = true;
        }
        else
        {
          Validate();
        }
      }
      else
      {
        return_value = false;
      }
      return return_value;
    }
  } // namespace display
} // namespace mpc




