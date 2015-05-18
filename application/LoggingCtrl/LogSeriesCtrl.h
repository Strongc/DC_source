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
/* CLASS NAME       : LogSeriesCtrl                                         */
/*                                                                          */
/* FILE NAME        : LogSeriesCtrl.h                                       */
/*                                                                          */
/* CREATED DATE     : 22-07-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :  Class to handle the log series                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcLogSeriesCtrl_h
#define mrcLogSeriesCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>
#include <SubTask.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <EventDataPoint.h>
#include <FloatDataPoint.h>
#include <U16VectorDataPoint.h>
#include <BoolVectorDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define MAX_NO_OF_LOG_SERIES    64    // Must match the number of log series (e.g. log item vector size)
#define NO_OF_LOG_STORAGE       5     // Must match the number of log series storages in data base
#define NO_OF_WORDS_PER_STORAGE 8128  // Must match the log series storage vector size in data base
#define MAX_NO_OF_WORDS_FOR_LOG (NO_OF_LOG_STORAGE*NO_OF_WORDS_PER_STORAGE)

// Log header
#define LOG_HEAD_SIZE           8
#define LOG_HEAD_VERSION        1
#define LOG_HEAD_CHECK_VALUE    0xDA1A
#define LOG_TAIL_SIZE           2
#define LOG_TAIL_CHECK_VALUE    0xDA10
// Word offsets in log header
#define LOG_HEAD_IDX            0
#define LOG_NUMBER_IDX          1
#define LOG_ITEM_IDX            2
#define LOG_RATE_IDX            3
#define LOG_SIZE_IDX            4
#define LOG_STAMP_HI_IDX        5
#define LOG_STAMP_LO_IDX        6
#define LOG_CYCLIC_IDX          7
#define LOG_DATA_IDX            LOG_HEAD_SIZE
#define LOG_CHECK_IDX           (LOG_HEAD_SIZE+0) // + no of samples, variable
#define LOG_TAIL_IDX            (LOG_HEAD_SIZE+1) // + no of samples, variable


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  LOG_ITEM_NOT_USED               =   0xFFFF,

  // System / Pit
  LOG_ITEM_WATER_LEVEL            =   1,
  LOG_ITEM_SWITCH_WATER_LEVEL     =   2,
  LOG_ITEM_FLOW                   =   3,
  LOG_ITEM_POWER                  =   7,
  LOG_ITEM_SPECIFIC_ENERGY        =   8,
  LOG_ITEM_SIGNAL_LEVEL           =   9,
  LOG_ITEM_MIXER_STARTS_PR_HOUR   =   10,
  LOG_ITEM_OUTLET_PRESSURE        =   11,
  LOG_ITEM_ESTIMATED_FLOW         =   12,
  LOG_ITEM_USER_DEFINED_SOURCE_1  =   500,
  LOG_ITEM_USER_DEFINED_SOURCE_2  =   501,
  LOG_ITEM_USER_DEFINED_SOURCE_3  =   502,
  LOG_ITEM_USER_DEFINED_COUNTER_1 =   503,
  LOG_ITEM_USER_DEFINED_COUNTER_2 =   504,
  LOG_ITEM_USER_DEFINED_COUNTER_3 =   505,

  // Pump 1
  LOG_ITEM_PUMP_1_FILTERED_FLOW     =   1000,
  LOG_ITEM_PUMP_1_LATEST_FLOW       =   1001,
  LOG_ITEM_PUMP_1_ACTUAL_CURRENT    =   1002,
  LOG_ITEM_PUMP_1_INSULATION        =   1003,
  LOG_ITEM_PUMP_1_WATER_IN_OIL      =   1004,
  LOG_ITEM_PUMP_1_TEMPERATURE1      =   1005,
  LOG_ITEM_PUMP_1_TEMPERATURE2      =   1006,
  LOG_ITEM_PUMP_1_POWER             =   1008,
  LOG_ITEM_PUMP_1_MAINS_VOLTAGE     =   1009,
  LOG_ITEM_PUMP_1_LATEST_CURRENT    =   1010,
  LOG_ITEM_PUMP_1_STARTS_PR_HOUR    =   1011,
  LOG_ITEM_PUMP_1_LATEST_RUN_TIME   =   1012,
  LOG_ITEM_PUMP_1_FREQUENCY         =   1013,
  LOG_ITEM_PUMP_1_TORQUE            =   1014,
  LOG_ITEM_PUMP_1_CURRENT_ASYMMETRY =   1015,
  LOG_ITEM_PUMP_1_MOISTURE_SWITCH   =   1100, // TBD

  // Pump 2
  LOG_ITEM_PUMP_2_FILTERED_FLOW     =   2000,
  LOG_ITEM_PUMP_2_LATEST_FLOW       =   2001,
  LOG_ITEM_PUMP_2_ACTUAL_CURRENT    =   2002,
  LOG_ITEM_PUMP_2_INSULATION        =   2003,
  LOG_ITEM_PUMP_2_WATER_IN_OIL      =   2004,
  LOG_ITEM_PUMP_2_TEMPERATURE1      =   2005,
  LOG_ITEM_PUMP_2_TEMPERATURE2      =   2006,
  LOG_ITEM_PUMP_2_POWER             =   2008,
  LOG_ITEM_PUMP_2_MAINS_VOLTAGE     =   2009,
  LOG_ITEM_PUMP_2_LATEST_CURRENT    =   2010,
  LOG_ITEM_PUMP_2_STARTS_PR_HOUR    =   2011,
  LOG_ITEM_PUMP_2_LATEST_RUN_TIME   =   2012,
  LOG_ITEM_PUMP_2_FREQUENCY         =   2013,
  LOG_ITEM_PUMP_2_TORQUE            =   2014,
  LOG_ITEM_PUMP_2_CURRENT_ASYMMETRY =   2015,
  LOG_ITEM_PUMP_2_MOISTURE_SWITCH   =   2100, // TBD

  // Pump 3
  LOG_ITEM_PUMP_3_FILTERED_FLOW     =   3000,
  LOG_ITEM_PUMP_3_LATEST_FLOW       =   3001,
  LOG_ITEM_PUMP_3_ACTUAL_CURRENT    =   3002,
  LOG_ITEM_PUMP_3_INSULATION        =   3003,
  LOG_ITEM_PUMP_3_WATER_IN_OIL      =   3004,
  LOG_ITEM_PUMP_3_TEMPERATURE1      =   3005,
  LOG_ITEM_PUMP_3_TEMPERATURE2      =   3006,
  LOG_ITEM_PUMP_3_POWER             =   3008,
  LOG_ITEM_PUMP_3_MAINS_VOLTAGE     =   3009,
  LOG_ITEM_PUMP_3_LATEST_CURRENT    =   3010,
  LOG_ITEM_PUMP_3_STARTS_PR_HOUR    =   3011,
  LOG_ITEM_PUMP_3_LATEST_RUN_TIME   =   3012,
  LOG_ITEM_PUMP_3_FREQUENCY         =   3013,
  LOG_ITEM_PUMP_3_TORQUE            =   3014,
  LOG_ITEM_PUMP_3_CURRENT_ASYMMETRY =   3015,
  LOG_ITEM_PUMP_3_MOISTURE_SWITCH   =   3100, // TBD

  // Pump 4
  LOG_ITEM_PUMP_4_FILTERED_FLOW     =   4000,
  LOG_ITEM_PUMP_4_LATEST_FLOW       =   4001,
  LOG_ITEM_PUMP_4_ACTUAL_CURRENT    =   4002,
  LOG_ITEM_PUMP_4_INSULATION        =   4003,
  LOG_ITEM_PUMP_4_WATER_IN_OIL      =   4004,
  LOG_ITEM_PUMP_4_TEMPERATURE1      =   4005,
  LOG_ITEM_PUMP_4_TEMPERATURE2      =   4006,
  LOG_ITEM_PUMP_4_POWER             =   4008,
  LOG_ITEM_PUMP_4_MAINS_VOLTAGE     =   4009,
  LOG_ITEM_PUMP_4_LATEST_CURRENT    =   4010,
  LOG_ITEM_PUMP_4_STARTS_PR_HOUR    =   4011,
  LOG_ITEM_PUMP_4_LATEST_RUN_TIME   =   4012,
  LOG_ITEM_PUMP_4_FREQUENCY         =   4013,
  LOG_ITEM_PUMP_4_TORQUE            =   4014,
  LOG_ITEM_PUMP_4_CURRENT_ASYMMETRY =   4015,
  LOG_ITEM_PUMP_4_MOISTURE_SWITCH   =   4100, // TBD

  // Pump 5
  LOG_ITEM_PUMP_5_FILTERED_FLOW     =   5000,
  LOG_ITEM_PUMP_5_LATEST_FLOW       =   5001,
  LOG_ITEM_PUMP_5_ACTUAL_CURRENT    =   5002,
  LOG_ITEM_PUMP_5_INSULATION        =   5003,
  LOG_ITEM_PUMP_5_WATER_IN_OIL      =   5004,
  LOG_ITEM_PUMP_5_TEMPERATURE1      =   5005,
  LOG_ITEM_PUMP_5_TEMPERATURE2      =   5006,
  LOG_ITEM_PUMP_5_POWER             =   5008,
  LOG_ITEM_PUMP_5_MAINS_VOLTAGE     =   5009,
  LOG_ITEM_PUMP_5_LATEST_CURRENT    =   5010,
  LOG_ITEM_PUMP_5_STARTS_PR_HOUR    =   5011,
  LOG_ITEM_PUMP_5_LATEST_RUN_TIME   =   5012,
  LOG_ITEM_PUMP_5_FREQUENCY         =   5013,
  LOG_ITEM_PUMP_5_TORQUE            =   5014,
  LOG_ITEM_PUMP_5_CURRENT_ASYMMETRY =   5015,
  LOG_ITEM_PUMP_5_MOISTURE_SWITCH   =   5100, // TBD

  // Pump 6
  LOG_ITEM_PUMP_6_FILTERED_FLOW     =   6000,
  LOG_ITEM_PUMP_6_LATEST_FLOW       =   6001,
  LOG_ITEM_PUMP_6_ACTUAL_CURRENT    =   6002,
  LOG_ITEM_PUMP_6_INSULATION        =   6003,
  LOG_ITEM_PUMP_6_WATER_IN_OIL      =   6004,
  LOG_ITEM_PUMP_6_TEMPERATURE1      =   6005,
  LOG_ITEM_PUMP_6_TEMPERATURE2      =   6006,
  LOG_ITEM_PUMP_6_POWER             =   6008,
  LOG_ITEM_PUMP_6_MAINS_VOLTAGE     =   6009,
  LOG_ITEM_PUMP_6_LATEST_CURRENT    =   6010,
  LOG_ITEM_PUMP_6_STARTS_PR_HOUR    =   6011,
  LOG_ITEM_PUMP_6_LATEST_RUN_TIME   =   6012,
  LOG_ITEM_PUMP_6_FREQUENCY         =   6013,
  LOG_ITEM_PUMP_6_TORQUE            =   6014,
  LOG_ITEM_PUMP_6_CURRENT_ASYMMETRY =   6015,
  LOG_ITEM_PUMP_6_MOISTURE_SWITCH   =   6100, // TBD

  LOG_ITEM_END_OF_LIST_DUMMY
} LOG_ITEM_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class LogSeriesCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    LogSeriesCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~LogSeriesCtrl();
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
    U16  GetLogValue(int offset);
    void SetLogValue(int offset, U16 value);
    void FillLogValues(int dest, U16 value, int count);
    void CopyLogValues(int dest, int src, int count);

    void ReorganizeLogValues(int offset);
    void AddLogValues(int offset, U16 value, int count, U32 time_stamp);

    void ValidateLogData(void);
    void FillLogDataAtPowerUp(void);
    void FillValuesForTest(bool test_active);
    void HandleChangedEnabled(void);
    void HandleChangedLogItem(void);
    void HandleChangedSampleRate(void);
    void HandleChangedNoOfSamples(void);
    U16  GetMeasuredValueToLog(int log_no);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Configuration
    SubjectPtr<BoolVectorDataPoint*>    mpLogSeriesEnabled;
    SubjectPtr<U16VectorDataPoint*>     mpLogSeriesItem;
    SubjectPtr<U16VectorDataPoint*>     mpLogSeriesSampleRate;
    SubjectPtr<U16VectorDataPoint*>     mpLogSeriesNoOfSamples;

    // Inputs:
    SubjectPtr<EventDataPoint*>         mpFillSimulatedLogData;
    SubjectPtr<U32DataPoint*>           mpRtcSecondsSince1970;
    SubjectPtr<U32DataPoint*>           mpFloatSwitchWaterLevel;
    SubjectPtr<EnumDataPoint<GSM_SIGNAL_LEVEL_TYPE>*> mpGsmSignalLevel;
    SubjectPtr<U32DataPoint*>           mpMixerStartsPrHour;
    SubjectPtr<U32DataPoint*>           mpPumpStartsPrHour[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>           mpPumpLatestRunTime[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*>          mpPumpMoistureSwitch[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*>         *mpSourceValueList;  // A list of source values for float value log items
    SubjectPtr<U32DataPoint*>           mpUserDefinedCounter_1;
    SubjectPtr<U32DataPoint*>           mpUserDefinedCounter_2;
    SubjectPtr<U32DataPoint*>           mpUserDefinedCounter_3;

    // Outputs:
    SubjectPtr<U16VectorDataPoint*>     mpLogSeriesDataOffsetTable;
    SubjectPtr<U16VectorDataPoint*>     mpLogSeriesData[NO_OF_LOG_STORAGE];

    U32   mIndexList[MAX_NO_OF_LOG_SERIES];
    U32   mOffsetList[MAX_NO_OF_LOG_SERIES+1];  // Must have one extra location

    U32   mAwaitStartCount;
    bool  mLogDataSimulation;
    bool  mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
