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
/* CLASS NAME       : GeniAppTestMode                                       */
/*                                                                          */
/* FILE NAME        : GeniAppTestMode.h                                     */
/*                                                                          */
/* CREATED DATE     : 04-03-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcGeniAppTestMode_h
#define mrcGeniAppTestMode_h

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
#include <EventDataPoint.h>
#include <EnumDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>
#include <BoolDataPoint.h>
#include <SoftwareVersion.h>

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
class GeniAppTestMode : public SubTask, public Observer
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static GeniAppTestMode* GetInstance();

    void InitSubTask();
    void RunSubTask();
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects();
    void SetSubjectPointer(int id, Subject* pSubject);

    bool GetTestMode(void);

  private:
    /********************************************************************
    LIFECYCLE - Default constructor. Private because it is a Singleton
    ********************************************************************/
    GeniAppTestMode();
    /********************************************************************
    LIFECYCLE - Destructor. Private because it is a Singleton
    ********************************************************************/
    ~GeniAppTestMode();
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void EnableTestMode(bool new_mode);
    void HandleGeniTestData(void);
    void ClearGeniTestData(void);
    void HandleIOSimulation(void);
    void HandleTestConfig(U8 new_config);
    void GetTestStatus(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static GeniAppTestMode* mInstance;

    // Test mode
    SubjectPtr<EventDataPoint*> mpTestModeEnable;
    SubjectPtr<EventDataPoint*> mpTestModeDisable;
    SubjectPtr<BoolDataPoint*>  mpTestModeStatus;

    // Firmware Update
    SubjectPtr<EnumDataPoint<FIRMWARE_UPDATE_STATE_TYPE>*> mpFirmwareUpdateState;

    // Display
    SubjectPtr<U32DataPoint*> mpDisplayBacklight;
    SubjectPtr<U32DataPoint*> mpDisplayContrast;

    // IO simulation
    SubjectPtr<EventDataPoint*> mpIOSimulationEnable;
    SubjectPtr<EventDataPoint*> mpIOSimulationDisable;
    SubjectPtr<U32DataPoint*> mpIOSimulationMode;
    SubjectPtr<U8DataPoint*> mpSimDigitalInputs;
    SubjectPtr<U32DataPoint*> mpSimValueAD0;
    SubjectPtr<U32DataPoint*> mpSimValueAD1;
    SubjectPtr<U32DataPoint*> mpSimValueAD2;
    SubjectPtr<U32DataPoint*> mpSimValueAD3;
    SubjectPtr<U32DataPoint*> mpSimValueAD4;
    SubjectPtr<U32DataPoint*> mpSimValueAD5;
    SubjectPtr<U32DataPoint*> mpIOSimulationStatus;

    SubjectPtr<EnumDataPoint<IOB_BOARD_ID_TYPE>*> mpIobBoardId;
    SubjectPtr<FloatDataPoint*> mpIobTemperature;
    SubjectPtr<FloatDataPoint*> mpIobPressure;
    SubjectPtr<FloatDataPoint*> mpIobBatteryVoltage;
    SubjectPtr<BoolDataPoint*> mpIobBusModulePressent;
    SubjectPtr<BoolDataPoint*> mpIobSupplyStatus;
    SubjectPtr<BoolDataPoint*> mpIobBatteryStatus;

    SubjectPtr<U32DataPoint*> mpAnalogInput0;
    SubjectPtr<U32DataPoint*> mpAnalogInput1;
    SubjectPtr<U32DataPoint*> mpAnalogInput2;
    SubjectPtr<U32DataPoint*> mpAnalogInput3;
    SubjectPtr<U32DataPoint*> mpAnalogInput4;
    SubjectPtr<U32DataPoint*> mpAnalogInput5;
    SubjectPtr<U32DataPoint*> mpDigitalInput;

    bool mTestModeEnabled;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
