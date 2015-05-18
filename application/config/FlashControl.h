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
/* CLASS NAME       : FlashControl                                          */
/*                                                                          */
/* FILE NAME        : FlashControl.h                                        */
/*                                                                          */
/* CREATED DATE     : 07-11-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __FLASH_CONTROL_H__
#define __FLASH_CONTROL_H__

/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <cu351_cpu_types.h>

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
  // Note: This list is used as an index
  FIRST_FLASH_CONTROL_BLOCK_ID    = 0,
  FLASH_CONTROL_BLOCK_ID_CONFIG   = FIRST_FLASH_CONTROL_BLOCK_ID,
  FLASH_CONTROL_BLOCK_ID_LOG      = 1,
  FLASH_CONTROL_BLOCK_ID_GSC      = 2,
  FLASH_CONTROL_BLOCK_ID_NO_BOOT  = 3,
  FLASH_CONTROL_BLOCK_ID_USER_LOG_1 = 4,
  FLASH_CONTROL_BLOCK_ID_USER_LOG_2 = 5,
  // NOTE: The highest possible id of a log/config blocks is 6, i.e. max 7 blocks
  NO_OF_FLASH_CONTROL_BLOCK_ID,
  LAST_FLASH_CONTROL_BLOCK_ID = NO_OF_FLASH_CONTROL_BLOCK_ID-1
} FLASH_CONTROL_BLOCK_ID_TYPE;

/*****************************************************************************
 * CLASS:
 * DESCRIPTION:
 *
 *****************************************************************************/
class FlashControl
{
  public:
    static FlashControl* GetInstance();

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool SaveData(U8* pBuf, int count, FLASH_CONTROL_BLOCK_ID_TYPE blockId);
    bool ReadData(U8* pBuf, int offset, int count, FLASH_CONTROL_BLOCK_ID_TYPE blockId);
    bool IsBlockValid(FLASH_CONTROL_BLOCK_ID_TYPE blockId);
    bool EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_TYPE blockId);
    U32 ReadSwVersionNo();
    void WriteSwVersionNo(U32 swNo);
    void Init();

  private:
    /********************************************************************
    LIFECYCLE - Default constructor.
    ********************************************************************/
    FlashControl();
    /********************************************************************
    LIFECYCLE - Destructor.
    ********************************************************************/
    ~FlashControl();

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    bool FindValidBlock(FLASH_CONTROL_BLOCK_ID_TYPE blockId, int* pValidBlock);
    int  FindNextBlock(FLASH_CONTROL_BLOCK_ID_TYPE blockId, int validBlock);
    bool WriteValidCode(FLASH_CONTROL_BLOCK_ID_TYPE blockId, int block);

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    static FlashControl *mInstance;
    U32 mSwVersionNo;
    bool OS_started;

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

    /********************************************************************
    ATTRIBUTE
    ********************************************************************/
};

#endif
