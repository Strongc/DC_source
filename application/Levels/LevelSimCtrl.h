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
/* CLASS NAME       : LevelSimCtrl                                          */
/*                                                                          */
/* FILE NAME        : LevelSimCtrl.h                                        */
/*                                                                          */
/* CREATED DATE     : 07-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcLevelSimCtrl_h
#define mrcLevelSimCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <subtask.h>
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
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class LevelSimCtrl: public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    LevelSimCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~LevelSimCtrl();
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
    void  SimulatePowerFlowPressure(float pit_level);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Config
    SubjectPtr<BoolDataPoint*>  mpPitLevelSimEnabled;
    SubjectPtr<FloatDataPoint*> mpPitSurfaceArea;
    SubjectPtr<FloatDataPoint*> mpPitDepth;

    // Input
    SubjectPtr<FloatDataPoint*> mpSimLevelInflow;
    SubjectPtr<EnumDataPoint<ACTUAL_OPERATION_MODE_TYPE>*> mpPumpOprMode[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpVfdOutputFrequency[NO_OF_PUMPS];

    // Output
    SubjectPtr<FloatDataPoint*> mpLevelSim;
    SubjectPtr<FloatDataPoint*> mpSimLevelInflowInPercent;
    SubjectPtr<FloatDataPoint*> mpMeasuredValuePowerPump[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpMeasuredValueFlow;
    SubjectPtr<FloatDataPoint*> mpPumpFlow[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> mpMeasuredValueOutletPressure;

    int mLevelSimCtrlLogCounter; 

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif
