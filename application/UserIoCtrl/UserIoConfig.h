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
/* FILE NAME        : UserIoConfig.h                                        */
/*                                                                          */
/* CREATED DATE     : 12-12-2008  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __UserIoConfig_h__
#define __UserIoConfig_h__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
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
 * CLASS: UserIoConfig
 *****************************************************************************/
class UserIoConfig : public Subject
{
public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    UserIoConfig(USER_IO_TYPE destination);
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~UserIoConfig();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/

public:
    bool GetEnabled(void);
    USER_FUNC_SOURCE_TYPE GetFirstSourceIndex(void);
    USER_FUNC_SOURCE_TYPE GetSecondSourceIndex(void);
    USER_IO_LOGIC_TYPE GetLogic(void);
    bool GetInverted(void);
    U32  GetMinHoldTime(void);
    U32  GetMaxHoldTime(void);
    bool GetMaxHoldTimeEnabled(void);
    USER_IO_TYPE GetDestination(void);

    bool SetEnabled(bool enabled);
    bool SetLogic(USER_IO_LOGIC_TYPE logic);
    bool SetInverted(bool invert);
    bool SetMinHoldTime(U32 timeInSeconds);
    bool SetMaxHoldTime(U32 timeInSeconds);
    bool SetMaxHoldTimeEnabled(bool enabled);
    bool SetDestination(USER_IO_TYPE destination);
    
    virtual FLASH_ID_TYPE GetFlashId(void);
  	virtual void SaveToFlash(IFlashWriter* pWrite, FLASH_SAVE_TYPE save);
  	virtual void LoadFromFlash(IFlashReader* pReader, FLASH_SAVE_TYPE savedAs);

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/
    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    bool mEnabled;
    USER_FUNC_SOURCE_TYPE mFirstSource;
    USER_FUNC_SOURCE_TYPE mSecondSource;
    USER_IO_LOGIC_TYPE mLogic;
    bool mInvert;
    U32  mMinHoldTime;
    U32  mMaxHoldTime;
    bool mMaxHoldTimeEnabled;
    USER_IO_TYPE mDestination;
   
};

#endif
