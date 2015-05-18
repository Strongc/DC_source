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
/* CLASS NAME       : IoChannelSlipPoint                                    */
/*                                                                          */
/* FILE NAME        : IoChannelSlipPoint.h                                  */
/*                                                                          */
/* CREATED DATE     : 16-12-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the IoChannelConfigs into common                         */
/* virtual DataPoints for the display to look at...                         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_IoChannelSlipPoint_h
#define mrc_IoChannelSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <EnumDataPoint.h>
#include <BoolDataPoint.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <FloatDataPoint.h>
#include <IoChannelConfig.h>
#include <IoChannelHeadlineState.h>


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
class IoChannelSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    IoChannelSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~IoChannelSlipPoint();

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void InitSubTask(void);
    virtual void RunSubTask(void);
    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void Update(Subject* pSubject);
    virtual void SetSubjectPointer(int Id,Subject* pSubject);
    virtual void ConnectToSubjects(void);

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    virtual void UpdateVirtualChannel(void);
    virtual void UpdateCurrentChannel(void);
    virtual void InitializeSourceFromType(void);
    virtual void InitializeIndexOfSource(bool resetValue);
    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<EnumDataPoint<USER_FUNC_SOURCE_TYPE>*> mpCurrentChannelNumber;
    SubjectPtr<IoChannelConfig*>    mpChannelDP[NO_OF_USER_FUNC_SOURCE + 1];

    SubjectPtr<EnumDataPoint<CHANNEL_SOURCE_DATATYPE_TYPE>*> mpVirtualSourceType;
    SubjectPtr<EnumDataPoint<CHANNEL_SOURCE_TYPE>*> mpVirtualSource;
    SubjectPtr<U8DataPoint*>        mpVirtualSourceIndex;
    SubjectPtr<FloatDataPoint*>     mpVirtualAiLimit;
    SubjectPtr<BoolDataPoint*>      mpVirtualInvert;
    SubjectPtr<U32DataPoint*>       mpVirtualResponseTime;
    SubjectPtr<U32DataPoint*>       mpVirtualTimerHigh;
    SubjectPtr<U32DataPoint*>       mpVirtualTimerLow;
    SubjectPtr<BoolDataPoint*>      mpVirtualConstantValue;
    bool                            mCurrentlyUpdating;

    IoChannelHeadlineState*         mpIoChannelHeadlineState;

  private:
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
