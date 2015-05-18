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
/* CLASS NAME       : IobMonitor                                            */
/*                                                                          */
/* FILE NAME        : IobMonitor.cpp                                        */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <ErrorLog.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <IobMonitor.h>
#include <IobComDrv.h>
#include <FactoryTypes.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define TIME_TO_SCAN 10
#define AV_REF 4.1f

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
IobMonitor::IobMonitor()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
IobMonitor::~IobMonitor()
{

}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobMonitor::InitSubTask(void)
{
  mRunCount = 0;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobMonitor::RunSubTask(void)
{
  IobComDrv* pComDrv = IobComDrv::GetInstance();

  if (mRunCount >= TIME_TO_SCAN)
  {
    mRunCount = 0;
    mpAnaIn0->SetValue((pComDrv->GetAnalogInput(IOB_ANA_IN_NO_0)) << 6);
    mpAnaIn1->SetValue((pComDrv->GetAnalogInput(IOB_ANA_IN_NO_1)) << 6);
    mpAnaIn2->SetValue((pComDrv->GetAnalogInput(IOB_ANA_IN_NO_2)) << 6);
    mpAnaIn3->SetValue((pComDrv->GetAnalogInput(IOB_ANA_IN_NO_3)) << 6);
    mpAnaIn4->SetValue((pComDrv->GetAnalogInput(IOB_ANA_IN_NO_4)) << 6);
    mpAnaIn5->SetValue((pComDrv->GetAnalogInput(IOB_ANA_IN_NO_5)) << 6);
    mpSimStatus->SetValue(pComDrv->GetIOSimulationStatus());

    U8 dig_in;
    pComDrv->GetDigitalInputs(&dig_in);
    mpIobBoardId->SetValue((IOB_BOARD_ID_TYPE)(dig_in >> 3)); // Board ID is placed on bit 3 and 4.
    mpIobTemperature->SetValue(ScaleTemperature(pComDrv->GetAnalogInput(IOB_ANA_IN_NO_6)));
    mpIobPressure->SetValue(ScalePressure(mpIobAiPressureRaw->GetValue()));
    mpIobBatteryVoltage->SetValue(ScaleBatteryVoltage(pComDrv->GetAnalogInput(IOB_ANA_IN_NO_7)));
  }
  mRunCount++;

  if(mpSimEnable.IsUpdated())
  {
    pComDrv->SetInputSimulationFlag();
  }
  if(mpSimDisable.IsUpdated())
  {
    pComDrv->ClearInputSimulationFlag();
  }
  if(mpSimMode.IsUpdated())
  {
    pComDrv->SetInputSimulationMode(mpSimMode->GetValue());
  }
  if(mpSimDigIn.IsUpdated())
  {
    pComDrv->SetDigitalInputSimulationValue(mpSimDigIn->GetValue());
  }
  if(mpSimValueAD0.IsUpdated())
  {
    pComDrv->SetAnalogInputSimulationValue(mpSimValueAD0->GetValue(), IOB_ANA_IN_NO_0);
  }
  if(mpSimValueAD1.IsUpdated())
  {
    pComDrv->SetAnalogInputSimulationValue(mpSimValueAD1->GetValue(), IOB_ANA_IN_NO_1);
  }
  if(mpSimValueAD2.IsUpdated())
  {
    pComDrv->SetAnalogInputSimulationValue(mpSimValueAD2->GetValue(), IOB_ANA_IN_NO_2);
  }
  if(mpSimValueAD3.IsUpdated())
  {
    pComDrv->SetAnalogInputSimulationValue(mpSimValueAD3->GetValue(), IOB_ANA_IN_NO_3);
  }
  if(mpSimValueAD4.IsUpdated())
  {
    pComDrv->SetAnalogInputSimulationValue(mpSimValueAD4->GetValue(), IOB_ANA_IN_NO_4);
  }
  if(mpSimValueAD5.IsUpdated())
  {
    pComDrv->SetAnalogInputSimulationValue(mpSimValueAD5->GetValue(), IOB_ANA_IN_NO_5);
  }

}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void IobMonitor::Update(Subject* pSubject)
{
  if (mpDigIn.Update(pSubject)) return;
  if (mpSimEnable.Update(pSubject)) return;
  if (mpSimDisable.Update(pSubject)) return;
  if (mpSimMode.Update(pSubject)) return;
  if (mpSimDigIn.Update(pSubject)) return;
  if (mpSimValueAD0.Update(pSubject)) return;
  if (mpSimValueAD1.Update(pSubject)) return;
  if (mpSimValueAD2.Update(pSubject)) return;
  if (mpSimValueAD3.Update(pSubject)) return;
  if (mpSimValueAD4.Update(pSubject)) return;
  if (mpSimValueAD5.Update(pSubject)) return;
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobMonitor::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobMonitor::ConnectToSubjects(void)
{
  mpDigIn.Subscribe(this);
  mpSimEnable.Subscribe(this);
  mpSimDisable.Subscribe(this);
  mpSimMode.Subscribe(this);
  mpSimDigIn.Subscribe(this);
  mpSimValueAD0.Subscribe(this);
  mpSimValueAD1.Subscribe(this);
  mpSimValueAD2.Subscribe(this);
  mpSimValueAD3.Subscribe(this);
  mpSimValueAD4.Subscribe(this);
  mpSimValueAD5.Subscribe(this);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void IobMonitor::SetSubjectPointer(int id, Subject* pSubject)
{
  switch(id)
  {
    case SP_IOBM_DIG_IN:
      mpDigIn.Attach(pSubject);
      break;
    case SP_IOBM_ANA_IN_0:
      mpAnaIn0.Attach(pSubject);
      break;
    case SP_IOBM_ANA_IN_1:
      mpAnaIn1.Attach(pSubject);
      break;
    case SP_IOBM_ANA_IN_2:
      mpAnaIn2.Attach(pSubject);
      break;
    case SP_IOBM_ANA_IN_3:
      mpAnaIn3.Attach(pSubject);
      break;
    case SP_IOBM_ANA_IN_4:
      mpAnaIn4.Attach(pSubject);
      break;
    case SP_IOBM_ANA_IN_5:
      mpAnaIn5.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ENABLE:
      mpSimEnable.Attach(pSubject);
      break;
    case SP_IOBM_SIM_DISABLE:
      mpSimDisable.Attach(pSubject);
      break;
    case SP_IOBM_SIM_MODE:
      mpSimMode.Attach(pSubject);
      break;
    case SP_IOBM_SIM_STATUS:
      mpSimStatus.Attach(pSubject);
      break;
    case SP_IOBM_SIM_DIG_IN:
      mpSimDigIn.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ANA_IN_0:
      mpSimValueAD0.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ANA_IN_1:
      mpSimValueAD1.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ANA_IN_2:
      mpSimValueAD2.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ANA_IN_3:
      mpSimValueAD3.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ANA_IN_4:
      mpSimValueAD4.Attach(pSubject);
      break;
    case SP_IOBM_SIM_ANA_IN_5:
      mpSimValueAD5.Attach(pSubject);
      break;
    case SP_IOBM_BOARD_ID:
      mpIobBoardId.Attach(pSubject);
      break;
    case SP_IOBM_TEMPERATURE:
      mpIobTemperature.Attach(pSubject);
      break;
    case SP_IOBM_PRESSURE:
      mpIobPressure.Attach(pSubject);
      break;
    case SP_IOBM_BATTERY_VOLTAGE:
      mpIobBatteryVoltage.Attach(pSubject);
      break;
    case SP_IOBM_AI_PRESSURE_RAW:
      mpIobAiPressureRaw.Attach(pSubject);
      break;
  }
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - ScaleTemperature
 * DESCRIPTION:
 *
 *****************************************************************************/
float IobMonitor::ScaleTemperature(U32 val)
{
  #define T_GAIN -0.01177f // -11,77mV/°C
  #define T_OFFSET 1.8639f // offset is 1863.9mV

  float temp, v_in;

  v_in = (val / 1023.0f) * AV_REF;
  temp = (v_in - T_OFFSET) / T_GAIN;

  return temp;
}


/*****************************************************************************
 * Function - ScalePressure
 * DESCRIPTION:
 *
 *****************************************************************************/
float IobMonitor::ScalePressure(U32 val)
{
  #define P_GAIN 20625.020625f
  #define P_OFFSET 30625.0f

  float press, v_in;

  v_in = (val / 1023.0f) * AV_REF;
  press = (v_in * P_GAIN) + P_OFFSET;

  return press;
}

/*****************************************************************************
 * Function - ScaleBatteryVoltage
 * DESCRIPTION:
 *
 *****************************************************************************/
float IobMonitor::ScaleBatteryVoltage(U32 val)
{
  #define V_GAIN (100.0f / (100.0f + 470.0f)) // Voltage divider are a 100K resistor and a 470K resistor
  #define V_OFFSET 1.24f // offset is 1,24V

  float v_bat, v_in;

  v_in = (val / 1023.0f) * AV_REF;

  v_bat = ( (v_in - V_OFFSET) / V_GAIN ) + V_OFFSET;

  return v_bat;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
