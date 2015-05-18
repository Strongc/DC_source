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
/* CLASS NAME       : Subject                                               */
/*                                                                          */
/* FILE NAME        : Subject.cpp                                           */
/*                                                                          */
/* CREATED DATE     : 23-07-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : see h-file                                      */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <rtos.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <Subject.h>
#include <Observer.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION: This is the constructor for the class, to construct
 * an object of the class type
 *****************************************************************************/
Subject::Subject(void)
{
  mId = (SUBJECT_ID_TYPE)-1;  // -1 -> not set yet, see SetSubjectId
  mType = (SUBJECT_TYPE)-1;   // -1 -> not set yet, see SetSubjectType
  mRefCount = 0;
  mDestroyed = false;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION: This is the destructor, to destruct an object.
 *
*****************************************************************************/
Subject::~Subject(void)
{
  OS_EnterRegion();
  mDestroyed = true;
  OS_LeaveRegion();

  std::vector<Observer*>::iterator iter;

  for (iter = mObserverList.begin(); iter != mObserverList.end(); ++iter)
  {
      (*iter)->SubscribtionCancelled(this);
  }
  for (iter = mObserverListE.begin(); iter != mObserverListE.end(); ++iter)
  {
      (*iter)->SubscribtionCancelled(this);
  }
}

/*****************************************************************************
 * Function - GetSubjectId
 * DESCRIPTION:
*****************************************************************************/
SUBJECT_ID_TYPE Subject::GetSubjectId(void)
{
  return mId;
}

/*****************************************************************************
 * Function - SetSubjectId
 * DESCRIPTION:
*****************************************************************************/
void Subject::SetSubjectId(SUBJECT_ID_TYPE id)
{
  if (mId == -1)
  {
    mId = id;
  }
  else
  {
    FatalErrorOccured("Subject: Setting ID twice!");
    // FATAL ERROR: somebody is trying to set the subject Id again !?!
  }
}

/*****************************************************************************
 * Function - GetSubjectType
 * DESCRIPTION:
*****************************************************************************/
SUBJECT_TYPE Subject::GetSubjectType(void)
{
  if (mType != -1)
  {
    return mType;
  }
  else
  {
    FatalErrorOccured("Subject: No subject type!");
    return (SUBJECT_TYPE) -1;// FATAL ERROR: subject type NOT set !?!
  }
}

/*****************************************************************************
 * Function - SetSubjectType
 * DESCRIPTION:
*****************************************************************************/
void Subject::SetSubjectType(SUBJECT_TYPE type)
{
  if (mType == -1)
  {
    mType = type;
  }
  else
  {
    FatalErrorOccured("Subject: Setting type twice!");
    // FATAL ERROR: somebody is trying to set the subject type again !?!
  }
}

/*****************************************************************************
 * Function - IncRefCount
 * DESCRIPTION: Increments the number of references to this subject
*****************************************************************************/
void Subject::IncRefCount()
{
	mRefCount++;
}

/*****************************************************************************
 * Function - GetRefCount
 * DESCRIPTION: Returns the number of references to this subject
 *
*****************************************************************************/
int Subject::GetRefCount()
{
	return mRefCount;
}

/*****************************************************************************
 * Function - Subscribe
 * DESCRIPTION:
 *
*****************************************************************************/
void Subject::Subscribe(Observer* pObserver)
{
  OS_EnterRegion();

  if (mDestroyed)
  {
    FatalErrorOccured("Subject: destroyed 1!");
    // fatal error, subject is destroyed!!
  }

  mObserverList.push_back(pObserver);

  OS_LeaveRegion();
}

/*****************************************************************************
 * Function - Subscribe
 * DESCRIPTION:
 *
*****************************************************************************/
void Subject::SubscribeE(Observer* pObserver)
{
  OS_EnterRegion();

  if (mDestroyed)
  {
    FatalErrorOccured("Subject: destroyed 2!");
    // fatal error, subject is destroyed!!
  }

  mObserverListE.push_back(pObserver);

  OS_LeaveRegion();
}

/*****************************************************************************
 * Function - UnSubscribe
 * DESCRIPTION:
 *
*****************************************************************************/
void Subject::Unsubscribe(Observer* pObserver)
{
  OS_EnterRegion();

  if (mDestroyed)
  {
    FatalErrorOccured("Subject: destroyed 3!");
  }

  std::vector<Observer*>::iterator iter;

  for (iter = mObserverList.begin(); iter != mObserverList.end(); ++iter)
  {
    if((*iter) == pObserver)
    {
      mObserverList.erase(iter);
      break;
    }
  }

  for (iter = mObserverListE.begin(); iter != mObserverListE.end(); ++iter)
  {
    if((*iter) == pObserver)
    {
      mObserverListE.erase(iter);
      break;
    }
  }

  OS_LeaveRegion();
}

/*****************************************************************************
 * Function - NotifyObservers
 * DESCRIPTION:
 *
*****************************************************************************/
void Subject::NotifyObservers(void)
{
  std::vector<Observer*>::iterator iter;

  OS_EnterRegion();
  for (iter = mObserverList.begin(); iter != mObserverList.end(); ++iter)
  {
      (*iter)->Update(this);
  }
  OS_LeaveRegion();
}

/*****************************************************************************
 * Function - NotifyObserversE
 * DESCRIPTION:
 *
*****************************************************************************/
void Subject::NotifyObserversE(void)
{
  std::vector<Observer*>::iterator iter;

  OS_EnterRegion();
  for (iter = mObserverListE.begin(); iter != mObserverListE.end(); ++iter)
  {
    (*iter)->Update(this);
  }
  OS_LeaveRegion();
}

FLASH_ID_TYPE Subject::GetFlashId(void)
{
  FatalErrorOccured("Subject: missing function 1!");
	return (FLASH_ID_TYPE) -1;// FATAL ERROR: must be overridden in derived class and assigned an UNIQUE id
}

void Subject::SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
{
  FatalErrorOccured("Subject: missing function 2!");
	// FATAL ERROR: must be overridden in derived class
}

void Subject::LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
{
  FatalErrorOccured("Subject: missing function 3!");
	// FATAL ERROR: must be overridden in derived class
}
