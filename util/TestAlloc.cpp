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
/* CLASS NAME       : TestAlloc                                             */
/*                                                                          */
/* FILE NAME        : TestAlloc.cpp                                         */
/*                                                                          */
/* CREATED DATE     : 28-04-2008 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : See h-file.                                     */
/****************************************************************************/
/*****************************************************************************
  SYSTEM INCLUDES
 *****************************************************************************/
#include <stdlib.h>

/*****************************************************************************
  PROJECT INCLUDES
 *****************************************************************************/
#include <TestAlloc.h>

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/

/*****************************************************************************
  DEFINES
 *****************************************************************************/

//#define TEST_USING_NEW_DELETE
//#define TEST_USING_C_MALLOC_FREE
#define TEST_USING_OS_MALLOC_FREE

#define MAX_BLOCK_SIZE  2*1024
#define MIN_BLOCK_SIZE  16


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
 * DESCRIPTION:
 *
 *****************************************************************************/
TestAlloc::TestAlloc(void)
{
  mAllocated = 0;
  mFirstBlock = NULL;
  mLastBlock = NULL;
}

/*****************************************************************************
 * Function - Destructor
 * DESCRIPTION:  Default
 *
 ****************************************************************************/
TestAlloc::~TestAlloc(void)
{
}

/*****************************************************************************
 * Function - AllocBlock
 * DESCRIPTION: Allocates one block of bytes and put it in the chain.
 *
 * Return:      True if 'bytes' were allocated, false otherwise
 *
 *****************************************************************************/
bool TestAlloc::AllocBlock(int bytes)
{
  #define MALLOC_OVERHEAD 8
  if (bytes < sizeof(BLOCK)+MALLOC_OVERHEAD)
  {
    bytes = sizeof(BLOCK)+MALLOC_OVERHEAD;
  }

  #if defined TEST_USING_NEW_DELETE
  char* p_block = new char[bytes-MALLOC_OVERHEAD];
  #elif defined TEST_USING_C_MALLOC_FREE
  char* p_block = (char*)malloc(bytes-MALLOC_OVERHEAD);
  #elif defined TEST_USING_OS_MALLOC_FREE
  char* p_block = (char*)OS_malloc(bytes-MALLOC_OVERHEAD);
  #endif

  if (p_block != NULL)
  {
    if (mFirstBlock == NULL)   //nothing allocated yet
    {
      mFirstBlock = (BLOCK*)p_block;
      mLastBlock  = mFirstBlock;
    }
    else
    {
      mLastBlock->next_block = (BLOCK*)p_block;
      mLastBlock = mLastBlock->next_block; //extend the chain
    }
    mLastBlock->next_block  = NULL;
    mLastBlock->memory_used = bytes;
  }

  return (p_block != NULL);
}

/*****************************************************************************
 * Function - Alloc
 * DESCRIPTION: Allocates a number of bytes by use of AllocBlock.
 *              If required, the block size is reduced to try to get the small
 *              chunks of fragmented memory (if anything available)
 *
 * Return:      The number of bytes allocated
 *
 *****************************************************************************/
int TestAlloc::Alloc(int bytes)
{
  int block_size  = MAX_BLOCK_SIZE;
  int block_count = 0;

  if (bytes < 0)
  {
    bytes = 0x7FFFFFFF;
  }
  if (block_size > bytes)
  {
    block_size = bytes;
  }
  mAllocated = 0;

  while (block_size >= MIN_BLOCK_SIZE)
  {
    while (mAllocated+block_size <= bytes && AllocBlock(block_size) == true)
    {
      mAllocated += block_size;
      block_count++;
    }
    block_size /= 2;
    if (block_size < 2*MIN_BLOCK_SIZE && block_size > MIN_BLOCK_SIZE)
    {
      block_size = MIN_BLOCK_SIZE;
    }
  }

  return mAllocated;
}

/*****************************************************************************
 * Function - DeAlloc
 * DESCRIPTION: Deallocates all the memory allocated by Alloc and AllocBlock
 *
 * Return:      The number of bytes deallocated
 *
 *****************************************************************************/
int TestAlloc::DeAlloc()
{
  BLOCK* current_block = mFirstBlock;
  int memory_deallocated = 0;

  while (current_block != NULL)
  {
    char* del_block = (char*)current_block;
    memory_deallocated += current_block->memory_used;
    current_block = current_block->next_block;
    #if defined  TEST_USING_NEW_DELETE
    delete [] del_block;
    #elif defined TEST_USING_C_MALLOC_FREE
    free(del_block);
    #elif defined TEST_USING_OS_MALLOC_FREE
    OS_free(del_block);
    #endif
  }

  mAllocated = 0;
  mFirstBlock = NULL;
  mLastBlock = NULL;

  return memory_deallocated;
}

/*****************************************************************************
 * Function - CheckMemoryAvailable
 * DESCRIPTION: Allocates and DeAllocates a number of bytes
 *
 * Return:      The number of bytes that could be allocated
 *
 *****************************************************************************/
int TestAlloc::CheckMemoryAvailable(int bytes)
{
  static int old_memory = 0;
  static int difference = 0;
  static int diff_count = 0;

  int memory_allocated = Alloc(bytes);
  int memory_deallocated = DeAlloc();

  if (memory_allocated != memory_deallocated)
  {
    memory_deallocated = 0; // Realy bad, should not happen
  }

  difference = memory_allocated - old_memory;
  if (difference != 0)
  {
    // Ok in case of call from different tasks or if all memory is used
    diff_count++;
    old_memory = memory_allocated;
  }

  return memory_allocated;
}

/*****************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 ****************************************************************************/
