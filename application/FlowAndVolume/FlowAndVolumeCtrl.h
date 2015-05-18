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
/* CLASS NAME       : FlowAndVolumeCtrl                                     */
/*                                                                          */
/* FILE NAME        : FlowAndVolumeCtrl.h                                   */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                    Calculate/select the best possible system flow.       */
/*                    Calculate and accumulate the total volume.            */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcFlowAndVolumeCtrl_h
#define mrcFlowAndVolumeCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>
#include <U16DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/



/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class FlowAndVolumeCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FlowAndVolumeCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FlowAndVolumeCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitSubTask();
    void RunSubTask();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);


  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void CheckVolumeConfig();
    void CalculateDeltaVolume();
    void UpdateAccumulatedVolume();
    void UpdateVolumeBasedFlow();
    void UpdateSystemFlow();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<U32DataPoint*>     mpPulseVolumeRatioConfig;
    SubjectPtr<FloatDataPoint*>     mpPulseVolumeRatioDisplay;
    SubjectPtr<EnumDataPoint<PULSE_VOLUME_UNIT_TYPE>*> mpPulseVolumeUnit;
    SubjectPtr<U32DataPoint*>     mpFlowUpdateTime;

    // Variable inputs:
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;
    SubjectPtr<U32DataPoint*>     mpRawVolumePulses;
    SubjectPtr<FloatDataPoint*>   mpMeasuredValueFlow;
    SubjectPtr<FloatDataPoint*>   mpPitBasedFlow;
    SubjectPtr<FloatDataPoint*>   mpAdvFlowBasedFlow;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpSystemFlow;
    SubjectPtr<EnumDataPoint<FLOW_QUALITY_TYPE>*> mpFlowQuality;
    SubjectPtr<U32DataPoint*>     mpVolumePulseCounter;
    SubjectPtr<U32DataPoint*>     mpPumpedVolumeLitreForLog;
    SubjectPtr<FloatDataPoint*>   mpPumpedVolumeM3ForDisplay;
    SubjectPtr<U32DataPoint*>     mpFreeRunningVolumeLitre;
	SubjectPtr<U16DataPoint*>     mpTotalVolumeOverRun;
	

    // Class variables
    bool                          mUpdatingAccumulatedVolume;
    float                         mDeltaVolumeInM3;
    float                         mVolumeBasedFlow;
    float                         mMaxFlow;
    U8                            mDeltaPulses;
    U8                            mOldPulses;
    bool                          mPulsesReady;
    float                         mIncrementLitre;
    U32                           mIncrementM3;
    U32                           mPulsesForUpdate;
    U32                           mTimeSinceUpdate;
	bool                          mValidateOverflowVolume;
	U32							  mPreviousLitre;
	

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
