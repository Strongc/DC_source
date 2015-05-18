/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MPC                                              */
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
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Redirects calls to new and delete to the thread safeOS_malloc and OS_free*/
/*                                                                          */
/****************************************************************************/

#include <RTOS.h>
#ifdef __PC__
  #include <windows.h>
  #include <crtdbg.h>
#include <stdlib.h>
#endif

#include <factory.h>                    // to get FatalErrorOccured

#ifndef MAX_PATH
#define MAX_PATH 260
#endif


/* --------------------------------------------------
* Memory leakage test -
* Define __MEMORY_LEAKAGE_TEST___ in the project
* to enable this test
* --------------------------------------------------*/
#ifdef __MEMORY_LEAKAGE_TEST___
int no_of_heap_blocks_allocated = 0;
#endif //__MEMORY_LEAKAGE_TEST___


//  #define MPC_BLOCK_ALLOC_SIZE (64-8) // If defined the size allocated by
                                        // the mpc_malloc is fitted to blocks
                                        // of size MPC_BLOCK_ALLOC_SIZE


//  #define MPC_TRACE_ALLOCATIONS       // If defined the number of bytes and blocks are summarized.
//  #define MPC_TRACE_MAX_BLOCKS  0xFFFF  // Max number of allocations.
                                          // Only used if MPC_TRACE_ALLOCATIONS is defined.


//  #define MPC_GUARD_BYTES       4     // Number of bytes to fill with the guard patter
                                        // if not defined guard is disabled.

//  #define MPC_GUARD_PATTERN     0xFC    // Guard pattern in memory
//  #define MPC_GUARD_TEST_ON_NEW         // If defined test for buffer overruns in operator new
//  #define MPC_GUARD_TEST_ON_DELETE      // If defined test for buffer overruns in operator delete



#ifdef MPC_TRACE_ALLOCATIONS
#include <functional>
#include <map>
#include <stdlib.h>

  struct MpcGuardArreas
  {
    void* pAllocationStart;
    #ifdef MPC_GUARD_BYTES
      void* pGuardBytesStart;
    #endif
    size_t   size;
    #ifdef __PC__
    char          sourceFile[MAX_PATH + 30];
    unsigned int  line;
    #endif
    MpcGuardArreas()
    {
      pAllocationStart = NULL;
      #ifdef MPC_GUARD_BYTES
        pGuardBytesStart = NULL;
      #endif
      size = 0;
      #ifdef __PC__
      sourceFile[0] = 0;
      line = 0;
      #endif
    }
  };

  MpcGuardArreas* g_mpc_heap_allocations = NULL;
  size_t          g_mpc_total_bytes_allocated = 0;
  unsigned int    g_mpc_dump_leak_start = 0;

  #ifdef __PC__
  template<class Type>
  struct str_cmp : public std::binary_function <Type, Type, bool>
  {
    bool operator()(const Type& _Left, const Type& _Right) const
    {
      return strcmp(_Left, _Right) != 0;
    }
  };
  void dump_leaks()
  {
    typedef std::map<const char*, int, str_cmp<const char*> > MyMap;
    unsigned int key_counter = 0;
    MyMap  leak_map;
    MpcGuardArreas* p_search_for;
    unsigned int i = g_mpc_dump_leak_start;

    for(; i < MPC_TRACE_MAX_BLOCKS; ++i)
    {
      p_search_for = &g_mpc_heap_allocations[i];
      if(p_search_for->pAllocationStart != NULL)
      {
        MyMap::iterator iter = leak_map.find(p_search_for->sourceFile);
        if(iter != leak_map.end())
          continue;
        leak_map[p_search_for->sourceFile] = 1;
        for(int j = i + 1; j <  MPC_TRACE_MAX_BLOCKS; ++j)
        {
          if( g_mpc_heap_allocations[j].pAllocationStart != NULL &&
            g_mpc_heap_allocations[j].line != 0 &&
            g_mpc_heap_allocations[j].line == p_search_for->line &&
            strcmp(g_mpc_heap_allocations[j].sourceFile, p_search_for->sourceFile)==0)
          {
            ++leak_map[p_search_for->sourceFile];
            ++key_counter;
          }
        }
      }
      else
      {
        break;
      }
    }

    g_mpc_dump_leak_start = i;
    MyMap::iterator iter = leak_map.begin();
    for(;iter != leak_map.end(); ++iter)
    {
      _RPT2( _CRT_WARN, "%s - %d leaks\r\n", iter->first, iter->second);
    }
  }
  #endif

  #ifdef MPC_GUARD_BYTES
  //inline void guard_test(int block_no)
  void guard_test(int block_no)
  {
    unsigned char pattern[MPC_GUARD_BYTES];
    unsigned int i, end;

    // Set the local buffer inside the loop to make sure the pattern is
    // correct even if the stack is corrupted.
    memset(pattern, MPC_GUARD_PATTERN, MPC_GUARD_BYTES);
    if (block_no == MPC_TRACE_MAX_BLOCKS)
    {
      // Check all blocks
      i = 0;
      end = no_of_heap_blocks_allocated;
    }
    else
    {
      // Check specific block
      i = block_no;
      end = block_no+1;
    }
    for(; i < end; ++i)
    {
      if(g_mpc_heap_allocations[i].pAllocationStart != NULL)
      {
        if( memcmp( g_mpc_heap_allocations[i].pGuardBytesStart, pattern,MPC_GUARD_BYTES) != 0)
        {
          #ifdef __PC__
          MessageBox(NULL, TEXT("Heap MEMORY CORRUPTION\nThrowing a string ^_^"), TEXT("PCMPC.DLL"), MB_ICONERROR|MB_OK);
          #endif
          while(1)
          {
            FatalErrorOccured("NEW - Heap memory corruption!!");
            OS_Delay(2);
          }
        }
      }
    }
  }
  #endif // #ifdef MPC_GUARD_BYTES

  inline int find_block(void* pAllocStart)
  {
    int i = no_of_heap_blocks_allocated; // Start searching down from the blocks allocated
    if ( i >= MPC_TRACE_MAX_BLOCKS)
    {
      i = MPC_TRACE_MAX_BLOCKS-1;
    }
    for(; i > 0; i--)
    {
      if(g_mpc_heap_allocations[i].pAllocationStart == pAllocStart)
      {
        return i;
      }
    }
    return 0;
  }


  void insert_block(size_t size, void* pAllocStart, void* pGuardBytesStart, const char* file, unsigned int line)
  {
    bool inserted = false;

    // Search for a free spot in the heap allocation trace array.
    int i = no_of_heap_blocks_allocated; // (Best gues for an index);

    // If best gues wasn't valid (should always be), start search from index 0.
    if(i > 0 && g_mpc_heap_allocations[no_of_heap_blocks_allocated-1].pAllocationStart == NULL)
    {
      i = 0;
    }

    // Lets find a block.
    for(; i < MPC_TRACE_MAX_BLOCKS; ++i)
    {
      if(g_mpc_heap_allocations[i].pAllocationStart == NULL)
      {
        // Store memory block allocation information
        g_mpc_heap_allocations[i].pAllocationStart = pAllocStart;
        #ifdef MPC_GUARD_BYTES
        g_mpc_heap_allocations[i].pGuardBytesStart = pGuardBytesStart;
        #endif

        #ifdef __PC__
        // Store source file and line information
        sprintf(g_mpc_heap_allocations[i].sourceFile, "%s(%d)", file, line);
        g_mpc_heap_allocations[i].sourceFile[MAX_PATH] = 0;
        g_mpc_heap_allocations[i].line = line;
        #endif
        g_mpc_heap_allocations[i].size = size;
        // Count up global counters
        g_mpc_total_bytes_allocated += size;
        inserted = true;
        break;
      }
      else
      {
        inserted = false; // Just a break point
      }
    }

    if(!inserted)
    {
      // Unable to insert the allocated block in the indexing list
      while(1)
      {
        FatalErrorOccured("Memory trace indexing error.");
        OS_Delay(2);
      }
    }
  }

  inline void remove_block(void* pAllocStart)
  {
    int index = find_block(pAllocStart);

    if (index > 0)
    {
      #ifdef MPC_GUARD_BYTES
        #ifdef MPC_GUARD_TEST_ON_DELETE
          guard_test(index);
        #endif
      #endif

      MpcGuardArreas* p_arrea = &g_mpc_heap_allocations[index];
      p_arrea->pAllocationStart = NULL;
      g_mpc_total_bytes_allocated -= p_arrea->size;

      // Compress MpcGuardArreas
      int i = index+1;
      for(; i < MPC_TRACE_MAX_BLOCKS; ++i)
      {
        if(g_mpc_heap_allocations[i].pAllocationStart != NULL)
        {
          g_mpc_heap_allocations[i-1] = g_mpc_heap_allocations[i];
          g_mpc_heap_allocations[i].pAllocationStart = NULL;
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      // Ooops, we didn't allocate that block. (Try a rebuild all).
      while(1)
      {
        FatalErrorOccured("NEW - attempt to remove unallocated block!!");
        OS_Delay(2);
      }
    }
  }
#endif // MPC_TRACE_ALLOCATIONS

#ifdef __MEMORY_LEAKAGE_TEST___
extern "C" int check_heap(void)
{
  int heap_used = 0;
  #ifdef MPC_GUARD_BYTES
    guard_test(MPC_TRACE_MAX_BLOCKS);
    heap_used = g_mpc_total_bytes_allocated;
  #endif
  return heap_used;
}
#endif


/*****************************************************************************
 * Function - mpc_malloc
 * DESCRIPTION:
*****************************************************************************/
void* mpc_malloc(size_t size, const char* file, unsigned int line)
{
  size_t  new_size = size;

  #ifdef MPC_TRACE_ALLOCATIONS
  if(g_mpc_heap_allocations == NULL)
  {
  #ifndef __PC__
    g_mpc_heap_allocations = (MpcGuardArreas*)OS_malloc( sizeof(MpcGuardArreas) * MPC_TRACE_MAX_BLOCKS);
  #else
    g_mpc_heap_allocations = (MpcGuardArreas*)malloc( sizeof(MpcGuardArreas) * MPC_TRACE_MAX_BLOCKS);
  #endif
    memset(g_mpc_heap_allocations,0, sizeof(MpcGuardArreas) * MPC_TRACE_MAX_BLOCKS);
  }

  #ifdef MPC_GUARD_BYTES
    new_size += MPC_GUARD_BYTES;
  #endif
  #endif // MPC_TRACE_ALLOCATIONS

  #ifdef MPC_BLOCK_ALLOC_SIZE
  new_size += MPC_BLOCK_ALLOC_SIZE - (new_size % MPC_BLOCK_ALLOC_SIZE);
  #endif

  #ifndef __PC__
  void* res = OS_malloc(new_size);
  #else
  void* res = malloc(new_size);
  #endif

  if(res != NULL)
  {
    #ifdef MPC_TRACE_ALLOCATIONS
      #ifdef MPC_GUARD_BYTES
        unsigned char* p_guard = ((unsigned char*)res) + size;
        memset(p_guard, MPC_GUARD_PATTERN, MPC_GUARD_BYTES);
      #else
        void* p_guard = NULL;
      #endif
      insert_block(new_size, res, p_guard, file, line);
    #endif // MPC_TRACE_ALLOCATIONS
  }
  else
  {
    #ifdef __PC__
      ::MessageBox(NULL,TEXT("Out of Mem"),TEXT("Memory allocation error."), MB_ICONERROR|MB_OK);
    #endif
    while(1)
    {
      FatalErrorOccured("NEW - out of memory!!");
      OS_Delay(2);
    }
  }

  #ifdef MPC_TRACE_ALLOCATIONS
    #ifdef MPC_GUARD_TEST_ON_NEW
      guard_test(MPC_TRACE_MAX_BLOCKS);
    #endif
  #endif

  #ifdef __MEMORY_LEAKAGE_TEST___
  ++no_of_heap_blocks_allocated;
  #endif

  return res;
}


/*****************************************************************************
 * Function - mpc_free
 * DESCRIPTION:
*****************************************************************************/
void mpc_free(void* pMem)
{
  if (pMem == NULL)
  {
    return;
  }

  #ifdef MPC_TRACE_ALLOCATIONS
    remove_block(pMem);
  #endif // MPC_TRACE_ALLOCATIONS

/* --------------------------------------------------
* Memory leakage test -
* Define __MEMORY_LEAKAGE_TEST___ in the project
* to enable this test
* --------------------------------------------------*/
#ifdef __MEMORY_LEAKAGE_TEST___
  --no_of_heap_blocks_allocated;
#endif

#ifndef __PC__
  OS_free(pMem);
#else
  free(pMem);
#endif
}

void* operator new(size_t size, const char* sourceFile, unsigned int line)
{
  return mpc_malloc(size, sourceFile, line);
}

void operator delete(void* pMem, const char* sourceFile, unsigned int line)
{
  operator delete(pMem);
}

void* operator new[](size_t size)
{
  return mpc_malloc(size, "", 0);
}

void* operator new(size_t size)
{
  return mpc_malloc(size, "", 0);
}

void operator delete[](void* pMem)
{
  operator delete(pMem);
}

void operator delete(void* pMem)
{
  mpc_free(pMem);
}
