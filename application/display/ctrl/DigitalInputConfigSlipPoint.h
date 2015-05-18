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
/* CLASS NAME       : DigitalInputConfigSlipPoint                           */
/*                                                                          */
/* FILE NAME        : DigitalInputConfigSlipPoint.H                         */
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
#ifndef mpcDIGITAL_INPUT_CONFIG_SLIP_POINT_h
#define mpcDIGITAL_INPUT_CONFIG_SLIP_POINT_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <cu351_cpu_types.h>
#include <Subtask.h>
#include <Observer.h>
#include <EnumDataPoint.h>
#include <IIntegerDataPoint.h>
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
class DigitalInputConfigSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    DigitalInputConfigSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~DigitalInputConfigSlipPoint();
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
    virtual void UpdateVirtualDigitalInputConfLogic(void);
    virtual void UpdateVirtualDigitalInputConfig(void);
    virtual void UpdateCurrentDigitalInputConfig(void);
    virtual void UpdateCurrentDigitalInputConfLogic(void);

    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mpDigitalConfigDP[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<BoolDataPoint*>                          mpDigitalConfLogicDP[MAX_NO_OF_DIG_INPUTS];
    SubjectPtr<IIntegerDataPoint*>                      mpCurrentDigitalInputNumber;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*> mpVirtualDigitalInputConfig;
    SubjectPtr<BoolDataPoint*>                          mpVirtualDigitalInputConfLogic;
    bool                                                mCurrentlyUpdating;

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
