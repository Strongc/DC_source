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
/* CLASS NAME       : ErrorLog                                              */
/*                                                                          */
/* FILE NAME        : ErrorLog.h                                            */
/*                                                                          */
/* CREATED DATE     : 29-07-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ERROR_LOG_H__
#define __ERROR_LOG_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>
#ifdef __cplusplus
#include <Observer.h>
#include <I32VectorDataPoint.h>
#endif

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define ERROR_LOG_LENGTH 10

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef enum
{
  /* RunFactory */
  ERROR_LOG_ID_RUNFACTORY_UNSED_SUBJECT_DETECTED = 1,  // one or more unused subject (ref count == 0)
  
  /* AnaInOnIOCtrl */
  ERROR_LOG_ID_ANAINONIOCTRL_UNKNOWNSENSORELECTRIC = 100,
  
  /* RTIP */
  ERROR_LOG_ID_RTIP_MEM_GUARD = 1800

} ERROR_LOG_ID_TYPE;

/********************************************************************
FUNCTION PROTOTYPES
********************************************************************/
EXTERN void LogError(ERROR_LOG_ID_TYPE newError);
EXTERN void ResetErrorLog();

#ifdef __cplusplus
/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class ErrorLog : public Observer
{
  public:
    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    static ErrorLog* GetInstance();
    void LogError(ERROR_LOG_ID_TYPE newError);
    void ResetErrorLog();
    virtual void Update(Subject* pSubject);
    virtual void SubscribtionCancelled(Subject* pSubject);
    virtual void SetSubjectPointer(int Id, Subject* pSubject);
    virtual void ConnectToSubjects(void);

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    ErrorLog();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~ErrorLog();

    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static ErrorLog* mInstance;
    SubjectPtr<I32VectorDataPoint*> mpErrorLog;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};
#endif // __cplusplus

#endif
