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
/* CLASS NAME       : VfdMasterCtrl                                         */
/*                                                                          */
/* FILE NAME        : VfdMasterCtrl.h                                       */
/*                                                                          */
/* CREATED DATE     : 04-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Master class for handling of                    */
/*                          variable frequency drives                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcVfdMasterCtrl_h
#define mrcVfdMasterCtrl_h

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
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
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
class VfdMasterCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    VfdMasterCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~VfdMasterCtrl();
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
    VFD_OPERATION_MODE_TYPE HandleAutoOperation(float &vfdFrequency, bool &vfdStart, bool &vfdReverse);
    float LinearRegulation(void);
    float MinimumRegulation(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration inputs:
    SubjectPtr<BoolDataPoint*>    mpVfdInstalled;
    SubjectPtr<EnumDataPoint<VFD_RUN_MODE_TYPE>*>           mpVfdRunMode;
    SubjectPtr<EnumDataPoint<VFD_REACTION_MODE_TYPE>*>      mpVfdReactionMode;
    SubjectPtr<FloatDataPoint*>   mpVfdFixedFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyLevel;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyMaxLevel;
    SubjectPtr<FloatDataPoint*>   mpVfdEconomyMinFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdMinFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdMaxFrequency;

    // Variable inputs:
    SubjectPtr<BoolDataPoint*>    mpVfdPumpStartRequest;
    SubjectPtr<BoolDataPoint*>    mpPumpReadyForAutoOpr;
    SubjectPtr<BoolDataPoint*>    mpAntiSeizingRequestPump;
    SubjectPtr<BoolDataPoint*>    mpHighLevelSwitchActivated;
    SubjectPtr<FloatDataPoint*>   mpSurfaceLevel;
    SubjectPtr<U8DataPoint*>      mpNoOfRunningPumps;
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*>     mpVfdFlushRequest;
    SubjectPtr<FloatDataPoint*>   mpVfdPidFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdEnergyTestFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdFlowTrainingFrequency;
    SubjectPtr<FloatDataPoint*>   mpVfdMaxReverseSpeed;

    // Outputs:
    SubjectPtr<FloatDataPoint*>   mpVfdFrequency;
    SubjectPtr<BoolDataPoint*>    mpVfdStart;
    SubjectPtr<BoolDataPoint*>    mpVfdReverse;
    SubjectPtr<EnumDataPoint<VFD_OPERATION_MODE_TYPE>*>     mpVfdState;
    SubjectPtr<BoolDataPoint*>    mpVfdAutoOprRequest;

    // Local variables:
    float mLinearScale;
    float mLinearOffset;
    U32   mSkipRunCounter;
    float mMinimumRegulationFrequency;
    U32   MinimumRegulationSwitchTime;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
