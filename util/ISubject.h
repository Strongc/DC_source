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
/* CLASS NAME       : ISubject                                              */
/*                                                                          */
/* FILE NAME        : ISubject.h                                            */
/*                                                                          */
/* CREATED DATE     : 23-07-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Subject interface                               */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __ISUBJECT_H__
#define __ISUBJECT_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
  FORWARD DECLARATIONS
 *****************************************************************************/
class Observer;

/*****************************************************************************
 * INTERFACE: ISubject
 * DESCRIPTION:
 *****************************************************************************/
class ISubject
{
protected:
  ISubject() {};
  
public:
  virtual ~ISubject() {};
  
  virtual SUBJECT_ID_TYPE GetSubjectId(void) = 0;	
		
  virtual SUBJECT_TYPE GetSubjectType(void) = 0;	
  
  virtual int GetRefCount(void) = 0;
		
  virtual void Subscribe(Observer* pObserver) = 0;	
  virtual void SubscribeE(Observer* pObserver) = 0;	
  virtual void Unsubscribe(Observer* pObserver) = 0;	
};

#endif
