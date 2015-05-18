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
/* CLASS NAME       : FloatSwitchCnfCtrl                                    */
/*                                                                          */
/* FILE NAME        : FloatSwitchCnfCtrl.h                                  */
/*                                                                          */
/* CREATED DATE     : 24-07-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcFloatSwitchCnfCtrl_h
#define mrcFloatSwitchCnfCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include "cu351_cpu_types.h"
#include <Observer.h>
#include <SubTask.h>
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <U8DataPoint.h>

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
class FloatSwitchCnfCtrl : public SubTask, public Observer
{
  public:
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

    static FloatSwitchCnfCtrl* GetInstance(); // returns the pointer to the singleton

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FloatSwitchCnfCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FloatSwitchCnfCtrl();

    /********************************************************************
    OPERATIONS
    ********************************************************************/

    void SetTemporaryFloatSwitchConfiguration(void);
    void TransferTemporaryFloatSwitchConfiguration(void);
    void HandleOverflowSwitch(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static FloatSwitchCnfCtrl* mpInstance;

    SubjectPtr<BoolDataPoint*> mpFloatSwitchInputMoved;

    SubjectPtr<U8DataPoint*> mpNoOfPumps;
    SubjectPtr<U8DataPoint*> mpNoOfFloatSwitches;
    SubjectPtr<U8DataPoint*> mpFloatSwitchConfigNumber;

    SubjectPtr<U8DataPoint*> mpCurrentFloatSwitchConfigNumber;
    SubjectPtr<U8DataPoint*> mpCurrentNoOfFloatSwitches;
    SubjectPtr<BoolDataPoint*> mpOverflowSwitchInstalled;

    SubjectPtr<BoolDataPoint*> mpFloatSwitchExecuteConfig;

    SubjectPtr<EnumDataPoint<FSW_TYPE>*> mpFloatSwitchCnf[MAX_NO_OF_FLOAT_SWITCHES];
    SubjectPtr<EnumDataPoint<FSW_TYPE>*> mpFloatSwitchTmpCnf[MAX_NO_OF_FLOAT_SWITCHES];

    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mpDiConfFloatSwitch[MAX_NO_OF_FLOAT_SWITCHES];

    SubjectPtr<EnumDataPoint<SENSOR_TYPE_TYPE>*> mpPitLevelCtrlType;

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
