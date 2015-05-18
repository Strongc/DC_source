/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : UserDefinedCounterCtrl                                */
/*                                                                          */
/* FILE NAME        : UserDefinedCounterCtrl.cpp                            */
/*                                                                          */
/* CREATED DATE     : 16-12-2011 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 ****************************************************************************/
 #include <Math.h>

/*****************************************************************************
  PROJECT INCLUDES
 ****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <UserDefinedCounterCtrl.h>
#include <MpcTime.h>
/*****************************************************************************
  DEFINES
 ****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 ****************************************************************************/



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
UserDefinedCounterCtrl::UserDefinedCounterCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
UserDefinedCounterCtrl::~UserDefinedCounterCtrl()
{
	delete mTimeChangeObj;
	delete mTimeCompareObs;
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::InitSubTask()
{ 
  mTimeChangeObj = new MpcTime(false);
  mTimeCompareObs = new MpcTimeCmpCtrl(mTimeChangeObj, this, MINUTE_TRIGGER);

  for(int use_counter = 0; use_counter < NO_OF_USD_COUNTERS; use_counter++)
  {
    mOldPulses[use_counter] = 0;
    mPulsesReady[use_counter] = false;
    mIncrement[use_counter] = 0;
    mOldPulseRatioDisplay[use_counter] = 1;
    mpTotalUserDefinedCount[use_counter]->SetQuality(DP_NEVER_AVAILABLE);
	today_cnt[use_counter] = 0;
	total_today_cnt[use_counter] = 0;
	yesterday[use_counter] = 0;
	mpTodayCounter[use_counter]->SetQuality(DP_NEVER_AVAILABLE);
	mpYesterdayCounter[use_counter]->SetQuality(DP_NEVER_AVAILABLE);
  }
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::RunSubTask()
{  
  HandleResetEvents();
  for(int use_counter = 0; use_counter < NO_OF_USD_COUNTERS; use_counter++)
  {
    CheckUSDCounterConfig(use_counter);
    UpdateAccumulatedUSDCounter(use_counter);   
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::ConnectToSubjects()
{
  for(int use_counter = 0; use_counter < NO_OF_USD_COUNTERS; use_counter++)
  {
    mpPulseUSDCntRatioDisplay[use_counter]->Subscribe(this);
    mpResetUserDefinedCounter[use_counter]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Check if pointer pSubject is one of the subjects observed.
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::Update(Subject* pSubject)
{
  for(int use_counter = 0; use_counter < NO_OF_USD_COUNTERS; use_counter++)
  {
    mpPulseUSDCntRatioDisplay[use_counter].Update(pSubject);
    mpResetUserDefinedCounter[use_counter].Update(pSubject);
	mpRawUserDefinedCntPulses[use_counter].Update(pSubject);
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubject to the member pointer for this subject.
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Variable inputs:
    case SP_USDC_RAW_USD_CNT_1_PULSES:
      mpRawUserDefinedCntPulses[0].Attach(pSubject);
      break;
    case SP_USDC_RAW_USD_CNT_2_PULSES:
      mpRawUserDefinedCntPulses[1].Attach(pSubject);
      break;
    case SP_USDC_RAW_USD_CNT_3_PULSES:
      mpRawUserDefinedCntPulses[2].Attach(pSubject);
      break;

    // Configuration inputs
    case SP_USDC_PULSE_USD_CNT_1_RATIO_DISPLAY:
      mpPulseUSDCntRatioDisplay[0].Attach(pSubject);
      break;
    case SP_USDC_PULSE_USD_CNT_2_RATIO_DISPLAY:
      mpPulseUSDCntRatioDisplay[1].Attach(pSubject);
      break;
    case SP_USDC_PULSE_USD_CNT_3_RATIO_DISPLAY:
      mpPulseUSDCntRatioDisplay[2].Attach(pSubject);
      break;

    // Outputs:
    case SP_USDC_TOTAL_USD_CNT_1:
      mpTotalUserDefinedCount[0].Attach(pSubject);
      break;
    case SP_USDC_TOTAL_USD_CNT_2:
      mpTotalUserDefinedCount[1].Attach(pSubject);
      break;
    case SP_USDC_TOTAL_USD_CNT_3:
      mpTotalUserDefinedCount[2].Attach(pSubject);
      break;

    //counter reset events
    case SP_USDC_RESET_USD_CNT_1_EVENT:
      mpResetUserDefinedCounter[0].Attach(pSubject);
      break;
    case SP_USDC_RESET_USD_CNT_2_EVENT:
      mpResetUserDefinedCounter[1].Attach(pSubject);
      break;
    case SP_USDC_RESET_USD_CNT_3_EVENT:
      mpResetUserDefinedCounter[2].Attach(pSubject);
      break;

	case SP_USDC_OLD_HOUR_LOG_TIMESTAMP:
	  mpHourLogTimestamp[0].Attach(pSubject);
	  break;
	case  SP_USDC_OLD_HOUR_LOG_TIMESTAMP_COUNTER2:
	  mpHourLogTimestamp[1].Attach(pSubject);
	  break;
	case SP_USDC_OLD_HOUR_LOG_TIMESTAMP_COUNTER3:
	  mpHourLogTimestamp[2].Attach(pSubject);
	  break;
	case SP_USDC_YESTERDAY_USD_COUNT_0:
	  mpYesterdayCounter[0].Attach(pSubject);
	  break;
	case SP_USDC_YESTERDAY_USD_COUNT_1:
	  mpYesterdayCounter[1].Attach(pSubject);
	  break;
	case SP_USDC_YESTERDAY_USD_COUNT_2:
	  mpYesterdayCounter[2].Attach(pSubject);
	  break;
	case SP_USDC_TODAY_USD_COUNT_0:
	  mpTodayCounter[0].Attach(pSubject);
	  break;
	case SP_USDC_TODAY_USD_COUNT_1:
	  mpTodayCounter[1].Attach(pSubject);
	  break;
	case SP_USDC_TODAY_USD_COUNT_2:
	  mpTodayCounter[2].Attach(pSubject);
	  break;

    default:
      break;
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
 * Function - UpdateAccumulatedUSDCounter
 * DESCRIPTION: Update counter accumulators
 *
 *****************************************************************************/
void UserDefinedCounterCtrl::UpdateAccumulatedUSDCounter(int use_counter)
{
  MpcTime new_time(true);
  U32 new_timestamp = new_time.GetSecondsSince1Jan1970();
  U32 new_hour = new_timestamp/3600;
  U32 old_hour = mpHourLogTimestamp[use_counter]->GetValue()/3600;

  if(old_hour ==0)
  {
		mpHourLogTimestamp[use_counter]->SetValue(new_timestamp);
  }


  if( mpRawUserDefinedCntPulses[0].IsUpdated())
  {
		U8 new_pulses = (U8)mpRawUserDefinedCntPulses[use_counter]->GetValue();
  }


  if (mpRawUserDefinedCntPulses[use_counter]->GetQuality() == DP_AVAILABLE)
  {
    U8 delta_pulses = 0;
    U8 new_pulses = (U8)mpRawUserDefinedCntPulses[use_counter]->GetValue();
    mpTotalUserDefinedCount[use_counter]->SetQuality(DP_AVAILABLE);
    if (mPulsesReady[use_counter] == false)
    {
      // Wait until pulse input is alive
      if (new_pulses != 0)
      {
        mPulsesReady[use_counter] = true;
        mOldPulses[use_counter] = new_pulses;
      }      
    }
    else
    {
	    float total_usdcnt = mpTotalUserDefinedCount[use_counter]->GetValue();
	    float pulse_usdcnt_ratio = mpPulseUSDCntRatioDisplay[use_counter]->GetValue();
//		float today_cnt[3];

      // Update pulses and calculate energy increment
      delta_pulses = new_pulses - mOldPulses[use_counter];
      if (delta_pulses > 40)
      {
        // Some kind of start up or illegal situation
        delta_pulses = 0; 
      }
//today_cnt- Start
	  if (new_hour > old_hour)
	  {
		if (new_hour%24 ==0)
		{			
			yesterday[use_counter] = total_today_cnt[use_counter];
			mpYesterdayCounter[use_counter]->SetValue(yesterday[use_counter]);
			total_today_cnt[use_counter] = 0;	
			mpTodayCounter[use_counter]->SetValue(total_today_cnt[use_counter]);			

		}
		 mpHourLogTimestamp[use_counter]->SetValue(new_timestamp);
	  }
	  else if (new_hour < old_hour)
      {
         // The clock must have been set back, e.g. daylight saving period gone
		// Just prepare to trig the logging on next clock hour
        mpHourLogTimestamp[use_counter]->SetValue(new_timestamp);
	  }
	  		
		 today_cnt[use_counter]       =  (((mIncrement[use_counter]) * pulse_usdcnt_ratio) + delta_pulses)/pulse_usdcnt_ratio;
		// total_today_cnt[use_counter] =   total_today_cnt[use_counter]+ today_cnt[use_counter]+ mpTodayCounter[use_counter]->GetValue();
		  total_today_cnt[use_counter] =   today_cnt[use_counter]+ mpTodayCounter[use_counter]->GetValue();
	     mpTodayCounter[use_counter]->SetValue(total_today_cnt[use_counter]);
//today_cnt-End



      mOldPulses[use_counter] = new_pulses;
	    mIncrement[use_counter] = (((total_usdcnt + mIncrement[use_counter]) * pulse_usdcnt_ratio) + 
                                    delta_pulses)/pulse_usdcnt_ratio;
	    mIncrement[use_counter] = modff(mIncrement[use_counter], &total_usdcnt);
	    mpTotalUserDefinedCount[use_counter]->SetValue((U32)total_usdcnt);	    
    }
  }
  else
  {
    // No User defined counter pulses available.
    mPulsesReady[use_counter] = false;    
    mpTotalUserDefinedCount[use_counter]->SetValue(0);
	  mpTotalUserDefinedCount[use_counter]->SetQuality(DP_NEVER_AVAILABLE); 
	//  mpTodayCounter[use_counter]->SetValue(0);
	  mpTodayCounter[use_counter]->SetQuality(DP_NEVER_AVAILABLE);
	//  mpYesterdayCounter[use_counter]->SetValue(0);
	  mpYesterdayCounter[use_counter]->SetQuality(DP_NEVER_AVAILABLE);

  }
}

/************************************************************************************
 * Function - CheckUSDCounterConfig
 * DESCRIPTION: Handle the User defined counter configuration parameters for display.
 *
 ************************************************************************************/
void UserDefinedCounterCtrl::CheckUSDCounterConfig(int use_counter)
{  
  if(mpPulseUSDCntRatioDisplay[use_counter].IsUpdated())
  {
    float total_usdcnt = mpTotalUserDefinedCount[use_counter]->GetValue();
	  float pulse_usdcnt_ratio = mpPulseUSDCntRatioDisplay[use_counter]->GetValue();

	  mIncrement[use_counter] = ((total_usdcnt + mIncrement[use_counter]) * mOldPulseRatioDisplay[use_counter])/pulse_usdcnt_ratio;
    mOldPulseRatioDisplay[use_counter] = pulse_usdcnt_ratio;
	  mIncrement[use_counter] = modff(mIncrement[use_counter], &total_usdcnt);
	  mpTotalUserDefinedCount[use_counter]->SetValue((U32)total_usdcnt);
	  mpTotalUserDefinedCount[use_counter]->SetQuality(DP_AVAILABLE);
  }
}
/************************************************************************************
 * Function - HandleResetEvents
 * DESCRIPTION: Resets User defined counter.
 *
 ************************************************************************************/
void UserDefinedCounterCtrl::HandleResetEvents()
{
  for(int use_counter = 0; use_counter < NO_OF_USD_COUNTERS; use_counter++)
  {
    if(mpResetUserDefinedCounter[use_counter].IsUpdated())
    {
      mpTotalUserDefinedCount[use_counter]->SetValue(0);
      mpRawUserDefinedCntPulses[use_counter]->SetValue(0);
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
