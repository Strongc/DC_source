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
/* CLASS NAME       : HeapSize                                              */
/*                                                                          */
/* FILE NAME        : HeapSize.cpp                                          */
/*                                                                          */
/* CREATED DATE     : 2004-09-01                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* This Class is responsible for how to show a HeapSize.                    */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef TO_RUN_ON_PC

#else

#endif
/*****************************************************************************
SYSTEM INCLUDES
*****************************************************************************/
#ifdef __PC__
#include <malloc.h>
#endif
/*****************************************************************************
PROJECT INCLUDES
*****************************************************************************/
/*****************************************************************************
LOCAL INCLUDES
****************************************************************************/

#include "HeapSize.h"

/*****************************************************************************
DEFINES
*****************************************************************************/

    /* --------------------------------------------------
    * Memory leakage test -
    * Define __MEMORY_LEAKAGE_TEST___ in the project
    * to enable this test
    * --------------------------------------------------*/
    #ifdef __MEMORY_LEAKAGE_TEST___
      extern int no_of_heap_blocks_allocated;
    #endif //__MEMORY_LEAKAGE_TEST___

/*****************************************************************************
TYPE DEFINES
*****************************************************************************/

namespace mpc
{
  namespace display
  {
    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Constructor
    *****************************************************************************/
    HeapSize::HeapSize(Component* pParent) : Text(pParent)
    {
      runs = 10000; // run now.
    }

    /*****************************************************************************
    * Function
    * DESCRIPTION:
    * Dectructor
    *****************************************************************************/
    HeapSize::~HeapSize()
    {
    }

    void HeapSize::Run()
    {
      if(runs >=5)
      {
        runs = 0;
        UpdateHeapInfo();
      }
      ++runs;
      Text::Run();
    }

    void HeapSize::UpdateHeapInfo()
    {
       mUsedHeap = 0;
       mFreeHeap = 0;

/*** heapwalk only exists for PC ***/
/* #ifdef __PC__
       _HEAPINFO hinfo;
       int heapstatus;
       hinfo._pentry = NULL;
       while( ( heapstatus = _heapwalk( &hinfo ) ) == _HEAPOK )
       {

            if(hinfo._useflag == _USEDENTRY)
            {
              mUsedHeap += hinfo._size;
            }
            else
            {
              mFreeHeap += hinfo._size;
            }
       }

       switch( heapstatus )
       {
       case _HEAPEMPTY:
       case _HEAPEND:
       {
         char sz_tmp[100];
         sprintf(sz_tmp, "U=%d, F=%d, T=%d", mUsedHeap, mFreeHeap, mUsedHeap + mFreeHeap);
         SetText(sz_tmp);
       }
       break;
       case _HEAPBADPTR:
          SetText( "ERROR - bad pointer to heap\n" );
          break;
       case _HEAPBADBEGIN:
          SetText( "ERROR - bad start of heap\n" );
          break;
       case _HEAPBADNODE:
          SetText( "ERROR - bad node in heap\n" );
          break;
       }
#else
*/
      /* --------------------------------------------------
      * Memory leakage test -
      * Define __MEMORY_LEAKAGE_TEST___ in the project
      * to enable this test
      * --------------------------------------------------*/
      #ifdef __MEMORY_LEAKAGE_TEST___
       char sz_tmp[50];
       sprintf(sz_tmp,"%i",no_of_heap_blocks_allocated);
       SetText(sz_tmp);
      #endif // __MEMORY_LEAKAGE_TEST___

// #endif // __PC__
    }

  } // namespace display
} // namespace mpc
