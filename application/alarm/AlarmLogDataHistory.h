/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AlarmLogDataHistory                                   */
/*                                                                          */
/* FILE NAME        : AlarmLogDataHistory.h                                 */
/*                                                                          */
/* CREATED DATE     : 25-06-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : AlarmLogDataHistory keeps a record of various   */
/* values for the last 3 seconds to give AlarmLog a chance to find their    */
/* state just before an alarm occurred.                                     */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ALARM_LOG_DATA_HISTORY_H__
#define __ALARM_LOG_DATA_HISTORY_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subject.h>
#include <SubjectPtr.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
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
 * CLASS: AlarmLog
 ****************************************************************************/
class AlarmLogDataHistory : public SubTask, public Observer
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static AlarmLogDataHistory* GetInstance();

    virtual void RunSubTask();
    virtual void InitSubTask(void);

    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

    struct DataHistoryEntry
    {
        DataHistoryEntry()
        {
            mSurfaceLevelReady = false;
            mSurfaceLevel = 0.0;
            mAverageFlow = 0.0;

            mSurfaceLevelReadyQuality = DP_NEVER_AVAILABLE;
            mSurfaceLevelQuality = DP_NEVER_AVAILABLE;
            mAverageFlowQuality = DP_NEVER_AVAILABLE;

            for (unsigned int i = 0; i < NO_OF_PUMPS; ++i)
            {
                mPumpOperatingMode[i] = FIRST_ACTUAL_OPERATION_MODE;
                mPumpVoltage[i] = 0.0;
                mPumpCurrent[i] = 0.0;
                mPumpCosPhi[i] = 0.0;
                mPumpPower[i] = 0.0;
                mPumpFlow[i] = 0.0;
                mPumpTemperature[i] = 0.0;

                mPumpOperatingModeQuality[i] = DP_NEVER_AVAILABLE;
                mPumpVoltageQuality[i] = DP_NEVER_AVAILABLE;
                mPumpCurrentQuality[i] = DP_NEVER_AVAILABLE;
                mPumpCosPhiQuality[i] = DP_NEVER_AVAILABLE;
                mPumpPowerQuality[i] = DP_NEVER_AVAILABLE;
                mPumpFlowQuality[i] = DP_NEVER_AVAILABLE;
                mPumpTemperatureQuality[i] = DP_NEVER_AVAILABLE;
            }
        }

        bool mSurfaceLevelReady;
        float mSurfaceLevel;
        float mAverageFlow;

        ACTUAL_OPERATION_MODE_TYPE mPumpOperatingMode[NO_OF_PUMPS];
        float mPumpVoltage[NO_OF_PUMPS];
        float mPumpCurrent[NO_OF_PUMPS];
        float mPumpCosPhi[NO_OF_PUMPS];
        float mPumpPower[NO_OF_PUMPS];
        float mPumpFlow[NO_OF_PUMPS];
        float mPumpTemperature[NO_OF_PUMPS];

        DP_QUALITY_TYPE mSurfaceLevelReadyQuality;
        DP_QUALITY_TYPE mSurfaceLevelQuality;
        DP_QUALITY_TYPE mAverageFlowQuality;

        DP_QUALITY_TYPE mPumpOperatingModeQuality[NO_OF_PUMPS];
        DP_QUALITY_TYPE mPumpVoltageQuality[NO_OF_PUMPS];
        DP_QUALITY_TYPE mPumpCurrentQuality[NO_OF_PUMPS];
        DP_QUALITY_TYPE mPumpCosPhiQuality[NO_OF_PUMPS];
        DP_QUALITY_TYPE mPumpPowerQuality[NO_OF_PUMPS];
        DP_QUALITY_TYPE mPumpFlowQuality[NO_OF_PUMPS];
        DP_QUALITY_TYPE mPumpTemperatureQuality[NO_OF_PUMPS];
    };

    const DataHistoryEntry GetBestDataHistoryEntry(int pumpErrorIndex) const;

  private:
    /********************************************************************
    LIFECYCLE - Default constructor. Private because it is a Singleton
    ********************************************************************/
    AlarmLogDataHistory();
    /********************************************************************
    LIFECYCLE - Destructor. Private because it is a Singleton
    ********************************************************************/
    ~AlarmLogDataHistory();
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    unsigned int mScanSkipCounter;

    SubjectPtr<BoolDataPoint*> mpSurfaceLevelReady;
    SubjectPtr<FloatDataPoint*> mpSurfaceLevel;
    SubjectPtr<FloatDataPoint*> mpAverageFlow;

    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpPumpOperatingMode[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpVoltage[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpCurrent[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpCosPhi[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpPower[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpFlow[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpPumpTemperature[NO_OF_PUMPS];


    DataHistoryEntry mDataHistory[6];
    unsigned int mDataHistoryIndex;
    bool mDataHistoryFull;
    mutable OS_RSEMA mSemaDataHistory;


    static AlarmLogDataHistory* mInstance;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
