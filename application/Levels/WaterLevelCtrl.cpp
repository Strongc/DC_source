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
/* CLASS NAME       : WaterLevelCtrl                                        */
/*                                                                          */
/* FILE NAME        : WaterLevelCtrl.cpp                                    */
/*                                                                          */
/* CREATED DATE     : 01-10-2007 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <WaterLevelCtrl.h>         // class implemented

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
 *****************************************************************************/
WaterLevelCtrl::WaterLevelCtrl()
{
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
WaterLevelCtrl::~WaterLevelCtrl()
{
}

/*****************************************************************************
 * Function - RunSubTask
 * DESCRIPTION:
 ****************************************************************************/
void WaterLevelCtrl::RunSubTask()
{
  mRunRequestedFlag = false;

  if( mpLevelSimEnabled->GetValue() )
  {
    mpSurfaceLevelReadyFlag->SetValue(true);
    mpSurfaceLevel->CopyValues( mpLevelSim.GetSubject() );
  }
  else
  {
    if ( mpAnalogValuesMeasuredFlag->GetValue() == true)
    {
      switch ( mpPitLevelCtrlType->GetValue() )
      {
        case SENSOR_TYPE_FLOAT_SWITCHES :
          if (mpMeasuredValueHeightPressure->GetQuality() != DP_NEVER_AVAILABLE)
          {
            mpSurfaceLevel->CopyValues( mpMeasuredValueHeightPressure.GetSubject() );
          }
          else
          {
            mpSurfaceLevel->CopyValues( mpWaterLevelUltraSound.GetSubject() );
          }
          break;
        case SENSOR_TYPE_PRESSURE :
          mpSurfaceLevel->CopyValues( mpMeasuredValueHeightPressure.GetSubject() );
          break;
        case SENSOR_TYPE_ULTRA_SONIC :
          mpSurfaceLevel->CopyValues( mpWaterLevelUltraSound.GetSubject() );
          break;
        default:
          /* Not possible */
          mpSurfaceLevel->SetValue( 0 );
          break;
      }
      mpSurfaceLevelReadyFlag->SetValue(true);
    }
  }
}

/*****************************************************************************
 * Function - SetSubjectPointer
 * DESCRIPTION: Set pointers to sub.
 *
 *****************************************************************************/
void WaterLevelCtrl::SetSubjectPointer(int Id, Subject* pSubject)
{
  switch(Id)
  {
    case SP_WLC_WATER_LEVEL_ULTRA_SOUND:
      mpWaterLevelUltraSound.Attach(pSubject);
      break;
    case SP_WLC_MEASURED_VALUE_HEIGHT_PRESSURE:
      mpMeasuredValueHeightPressure.Attach(pSubject);
      break;
    case SP_WLC_PIT_LEVEL_CTRL_TYPE:
      mpPitLevelCtrlType.Attach(pSubject);
      break;
    case SP_WLC_SURFACE_LEVEL:
      mpSurfaceLevel.Attach(pSubject);
      break;
    case SP_WLC_LEVEL_SIM_ENABLED:
      mpLevelSimEnabled.Attach(pSubject);
      break;
    case SP_WLC_LEVEL_SIM:
      mpLevelSim.Attach(pSubject);
      break;

    case SP_WLC_ANALOG_VALUES_MEASURED:
      mpAnalogValuesMeasuredFlag.Attach(pSubject);
      break;
    case SP_WLC_SURFACE_LEVEL_READY:
      mpSurfaceLevelReadyFlag.Attach(pSubject);
      break;

    default:
      break;
  }
}


/*****************************************************************************
 * Function - Update
 * DESCRIPTION: Update operation from Observer class
 *
 *****************************************************************************/
void WaterLevelCtrl::Update(Subject* pSubject)
{
  if(mRunRequestedFlag == false)
  {
    mRunRequestedFlag = true;
    ReqTaskTime();
  }
}

/*****************************************************************************
 * Function - SubscribtionCancelled
 * DESCRIPTION:
 *
 *****************************************************************************/
void WaterLevelCtrl::SubscribtionCancelled(Subject* pSubject)
{
}

/*****************************************************************************
 * Function - ConnectToSubjects
 * DESCRIPTION:
 *
 *****************************************************************************/
void WaterLevelCtrl::ConnectToSubjects()
{
  mpWaterLevelUltraSound->Subscribe(this);
  mpMeasuredValueHeightPressure->Subscribe(this);
  mpPitLevelCtrlType->Subscribe(this);
  mpLevelSimEnabled->Subscribe(this);
  mpLevelSim->Subscribe(this);
}

/*****************************************************************************
 * Function - InitSubTask
 * DESCRIPTION:
 *
 *****************************************************************************/
void WaterLevelCtrl::InitSubTask()
{
  mRunRequestedFlag = true;
  ReqTaskTime();
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

