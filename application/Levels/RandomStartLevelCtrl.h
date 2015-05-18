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
/* CLASS NAME       : RandomStartLevelCtrl                                  */
/*                                                                          */
/* FILE NAME        : RandomStartLevelCtrl.h                                */
/*                                                                          */
/* CREATED DATE     : 03-05-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcRandomStartLevelCtrl_h
#define mrcRandomStartLevelCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
#include <AppTypeDefs.h>
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <FloatDataPoint.h>
#include <U8DataPoint.h>
#include <AlarmConfig.h>

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
class RandomStartLevelCtrl: public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    RandomStartLevelCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~RandomStartLevelCtrl();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    // Subject.
    void SetSubjectPointer(int Id, Subject* pSubject);
    void ConnectToSubjects();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);

    // SubTask
    virtual void RunSubTask();
    virtual void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mSensorType;
    SubjectPtr<BoolDataPoint*>  mpVariationEnabled;
    SubjectPtr<BoolDataPoint*>  mpAdvFlowLearningInProgress;
    SubjectPtr<U8DataPoint*>    mpNoOfPumps;
    SubjectPtr<U8DataPoint*>    mpNumberOfRunningPumps;

    SubjectPtr<FloatDataPoint*> mpUpperMeasurementLevel;
    SubjectPtr<AlarmConfig*>    mpHighLevel;
    SubjectPtr<FloatDataPoint*> mpLowestStartLevel;
    SubjectPtr<FloatDataPoint*> mpSecondLowestStartLevel;
    SubjectPtr<FloatDataPoint*> mpVariation;

    SubjectPtr<FloatDataPoint*> mpStartLevelWithVariation;
    SubjectPtr<FloatDataPoint*> mpMaxLevelWithVariation;

    bool mRunRequestedFlag;
    bool mRunRequestedByConfigFlag;


  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
