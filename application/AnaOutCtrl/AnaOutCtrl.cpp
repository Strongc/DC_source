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
/* FILE NAME        : AnaOutCtrl.cpp                                        */
/*                                                                          */
/* CREATED DATE     : 2008-09-09                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <AnaOutCtrl.h>

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
AnaOutCtrl::AnaOutCtrl()
{
  mAnaOutFuncChanged = true;
  mpGeniSlaveIf = GeniSlaveIf::GetInstance();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
AnaOutCtrl::~AnaOutCtrl()
{
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION: Initialise AnaOutCtrl sub task
 *****************************************************************************/
void AnaOutCtrl::InitSubTask(void)
{
  for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
  {
    if (mpConfAnaOutFunc[i]->GetValue() == ANA_OUT_FUNC_NO_FUNCTION)
    {
      DisableRangeDatapoints(i);
    }
  }

  mpNoOfIo351.SetUpdated();
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::RunSubTask(void)
{
  if (mpNoOfIo351.IsUpdated())
  {
    HandleFuncChange(true);
  }

  if (mAnaOutFuncChanged)
  {
    mAnaOutFuncChanged = false;

    HandleFuncChange(false);
  }

  CalculateOutput();

  CopyOutput();
}

/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void AnaOutCtrl::Update(Subject* pSubject)
{
  for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
  {
    if (mpConfAnaOutFunc[i].Update(pSubject))
    {
      mAnaOutFuncChanged = true;
      break;
    }
  }

  mpNoOfIo351.Update(pSubject);

}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::SubscribtionCancelled(Subject* pSubject)
{
  /* Nothing to do because subjects are never destroyed */
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::ConnectToSubjects(void)
{
  //only few subscriptions are needed as AnaOutCtrl runs in a periodic task

  for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
  {
    mpConfAnaOutFunc[i]->Subscribe(this);
  }

  mpNoOfIo351->Subscribe(this);
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_AOC_NO_OF_IO_MODULES:
      mpNoOfIo351.Attach(pSubject);
      break;

    // Functions
    case SP_AOC_ANA_OUT_FUNC_NO_FUNCTION:
      mpAnaOutInputValue[ANA_OUT_FUNC_NO_FUNCTION].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_LEVEL:
      mpAnaOutInputValue[ANA_OUT_FUNC_LEVEL].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_VFD_1:
      mpAnaOutInputValue[ANA_OUT_FUNC_VFD_1].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_VFD_2:
      mpAnaOutInputValue[ANA_OUT_FUNC_VFD_2].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_VFD_3:
      mpAnaOutInputValue[ANA_OUT_FUNC_VFD_3].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_VFD_4:
      mpAnaOutInputValue[ANA_OUT_FUNC_VFD_4].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_VFD_5:
      mpAnaOutInputValue[ANA_OUT_FUNC_VFD_5].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_VFD_6:
      mpAnaOutInputValue[ANA_OUT_FUNC_VFD_6].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_USER_DEFINED_1:
      mpAnaOutInputValue[ANA_OUT_FUNC_USER_DEFINED_1].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_USER_DEFINED_2:
      mpAnaOutInputValue[ANA_OUT_FUNC_USER_DEFINED_2].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_USER_DEFINED_3:
      mpAnaOutInputValue[ANA_OUT_FUNC_USER_DEFINED_3].Attach(pSubject);
      break;
    case SP_AOC_ANA_OUT_FUNC_DOSING_PUMP:
      mpAnaOutInputValue[ANA_OUT_FUNC_DOSING_PUMP].Attach(pSubject);
      break;


    // Analog outputs configuration
    case SP_AOC_CONF_1:
      mpConfAnaOutFunc[0].Attach(pSubject);
      break;
    case SP_AOC_CONF_2:
      mpConfAnaOutFunc[1].Attach(pSubject);
      break;
    case SP_AOC_CONF_3:
      mpConfAnaOutFunc[2].Attach(pSubject);
      break;
    case SP_AOC_CONF_4:
      mpConfAnaOutFunc[3].Attach(pSubject);
      break;
    case SP_AOC_CONF_5:
      mpConfAnaOutFunc[4].Attach(pSubject);
      break;
    case SP_AOC_CONF_6:
      mpConfAnaOutFunc[5].Attach(pSubject);
      break;
    case SP_AOC_CONF_7:
      mpConfAnaOutFunc[6].Attach(pSubject);
      break;
    case SP_AOC_CONF_8:
      mpConfAnaOutFunc[7].Attach(pSubject);
      break;
    case SP_AOC_CONF_9:
      mpConfAnaOutFunc[8].Attach(pSubject);
      break;

    case SP_AOC_CONF_1_MIN:
      mpConfAnaOutMin[0].Attach(pSubject);
      break;
    case SP_AOC_CONF_2_MIN:
      mpConfAnaOutMin[1].Attach(pSubject);
      break;
    case SP_AOC_CONF_3_MIN:
      mpConfAnaOutMin[2].Attach(pSubject);
      break;
    case SP_AOC_CONF_4_MIN:
      mpConfAnaOutMin[3].Attach(pSubject);
      break;
    case SP_AOC_CONF_5_MIN:
      mpConfAnaOutMin[4].Attach(pSubject);
      break;
    case SP_AOC_CONF_6_MIN:
      mpConfAnaOutMin[5].Attach(pSubject);
      break;
    case SP_AOC_CONF_7_MIN:
      mpConfAnaOutMin[6].Attach(pSubject);
      break;
    case SP_AOC_CONF_8_MIN:
      mpConfAnaOutMin[7].Attach(pSubject);
      break;
    case SP_AOC_CONF_9_MIN:
      mpConfAnaOutMin[8].Attach(pSubject);
      break;

    case SP_AOC_CONF_1_MAX:
      mpConfAnaOutMax[0].Attach(pSubject);
      break;
    case SP_AOC_CONF_2_MAX:
      mpConfAnaOutMax[1].Attach(pSubject);
      break;
    case SP_AOC_CONF_3_MAX:
      mpConfAnaOutMax[2].Attach(pSubject);
      break;
    case SP_AOC_CONF_4_MAX:
      mpConfAnaOutMax[3].Attach(pSubject);
      break;
    case SP_AOC_CONF_5_MAX:
      mpConfAnaOutMax[4].Attach(pSubject);
      break;
    case SP_AOC_CONF_6_MAX:
      mpConfAnaOutMax[5].Attach(pSubject);
      break;
    case SP_AOC_CONF_7_MAX:
      mpConfAnaOutMax[6].Attach(pSubject);
      break;
    case SP_AOC_CONF_8_MAX:
      mpConfAnaOutMax[7].Attach(pSubject);
      break;
    case SP_AOC_CONF_9_MAX:
      mpConfAnaOutMax[8].Attach(pSubject);
      break;

    case SP_AOC_ELECTRICAL_VALUE_1:
      mpAnaOutElectricalValue[0].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_2:
      mpAnaOutElectricalValue[1].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_3:
      mpAnaOutElectricalValue[2].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_4:
      mpAnaOutElectricalValue[3].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_5:
      mpAnaOutElectricalValue[4].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_6:
      mpAnaOutElectricalValue[5].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_7:
      mpAnaOutElectricalValue[6].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_8:
      mpAnaOutElectricalValue[7].Attach(pSubject);
      break;
    case SP_AOC_ELECTRICAL_VALUE_9:
      mpAnaOutElectricalValue[8].Attach(pSubject);
      break;

    case SP_AOC_OUTPUT_VALUE_1:
      mpAnaOutOutputValue[0].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_2:
      mpAnaOutOutputValue[1].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_3:
      mpAnaOutOutputValue[2].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_4:
      mpAnaOutOutputValue[3].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_5:
      mpAnaOutOutputValue[4].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_6:
      mpAnaOutOutputValue[5].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_7:
      mpAnaOutOutputValue[6].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_8:
      mpAnaOutOutputValue[7].Attach(pSubject);
      break;
    case SP_AOC_OUTPUT_VALUE_9:
      mpAnaOutOutputValue[8].Attach(pSubject);
      break;

    case SP_AOC_ANA_OUT_SETUP_FROM_GENI_FLAG:
      mpAnaOutSetupFromGeniFlag.Attach(pSubject);
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
 * Function - HandleFuncChange
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::HandleFuncChange(bool UpdateAllConfigurations)
{
  int no_of_available_outputs = mpNoOfIo351->GetAsInt() * NO_OF_ANA_OUT_CHANNELS;

  for (int i = 0; i < MAX_NO_OF_ANA_OUTPUTS; i++)
  {
    if (UpdateAllConfigurations || mpConfAnaOutFunc[i].IsUpdated())
    {
      ANA_OUT_FUNC_TYPE func = mpConfAnaOutFunc[i]->GetValue();

      DP_QUALITY_TYPE output_quality;

      QUANTITY_TYPE quantity = mpAnaOutInputValue[func]->GetQuantity();
      switch (quantity)
      {
      case Q_FREQUENCY:
        mpConfAnaOutMin[i]->SetMinValue(0.0f);
        mpConfAnaOutMin[i]->SetMaxValue(10000.0f);
        mpConfAnaOutMin[i]->SetQuantity(Q_FREQUENCY);
        mpConfAnaOutMin[i]->SetQuality(DP_AVAILABLE);

        mpConfAnaOutMax[i]->SetMinValue(0.0f);
        mpConfAnaOutMax[i]->SetMaxValue(10000.0f);
        mpConfAnaOutMax[i]->SetQuantity(Q_FREQUENCY);
        mpConfAnaOutMax[i]->SetQuality(DP_AVAILABLE);

        if (mpAnaOutSetupFromGeniFlag->GetValue() == false
          && !UpdateAllConfigurations)
        {
          // the change is not arisen from GENI, so set min. + max.
          mpConfAnaOutMin[i]->SetValue(0.0f);  //  0 Hz
          mpConfAnaOutMax[i]->SetValue(50.0f); // 50 Hz
        }

        output_quality = DP_AVAILABLE;
        break;

      case Q_HEIGHT:
        mpConfAnaOutMin[i]->SetMinValue(0.0f);
        mpConfAnaOutMin[i]->SetMaxValue(10000.0f);
        mpConfAnaOutMin[i]->SetQuantity(Q_HEIGHT);
        mpConfAnaOutMin[i]->SetQuality(DP_AVAILABLE);

        mpConfAnaOutMax[i]->SetMinValue(0.0f);
        mpConfAnaOutMax[i]->SetMaxValue(10000.0f);
        mpConfAnaOutMax[i]->SetQuantity(Q_HEIGHT);
        mpConfAnaOutMax[i]->SetQuality(DP_AVAILABLE);

        if (mpAnaOutSetupFromGeniFlag->GetValue() == false
          && !UpdateAllConfigurations)
        {
          // the change is not arisen from GENI, so set min. + max.
          mpConfAnaOutMin[i]->SetValue(0.0f); // 0 m
          mpConfAnaOutMax[i]->SetValue(5.0f); // 5 m
        }


        output_quality = DP_AVAILABLE;
        break;

      case Q_PERCENT:
        mpConfAnaOutMin[i]->SetMinValue(0.0f);
        mpConfAnaOutMin[i]->SetMaxValue(10000.0f);
        mpConfAnaOutMin[i]->SetQuantity(Q_PERCENT);
        mpConfAnaOutMin[i]->SetQuality(DP_AVAILABLE);

        mpConfAnaOutMax[i]->SetMinValue(0.0f);
        mpConfAnaOutMax[i]->SetMaxValue(10000.0f);
        mpConfAnaOutMax[i]->SetQuantity(Q_PERCENT);
        mpConfAnaOutMax[i]->SetQuality(DP_AVAILABLE);

        if (mpAnaOutSetupFromGeniFlag->GetValue() == false
          && !UpdateAllConfigurations)
        {
          // the change is not arisen from GENI, so set min. + max.
          mpConfAnaOutMin[i]->SetValue(0.0f); // 0 %
          mpConfAnaOutMax[i]->SetValue(100.0f); // 100 %
        }

        output_quality = DP_AVAILABLE;
        break;

      case Q_FLOW:
        mpConfAnaOutMin[i]->SetMinValue(0.0f);
        mpConfAnaOutMin[i]->SetMaxValue(0.00027778f);
        mpConfAnaOutMin[i]->SetQuantity(Q_FLOW);
        mpConfAnaOutMin[i]->SetQuality(DP_AVAILABLE);

        mpConfAnaOutMax[i]->SetMinValue(0.0f);
        mpConfAnaOutMax[i]->SetMaxValue(0.00027778f);
        mpConfAnaOutMax[i]->SetQuantity(Q_FLOW);
        mpConfAnaOutMax[i]->SetQuality(DP_AVAILABLE);

        if (mpAnaOutSetupFromGeniFlag->GetValue() == false
          && !UpdateAllConfigurations)
        {
          // the change is not arisen from GENI, so set min. + max.
          mpConfAnaOutMin[i]->SetValue(0.0f); // 0
          mpConfAnaOutMax[i]->SetValue(0.0f); // 
        }

        output_quality = DP_AVAILABLE;
        break;

      default:
        DisableRangeDatapoints(i);

        output_quality = DP_NOT_AVAILABLE;
        break;
      }

      if (i < no_of_available_outputs)
      {
        mpAnaOutElectricalValue[i]->SetQuality(output_quality);
        mpAnaOutOutputValue[i]->SetQuality(output_quality);
      }
      else
      {
        mpAnaOutElectricalValue[i]->SetQuality(DP_NEVER_AVAILABLE);
        mpAnaOutOutputValue[i]->SetQuality(DP_NEVER_AVAILABLE);
      }
    }
  }

  mpAnaOutSetupFromGeniFlag->SetValue(false);
}


/*****************************************************************************
 * Function - CalculateOutput
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::CalculateOutput(void)
{
  int no_of_available_outputs = mpNoOfIo351->GetAsInt() * NO_OF_ANA_OUT_CHANNELS;

  for (int i = 0; i < no_of_available_outputs; i++)
  {
    ANA_OUT_FUNC_TYPE func = mpConfAnaOutFunc[i]->GetValue();
    float func_value = mpAnaOutInputValue[func]->GetAsFloat();
    float min = mpConfAnaOutMin[i]->GetAsFloat();
    float max = mpConfAnaOutMax[i]->GetAsFloat();

    if (func != ANA_OUT_FUNC_NO_FUNCTION)
    {
      mpAnaOutOutputValue[i]->CopyValues(mpAnaOutInputValue[func].GetSubject());
      ///Todo 20150810 ->Overwrite the max as below to avoid decimal point
      switch(func)
      {
        case ANA_OUT_FUNC_VFD_1:
        case ANA_OUT_FUNC_VFD_2:
        case ANA_OUT_FUNC_VFD_3:
        case ANA_OUT_FUNC_VFD_4:
        case ANA_OUT_FUNC_VFD_5:
        case ANA_OUT_FUNC_VFD_6:
          mpAnaOutOutputValue[i]->SetMaxValue(6000.0f);
          break;
        case ANA_OUT_FUNC_LEVEL:
        case ANA_OUT_FUNC_USER_DEFINED_1:
        case ANA_OUT_FUNC_USER_DEFINED_2:
        case ANA_OUT_FUNC_USER_DEFINED_3:
          mpAnaOutOutputValue[i]->SetMaxValue(10000.0f);
          break;
        case ANA_OUT_FUNC_DOSING_PUMP:
          mpAnaOutOutputValue[i]->SetMaxValue(0.00027778f);
          break;
      }
  
    }

    if (func_value < min)
    {
      mpAnaOutElectricalValue[i]->SetValueAsPercent(0.0f);
    }
    else if (func_value > max)
    {
      mpAnaOutElectricalValue[i]->SetValueAsPercent(100.0f);
    }
    else
    {
      if (max > min)
      {
        float out_value_in_percent = 100.0f * (func_value - min) / (max - min);

        mpAnaOutElectricalValue[i]->SetValueAsPercent(out_value_in_percent);
      }
      else
      {
        mpAnaOutElectricalValue[i]->SetValueAsPercent(0.0f);
      }
    }

  }
}

/*****************************************************************************
 * Function - CopyOutput
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::CopyOutput(void)
{
  const IO351_NO_TYPE io_module_no[] =
  {
    IO351_IOM_NO_1,
    IO351_IOM_NO_2,
    IO351_IOM_NO_3
  };

  int no_of_io = mpNoOfIo351->GetAsInt();

  for (int i = 0; i < no_of_io; i++)
  {
    for (int n = 0; n < NO_OF_ANA_OUT_CHANNELS; n++)
    {
      float out_value = mpAnaOutElectricalValue[(i * NO_OF_ANA_OUT_CHANNELS) + n]->GetValueAsPercent();

      mpGeniSlaveIf->SetIO351AnalogOutput(io_module_no[i], (IO351_ANA_OUT_NO_TYPE)n, out_value);
    }
  }
}

/*****************************************************************************
 * Function - DisableRangeDatapoints
 * DESCRIPTION:
 *
 *****************************************************************************/
void AnaOutCtrl::DisableRangeDatapoints(U8 AnalogOutputIndex)
{
  mpConfAnaOutMin[AnalogOutputIndex]->SetMinValue(0.0f);
  mpConfAnaOutMin[AnalogOutputIndex]->SetMaxValue(0.0f);
  mpConfAnaOutMin[AnalogOutputIndex]->SetQuantity(Q_NO_UNIT);
  mpConfAnaOutMin[AnalogOutputIndex]->SetQuality(DP_NOT_AVAILABLE);

  mpConfAnaOutMax[AnalogOutputIndex]->SetMinValue(0.0f);
  mpConfAnaOutMax[AnalogOutputIndex]->SetMaxValue(0.0f);
  mpConfAnaOutMax[AnalogOutputIndex]->SetQuantity(Q_NO_UNIT);
  mpConfAnaOutMax[AnalogOutputIndex]->SetQuality(DP_NOT_AVAILABLE);
}


/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/

