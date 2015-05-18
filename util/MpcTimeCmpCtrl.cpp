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
/* CLASS NAME       : MpcTimeCmpCtrl                                        */
/*                                                                          */
/* FILE NAME        : MpcTimeCmpCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : This class, is able to give you an update at a  */
/*                          Specifyed time, eg. at 20.00                    */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <MpcTimeCmpCtrl.h>
/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

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
MpcTimeCmpCtrl::MpcTimeCmpCtrl(MpcTime* pTimeObj, Observer* pObserver, TIMER_CMP_PRECISION cmpPrecision)
{
  Subscribe(pObserver);  // AUTO SUBSCRIBE THE OBJ. that Ownes the the MPC Time cmp.

  ActTime::GetInstance()->Subscribe(this); // AUTO SUBSCRIBE THE OBJ to the ACT. time obj.
  mpTimeObj = pTimeObj;
  mNewTime = true;
  mCmpPrecision = cmpPrecision;
  mCurrentMinute = -1;
}
/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
void MpcTimeCmpCtrl::ChangeTime(MpcTime* pTimeObj)
{
  mpTimeObj = pTimeObj;
}
/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
MpcTimeCmpCtrl::~MpcTimeCmpCtrl()
{

}

/****************************************************************************************
 * Function - Update
 * DESCRIPTION:
 * TO DO:
 * Change back to ReqTestTime, We don't know the reason for the change to RunSubTask
 *
 **************************************************************************************/
void MpcTimeCmpCtrl::Update(Subject* pSubject)
{
  if ( ActTime::GetInstance() == pSubject )
  {
    mNewTime = true;
  }
  //ReqTaskTime();
  RunSubTask();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 ****************************************************************************/
void MpcTimeCmpCtrl::RunSubTask()
{
  bool return_value = false;

  if (mNewTime == true)
  {
    auto int new_minute;
    new_minute = ActTime::GetInstance()->GetTime(MINUTES);
    if (new_minute != mCurrentMinute)
    {
      if (mCurrentMinute == -1)
      {
        // First time, init hour + day triggers
        mCurrentDay =  ActTime::GetInstance()->GetDate(DAY);
        mCurrentHour = ActTime::GetInstance()->GetTime(HOURS);
      }
      mCurrentMinute = new_minute;

      mNewTime = false;
      switch ( mCmpPrecision)
      {
      case CMP_DATE_AND_TIME:
        if ( mpTimeObj->GetTime(MINUTES) == ActTime::GetInstance()->GetTime(MINUTES))
        {
          if ( mpTimeObj->GetDate(DAY) == ActTime::GetInstance()->GetDate(DAY))
          {
            if ( mpTimeObj->GetTime(HOURS) == ActTime::GetInstance()->GetTime(HOURS))
            {
              if ( mpTimeObj->GetDate(MONTH) == ActTime::GetInstance()->GetDate(MONTH))
              {
                if ( mpTimeObj->GetDate(YEAR) == ActTime::GetInstance()->GetDate(YEAR))
                {
                  return_value = true;
                }
              }
            }
          }
        }
        break;

      case CMP_WEEKDAY_AND_TIME:
        if ( mpTimeObj->GetTime(MINUTES) == ActTime::GetInstance()->GetTime(MINUTES))
        {
          if ( mpTimeObj->GetTime(HOURS) == ActTime::GetInstance()->GetTime(HOURS))
          {
            if ( mpTimeObj->GetDate(DAY_OF_WEEK) == ActTime::GetInstance()->GetDate(DAY_OF_WEEK))
            {
              return_value = true;
            }
          }
        }
        break;


      case CMP_TIME:
        if ( mpTimeObj->GetTime(MINUTES) == ActTime::GetInstance()->GetTime(MINUTES))
        {
          if ( mpTimeObj->GetTime(HOURS) == ActTime::GetInstance()->GetTime(HOURS))
          {
            return_value = true;
          }
        }
        break;

      case DAY_TRIGGER:
        if (mCurrentDay != ActTime::GetInstance()->GetDate(DAY))
        {
          mCurrentDay = ActTime::GetInstance()->GetDate(DAY);
          return_value = true;
        }
        break;

      case HOUR_TRIGGER:
        if (mCurrentHour != ActTime::GetInstance()->GetTime(HOURS))
        {
          mCurrentHour = ActTime::GetInstance()->GetTime(HOURS);
          return_value = true;
        }
        break;

      case MINUTE_TRIGGER:
        return_value = true;
        break;
      }

      if ( return_value == true)
      {
        NotifyObservers();
      }
    }
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
