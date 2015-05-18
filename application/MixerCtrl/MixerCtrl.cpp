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
/* CLASS NAME       : MixerCtrl                                             */
/*                                                                          */
/* FILE NAME        : MixerCtrl.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 05-09-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
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
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <MixerCtrl.h>         // class implemented

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  MIXER_RUN_TIMER
};

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
 *****************************************************************************/
MixerCtrl::MixerCtrl()
{
  mRunRequestedFlag = false;

  // Create objects for handling setting, clearing and delaying of alarm and warnings
  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    mpMixerAlarmDelay[fault_id] = new AlarmDelay(this);
    mMixerAlarmDelayCheckFlag[fault_id] = false;
  }

}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
MixerCtrl::~MixerCtrl()
{
  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    delete(mpMixerAlarmDelay[fault_id]);
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void MixerCtrl::RunSubTask()
{
  mRunRequestedFlag = false;
  

  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    if (mMixerAlarmDelayCheckFlag[fault_id] == true)
    {
      mMixerAlarmDelayCheckFlag[fault_id] = false;
      mpMixerAlarmDelay[fault_id]->CheckErrorTimers();
    }
  }

  //Handle enable/disable
  if ( mpMixerEnabled.IsUpdated() || mpPitLevelCtrlType.IsUpdated())
  {
    if( mpMixerEnabled->GetValue()==false || mpPitLevelCtrlType->GetValue()==SENSOR_TYPE_FLOAT_SWITCHES )
    {
      mMixerCtrlMode = MM_PRE_DISABLED;
    }
    else
    {
      mMixerCtrlMode = MM_PRE_WAIT_FOR_RATIO;
    }
  }

  //Check for new configuration
  if ( mpMixerMaxRunTimeCnf.IsUpdated() )
  {
    mpTimerObjList[MIXER_RUN_TIMER]->SetSwTimerPeriod(mpMixerMaxRunTimeCnf->GetValue(), S, false);
  }

  //Calculate Mixer Start Level
  float mixer_start_level;
  mixer_start_level = mpStartLevel_1->GetValue()- mpMixerStartLevelOffset->GetValue();
 
  //Handle main state machine
  switch ( mMixerCtrlMode )
  {
    case MM_PRE_DISABLED :
      mpMixerRelay->SetValue( false );
      mpOprModeMixer->SetValue(ACTUAL_OPERATION_MODE_DISABLED);
      mMixerCtrlMode = MM_DISABLED;
      mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->ResetFault();
    case MM_DISABLED :
      /* Do nothing */
      break;
    case MM_PRE_WAIT_FOR_RATIO :
      mRatioCnt = 0;
      StopMixer();
      mMixerCtrlMode = MM_WAIT_FOR_RATIO;
      mpNumberOfRunningPumps.ResetUpdated();
      break;
    case MM_WAIT_FOR_RATIO :
	  if(mpMixFallingLevelOnly->GetValue()== true)
	  {
		  if((mpPitLevel->GetValue() > mpMixerStopLevel->GetValue()) && (mpPitLevel->GetValue() < mixer_start_level)&&((mprepitlevel > mixer_start_level) ||(mprepitinitlevel == true) ))
		  {
			mprepitinitlevel = false;
			mMixerCtrlMode = MM_PRE_STARTED;
			ReqTaskTime();
		  }
		  else
		  {
			   mMixerCtrlMode = MM_PRE_WAIT_FOR_RATIO;
			   ReqTaskTime();
		  }
		
	  }
	  else
	  {
		if(mpNumberOfRunningPumps.IsUpdated())
		{
			if(mpNumberOfRunningPumps->GetValue()==0)
			{
				mRatioCnt++;
				if( mRatioCnt >= mpMixerRatio->GetValue() )
				{
					mMixerCtrlMode = MM_WAIT_FOR_START_LEVEL;
					ReqTaskTime();
				}
			}
		}
	  }
      break;
    case MM_WAIT_FOR_START_LEVEL :
      if( mpPitLevel->GetValue() > mixer_start_level  )
      {
        if( mpMixerStopLevel->GetValue() < mixer_start_level ) //To prevent cycling between start and stop
        {
          //if( ((mpNumberOfRunningPumps->GetValue()==0) || (mpMixWhilePumping->GetValue()==true)) && (mpNoOfAvailablePumps->GetValue()>0))
          if ((mpNumberOfRunningPumps->GetValue()==0) || (mpMixWhilePumping->GetValue()==true))
          {
            mMixerCtrlMode = MM_PRE_STARTED;
            ReqTaskTime();
          }
        }
      }
      break;
    case MM_PRE_STARTED :
      
      if( mpMaxStartsPerHourAlarm->GetAlarmPresent() == ALARM_STATE_ALARM )
      {
        mMixerCtrlMode = MM_WAIT_FOR_STOP_LEVEL;
        ReqTaskTime();
      }
      else
      {
        mMixingTimeout = false;
        StartMixer();
        mMixerCtrlMode = MM_STARTED;
      }
    case MM_STARTED :
	  if(mpMixFallingLevelOnly->GetValue()== true)
	  {
		  if((mpPitLevel->GetValue() > mixer_start_level))
		  {
			 StopMixer();
			 mMixerCtrlMode = MM_WAIT_FOR_RATIO;
			 break;
		  }

	  }
	/*  if(mpPitLevel->GetValue() <  mixer_start_level && mpMixFallingLevelOnly->GetValue()== false)
	  {
		  mMixerCtrlMode = MM_PRE_WAIT_FOR_RATIO;
		  ReqTaskTime();

	  }*/
      if( (mpPitLevel->GetValue() <= mpMixerStopLevel->GetValue()) || ((mpNumberOfRunningPumps->GetValue()>0)&&(mpMixWhilePumping->GetValue()==false)) )
      {
        mMixerCtrlMode = MM_PRE_WAIT_FOR_RATIO;
        ReqTaskTime();
      }
      if( (mpNumberOfRunningPumps->GetValue()>0) && (mpMixWhilePumping->GetValue()==false) )
      {
        mMixerCtrlMode = MM_PRE_WAIT_FOR_RATIO;
        ReqTaskTime();
      }
      if(mMixingTimeout)
      {
        mMixerCtrlMode = MM_WAIT_FOR_STOP_LEVEL;
        ReqTaskTime();
        StopMixer();
      }
      //if( mpNoOfAvailablePumps->GetValue()==0 )
      //{
      //  mMixerCtrlMode = MM_WAIT_FOR_START_LEVEL;
      //  StopMixer();
      //}
      break;
    case MM_WAIT_FOR_STOP_LEVEL :
      if( mpPitLevel->GetValue() <= mpMixerStopLevel->GetValue() )
      {
        mMixerCtrlMode = MM_PRE_WAIT_FOR_RATIO;
        ReqTaskTime();
      }
      break;
    default:
      break;
  }

  //Handle contactor feedback
  if( mMixerCtrlMode != MM_DISABLED )
  {
    switch( mpMixerContactorFeedback->GetValue() )
    {
      case DIGITAL_INPUT_FUNC_STATE_ACTIVE :
        // 2012-03-08 XMD: Only update the operation mode if the feedback changed. Otherwise the (new) operation set by Start/Stop methods will be overwritten.
        if(mpMixerContactorFeedback.IsUpdated())
        {
          mpOprModeMixer->SetValue(ACTUAL_OPERATION_MODE_STARTED);
        }

        if( mpMixerRelay->GetValue() == false )
        {
          mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->SetFault();
        }
        else
        {
          mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->ResetFault();
        }
        break;
      case DIGITAL_INPUT_FUNC_STATE_NOT_ACTIVE :

        // 2012-03-08 XMD: Same as above.
        if(mpMixerContactorFeedback.IsUpdated())
        {
          mpOprModeMixer->SetValue(ACTUAL_OPERATION_MODE_STOPPED);
        }

        if( mpMixerRelay->GetValue() == true )
        {
          mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->SetFault();
        }
        else
        {
          mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->ResetFault();
        }
      break;
      case DIGITAL_INPUT_FUNC_STATE_NOT_CONFIGURED :
        mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->ResetFault();
        break;
      default:
        /* Not possible */
        break;
    }
  }

   //Get the previous pit level
  mprepitlevel = mpPitLevel->GetValue();

  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    mpMixerAlarmDelay[fault_id]->UpdateAlarmDataPoint();
  }

}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *
 *****************************************************************************/
void MixerCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_MC_OPR_MODE_MIXER:
      mpOprModeMixer.Attach(pSubject);
      break;
    case SP_MC_MIXER_RELAY:
      mpMixerRelay.Attach(pSubject);
      break;
    case SP_MC_MIXER_ENABLED:
      mpMixerEnabled.Attach(pSubject);
      break;
    case SP_MC_MIX_WHILE_PUMPING:
      mpMixWhilePumping.Attach(pSubject);
      break;
    case SP_MC_MIXER_RATIO:
      mpMixerRatio.Attach(pSubject);
      break;
    case SP_MC_MIXER_MAX_RUN_TIME:
      mpMixerMaxRunTimeCnf.Attach(pSubject);
      break;
    case SP_MC_PIT_LEVEL:
      mpPitLevel.Attach(pSubject);
      break;
    case SP_MC_START_LEVEL_1:
      mpStartLevel_1.Attach(pSubject);
      break;
    case SP_MC_MIXER_STOP_LEVEL:
      mpMixerStopLevel.Attach(pSubject);
      break;
    case SP_MC_MIXER_START_LEVEL_OFFSET:
      mpMixerStartLevelOffset.Attach(pSubject);
      break;
    case SP_MC_MIXER_CONTACTOR_FEEDBACK:
      mpMixerContactorFeedback.Attach(pSubject);
      break;
    case SP_MC_PIT_LEVEL_CTRL_TYPE:
      mpPitLevelCtrlType.Attach(pSubject);
      break;
    case SP_MC_MIXER_CONTACTOR_FEEDBACK_ALARM_OBJ :
      mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK]->SetSubjectPointer(Id, pSubject);
      break;
    case SP_MC_NUMBER_OF_RUNNING_PUMPS :
      mpNumberOfRunningPumps.Attach(pSubject);
      break;
    case SP_MC_NO_OF_AVAILABLE_PUMPS:
      mpNoOfAvailablePumps.Attach(pSubject);
      break;
    case SP_MC_MIX_ALARM_MIXER_STARTS_HOUR:
      mpMaxStartsPerHourAlarm.Attach(pSubject);
      break;
	case SP_MC_MIXER_FALLING_LEVEL_ONLY:
	  mpMixFallingLevelOnly.Attach(pSubject);
	  break;
    default:
      break;
  }
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void MixerCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[MIXER_RUN_TIMER])
  {
    mMixingTimeout = true;
  }
  mpMixerEnabled.Update( pSubject );
  mpPitLevelCtrlType.Update( pSubject );
  mpMixerMaxRunTimeCnf.Update( pSubject );
  mpNumberOfRunningPumps.Update( pSubject );
  mpMixerContactorFeedback.Update(pSubject);

  if (pSubject == mpMixerAlarmDelay[MM_FAULT_OBJ_CONTACTOR_FEEDBACK])
  {
    mMixerAlarmDelayCheckFlag[MM_FAULT_OBJ_CONTACTOR_FEEDBACK] = true;
  }

  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void MixerCtrl::SubscribtionCancelled(Subject* pSubject)
{
  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    mpMixerAlarmDelay[fault_id]->SubscribtionCancelled(pSubject);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void MixerCtrl::ConnectToSubjects()
{
  mpMixerContactorFeedback->Subscribe(this);
  mpMixerEnabled->Subscribe(this);
  mpMixerMaxRunTimeCnf->Subscribe(this);
  mpPitLevel->Subscribe(this);
  mpPitLevelCtrlType->Subscribe(this);
  mpMixWhilePumping->Subscribe(this);
  mpNumberOfRunningPumps->Subscribe(this);
  mpNoOfAvailablePumps->Subscribe(this);
  mpMixFallingLevelOnly->Subscribe(this);
	
  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    mpMixerAlarmDelay[fault_id]->ConnectToSubjects();
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void MixerCtrl::InitSubTask()
{
  ReqTaskTime();
  mpTimerObjList[MIXER_RUN_TIMER] = new SwTimer( mpMixerMaxRunTimeCnf->GetValue(), S, false, false, this);
  mpMixerEnabled.SetUpdated();

  for (int fault_id = FIRST_MM_FAULT_OBJ; fault_id < NO_OF_MM_FAULT_OBJ; fault_id++)
  {
    mpMixerAlarmDelay[fault_id]->InitAlarmDelay();
  }
  mprepitlevel = mpPitLevel->GetValue();
  mprepitinitlevel = true;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - StartMixer
 * DESCRIPTION:
 *
 *****************************************************************************/
void MixerCtrl::StartMixer()
{
  mpTimerObjList[MIXER_RUN_TIMER]->RetriggerSwTimer();
  mpMixerRelay->SetValue(true);
  mpOprModeMixer->SetValue(ACTUAL_OPERATION_MODE_STARTED);
}

/*****************************************************************************
 * Function - StopMixer
 * DESCRIPTION:
 *
 *****************************************************************************/
void MixerCtrl::StopMixer()
{
  mpOprModeMixer->SetValue(ACTUAL_OPERATION_MODE_STOPPED);
  mpMixerRelay->SetValue(false);
}

