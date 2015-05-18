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
/* FILE NAME        : VfdConfSlipPoint.H                                    */
/*                                                                          */
/* CREATED DATE     : 07-04-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the VFD configuration DataPoints into one                */
/* set of virtual DataPoints for the display to look at...                  */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcVfdConfSlipPoint_h
#define mrcVfdConfSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

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
    namespace ctrl
    {

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class VfdConfSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    VfdConfSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~VfdConfSlipPoint();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void InitSubTask(void);
    virtual void RunSubTask(void);

    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject);
    virtual void ConnectToSubjects(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void UpdateVirtualVfdConf(void);
    virtual void UpdateCurrentVfdConf(void);
    virtual void UpdateVirtualFixedSetpoint(bool ResetValue);

    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U8DataPoint*>                      mpCurrentVfdNumber;

    SubjectPtr<FloatDataPoint*> 	                mpVirtualFixedFreq;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualEcoFreq;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualEcoLevel;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualEcoMaxLevel;
    SubjectPtr<EnumDataPoint<VFD_RUN_MODE_TYPE>*>	mpVirtualRunMode;
    SubjectPtr<EnumDataPoint<VFD_REACTION_MODE_TYPE>*> mpVirtualReactionMode;
    SubjectPtr<BoolDataPoint*>                    mpVirtualReverseStartEnabled;
    SubjectPtr<U32DataPoint*>                     mpVirtualReverseTime;
    SubjectPtr<U32DataPoint*>                     mpVirtualReverseInterval;
    SubjectPtr<BoolDataPoint*>                    mpVirtualStartFlushEnabled;
    SubjectPtr<U32DataPoint*>                     mpVirtualStartFlushTime;
    SubjectPtr<BoolDataPoint*>                    mpVirtualRunFlushEnabled;
    SubjectPtr<U32DataPoint*>                     mpVirtualRunFlushTime;
    SubjectPtr<U32DataPoint*>                     mpVirtualRunFlushInterval;
    SubjectPtr<BoolDataPoint*>                    mpVirtualStopFlushEnabled;
    SubjectPtr<U32DataPoint*>                     mpVirtualStopFlushTime;
    SubjectPtr<BoolDataPoint*>                    mpVirtualCueEnabled;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualCueMinFreq;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualCueMaxFreq;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualEcoMinFrequency;
    SubjectPtr<BoolDataPoint*>                    mpVirtualFrequencyLearnEnabled;
    SubjectPtr<EnumDataPoint<PID_TYPE_TYPE>*>	    mpVirtualPidType;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualPidKp;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualPidTi;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualPidTd;
    SubjectPtr<BoolDataPoint*>                    mpVirtualPidInverseControl;
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpVirtualPidFeedback;
    SubjectPtr<EnumDataPoint<PID_SETPOINT_TYPE_TYPE>*> mpVirtualPidSetpointType;
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpVirtualPidSetpointAi;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualPidSetpointFixed;
    SubjectPtr<BoolDataPoint*>                    mpVirtualMinVelocityEnabled;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualMinVelocity;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualPipeDiameter;

    SubjectPtr<FloatDataPoint*> 	                mpFixedFreq[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpEcoFreq[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpEcoLevel[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpEcoMaxLevel[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<VFD_RUN_MODE_TYPE>*>	mpRunMode[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<VFD_REACTION_MODE_TYPE>*> mpReactionMode[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpReverseStartEnabled[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpReverseTime[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpReverseInterval[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpStartFlushEnabled[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpStartFlushTime[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpRunFlushEnabled[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpRunFlushTime[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpRunFlushInterval[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpStopFlushEnabled[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpStopFlushTime[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpCueEnabled[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpCueMinFreq[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpCueMaxFreq[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpEcoMinFrequency[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpFrequencyLearnEnabled[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpFrequencyLearnSettlingTime[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<PID_TYPE_TYPE>*>	    mpPidType[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpPidKp[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpPidTi[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpPidTd[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>                    mpPidInverseControl[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpPidFeedback[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<PID_SETPOINT_TYPE_TYPE>*> mpPidSetpointType[NO_OF_PUMPS];
    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpPidSetpointAi[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpPidSetpointFixed[NO_OF_PUMPS];

    SubjectPtr<BoolDataPoint*> 	                  mpMinVelocityEnabled[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpMinVelocity[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpPipeDiameter[NO_OF_PUMPS];

    SubjectPtr<FloatDataPoint*> 	                mpMeasuredFlow;
    SubjectPtr<FloatDataPoint*> 	                mpMeasuredLevelPressure;
    SubjectPtr<FloatDataPoint*> 	                mpMeasuredLevelUltrasound;
    SubjectPtr<FloatDataPoint*> 	                mpMeasuredUserDefinedSensor1;
    SubjectPtr<FloatDataPoint*> 	                mpMeasuredUserDefinedSensor2;
    SubjectPtr<FloatDataPoint*> 	                mpMeasuredUserDefinedSensor3;

	SubjectPtr<BoolDataPoint*>						mpVfdInstalled[NO_OF_PUMPS];


    bool mCurrentlyUpdating;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
    } // namespace ctrl
  } // namespace display
} // namespace mpc
#endif
