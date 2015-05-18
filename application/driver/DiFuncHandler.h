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
/* CLASS NAME       : DiFuncHandler                                         */
/*                                                                          */
/* FILE NAME        : DiFuncHandler.h                                       */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Tests digital inputs and sets digital functions */
/*                          according to configuration.                     */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __DI_FUNC_HANDLER_H__
#define __DI_FUNC_HANDLER_H__

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
#include <U16DataPoint.h>
#include <U32DataPoint.h>
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
 * CLASS: DiFuncHandler
 * DESCRIPTION:
 *
 *****************************************************************************/
class DiFuncHandler : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    DiFuncHandler();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~DiFuncHandler();
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
    void UpdateInputCounters();

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mpDiConf[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<U32DataPoint*> mpConfLogic;
    SubjectPtr<BoolDataPoint*> mpInputConfLogic[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<U16DataPoint*> mpIO351DigInStatusBits[3];  // digital input status bits from IO351 IO modules
    SubjectPtr<U32DataPoint*> mpDigInStatusBits;
    SubjectPtr<BoolDataPoint*> mpDigInLevel[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<U32DataPoint*> mpDiFuncInput[NO_OF_DIGITAL_INPUT_FUNC];
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpDiFuncState[NO_OF_DIGITAL_INPUT_FUNC];
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpDigInState[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_STATE_TYPE>*> mpDigInLogicState[MAX_NO_OF_DIG_INPUTS];

    // Counter inputs
    SubjectPtr<U32DataPoint*> mpDigInCount[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<U32DataPoint*> mpDiFuncCount[NO_OF_COUNTER_INPUT_FUNC];

    // Bool to indicate that DiFuncHandler has handled a reconfiguration af float switch inputs
    SubjectPtr<BoolDataPoint*> mpFloatSwitchInputMoved;

    U32 mDigInBuffer;
    U32 mOldDigInBuffer;
    bool mDiConfChanged;
    bool mInputConfLogicChanged;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
