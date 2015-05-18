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
/* CLASS NAME       : BlockageDetectionCtrl                                 */
/*                                                                          */
/* FILE NAME        : BlockageDetectionCtrl.h                               */
/*                                                                          */
/* CREATED DATE     : 16-12-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Class to detect blockage of a pump.             */
/* Compares the present pump parameters to a previously recorded set ´      */
/* of pump parameters                                                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcBlockageDetectionCtrl_h
#define mrcBlockageDetectionCtrl_h

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
#include <EventDataPoint.h>
#include <FloatDataPoint.h>
#include <FloatVectorDataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_PARAMETER = 0,
  PARAMETER_COSHPI = FIRST_PARAMETER,
  PARAMETER_CURRENT,
  PARAMETER_POWER,

  NO_OF_PARAMETER,
  LAST_PARAMETER = NO_OF_PARAMETER - 1
} PARAMETER_TYPE;

typedef enum
{
  BDC_IDLE = 0,                       //waiting for start event
  BDC_WAITING_FOR_ALL_PUMPS_TO_STOP,  //waiting for start conditions
  BDC_WAITING_FOR_PUMP_TO_START,      //waiting for start conditions
  BDC_SAMPLE_VALUES,                  //while pumping (and start conditions are meet)
  BDC_SAMPLING_COMPLETED,             //when sampling is completed with success

  NO_OF_BDC_STORING_STATE,
  LAST_BDC_STORING_STATE = NO_OF_BDC_STORING_STATE - 1
} BDC_STORING_STATE;


/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class BlockageDetectionCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    BlockageDetectionCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~BlockageDetectionCtrl();
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
    void RunParameterStorage(void);
    void SampleParameters(bool samplingIsCompleted);
    void UpdateMinMaxValues(void);
    void RunBlockageDetection(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    // Configuration inputs:
    SubjectPtr<FloatDataPoint*>   mpMaxVariation[NO_OF_PARAMETER];
    SubjectPtr<U32DataPoint*>     mpDetectionDelay;

    // Variable inputs:
    SubjectPtr<EventDataPoint*>   mpStoreNormalParametersEvent;
    SubjectPtr<FloatDataPoint*>   mpMeasuredValue[NO_OF_PARAMETER];
    SubjectPtr<FloatDataPoint*>   mpStartLevel;
    SubjectPtr<FloatDataPoint*>   mpStopLevel;
    SubjectPtr<FloatDataPoint*>   mpMeasuredLevel;
    SubjectPtr<FloatDataPoint*>   mpVfdFrequency;
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*> mpVfdState;
    SubjectPtr<BoolDataPoint*>    mpPumpStarted;
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpMinOfNormal[NO_OF_PARAMETER];
    SubjectPtr<FloatDataPoint*>   mpMaxOfNormal[NO_OF_PARAMETER];
    SubjectPtr<U32DataPoint*>     mpDateOfNormalParameters;
    SubjectPtr<BoolDataPoint*>    mpBlockageDetected;

    // Local variables:
    SubjectPtr<FloatVectorDataPoint*>  mpNormalValues[NO_OF_PARAMETER];

    //flag to control process of parameter sampling/storage
    BDC_STORING_STATE mStoringState;

    float             mIntervalSize;
    float             mTotalValueInInterval[NO_OF_PARAMETER];
    U32               mIntervalIndex;
    I32               mNoOfSamplesInInterval;
    I32               mBlockedTimeInSeconds;
    I32               mPumpRunningSeconds;
    bool              mPumpRunningNormal;
    bool              mPumpStartedAtStartLevel;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
