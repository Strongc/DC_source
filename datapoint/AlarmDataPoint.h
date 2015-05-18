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
/* CLASS NAME       : AlarmDataPoint                                        */
/*                                                                          */
/* FILE NAME        : AlarmDataPoint.h                                      */
/*                                                                          */
/* CREATED DATE     : 30-05-2007  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Data point used for interfacing AlarmControl.   */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ALARM_DATA_POINT_H__
#define __ALARM_DATA_POINT_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <DataPoint.h>
#include <AlarmDef.h>
#include <FactoryTypes.h>
#include <AlarmConfig.h>
#include <SubjectPtr.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
#define ALARM_COUNTER_MAX 2000000000

/*****************************************************************************
 * CLASS: AlarmDataPoint
 *****************************************************************************/
class AlarmDataPoint : public DataPoint, public Observer
{
public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmDataPoint() : Observer()
    {
      mAlarmCounter = 0;
      mActConfig = 0;
      mErrorPresent = ALARM_STATE_READY;
      mAlarmPresent = ALARM_STATE_READY;
      mActive = true;

      #ifdef __PC__
      mErrorPresentForSimulation = false;
      #endif //__PC__
    }

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~AlarmDataPoint()
    {
    }

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/

public:

    /*****************************************************************************
    * Function - Assignment operator
    * DESCRIPTION: - param src The value to assign to this object.
    *                return A reference to this object.
    *
    ****************************************************************************/
    virtual AlarmDataPoint& operator=(const AlarmDataPoint& src)
    {
      if (this != &src)
      {
        mValue = src.mValue;
        mErroneousUnit = src.mErroneousUnit;
        mAlarmCounter = src.mAlarmCounter;
        mErroneousUnitNumber = src.mErroneousUnitNumber;
        mAlarmPresent = src.mAlarmPresent;
        mErrorPresent = src.mErrorPresent;
        mpAlarmConfig = src.mpAlarmConfig;
        mpAlarmConfig2 = src.mpAlarmConfig2;
        mActive = src.mActive;
      }
      return *this;
    }// =

    /********************************************************************
    Function - GetValue
    ********************************************************************/
    virtual ALARM_ID_TYPE GetValue(void)
    {
      ALARM_ID_TYPE ret_val;

      OS_EnterRegion();
      ret_val = mValue;
      OS_LeaveRegion();

      return ret_val;
    }

    /********************************************************************
    Function - SetValue
    ********************************************************************/
    virtual bool SetValue(ALARM_ID_TYPE newValue)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mValue != newValue)
      {
        mValue = newValue;
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - GetAlarmCounter
    ********************************************************************/
    virtual int GetAlarmCounter()
    {
      return mAlarmCounter;
    }

    /********************************************************************
    Function - SetAlarmCounter
    ********************************************************************/
    virtual bool SetAlarmCounter(int alarmCounter)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mAlarmCounter != alarmCounter)
      {
        mAlarmCounter = alarmCounter;
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - ResetAlarmCounter
    ********************************************************************/
    virtual bool ResetAlarmCounter()
    {
      bool notify = false;

      OS_EnterRegion();
      if (mAlarmCounter != 0)
      {
        mAlarmCounter = 0;
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - SetDataPointActive
    ********************************************************************/
    virtual bool SetDataPointActive(bool state)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mActive != state)
      {
        mActive = state;
        if (mActive == true)
        {
          // If the datapoint is set active mAlarmPresent should be syncronized to
          // mErrorPresent to handle if an alarm has been manually reset from the
          // current alarm list. This is done by setting mErrorPresent to its own value
          // this will not change mErrorPresent, but mAlarmPresent will be checked and
          // set if needed.
          SetErrorPresent(mErrorPresent);
        }
        notify = true;
      }
      OS_LeaveRegion();

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - GetErroneousUnit
    ********************************************************************/
    virtual ERRONEOUS_UNIT_TYPE GetErroneousUnit()
    {
      ERRONEOUS_UNIT_TYPE ret_val;
      OS_EnterRegion();
      ret_val = mErroneousUnit;
      OS_LeaveRegion();
      return ret_val;
    }

    /********************************************************************
    Function - SetErroneousUnit
    ********************************************************************/
    virtual bool SetErroneousUnit(ERRONEOUS_UNIT_TYPE erroneousUnit)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mErroneousUnit != erroneousUnit)
      {
        mErroneousUnit = erroneousUnit;
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }
    /********************************************************************
    Function - GetErroneousUnitNumber
    ********************************************************************/
    virtual int GetErroneousUnitNumber()
    {
      return mErroneousUnitNumber;
    }

    /********************************************************************
    Function - SetErroneousUnit
    ********************************************************************/
    virtual bool SetErroneousUnitNumber(int erroneousUnitNumber)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mErroneousUnitNumber != erroneousUnitNumber)
      {
        mErroneousUnitNumber = erroneousUnitNumber;
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - GetAlarmPresent
    ********************************************************************/
    virtual ALARM_STATE_TYPE GetAlarmPresent()
    {
      if(mActive)
        return mAlarmPresent;
      else
        return ALARM_STATE_READY;

    }

    /********************************************************************
    Function - SetAlarmPresent
    ********************************************************************/
    virtual bool SetAlarmPresent(ALARM_STATE_TYPE alarmPresent)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mAlarmPresent != alarmPresent)
      {
        mAlarmPresent = alarmPresent;
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - GetErrorPresent
    ********************************************************************/
    virtual ALARM_STATE_TYPE GetErrorPresent()
    {
      if(mActive)
        return mErrorPresent;
      else
        return ALARM_STATE_READY;
    }

    /********************************************************************
    Function - SetErrorPresent
    ********************************************************************/
    virtual bool SetErrorPresent(ALARM_STATE_TYPE errorPresent)
    {
      #ifdef __PC__
      if (mErrorPresentForSimulation)
      {
        errorPresent = mErrorPresent;
      }
      #endif //__PC__

      bool notify = false;

      OS_EnterRegion();

      if (mErrorPresent != errorPresent)
      {
        if (mErrorPresent == ALARM_STATE_READY)
        {
          // The state is changing from ready - that means an alarm or warning has occured
          if (mAlarmCounter < ALARM_COUNTER_MAX)
          {
            mAlarmCounter++;
          }
        }

        mErrorPresent = errorPresent;
        notify = true;
      }

      // Check if mAlarmPresent should be syncronized with the mErrorPresent.
      // This check must be done - even if mErrorPresent has not been changed - because the setting of AutoAcknowledge
      // could be changed. Especially if a change has happened from manual ack to auto ack it is important that a
      // hanging mAlarmPresent is set to the value of mErrorPresent.
      if (GetAlarmConfig()->GetAutoAcknowledge() == true)
      {
        // If alarmdatapoint is configured for autoacknowledge error and alarm present are set and reset together.
        if (mAlarmPresent != errorPresent)
        {
          mAlarmPresent = errorPresent;
          notify = true;
        }
      }
      else
      {
        // If alarmdatapoint is configured for manual acknowledge alarm present must not be changed if it is
        // ALARM_STATE_ALARM. Such an alarm present must be reset manually.
        // ALARM_STATE_WARNING is default autoacknowledge and is treated as so.
        if (mAlarmPresent != ALARM_STATE_ALARM)
        {
          if (mAlarmPresent != errorPresent)
          {
            mAlarmPresent = errorPresent;
            notify = true;
          }
        }
      }

      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();

      return notify;
    }

    /********************************************************************
    Function - GetAlarmConfig
    ********************************************************************/
    virtual AlarmConfig* GetAlarmConfig()
    {
      switch ( mActConfig)
      {
        case 0 :
          return mpAlarmConfig.GetSubject();
        case 1 :
          return mpAlarmConfig2.GetSubject();
        default:
          return mpAlarmConfig.GetSubject();
      }
    }


    /********************************************************************
    Function - SetAlarmConfig
    ********************************************************************/
    virtual bool SetAlarmConfig(AlarmConfig* pAlarmConfig)
    {

      OS_EnterRegion();
      if (!mpAlarmConfig.IsValid())
      {
        mpAlarmConfig.Attach(pAlarmConfig);
      }
      else
      {
        // if already set, then copy by value
        *(mpAlarmConfig.GetSubject()) = *pAlarmConfig;
      }
      OS_LeaveRegion();

      return false;
    }

    /********************************************************************
    Function - SetAlarmConfig2
    ********************************************************************/
    virtual bool SetAlarmConfig2(AlarmConfig* pAlarmConfig)
    {

      OS_EnterRegion();
      if (!mpAlarmConfig2.IsValid())
      {
        mpAlarmConfig2.Attach(pAlarmConfig);
      }
      else
      {
        // if already set, then copy by value
        *(mpAlarmConfig2.GetSubject()) = *pAlarmConfig;
      }
      OS_LeaveRegion();

      return false;
    }

    /********************************************************************
    Function - SetActualAlarmConfig
    ********************************************************************/
    virtual void SetActualAlarmConfig(int actConfig)
    {
      bool notify = false;

      OS_EnterRegion();
      if (mActConfig != actConfig)
      {
        if (actConfig>=0 && actConfig<=1)
        {
          mActConfig = actConfig;
        }
        else
        {
          mActConfig = 0;
        }
        notify = true;
      }
      OS_LeaveRegion();

      notify |= SetQuality(DP_AVAILABLE, false);

      if (notify)
        NotifyObservers();

      NotifyObserversE();
    }

    /********************************************************************
    Function - SimulateErrorPresent
    Only used for pc simulator
    ********************************************************************/
#ifdef __PC__
    void SimulateErrorPresent(bool errorPresent, bool warningPresent)
    {
      mErrorPresentForSimulation = false;

      if (errorPresent)
      {
        SetErrorPresent(ALARM_STATE_ALARM);
        mErrorPresentForSimulation = true;
      }
      else if (warningPresent)
      {
        SetErrorPresent(ALARM_STATE_WARNING );
        mErrorPresentForSimulation = true;
      }
      else
      {
        SetErrorPresent(ALARM_STATE_READY);
      }
    }
#endif //__PC__


  /*****************************************************************************
  * Observer::Update implementation
  *****************************************************************************/
  void Update(Subject* pSubject)
  {
    bool update = false;

    update |= mpAlarmConfig.Update(pSubject);
    update |= mpAlarmConfig2.Update(pSubject);
    if (update)
    {
      mpAlarmConfig.ResetUpdated();
      mpAlarmConfig2.ResetUpdated();

      OS_EnterRegion();
      if(mActConfig==1)
      {
        if (mpAlarmConfig2->GetAutoAcknowledge() == true)
        {
          // Acknowledge is auto - assure that mErrorPresent and mAlarmPresent are
          // syncronized. This might not be the case, if the Acknowledge setting
          // has just been changed.
          SetAlarmPresent(GetErrorPresent());
        }
      }
      else
      {
        if (mpAlarmConfig->GetAutoAcknowledge() == true)
        {
          // Acknowledge is auto - assure that mErrorPresent and mAlarmPresent are
          // syncronized. This might not be the case, if the Acknowledge setting
          // has just been changed.
          SetAlarmPresent(GetErrorPresent());
        }
      }
      OS_LeaveRegion();

      NotifyObservers();
    }
    NotifyObserversE();
  }

  /*****************************************************************************
  * Observer::SetSubjectPointer implementation
  *****************************************************************************/
  void SetSubjectPointer(int Id,Subject* pSubject)
  {
    //TEST JSM This code does not seem to be used !
    switch ( Id)
    {
      case 0 :
        mpAlarmConfig.Attach(pSubject);
        break;
      case 1 :
        mpAlarmConfig2.Attach(pSubject);
        break;
      default:
        mpAlarmConfig.Attach(pSubject);
        break;
    }
  }

  /*****************************************************************************
  * Observer::ConnectToSubjects implementation
  *****************************************************************************/
  void ConnectToSubjects(void)
  {
    if ( mpAlarmConfig.IsValid() )
    {
      mpAlarmConfig->Subscribe(this);
    }
    if ( mpAlarmConfig2.IsValid() )
    {
      mpAlarmConfig2->Subscribe(this);
    }
  }

  /*****************************************************************************
  * Observer::SubscribtionCancelled implementation
  *****************************************************************************/
  void SubscribtionCancelled(Subject* pSubject)
  {
    mpAlarmConfig.Detach(pSubject);
    mpAlarmConfig2.Detach(pSubject);
  }


  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    ALARM_ID_TYPE mValue;
    ERRONEOUS_UNIT_TYPE mErroneousUnit;
    int mAlarmCounter;
    int mErroneousUnitNumber;
    bool mActive;
    ALARM_STATE_TYPE mAlarmPresent;
    ALARM_STATE_TYPE mErrorPresent;
    SubjectPtr<AlarmConfig*> mpAlarmConfig;
    SubjectPtr<AlarmConfig*> mpAlarmConfig2;
    int mActConfig;
#ifdef __PC__
    bool mErrorPresentForSimulation;
#endif //__PC__
};

#endif
