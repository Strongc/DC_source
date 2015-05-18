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
/* FILE NAME        : FlashControl.cpp                                      */
/*                                                                          */
/* CREATED DATE     : 07-11-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <c1648.h>
#include <ConfigControl.h>
#include <GPio.h>
#include <rtos.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <FlashControl.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/
#define VALID_CODE                      0x361DC361 // unique bit pattern for valid code

#define SW_VERSION_NO_SIZE              2 // 2 = 32 bit
#define VALID_BLOCK_ID_SIZE             2 // 2 = 32 bit

#define CONFIG_VALID_BLOCK_ID_OFFSET    0
#define SW_VERSION_NO_OFFSET            CONFIG_VALID_BLOCK_ID_OFFSET + VALID_BLOCK_ID_SIZE

#define LOG_VALID_BLOCK_ID_OFFSET       0

#define GSC_VALID_BLOCK_ID_OFFSET       0

#define NO_BOOT_VALID_BLOCK_ID_OFFSET   0

#define USER_LOG_VALID_BLOCK_ID_OFFSET  0

#define BUF_START_OFFSET                SW_VERSION_NO_OFFSET + SW_VERSION_NO_SIZE

#define RETRY_CNT                       5     // No of times to retry before give up flash read/write
#define LO0PS_FOR_1MS                   17500 // Value that makes the ms-loop belov wait for approx. 1 ms
#define LOOP_DELAY(ms)                  { for (int t=0;t<ms;t++) for (int i=0;i<LO0PS_FOR_1MS;i++){i++;i--;} }
#define MAKE_DELAY(ms)                  { if (OS_started) {OS_Delay(ms);} else {LOOP_DELAY(ms);} }
                                        // Steps in RESET_FLASH
                                        // 1. Reset is active low
                                        // 2. Secure pulse is long enough
                                        // 3. Release reset pin
                                        // 4. Just wait
#define RESET_FLASH                     { GPio::GetInstance()->SetAs(GPIO38, OUTPUT, 0);    \
                                          MAKE_DELAY(1);                                    \
                                          GPio::GetInstance()->Set(GPIO38, 1);              \
                                          MAKE_DELAY(100);                                  \
                                        }


/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/
typedef struct
{
  int Block1;
  int Block2;
} BLOCK_TAB_TYPE;

/*****************************************************************************
  CONST
 *****************************************************************************/
//TCSL - Define for the new size changes to the RAM and ROM
#ifdef _RAM_32_ROM_16_UPGRADE_
const BLOCK_TAB_TYPE BLOCK_TAB[] = {
// Save ID        Block1        Block2
/* CONFIG    */     98,            121,   // Bit pattern for test in block 0 - Boot loader in block 127
/* LOG       */     99,            122,
/* GSC       */     100,           123,
/* NO_BOOT   */     101,           124,
/* User log 1*/     102,           125,
/* User log 2*/     103,           126
};
#else
const BLOCK_TAB_TYPE BLOCK_TAB[] = {
// Save ID          Block1        Block2
/* CONFIG    */     1,            65,   // Bit pattern for test in block 0 - Boot loader in block 64
/* LOG       */     2,            66,
/* GSC       */     3,            67,
/* NO_BOOT   */     4,            68,
/* User log 1*/     5,            69,
/* User log 2*/     6,            70,
// NOTE: The highest allowed block number is 7/71 since the program (flash image) is located from block 8/72
};
#endif

const int VALID_BLOCK_ID_OFFSET_TAB[] = {
// Save ID
/* CONFIG    */     CONFIG_VALID_BLOCK_ID_OFFSET,
/* LOG       */     LOG_VALID_BLOCK_ID_OFFSET,
/* GSC       */     GSC_VALID_BLOCK_ID_OFFSET,
/* NO_BOOT   */     NO_BOOT_VALID_BLOCK_ID_OFFSET,
/* User log 1*/     USER_LOG_VALID_BLOCK_ID_OFFSET,
/* User log 2*/     USER_LOG_VALID_BLOCK_ID_OFFSET,
};


/*****************************************************************************
  CREATES AN OBJECT.
 ******************************************************************************/
FlashControl* FlashControl::mInstance = 0;

/*****************************************************************************
 *
 *
 *              PUBLIC FUNCTIONS
 *
 *
 *****************************************************************************/

/*****************************************************************************
 * Function - GetInstance
 * DESCRIPTION:
 *
 *****************************************************************************/
FlashControl* FlashControl::GetInstance()
{
    if (!mInstance)
    {
        mInstance = new FlashControl();
    }
    return mInstance;
}

/*****************************************************************************
 * Function - Init
 * DESCRIPTION: Find valid block and then delete the other block. This is done
 * because delete of the non valid block is done during power down. Maby there
 * is not enough time during power down to delete the block. Therefore it is
 * done during power on / init.
 *
 *****************************************************************************/
void FlashControl::Init()
{
  int block, del_block;

  for (int i = FIRST_FLASH_CONTROL_BLOCK_ID; i <= LAST_FLASH_CONTROL_BLOCK_ID; i++)
  {
    if (FindValidBlock((FLASH_CONTROL_BLOCK_ID_TYPE)i, &block))
    {
      del_block = FindNextBlock((FLASH_CONTROL_BLOCK_ID_TYPE)i, block);
      FlashBlockErase(del_block);
    }
    else
    {
      EraseFlashBlock((FLASH_CONTROL_BLOCK_ID_TYPE)i);          // delete boths blocks
    }
  }

  OS_started = true;
}

/*****************************************************************************
 * Function - SaveData
 * DESCRIPTION:
 *
 *****************************************************************************/
bool FlashControl::SaveData(U8* pBuf, int count, FLASH_CONTROL_BLOCK_ID_TYPE blockId)
{
  int write_block, erase_block, addr;
  bool save_ok = false;
  bool erase_ok = false;

  if ((count % 2) != 0)
  {
    return false;
  }

  FindValidBlock(blockId, &erase_block);
  write_block = FindNextBlock(blockId, erase_block);
  addr = BlockOffset[write_block] + BUF_START_OFFSET;

  for (int i = 0; i < RETRY_CNT; i++)
  {
    if (FlashProgram(addr, count / 2, (U16*)pBuf) == Flash_Success)
    {
      if (WriteValidCode(blockId, write_block))
      {
        save_ok = true;
      }
    }
    if (save_ok == true)
    {
      break; // Everything OK, leave the for loop
    }
    else
    {
      RESET_FLASH;  // Something failed - Reset flash and try again
      if (i == RETRY_CNT/2)
      {
        // Try to re-erase the WRITE block, but only once during the retries
        FlashBlockErase(write_block);
      }
    }
  }

  if (save_ok == true)
  {
    if (FlashBlockErase(erase_block) == Flash_Success)
    {
      erase_ok = true;
    }
    // Just try once more
    else if (FlashBlockErase(erase_block) == Flash_Success)
    {
      erase_ok = true;
    }
  }

  return(save_ok && erase_ok);
}

/*****************************************************************************
 * Function - ReadData
 * DESCRIPTION:
 *
 *****************************************************************************/
bool FlashControl::ReadData(U8* pBuf, int offset, int count, FLASH_CONTROL_BLOCK_ID_TYPE blockId)
{
  int block, addr;
  U16 data;
  bool return_value = false;

  if (((offset % 2) == 0) && ((count % 2) == 0) && FindValidBlock(blockId, &block))
  {
    addr = BlockOffset[block] + BUF_START_OFFSET + offset / 2;

    for (int i = 0; i < count / 2; i++)
    {
      data = FlashRead(addr + i);
      *((U16*)(pBuf + i * 2)) = data;
    }

    return_value = true;
  }

  return return_value;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool FlashControl::IsBlockValid(FLASH_CONTROL_BLOCK_ID_TYPE blockId)
{
  int block;
  bool return_value;

  if (FindValidBlock(blockId, &block))
  {
    return_value = true;
  }
  else
  {
    return_value = false;
  }
  return return_value;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool FlashControl::EraseFlashBlock(FLASH_CONTROL_BLOCK_ID_TYPE blockId)
{
  bool return_value = true;

  if (FlashBlockErase(BLOCK_TAB[blockId].Block1) != Flash_Success) return_value = false;
  if (FlashBlockErase(BLOCK_TAB[blockId].Block2) != Flash_Success) return_value = false;

  return return_value;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
U32 FlashControl::ReadSwVersionNo()
{
  U32 sw_no_flash;
  int block;
  int addr;

  FindValidBlock(FLASH_CONTROL_BLOCK_ID_CONFIG, &block);       // Version number is placed in config

  addr = BlockOffset[block] + SW_VERSION_NO_OFFSET;
  sw_no_flash = (U32)FlashRead(addr);
  sw_no_flash += (U32)(FlashRead(addr+1)<<16);

  return sw_no_flash;
}

/*****************************************************************************
 * Function - WriteSwVersionNo
 * DESCRIPTION: mSwVersionNo is written into flash in operation WriteValidCode
 *
 *****************************************************************************/
void FlashControl::WriteSwVersionNo(U32 swNo)
{
  mSwVersionNo = swNo;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/

/*****************************************************************************
 * Function - Constructor
 * DESCRIPTION:
 *
 *****************************************************************************/
FlashControl::FlashControl()
{
  mSwVersionNo = 0;
  OS_started = false;
  FlashInit();
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:
 *
 ****************************************************************************/
FlashControl::~FlashControl()
{

}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool FlashControl::FindValidBlock(FLASH_CONTROL_BLOCK_ID_TYPE blockId, int* pValidBlock)
{
  U32 code;
  int addr;
  bool return_value = false;

  for (int i = 0; i < RETRY_CNT; i++)
  {
    addr = BlockOffset[BLOCK_TAB[blockId].Block1] + VALID_BLOCK_ID_OFFSET_TAB[blockId];
    code = (U32)FlashRead(addr);
    code += (U32)(FlashRead(addr+1)<<16);
    if (code == VALID_CODE)
    {
      *pValidBlock = BLOCK_TAB[blockId].Block1;
      return_value = true;
    }
    else
    {
      addr = BlockOffset[BLOCK_TAB[blockId].Block2] + VALID_BLOCK_ID_OFFSET_TAB[blockId];
      code = (U32)FlashRead(addr);
      code += (U32)(FlashRead(addr+1)<<16);
      if (code == VALID_CODE)
      {
        *pValidBlock = BLOCK_TAB[blockId].Block2;
        return_value = true;
      }
    }

    if (return_value == true)
    {
      break; // Everything OK, leave the for loop
    }
    else
    {
      RESET_FLASH;  // Something failed - Reset flash and try again
    }
  }

  if (return_value == false)
  {
    *pValidBlock = BLOCK_TAB[blockId].Block2;  // flash is virgin or corrupted. (But return block 2 for next erase)
  }

  return return_value;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
int FlashControl::FindNextBlock(FLASH_CONTROL_BLOCK_ID_TYPE blockId, int validBlock)
{
  int next_block;

  if (BLOCK_TAB[blockId].Block1 == validBlock)
  {
    next_block = BLOCK_TAB[blockId].Block2;
  }
  else if (BLOCK_TAB[blockId].Block2 == validBlock)
  {
    next_block = BLOCK_TAB[blockId].Block1;
  }
  else
  {
    // Write to error log
  }
  return next_block;
}

/*****************************************************************************
 * Function
 * DESCRIPTION:
 *
 *****************************************************************************/
bool FlashControl::WriteValidCode(FLASH_CONTROL_BLOCK_ID_TYPE blockId, int block)
{
  U32 code;
  int addr, del_block;
  bool return_value = false;

  addr = BlockOffset[block] + SW_VERSION_NO_OFFSET;
  FlashSingleProgram(addr, (U16)(mSwVersionNo & 0x0000FFFF));
  FlashSingleProgram(addr+1, (U16)((mSwVersionNo & 0xFFFF0000)>>16));

  addr = BlockOffset[block] + VALID_BLOCK_ID_OFFSET_TAB[blockId];
  FlashSingleProgram(addr, (U16)(VALID_CODE & 0x0000FFFF));
  FlashSingleProgram(addr+1, (U16)((VALID_CODE & 0xFFFF0000)>>16));

  code = (U32)FlashRead(addr);
  code += (U32)(FlashRead(addr+1)<<16);
  if (code == VALID_CODE)
  {
    // Make old block invalid and check it
    del_block = FindNextBlock(blockId, block);
    addr = BlockOffset[del_block] + VALID_BLOCK_ID_OFFSET_TAB[blockId];
    FlashSingleProgram(addr, 0);
    FlashSingleProgram(addr+1, 0);
    code = (U32)FlashRead(addr);
    code += (U32)(FlashRead(addr+1)<<16);
    if (code == 0)
    {
      return_value = true;
    }
  }

  return return_value;
}

/*****************************************************************************
 *
 *
 *              PROTECTED FUNCTIONS
 *                 - RARE USED -
 *
 ****************************************************************************/
