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
/* CLASS NAME       : UserIoSlipPoint                                       */
/*                                                                          */
/* FILE NAME        : UserIoSlipPoint.h                                     */
/*                                                                          */
/* CREATED DATE     : 16-12-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class maps the UserIoConfigs into common                            */
/* virtual DataPoints for the display to look at...                         */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_UserIoSlipPoint_h
#define mrc_UserIoSlipPoint_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Subtask.h>
#include <Observer.h>
#include <AppTypeDefs.h>
#include <UserIoConfig.h>
#include <U8DataPoint.h>
#include <U32DataPoint.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>
#include <StringDataPoint.h>

#include <IoChannelConfig.h>
#include <UserIoState.h>

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
class UserIoSlipPoint: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    UserIoSlipPoint();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~UserIoSlipPoint();
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
    virtual void UpdateVirtualUserIo(void);
    virtual void UpdateCurrentUserIo(void);
    virtual void UpdateUserIoChannelSources(void);
    virtual void UpdateUpperStatusLine(void);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<U8DataPoint*>        mpInputNumber;
    SubjectPtr<EnumDataPoint<USER_FUNC_SOURCE_TYPE>*> mpCurrentChannelNumber;
    SubjectPtr<EnumDataPoint<USER_IO_TYPE>*> mpCurrentUserIoNumber;
    SubjectPtr<UserIoConfig*> 	    mpUserIoDP[NO_OF_USER_IO];
    SubjectPtr<BoolDataPoint*>      mpVirtualEnabled;
    SubjectPtr<StringDataPoint*>    mpVirtualName;
    SubjectPtr<U8DataPoint*>        mpVirtualChannelIndex1;
    SubjectPtr<U8DataPoint*>        mpVirtualChannelIndex2;
    SubjectPtr<EnumDataPoint<USER_IO_LOGIC_TYPE>*> mpVirtualLogic;
    SubjectPtr<BoolDataPoint*>      mpVirtualInvert;
    SubjectPtr<U32DataPoint*>       mpVirtualMinHoldTime;
    SubjectPtr<U32DataPoint*>       mpVirtualMaxHoldTime;
    SubjectPtr<BoolDataPoint*>      mpVirtualMaxHoldTimeEnabled;
    SubjectPtr<EnumDataPoint<USER_IO_TYPE>*> mpVirtualDestination;

    SubjectPtr<IoChannelConfig*>    mpCh1Config;
    SubjectPtr<IoChannelConfig*>    mpCh2Config;

    SubjectPtr<IoChannelConfig*>    mpIoChConfigs[NO_OF_USER_FUNC_SOURCE + 1];
    SubjectPtr<StringDataPoint*>    mpNames[NO_OF_USER_IO + 1];
    
    bool                            mCurrentlyUpdating;

    UserIoState*                    mpUserIoState;
    
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
