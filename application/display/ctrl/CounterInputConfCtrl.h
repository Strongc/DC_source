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
/* CLASS NAME       : CounterInputConfCtrl                                  */
/*                                                                          */
/* FILE NAME        : CounterInputConfCtrl.H                                */
/*                                                                          */
/* CREATED DATE     : 2008-03-05                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This class                                                               */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mrc_CounterInputConfCtrl_h
#define mrc_CounterInputConfCtrl_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Subtask.h>
#include <Observer.h>
#include <BoolDataPoint.h>
#include <EnumDataPoint.h>

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
class CounterInputConfCtrl: public SubTask, public Observer
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    CounterInputConfCtrl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~CounterInputConfCtrl();
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
    void ClearFuncTwiceDi4(DIGITAL_INPUT_FUNC_TYPE diType);
    void ClearFuncTwiceDi5(DIGITAL_INPUT_FUNC_TYPE diType);
    void ClearFuncTwiceDi13(DIGITAL_INPUT_FUNC_TYPE diType);
    void ClearFuncTwiceDi14(DIGITAL_INPUT_FUNC_TYPE diType);
    void ClearFuncTwiceDi22(DIGITAL_INPUT_FUNC_TYPE diType);
    void ClearFuncTwiceDi23(DIGITAL_INPUT_FUNC_TYPE diType);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    SubjectPtr<BoolDataPoint*>      mDpVolumeCounterNotUsed;
    SubjectPtr<BoolDataPoint*>      mDpEnergyCounterNotUsed;
    SubjectPtr<BoolDataPoint*>      mDpUserDefineCounter1NotUsed;
    SubjectPtr<BoolDataPoint*>      mDpUserDefineCounter2NotUsed;
    SubjectPtr<BoolDataPoint*>      mDpUserDefineCounter3NotUsed;

    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*>  mDpDi4Function;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*>  mDpDi5Function;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*>  mDpDi13Function;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*>  mDpDi14Function;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*>  mDpDi22Function;
    SubjectPtr<EnumDataPoint<DIGITAL_INPUT_FUNC_TYPE>*>  mDpDi23Function;

    bool                            mCurrentlyUpdating;

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
