//#define __PERFORMANCE_TEST___
//#define __MEMORY_LEAKAGE_TEST___
//#define __INTERRUPT_LOAD_TEST___


#ifdef __PERFORMANCE_TEST___
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
/* CLASS NAME       : TestPerformance                                       */
/*                                                                          */
/* FILE NAME        : TestPerformance.cpp                                   */
/*                                                                          */
/* CREATED DATE     : 17-08-2008 dd-mm-yyyy                                 */
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
#include <typedef.h>
#include <cu351_cpu_types.h>
extern "C"
{
  #include <RTOS.H>
}

/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <TestPerformance.h>
#include <geni_if.h>
#include <TestAlloc.h>

/*****************************************************************************
  DEFINES
 *****************************************************************************/

/*****************************************************************************
  TYPE DEFINES
 *****************************************************************************/

/*****************************************************************************
 *
 *
 *              PUBLICS
 *
 *
 *****************************************************************************/

// Fifo queue test
extern int act_sw_timer_queue_size = 0;
extern int max_sw_timer_queue_size = 0;
extern int act_event_task_queue_size = 0;
extern int max_event_task_queue_size = 0;
extern int act_geni_app_queue_size = 0;
extern int max_geni_app_queue_size = 0;
extern int act_config_control_queue_size = 0;
extern int max_config_control_queue_size = 0;

/******************************************************************************
 * Function - PutOnGeni14ForTest
 * DESCRIPTION: Put some test data on Geni by misuse of class 14.
 *              Note that only unused id's up to HIGH_MEAS32 can be used.
 *              Modifying bu-tab14 even though the tables are constant arrays !
 *
 *****************************************************************************/
extern "C" void PutOnGeni14ForTest(U8 id, int *p_value)
{
  extern const ID_PTR   meas32_tab[];
  extern const ID_INFO  meas32_info_tab[];
  ID_PTR*  p_meas_tab = (ID_PTR*)meas32_tab;
  ID_INFO* p_info_tab = (ID_INFO*)meas32_info_tab;
  if (id <= HIGH_MEAS32_ID)
  {
    p_info_tab[id] = 0x41; // Info = unscaled
    p_meas_tab[id] = (ID_PTR)p_value;
  }
}

/******************************************************************************
 * Function - GetFromGeni5ForTest
 * DESCRIPTION: Get some test data from Geni by misuse of class 5.
 *              Note that only existing id's can be used e.g. 10-15.(clock set)
 *              Modifying bu-tab5 even though the tables are constant arrays !
 *
 *****************************************************************************/
extern "C" U8 GetFromGeni5ForTest(U8 id)
{
  extern const ID_PTR   ref_tab[];
  extern const ID_INFO  ref_info_tab[];
  ID_PTR* p_ref_tab   = (ID_PTR*)ref_tab;
  ID_INFO* p_info_tab = (ID_INFO*)ref_info_tab;
  if (id >= 10 && id <= 15)
  {
    p_info_tab[id] = 0x41; // Info = unscaled
    return *p_ref_tab[id];
  }
  return 0xFF;
}

/******************************************************************************
 * Function - PerformanceTestTask
 * DESCRIPTION:
 *
 *****************************************************************************/
extern "C" void PerformanceTestTask(void)
{
  int task_wait_time;

  static TestAlloc MemTest;
  static int act_mem_available = 0;
  static int min_mem_available = 99999999;
  static int heap_blocks_allocated = 0;
  static int max_heap_blocks_allocated = 0;

  PutOnGeni14ForTest(100, &act_mem_available);
  PutOnGeni14ForTest(101, &min_mem_available);
  PutOnGeni14ForTest(102, &heap_blocks_allocated);
  PutOnGeni14ForTest(103, &max_heap_blocks_allocated);

  // Fifo queue test
  PutOnGeni14ForTest(110, &act_sw_timer_queue_size);
  PutOnGeni14ForTest(111, &max_sw_timer_queue_size);
  PutOnGeni14ForTest(112, &act_event_task_queue_size);
  PutOnGeni14ForTest(113, &max_event_task_queue_size);
  PutOnGeni14ForTest(114, &act_geni_app_queue_size);
  PutOnGeni14ForTest(115, &max_geni_app_queue_size);
  PutOnGeni14ForTest(116, &act_config_control_queue_size);
  PutOnGeni14ForTest(117, &max_config_control_queue_size);

  while (1)
  {
    #ifdef __INTERRUPT_LOAD_TEST___
    extern void CheckInteruptLoad(void);
    CheckInteruptLoad();
    #endif // __INTERRUPT_LOAD_TEST___

    #ifdef __MEMORY_LEAKAGE_TEST___
    extern int no_of_heap_blocks_allocated; // Updated new.cpp
    static int heap_check_counter, heap_check_value;
    heap_blocks_allocated = no_of_heap_blocks_allocated;
    heap_check_value = GetFromGeni5ForTest(10);
    if (heap_check_value != 0)
    {
      heap_check_counter++;
      if ((heap_check_counter % heap_check_value) == 0)
      {
        extern int check_heap(void);
        check_heap(); // Works only when enabled in new.cpp

        act_mem_available = MemTest.CheckMemoryAvailable(0xFFFFFFFF); // Allocate/deallocate all heap
        if (min_mem_available > act_mem_available)
        {
          min_mem_available = act_mem_available;
        }
      }
    }
    else
    {
      min_mem_available = 99999999;
      max_heap_blocks_allocated = 0;
    }
    if (max_heap_blocks_allocated < heap_blocks_allocated)
    {
      max_heap_blocks_allocated = heap_blocks_allocated;
    }
    #endif // __MEMORY_LEAKAGE_TEST___

    // Fifo queue test clear
    if (GetFromGeni5ForTest(11) > 1)
    {
      max_sw_timer_queue_size = 0;
      max_event_task_queue_size = 0;
      max_geni_app_queue_size = 0;
      max_config_control_queue_size = 0;
    }

    task_wait_time = GetFromGeni5ForTest(12);
    if (task_wait_time < 5)
    {
      task_wait_time = 100;
    }
    OS_Delay(task_wait_time);
  }
}


/******************************************************************************
 * Function - InitPerformanceTest
 * DESCRIPTION: Setup a task used for the performance test
 *
 *****************************************************************************/
void InitPerformanceTest(void)
{
  static OS_TASK TCBPerformanceTestTask;
  static OS_STACKPTR int PerformanceTestTaskStack[250];

#ifdef __PC__
  OS_CREATETASK(&TCBPerformanceTestTask, "Performance Test", PerformanceTestTask, 100, PerformanceTestTaskStack);
#else
  OS_CREATETASK(&TCBPerformanceTestTask, "Performance Test", PerformanceTestTask, 195, PerformanceTestTaskStack);
#endif
}

/******************************************************************************
 *
 *
 *              PRIVATE FUNCTIONS
 *
 *
 *****************************************************************************/

#ifdef __INTERRUPT_LOAD_TEST___
extern int acc_us_in_interrupt = 0;   // Updated in RtosInit.cpp
extern int peak_us_in_interrupt = 0;  // Updated in RtosInit.cpp
/******************************************************************************
 * Function - CheckInteruptLoad
 * DESCRIPTION:
 *
 *****************************************************************************/
extern void CheckInteruptLoad(void)
{
  static bool initialized = false;
  static int  max_us_in_interrupt = 0;
  static int  max_max_us_in_interrupt = 0;
  static int  interrupt_load = 0;
  static int  new_ms = 0, old_ms = 0;

  if (initialized == false)
  {
    old_ms = OS_GetTime();
    PutOnGeni14ForTest(120, &interrupt_load);           // Interrupt load (propille) within last 2 seconds
    PutOnGeni14ForTest(121, &max_us_in_interrupt);      // Interrupt peak (us) within last 2 seconds
    PutOnGeni14ForTest(122, &max_max_us_in_interrupt);  // Interrupt peak (us) since power up
    PutOnGeni14ForTest(123, &new_ms);                   // Accumulated time (ms) since power up
    initialized = true;
  }
  else
  {
    new_ms = OS_GetTime();
    if (new_ms-old_ms >= 5000) // Update every 5 seconds
    {
      interrupt_load = acc_us_in_interrupt/(new_ms-old_ms);
      acc_us_in_interrupt = 0;
      max_us_in_interrupt = peak_us_in_interrupt;
      peak_us_in_interrupt = 0;
      if (max_max_us_in_interrupt < max_us_in_interrupt)
      {
        max_max_us_in_interrupt = max_us_in_interrupt;
      }
      old_ms = new_ms;
    }
  }
}
#endif // __INTERRUPT_LOAD_TEST___


#endif // __PERFORMANCE_TEST___
