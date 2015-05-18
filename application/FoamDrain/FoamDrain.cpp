/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MidRagne                                      */
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
/* CLASS NAME       : FoamDrain                                             */
/*                                                                          */
/* FILE NAME        : FoamDrain.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 30-06-2009  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <FoamDrain.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  LOCAL CONST VARIABLES
 *****************************************************************************/

/*****************************************************************************
  C functions declarations
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
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
FoamDrain::FoamDrain(void)
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
FoamDrain::~FoamDrain(void)
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FoamDrain::InitSubTask(void)
{
  mpFoamDrainIntervalTimer = new SwTimer(mpFoamDrainIntervalTime->GetValue(), S, true, true, this);
  mpFoamDrainTimer = new SwTimer(mpFoamDrainTime->GetValue(), S, false, false, this);

  mFoamDrainTimeOut = false;
  mFoamDrainIntervalTimeOut = false;
  mFoamDrainState = FD_IDLE;
  mpFoamDrainRequest->SetValue(false);

  mReqTaskTimeFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void FoamDrain::RunSubTask(void)
{
  mReqTaskTimeFlag = false;

  if (mpPitLevelCtrlType->GetValue() == SENSOR_TYPE_FLOAT_SWITCHES)
  {
    return;
  }

  // Do not attempt to control before the surface level has been initiated
  if (mpSurfaceLevelReady->GetValue())
  {
    if (mpFoamDrainIntervalTime.IsUpdated())
    {
      mpFoamDrainIntervalTimer->SetSwTimerPeriod(mpFoamDrainIntervalTime->GetValue(), S, true);
    }
    if (mpFoamDrainTime.IsUpdated())
    {
      mpFoamDrainTimer->SetSwTimerPeriod(mpFoamDrainTime->GetValue(), S, false);
    }


    if (mpFoamDrainEnabled->GetValue() &&
        mpUnderDryRunLevel->GetQuality() == DP_AVAILABLE) //LevelCtrl sets quality to DP_NOT_AVAILABLE if there is a problem with the analogue level sensor or there are conflicting levels
    {
      if (mpFoamDrainDigInRequest.IsUpdated() == true && mpFoamDrainDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
      {
        // Digital input trigger should give same result as foam drain time out
        mFoamDrainIntervalTimeOut = true;
      }

      switch (mFoamDrainState)
      {
        case FD_IDLE:
          mpFoamDrainTimer->StopSwTimer();
          if (mFoamDrainIntervalTimeOut)
          {
            mFoamDrainIntervalTimeOut = false;
            mFoamDrainState = FD_WAIT_FOR_PUMP_START;
          }
          else
          {
            break;
          }
          // Possibly fall through

        case FD_WAIT_FOR_PUMP_START:
        {
          bool found_running_pump = false;
          GF_UINT8 no_of_pumps = mpNoOfPumps->GetValue();
          for (unsigned int i = 0; i < no_of_pumps; i++)
          {
            if (mpPumpRefLevel[i]->GetValue())
            {
              found_running_pump = true;
              break;
            }
          }

          if (found_running_pump)
          {
            mFoamDrainState = FD_WAIT_FOR_FOAM_DRAIN_LEVEL;
          }
          else
          {
            break;
          }
        }
        // Possibly fall through

        case FD_WAIT_FOR_FOAM_DRAIN_LEVEL:
          if (mpSurfaceLevel->GetValue() < mpFoamDrainLevel->GetValue())
          {
            mFoamDrainState = FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT;
          }
          else
          {
            break;
          }
          // Possibly fall through

        case FD_PRE_WAIT_FOR_FOAM_DRAIN_TIMEOUT:
          mFoamDrainState = FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT;
          if (mpFoamDrainTime->GetValue() > 0)
          {
            mFoamDrainTimeOut = false;
            mpFoamDrainTimer->RetriggerSwTimer();
          }
          else
          {
            mFoamDrainTimeOut = true;
          }
          // Fall through

        case FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT:
          if (mFoamDrainTimeOut)
          {
            mFoamDrainState = FD_IDLE;
            if (mpFoamDrainDigInRequest->GetValue() == DIGITAL_INPUT_FUNC_STATE_ACTIVE)
            {
              // Keep triggering foam draining when input is activated
              mFoamDrainIntervalTimeOut = true;
            }
          }
          break;

        default:
          FatalErrorOccured("mFoamDrainState has unknown value!");
          break;
      } // switch (mFoamDrainState)
    } // if (mpFoamDrainEnabled->GetValue() && mpUnderDryRunLevel->GetQuality() == DP_AVAILABLE)
    else
    {
      // Foam drain disabled or sensor error - do not perform a foam drain and/or abort an ongoing foam drain

      mpFoamDrainTimer->StopSwTimer();
      mFoamDrainTimeOut = false;
      mFoamDrainState = FD_IDLE;
      if (!mpFoamDrainEnabled->GetValue())
      {
        // Force a foam drain first time after it has been enabled
        mFoamDrainIntervalTimeOut = true;
      }
    }

    mpFoamDrainRequest->SetValue(mFoamDrainState == FD_WAIT_FOR_FOAM_DRAIN_LEVEL || mFoamDrainState == FD_WAIT_FOR_FOAM_DRAIN_TIMEOUT);
  } // if (mpSurfaceLevelReadyFlag->GetValue() == true && mpSurfaceLevel->GetQuality() == DP_AVAILABLE)
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void FoamDrain::Update(Subject* pSubject)
{
  if (mpSurfaceLevel.Update(pSubject))
  {
  }
  else if (pSubject == mpFoamDrainIntervalTimer)
  {
    mFoamDrainIntervalTimeOut = true;
  }
  else if (pSubject == mpFoamDrainTimer)
  {
    mFoamDrainTimeOut = true;
  }
  else if (mpUnderDryRunLevel.Update(pSubject))
  {
  }
  else if (mpFoamDrainDigInRequest.Update(pSubject))
  {
  }
  else if (mpFoamDrainLevel.Update(pSubject))
  {
  }
  else if (mpFoamDrainTime.Update(pSubject))
  {
  }
  else if (mpFoamDrainIntervalTime.Update(pSubject))
  {
  }
  else if (mpFoamDrainEnabled.Update(pSubject))
  {
  }
  else if (mpPitLevelCtrlType.Update(pSubject))
  {
  }
  else if (mpSurfaceLevelReady.Update(pSubject))
  {
  }
  else if (mpNoOfPumps.Update(pSubject))
  {
  }
  else if (mpPumpRefLevel[PUMP_1].Update(pSubject))
  {
  }
  else if (mpPumpRefLevel[PUMP_2].Update(pSubject))
  {
  }
  else if (mpPumpRefLevel[PUMP_3].Update(pSubject))
  {
  }
  else if (mpPumpRefLevel[PUMP_4].Update(pSubject))
  {
  }
  else if (mpPumpRefLevel[PUMP_5].Update(pSubject))
  {
  }
  else if (mpPumpRefLevel[PUMP_6].Update(pSubject))
  {
  }
  else
  {
    // Update from unknown source - this must not happen
    FatalErrorOccured("FoamDrain updated by unknown subject!");
  }

  if (!mReqTaskTimeFlag)
  {
    mReqTaskTimeFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void FoamDrain::SubscribtionCancelled(Subject* pSubject)
{
  mpPitLevelCtrlType.Detach(pSubject);
  mpSurfaceLevelReady.Detach(pSubject);
  mpSurfaceLevel.Detach(pSubject);
  mpNoOfPumps.Detach(pSubject);
  mpUnderDryRunLevel.Detach(pSubject);

  mpFoamDrainLevel.Detach(pSubject);
  mpFoamDrainTime.Detach(pSubject);
  mpFoamDrainIntervalTime.Detach(pSubject);
  mpFoamDrainEnabled.Detach(pSubject);
  mpFoamDrainDigInRequest.Detach(pSubject);

  for (unsigned int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpPumpRefLevel[i].Detach(pSubject);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void FoamDrain::ConnectToSubjects(void)
{
  mpPitLevelCtrlType->Subscribe(this);
  mpSurfaceLevelReady->Subscribe(this);
  mpSurfaceLevel->Subscribe(this);
  mpNoOfPumps->Subscribe(this);
  mpUnderDryRunLevel->Subscribe(this);

  mpFoamDrainLevel->Subscribe(this);
  mpFoamDrainTime->Subscribe(this);
  mpFoamDrainIntervalTime->Subscribe(this);
  mpFoamDrainEnabled->Subscribe(this);
  mpFoamDrainDigInRequest->Subscribe(this);

  for (unsigned int i = 0; i < MAX_NO_OF_PUMPS; i++)
  {
    mpPumpRefLevel[i]->Subscribe(this);
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void FoamDrain::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    case SP_FD_PIT_LEVEL_CTRL_TYPE:       mpPitLevelCtrlType      .Attach(pSubject); break;
    case SP_FD_SURFACE_LEVEL_READY:       mpSurfaceLevelReady     .Attach(pSubject); break;
    case SP_FD_SURFACE_LEVEL:             mpSurfaceLevel          .Attach(pSubject); break;
    case SP_FD_NO_OF_PUMPS:               mpNoOfPumps             .Attach(pSubject); break;
    case SP_FD_UNDER_DRY_RUN_LEVEL:       mpUnderDryRunLevel      .Attach(pSubject); break;
    case SP_FD_PUMP_REF_LEVEL_1:          mpPumpRefLevel[PUMP_1]  .Attach(pSubject); break;
    case SP_FD_PUMP_REF_LEVEL_2:          mpPumpRefLevel[PUMP_2]  .Attach(pSubject); break;
    case SP_FD_PUMP_REF_LEVEL_3:          mpPumpRefLevel[PUMP_3]  .Attach(pSubject); break;
    case SP_FD_PUMP_REF_LEVEL_4:          mpPumpRefLevel[PUMP_4]  .Attach(pSubject); break;
    case SP_FD_PUMP_REF_LEVEL_5:          mpPumpRefLevel[PUMP_5]  .Attach(pSubject); break;
    case SP_FD_PUMP_REF_LEVEL_6:          mpPumpRefLevel[PUMP_6]  .Attach(pSubject); break;

    case SP_FD_FOAM_DRAIN_LEVEL:          mpFoamDrainLevel        .Attach(pSubject); break;
    case SP_FD_FOAM_DRAIN_TIME:           mpFoamDrainTime         .Attach(pSubject); break;
    case SP_FD_FOAM_DRAIN_INTERVAL_TIME:  mpFoamDrainIntervalTime .Attach(pSubject); break;
    case SP_FD_FOAM_DRAIN_ENABLED:        mpFoamDrainEnabled      .Attach(pSubject); break;
    case SP_FD_FOAM_DRAIN_DIG_IN_REQUEST: mpFoamDrainDigInRequest .Attach(pSubject); break;
    case SP_FD_FOAM_DRAIN_REQUEST:        mpFoamDrainRequest      .Attach(pSubject); break;
  }
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
*****************************************************************************/

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                              - RARE USED -
 *
 ****************************************************************************/

