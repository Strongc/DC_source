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
/* CLASS NAME       : AnyAvailableCtrl                                      */
/*                                                                          */
/* FILE NAME        : AnyAvailableCtrl.h                                    */
/*                                                                          */
/* CREATED DATE     : 25-05-2009  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcAnyAvailableCtrl_h
#define mpcAnyAvailableCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <BoolDataPoint.h>
#include <SubTask.h>
#include <vector>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_AAC_TASK_TYPE = 0,
  AAC_TASK_TYPE_EVENT = FIRST_AAC_TASK_TYPE,
  AAC_TASK_TYPE_PERIODIC,
  AAC_TASK_TYPE_LOW_PRIO,
  // insert new items above
  NO_OF_AAC_TASK_TYPE,
  LAST_AAC_TASK_TYPE = NO_OF_AAC_TASK_TYPE - 1
} AAC_TASK_TYPE_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class AnyAvailableCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AnyAvailableCtrl(AAC_TASK_TYPE_TYPE taskType);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AnyAvailableCtrl(void);

    /********************************************************************
    ASSIGNMENT OPERATORS
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void RunSubTask(void);
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int Id, Subject* pSubject);
    void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    std::vector<DataPoint*> mSourceList;
    SubjectPtr<BoolDataPoint*> mpOutput;

    bool mReqTaskTimeFlag;
    AAC_TASK_TYPE_TYPE mTaskType;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

