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
/* CLASS NAME       : AnaOutCtrl                                            */
/*                                                                          */
/* FILE NAME        : AnaOutCtrl.h                                          */
/*                                                                          */
/* CREATED DATE     : 2008-09-09                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Sets analog outputs and connects them to        */
/*                          analog functions according to configuration.    */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ANA_OUT_CTRL_H__
#define __ANA_OUT_CTRL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AppTypeDefs.h>
#include <Observer.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>
#include <GeniSlaveIf.h>

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
 * CLASS: AnaOutCtrl
 * DESCRIPTION:
 *
 *****************************************************************************/
class AnaOutCtrl : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AnaOutCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~AnaOutCtrl();

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
    void SetSubjectPointer(int Id, Subject* pSubject);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void HandleFuncChange(bool UpdateAllConfigurations);
    void CalculateOutput(void);
    void CopyOutput(void);
    void DisableRangeDatapoints(U8 AnalogOutputIndex);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<FloatDataPoint*> mpAnaOutInputValue[NO_OF_ANA_OUT_FUNC];
    
    SubjectPtr<EnumDataPoint<ANA_OUT_FUNC_TYPE>*> mpConfAnaOutFunc[MAX_NO_OF_ANA_OUTPUTS];
    SubjectPtr<FloatDataPoint*> mpConfAnaOutMin[MAX_NO_OF_ANA_OUTPUTS];
    SubjectPtr<FloatDataPoint*> mpConfAnaOutMax[MAX_NO_OF_ANA_OUTPUTS];

    SubjectPtr<FloatDataPoint*> mpAnaOutElectricalValue[MAX_NO_OF_ANA_OUTPUTS];
    SubjectPtr<FloatDataPoint*> mpAnaOutOutputValue[MAX_NO_OF_ANA_OUTPUTS];
    SubjectPtr<IIntegerDataPoint*> mpNoOfIo351;


    SubjectPtr<BoolDataPoint*>  mpAnaOutSetupFromGeniFlag;

    GeniSlaveIf* mpGeniSlaveIf;

    bool mAnaOutFuncChanged;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
