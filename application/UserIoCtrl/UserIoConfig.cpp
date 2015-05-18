/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: WW Midrange                                      */
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
/* CLASS NAME       : UserIoConfig                                          */
/*                                                                          */
/* FILE NAME        : UserIoConfig.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 12-12-2008 (dd-mm-yyyy)                               */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
#include <Factory.h>

/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/
#include "UserIoConfig.h"

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
* DESCRIPTION: Initialises private data.
*
*****************************************************************************/
UserIoConfig::UserIoConfig(USER_IO_TYPE destination)
{
  mEnabled = false;
  mLogic = USER_IO_LOGIC_AND;
  mInvert = false;
  mMinHoldTime = 0;
  mMaxHoldTime = 0;
  mMaxHoldTimeEnabled = false;
  mDestination = destination;

  switch(destination)
  {
  case USER_IO_1:
    mFirstSource = USER_FUNC_SOURCE_1_1;
    mSecondSource = USER_FUNC_SOURCE_1_2;
    break;
  case USER_IO_2:
    mFirstSource = USER_FUNC_SOURCE_2_1;
    mSecondSource = USER_FUNC_SOURCE_2_2;
    break;
  case USER_IO_3:
    mFirstSource = USER_FUNC_SOURCE_3_1;
    mSecondSource = USER_FUNC_SOURCE_3_2;
    break;
  case USER_IO_4:
    mFirstSource = USER_FUNC_SOURCE_4_1;
    mSecondSource = USER_FUNC_SOURCE_4_2;
    break;
  case USER_IO_5:
    mFirstSource = USER_FUNC_SOURCE_5_1;
    mSecondSource = USER_FUNC_SOURCE_5_2;
    break;
  case USER_IO_6:
    mFirstSource = USER_FUNC_SOURCE_6_1;
    mSecondSource = USER_FUNC_SOURCE_6_2;
    break;
  case USER_IO_7:
    mFirstSource = USER_FUNC_SOURCE_7_1;
    mSecondSource = USER_FUNC_SOURCE_7_2;
    break;
  case USER_IO_8:
    mFirstSource = USER_FUNC_SOURCE_8_1;
    mSecondSource = USER_FUNC_SOURCE_8_2;
    break;

  }
}

/*****************************************************************************
* Function - Destructor
* DESCRIPTION: -
*
****************************************************************************/
UserIoConfig::~UserIoConfig()
{
}

/*****************************************************************************
* Function - GetEnabled
* DESCRIPTION: Returns if enabled or not
*
*****************************************************************************/
bool UserIoConfig::GetEnabled()
{
  return mEnabled;
}

/*****************************************************************************
* Function - GetFirstSourceIndex
* DESCRIPTION: Returns index of first I/O-Channel
*
*****************************************************************************/
USER_FUNC_SOURCE_TYPE UserIoConfig::GetFirstSourceIndex()
{
  return mFirstSource;
}

/*****************************************************************************
* Function - GetSecondSourceIndex
* DESCRIPTION: Returns index of second I/O-Channel
*
*****************************************************************************/
USER_FUNC_SOURCE_TYPE UserIoConfig::GetSecondSourceIndex()
{
  return mSecondSource;
}

/*****************************************************************************
* Function - GetLogic
* DESCRIPTION: Returns logic to apply to both I/O channels
*
*****************************************************************************/
USER_IO_LOGIC_TYPE UserIoConfig::GetLogic()
{
  return mLogic;
}

/*****************************************************************************
* Function - GetInverted
* DESCRIPTION: Returns if output should be inverted (before hold-delay-time)
*
*****************************************************************************/
bool UserIoConfig::GetInverted()
{
  return mInvert;
}

/*****************************************************************************
* Function - GetMinHoldTime
* DESCRIPTION: Returns hold-delay-time in seconds
*
*****************************************************************************/
U32 UserIoConfig::GetMinHoldTime()
{
  return mMinHoldTime;
}

/*****************************************************************************
* Function - GetMaxHoldTime
* DESCRIPTION: Returns hold-delay-time in seconds
*
*****************************************************************************/
U32 UserIoConfig::GetMaxHoldTime()
{
  return mMaxHoldTime;
}

/*****************************************************************************
* Function - GetMaxHoldTimeEnabled
* DESCRIPTION: Return true if max hold time is to be used
*
*****************************************************************************/
bool UserIoConfig::GetMaxHoldTimeEnabled()
{
  return mMaxHoldTimeEnabled;
}

/*****************************************************************************
* Function - GetDestination
* DESCRIPTION: Returns type of destination
*
*****************************************************************************/
USER_IO_TYPE UserIoConfig::GetDestination()
{
  return mDestination;
}

/*****************************************************************************
* Function - SetEnabled
* DESCRIPTION: Sets enabled
*
*****************************************************************************/
bool UserIoConfig::SetEnabled(bool enabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mEnabled != enabled)
  {
    mEnabled = enabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetLogic
* DESCRIPTION: Sets logic operator
*
*****************************************************************************/
bool UserIoConfig::SetLogic(USER_IO_LOGIC_TYPE logic)
{
  bool notify = false;

  OS_EnterRegion();
  if (mLogic != logic)
  {
    mLogic = logic;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetInverted
* DESCRIPTION: Sets inverted (before hold-delay-time)
*
*****************************************************************************/
bool UserIoConfig::SetInverted(bool invert)
{
  bool notify = false;

  OS_EnterRegion();
  if (mInvert != invert)
  {
    mInvert = invert;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetMinHoldTime
* DESCRIPTION: Sets hold-delay-time value in seconds
*
*****************************************************************************/
bool UserIoConfig::SetMinHoldTime(U32 timeInSeconds)
{
  bool notify = false;

  OS_EnterRegion();
  if (mMinHoldTime != timeInSeconds)
  {
    mMinHoldTime = timeInSeconds;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetMaxHoldTime
* DESCRIPTION: Sets hold-delay-time value in seconds
*
*****************************************************************************/
bool UserIoConfig::SetMaxHoldTime(U32 timeInSeconds)
{
  bool notify = false;

  OS_EnterRegion();
  if (mMaxHoldTime != timeInSeconds)
  {
    mMaxHoldTime = timeInSeconds;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}


/*****************************************************************************
* Function - SetMaxHoldTimeEnabled
* DESCRIPTION: Sets max hold time enabled
*
*****************************************************************************/
bool UserIoConfig::SetMaxHoldTimeEnabled(bool enabled)
{
  bool notify = false;

  OS_EnterRegion();
  if (mMaxHoldTimeEnabled != enabled)
  {
    mMaxHoldTimeEnabled = enabled;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* Function - SetDestination
* DESCRIPTION: Sets destination
*
*****************************************************************************/
bool UserIoConfig::SetDestination(USER_IO_TYPE destination)
{
  bool notify = false;

  OS_EnterRegion();
  if (mDestination != destination)
  {
    mDestination = destination;
    notify = true;
  }
  OS_LeaveRegion();

  if (notify)
    NotifyObservers();

  NotifyObserversE();

  return notify;
}

/*****************************************************************************
* subject::GetFlashId implementation
*****************************************************************************/
FLASH_ID_TYPE UserIoConfig::GetFlashId(void)
{
  return FLASH_ID_USER_IO_CONFIG;
}

/*****************************************************************************
* subject::SaveToFlash implementation
*****************************************************************************/
void UserIoConfig::SaveToFlash(IFlashWriter* pWriter, FLASH_SAVE_TYPE save)
{
  // Write a version code of the data type
  pWriter->WriteU8(2);

  switch (save)
  {
  case FLASH_SAVE_VALUE:
  case FLASH_SAVE_ALL:
    pWriter->WriteI16((I16)mLogic);
    pWriter->WriteBool(mInvert);
    pWriter->WriteU32(mMinHoldTime);
    pWriter->WriteU32(mMaxHoldTime);
    pWriter->WriteI16((I16)mDestination);
    pWriter->WriteBool(mEnabled);
    pWriter->WriteBool(mMaxHoldTimeEnabled);
    break;
  }
}

/*****************************************************************************
* subject::LoadFromFlash implementation
*****************************************************************************/
void UserIoConfig::LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs)
{
  const U8 version = pReader->ReadU8(0);

  if (version != 2)
  {
    // Nothing, unknown version code for the data type
  }
  else switch (savedAs)
  {
  case FLASH_SAVE_VALUE:
  case FLASH_SAVE_ALL:
    SetLogic((USER_IO_LOGIC_TYPE) pReader->ReadI16(mLogic));
    SetInverted(pReader->ReadBool(mInvert));
    SetMinHoldTime(pReader->ReadU32(mMinHoldTime));
    SetMaxHoldTime(pReader->ReadU32(mMaxHoldTime));
    SetDestination((USER_IO_TYPE) pReader->ReadI16(mDestination));
    SetEnabled(pReader->ReadBool(mEnabled));
    SetMaxHoldTimeEnabled(pReader->ReadBool(mMaxHoldTimeEnabled));
    break;
  }
}


/*****************************************************************************
*
*
*              PRIVATE FUNCTIONS
*
*
****************************************************************************/

/*****************************************************************************
*
*
*              PROTECTED FUNCTIONS
*                 - RARE USED -
*
****************************************************************************/
