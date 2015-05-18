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
/* FILE NAME        : VfdEnergyTestSlipPoint.h                              */
/*                                                                          */
/* CREATED DATE     : 26-10-2009 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the VFD energy test DataPoints into one                  */
/* set of virtual DataPoints for the display to look at...                  */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrcVfdEnergyTestSlipPoint_h
#define mrcVfdEnergyTestSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <FloatDataPoint.h>
#include <EventDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatVectorDataPoint.h>
#include <CurrentPointCurveData.h>
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
class VfdEnergyTestSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    VfdEnergyTestSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~VfdEnergyTestSlipPoint();
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
    virtual void UpdateVirtual(void);
    virtual void UpdateCurrent(void);

    void TransformVectorToGraphData(int vfdNo);

    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U8DataPoint*>                      mpCurrentVfdNumber;

    SubjectPtr<U32DataPoint*>                     mpVirtualSettlingTime;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualLevelRange;
    SubjectPtr<EventDataPoint*> 	                mpVirtualStartTestEvent;
    SubjectPtr<U32DataPoint*>                     mpVirtualTimeOfLastRun;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualActualFrequency;
    SubjectPtr<FloatDataPoint*> 	                mpVirtualActualEnergy;
    SubjectPtr<FloatVectorDataPoint*> 	          mpVirtualListData;
    SubjectPtr<CurrentPointCurveData*> 	          mpVirtualGraphData;

    SubjectPtr<U32DataPoint*>                     mpSettlingTime[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpLevelRange[NO_OF_PUMPS];
    SubjectPtr<EventDataPoint*> 	                mpStartTestEvent[NO_OF_PUMPS];
    SubjectPtr<U32DataPoint*>                     mpTimeOfLastRun[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpActualFrequency[NO_OF_PUMPS];
    SubjectPtr<FloatDataPoint*> 	                mpActualEnergy[NO_OF_PUMPS];
    SubjectPtr<FloatVectorDataPoint*> 	          mpListData[NO_OF_PUMPS];
    
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
