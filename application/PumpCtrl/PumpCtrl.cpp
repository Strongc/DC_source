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
/* CLASS NAME       : PumpCtrl                                              */
/*                                                                          */
/* FILE NAME        : PumpCtrl.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 13-08-2007 dd-mm-yyyy                                 */
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
#include <ActTime.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <PumpCtrl.h>         // class implemented

/*****************************************************************************
  DEFINES
 *****************************************************************************/

// SW Timers
enum
{
  START_UP_DELAY_TIMER
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
PumpCtrl::PumpCtrl()
{
  mRunRequestedFlag = false;
  mPumpRefOld.Clear();
  mAlternationStartGroup = 0;
  mpTimerObjList[START_UP_DELAY_TIMER]   = new SwTimer(10, MS, false, false, this);
  mPumpInTempStop = NO_OF_PUMPS;
  mTempStopAllowed = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
PumpCtrl::~PumpCtrl()
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void PumpCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  int no_of_groups;
  int no_of_pumps = mpNoOfPumps->GetValue();
  PumpRef new_pump_ref(no_of_pumps);
  bool any_level;
  int i;
  int no_of_pumps_wish;
  int pump_group[MAX_NO_OF_PUMPS];
  bool manual_started[MAX_NO_OF_PUMPS];
  int manual_started_total;
  int manual_started_group;
  int reduce;
  bool anti_seiz_found;
  bool anti_seiz_pump[MAX_NO_OF_PUMPS];
  APPLICATION_MODE_TYPE mAppMode = APPLICATION_MODE_PUMPING;
  int group_to_clear;
  bool status_group_max[MAX_NO_OF_GROUPS];
  bool status_max = false;

  // Init some vars
  manual_started_total = 0;
  for( i=0; i<MAX_NO_OF_PUMPS; i++ )
  {
    manual_started[i] = false;
    anti_seiz_pump[i] = false;
  }

  /* Handle group config */
  if( mpGroup2Enabled->GetValue()==true )
  {
    no_of_groups = 2;
  }
  else
  {
    no_of_groups = 1;
  }
  for( i=0; i<no_of_pumps; i++ )
  {
    if( mpGroup2Enabled->GetValue()==true && i>=(mpGroup2FirstPump->GetValue()-1) )
    {
      pump_group[i] = 1; //Group 2
    }
    else
    {
      pump_group[i] = 0; //Group 1
    }
  }

  /* Get level references from level ctrl */
  for( i=0; i<no_of_pumps; i++ )
  {
    new_pump_ref.SetRef( i, mpRefLevel[i]->GetValue() );
  }

  /* Determine if any levels are set */
  any_level = false;
  if( new_pump_ref.NoOfPumps()>0)
  {
    any_level = true;
  }

  /* Check for daily emptying */
  if( (mpDailyEmptyingRequested->GetValue() == true) && new_pump_ref.NoOfPumps()==0 )
  {
    //TEST JSM  snak med JSZ om algoritme
    int found = 0;
    int min_no_of_pumps = mpMinStartedPumpsInTotal->GetValue();
    if (mpMinStartedPumpsInTotal->GetValue() < mpMinStartedPumpsInGroup[1]->GetValue())
    {
      min_no_of_pumps = mpMinStartedPumpsInGroup[1]->GetValue();
    }
    for( i=0; i<no_of_pumps; i++ )
    {
      if (pump_group[i] == 1 && found<min_no_of_pumps)
      {
        new_pump_ref.SetRef( i );
        found++;
      }
    }
    mAppMode = APPLICATION_MODE_DAILY_EMPTYING;
  }

  /* Check for foam drain */
  if (mpFoamDrainRequest->GetValue() )
  {
    mAppMode = APPLICATION_MODE_FOAM_DRAINING;
    if (any_level == true)
    {
      mFoamDrainRef = new_pump_ref;
    }
    else
    {
      new_pump_ref = mFoamDrainRef;
    }
  }

  /* Handle group alternation */
  if( mpGroupAlternation->GetValue()==true && no_of_groups==2 && mAlternationStartGroup==1)
  {
    new_pump_ref.ShiftRefs( mpGroup2FirstPump->GetValue()-1 );
  }

  for( int group=0; group<no_of_groups; group++ )
  {
    /* Find number of pumps started manually in group */
    manual_started_group = 0;
    for( i=0; i<no_of_pumps; i++ )
    {
      if(mpPumpReady[i]->GetValue()==false && mpOprModeActPump[i]->GetValue()==ACTUAL_OPERATION_MODE_STARTED  && pump_group[i]==group )
      {
        manual_started_total++;
        manual_started_group++;
        manual_started[i] = true;
      }
    }

    if( mpAlternationInGroup[group]->GetValue()==true )
    {
      /* Find number of pumps to run in group*/
      no_of_pumps_wish = 0;
      for( i=0; i<no_of_pumps; i++ )
      {
        if( new_pump_ref.GetRef(i)==true && pump_group[i]==group )
        {
          no_of_pumps_wish++;
        }
      }

      /* Limit number of started pumps to the configuration "MaxStartedPumpsInGroup" */
      status_group_max[group]  = no_of_pumps_wish>=mpMaxStartedPumpsInGroup[group]->GetValue() ? true  : false;

      if( no_of_pumps_wish>mpMaxStartedPumpsInGroup[group]->GetValue() )
      {
        no_of_pumps_wish = mpMaxStartedPumpsInGroup[group]->GetValue();
      }

      /* Limit nummber of started pumps */
      while( (manual_started_group+no_of_pumps_wish)>mpMaxStartedPumpsInGroup[group]->GetValue() && no_of_pumps_wish>0 )
      {
        no_of_pumps_wish--;
      }

      /* Find which pump to run, due to alternation */
      for( i=0; i<no_of_pumps; i++ )//Clear actual group
      {
        if (pump_group[i] == group)
        {
          new_pump_ref.ClearRef(i);
        }
      }

      int pumps_evaluated = 0;
      int pumps_found = 0;
      // Before alternation, check for a temporary stop/start request for group 1 and do not alternate in this case
      if (group == 0)
      {
        U8 temp_stop_pump;
        switch (CheckTempStop(no_of_pumps_wish, manual_started_group, &temp_stop_pump))
        {
          case TEMP_STOP_STOP:
            no_of_pumps_wish = 0;
            break;
          case TEMP_STOP_RESUME:
            new_pump_ref.SetRef(temp_stop_pump);
            pumps_found = 1;
            break;
          case TEMP_STOP_IGNORE:
          default:
            break;
        }
      } // End temporary stop/start handling

      while( pumps_found<no_of_pumps_wish && pumps_evaluated<no_of_pumps )// Find pumps already running
      {
        int p = GetPumpByTime( no_of_pumps-1-pumps_evaluated );
        if (mpPumpReady[p]->GetValue()==true && pump_group[p]==group && mpOprModeActPump[p]->GetValue()==ACTUAL_OPERATION_MODE_STARTED)
        {
          new_pump_ref.SetRef( p );
          pumps_found++;
        }
        pumps_evaluated++;
      }
      pumps_evaluated = 0;
      no_of_pumps_wish = no_of_pumps_wish-pumps_found;
      pumps_found = 0;
      while( pumps_found<no_of_pumps_wish && pumps_evaluated<no_of_pumps ) //Fill up with stopped pumps
      {
        int p = GetPumpByTime( pumps_evaluated );
        if( mpPumpReady[p]->GetValue()==true && pump_group[p]==group && new_pump_ref.GetRef( p )==false)
        {
          new_pump_ref.SetRef( p );
          pumps_found++;
        }
        pumps_evaluated++;
      }
    }
    else //No alternation in actual group
    {
      /* Remove "pump start wish" from pumps that are not ready */
      for( i=0; i<no_of_pumps; i++ )
      {
        if( mpPumpReady[i]->GetValue()==false && new_pump_ref.GetRef(i)==true && pump_group[i]==group )
        {
          new_pump_ref.ClearRef(i);
        }
      }

      /* Find number of pumps to run wish*/
      no_of_pumps_wish = 0;
      for( i=0; i<no_of_pumps; i++ )
      {
        if( new_pump_ref.GetRef(i)==true && pump_group[i]==group )
        {
          no_of_pumps_wish++;
        }
      }

      /* Limit nummber of started pumps */
      reduce = (no_of_pumps_wish+manual_started_group)-mpMaxStartedPumpsInGroup[group]->GetValue();
      // Check for a temporary stop request for group 1
      if (group == 0)
      {
        U8 temp_stop_pump;
        if (CheckTempStop(no_of_pumps_wish, manual_started_group, &temp_stop_pump) == TEMP_STOP_STOP)
        {
          new_pump_ref.ClearRef(temp_stop_pump);
        }
      } // End temporary stop/start handling
      status_group_max[group]  = reduce>=0 ? true  : false;  //Used for system status
      if( reduce>0 )
      {
        if( reduce>no_of_pumps_wish )
        {
          reduce = no_of_pumps_wish;
        }
        i = no_of_pumps;
        while( reduce>0 && i>0 )
        {
          i--;
          if( new_pump_ref.GetRef(i)==true && pump_group[i]==group )
          {
            new_pump_ref.ClearRef(i);
            reduce--;
          }
        }
      }
    }

    /* Handle Anti Seizing */
    no_of_pumps_wish = 0;
    anti_seiz_found = false;
    for( i=0; i<no_of_pumps; i++ )  // Find number of pumps to run requested by level control
    {
      if( new_pump_ref.GetRef(i)==true && pump_group[i]==group )
      {
        no_of_pumps_wish++;
      }
    }

    for( i=0; i<no_of_pumps; i++ )
    {
      // Check anti seizing request for pumps in group.
      // Normally just anti seize one pump at a time, but use pumps when mpMinStartedPumpsInGroup must be fulfilled.
      if ( (mpAntiSeizingRequestPump[i]->GetValue() == true)
        && (pump_group[i] == group)
        && (mpPumpReady[i]->GetValue() == true)
        && (new_pump_ref.GetRef(i) == false)
        && ((no_of_pumps_wish+manual_started_group) < mpMinStartedPumpsInGroup[group]->GetValue() || anti_seiz_found == false)
        && ((no_of_pumps_wish+manual_started_group) < mpMaxStartedPumpsInGroup[group]->GetValue()) )
      {
        new_pump_ref.SetRef(i);
        anti_seiz_pump[i] = true;
        anti_seiz_found = true;
        no_of_pumps_wish++;
      }
    }

    /* Handle min started pumps in group */
    if( manual_started_group+no_of_pumps_wish<mpMinStartedPumpsInGroup[group]->GetValue() )
    {
      for( i=0; i<no_of_pumps; i++ )
      {
        if( pump_group[i]==group )
        {
          new_pump_ref.ClearRef(i);
        }
      }
    }
  }//Each group

  if (no_of_groups==2)
  {
    /* Handle configuration for group concurrent run */
    if( mpGroupsMayRunTogether->GetValue() == false )
    {
      int grp_1_active_pumps = 0;
      int grp_2_active_pumps = 0;
      for( i=0; i<no_of_pumps; i++ )
      {
        if(new_pump_ref.GetRef( i ) == true || manual_started[i]==true )
        {
          if( pump_group[i] == 0 )
          {
            grp_1_active_pumps++;
          }
          else
          {
            grp_2_active_pumps++;
          }
        }
      }
      if( grp_1_active_pumps>0 && grp_2_active_pumps>0 ) //Pumps active in both groups ?
      {
        // Clear group 2 if it just want anti seizing, otherwise clear group 1
        group_to_clear = 1;
        for( i=0; i<no_of_pumps; i++ )
        {
          if( pump_group[i]==1 && anti_seiz_pump[i] == false && (new_pump_ref.GetRef(i)==true || manual_started[i]==true) )
          {
            group_to_clear = 0; // Something in group 2 started and not anti seizing, clear group 1
          }
        }
        /* Clear group */
        for( i=0; i<no_of_pumps; i++ )
        {
          if( pump_group[i]==group_to_clear )
          {
            new_pump_ref.ClearRef( i );
          }
        }
      }
    }

    /* HandleTotalMin */
    if (new_pump_ref.NoOfPumps()+manual_started_total < mpMinStartedPumpsInTotal->GetValue())
    {
      /* Clear all pumps */
      new_pump_ref.Clear();
    }

    /* HandleTotalMax */
    reduce = (new_pump_ref.NoOfPumps() + manual_started_total) - mpMaxStartedPumpsInTotal->GetValue();
    status_max = reduce >= 0 ? true  : false;  // Used for system status
    // First remove anti seizing pumps
    i = no_of_pumps;
    while (reduce>0 && i>0)
    {
      i--;
      if (anti_seiz_pump[i] == true && new_pump_ref.GetRef(i) == true)
      {
        new_pump_ref.ClearRef(i);
        reduce--;
      }
    }
    // Then remove other pumps
    i = no_of_pumps;
    while (reduce>0 && i>0)
    {
      i--;
      if (new_pump_ref.GetRef(i) == true)
      {
        new_pump_ref.ClearRef(i);
        reduce--;
      }
    }

    /* Handle group alternation */
    if( mPumpRefOld.NoOfPumps()>0 && new_pump_ref.NoOfPumps()==0 )
    {
      mAlternationStartGroup = (mAlternationStartGroup+1)%2;  //Change start group
    }
  }
  else //Only one group, update status_max for system status
  {
    status_max = new_pump_ref.NoOfPumps()+manual_started_total >= mpMaxStartedPumpsInGroup[0]->GetValue() ? true  : false;
  }

  /* Handle system mode */
  if( mAppMode == APPLICATION_MODE_PUMPING)
  {
    if (status_max)
    {
      mAppMode = APPLICATION_MODE_PUMPING_MAX;
    }
    else if (new_pump_ref.NoOfPumps() == 0)
    {
      mAppMode = APPLICATION_MODE_STANDBY;
    }
    else if (no_of_groups==2)
    {
      if (status_group_max[0] && status_group_max[1])
      {
        mAppMode = APPLICATION_MODE_PUMPING_MAX;
      }
    }
    else //Only group 1
    {
      if (status_group_max[0])
      {
        mAppMode = APPLICATION_MODE_PUMPING_MAX;
      }
    }
  }

  /* Check for interlocked */
  if( mpInterlocked->GetValue()==true )
  {
    new_pump_ref.Clear();
    mAppMode = APPLICATION_MODE_INTERLOCKED;
  }

  /* Check for random start up delay */
  if( !mStartUpDelayTimeout )
  {
    new_pump_ref.Clear();
    mAppMode = APPLICATION_MODE_STARTUP_DELAY;
  }

  /* Copy foud pump refs to datapoints */
  for( i=0; i<no_of_pumps; i++ )
  {
    if( (new_pump_ref.GetRef(i) == true) )
    {
      mpOprModeRefPump[i]->SetValue( PUMP_OPERATION_MODE_PUMP_ON );
      if (anti_seiz_pump[i] == true)
      {
        mAppMode = APPLICATION_MODE_ANTISEIZING;
      }
    }
    else
    {
      mpOprModeRefPump[i]->SetValue( PUMP_OPERATION_MODE_PUMP_OFF );
    }
  }

  /* Copy local found appmode to datapoint */
  mpAppMode->SetValue( mAppMode );

  /* Save current run state */
  mPumpRefOld = new_pump_ref;  //det er måske kun nødvendigt at gemme antal kørende pumper, eller endnu mindre kun en bool der siger om noget kørte
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *
 *****************************************************************************/
void PumpCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_PC_GROUP_2_ENABLED:
      mpGroup2Enabled.Attach(pSubject);
      break;
    case SP_PC_GROUP_2_FIRST_PUMP:
      mpGroup2FirstPump.Attach(pSubject);
      break;
    case SP_PC_GROUP_ALTERNATION:
      mpGroupAlternation.Attach(pSubject);
      break;
    case SP_PC_GROUPS_MAY_RUN_TOGETHER:
      mpGroupsMayRunTogether.Attach(pSubject);
      break;
    case SP_PC_MIN_STARTED_PUMPS_TOTAL:
      mpMinStartedPumpsInTotal.Attach(pSubject);
      break;
    case SP_PC_MAX_STARTED_PUMPS_TOTAL:
      mpMaxStartedPumpsInTotal.Attach(pSubject);
      break;
    case SP_PC_GROUP_1_ALTERNATION:
      mpAlternationInGroup[0].Attach(pSubject);
      break;
    case SP_PC_GROUP_2_ALTERNATION:
      mpAlternationInGroup[1].Attach(pSubject);
      break;
    case SP_PC_GROUP_1_MIN_STARTED_PUMPS:
      mpMinStartedPumpsInGroup[0].Attach(pSubject);
      break;
    case SP_PC_GROUP_2_MIN_STARTED_PUMPS:
      mpMinStartedPumpsInGroup[1].Attach(pSubject);
      break;
    case SP_PC_GROUP_1_MAX_STARTED_PUMPS:
      mpMaxStartedPumpsInGroup[0].Attach(pSubject);
      break;
    case SP_PC_GROUP_2_MAX_STARTED_PUMPS:
      mpMaxStartedPumpsInGroup[1].Attach(pSubject);
      break;
    case SP_PC_PUMP_1_REF_LEVEL:
      mpRefLevel[0].Attach(pSubject);
      break;
    case SP_PC_PUMP_2_REF_LEVEL:
      mpRefLevel[1].Attach(pSubject);
      break;
    case SP_PC_PUMP_3_REF_LEVEL:
      mpRefLevel[2].Attach(pSubject);
      break;
    case SP_PC_PUMP_4_REF_LEVEL:
      mpRefLevel[3].Attach(pSubject);
      break;
    case SP_PC_PUMP_5_REF_LEVEL:
      mpRefLevel[4].Attach(pSubject);
      break;
    case SP_PC_PUMP_6_REF_LEVEL:
      mpRefLevel[5].Attach(pSubject);
      break;
    case SP_PC_DAILY_EMPTYING_REQUESTED:
      mpDailyEmptyingRequested.Attach(pSubject);
      break;
    case SP_PC_RUN_FOR_LOWEST_STOP_REF_1:
      mpPumpRunForLowestStop[0].Attach(pSubject);
      break;
    case SP_PC_RUN_FOR_LOWEST_STOP_REF_2:
      mpPumpRunForLowestStop[1].Attach(pSubject);
      break;
    case SP_PC_RUN_FOR_LOWEST_STOP_REF_3:
      mpPumpRunForLowestStop[2].Attach(pSubject);
      break;
    case SP_PC_RUN_FOR_LOWEST_STOP_REF_4:
      mpPumpRunForLowestStop[3].Attach(pSubject);
      break;
    case SP_PC_RUN_FOR_LOWEST_STOP_REF_5:
      mpPumpRunForLowestStop[4].Attach(pSubject);
      break;
    case SP_PC_RUN_FOR_LOWEST_STOP_REF_6:
      mpPumpRunForLowestStop[5].Attach(pSubject);
      break;
    case SP_PC_PUMP_1_LAST_START_TIME:
      mpLastRunTime[0].Attach(pSubject);
      break;
    case SP_PC_PUMP_2_LAST_START_TIME:
      mpLastRunTime[1].Attach(pSubject);
      break;
    case SP_PC_PUMP_3_LAST_START_TIME:
      mpLastRunTime[2].Attach(pSubject);
      break;
    case SP_PC_PUMP_4_LAST_START_TIME:
      mpLastRunTime[3].Attach(pSubject);
      break;
    case SP_PC_PUMP_5_LAST_START_TIME:
      mpLastRunTime[4].Attach(pSubject);
      break;
    case SP_PC_PUMP_6_LAST_START_TIME:
      mpLastRunTime[5].Attach(pSubject);
      break;
    case SP_PC_FOAM_DRAIN_REQUESTED:
      mpFoamDrainRequest.Attach(pSubject);
      break;
    case SP_PC_PUMP_1_READY_FOR_AUTO_OPR:
      mpPumpReady[0].Attach(pSubject);
      break;
    case SP_PC_PUMP_2_READY_FOR_AUTO_OPR:
      mpPumpReady[1].Attach(pSubject);
      break;
    case SP_PC_PUMP_3_READY_FOR_AUTO_OPR:
      mpPumpReady[2].Attach(pSubject);
      break;
    case SP_PC_PUMP_4_READY_FOR_AUTO_OPR:
      mpPumpReady[3].Attach(pSubject);
      break;
    case SP_PC_PUMP_5_READY_FOR_AUTO_OPR:
      mpPumpReady[4].Attach(pSubject);
      break;
    case SP_PC_PUMP_6_READY_FOR_AUTO_OPR:
      mpPumpReady[5].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_REF_PUMP_1:
      mpOprModeRefPump[0].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_REF_PUMP_2:
      mpOprModeRefPump[1].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_REF_PUMP_3:
      mpOprModeRefPump[2].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_REF_PUMP_4:
      mpOprModeRefPump[3].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_REF_PUMP_5:
      mpOprModeRefPump[4].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_REF_PUMP_6:
      mpOprModeRefPump[5].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_ACT_PUMP_1:
      mpOprModeActPump[0].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_ACT_PUMP_2:
      mpOprModeActPump[1].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_ACT_PUMP_3:
      mpOprModeActPump[2].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_ACT_PUMP_4:
      mpOprModeActPump[3].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_ACT_PUMP_5:
      mpOprModeActPump[4].Attach(pSubject);
      break;
    case SP_PC_OPR_MODE_ACT_PUMP_6:
      mpOprModeActPump[5].Attach(pSubject);
      break;
    case SP_PC_INTERLOCKED:
      mpInterlocked.Attach(pSubject);
      break;
    case SP_PC_START_UP_DELAY:
      mpStartUpDelay.Attach(pSubject);
      break;
    case SP_PC_APPLICATION_MODE:
      mpAppMode.Attach(pSubject);
      break;
    case SP_PC_ANTI_SEIZING_REQUEST_PUMP_1:
      mpAntiSeizingRequestPump[0].Attach(pSubject);
      break;
    case SP_PC_ANTI_SEIZING_REQUEST_PUMP_2:
      mpAntiSeizingRequestPump[1].Attach(pSubject);
      break;
    case SP_PC_ANTI_SEIZING_REQUEST_PUMP_3:
      mpAntiSeizingRequestPump[2].Attach(pSubject);
      break;
    case SP_PC_ANTI_SEIZING_REQUEST_PUMP_4:
      mpAntiSeizingRequestPump[3].Attach(pSubject);
      break;
    case SP_PC_ANTI_SEIZING_REQUEST_PUMP_5:
      mpAntiSeizingRequestPump[4].Attach(pSubject);
      break;
    case SP_PC_ANTI_SEIZING_REQUEST_PUMP_6:
      mpAntiSeizingRequestPump[5].Attach(pSubject);
      break;
    case SP_PC_NO_OF_PUMPS:
      mpNoOfPumps.Attach(pSubject);
      break;
    case SP_PC_ADV_FLOW_TEMP_STOP_REQUEST:
      mpAdvFlowTempStopRequest.Attach(pSubject);
      break;
    case SP_PC_HIGH_LEVEL_STATE_DETECTED :
      mpHighLevelStateDetected.Attach(pSubject);
      break;
    case SP_PC_NO_OF_AVAILABLE_PUMPS:
      mpNoOfAvailablePumps.Attach(pSubject);
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
void PumpCtrl::Update(Subject* pSubject)
{
  if (pSubject == mpTimerObjList[START_UP_DELAY_TIMER])
  {
    mStartUpDelayTimeout = true;
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
void PumpCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpCtrl::ConnectToSubjects()
{
  int i;

  mpGroup2Enabled->Subscribe(this);
  mpGroup2FirstPump->Subscribe(this);
  mpDailyEmptyingRequested->Subscribe(this);
  mpFoamDrainRequest->Subscribe(this);
  mpGroupAlternation->Subscribe(this);
  mpMinStartedPumpsInTotal->Subscribe(this);
  mpMaxStartedPumpsInTotal->Subscribe(this);
  mpInterlocked->Subscribe(this);
  mpGroupsMayRunTogether->Subscribe(this);
  mpNoOfPumps->Subscribe(this);
  mpAdvFlowTempStopRequest->Subscribe(this);
  mpHighLevelStateDetected->Subscribe(this);

  for(i=0; i<MAX_NO_OF_GROUPS; i++)
  {
    mpAlternationInGroup[i]->Subscribe(this);
    mpMinStartedPumpsInGroup[i]->Subscribe(this);
    mpMaxStartedPumpsInGroup[i]->Subscribe(this);
  }

  for(i=0; i<MAX_NO_OF_PUMPS; i++)
  {
    mpRefLevel[i]->Subscribe(this);
    mpPumpReady[i]->Subscribe(this);
    mpAntiSeizingRequestPump[i]->Subscribe(this);
    mpOprModeActPump[i]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpCtrl::InitSubTask()
{
  int delay;

  /* Handle StartUpDelay */
  mStartUpDelayTimeout = true;
  delay = (rand()*mpStartUpDelay->GetValue()*1000)/0x7FFF;
  if(delay>10) //if delay < 10ms, do not delay startup
  {
    mStartUpDelayTimeout = false;
    mpTimerObjList[START_UP_DELAY_TIMER]->SetSwTimerPeriod(delay, MS, false);
    mpTimerObjList[START_UP_DELAY_TIMER]->RetriggerSwTimer();
  }
  mpOprModeRefPump[PUMP_1]->SetValue(PUMP_OPERATION_MODE_PUMP_OFF);
  mpOprModeRefPump[PUMP_2]->SetValue(PUMP_OPERATION_MODE_PUMP_OFF);
  mpOprModeRefPump[PUMP_3]->SetValue(PUMP_OPERATION_MODE_PUMP_OFF);
  mpOprModeRefPump[PUMP_4]->SetValue(PUMP_OPERATION_MODE_PUMP_OFF);
  mpOprModeRefPump[PUMP_5]->SetValue(PUMP_OPERATION_MODE_PUMP_OFF);
  mpOprModeRefPump[PUMP_6]->SetValue(PUMP_OPERATION_MODE_PUMP_OFF);

  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - GetPumpByTime
 * DESCRIPTION:
 *****************************************************************************/
int PumpCtrl::GetPumpByTime(int index)
{
  int i,j;
  U32 temp_time;
  int temp_index;

  int mTimeIndex[MAX_NO_OF_PUMPS];
  U32 mLastRunTime[MAX_NO_OF_PUMPS];

  int no_of_pumps = mpNoOfPumps->GetValue();

  for (j=0; j<no_of_pumps; j++)
  {
    mTimeIndex[j] = j;
    mLastRunTime[j] = mpLastRunTime[j]->GetValue();
  }
  for (j=0; j<no_of_pumps-1; j++)
  {
    for (i=j+1; i<no_of_pumps; i++)
    {
      if (mLastRunTime[i]<mLastRunTime[j])
      {
        temp_time = mLastRunTime[j];
        temp_index = mTimeIndex[j];
        mLastRunTime[j] = mLastRunTime[i];
        mLastRunTime[i] = temp_time;
        mTimeIndex[j] = mTimeIndex[i];
        mTimeIndex[i] = temp_index;
      }
    }
  }
  return mTimeIndex[index];
}

/*****************************************************************************
 * Function - CheckTempStop
 * DESCRIPTION:
 *****************************************************************************/
TEMP_STOP_TYPE PumpCtrl::CheckTempStop(int no_of_pumps_wish, int manual_started, U8 *tempStopPump)
{
  TEMP_STOP_TYPE temp_stop_handling = TEMP_STOP_IGNORE;

  if (no_of_pumps_wish > 1 || manual_started > 0 || mpHighLevelStateDetected->GetValue() == true)
  {
    // Invalid situation for temp stop
    // Ensure that no pump stop can be temporary stopped before all pumps has been stopped.
    mTempStopAllowed = false;
    mPumpInTempStop = NO_OF_PUMPS;
  }
  else if (no_of_pumps_wish == 0)
  {
    mTempStopAllowed = true; // OK again
  }

  if (mpAdvFlowTempStopRequest->GetQuality() == DP_AVAILABLE)
  {
    // A temp stop/resume request is present
    if (mTempStopAllowed == true && no_of_pumps_wish == 1)
    {
      U8 temp_stop_pump = mpAdvFlowTempStopRequest->GetValue();
      if (temp_stop_pump < NO_OF_PUMPS) // This is a temp stop request
      {
        if (mpPumpReady[temp_stop_pump]->GetValue() == true)
        {
          mPumpInTempStop = temp_stop_pump;
          *tempStopPump = mPumpInTempStop;
          temp_stop_handling = TEMP_STOP_STOP;
        }
      }
      else if (mPumpInTempStop < NO_OF_PUMPS) // Resume after temp stop
      {
        if (mpPumpReady[mPumpInTempStop]->GetValue() == true)
        {
          *tempStopPump = mPumpInTempStop;
          temp_stop_handling = TEMP_STOP_RESUME;
        }
        mPumpInTempStop = NO_OF_PUMPS;
      }
    }
  }
  return temp_stop_handling;
}

/*****************************************************************************
 *
 *
 *              Class PumpRef
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
PumpRef::PumpRef( )
{
  int i;

  SetNoOfPumps( MAX_NO_OF_PUMPS );
  for( i=0;i<MAX_NO_OF_PUMPS;i++ )
  {
    mPumpStart[i] = false;
  }
}

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *****************************************************************************/
PumpRef::PumpRef( int noOfPumps )
{
  int i;

  SetNoOfPumps( noOfPumps );
  for( i=0;i<MAX_NO_OF_PUMPS;i++ )
  {
    mPumpStart[i] = false;
  }
}

/*****************************************************************************
 * Function - Denstructor
 * DESCRIPTION:
 *****************************************************************************/
PumpRef::~PumpRef()
{
  ;
}

/*****************************************************************************
 * Function - SetNoOfPumps
 * DESCRIPTION:
 *****************************************************************************/
 void PumpRef::SetNoOfPumps(int noOfPumps)
 {
  if(noOfPumps<=MAX_NO_OF_PUMPS)
  {
    mNoOfPumps = noOfPumps;
  }
  else
  {
    mNoOfPumps = MAX_NO_OF_PUMPS;
  }
 }

/*****************************************************************************
 * Function - SetRef
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpRef::SetRef(int number, bool state)
{
  if( number<MAX_NO_OF_PUMPS )
  {
    mPumpStart[number] = state;
  }
}


/*****************************************************************************
 * Function - ClearRef
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpRef::ClearRef(int number)
{
  if( number<MAX_NO_OF_PUMPS )
  {
    mPumpStart[number] = false;
  }
}

/*****************************************************************************
 * Function - Clear
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpRef::Clear( void )
{
  int i;

  for( i=0;i<MAX_NO_OF_PUMPS;i++ )
  {
    mPumpStart[i] = false;
  }
}

/*****************************************************************************
 * Function - GetRef
 * DESCRIPTION:
 *
 *****************************************************************************/
bool PumpRef::GetRef(int number)
{
  bool ret_val = false;

  if( number<MAX_NO_OF_PUMPS )
  {
    ret_val = mPumpStart[number];
  }
  return ret_val;
}


/*****************************************************************************
 * Function - NoOfPumps
 * DESCRIPTION:
 *
 *****************************************************************************/
int PumpRef::NoOfPumps( void )
{
  int i;
  int cnt = 0;

  for( i=0;i<mNoOfPumps;i++ )
  {
    if( mPumpStart[i] == true )
    {
      cnt++;
    }
  }
  return cnt;
}

/*****************************************************************************
 * Function - ShiftRefs
 * DESCRIPTION:
 *
 *****************************************************************************/
void PumpRef::ShiftRefs( int shift )
{
  PumpRef temp_ref;

  unsigned int i;

  temp_ref = *this;
  for( i=0;i<mNoOfPumps;i++ )
  {
    mPumpStart[i] = temp_ref.GetRef((i-shift)%mNoOfPumps);
  }
}

/*****************************************************************************
* Function - Assignment operator
* DESCRIPTION: - param src The value to assign to this object.
*                return A reference to this object.
*
****************************************************************************/
PumpRef& PumpRef::operator=(const PumpRef& src)
{
  int i;

  if (this != &src)
  {
    for( i=0;i<MAX_NO_OF_PUMPS;i++ )
    {
      mPumpStart[i] = src.mPumpStart[i];
    }
    mNoOfPumps = src.mNoOfPumps;
  }
  return *this;
}// =


