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
/* CLASS NAME       : IoChannel                                             */
/*                                                                          */
/* FILE NAME        : IoChannel.h                                           */
/*                                                                          */
/* CREATED DATE     : 15-22-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcIoChannel_h
#define mrcIoChannel_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <SubTask.h>
#include <BoolDataPoint.h>
#include <SwTimer.h>

#include <FloatDataPoint.h>
#include <AlarmDataPoint.h>
#include <EnumDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include "IoChannelConfig.h"

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
class IoChannel : public SubTask, public SwTimerBaseClass, public BoolDataPoint
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    IoChannel();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~IoChannel();
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

    virtual IoChannelConfig* GetIoChannelConfig();

    // don't allow change of quality for I/O Channels
    virtual bool SetQuality(DP_QUALITY_TYPE quality){return false;};
    virtual bool SetQuality(DP_QUALITY_TYPE quality, bool notify){return false;};

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    bool  mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool GetValueFromSource(void);
    void SwitchSource(void);
    bool IsReadyToChangeOutput(void);
    
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<IoChannelConfig*> mpIoChannelConfig;

    SubjectPtr<Subject*> mpDpSource;

    FloatDataPoint* mpFloatSource;
    AlarmDataPoint* mpAlarmSource;
    BoolDataPoint* mpBoolSource;
    EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>* mpPumpOperationMode;
    EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>* mpDiEnumSource;

    
    bool mResponseTimeTimeOut;
    bool mTimerFunctionTimeOut;
    bool mReduceUpdateRateTimeOut;
    bool mReduceUpdateRate;

};

#endif
