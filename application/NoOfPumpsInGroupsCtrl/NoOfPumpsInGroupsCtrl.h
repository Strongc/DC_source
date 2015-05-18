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
/* CLASS NAME       : NoOfPumpsInGroupsCtrl                                 */
/*                                                                          */
/* FILE NAME        : NoOfPumpsInGroupsCtrl.h                               */
/*                                                                          */
/* CREATED DATE     : 06-04-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcNoOfPumpsInGroupsCtrl_h
#define mrcNoOfPumpsInGroupsCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Observer.h>
#include <SubTask.h>
#include <U8DataPoint.h>
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
class NoOfPumpsInGroupsCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    NoOfPumpsInGroupsCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~NoOfPumpsInGroupsCtrl();
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

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U8DataPoint*> mpNoOfPumps;
    SubjectPtr<U8DataPoint*> mpGroupOfPump[NO_OF_PUMPS];

    SubjectPtr<U32DataPoint*> mpNoOfPumpsInGroup[NO_OF_PUMP_GROUPS];
    SubjectPtr<U8DataPoint*> mpNoOfGroupsUsed;

    SubjectPtr<U32DataPoint*> mpMinStartedPumps[NO_OF_PUMP_GROUPS];
    SubjectPtr<U32DataPoint*> mpMaxStartedPumps[NO_OF_PUMP_GROUPS];
    SubjectPtr<U32DataPoint*> mpTotalMinStartedPumps;
    SubjectPtr<U32DataPoint*> mpTotalMaxStartedPumps;

    SubjectPtr<BoolDataPoint*> mpPumpGroupsEnabled;
    SubjectPtr<U32DataPoint*>   mpFirstPumpInGroup2;

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
