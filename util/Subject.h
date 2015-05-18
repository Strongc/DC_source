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
/* CLASS NAME       : Subject                                              */
/*                                                                          */
/* FILE NAME        : Subject.h                                             */
/*                                                                          */
/* CREATED DATE     : 23-07-2004  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION : Implementation of Observer pattern              */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __SUBJECT_H__
#define __SUBJECT_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <vector>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <AppTypeDefs.h>
#include <FactoryTypes.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <ISubject.h>

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
class MpcTime;

/*****************************************************************************
 * CLASS: IFlashWriter
 * DESCRIPTION: Flash block writer interface
 *****************************************************************************/
class IFlashWriter
{
public:
	virtual void WriteBool(const bool value) = 0;
	virtual void WriteU8(const U8 value) = 0;
	virtual void WriteU16(const U16 value) = 0;
	virtual void WriteI16(const I16 value) = 0;
	virtual void WriteU32(const U32 value) = 0;
	virtual void WriteI32(const I32 value) = 0;
	virtual void WriteFloat(const float value) = 0;
	virtual void WriteDouble(const double value) = 0;
	virtual void WriteString(const char* szValue, int maxLen) = 0;
  virtual void WriteMpcTime(MpcTime* pTime) = 0;
};

/*****************************************************************************
 * CLASS: IFlashReader
 * DESCRIPTION: Flash block reader interface
 *****************************************************************************/
class IFlashReader
{
public:
	virtual bool ReadBool(const bool defaultValue) = 0;
	virtual U8 ReadU8(const U8 defaultValue) = 0;
	virtual U16 ReadU16(const U16 defaultValue) = 0;
	virtual I16 ReadI16(const I16 defaultValue) = 0;
	virtual U32 ReadU32(const U32 defaultValue) = 0;
	virtual I32 ReadI32(const I32 defaultValue) = 0;
	virtual float ReadFloat(const float defaultValue) = 0;
	virtual double ReadDouble(const double defaultValue) = 0;
	virtual void ReadString(char* szValue, int maxLen, const char* szDefaultValue) = 0;
  virtual void ReadMpcTime(MpcTime* pTime) = 0;
};

/*****************************************************************************
 * CLASS: Subject
 * DESCRIPTION:
 *****************************************************************************/
class Subject : public virtual ISubject
{
protected:
  /********************************************************************
  LIFECYCLE - Default constructor.
  ********************************************************************/
  Subject();
		
  /********************************************************************
  LIFECYCLE - Destructor.
  ********************************************************************/
  virtual ~Subject(void);

  /********************************************************************
  ASSIGNMENT OPERATOR
  ********************************************************************/

  /********************************************************************
  OPERATIONS
  ********************************************************************/
public:
  virtual void SetSubjectId(SUBJECT_ID_TYPE id);
  virtual SUBJECT_ID_TYPE GetSubjectId(void);

  virtual SUBJECT_TYPE GetSubjectType(void);	
  virtual void SetSubjectType(SUBJECT_TYPE type);	
  
	void IncRefCount();	// may ONLY be called by the SubjectPtr::Attach function
  int GetRefCount();
	
  virtual void Subscribe(Observer* pObserver);
  virtual void SubscribeE(Observer* pObserver);
  virtual void Unsubscribe(Observer* pObserver);

	// flash operations
	virtual FLASH_ID_TYPE GetFlashId(void);
	virtual void SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save);
	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs);

protected:
  virtual void NotifyObservers(void);
  virtual void NotifyObserversE(void);

private:
  /********************************************************************
  OPERATIONS
  ********************************************************************/

  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  std::vector<Observer*> mObserverList;
  std::vector<Observer*> mObserverListE;

protected:
  /********************************************************************
  OPERATIONS
  ********************************************************************/

  /********************************************************************
  ATTRIBUTE
  ********************************************************************/
  SUBJECT_ID_TYPE mId;
  SUBJECT_TYPE mType;
  int mRefCount;
  bool mDestroyed;
};


#endif
