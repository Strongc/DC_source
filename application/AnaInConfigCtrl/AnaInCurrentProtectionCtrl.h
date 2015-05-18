/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: CU 351 Platform                                  */
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
/* CLASS NAME       : AnaInMeasureValueCtrl                                 */
/*                                                                          */
/* FILE NAME        : AnaInMeasureValueCtrl.h                               */
/*                                                                          */
/* CREATED DATE     : 15-12-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
Protect against multiple inclusion through the use of guards:
****************************************************************************/
#ifndef __ANA_IN_CURRENT_PROTECTION_CTRL_H__
#define __ANA_IN_CURRENT_PROTECTION_CTRL_H__

/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <Subject.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
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
class AnaInCurrentProtectionCtrl : public Observer, public SubTask
{
public:
  /********************************************************************
  LIFECYCLE - Default constructor.
  ********************************************************************/
  AnaInCurrentProtectionCtrl();
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  ~AnaInCurrentProtectionCtrl();
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

  virtual void RunSubTask();
  virtual void InitSubTask(void);
private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/

  /********************************************************************
  ATTRIBUTE
  ********************************************************************/

  SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*> mpConfMeasuredValue;
  SubjectPtr<BoolDataPoint*> mpCurrentProtectionAllowed;

  bool mReqTaskTime;   //flag to control requesting of task time

protected:
  /********************************************************************
  OPERATIONS
  ********************************************************************/
  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
};
#endif
