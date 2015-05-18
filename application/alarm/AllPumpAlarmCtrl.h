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
/* CLASS NAME       : AllPumpAlarmCtrl                                      */
/*                                                                          */
/* FILE NAME        : AllPumpAlarmCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 15-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Compares the pump alarm status with the number  */
/*                          of available pumps to decide if all pumps are   */
/*                          in alarm.                                       */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAllPumpAlarmCtrl_h
#define mrcAllPumpAlarmCtrl_h

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
class AllPumpAlarmCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AllPumpAlarmCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AllPumpAlarmCtrl();
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

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    // Input:
    SubjectPtr<BoolDataPoint*> mpPumpAlarmFlag[NO_OF_PUMPS];
    SubjectPtr<BoolDataPoint*> mpPumpAvailableFlag[NO_OF_PUMPS];

    // Output:
    SubjectPtr<BoolDataPoint*> mpAllPumpAlarmFlag;
    SubjectPtr<BoolDataPoint*> mpAnyPumpAlarmFlag;

    // Class parameters
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
