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
/* CLASS NAME       : IO351                                                 */
/*                                                                          */
/* FILE NAME        : IO351.h                                               */
/*                                                                          */
/* CREATED DATE     : 04-03-2004 dd-mm-yyyy                                 */
/*                                                                          */
/* SHORT FILE DESCRIPTION : IO 351 base class                               */
/****************************************************************************/
/*****************************************************************************
   Protect against multiple inclusion through the use of guards:
 ****************************************************************************/
#ifndef __IO351_H__
#define __IO351_H__

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
  IO351_PM_NO_1,    // IO351 Pump Module #1
  IO351_PM_NO_2,    // IO351 Pump Module #2
  IO351_PM_NO_3,    // IO351 Pump Module #3
  IO351_PM_NO_4,    // IO351 Pump Module #4
  IO351_PM_NO_5,    // IO351 Pump Module #5
  IO351_PM_NO_6,    // IO351 Pump Module #6
  LAST_IO351_PM,
  NO_OF_IO351_PM_NO = LAST_IO351_PM,


  IO351_IOM_NO_1 = LAST_IO351_PM + 1,   // IO351 IO Module #1
  IO351_IOM_NO_2,   // IO351 IO Module #2
  IO351_IOM_NO_3,   // IO351 IO Module #3
  LAST_IO351_IOM,
  NO_OF_IO351_IOM_NO = LAST_IO351_IOM - IO351_IOM_NO_1,

  IO111_NO_1 = LAST_IO351_IOM + 1,
  IO111_NO_2,
  IO111_NO_3,
  IO111_NO_4,
  IO111_NO_5,
  IO111_NO_6,
  LAST_IO111,
  NO_OF_IO111_NO = LAST_IO111 - IO111_NO_1,

  // Inserted to prepare for IO113 with attached mixer.
  // If we can assume that only one wil exist and it will be coded to num 1 then we only need one.
  IO113_WITH_MIXER_NO_1 = LAST_IO111 + 1,
  LAST_IO113_WITH_MIXER,
  NO_OF_IO113_WITH_MIXER_NO = LAST_IO113_WITH_MIXER - IO113_WITH_MIXER_NO_1,

  // CUE_NO_1 = LAST_IO111 + 1,

  CUE_NO_1 = LAST_IO113_WITH_MIXER + 1,
  CUE_NO_2,
  CUE_NO_3,
  CUE_NO_4,
  CUE_NO_5,
  CUE_NO_6,
  LAST_CUE,
  NO_OF_CUE_NO = LAST_CUE - CUE_NO_1,

  MP204_NO_1 = LAST_CUE + 1,
  MP204_NO_2,
  MP204_NO_3,
  MP204_NO_4,
  MP204_NO_5,
  MP204_NO_6,
  LAST_MP204,
  NO_OF_MP204_NO = LAST_MP204 - MP204_NO_1,

  NO_OF_MODULE_TYPE
} IO351_NO_TYPE;

typedef enum
{
  IO351_DIG_IN_NO_1,
  IO351_DIG_IN_NO_2,
  IO351_DIG_IN_NO_3,
  IO351_DIG_IN_NO_4,
  IO351_DIG_IN_NO_5,
  IO351_DIG_IN_NO_6,
  IO351_DIG_IN_NO_7,
  IO351_DIG_IN_NO_8,
  IO351_DIG_IN_NO_9
} IO351_DIG_IN_NO_TYPE;

typedef enum
{
  IO351_DIG_OUT_NO_1,
  IO351_DIG_OUT_NO_2,
  IO351_DIG_OUT_NO_3,
  IO351_DIG_OUT_NO_4,
  IO351_DIG_OUT_NO_5,
  IO351_DIG_OUT_NO_6,
  IO351_DIG_OUT_NO_7
} IO351_DIG_OUT_NO_TYPE;

typedef enum
{
  IO351_ANA_IN_NO_1,
  IO351_ANA_IN_NO_2
} IO351_ANA_IN_NO_TYPE;

typedef enum
{
  IO351_ANA_OUT_NO_1,
  IO351_ANA_OUT_NO_2,
  IO351_ANA_OUT_NO_3,

  NO_OF_ANA_OUT_CHANNELS
} IO351_ANA_OUT_NO_TYPE;

/*****************************************************************************
 * CLASS: IO351
 * DESCRIPTION: IO 351 base class
 *****************************************************************************/
class IO351
{
  protected:
    /********************************************************************
    LIFECYCLE - Constructor
    ********************************************************************/
    IO351(const IO351_NO_TYPE moduleNo) : mModuleNo(moduleNo)
    {
    }

public:
    /********************************************************************
    IO351 - Destructor
    ********************************************************************/
    ~IO351()
    {
    }

    /********************************************************************
    ASSIGNMENT OPERATOR
    ********************************************************************/

    /********************************************************************
    OPERATIONS
    ********************************************************************/
    friend class GeniSlaveIf;     // GeniSlaveIf callback
    virtual void ConfigReceived(bool rxedOk, U8 noOfPumps, U8 noOfVlt, U8 pumpOffset, U8 moduleType) = 0;

  private:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

  protected:
    /********************************************************************
    OPERATIONS
    ********************************************************************/

   /********************************************************************
    ATTRIBUTE
    ********************************************************************/
    const IO351_NO_TYPE mModuleNo;  // IO 351 module number
};
#endif
