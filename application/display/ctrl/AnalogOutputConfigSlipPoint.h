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
/* CLASS NAME       : AnalogOutputConfigSlipPoint                           */
/*                                                                          */
/* FILE NAME        : AnalogOutputConfigSlipPoint.h                         */
/*                                                                          */
/* CREATED DATE     :                                                       */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the analog output configuration DataPoints into one      */
/* virtual DataPoint for the display to look at...                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcAnalogOutputConfigSlipPoint_h
#define mrcAnalogOutputConfigSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <AppTypeDefs.h>
#include <Subtask.h>
#include <Observer.h>
#include <EnumDataPoint.h>
#include <FloatDataPoint.h>
#include <IIntegerDataPoint.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
/*****************************************************************************
  DEFINES
 *****************************************************************************/
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
class AnalogOutputConfigSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    AnalogOutputConfigSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~AnalogOutputConfigSlipPoint();
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
    virtual void UpdateVirtualAnalogOutputConfig(void);
    virtual void UpdateCurrentAnalogOutputConfig(void);
    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<ANA_OUT_FUNC_TYPE>*>  	  mpAnalogOutputFunc[MAX_NO_OF_ANA_OUTPUTS];
    SubjectPtr<FloatDataPoint*>                				mpAnalogOutputMin[MAX_NO_OF_ANA_OUTPUTS];
    SubjectPtr<FloatDataPoint*>                				mpAnalogOutputMax[MAX_NO_OF_ANA_OUTPUTS];

    SubjectPtr<IIntegerDataPoint*>                    mpCurrentAnalogOutputNumber;

    SubjectPtr<EnumDataPoint<ANA_OUT_FUNC_TYPE>*>  	  mpVirtualAnalogOutputFunc;
    SubjectPtr<FloatDataPoint*>       				        mpVirtualAnalogOutputMin;
    SubjectPtr<FloatDataPoint*>				                mpVirtualAnalogOutputMax;

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
