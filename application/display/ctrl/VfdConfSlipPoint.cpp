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
/* CLASS NAME       : VfdConfSlipPoint                                      */
/*                                                                          */
/* FILE NAME        : VfdConfSlipPoint.cpp                                  */
/*                                                                          */
/* CREATED DATE     : 07-04-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the VFD configuration DataPoints into one                */
/* set of virtual DataPoints for the display to look at...                  */
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
#include "VfdConfSlipPoint.h"

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DISPLAY_VFD_SETTINGS_ID 138
#define DISPLAY_VFD_FLUSH_SETTINGS_ID 145
#define DISPLAY_VFD_PID_SETTINGS_ID 146
#define DISPLAY_VFD_PID_FEEDBACK_ID 147
#define DISPLAY_VFD_PID_SETPOINT_ID 148

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
VfdConfSlipPoint::VfdConfSlipPoint()
{
  mCurrentlyUpdating = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
VfdConfSlipPoint::~VfdConfSlipPoint()
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void VfdConfSlipPoint::InitSubTask(void)
{
  if (mpCurrentVfdNumber.IsValid())
  {
    mpCurrentVfdNumber->SetAsInt(0);
  }

  UpdateVirtualVfdConf();
  UpdateUpperStatusLine();
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void VfdConfSlipPoint::RunSubTask(void)
{
  mCurrentlyUpdating = true;  // Guard the SubTask

  if (mpCurrentVfdNumber.IsUpdated() )
  {
    UpdateVirtualVfdConf();
    //TEST JSM UpdateVirtualFixedSetpoint(false);
    UpdateUpperStatusLine();
  }

  if ( mpVirtualFixedFreq.IsUpdated()
    || mpVirtualEcoFreq.IsUpdated()
    || mpVirtualEcoLevel.IsUpdated()
    || mpVirtualEcoMaxLevel.IsUpdated()
    || mpVirtualRunMode.IsUpdated()
    || mpVirtualReactionMode.IsUpdated()
    || mpVirtualReverseStartEnabled.IsUpdated()
    || mpVirtualReverseTime.IsUpdated()
    || mpVirtualReverseInterval.IsUpdated()
    || mpVirtualStartFlushEnabled.IsUpdated()
    || mpVirtualStartFlushTime.IsUpdated()
    || mpVirtualRunFlushEnabled.IsUpdated()
    || mpVirtualRunFlushTime.IsUpdated()
    || mpVirtualRunFlushInterval.IsUpdated()
    || mpVirtualStopFlushEnabled.IsUpdated()
    || mpVirtualStopFlushTime.IsUpdated()
    || mpVirtualCueEnabled.IsUpdated()
    || mpVirtualCueMinFreq.IsUpdated()
    || mpVirtualCueMaxFreq.IsUpdated()
    || mpVirtualEcoMinFrequency.IsUpdated()
    || mpVirtualFrequencyLearnEnabled.IsUpdated()
    || mpVirtualPidType.IsUpdated()
    || mpVirtualPidKp.IsUpdated()
    || mpVirtualPidTi.IsUpdated()
    || mpVirtualPidTd.IsUpdated()
    || mpVirtualPidInverseControl.IsUpdated()
    || mpVirtualPidSetpointType.IsUpdated()
    || mpVirtualPidSetpointAi.IsUpdated()
    || mpVirtualPidSetpointFixed.IsUpdated()
    || mpVirtualMinVelocityEnabled.IsUpdated()
    || mpVirtualMinVelocity.IsUpdated()
    || mpVirtualPipeDiameter.IsUpdated()
   )
  {
    UpdateCurrentVfdConf();
  }
  else if (mpVirtualPidFeedback.IsUpdated())
  {
    //TEST JSM UpdateVirtualFixedSetpoint(true);
    UpdateCurrentVfdConf();
  }

  {
    const int vfd_no = mpCurrentVfdNumber->GetAsInt();
	int vfd_check = 0;

    if ((vfd_no >= 0) && (vfd_no < NO_OF_PUMPS))
    {
      if (mpFixedFreq[vfd_no].IsUpdated()
        || mpEcoFreq[vfd_no].IsUpdated()
        || mpEcoLevel[vfd_no].IsUpdated()
        || mpEcoMaxLevel[vfd_no].IsUpdated()
        || mpRunMode[vfd_no].IsUpdated()
        || mpReactionMode[vfd_no].IsUpdated()
        || mpReverseStartEnabled[vfd_no].IsUpdated()
        || mpReverseTime[vfd_no].IsUpdated()
        || mpReverseInterval[vfd_no].IsUpdated()
        || mpStartFlushEnabled[vfd_no].IsUpdated()
        || mpStartFlushTime[vfd_no].IsUpdated()
        || mpRunFlushEnabled[vfd_no].IsUpdated()
        || mpRunFlushTime[vfd_no].IsUpdated()
        || mpRunFlushInterval[vfd_no].IsUpdated()
        || mpStopFlushEnabled[vfd_no].IsUpdated()
        || mpStopFlushTime[vfd_no].IsUpdated()
        || mpCueEnabled[vfd_no].IsUpdated()
        || mpCueMinFreq[vfd_no].IsUpdated()
        || mpCueMaxFreq[vfd_no].IsUpdated()
        || mpEcoMinFrequency[vfd_no].IsUpdated()
        || mpFrequencyLearnEnabled[vfd_no].IsUpdated()
        || mpPidType[vfd_no].IsUpdated()
        || mpPidKp[vfd_no].IsUpdated()
        || mpPidTi[vfd_no].IsUpdated()
        || mpPidTd[vfd_no].IsUpdated()
        || mpPidInverseControl[vfd_no].IsUpdated()
        || mpPidFeedback[vfd_no].IsUpdated()
        || mpPidSetpointType[vfd_no].IsUpdated()
        || mpPidSetpointAi[vfd_no].IsUpdated()
        || mpPidSetpointFixed[vfd_no].IsUpdated()
        || mpMinVelocityEnabled[vfd_no].IsUpdated()
        || mpMinVelocity[vfd_no].IsUpdated()
        || mpPipeDiameter[vfd_no].IsUpdated())
      {
        UpdateVirtualVfdConf();
		vfd_check = 1;
      }
    }
    else
    {
      FatalErrorOccured("VCSP index out of range!");
	  vfd_check =1;
    }
	if(vfd_check == 0)
	{
	  for (int vfd_number = 0; vfd_number < NO_OF_PUMPS; vfd_number++)
      {
		  if(mpVfdInstalled[vfd_number]->GetValue() == true)
		  {
			     if(vfd_number != vfd_no)
				 {
						if (mpFixedFreq[vfd_number].IsUpdated()
						|| mpRunMode[vfd_number].IsUpdated()
						|| mpPidFeedback[vfd_number].IsUpdated()
						|| mpPidSetpointType[vfd_number].IsUpdated()
						|| mpPidSetpointAi[vfd_number].IsUpdated()
						|| mpPidSetpointFixed[vfd_number].IsUpdated() )
						{
								int temp_vfd_no = vfd_no;
								mpCurrentVfdNumber->SetAsInt(vfd_number);
								UpdateVirtualVfdConf();
								mpCurrentVfdNumber->SetAsInt(temp_vfd_no);
								break;

						}
				 }

		  }
		  else
		  {			  
			  if(mpPidFeedback[vfd_number].IsUpdated())
			  {				  
				  mpPidFeedback[vfd_number]->SetValue(MEASURED_VALUE_FLOW);
				  break;				  
			  }
			  if(mpPidSetpointAi[vfd_number].IsUpdated())
			  {
				 mpPidSetpointAi[vfd_number]->SetValue(MEASURED_VALUE_USER_DEFINED_SOURCE_1);
				 break;

			  }
		  }

	  }
	}
  }

  mCurrentlyUpdating = false; // End of: Guard the SubTask
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void VfdConfSlipPoint::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void VfdConfSlipPoint::Update(Subject* pSubject)
{
  if ( !mCurrentlyUpdating )
  {
    if (mpCurrentVfdNumber.Update(pSubject)){}
    else if (mpVirtualFixedFreq.Update(pSubject)){}
    else if (mpVirtualEcoFreq.Update(pSubject)){}
    else if (mpVirtualEcoLevel.Update(pSubject)){}
    else if (mpVirtualEcoMaxLevel.Update(pSubject)){}
    else if (mpVirtualRunMode.Update(pSubject)){}
    else if (mpVirtualReactionMode.Update(pSubject)){}
    else if (mpVirtualReverseStartEnabled.Update(pSubject)){}
    else if (mpVirtualReverseTime.Update(pSubject)){}
    else if (mpVirtualReverseInterval.Update(pSubject)){}
    else if (mpVirtualStartFlushEnabled.Update(pSubject)){}
    else if (mpVirtualStartFlushTime.Update(pSubject)){}
    else if (mpVirtualRunFlushEnabled.Update(pSubject)){}
    else if (mpVirtualRunFlushTime.Update(pSubject)){}
    else if (mpVirtualRunFlushInterval.Update(pSubject)){}
    else if (mpVirtualStopFlushEnabled.Update(pSubject)){}
    else if (mpVirtualStopFlushTime.Update(pSubject)){}
    else if (mpVirtualCueEnabled.Update(pSubject)){}
    else if (mpVirtualCueMinFreq.Update(pSubject)){}
    else if (mpVirtualCueMaxFreq.Update(pSubject)){}
    else if (mpVirtualEcoMinFrequency.Update(pSubject)){}
    else if (mpVirtualFrequencyLearnEnabled.Update(pSubject)){}
    else if (mpVirtualPidType.Update(pSubject)){}
    else if (mpVirtualPidKp.Update(pSubject)){}
    else if (mpVirtualPidTi.Update(pSubject)){}
    else if (mpVirtualPidTd.Update(pSubject)){}
    else if (mpVirtualPidInverseControl.Update(pSubject)){}
    else if (mpVirtualPidFeedback.Update(pSubject)){}
    else if (mpVirtualPidSetpointType.Update(pSubject)){}
    else if (mpVirtualPidSetpointAi.Update(pSubject)){}
    else if (mpVirtualPidSetpointFixed.Update(pSubject)){}
    else if (mpVirtualMinVelocityEnabled.Update(pSubject)){}
    else if (mpVirtualMinVelocity.Update(pSubject)){}
    else if (mpVirtualPipeDiameter.Update(pSubject)){}
    else
    {
      for (int vfd_no = 0; vfd_no < NO_OF_PUMPS; vfd_no++)
      {
        if(mpFixedFreq[vfd_no].Update(pSubject)){break;}
        else if(mpEcoFreq[vfd_no].Update(pSubject)){break;}
        else if(mpEcoLevel[vfd_no].Update(pSubject)){break;}
        else if(mpEcoMaxLevel[vfd_no].Update(pSubject)){break;}
        else if(mpRunMode[vfd_no].Update(pSubject)){break;}
        else if(mpReactionMode[vfd_no].Update(pSubject)){break;}
        else if(mpReverseStartEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpReverseTime[vfd_no].Update(pSubject)){break;}
        else if(mpReverseInterval[vfd_no].Update(pSubject)){break;}
        else if(mpStartFlushEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpStartFlushTime[vfd_no].Update(pSubject)){break;}
        else if(mpRunFlushEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpRunFlushTime[vfd_no].Update(pSubject)){break;}
        else if(mpRunFlushInterval[vfd_no].Update(pSubject)){break;}
        else if(mpStopFlushEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpStopFlushTime[vfd_no].Update(pSubject)){break;}
        else if(mpCueEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpCueMinFreq[vfd_no].Update(pSubject)){break;}
        else if(mpCueMaxFreq[vfd_no].Update(pSubject)){break;}
        else if(mpEcoMinFrequency[vfd_no].Update(pSubject)){break;}
        else if(mpFrequencyLearnEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpPidType[vfd_no].Update(pSubject)){break;}
        else if(mpPidKp[vfd_no].Update(pSubject)){break;}
        else if(mpPidTi[vfd_no].Update(pSubject)){break;}
        else if(mpPidTd[vfd_no].Update(pSubject)){break;}
        else if(mpPidInverseControl[vfd_no].Update(pSubject)){break;}
        else if(mpPidFeedback[vfd_no].Update(pSubject)){break;}
        else if(mpPidSetpointType[vfd_no].Update(pSubject)){break;}
        else if(mpPidSetpointAi[vfd_no].Update(pSubject)){break;}
        else if(mpPidSetpointFixed[vfd_no].Update(pSubject)){break;}
        else if(mpMinVelocityEnabled[vfd_no].Update(pSubject)){break;}
        else if(mpMinVelocity[vfd_no].Update(pSubject)){break;}
        else if(mpPipeDiameter[vfd_no].Update(pSubject)){break;}
        
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
void VfdConfSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch ( Id )
  {
  case SP_VCSP_CURRENT_NO:
    mpCurrentVfdNumber.Attach(pSubject);
    break;

  case SP_VCSP_VIRTUAL_FIXED_FREQ :
    mpVirtualFixedFreq.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_ECO_FREQ :
    mpVirtualEcoFreq.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_ECO_LEVEL :
    mpVirtualEcoLevel.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_ECO_MAX_LEVEL :
    mpVirtualEcoMaxLevel.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_RUN_MODE :
    mpVirtualRunMode.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_REACTION_MODE :
    mpVirtualReactionMode.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_REVERSE_START_ENABLED :
    mpVirtualReverseStartEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_REVERSE_TIME :
    mpVirtualReverseTime.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_REVERSE_INTERVAL :
    mpVirtualReverseInterval.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_START_FLUSH_ENABLED :
    mpVirtualStartFlushEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_START_FLUSH_TIME :
    mpVirtualStartFlushTime.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_RUN_FLUSH_ENABLED :
    mpVirtualRunFlushEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_RUN_FLUSH_TIME :
    mpVirtualRunFlushTime.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_RUN_FLUSH_INTERVAL :
    mpVirtualRunFlushInterval.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_STOP_FLUSH_ENABLED :
    mpVirtualStopFlushEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_STOP_FLUSH_TIME :
    mpVirtualStopFlushTime.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_CUE_ENABLED :
    mpVirtualCueEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_CUE_MIN_FREQ :
    mpVirtualCueMinFreq.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_CUE_MAX_FREQ :
    mpVirtualCueMaxFreq.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_ECO_MIN_FREQUENCY:
    mpVirtualEcoMinFrequency.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_FREQUENCY_LEARN_ENABLED:
    mpVirtualFrequencyLearnEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_TYPE:
    mpVirtualPidType.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_KP:
    mpVirtualPidKp.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_TI:
    mpVirtualPidTi.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_TD:
    mpVirtualPidTd.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_INVERSE_CONTROL:
    mpVirtualPidInverseControl.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_FEEDBACK:
    mpVirtualPidFeedback.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_SETPOINT_TYPE:
    mpVirtualPidSetpointType.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_SETPOINT_AI:
    mpVirtualPidSetpointAi.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PID_SETPOINT_FIXED:
    mpVirtualPidSetpointFixed.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_MIN_VELOCITY_ENABLED:
    mpVirtualMinVelocityEnabled.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_MIN_VELOCITY:
    mpVirtualMinVelocity.Attach(pSubject);
    break;
  case SP_VCSP_VIRTUAL_PIPE_DIAMETER:
    mpVirtualPipeDiameter.Attach(pSubject);
    break;

  case SP_VCSP_FIXED_FREQ_PUMP_1 :
    mpFixedFreq[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_ECO_FREQ_PUMP_1 :
    mpEcoFreq[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_ECO_LEVEL_PUMP_1 :
    mpEcoLevel[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MAX_LEVEL_PUMP_1 :
    mpEcoMaxLevel[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_RUN_MODE_PUMP_1 :
    mpRunMode[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_REACTION_MODE_PUMP_1 :
    mpReactionMode[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_START_ENABLED_PUMP_1 :
    mpReverseStartEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_TIME_PUMP_1 :
    mpReverseTime[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_INTERVAL_PUMP_1 :
    mpReverseInterval[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_ENABLED_PUMP_1 :
    mpStartFlushEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_TIME_PUMP_1 :
    mpStartFlushTime[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_ENABLED_PUMP_1 :
    mpRunFlushEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_TIME_PUMP_1 :
    mpRunFlushTime[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_INTERVAL_PUMP_1 :
    mpRunFlushInterval[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_ENABLED_PUMP_1 :
    mpStopFlushEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_TIME_PUMP_1 :
    mpStopFlushTime[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_CUE_ENABLED_PUMP_1 :
    mpCueEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MIN_FREQ_PUMP_1 :
    mpCueMinFreq[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MAX_FREQ_PUMP_1 :
    mpCueMaxFreq[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MIN_FREQUENCY_PUMP_1 :
    mpEcoMinFrequency[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_FREQUENCY_LEARN_ENABLED_PUMP_1 :
    mpFrequencyLearnEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_TYPE_PUMP_1 :
    mpPidType[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_KP_PUMP_1 :
    mpPidKp[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_TI_PUMP_1 :
    mpPidTi[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_TD_PUMP_1 :
    mpPidTd[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_INVERSE_CONTROL_PUMP_1 :
    mpPidInverseControl[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_FEEDBACK_PUMP_1 :
    mpPidFeedback[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_TYPE_PUMP_1 :
    mpPidSetpointType[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_AI_PUMP_1 :
    mpPidSetpointAi[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_FIXED_PUMP_1 :
    mpPidSetpointFixed[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_ENABLED_1 :
    mpMinVelocityEnabled[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_1 :
    mpMinVelocity[PUMP_1].Attach(pSubject);
    break;
  case SP_VCSP_PIPE_DIAMETER_1 :
    mpPipeDiameter[PUMP_1].Attach(pSubject);
    break;

  case SP_VCSP_FIXED_FREQ_PUMP_2 :
    mpFixedFreq[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_ECO_FREQ_PUMP_2 :
    mpEcoFreq[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_ECO_LEVEL_PUMP_2 :
    mpEcoLevel[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MAX_LEVEL_PUMP_2 :
    mpEcoMaxLevel[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_RUN_MODE_PUMP_2 :
    mpRunMode[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_REACTION_MODE_PUMP_2 :
    mpReactionMode[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_START_ENABLED_PUMP_2 :
    mpReverseStartEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_TIME_PUMP_2 :
    mpReverseTime[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_INTERVAL_PUMP_2 :
    mpReverseInterval[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_ENABLED_PUMP_2 :
    mpStartFlushEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_TIME_PUMP_2 :
    mpStartFlushTime[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_ENABLED_PUMP_2 :
    mpRunFlushEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_TIME_PUMP_2 :
    mpRunFlushTime[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_INTERVAL_PUMP_2 :
    mpRunFlushInterval[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_ENABLED_PUMP_2 :
    mpStopFlushEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_TIME_PUMP_2 :
    mpStopFlushTime[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_CUE_ENABLED_PUMP_2 :
    mpCueEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MIN_FREQ_PUMP_2 :
    mpCueMinFreq[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MAX_FREQ_PUMP_2 :
    mpCueMaxFreq[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MIN_FREQUENCY_PUMP_2 :
    mpEcoMinFrequency[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_FREQUENCY_LEARN_ENABLED_PUMP_2 :
    mpFrequencyLearnEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_TYPE_PUMP_2 :
    mpPidType[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_KP_PUMP_2 :
    mpPidKp[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_TI_PUMP_2 :
    mpPidTi[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_TD_PUMP_2 :
    mpPidTd[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_INVERSE_CONTROL_PUMP_2 :
    mpPidInverseControl[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_FEEDBACK_PUMP_2 :
    mpPidFeedback[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_TYPE_PUMP_2 :
    mpPidSetpointType[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_AI_PUMP_2 :
    mpPidSetpointAi[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_FIXED_PUMP_2 :
    mpPidSetpointFixed[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_ENABLED_2 :
    mpMinVelocityEnabled[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_2 :
    mpMinVelocity[PUMP_2].Attach(pSubject);
    break;
  case SP_VCSP_PIPE_DIAMETER_2 :
    mpPipeDiameter[PUMP_2].Attach(pSubject);
    break;

  case SP_VCSP_FIXED_FREQ_PUMP_3 :
    mpFixedFreq[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_ECO_FREQ_PUMP_3 :
    mpEcoFreq[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_ECO_LEVEL_PUMP_3 :
    mpEcoLevel[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MAX_LEVEL_PUMP_3 :
    mpEcoMaxLevel[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_RUN_MODE_PUMP_3 :
    mpRunMode[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_REACTION_MODE_PUMP_3 :
    mpReactionMode[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_START_ENABLED_PUMP_3 :
    mpReverseStartEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_TIME_PUMP_3 :
    mpReverseTime[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_INTERVAL_PUMP_3 :
    mpReverseInterval[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_ENABLED_PUMP_3 :
    mpStartFlushEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_TIME_PUMP_3 :
    mpStartFlushTime[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_ENABLED_PUMP_3 :
    mpRunFlushEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_TIME_PUMP_3 :
    mpRunFlushTime[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_INTERVAL_PUMP_3 :
    mpRunFlushInterval[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_ENABLED_PUMP_3 :
    mpStopFlushEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_TIME_PUMP_3 :
    mpStopFlushTime[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_CUE_ENABLED_PUMP_3 :
    mpCueEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MIN_FREQ_PUMP_3 :
    mpCueMinFreq[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MAX_FREQ_PUMP_3 :
    mpCueMaxFreq[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MIN_FREQUENCY_PUMP_3 :
    mpEcoMinFrequency[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_FREQUENCY_LEARN_ENABLED_PUMP_3 :
    mpFrequencyLearnEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_TYPE_PUMP_3 :
    mpPidType[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_KP_PUMP_3 :
    mpPidKp[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_TI_PUMP_3 :
    mpPidTi[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_TD_PUMP_3 :
    mpPidTd[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_INVERSE_CONTROL_PUMP_3 :
    mpPidInverseControl[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_FEEDBACK_PUMP_3 :
    mpPidFeedback[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_TYPE_PUMP_3 :
    mpPidSetpointType[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_AI_PUMP_3 :
    mpPidSetpointAi[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_FIXED_PUMP_3 :
    mpPidSetpointFixed[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_ENABLED_3 :
    mpMinVelocityEnabled[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_3 :
    mpMinVelocity[PUMP_3].Attach(pSubject);
    break;
  case SP_VCSP_PIPE_DIAMETER_3 :
    mpPipeDiameter[PUMP_3].Attach(pSubject);
    break;

  case SP_VCSP_FIXED_FREQ_PUMP_4 :
    mpFixedFreq[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_ECO_FREQ_PUMP_4 :
    mpEcoFreq[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_ECO_LEVEL_PUMP_4 :
    mpEcoLevel[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MAX_LEVEL_PUMP_4 :
    mpEcoMaxLevel[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_RUN_MODE_PUMP_4 :
    mpRunMode[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_REACTION_MODE_PUMP_4 :
    mpReactionMode[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_START_ENABLED_PUMP_4 :
    mpReverseStartEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_TIME_PUMP_4 :
    mpReverseTime[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_INTERVAL_PUMP_4 :
    mpReverseInterval[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_ENABLED_PUMP_4 :
    mpStartFlushEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_TIME_PUMP_4 :
    mpStartFlushTime[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_ENABLED_PUMP_4 :
    mpRunFlushEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_TIME_PUMP_4 :
    mpRunFlushTime[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_INTERVAL_PUMP_4 :
    mpRunFlushInterval[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_ENABLED_PUMP_4 :
    mpStopFlushEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_TIME_PUMP_4 :
    mpStopFlushTime[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_CUE_ENABLED_PUMP_4 :
    mpCueEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MIN_FREQ_PUMP_4 :
    mpCueMinFreq[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MAX_FREQ_PUMP_4 :
    mpCueMaxFreq[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MIN_FREQUENCY_PUMP_4 :
    mpEcoMinFrequency[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_FREQUENCY_LEARN_ENABLED_PUMP_4 :
    mpFrequencyLearnEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_TYPE_PUMP_4 :
    mpPidType[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_KP_PUMP_4 :
    mpPidKp[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_TI_PUMP_4 :
    mpPidTi[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_TD_PUMP_4 :
    mpPidTd[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_INVERSE_CONTROL_PUMP_4 :
    mpPidInverseControl[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_FEEDBACK_PUMP_4 :
    mpPidFeedback[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_TYPE_PUMP_4 :
    mpPidSetpointType[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_AI_PUMP_4 :
    mpPidSetpointAi[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_FIXED_PUMP_4 :
    mpPidSetpointFixed[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_ENABLED_4 :
    mpMinVelocityEnabled[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_4 :
    mpMinVelocity[PUMP_4].Attach(pSubject);
    break;
  case SP_VCSP_PIPE_DIAMETER_4 :
    mpPipeDiameter[PUMP_4].Attach(pSubject);
    break;

  case SP_VCSP_FIXED_FREQ_PUMP_5 :
    mpFixedFreq[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_ECO_FREQ_PUMP_5 :
    mpEcoFreq[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_ECO_LEVEL_PUMP_5 :
    mpEcoLevel[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MAX_LEVEL_PUMP_5 :
    mpEcoMaxLevel[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_RUN_MODE_PUMP_5 :
    mpRunMode[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_REACTION_MODE_PUMP_5 :
    mpReactionMode[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_START_ENABLED_PUMP_5 :
    mpReverseStartEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_TIME_PUMP_5 :
    mpReverseTime[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_INTERVAL_PUMP_5 :
    mpReverseInterval[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_ENABLED_PUMP_5 :
    mpStartFlushEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_TIME_PUMP_5 :
    mpStartFlushTime[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_ENABLED_PUMP_5 :
    mpRunFlushEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_TIME_PUMP_5 :
    mpRunFlushTime[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_INTERVAL_PUMP_5 :
    mpRunFlushInterval[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_ENABLED_PUMP_5 :
    mpStopFlushEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_TIME_PUMP_5 :
    mpStopFlushTime[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_CUE_ENABLED_PUMP_5 :
    mpCueEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MIN_FREQ_PUMP_5 :
    mpCueMinFreq[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MAX_FREQ_PUMP_5 :
    mpCueMaxFreq[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MIN_FREQUENCY_PUMP_5 :
    mpEcoMinFrequency[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_FREQUENCY_LEARN_ENABLED_PUMP_5 :
    mpFrequencyLearnEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_TYPE_PUMP_5 :
    mpPidType[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_KP_PUMP_5 :
    mpPidKp[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_TI_PUMP_5 :
    mpPidTi[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_TD_PUMP_5 :
    mpPidTd[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_INVERSE_CONTROL_PUMP_5 :
    mpPidInverseControl[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_FEEDBACK_PUMP_5 :
    mpPidFeedback[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_TYPE_PUMP_5 :
    mpPidSetpointType[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_AI_PUMP_5 :
    mpPidSetpointAi[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_FIXED_PUMP_5 :
    mpPidSetpointFixed[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_ENABLED_5 :
    mpMinVelocityEnabled[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_5 :
    mpMinVelocity[PUMP_5].Attach(pSubject);
    break;
  case SP_VCSP_PIPE_DIAMETER_5 :
    mpPipeDiameter[PUMP_5].Attach(pSubject);
    break;

  case SP_VCSP_FIXED_FREQ_PUMP_6 :
    mpFixedFreq[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_ECO_FREQ_PUMP_6 :
    mpEcoFreq[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_ECO_LEVEL_PUMP_6 :
    mpEcoLevel[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MAX_LEVEL_PUMP_6 :
    mpEcoMaxLevel[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_RUN_MODE_PUMP_6 :
    mpRunMode[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_REACTION_MODE_PUMP_6 :
    mpReactionMode[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_START_ENABLED_PUMP_6 :
    mpReverseStartEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_TIME_PUMP_6 :
    mpReverseTime[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_REVERSE_INTERVAL_PUMP_6 :
    mpReverseInterval[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_ENABLED_PUMP_6 :
    mpStartFlushEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_START_FLUSH_TIME_PUMP_6 :
    mpStartFlushTime[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_ENABLED_PUMP_6 :
    mpRunFlushEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_TIME_PUMP_6 :
    mpRunFlushTime[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_RUN_FLUSH_INTERVAL_PUMP_6 :
    mpRunFlushInterval[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_ENABLED_PUMP_6 :
    mpStopFlushEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_STOP_FLUSH_TIME_PUMP_6 :
    mpStopFlushTime[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_CUE_ENABLED_PUMP_6 :
    mpCueEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MIN_FREQ_PUMP_6 :
    mpCueMinFreq[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_CUE_MAX_FREQ_PUMP_6 :
    mpCueMaxFreq[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_ECO_MIN_FREQUENCY_PUMP_6 :
    mpEcoMinFrequency[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_FREQUENCY_LEARN_ENABLED_PUMP_6 :
    mpFrequencyLearnEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_TYPE_PUMP_6 :
    mpPidType[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_KP_PUMP_6 :
    mpPidKp[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_TI_PUMP_6 :
    mpPidTi[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_TD_PUMP_6 :
    mpPidTd[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_INVERSE_CONTROL_PUMP_6 :
    mpPidInverseControl[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_FEEDBACK_PUMP_6 :
    mpPidFeedback[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_TYPE_PUMP_6 :
    mpPidSetpointType[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_AI_PUMP_6 :
    mpPidSetpointAi[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PID_SETPOINT_FIXED_PUMP_6 :
    mpPidSetpointFixed[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_ENABLED_6 :
    mpMinVelocityEnabled[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_MIN_VELOCITY_6 :
    mpMinVelocity[PUMP_6].Attach(pSubject);
    break;
  case SP_VCSP_PIPE_DIAMETER_6 :
    mpPipeDiameter[PUMP_6].Attach(pSubject);
    break;

  case SP_VCSP_SENSOR_FLOW :
    mpMeasuredFlow.Attach(pSubject);
    break;
  case SP_VCSP_SENSOR_LEVEL_PRESSURE :
    mpMeasuredLevelPressure.Attach(pSubject);
    break;
  case SP_VCSP_SENSOR_LEVEL_ULTRASOUND :
    mpMeasuredLevelUltrasound.Attach(pSubject);
    break;
  case SP_VCSP_SENSOR_USER_1 :
    mpMeasuredUserDefinedSensor1.Attach(pSubject);
    break;
  case SP_VCSP_SENSOR_USER_2 :
    mpMeasuredUserDefinedSensor2.Attach(pSubject);
    break;
  case SP_VCSP_SENSOR_USER_3 :
    mpMeasuredUserDefinedSensor3.Attach(pSubject);
    break;
  case SP_VCSP_VFD_INSTALLED_PUMP1:
	mpVfdInstalled[PUMP_1].Attach(pSubject);
	break;
  case SP_VCSP_VFD_INSTALLED_PUMP2:
	mpVfdInstalled[PUMP_2].Attach(pSubject);
	break;
  case SP_VCSP_VFD_INSTALLED_PUMP3:
	mpVfdInstalled[PUMP_3].Attach(pSubject);
	break;
  case SP_VCSP_VFD_INSTALLED_PUMP4:
	mpVfdInstalled[PUMP_4].Attach(pSubject);
	break;
  case SP_VCSP_VFD_INSTALLED_PUMP5:
	mpVfdInstalled[PUMP_5].Attach(pSubject);
	break;
  case SP_VCSP_VFD_INSTALLED_PUMP6:
	mpVfdInstalled[PUMP_6].Attach(pSubject);
	break;
  }
}

/*****************************************************************************
 * Function -
 * DESCRIPTION:
 *
 ****************************************************************************/
void VfdConfSlipPoint::ConnectToSubjects(void)
{
  mpCurrentVfdNumber->Subscribe(this);

  for (int vfd_no = 0; vfd_no < NO_OF_PUMPS; vfd_no++)
  {
    mpFixedFreq[vfd_no]->Subscribe(this);
    mpEcoFreq[vfd_no]->Subscribe(this);
    mpEcoLevel[vfd_no]->Subscribe(this);
    mpEcoMaxLevel[vfd_no]->Subscribe(this);
    mpRunMode[vfd_no]->Subscribe(this);
    mpReactionMode[vfd_no]->Subscribe(this);
    mpReverseStartEnabled[vfd_no]->Subscribe(this);
    mpReverseTime[vfd_no]->Subscribe(this);
    mpReverseInterval[vfd_no]->Subscribe(this);
    mpStartFlushEnabled[vfd_no]->Subscribe(this);
    mpStartFlushTime[vfd_no]->Subscribe(this);
    mpRunFlushEnabled[vfd_no]->Subscribe(this);
    mpRunFlushTime[vfd_no]->Subscribe(this);
    mpRunFlushInterval[vfd_no]->Subscribe(this);
    mpStopFlushEnabled[vfd_no]->Subscribe(this);
    mpStopFlushTime[vfd_no]->Subscribe(this);
    mpCueEnabled[vfd_no]->Subscribe(this);
    mpCueMinFreq[vfd_no]->Subscribe(this);
    mpCueMaxFreq[vfd_no]->Subscribe(this);
    mpEcoMinFrequency[vfd_no]->Subscribe(this);
    mpFrequencyLearnEnabled[vfd_no]->Subscribe(this);
    mpPidType[vfd_no]->Subscribe(this);
    mpPidKp[vfd_no]->Subscribe(this);
    mpPidTi[vfd_no]->Subscribe(this);
    mpPidTd[vfd_no]->Subscribe(this);
    mpPidInverseControl[vfd_no]->Subscribe(this);
    mpPidFeedback[vfd_no]->Subscribe(this);
    mpPidSetpointType[vfd_no]->Subscribe(this);
    mpPidSetpointAi[vfd_no]->Subscribe(this);
    mpPidSetpointFixed[vfd_no]->Subscribe(this);
    mpMinVelocityEnabled[vfd_no]->Subscribe(this);
    mpMinVelocity[vfd_no]->Subscribe(this);
    mpPipeDiameter[vfd_no]->Subscribe(this);
	mpVfdInstalled[vfd_no]->Subscribe(this);
  }

  mpVirtualFixedFreq->Subscribe(this);
  mpVirtualEcoFreq->Subscribe(this);
  mpVirtualEcoLevel->Subscribe(this);
  mpVirtualEcoMaxLevel->Subscribe(this);
  mpVirtualRunMode->Subscribe(this);
  mpVirtualReactionMode->Subscribe(this);
  mpVirtualReverseStartEnabled->Subscribe(this);
  mpVirtualReverseTime->Subscribe(this);
  mpVirtualReverseInterval->Subscribe(this);
  mpVirtualStartFlushEnabled->Subscribe(this);
  mpVirtualStartFlushTime->Subscribe(this);
  mpVirtualRunFlushEnabled->Subscribe(this);
  mpVirtualRunFlushTime->Subscribe(this);
  mpVirtualRunFlushInterval->Subscribe(this);
  mpVirtualStopFlushEnabled->Subscribe(this);
  mpVirtualStopFlushTime->Subscribe(this);
  mpVirtualCueEnabled->Subscribe(this);
  mpVirtualCueMinFreq->Subscribe(this);
  mpVirtualCueMaxFreq->Subscribe(this);
  mpVirtualEcoMinFrequency->Subscribe(this);
  mpVirtualFrequencyLearnEnabled->Subscribe(this);
  mpVirtualPidType->Subscribe(this);
  mpVirtualPidKp->Subscribe(this);
  mpVirtualPidTi->Subscribe(this);
  mpVirtualPidTd->Subscribe(this);
  mpVirtualPidInverseControl->Subscribe(this);
  mpVirtualPidFeedback->Subscribe(this);
  mpVirtualPidSetpointType->Subscribe(this);
  mpVirtualPidSetpointAi->Subscribe(this);
  mpVirtualPidSetpointFixed->Subscribe(this);
  mpVirtualMinVelocityEnabled->Subscribe(this);
  mpVirtualMinVelocity->Subscribe(this);
  mpVirtualPipeDiameter->Subscribe(this);
}



/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
void VfdConfSlipPoint::UpdateVirtualVfdConf()
{
  const int vfd_no = mpCurrentVfdNumber->GetAsInt();

  if ((vfd_no >= 0) && (vfd_no < NO_OF_PUMPS))
  {
    mpVirtualFixedFreq->SetValue(mpFixedFreq[vfd_no]->GetValue());
    mpVirtualEcoFreq->SetValue(mpEcoFreq[vfd_no]->GetValue());
    mpVirtualEcoLevel->SetValue(mpEcoLevel[vfd_no]->GetValue());
    mpVirtualEcoMaxLevel->SetValue(mpEcoMaxLevel[vfd_no]->GetValue());
    mpVirtualRunMode->SetValue(mpRunMode[vfd_no]->GetValue());
    mpVirtualReactionMode->SetValue(mpReactionMode[vfd_no]->GetValue());
    mpVirtualReverseStartEnabled->SetValue(mpReverseStartEnabled[vfd_no]->GetValue());
    mpVirtualReverseTime->SetValue(mpReverseTime[vfd_no]->GetValue());
    mpVirtualReverseInterval->SetValue(mpReverseInterval[vfd_no]->GetValue());
    mpVirtualStartFlushEnabled->SetValue(mpStartFlushEnabled[vfd_no]->GetValue());
    mpVirtualStartFlushTime->SetValue(mpStartFlushTime[vfd_no]->GetValue());
    mpVirtualRunFlushEnabled->SetValue(mpRunFlushEnabled[vfd_no]->GetValue());
    mpVirtualRunFlushTime->SetValue(mpRunFlushTime[vfd_no]->GetValue());
    mpVirtualRunFlushInterval->SetValue(mpRunFlushInterval[vfd_no]->GetValue());
    mpVirtualStopFlushEnabled->SetValue(mpStopFlushEnabled[vfd_no]->GetValue());
    mpVirtualStopFlushTime->SetValue(mpStopFlushTime[vfd_no]->GetValue());
    mpVirtualCueEnabled->SetValue(mpCueEnabled[vfd_no]->GetValue());
    mpVirtualCueMinFreq->SetValue(mpCueMinFreq[vfd_no]->GetValue());
    mpVirtualCueMaxFreq->SetValue(mpCueMaxFreq[vfd_no]->GetValue());
    mpVirtualEcoMinFrequency->SetValue(mpEcoMinFrequency[vfd_no]->GetValue());
    mpVirtualFrequencyLearnEnabled->SetValue(mpFrequencyLearnEnabled[vfd_no]->GetValue());
    mpVirtualPidType->SetValue(mpPidType[vfd_no]->GetValue());
    mpVirtualPidKp->SetValue(mpPidKp[vfd_no]->GetValue());
    mpVirtualPidTi->SetValue(mpPidTi[vfd_no]->GetValue());
    mpVirtualPidTd->SetValue(mpPidTd[vfd_no]->GetValue());
    mpVirtualPidInverseControl->SetValue(mpPidInverseControl[vfd_no]->GetValue());
	if((mpPidFeedback[vfd_no]->GetValue() > 3 && mpPidFeedback[vfd_no]->GetValue() < 7) ||
	  (mpPidFeedback[vfd_no]->GetValue()  > 12 && mpPidFeedback[vfd_no]->GetValue() < 16))
	{
		mpVirtualPidFeedback->SetValue(mpPidFeedback[vfd_no]->GetValue());
	}
	else
	{
		if(mpPidFeedback[vfd_no]->GetValue() >= 0 && mpPidFeedback[vfd_no]->GetValue() < 4)
		{
			mpPidFeedback[vfd_no]->SetValue(MEASURED_VALUE_FLOW);
			mpVirtualPidFeedback->SetValue(mpPidFeedback[vfd_no]->GetValue());
		}
		else if(mpPidFeedback[vfd_no]->GetValue() > 6 && mpPidFeedback[vfd_no]->GetValue() < 13)
		{
			mpPidFeedback[vfd_no]->SetValue(MEASURED_VALUE_USER_DEFINED_SOURCE_1);		
			mpVirtualPidFeedback->SetValue(mpPidFeedback[vfd_no]->GetValue());
		}
		else
		{
			mpPidFeedback[vfd_no]->SetValue(MEASURED_VALUE_USER_DEFINED_SOURCE_3);	
			mpVirtualPidFeedback->SetValue(mpPidFeedback[vfd_no]->GetValue());
		}		

	}
    mpVirtualPidSetpointType->SetValue(mpPidSetpointType[vfd_no]->GetValue());
	if((mpPidSetpointAi[vfd_no]->GetValue() > 12 && mpPidSetpointAi[vfd_no]->GetValue() < 16))
	{
		mpVirtualPidSetpointAi->SetValue(mpPidSetpointAi[vfd_no]->GetValue());
	}
	else
	{
		if(mpPidSetpointAi[vfd_no]->GetValue() >= 0 && mpPidSetpointAi[vfd_no]->GetValue() < 13)
		{
			mpPidSetpointAi[vfd_no]->SetValue(MEASURED_VALUE_USER_DEFINED_SOURCE_1);
			mpVirtualPidSetpointAi->SetValue(mpPidSetpointAi[vfd_no]->GetValue());
		}
		else
		{
			mpPidSetpointAi[vfd_no]->SetValue(MEASURED_VALUE_USER_DEFINED_SOURCE_3);
			mpVirtualPidSetpointAi->SetValue(mpPidSetpointAi[vfd_no]->GetValue());
		}
	}
    mpVirtualPidSetpointFixed->SetValue(mpPidSetpointFixed[vfd_no]->GetValue());
    mpVirtualMinVelocityEnabled->SetValue(mpMinVelocityEnabled[vfd_no]->GetValue());
    mpVirtualMinVelocity->SetValue(mpMinVelocity[vfd_no]->GetValue());
    mpVirtualPipeDiameter->SetValue(mpPipeDiameter[vfd_no]->GetValue());
  }
  else
  {
    FatalErrorOccured("VCSP index out of range!");
  }
}

void VfdConfSlipPoint::UpdateVirtualFixedSetpoint(bool ResetValue)
{
  //using CopyValues will set min/max/quantity and reset setpoint value to the currently measured value
  switch (mpVirtualPidFeedback->GetValue())
  {
  case MEASURED_VALUE_FLOW:
    mpVirtualPidSetpointFixed->CopyValues(mpMeasuredFlow.GetSubject());
    break;
  case MEASURED_VALUE_LEVEL_PRESSURE:
    mpVirtualPidSetpointFixed->CopyValues(mpMeasuredLevelPressure.GetSubject());
    break;
  case MEASURED_VALUE_LEVEL_ULTRA_SOUND:
    mpVirtualPidSetpointFixed->CopyValues(mpMeasuredLevelUltrasound.GetSubject());
    break;
  case MEASURED_VALUE_USER_DEFINED_SOURCE_1:
    mpVirtualPidSetpointFixed->CopyValues(mpMeasuredUserDefinedSensor1.GetSubject());
    break;
  case MEASURED_VALUE_USER_DEFINED_SOURCE_2:
    mpVirtualPidSetpointFixed->CopyValues(mpMeasuredUserDefinedSensor2.GetSubject());
    break;
  case MEASURED_VALUE_USER_DEFINED_SOURCE_3:
    mpVirtualPidSetpointFixed->CopyValues(mpMeasuredUserDefinedSensor3.GetSubject());
    break;
  }

  // the PID setpoint value should always be available, even when the source is not
  mpVirtualPidSetpointFixed->SetQuality(DP_AVAILABLE);

  // restore the current setpoint value
  if (!ResetValue)
  {
    const int vfd_no = mpCurrentVfdNumber->GetAsInt();
    mpVirtualPidSetpointFixed->SetValueAsPercent(mpPidSetpointFixed[vfd_no]->GetValueAsPercent());
  }
}


void VfdConfSlipPoint::UpdateCurrentVfdConf()
{
  const int vfd_no = mpCurrentVfdNumber->GetAsInt();

  if ((vfd_no >= 0) && (vfd_no < NO_OF_PUMPS))
  {
    mpFixedFreq[vfd_no]->SetValue(mpVirtualFixedFreq->GetValue());
    mpEcoFreq[vfd_no]->SetValue(mpVirtualEcoFreq->GetValue());
    mpEcoLevel[vfd_no]->SetValue(mpVirtualEcoLevel->GetValue());
    mpEcoMaxLevel[vfd_no]->SetValue(mpVirtualEcoMaxLevel->GetValue());
    mpRunMode[vfd_no]->SetValue(mpVirtualRunMode->GetValue());
    mpReactionMode[vfd_no]->SetValue(mpVirtualReactionMode->GetValue());
    mpReverseStartEnabled[vfd_no]->SetValue(mpVirtualReverseStartEnabled->GetValue());
    mpReverseTime[vfd_no]->SetValue(mpVirtualReverseTime->GetValue());
    mpReverseInterval[vfd_no]->SetValue(mpVirtualReverseInterval->GetValue());
    mpStartFlushEnabled[vfd_no]->SetValue(mpVirtualStartFlushEnabled->GetValue());
    mpStartFlushTime[vfd_no]->SetValue(mpVirtualStartFlushTime->GetValue());
    mpRunFlushEnabled[vfd_no]->SetValue(mpVirtualRunFlushEnabled->GetValue());
    mpRunFlushTime[vfd_no]->SetValue(mpVirtualRunFlushTime->GetValue());
    mpRunFlushInterval[vfd_no]->SetValue(mpVirtualRunFlushInterval->GetValue());
    mpStopFlushEnabled[vfd_no]->SetValue(mpVirtualStopFlushEnabled->GetValue());
    mpStopFlushTime[vfd_no]->SetValue(mpVirtualStopFlushTime->GetValue());
    mpCueEnabled[vfd_no]->SetValue(mpVirtualCueEnabled->GetValue());
    mpCueMinFreq[vfd_no]->SetValue(mpVirtualCueMinFreq->GetValue());
    mpCueMaxFreq[vfd_no]->SetValue(mpVirtualCueMaxFreq->GetValue());
    mpEcoMinFrequency[vfd_no]->SetValue(mpVirtualEcoMinFrequency->GetValue());
    mpFrequencyLearnEnabled[vfd_no]->SetValue(mpVirtualFrequencyLearnEnabled->GetValue());
    mpPidType[vfd_no]->SetValue(mpVirtualPidType->GetValue());
    mpPidKp[vfd_no]->SetValue(mpVirtualPidKp->GetValue());
    mpPidTi[vfd_no]->SetValue(mpVirtualPidTi->GetValue());
    mpPidTd[vfd_no]->SetValue(mpVirtualPidTd->GetValue());
    mpPidInverseControl[vfd_no]->SetValue(mpVirtualPidInverseControl->GetValue());
	mpPidFeedback[vfd_no]->SetValue(mpVirtualPidFeedback->GetValue());
/*	if((mpVirtualPidFeedback->GetValue() > 3 && mpVirtualPidFeedback->GetValue() < 7) ||
	  (mpVirtualPidFeedback->GetValue()  > 12 && mpVirtualPidFeedback->GetValue() < 16))
	{
		mpPidFeedback[vfd_no]->SetValue(mpVirtualPidFeedback->GetValue());
	}
	else
	{
		mpPidFeedback[vfd_no]->SetValue(mpPidFeedback[vfd_no]->GetValue());

	}*/
    mpPidSetpointType[vfd_no]->SetValue(mpVirtualPidSetpointType->GetValue());
    mpPidSetpointAi[vfd_no]->SetValue(mpVirtualPidSetpointAi->GetValue());
    mpPidSetpointFixed[vfd_no]->CopyValues(mpVirtualPidSetpointFixed.GetSubject());
    mpMinVelocityEnabled[vfd_no]->SetValue(mpVirtualMinVelocityEnabled->GetValue());
    mpMinVelocity[vfd_no]->SetValue(mpVirtualMinVelocity->GetValue());
    mpPipeDiameter[vfd_no]->SetValue(mpVirtualPipeDiameter->GetValue());
  }
  else
  {
    FatalErrorOccured("VCSP index out of range!");
  }
}

void VfdConfSlipPoint::UpdateUpperStatusLine()
{
  char display_number[14];

  Display* p_display = GetDisplay(DISPLAY_VFD_SETTINGS_ID);

  int index = mpCurrentVfdNumber->GetAsInt() + 1;

  sprintf(display_number, "4.2.10.%i", index);
  p_display->SetDisplayNumber( display_number );
  p_display->SetName(SID_VFD_SETTINGS_PUMP_1 + index - 1);

  p_display = GetDisplay(DISPLAY_VFD_PID_SETTINGS_ID);
  sprintf(display_number, "4.2.10.%i.1", index);
  p_display->SetDisplayNumber( display_number );
  p_display->SetName(SID_PID_SETTINGS_PUMP_1 + index - 1);

  p_display = GetDisplay(DISPLAY_VFD_PID_FEEDBACK_ID);
  sprintf(display_number, "4.2.10.%i.1.1", index);
  p_display->SetDisplayNumber( display_number );

  p_display = GetDisplay(DISPLAY_VFD_PID_SETPOINT_ID);
  sprintf(display_number, "4.2.10.%i.1.2", index);
  p_display->SetDisplayNumber( display_number );

  p_display = GetDisplay(DISPLAY_VFD_FLUSH_SETTINGS_ID);
  sprintf(display_number, "4.2.10.%i.2", index);
  p_display->SetDisplayNumber( display_number );
  p_display->SetName(SID_FLUSH_SETTINGS_PUMP_1 + index - 1);

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


