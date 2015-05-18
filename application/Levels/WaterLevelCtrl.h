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
/* CLASS NAME       : WaterLevelCtrl                                        */
/*                                                                          */
/* FILE NAME        : WaterLevelCtrl.h                                      */
/*                                                                          */
/* CREATED DATE     : 01-10-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcWaterLevelCtrl_h
#define mrcWaterLevelCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
#include <FloatDataPoint.h>
#include <EnumDataPoint.h>
#include <AppTypeDefs.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>


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
class WaterLevelCtrl : public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    WaterLevelCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~WaterLevelCtrl();
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
    SubjectPtr<FloatDataPoint*> mpWaterLevelUltraSound;
    SubjectPtr<FloatDataPoint*> mpMeasuredValueHeightPressure;
    SubjectPtr<FloatDataPoint*> mpSurfaceLevel;
    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;
    SubjectPtr<BoolDataPoint*> mpLevelSimEnabled;
    SubjectPtr<FloatDataPoint*> mpLevelSim;

    SubjectPtr<BoolDataPoint*> mpAnalogValuesMeasuredFlag;
    SubjectPtr<BoolDataPoint*> mpSurfaceLevelReadyFlag;

    bool mRunRequestedFlag;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
