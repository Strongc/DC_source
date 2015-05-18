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
/* CLASS NAME       : IoChannelConfig                                       */
/*                                                                          */
/* FILE NAME        : IoChannelConfig.h                                     */
/*                                                                          */
/* CREATED DATE     : 12-12-2008  (dd-mm-yyyy)                              */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __IoChannelConfig_h__
#define __IoChannelConfig_h__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <Subject.h>
#include <FloatDataPoint.h>
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define DEFAULT_AI_LIMIT_MIN    0.0f
#define DEFAULT_AI_LIMIT_MAX  100.0f
#define DEFAULT_AI_LIMIT_VALUE  0.0f
#define MAX_CHANNEL_PREFIX_LEN 5
/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
/*****************************************************************************
 * CLASS: IoChannelConfig
 *****************************************************************************/
class IoChannelConfig : public Subject
{

public:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    IoChannelConfig();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    virtual ~IoChannelConfig();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/
    /********************************************************************
    OPERATIONS
    ********************************************************************/

public:

    CHANNEL_SOURCE_DATATYPE_TYPE GetSourceType(void);
    CHANNEL_SOURCE_TYPE GetSource(void);
    U8  GetSourceIndex(void);
    FloatDataPoint* GetAiLimit(void);
    bool GetInverted(void);
    U32  GetResponseTime(void);
    U32  GetTimerHighPeriod(void);
    U32  GetTimerLowPeriod(void);
    bool GetConstantValue(void);
    const char* GetChannelPrefix(void);

    bool SetSourceType(CHANNEL_SOURCE_DATATYPE_TYPE sourceDataType);
    bool SetSource(CHANNEL_SOURCE_TYPE source);
    bool SetSourceIndex(U8 index);
    bool SetAiLimitRange(float min, float max, QUANTITY_TYPE quantity);
    bool SetAiLimitRange(FloatDataPoint* pLimit);
    bool SetAiLimitValue(float limit);
    bool SetInverted(bool invert);
    bool SetResponseTime(U32 timeInSeconds);
    bool SetTimerHighPeriod(U32 timeInSeconds);
    bool SetTimerLowPeriod(U32 timeInSeconds);
    bool SetConstantValue(bool value);
    void SetChannelPrefix(char* pPrefix);

    bool CopyValues(IoChannelConfig* pSource);

    bool IsSourceChanged(void);

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
    bool IndexIsValid(U8 index);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    CHANNEL_SOURCE_DATATYPE_TYPE mSourceDataType;
    CHANNEL_SOURCE_TYPE mSource;
    U8 mSourceIndex;
    MEASURED_VALUE_TYPE mAiFunc;
    FloatDataPoint* mAiLimit;
    DIGITAL_INPUT_FUNC_TYPE mDiFunc;
    bool mInvert;
    U32 mResponseTime;
    U32 mTimerHigh;
    U32 mTimerLow;
    char mChannelPrefix[MAX_CHANNEL_PREFIX_LEN];

    bool mSourceChanged;
   
};

#endif
