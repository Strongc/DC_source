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
/* CLASS NAME       : PowerAndEnergyCtrl                                    */
/*                                                                          */
/* FILE NAME        : PowerAndEnergyCtrl.h                                  */
/*                                                                          */
/* CREATED DATE     : 03-08-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcPowerAndEnergyCtrl_h
#define mrcPowerAndEnergyCtrl_h

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
class PowerAndEnergyCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    PowerAndEnergyCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~PowerAndEnergyCtrl();
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
    void CheckEnergyConfig();
    void CalculateDeltaEnergy();
    void UpdateAccumulatedEnergy();
    void UpdateEnergyBasedPower();
    void UpdateSystemPower();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs
    SubjectPtr<U32DataPoint*>     mpPulseEnergyRatioConfig;
    SubjectPtr<U32DataPoint*>     mpPulseEnergyRatioDisplay;
    SubjectPtr<EnumDataPoint<PULSE_ENERGY_UNIT_TYPE>*> mpPulseEnergyUnit;
    SubjectPtr<U32DataPoint*>     mpPowerUpdateTime;

    // Variable inputs:
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;
    SubjectPtr<FloatDataPoint*>   mpMeasuredValuePower;
    SubjectPtr<U32DataPoint*>     mpRawEnergyPulses;
    SubjectPtr<FloatDataPoint*>   mpPowerPump[MAX_NO_OF_PUMPS];
    SubjectPtr<U8DataPoint*>      mpNoOfPumps;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpOperationModeActualPump[MAX_NO_OF_PUMPS];

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpSystemPower;
    SubjectPtr<U32DataPoint*>     mpEnergyPulseCounter;
    SubjectPtr<U32DataPoint*>     mpTotalEnergyKwh;
    SubjectPtr<U32DataPoint*>     mpFreeRunningEnergyWh;
    SubjectPtr<FloatDataPoint*>   mpTotalEnergy;

    // Class variables
    float                         mDeltaEnergy;
    float                         mEnergyBasedPower;
    U8                            mDeltaPulses;
    bool                          mPulsesReady;
    U8                            mOldPulses;
    float                         mIncrementWh;
    U32                           mIncrementKWh;
    U32                           mPulsesForUpdate;
    U32                           mTimeSinceUpdate;
    bool                          mPumpHasBeenStopped;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
