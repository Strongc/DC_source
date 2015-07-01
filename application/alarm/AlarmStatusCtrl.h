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
/* CLASS NAME       : AlarmStatusCtrl                                       */
/*                                                                          */
/* FILE NAME        : AlarmStatusCtrl.h                                     */
/*                                                                          */
/* CREATED DATE     : 02-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Generates alarm status parameters for Geni      */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAlarmStatusCtrl_h
#define mrcAlarmStatusCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
#include <AlarmLog.h>
#include <AlarmDef.h>
#include <AppTypeDefs.h>
#include <EnumDataPoint.h>
#include <EnumDataPoint.h>
#include <U16DataPoint.h>
#include <BoolDataPoint.h>
#include <U32DataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define NO_OF_GENI_STATUS_WORDS   4   // Max number of status words per group in the geni profile

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

 /*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class AlarmStatusCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AlarmStatusCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AlarmStatusCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer( int Id, Subject* pSubject );
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled( Subject* pSubject );

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool ConvertSystemAlarmToGeniStatus(ALARM_ID_TYPE alarm_id, U16 &word_no, U16 &bit_no);
    bool ConvertPumpAlarmToGeniStatus(ALARM_ID_TYPE alarm_id, U16 &word_no, U16 &bit_no);
    bool ConvertDosingPumpAlarmToGeniStatus(ALARM_ID_TYPE alarm_id, U16 &word_no, U16 &bit_no);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Input:
    SubjectPtr<AlarmLog*>      mpAlarmLog;
    SubjectPtr<BoolDataPoint*> mpScadaEnabled;
    SubjectPtr<U32DataPoint*>  mpTimeStampLastScadaAck;
    SubjectPtr<BoolDataPoint*> mpScadaLatchingEnabled;

    // Output:
    SubjectPtr<EnumDataPoint<ALARM_ID_TYPE>*> mpActualAlarm;
    SubjectPtr<EnumDataPoint<ALARM_ID_TYPE>*> mpActualWarning;
    SubjectPtr<U16DataPoint*> mpSystemAlarmStatus[NO_OF_GENI_STATUS_WORDS];
    SubjectPtr<U16DataPoint*> mpSystemWarningStatus[NO_OF_GENI_STATUS_WORDS];
    SubjectPtr<U16DataPoint*> mpPumpAlarmStatus[NO_OF_GENI_STATUS_WORDS][NO_OF_PUMPS];
    SubjectPtr<U16DataPoint*> mpPumpWarningStatus[NO_OF_GENI_STATUS_WORDS][NO_OF_PUMPS];
    SubjectPtr<U16DataPoint*> mpPumpFault;
    SubjectPtr<U16DataPoint*> mpPumpMonitoringFault;

    SubjectPtr<BoolDataPoint*> mpPumpAlarmFlag[NO_OF_PUMPS];

    SubjectPtr<BoolDataPoint*> mpServiceModeEnabled;

    // Class parameters
    U16 mSystemAlarmStatus[NO_OF_GENI_STATUS_WORDS];
    U16 mSystemWarningStatus[NO_OF_GENI_STATUS_WORDS];
    U16 mSystemAlarmScadaPending[NO_OF_GENI_STATUS_WORDS];
    U16 mSystemWarningScadaPending[NO_OF_GENI_STATUS_WORDS];
    U16 mPumpAlarmStatus[NO_OF_GENI_STATUS_WORDS][NO_OF_PUMPS];
    U16 mPumpWarningStatus[NO_OF_GENI_STATUS_WORDS][NO_OF_PUMPS];
    U16 mPumpAlarmScadaPending[NO_OF_GENI_STATUS_WORDS][NO_OF_PUMPS];
    U16 mPumpWarningScadaPending[NO_OF_GENI_STATUS_WORDS][NO_OF_PUMPS];
    U16 mPumpFault;
    U16 mPumpMonitoringFault;

    bool mReqTaskTimeFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
