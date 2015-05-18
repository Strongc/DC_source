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
/* CLASS NAME       : BoolLogic                                             */
/*                                                                          */
/* FILE NAME        : BoolLogic.h                                           */
/*                                                                          */
/* CREATED DATE     : 25-05-2009  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/

/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef mpcBoolLogic_h
#define mpcBoolLogic_h

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#include <Observer.h>
#include <BoolDataPoint.h>
#include <SubTask.h>
#include <vector>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
/*****************************************************************************
  DEFINES
 *****************************************************************************/
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  FIRST_BOOL_LOGIC_OPERATOR = 0,
  BOOL_LOGIC_OPERATOR_AND = FIRST_USER_IO_LOGIC,
  BOOL_LOGIC_OPERATOR_OR,
  BOOL_LOGIC_OPERATOR_XOR,
  // insert new items above
  NO_OF_BOOL_LOGIC_OPERATOR,
  LAST_BOOL_LOGIC_OPERATOR = NO_OF_BOOL_LOGIC_OPERATOR - 1
}BOOL_LOGIC_OPERATOR_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class BoolLogic : public Observer, public SubTask
{
  public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    BoolLogic(BOOL_LOGIC_OPERATOR_TYPE boolOperator);

    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~BoolLogic(void);

    /********************************************************************
    ASSIGNMENT OPERATORS
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    void RunSubTask(void);
    void Update(Subject* pSubject);
    void SubscribtionCancelled(Subject* pSubject);
    void ConnectToSubjects(void);
    void SetSubjectPointer(int Id, Subject* pSubject);
    void InitSubTask(void);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTES
    ********************************************************************/
    std::vector<BoolDataPoint*> mSourceList;
    SubjectPtr<BoolDataPoint*> mpOutput;

    bool mReqTaskTimeFlag;

    BOOL_LOGIC_OPERATOR_TYPE mOperator;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif

