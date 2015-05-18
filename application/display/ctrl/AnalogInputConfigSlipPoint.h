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
/* CLASS NAME       : AnalogInputConfigSlipPoint                            */
/*                                                                          */
/* FILE NAME        : AnalogInputConfigSlipPoint.h                          */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the digital input configuration DataPoints into one      */
/* virtual DataPoint for the display to look at...                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ANALOG_INPUT_CONFIG_SLIP_POINT_H__
#define __ANALOG_INPUT_CONFIG_SLIP_POINT_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <IIntegerDataPoint.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
const int NO_OF_ANA_IN = 9;

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
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class AnalogInputConfigSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AnalogInputConfigSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~AnalogInputConfigSlipPoint();
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void InitSubTask(void);
    virtual void RunSubTask(void);

    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject);
    virtual void ConnectToSubjects(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void UpdateVirtualAnalogInputConfig(void);
    virtual void UpdateCurrentAnalogInputConfig(void);

    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*>  	mpAnalogConfigDP[NO_OF_ANA_IN];
    SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*> 	mpAnalogElectricalConfigDP[NO_OF_ANA_IN];
    SubjectPtr<FloatDataPoint*>                				mpAnalogMinSensorRange[NO_OF_ANA_IN];
    SubjectPtr<FloatDataPoint*>                				mpAnalogMaxSensorRange[NO_OF_ANA_IN];

    SubjectPtr<IIntegerDataPoint*>  mpCurrentAnalogInputNumber;

    SubjectPtr<EnumDataPoint<MEASURED_VALUE_TYPE>*>  	mpVirtualAnalogInputConfig;
    SubjectPtr<EnumDataPoint<SENSOR_ELECTRIC_TYPE>*> 	mpVirtualAnalogInputElectricalConfig;
    SubjectPtr<FloatDataPoint*>       				        mpVirtualAnalogMinSensorRange;
    SubjectPtr<FloatDataPoint*>				                mpVirtualAnalogMaxSensorRange;

    bool mCurrentlyUpdating;
  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
    } // namespace ctrl
  } // namespace display
} // namespace mpc
#endif
