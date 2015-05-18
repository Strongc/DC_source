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
/* FILE NAME        : LevelSimCtrl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 07-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <LevelSimCtrl.h>
#include <IobComDrv.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

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
LevelSimCtrl::LevelSimCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
LevelSimCtrl::~LevelSimCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelSimCtrl::InitSubTask()
{
  mpLevelSim->SetValue(0.0f);
  mLevelSimCtrlLogCounter = 1000;
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 * Calculates new simulated pit level. Has to be called with sample time 1s
 *****************************************************************************/
void LevelSimCtrl::RunSubTask()
{
  if (mpPitLevelSimEnabled->GetValue() == true && mpSimLevelInflow->GetQuality() == DP_AVAILABLE)
  {
    float level = mpLevelSim->GetValue();

    SimulatePowerFlowPressure(level);

    float pit_area = mpPitSurfaceArea->GetValue();
    if (pit_area <= 0.0f)
    {
      pit_area = 1.0f;
    }
    for (int i = 0; i < NO_OF_PUMPS; i++)
    {
      level -= mpPumpFlow[i]->GetValue()/pit_area;
    }

    float in_flow = mpSimLevelInflow->GetValueAsPercent();
    mpSimLevelInflowInPercent->SetValue(in_flow);
    level += in_flow/100.0f * mpPitDepth->GetValue()/60.0f; // Fill up in 60 seconds at max in flow

    mpLevelSim->SetValue(level);
  }
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION: Subscribe to subjects.
 *
 *****************************************************************************/
void LevelSimCtrl::ConnectToSubjects()
{
}


/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void LevelSimCtrl::Update(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void LevelSimCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: If the id is equal to a id for a subject observed.
 * Then take a copy of pSubjet to the member pointer for this subject.
 *
 *****************************************************************************/
void LevelSimCtrl::SetSubjectPointer(int id, Subject* pSubject)
{
  switch (id)
  {
    // Config:
    case SP_LSC_PIT_LEVEL_SIM_ENABLED:
      mpPitLevelSimEnabled.Attach(pSubject);
      break;
    case SP_LSC_PIT_SURFACE_AREA:
      mpPitSurfaceArea.Attach(pSubject);
      break;
    case SP_LSC_PIT_DEPTH:
      mpPitDepth.Attach(pSubject);
      break;

    // Input
    case SP_LSC_SIM_LEVEL_INFLOW:
      mpSimLevelInflow.Attach(pSubject);
      break;
    case SP_LSC_PUMP_1_OPR_MODE:
      mpPumpOprMode[PUMP_1].Attach(pSubject);
      break;
    case SP_LSC_PUMP_2_OPR_MODE:
      mpPumpOprMode[PUMP_2].Attach(pSubject);
      break;
    case SP_LSC_PUMP_3_OPR_MODE:
      mpPumpOprMode[PUMP_3].Attach(pSubject);
      break;
    case SP_LSC_PUMP_4_OPR_MODE:
      mpPumpOprMode[PUMP_4].Attach(pSubject);
      break;
    case SP_LSC_PUMP_5_OPR_MODE:
      mpPumpOprMode[PUMP_5].Attach(pSubject);
      break;
    case SP_LSC_PUMP_6_OPR_MODE:
      mpPumpOprMode[PUMP_6].Attach(pSubject);
      break;
    case SP_LSC_VFD_1_OUTPUT_FREQUENCY:
      mpVfdOutputFrequency[PUMP_1].Attach(pSubject);
      break;
    case SP_LSC_VFD_2_OUTPUT_FREQUENCY:
      mpVfdOutputFrequency[PUMP_2].Attach(pSubject);
      break;
    case SP_LSC_VFD_3_OUTPUT_FREQUENCY:
      mpVfdOutputFrequency[PUMP_3].Attach(pSubject);
      break;
    case SP_LSC_VFD_4_OUTPUT_FREQUENCY:
      mpVfdOutputFrequency[PUMP_4].Attach(pSubject);
      break;
    case SP_LSC_VFD_5_OUTPUT_FREQUENCY:
      mpVfdOutputFrequency[PUMP_5].Attach(pSubject);
      break;
    case SP_LSC_VFD_6_OUTPUT_FREQUENCY:
      mpVfdOutputFrequency[PUMP_6].Attach(pSubject);
      break;

    // Output
    case SP_LSC_LEVEL_SIM:
      mpLevelSim.Attach(pSubject);
      break;
    case SP_LSC_SIM_LEVEL_INFLOW_IN_PERCENT:
      mpSimLevelInflowInPercent.Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_POWER_PUMP_1:
      mpMeasuredValuePowerPump[PUMP_1].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_POWER_PUMP_2:
      mpMeasuredValuePowerPump[PUMP_2].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_POWER_PUMP_3:
      mpMeasuredValuePowerPump[PUMP_3].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_POWER_PUMP_4:
      mpMeasuredValuePowerPump[PUMP_4].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_POWER_PUMP_5:
      mpMeasuredValuePowerPump[PUMP_5].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_POWER_PUMP_6:
      mpMeasuredValuePowerPump[PUMP_6].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_FLOW:
      mpMeasuredValueFlow.Attach(pSubject);
      break;
    case SP_LSC_PUMP_1_FLOW:
      mpPumpFlow[PUMP_1].Attach(pSubject);
      break;
    case SP_LSC_PUMP_2_FLOW:
      mpPumpFlow[PUMP_2].Attach(pSubject);
      break;
    case SP_LSC_PUMP_3_FLOW:
      mpPumpFlow[PUMP_3].Attach(pSubject);
      break;
    case SP_LSC_PUMP_4_FLOW:
      mpPumpFlow[PUMP_4].Attach(pSubject);
      break;
    case SP_LSC_PUMP_5_FLOW:
      mpPumpFlow[PUMP_5].Attach(pSubject);
      break;
    case SP_LSC_PUMP_6_FLOW:
      mpPumpFlow[PUMP_6].Attach(pSubject);
      break;
    case SP_LSC_MEASURED_VALUE_OUTLET_PRESSURE:
      mpMeasuredValueOutletPressure.Attach(pSubject);
      break;

    default:
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
 * Function - SimulatePowerFlowPressure
 * DESCRIPTION: Simulation of power and flow and pressure.
 *              For development+demo only !!!
 *
 *****************************************************************************/
void LevelSimCtrl::SimulatePowerFlowPressure(float pit_level)
{
  // These parameters are simulating a system with pumps, pipes and a pit:
  // f = 50Hz --> approx: Q = 30m3/h, H = 8m, P = 3350W, E = 112W/m3
  // If the average lift height is less than 8m, the economy frequency is below 50Hz e.g. 6m --> 43Hz, E = 85W/m3
  static float Jp = 20000000.0f;
  static float Kp = 500000000.0f;
  static float ah2 = -2640540.0f;
  static float ah1 = -5266.7f;
  static float ah0 = 1.273f;
  static float aP2 = -329.38f;
  static float aP1 = 0.16907f;
  static float aP0 = 0.00010406f;
  static float lift_level = 0.0f;
  static float level_to_pressure_factor = 9806.7098f;

  static float pump_flow[NO_OF_PUMPS] = {0.0f};
  static float total_flow = 0.0f;
  float omega, h_loss, h_geo, h_out, h_pump, delta_flow, pump_power, h_pump_max;

  h_pump_max = 0.0f;

  h_loss = Kp*total_flow*total_flow;
  h_geo = (mpPitDepth->GetValue() + lift_level - pit_level) * level_to_pressure_factor;
  h_out = h_geo+h_loss;

  total_flow = 0.0f;
  for (int i=0; i<NO_OF_PUMPS; i++)
  {
    omega = 0.0f;
    if (mpVfdOutputFrequency[i]->GetQuality() == DP_AVAILABLE)
    {
      omega = mpVfdOutputFrequency[i]->GetValue() * 2.0f*3.14f;
    }
    else if (mpPumpOprMode[i]->GetValue() == ACTUAL_OPERATION_MODE_STARTED)
    {
      omega = 50.0f * 2.0f*3.14f;
    }

    h_pump = ah2*pump_flow[i]*pump_flow[i] + ah1*pump_flow[i]*omega + ah0*omega*omega;

    if (h_pump > h_pump_max)
    {
      h_pump_max = h_pump;
    }

    delta_flow = (h_pump - h_out)/Jp;
    pump_flow[i] += delta_flow;
    if (pump_flow[i] < 0.0f)
    {
      pump_flow[i] = 0.0f;
    }
    mpPumpFlow[i]->SetValue(pump_flow[i]);
    total_flow += pump_flow[i];

    pump_power = aP2*omega*pump_flow[i]*pump_flow[i] + aP1*omega*omega*pump_flow[i] + aP0*omega*omega*omega;
    mpMeasuredValuePowerPump[i]->SetValue(pump_power);
  }

  float p_out = pit_level * level_to_pressure_factor + h_pump_max;

  // Make the simulated outlet pressure and system flow available via 0-10V simulation of analog sensor 2 and 3.
  // This corresponds to ana_in_2 and ana_in_4 on the io board.
  IobComDrv* pComDrv = IobComDrv::GetInstance();
  pComDrv->SetInputSimulationFlag();
  pComDrv->SetInputSimulationMode((1<<2)+(1<<4)); // Enable simulation of ana_in_2+4 on io board
  
  // Pressure scaling: 10V --> 2.0bar (/100000 internal Pa-->bar. 10V = 884.09 ad-units)
  pComDrv->SetAnalogInputSimulationValue(p_out/100000.0f/2.0f*884.09f+0.5f, IOB_ANA_IN_NO_2);

  // Flow scaling: 10V --> 99m3/h (*3600 internal m3/s-->m3/h. 10V = 884.09 ad-units)
  pComDrv->SetAnalogInputSimulationValue(total_flow*3600.0f/99.0f*884.09f+0.5f, IOB_ANA_IN_NO_4);

  #ifdef __PC__
  FloatDataPoint* estimatedFlow = dynamic_cast<FloatDataPoint*>(GetSubject(4129));

  if (estimatedFlow->IsAvailable() && (estimatedFlow->GetValue() > 0 || total_flow > 0))
  {
    FILE* file;
    char* filename_a = "LevelSimCtrl_a.csv";
    char* filename_b = "LevelSimCtrl_b.csv";

    char* p_filename = filename_a;

    if (mLevelSimCtrlLogCounter >= 3000)
    {
      mLevelSimCtrlLogCounter = 1000;
    }

    if (mLevelSimCtrlLogCounter >= 2000)
    {
      p_filename = filename_b;
    }
     
    if (mLevelSimCtrlLogCounter % 1000 == 0)
    {
      file = fopen(p_filename,"w");
    }
    else
    {
      file = fopen(p_filename,"a");
    }

    mLevelSimCtrlLogCounter++;

    if (file != 0)
    {
      char log_line[150];

      int pos = _snprintf(log_line, 25, "Frequency;%f;", mpVfdOutputFrequency[0]->GetValue());
      pos += _snprintf(log_line + pos, 25, " Power;%f;", mpMeasuredValuePowerPump[0]->GetValue());
      pos += _snprintf(log_line + pos, 25, " Sim-Flow;%f;", total_flow);
      pos += _snprintf(log_line + pos, 25, " Est-Flow;%f;", estimatedFlow->GetValue());
      pos += _snprintf(log_line + pos, 25, " p_out;%f;", p_out);
      _snprintf(log_line + pos, 25, " Level;%f;\n", pit_level);
     
      //replace . with ,
      for(int i = 0; i < 6; i++)
      {
        char* p_pos = strstr(log_line, ".");
        if (p_pos > 0)
        {
          *p_pos = ',';
        }
      }
      
      fprintf(file, "%s", log_line); 

      fclose(file);
    }
  }
  #endif //__PC__

}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
