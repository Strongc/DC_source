/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW MRC                                           */
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
/* CLASS NAME       : VfdEnergyTestSlipPoint                                */
/*                                                                          */
/* FILE NAME        : VfdEnergyTestSlipPoint.cpp                            */
/*                                                                          */
/* CREATED DATE     : 26-10-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the VFD energy test DataPoints into one                  */
/* set of virtual DataPoints for the display to look at...                  */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/

/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>
#include <DataPoint.h>
#include <DisplayController.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "VfdEnergyTestSlipPoint.h"

/*****************************************************************************
DEFINES
*****************************************************************************/
#define DISPLAY_VFD_ENERGY_TEST_ID 154
#define DISPLAY_VFD_ENERGY_GRAPH_ID 155

#define DEFAULT_MAX_Y_VALUE 900000 //900000 J/m3 = 0.25 kWh/m3
#define DEFAULT_MIN_Y_VALUE 0      //0 J/m3 = 0.0 kWh/m3
#define Y_AXIS_RESOLUTION   180000


/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    namespace ctrl
    {

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
      VfdEnergyTestSlipPoint::VfdEnergyTestSlipPoint()
      {
        mCurrentlyUpdating = false;
      }

      /*****************************************************************************
      * Function - Destructor
      * DESCRIPTION:
      *
      ****************************************************************************/
      VfdEnergyTestSlipPoint::~VfdEnergyTestSlipPoint()
      {
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::InitSubTask(void)
      {
        if (mpCurrentVfdNumber.IsValid())
        {
          mpCurrentVfdNumber->SetAsInt(0);
        }

        mpVirtualGraphData->SetYQuantity(Q_SPECIFIC_ENERGY);
        mpVirtualGraphData->SetXQuantity(Q_FREQUENCY);
        mpVirtualGraphData->SetMinXValue(30.0f);
        mpVirtualGraphData->SetMaxXValue(60.0f);

        TransformVectorToGraphData(0);

        UpdateVirtual();
        UpdateUpperStatusLine();
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::RunSubTask(void)
      {
        mCurrentlyUpdating = true;  // Guard the SubTask

        const int vfd_no = mpCurrentVfdNumber->GetAsInt();

        if (mpCurrentVfdNumber.IsUpdated() )
        {
          UpdateVirtual();

          TransformVectorToGraphData(vfd_no);

          UpdateUpperStatusLine();
        }

        if (mpVirtualSettlingTime.IsUpdated()
          || mpVirtualLevelRange.IsUpdated())
        {
          UpdateCurrent();
        }

        if (mpVirtualStartTestEvent.IsUpdated())
        {
          mpStartTestEvent[vfd_no]->SetEvent();
        }
        

        if ((vfd_no >= 0) && (vfd_no < NO_OF_PUMPS))
        {
          bool is_updated = mpSettlingTime[vfd_no].IsUpdated();          
          is_updated |= mpStartTestEvent[vfd_no].IsUpdated();
          is_updated |= mpTimeOfLastRun[vfd_no].IsUpdated();
          is_updated |= mpActualFrequency[vfd_no].IsUpdated();
          is_updated |= mpActualEnergy[vfd_no].IsUpdated();

          if (is_updated)
          {
            UpdateVirtual();
          }

          if (mpListData[vfd_no].IsUpdated())
          {
            if(!is_updated)
            {
              UpdateVirtual();
            }

            TransformVectorToGraphData(vfd_no);
          }


        }
        else
        {
          FatalErrorOccured("VETSP index out of range!");
        }


        mCurrentlyUpdating = false; // End of: Guard the SubTask
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::SubscribtionCancelled(Subject* pSubject)
      {
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::Update(Subject* pSubject)
      {
        if ( !mCurrentlyUpdating )
        {
          if (mpCurrentVfdNumber.Update(pSubject)){}
          else if(mpVirtualSettlingTime.Update(pSubject)){}
          else if(mpVirtualLevelRange.Update(pSubject)){}
          else if(mpVirtualStartTestEvent.Update(pSubject)){}
          else
          {   
            int vfd_no = mpCurrentVfdNumber->GetValue();

            if (mpSettlingTime[vfd_no].Update(pSubject)){}
            else if(mpLevelRange[vfd_no].Update(pSubject)){}
            else if(mpStartTestEvent[vfd_no].Update(pSubject)){}
            else if(mpTimeOfLastRun[vfd_no].Update(pSubject)){}
            else if(mpActualFrequency[vfd_no].Update(pSubject)){}
            else if(mpActualEnergy[vfd_no].Update(pSubject)){}
            else if(mpListData[vfd_no].Update(pSubject)){}
          }
          ReqTaskTime();
        }
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::SetSubjectPointer(int Id, Subject* pSubject)
      {
        switch ( Id )
        {
          case SP_VETSP_CURRENT_NO:
          mpCurrentVfdNumber.Attach(pSubject);
          break;

          case SP_VETSP_VIRTUAL_SETTLING_TIME :
          mpVirtualSettlingTime.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_LEVEL_RANGE :
          mpVirtualLevelRange.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_START_TEST_EVENT :
          mpVirtualStartTestEvent.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_TIME_OF_LAST_RUN :
          mpVirtualTimeOfLastRun.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_ACTUAL_FREQUENCY :
          mpVirtualActualFrequency.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_ACTUAL_ENERGY :
          mpVirtualActualEnergy.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_GRAPH_DATA :
          mpVirtualGraphData.Attach(pSubject);
          break;
          case SP_VETSP_VIRTUAL_LIST_DATA :
          mpVirtualListData.Attach(pSubject);
          break;

          case SP_VETSP_SETTLING_TIME_PUMP_1 :
          mpSettlingTime[PUMP_1].Attach(pSubject);
          break;
          case SP_VETSP_LEVEL_RANGE_PUMP_1 :
          mpLevelRange[PUMP_1].Attach(pSubject);
          break;
          case SP_VETSP_START_TEST_EVENT_PUMP_1 :
          mpStartTestEvent[PUMP_1].Attach(pSubject);
          break;
          case SP_VETSP_TIME_OF_LAST_RUN_PUMP_1 :
          mpTimeOfLastRun[PUMP_1].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_FREQUENCY_PUMP_1 :
          mpActualFrequency[PUMP_1].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_ENERGY_PUMP_1 :
          mpActualEnergy[PUMP_1].Attach(pSubject);
          break;
          case SP_VETSP_LIST_DATA_PUMP_1 :
          mpListData[PUMP_1].Attach(pSubject);
          break;

          case SP_VETSP_SETTLING_TIME_PUMP_2 :
          mpSettlingTime[PUMP_2].Attach(pSubject);
          break;
          case SP_VETSP_LEVEL_RANGE_PUMP_2 :
          mpLevelRange[PUMP_2].Attach(pSubject);
          break;
          case SP_VETSP_START_TEST_EVENT_PUMP_2 :
          mpStartTestEvent[PUMP_2].Attach(pSubject);
          break;
          case SP_VETSP_TIME_OF_LAST_RUN_PUMP_2 :
          mpTimeOfLastRun[PUMP_2].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_FREQUENCY_PUMP_2 :
          mpActualFrequency[PUMP_2].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_ENERGY_PUMP_2 :
          mpActualEnergy[PUMP_2].Attach(pSubject);
          break;
          case SP_VETSP_LIST_DATA_PUMP_2 :
          mpListData[PUMP_2].Attach(pSubject);
          break;

          case SP_VETSP_SETTLING_TIME_PUMP_3 :
          mpSettlingTime[PUMP_3].Attach(pSubject);
          break;
          case SP_VETSP_LEVEL_RANGE_PUMP_3 :
          mpLevelRange[PUMP_3].Attach(pSubject);
          break;
          case SP_VETSP_START_TEST_EVENT_PUMP_3 :
          mpStartTestEvent[PUMP_3].Attach(pSubject);
          break;
          case SP_VETSP_TIME_OF_LAST_RUN_PUMP_3 :
          mpTimeOfLastRun[PUMP_3].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_FREQUENCY_PUMP_3 :
          mpActualFrequency[PUMP_3].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_ENERGY_PUMP_3 :
          mpActualEnergy[PUMP_3].Attach(pSubject);
          break;
          case SP_VETSP_LIST_DATA_PUMP_3 :
          mpListData[PUMP_3].Attach(pSubject);
          break;

          case SP_VETSP_SETTLING_TIME_PUMP_4 :
          mpSettlingTime[PUMP_4].Attach(pSubject);
          break;
          case SP_VETSP_LEVEL_RANGE_PUMP_4 :
          mpLevelRange[PUMP_4].Attach(pSubject);
          break;
          case SP_VETSP_START_TEST_EVENT_PUMP_4 :
          mpStartTestEvent[PUMP_4].Attach(pSubject);
          break;
          case SP_VETSP_TIME_OF_LAST_RUN_PUMP_4 :
          mpTimeOfLastRun[PUMP_4].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_FREQUENCY_PUMP_4 :
          mpActualFrequency[PUMP_4].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_ENERGY_PUMP_4 :
          mpActualEnergy[PUMP_4].Attach(pSubject);
          break;
          case SP_VETSP_LIST_DATA_PUMP_4 :
          mpListData[PUMP_4].Attach(pSubject);
          break;

          case SP_VETSP_SETTLING_TIME_PUMP_5 :
          mpSettlingTime[PUMP_5].Attach(pSubject);
          break;
          case SP_VETSP_LEVEL_RANGE_PUMP_5 :
          mpLevelRange[PUMP_5].Attach(pSubject);
          break;
          case SP_VETSP_START_TEST_EVENT_PUMP_5 :
          mpStartTestEvent[PUMP_5].Attach(pSubject);
          break;
          case SP_VETSP_TIME_OF_LAST_RUN_PUMP_5 :
          mpTimeOfLastRun[PUMP_5].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_FREQUENCY_PUMP_5 :
          mpActualFrequency[PUMP_5].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_ENERGY_PUMP_5 :
          mpActualEnergy[PUMP_5].Attach(pSubject);
          break;
          case SP_VETSP_LIST_DATA_PUMP_5 :
          mpListData[PUMP_5].Attach(pSubject);
          break;

          case SP_VETSP_SETTLING_TIME_PUMP_6 :
          mpSettlingTime[PUMP_6].Attach(pSubject);
          break;
          case SP_VETSP_LEVEL_RANGE_PUMP_6 :
          mpLevelRange[PUMP_6].Attach(pSubject);
          break;
          case SP_VETSP_START_TEST_EVENT_PUMP_6 :
          mpStartTestEvent[PUMP_6].Attach(pSubject);
          break;
          case SP_VETSP_TIME_OF_LAST_RUN_PUMP_6 :
          mpTimeOfLastRun[PUMP_6].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_FREQUENCY_PUMP_6 :
          mpActualFrequency[PUMP_6].Attach(pSubject);
          break;
          case SP_VETSP_ACTUAL_ENERGY_PUMP_6 :
          mpActualEnergy[PUMP_6].Attach(pSubject);
          break;
          case SP_VETSP_LIST_DATA_PUMP_6 :
          mpListData[PUMP_6].Attach(pSubject);
          break;
        }
      }

      /*****************************************************************************
      * Function -
      * DESCRIPTION:
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::ConnectToSubjects(void)
      {
        mpCurrentVfdNumber->Subscribe(this);

        for (int vfd_no = 0; vfd_no < NO_OF_PUMPS; vfd_no++)
        {
          mpSettlingTime[vfd_no]->Subscribe(this);
          mpLevelRange[vfd_no]->Subscribe(this);
          mpStartTestEvent[vfd_no]->Subscribe(this);
          mpTimeOfLastRun[vfd_no]->Subscribe(this);
          mpActualFrequency[vfd_no]->Subscribe(this);
          mpActualEnergy[vfd_no]->Subscribe(this);
          mpListData[vfd_no]->Subscribe(this);
        }

        mpVirtualSettlingTime->Subscribe(this);
        mpVirtualLevelRange->Subscribe(this);
        mpVirtualStartTestEvent->Subscribe(this);
      }



      /*****************************************************************************
      *
      *
      *              PRIVATE FUNCTIONS
      *
      *
      ****************************************************************************/
      void VfdEnergyTestSlipPoint::UpdateVirtual()
      {
        const int vfd_no = mpCurrentVfdNumber->GetAsInt();

        if ((vfd_no >= 0) && (vfd_no < NO_OF_PUMPS))
        {
          mpVirtualSettlingTime->SetValue(mpSettlingTime[vfd_no]->GetValue());
          mpVirtualLevelRange->SetValue(mpLevelRange[vfd_no]->GetValue());
          mpVirtualTimeOfLastRun->SetValue(mpTimeOfLastRun[vfd_no]->GetValue());
          mpVirtualActualFrequency->CopyValues(mpActualFrequency[vfd_no].GetSubject());
          mpVirtualActualEnergy->CopyValues(mpActualEnergy[vfd_no].GetSubject());
          mpVirtualListData->CopyValues(mpListData[vfd_no].GetSubject());
        }
        else
        {
          FatalErrorOccured("VETSP index out of range!");
        }
      }


      void VfdEnergyTestSlipPoint::TransformVectorToGraphData(int vfdNo)
      {
        float min_specific_energy = 0.0f;
        float max_specific_energy = 0.0f;
        int index_of_min_specific_energy = 0;
        int no_of_samples = 0;

        float max_frequency = mpActualFrequency[vfdNo]->GetMaxValue();

        mpVirtualGraphData->SetMaxXValue(max_frequency > 50.0f ? 60.0f : 50.0f);

        float frequency_range = (max_frequency - 30.0f) / 2.0f;

        mpVirtualGraphData->ClearCurveValues();

        for (int i = 0; i < mpListData[vfdNo]->GetSize(); i++)
        {
          float specific_energy = mpListData[vfdNo]->GetValue(i);

          if (specific_energy > 0)
          {
            no_of_samples++;

            if (min_specific_energy == 0 || specific_energy < min_specific_energy)
            {
              min_specific_energy = specific_energy;
              index_of_min_specific_energy = i;
            }
            if (specific_energy > max_specific_energy)
            {
              max_specific_energy = specific_energy;
            }
          }
        }

        if (no_of_samples == 0)
        {
          mpVirtualGraphData->SetCurrentQuality(DP_NOT_AVAILABLE);
        }
        else
        {
          float frequency = 30.0f + index_of_min_specific_energy * 2.0f; 
          mpVirtualGraphData->SetCurrentXValue(frequency);
          mpVirtualGraphData->SetCurrentYValue(min_specific_energy);
        }

        if (no_of_samples > 1)
        {
          min_specific_energy = Y_AXIS_RESOLUTION * (int)(min_specific_energy / Y_AXIS_RESOLUTION);
          max_specific_energy = Y_AXIS_RESOLUTION * (int)(1 + (max_specific_energy / Y_AXIS_RESOLUTION));

          mpVirtualGraphData->SetMinYValue(min_specific_energy);
          mpVirtualGraphData->SetMaxYValue(max_specific_energy);
        }
        else
        {
          mpVirtualGraphData->SetMinYValue(DEFAULT_MIN_Y_VALUE);
          mpVirtualGraphData->SetMaxYValue(DEFAULT_MAX_Y_VALUE);
        }

        for (int i = 0; i < mpListData[vfdNo]->GetSize(); i++)
        {
          float specific_energy = mpListData[vfdNo]->GetValue(i);

          if (specific_energy > 0)
          {
            int pixel_index = 4 + (int) 176 * (i / frequency_range);

            mpVirtualGraphData->SetCurveValue(pixel_index, specific_energy);
          }
        }

        mpVirtualGraphData->NotifyObservers();
      }


      void VfdEnergyTestSlipPoint::UpdateCurrent()
      {
        const int vfd_no = mpCurrentVfdNumber->GetAsInt();

        if ((vfd_no >= 0) && (vfd_no < NO_OF_PUMPS))
        {
          mpSettlingTime[vfd_no]->SetValue(mpVirtualSettlingTime->GetValue());
          mpLevelRange[vfd_no]->SetValue(mpVirtualLevelRange->GetValue());
        }
        else
        {
          FatalErrorOccured("VETSP index out of range!");
        }
      }

      void VfdEnergyTestSlipPoint::UpdateUpperStatusLine()
      {
        char display_number[14];

        Display* p_display = GetDisplay(DISPLAY_VFD_ENERGY_TEST_ID);

        int index = mpCurrentVfdNumber->GetAsInt() + 1;

        sprintf(display_number, "4.2.10.%i.3", index);
        p_display->SetDisplayNumber( display_number );

        p_display = GetDisplay(DISPLAY_VFD_ENERGY_GRAPH_ID);
        sprintf(display_number, "4.2.10.%i.3.1", index);
        p_display->SetDisplayNumber( display_number );


        DisplayController::GetInstance()->RequestTitleUpdate();
      }
      /*****************************************************************************
      *
      *
      *              PROTECTED FUNCTIONS
      *                 - RARE USED -
      *
      ****************************************************************************/
    } // namespace ctrl
  } // namespace display
} // namespace mpc


