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
/* CLASS NAME       : WizardCtrl                                            */
/*                                                                          */
/* FILE NAME        : WizardCtrl.h                                          */
/*                                                                          */
/* CREATED DATE     : 23-01-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Changes current display based on actions        */
/*                          and settings made in display wizard.            */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcWizardCtrl_h
#define mrcWizardCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subtask.h>
#include <Observer.h>
#include <EnumDataPoint.h>
#include <U32DataPoint.h>
#include <U16DataPoint.h>
#include <U8DataPoint.h>
#include <BoolDataPoint.h>
#include <EventDataPoint.h>
#include <StringDataPoint.h>
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

namespace mpc
{
  namespace display
  {
    namespace ctrl
    {
      
  typedef struct {
    bool energy;
    bool volume;
    bool user1;
    bool user2;
    bool user3;
  }SELECTED_COUNTER_INPUTS;

    /*****************************************************************************
    * CLASS:
    * DESCRIPTION:
    *
    *****************************************************************************/
    class WizardCtrl: public SubTask, public Observer
    {
      public:
        /********************************************************************
        LIFECYCLE - Default constructor.
        ********************************************************************/
        WizardCtrl();
        /********************************************************************
        LIFECYCLE - Destructor.
        ********************************************************************/
        virtual ~WizardCtrl();
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
        void InitializePitSettings(void);
        void CommitPitSettings(void);
        void CommitMixerSettings(void);
        void InitializeOptionSettings(void);
        void CommitOptionSettings(void);
        void CommitCommunicationSettings(void);
        int CalcFreeDigitalInputsCount(void);
        bool IsOptionComboLegal(void);
        bool OneOrMoreVfdInstalled(void);
        
        void AssignDigitalInput(DI_INDEX_TYPE di, DIGITAL_INPUT_FUNC_TYPE input);
        void AssignCounterInput(DI_INDEX_TYPE di, SELECTED_COUNTER_INPUTS* inputs, std::vector<DI_INDEX_TYPE>* pAvailableInputs);


        /********************************************************************
        ATTRIBUTE
        ********************************************************************/
        SubjectPtr<U16DataPoint*>       mCurrentDisplayId;
        SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*>  mControlType;
        SubjectPtr<BoolDataPoint*>      mWizardEnabled;
        SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*>  mAI1MeasuredValue;
        SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*>  mAI1ElectricalType;
        SubjectPtr<BoolDataPoint*>      mMixerInstalled;
        SubjectPtr<BoolDataPoint*>      mMixerEnabled;
        SubjectPtr<U8DataPoint*>        mNoOfFsw;
        SubjectPtr<U8DataPoint*>        mNoOfPumps;
        SubjectPtr<U8DataPoint*>        mFswConfigNumber;
        SubjectPtr<BoolDataPoint*>      mOverflowSwitchInstalled;

        SubjectPtr<BoolDataPoint*>      mPumpInOperation[NO_OF_PUMPS];

        SubjectPtr<BoolDataPoint*>      mUltrasonicSensorInversed;
        SubjectPtr<FloatDataPoint*>     mUltrasonicSensorOffset;

        SubjectPtr<FloatDataPoint*>     mMixerStopLevel;
        SubjectPtr<FloatDataPoint*>     mMixerStartLevelOffset;

        SubjectPtr<BoolDataPoint*>      mPumpGroupsEnabled;
        SubjectPtr<U32DataPoint*>       mPumpGroup1MaxStartedPumps;
        SubjectPtr<U32DataPoint*>       mPumpGroup1MinStartedPumps;
        SubjectPtr<BoolDataPoint*>      mPumpGroup1AlternationEnabled;

        SubjectPtr<BoolDataPoint*>      mVfdInstalled[NO_OF_PUMPS];
        SubjectPtr<BoolDataPoint*>      mCueInterfaceUsed[NO_OF_PUMPS];
        SubjectPtr<EnumDataPoint<ANA_OUT_FUNC_TYPE>*> mVfdAoFunc[NO_OF_PUMPS];

        /* step 11: options */
        SubjectPtr<U8DataPoint*>        mFreeDICount;
        SubjectPtr<BoolDataPoint*>      mPumpCFEnabled[NO_OF_PUMPS];
        SubjectPtr<BoolDataPoint*>      mPumpOOAEnabled[NO_OF_PUMPS];
        SubjectPtr<BoolDataPoint*>      mPumpMPEnabled[NO_OF_PUMPS];
        SubjectPtr<BoolDataPoint*>      mPumpVfdReadyEnabled[NO_OF_PUMPS];
        SubjectPtr<BoolDataPoint*>      mCommonPhaseErrorEnabled;
        SubjectPtr<BoolDataPoint*>      mVolumeMeterEnabled;
        SubjectPtr<BoolDataPoint*>      mEnergyMeterEnabled;
        SubjectPtr<BoolDataPoint*>      mMixerCFEnabled;
        SubjectPtr<BoolDataPoint*>      mAlarmResetEnabled;
        SubjectPtr<BoolDataPoint*>      mRelayResetEnabled;
        SubjectPtr<BoolDataPoint*>      mExternalFaultEnabled;
        SubjectPtr<BoolDataPoint*>      mUserDefinedCounter1Enabled;
        SubjectPtr<BoolDataPoint*>      mUserDefinedCounter2Enabled;
        SubjectPtr<BoolDataPoint*>      mUserDefinedCounter3Enabled;

        SubjectPtr<U32DataPoint*>       mPumpCFDiNo[NO_OF_PUMPS];
        SubjectPtr<U32DataPoint*>       mPumpOODiNo[NO_OF_PUMPS];
        SubjectPtr<U32DataPoint*>       mPumpAMDiNo[NO_OF_PUMPS];
        SubjectPtr<U32DataPoint*>       mPumpMPDiNo[NO_OF_PUMPS];
        SubjectPtr<U32DataPoint*>       mPumpVfdReadyDiNo[NO_OF_PUMPS];
        SubjectPtr<U32DataPoint*>       mCommonPhaseDiNo;
        SubjectPtr<U32DataPoint*>       mVolumeMeterDiNo;
        SubjectPtr<U32DataPoint*>       mEnergyMeterDiNo;
        SubjectPtr<U32DataPoint*>       mMixerCFDiNo;
        SubjectPtr<U32DataPoint*>       mAlarmResetDiNo;
        SubjectPtr<U32DataPoint*>       mRelayResetDiNo;
        SubjectPtr<U32DataPoint*>       mExternalFaultDiNo;
        SubjectPtr<U32DataPoint*>       mOverflowSwitchDiNo;
        SubjectPtr<U32DataPoint*>       mUserDefinedCounter1DiNo;
        SubjectPtr<U32DataPoint*>       mUserDefinedCounter2DiNo;
        SubjectPtr<U32DataPoint*>       mUserDefinedCounter3DiNo;

        SubjectPtr<StringDataPoint*>    mScadaNo;
        SubjectPtr<EventDataPoint*>     mEnableScadaInAlarmConfigs;

        SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mDiFunc[NO_OF_DI_INDEX];
        SubjectPtr<BoolDataPoint*>      mDiLogic[NO_OF_DI_INDEX];

        SubjectPtr<EnumDataPoint<RELAY_FUNC_TYPE>*> mDoFunc[NO_OF_DO_INDEX];

        SubjectPtr<U32DataPoint*>       mNoOfIo351Modules;
        
        bool                            mCurrentlyUpdating;

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
