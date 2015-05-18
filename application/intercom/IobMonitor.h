/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/* CLASS NAME       : IobMonitor                                            */
/*                                                                          */
/* FILE NAME        : IobMonitor.h                                          */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcIobMonitor_h
#define mpcIobMonitor_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <U8DataPoint.h>
#include <U16DataPoint.h>
#include <U32DataPoint.h>
#include <EventDataPoint.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>

#include <SubTask.h>

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
class IobMonitor : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    IobMonitor();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~IobMonitor();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void InitSubTask(void);
    void RunSubTask(void);
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    float ScaleTemperature(U32 val);
    float ScalePressure(U32 val);
    float ScaleBatteryVoltage(U32 val);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U8DataPoint*> mpDigIn;
    SubjectPtr<U32DataPoint*> mpAnaIn0;
    SubjectPtr<U32DataPoint*> mpAnaIn1;
    SubjectPtr<U32DataPoint*> mpAnaIn2;
    SubjectPtr<U32DataPoint*> mpAnaIn3;
    SubjectPtr<U32DataPoint*> mpAnaIn4;
    SubjectPtr<U32DataPoint*> mpAnaIn5;
    SubjectPtr<EventDataPoint*> mpSimEnable;
    SubjectPtr<EventDataPoint*> mpSimDisable;
    SubjectPtr<U32DataPoint*> mpSimMode;
    SubjectPtr<U32DataPoint*> mpSimStatus;
    SubjectPtr<U8DataPoint*> mpSimDigIn;
    SubjectPtr<U32DataPoint*> mpSimValueAD0;
    SubjectPtr<U32DataPoint*> mpSimValueAD1;
    SubjectPtr<U32DataPoint*> mpSimValueAD2;
    SubjectPtr<U32DataPoint*> mpSimValueAD3;
    SubjectPtr<U32DataPoint*> mpSimValueAD4;
    SubjectPtr<U32DataPoint*> mpSimValueAD5;
    int mRunCount;

    SubjectPtr<EnumDataPoint<IOB_BOARD_ID_TYPE>*> mpIobBoardId;
    SubjectPtr<FloatDataPoint*> mpIobTemperature;
    SubjectPtr<FloatDataPoint*> mpIobPressure;
    SubjectPtr<FloatDataPoint*> mpIobBatteryVoltage;
    SubjectPtr<U16DataPoint*> mpIobAiPressureRaw;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
