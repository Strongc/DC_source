/***************** Header File for STFL-I Flash Memory Driver ******************

   Filename:    c1648.h
   Description: Header file for c1648.c
                Consult also the C file for more details.

   Version:     $Id: c1648.h,v 1.9 2003/05/19 15:12:17 russog Exp $
   Author:      Giuseppe Russo, STMicroelectronics, Agrate Brianza (Italy)

   Copyright (c) 2002 STMicroelectronics.

   THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,EITHER
   EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
   AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
   PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
   REPAIR OR CORRECTION.
********************************************************************************

   Version History.

   Ver.   Date        Comments

   1.00   02/09/2002  Alpha release of the software.
   1.01   02/09/2002  New alpha release using standard STFL-I.
   1.1    27/09/2002  Program Style Update
   1.5    11/10/2002  Initial release
   1.6    20/11/2002  Second release
   1.9    19/05/2003  Updated Corporate Publication Style
*******************************************************************************/


  /*************** User Change Area *******************************************

   This section is meant to give all the opportunities to customize the
   SW Drivers according to the requirements of hardware and flash configuration.
   It is possible to choose flash start address, CPU Bitdepth, number of flash
   chips, hardware configuration and performance data (Timeout Info).

   The options are listed and explained below:

   ********* Data Types *********
   The source code defines hardware independent datatypes assuming the
   compiler implements the numerical types as

   unsigned char    8 bits (defined as ubyte)
   char             8 bits (defined as byte)
   unsigned short  16 bits (defined as uword)
   short           16 bits (defined as word)
   unsigned int    32 bits (defined as udword)
   int             32 bits (defined as dword)

   In case the compiler does not support the currently used numerical types,
   they can be easily changed just once here in the user area of the headerfile.
   The data types are consequently referenced in the source code as (u)byte,
   (u)word and (u)dword. No other data types like 'CHAR','SHORT','INT','LONG'
   directly used in the code.



   ********* Flash Type *********
   This driver supports the following Flash Types

   M29W641D    16bit, 64Mbit Flash  #define USE_M29W641D


   ********* Base Address *********
   The start address where the flash memory chips are "visible" within
   the memory of the CPU is called the BASE_ADDR. This address must be
   set according to the current system. This value is used by FlashRead()
   FlashWrite(). Some applications which require a more complicated
   FlashRead() or FlashWrite() may not use BASE_ADDR.


   ********* Flash and Board Configuration *********
   The driver supports also different configurations of the flash chips
   on the board. In each configuration a new data Type called
   'uCPUBusType' is defined to match the current CPU data bus width.
   This data type is then used for all accesses to the memory.

   The different options (defines) are explained below:

   - USE_16BIT_CPU_ACCESSING_1_16BIT_FLASH
   Using this define enables a configuration consisting of an environment
   containing a CPU with 16 bit Databus and 1 16bit flash chip connected
   to it. Standard Configuration

   - USE_32BIT_CPU_ACCESSING_2_16BIT_FLASH
   Using this define enables a configuration consisting of an environment
   containing a CPU with 32bit databus and two flash chips connected to it.
   With every write access on the databus the first 16bit reaches,therefore,
   flash 1 and the second 16 bit reaches thus flash 2.
   The address pins are connected in parallel.



   ********* TimeOut *********
   There are timeouts implemented in the loops of the code, in order to enable an exit for
   operations that would otherwise never terminate.At each point where an such a loop is
   implemented a comment /# TimeOut! #/ has been placed. There are two possibilities:

   1) The Option ANSI Library functions declared in 'time.h' exists
      If the current compiler supports 'time.h' the define statement TIME_H_EXISTS
      should be activated. This makes sure that the performance of the current evaluation
      HW does not change the timeout settings.

      #define TIME_H_EXISTS

   2) The Option COUNT_FOR_A_SECOND
      If the current compiler does not support 'time.h', the define statement TIME_H_EXISTS
      can not be used. To overcome this constraint the value COUNT_FOR_A_SECOND has to be defined
      in order to create a one second delay. For example, if 100000 repetitions of a loop are
      needed, to give a time delay of one second, then COUNT_FOR_A_SECOND should have the
      value 100000.

      #define COUNT_FOR_A_SECOND (chosen value).

      Note: This delay is HW (Performance) dependent and needs, therefore, updated with every new HW.
      This driver has been tested with a certain configuration and other target platforms may have
      other performance data, therefore, the value may have to be changed.

      It is up to the user to implement this value to avoid the code timing out too early instead
      of completing correctly.

   ********* Pause Constant *********
   The function Flashpause() is used in several areas of the code to generate a delay required
   for correct operation of the flash device. There are two options provided:


   1) The Option ANSI Library functions declared in 'time.h' exists
      If the current compiler supports 'time.h' the define statement TIME_H_EXISTS should be
      activated. This makes sure that the performance of the current evaluation HW does not
      change the timeout settings.

      #define TIME_H_EXISTS

    2)    The Option COUNT_FOR_A_MICROSECOND
      If the current compiler does not support 'time.h', the define statement TIME_H_EXISTS can
      not be used. To overcome this constraint the value COUNT_FOR_A_MICROSECOND has to be defined
      in order to create a one micro second delay.
      Depending on a 'While(count-- != 0);' loop a value has to be found which creates the
      necessary delay.
      - An approximate approach can be given by using the clock frequency of the test plattform.
      That means if an evaluation board with 200 Mhz is used, the value for COUNT_FOR_A_MICROSECOND
      would be: 200.
      - The real exact value can only be found using a logic state analyser.

      #define COUNT_FOR_A_MICROSECOND (chosen value).

      Note: This delay is HW (Performance) dependent and needs, therefore, updated with every new HW.
      This driver has been tested with a certain configuration and other target platforms may have other
      performance data, therefore, the value may have to be changed.

   No further changes should be necessary.

*****************************************************************************/

#ifndef __C1648__H__
#define __C1648__H__

#include "cu351_cpu_types.h"

typedef unsigned char   ubyte; /* All HW dependent Basic Data Types */
//typedef          char    byte; DAU!!
typedef unsigned short  uword;
typedef          short   word;
typedef unsigned int   udword;
typedef          int    dword;

 #define USE_M29W641D
/* Possible Values: USE_M29W641D */

//TCSL - Define for the new size changes to the RAM and ROM
#ifdef _RAM_32_ROM_16_UPGRADE_
#define BASE_ADDR ((volatile uCPUBusType*)0xBF000000)
#else
#define BASE_ADDR ((volatile uCPUBusType*)0xBF800000)
#endif
/* BASE_ADDR is the base or start address of the flash, see the functions
   FlashRead and FlashWrite(). Some applications which require a more
   complicated FlashRead() or FlashWrite() may not use BASE_ADDR */

#define USE_16BIT_CPU_ACCESSING_1_16BIT_FLASH /* Current PCB Info */
/* Possible Values: USE_16BIT_CPU_ACCESSING_1_16BIT_FLASH
                    USE_32BIT_CPU_ACCESSING_2_16BIT_FLASH */

// DAU!! #define VERBOSE  /* Activates additional Routines */
                 /* Currently the Error String Definition */


//PTA #define TIME_H_EXISTS  /* set this macro if C-library "time.h" is supported */

#ifndef TIME_H_EXISTS
  #define COUNT_FOR_A_SECOND 3448000  /* Timer Usage */  // Execution time for FlashTimeOut measured and the constant 3.448.000 found
  // Not used DAU!! #define COUNT_FOR_A_MICROSECOND 20   /* Used in FlashPause function */
#endif

/********************** End of User Change Area *****************************/


/*****************************************************************************
HW Structure Info, Usage of the Flash Memory (Circuitry)
*****************************************************************************/

#ifdef USE_16BIT_CPU_ACCESSING_1_16BIT_FLASH
   typedef uword uCPUBusType;
   typedef  word  CPUBusType;
   #define FLASH_BIT_DEPTH 16
   #define HEX "04Xh"
   #define CMD(A) (A)
   #define CONFIGURATION_DEFINED
#endif

#ifdef USE_32BIT_CPU_ACCESSING_2_16BIT_FLASH
   typedef udword uCPUBusType;
   typedef  dword  CPUBusType;
   #define FLASH_BIT_DEPTH 16
   #define HEX "08Xh"
   #define CMD(A) (A+(A<<16))
   #define CONFIGURATION_DEFINED
#endif


/*******************************************************************************
     Specific Return Codes
*******************************************************************************/
typedef enum {
   FlashSpec_TooManyBlocks,
   FlashSpec_MpuTooSlow,
   FlashSpec_ToggleFailed
} SpecificReturnType;


/*******************************************************************************
     CONFIGURATION CHECK
*******************************************************************************/

#ifndef CONFIGURATION_DEFINED
#error  User Change Area Error: PCB Info uncorrect Check the USE_xxBIT_CPU_ACCESSING_n_yyBIT_FLASH Value
#endif

/*******************************************************************************
     DERIVED DATATYPES
*******************************************************************************/

/******** CommandsType ********/

typedef enum {
   BankErase,
   BankReset,
   BankResume,
   BankSuspend,
   BlockErase,
   BlockProtect,
   BlockUnprotect,
   CheckBlockProtection,
   CheckCompatibility,
   ChipErase,
   ChipUnprotect,
   GroupProtect,
   Program,
   Read,
   ReadCfi,
   ReadDeviceId,
   ReadManufacturerCode,
   Reset,
   Resume,
   SingleProgram,
   Suspend,
   Write
} CommandType;


/******** ReturnType ********/

typedef enum {
   Flash_AddressInvalid,
   Flash_BankEraseFailed,
   Flash_BlockEraseFailed,
   Flash_BlockNrInvalid,
   Flash_BlockProtected,
   Flash_BlockProtectFailed,
   Flash_BlockProtectionUnclear,
   Flash_BlockUnprotected,
   Flash_BlockUnprotectFailed,
   Flash_CfiFailed,
   Flash_ChipEraseFailed,
   Flash_ChipUnprotectFailed,
   Flash_FunctionNotSupported,
   Flash_GroupProtectFailed,
   Flash_NoInformationAvailable,
   Flash_NoOperationToSuspend,
   Flash_OperationOngoing,
   Flash_OperationTimeOut,
   Flash_ProgramFailed,
   Flash_ResponseUnclear,
   Flash_SpecificError,
   Flash_Success,
   Flash_VppInvalid,
   Flash_WrongType
} ReturnType;

/******** BlockType ********/

typedef uword uBlockType;

/******** ParameterType ********/

typedef union {
    /**** BankErase Parameters ****/
    struct {
      uBlockType ublBlockNr;
      ReturnType *rpResults;
    } BankErase;

    /**** BankReset Parameters ****/
    struct {
      udword udBankAddrOff;
    } BankReset;

    /**** BankResume Parameters ****/
    struct {
      udword udAddrOff;
    } BankResume;

    /**** BankSuspend Parameters ****/
    struct {
      udword udAddrOff;
    } BankSuspend;

    /**** BlockErase Parameters ****/
    struct {
      uBlockType ublBlockNr;
    } BlockErase;

    /**** BlockProtect Parameters ****/
    struct {
      uBlockType ublBlockNr;
    } BlockProtect;

    /**** BlockUnprotect Parameters ****/
    struct {
      uBlockType ublBlockNr;
    } BlockUnprotect;

    /**** CheckBlockProtection Parameters ****/
    struct {
      uBlockType ublBlockNr;
    } CheckBlockProtection;

    /**** CheckCompatibility has no parameters ****/

    /**** ChipErase Parameters ****/
    struct {
      ReturnType *rpResults;
    } ChipErase;

    /**** ChipUnprotect Parameters ****/
    struct {
      ReturnType *rpResults;
    } ChipUnprotect;

    /**** GroupProtect Parameters ****/
    struct {
      uBlockType ublBlockNr;
    } GroupProtect;

    /**** Program Parameters ****/
    struct {
      udword udAddrOff;
      udword udNrOfElementsInArray;
        void *pArray;
      udword udMode;
    } Program;

    /**** Read Parameters ****/
    struct {
      udword  udAddrOff;
      uCPUBusType ucValue;
    } Read;

    /**** ReadCfi Parameters ****/
    struct {
      uword  uwCfiFunc;
      uCPUBusType ucCfiValue;
    } ReadCfi;

    /**** ReadDeviceId Parameters ****/
    struct {
      uCPUBusType ucDeviceId;
    } ReadDeviceId;

    /**** ReadManufacturerCode Parameters ****/
    struct {
      uCPUBusType ucManufacturerCode;
    } ReadManufacturerCode;

    /**** Reset has no parameters ****/

    /**** Resume has no parameters ****/

    /**** SingleProgram Parameters ****/
    struct {
      udword udAddrOff;
      uCPUBusType ucValue;
    } SingleProgram;

    /**** Suspend has no parameters ****/

    /**** Write Parameters ****/
    struct {
      udword udAddrOff;
      uCPUBusType ucValue;
    } Write;

} ParameterType;

/******** ErrorInfoType ********/

typedef struct {
  SpecificReturnType sprRetVal;
  udword             udGeneralInfo[4];
} ErrorInfoType;

/******************************************************************************
    Global variables
*******************************************************************************/
extern ErrorInfoType eiErrorInfo;



/******************************************************************************
    Standard functions
*******************************************************************************/
  EXTERN ReturnType  Flash( CommandType cmdCommand, ParameterType *fp );
  EXTERN ReturnType  FlashBankErase( uBlockType ublBlockNr, ReturnType  *rpResults );
  EXTERN ReturnType  FlashBankReset( udword udBankAddrOff );
  EXTERN ReturnType  FlashBankResume( udword udBankAddrOff );
  EXTERN ReturnType  FlashBankSuspend( udword udBankAddrOff );
  EXTERN ReturnType  FlashBlockErase( uBlockType ublBlockNr );
  EXTERN ReturnType  FlashBlockProtect( uBlockType ublBlockNr );
  EXTERN ReturnType  FlashBlockUnprotect( uBlockType ublBlockNr );
  EXTERN ReturnType  FlashCheckBlockProtection( uBlockType ublBlockNr );
  EXTERN ReturnType  FlashCheckCompatibility( void );
  EXTERN ReturnType  FlashChipErase( ReturnType  *rpResults );
  EXTERN ReturnType  FlashChipUnprotect( ReturnType  *rpResults );
  EXTERN ReturnType  FlashGroupProtect( uBlockType ublBlockNr );
  EXTERN ReturnType  FlashProgram( udword udAddrOff, udword udNrOfElementsInArray, void *pArray );
 EXTERN uCPUBusType  FlashRead( udword udAddrOff );
  EXTERN ReturnType  FlashReadCfi( uword uwCFIFunc, uCPUBusType *ucpCFIValue );
  EXTERN ReturnType  FlashReadDeviceId( uCPUBusType *ucpDeviceID);
  EXTERN ReturnType  FlashReadManufacturerCode( uCPUBusType *ucpManufacturerCode);
  EXTERN ReturnType  FlashReset( void );
  EXTERN ReturnType  FlashResume( void );
  EXTERN ReturnType  FlashSingleProgram( udword udAddrOff, uCPUBusType ucVal );
  EXTERN ReturnType  FlashSuspend( void );
  EXTERN       void  FlashWrite( udword udAddrOff, uCPUBusType ucVal );
  EXTERN       void  FlashInit( void );


/******************************************************************************
    Utility functions
*******************************************************************************/
#ifdef VERBOSE
   byte *FlashErrorStr( ReturnType rErrNum );
#endif

  ReturnType  FlashResponseIntegrityCheck( uCPUBusType *ucpFlashResponse );
  ReturnType  FlashTimeOut( udword udSeconds );

/*******************************************************************************
Device Constants
*******************************************************************************/

#define FLASH_MWA 1  /* Minimum Write Access  */
#define MANUFACTURER_ST (0x0020)  /* ST Manufacturer Code is 0x20 */
#define ANY_ADDR (0x0000001) /* Any addroffset within the Flash Memory will do */


#ifdef USE_M29W641D
	//TCSL - Define for the new size changes to the RAM and ROM
	#ifdef _RAM_32_ROM_16_UPGRADE_
		#define FLASH_SIZE (0x00800000)  /* Total device size in Words (16MB)*/
	#else	
		#define FLASH_SIZE (0x00400000)  /* Total device size in Words: 1M x 16 */
	#endif
    #define EXPECTED_DEVICE (0x22C7) /* Device code for the M29W641DT */
    #define FLASH_WRITE_BUFFER_SIZE 1  /* Write Buffer = 1 Words */

#ifndef _RAM_32_ROM_16_UPGRADE_
   static const udword BlockOffset[] = {
      0x000000,  /* Start offset of block 0   */
      0x008000,  /* Start offset of block 1   */
      0x010000,  /* Start offset of block 2   */
      0x018000,  /* Start offset of block 3   */
      0x020000,  /* Start offset of block 4   */
      0x028000,  /* Start offset of block 5   */
      0x030000,  /* Start offset of block 6   */
      0x038000,  /* Start offset of block 7   */
      0x040000,  /* Start offset of block 8   */
      0x048000,  /* Start offset of block 9   */
      0x050000,  /* Start offset of block 10  */
      0x058000,  /* Start offset of block 11  */
      0x060000,  /* Start offset of block 12  */
      0x068000,  /* Start offset of block 13  */
      0x070000,  /* Start offset of block 14  */
      0x078000,  /* Start offset of block 15  */
      0x080000,  /* Start offset of block 16  */
      0x088000,  /* Start offset of block 17  */
      0x090000,  /* Start offset of block 18  */
      0x098000,  /* Start offset of block 19  */
      0x0A0000,  /* Start offset of block 20  */
      0x0A8000,  /* Start offset of block 21  */
      0x0B0000,  /* Start offset of block 22  */
      0x0B8000,  /* Start offset of block 23  */
      0x0C0000,  /* Start offset of block 24  */
      0x0C8000,  /* Start offset of block 25  */
      0x0D0000,  /* Start offset of block 26  */
      0x0D8000,  /* Start offset of block 27  */
      0x0E0000,  /* Start offset of block 28  */
      0x0E8000,  /* Start offset of block 29  */
      0x0F0000,  /* Start offset of block 30  */
      0x0F8000,  /* Start offset of block 31  */
      0x100000,  /* Start offset of block 32  */
      0x108000,  /* Start offset of block 33  */
      0x110000,  /* Start offset of block 34  */
      0x118000,  /* Start offset of block 35  */
      0x120000,  /* Start offset of block 36  */
      0x128000,  /* Start offset of block 37  */
      0x130000,  /* Start offset of block 38  */
      0x138000,  /* Start offset of block 39  */
      0x140000,  /* Start offset of block 40  */
      0x148000,  /* Start offset of block 41  */
      0x150000,  /* Start offset of block 42  */
      0x158000,  /* Start offset of block 43  */
      0x160000,  /* Start offset of block 44  */
      0x168000,  /* Start offset of block 45  */
      0x170000,  /* Start offset of block 46  */
      0x178000,  /* Start offset of block 47  */
      0x180000,  /* Start offset of block 48  */
      0x188000,  /* Start offset of block 49  */
      0x190000,  /* Start offset of block 50  */
      0x198000,  /* Start offset of block 51  */
      0x1A0000,  /* Start offset of block 52  */
      0x1A8000,  /* Start offset of block 53  */
      0x1B0000,  /* Start offset of block 54  */
      0x1B8000,  /* Start offset of block 55  */
      0x1C0000,  /* Start offset of block 56  */
      0x1C8000,  /* Start offset of block 57  */
      0x1D0000,  /* Start offset of block 58  */
      0x1D8000,  /* Start offset of block 59  */
      0x1E0000,  /* Start offset of block 60  */
      0x1E8000,  /* Start offset of block 61  */
      0x1F0000,  /* Start offset of block 62  */
      0x1F8000,  /* Start offset of block 63  */
      0x200000,  /* Start offset of block 64  */
      0x208000,  /* Start offset of block 65  */
      0x210000,  /* Start offset of block 66  */
      0x218000,  /* Start offset of block 67  */
      0x220000,  /* Start offset of block 68  */
      0x228000,  /* Start offset of block 69  */
      0x230000,  /* Start offset of block 70  */
      0x238000,  /* Start offset of block 71  */
      0x240000,  /* Start offset of block 72  */
      0x248000,  /* Start offset of block 73  */
      0x250000,  /* Start offset of block 74  */
      0x258000,  /* Start offset of block 75  */
      0x260000,  /* Start offset of block 76  */
      0x268000,  /* Start offset of block 77  */
      0x270000,  /* Start offset of block 78  */
      0x278000,  /* Start offset of block 79  */
      0x280000,  /* Start offset of block 80  */
      0x288000,  /* Start offset of block 81  */
      0x290000,  /* Start offset of block 82  */
      0x298000,  /* Start offset of block 83  */
      0x2A0000,  /* Start offset of block 84  */
      0x2A8000,  /* Start offset of block 85  */
      0x2B0000,  /* Start offset of block 86  */
      0x2B8000,  /* Start offset of block 87  */
      0x2C0000,  /* Start offset of block 88  */
      0x2C8000,  /* Start offset of block 89  */
      0x2D0000,  /* Start offset of block 90  */
      0x2D8000,  /* Start offset of block 91  */
      0x2E0000,  /* Start offset of block 92  */
      0x2E8000,  /* Start offset of block 93  */
      0x2F0000,  /* Start offset of block 94  */
      0x2F8000,  /* Start offset of block 95  */
      0x300000,  /* Start offset of block 96  */
      0x308000,  /* Start offset of block 97  */
      0x310000,  /* Start offset of block 98  */
      0x318000,  /* Start offset of block 99  */
      0x320000,  /* Start offset of block 100 */
      0x328000,  /* Start offset of block 101 */
      0x330000,  /* Start offset of block 102 */
      0x338000,  /* Start offset of block 103 */
      0x340000,  /* Start offset of block 104 */
      0x348000,  /* Start offset of block 105 */
      0x350000,  /* Start offset of block 106 */
      0x358000,  /* Start offset of block 107 */
      0x360000,  /* Start offset of block 108 */
      0x368000,  /* Start offset of block 109 */
      0x370000,  /* Start offset of block 110 */
      0x378000,  /* Start offset of block 111 */
      0x380000,  /* Start offset of block 112 */
      0x388000,  /* Start offset of block 113 */
      0x390000,  /* Start offset of block 114 */
      0x398000,  /* Start offset of block 115 */
      0x3A0000,  /* Start offset of block 116 */
      0x3A8000,  /* Start offset of block 117 */
      0x3B0000,  /* Start offset of block 118 */
      0x3B8000,  /* Start offset of block 119 */
      0x3C0000,  /* Start offset of block 120 */
      0x3C8000,  /* Start offset of block 121 */
      0x3D0000,  /* Start offset of block 122 */
      0x3D8000,  /* Start offset of block 123 */
      0x3E0000,  /* Start offset of block 124 */
      0x3E8000,  /* Start offset of block 125 */
      0x3F0000,  /* Start offset of block 126 */
      0x3F8000   /* Start offset of block 127 */
   }; /* EndArray BlockOffset[] */
#endif //if not defined ( _RAM_32_ROM_16_UPGRADE_ )

//TCSL - Define for the new size changes to the RAM and ROM
#ifdef _RAM_32_ROM_16_UPGRADE_
   static const udword BlockOffset[] = 
	{
	  0x000000, /* Start offset of block 0 Start of Secondary Image*/
	  0x010000, /* Start offset of block 1 */
	  0x020000, /* Start offset of block 2 */
	  0x030000, /* Start offset of block 3 */
	  0x040000, /* Start offset of block 4 */
	  0x050000, /* Start offset of block 5 */
	  0x060000, /* Start offset of block 6 */
	  0x070000, /* Start offset of block 7 */
	  0x080000, /* Start offset of block 8 */
	  0x090000, /* Start offset of block 9 */
	  0x0A0000, /* Start offset of block 10 */
	  0x0B0000, /* Start offset of block 11 */
	  0x0C0000, /* Start offset of block 12 */
	  0x0D0000, /* Start offset of block 13 */
	  0x0E0000, /* Start offset of block 14 */
	  0x0F0000, /* Start offset of block 15 */
	  0x100000, /* Start offset of block 16 */
	  0x110000, /* Start offset of block 17 */
	  0x120000, /* Start offset of block 18 */
	  0x130000, /* Start offset of block 19 */
	  0x140000, /* Start offset of block 20 */
	  0x150000, /* Start offset of block 21 */
	  0x160000, /* Start offset of block 22 */
	  0x170000, /* Start offset of block 23 */
	  0x180000, /* Start offset of block 24 */
	  0x190000, /* Start offset of block 25 */
	  0x1A0000, /* Start offset of block 26 */
	  0x1B0000, /* Start offset of block 27 */
	  0x1C0000, /* Start offset of block 28 */
	  0x1D0000, /* Start offset of block 29 */
	  0x1E0000, /* Start offset of block 30 */
	  0x1F0000, /* Start offset of block 31 */
	  0x200000, /* Start offset of block 32 */
	  0x210000, /* Start offset of block 33 */
	  0x220000, /* Start offset of block 34 */
	  0x230000, /* Start offset of block 35 */
	  0x240000, /* Start offset of block 36 */
	  0x250000, /* Start offset of block 37 */
	  0x260000, /* Start offset of block 38 */
	  0x270000, /* Start offset of block 39 */
	  0x280000, /* Start offset of block 40 */
	  0x290000, /* Start offset of block 41 */
	  0x2A0000, /* Start offset of block 42 */
	  0x2B0000, /* Start offset of block 43 */
	  0x2C0000, /* Start offset of block 44 */
	  0x2D0000, /* Start offset of block 45 */
	  0x2E0000, /* Start offset of block 46 */
	  0x2F0000, /* Start offset of block 47 */
	  0x300000, /* Start offset of block 48 Start of Primary Image*/
	  0x310000, /* Start offset of block 49 */
	  0x320000, /* Start offset of block 50 */
	  0x330000, /* Start offset of block 51 */
	  0x340000, /* Start offset of block 52 */
	  0x350000, /* Start offset of block 53 */
	  0x360000, /* Start offset of block 54 */
	  0x370000, /* Start offset of block 55 */
	  0x380000, /* Start offset of block 56 */
	  0x390000, /* Start offset of block 57 */
	  0x3A0000, /* Start offset of block 58 */
	  0x3B0000, /* Start offset of block 59 */
	  0x3C0000, /* Start offset of block 60 */
	  0x3D0000, /* Start offset of block 61 */
	  0x3E0000, /* Start offset of block 62 */
	  0x3F0000, /* Start offset of block 63 */
	  0x400000, /* Start offset of block 64 */
	  0x410000, /* Start offset of block 65 */
	  0x420000, /* Start offset of block 66 */
	  0x430000, /* Start offset of block 67 */
	  0x440000, /* Start offset of block 68 */
	  0x450000, /* Start offset of block 69 */
	  0x460000, /* Start offset of block 70 */
	  0x470000, /* Start offset of block 71 */
	  0x480000, /* Start offset of block 72 */
	  0x490000, /* Start offset of block 73 */
	  0x4A0000, /* Start offset of block 74 */
	  0x4B0000, /* Start offset of block 75 */
	  0x4C0000, /* Start offset of block 76 */
	  0x4D0000, /* Start offset of block 77 */
	  0x4E0000, /* Start offset of block 78 */
	  0x4F0000, /* Start offset of block 79 */
	  0x500000, /* Start offset of block 80 */
	  0x510000, /* Start offset of block 81 */
	  0x520000, /* Start offset of block 82 */
	  0x530000, /* Start offset of block 83 */
	  0x540000, /* Start offset of block 84 */
	  0x550000, /* Start offset of block 85 */
	  0x560000, /* Start offset of block 86 */
	  0x570000, /* Start offset of block 87 */
	  0x580000, /* Start offset of block 88 */
	  0x590000, /* Start offset of block 89 */
	  0x5A0000, /* Start offset of block 90 */
	  0x5B0000, /* Start offset of block 91 */
	  0x5C0000, /* Start offset of block 92 */
	  0x5D0000, /* Start offset of block 93 */
	  0x5E0000, /* Start offset of block 94 */
	  0x5F0000, /* Start offset of block 95 */
	  0x600000, /* Start offset of block 96 Boot Loader*/
	  0x610000, /* Start offset of block 97 Bit Patter / Test Code*/
	  0x620000, /* Start offset of block 98 Secondary - Config*/
	  0x630000, /* Start offset of block 99 Secondary - Log*/
	  0x640000, /* Start offset of block 100 Secondary - GSC*/
	  0x650000, /* Start offset of block 101 Secondary - No Boot*/
	  0x660000, /* Start offset of block 102 Secondary - Not Used*/
	  0x670000, /* Start offset of block 103 Secondary - Not Used*/
	  0x680000, /* Start offset of block 104 Secondary - Not Used*/
	  0x690000, /* Start offset of block 105 */
	  0x6A0000, /* Start offset of block 106 */
	  0x6B0000, /* Start offset of block 107 */
	  0x6C0000, /* Start offset of block 108 */
	  0x6D0000, /* Start offset of block 109 */
	  0x6E0000, /* Start offset of block 110 */
	  0x6F0000, /* Start offset of block 111 */
	  0x700000, /* Start offset of block 112 */
	  0x710000, /* Start offset of block 113 */
	  0x720000, /* Start offset of block 114 */
	  0x730000, /* Start offset of block 115 */
	  0x740000, /* Start offset of block 116 */
	  0x750000, /* Start offset of block 117 */
	  0x760000, /* Start offset of block 118 */
	  0x770000, /* Start offset of block 119 */
	  0x780000, /* Start offset of block 120 */
	  0x790000, /* Start offset of block 121 Primary - Config*/
	  0x7A0000, /* Start offset of block 122 Primary - Log*/
	  0x7B0000, /* Start offset of block 123 Primary - GSC*/
	  0x7C0000, /* Start offset of block 124 Primary - No Boot*/
	  0x7D0000, /* Start offset of block 125 Primary - Not Used*/
	  0x7E0000, /* Start offset of block 126 Primary - Not Used*/
	  0x7F0000 /* Start offset of block 127 Primary - Not Used*/
   }; /* EndArray BlockOffset[] */
#endif //if defined ( _RAM_32_ROM_16_UPGRADE_ )

#endif /* USE_M29W641D */
//TCSL - Changes to the Flash offset for 64 KWord
#define NUM_BLOCKS (sizeof(BlockOffset)/sizeof(BlockOffset[1]))

/*******************************************************************************
Not-standard Function Prototypes
*******************************************************************************/





/*******************************************************************************
List of Errors and Return values, Explanations and Help.
********************************************************************************

Error Name:   Flash_AddressInvalid
Description:  The address offset given is out of the range of the flash device.
Solution:     Check the address offset whether it is in the valid range of the
              flash device.
********************************************************************************

Error Name:   Flash_BankEraseFailed
Description:  The bank erase command did not finish successfully.
Solution:     Check that Vpp is not floating. Try erasing the block again. If
              this fails once more, the device may be faulty and needs to be
              replaced.
********************************************************************************

Error Name:   Flash_BlockEraseFailed
Description:  The block erase command did not finish successfully.
Solution:     Check that Vpp is not floating. Try erasing the block again. If
              this fails once more, the device may be faulty and needs to be
              replaced.
********************************************************************************

Error Name:   Flash_BlockNrInvalid
Note:         This is not a flash problem.
Description:  A selection for a block has been made (Parameter), which is not
              within the valid range. Valid block numbers are from 0 to
              NUM_BLOCKS-1.
Solution:     Check that the block number given is in the valid range.
********************************************************************************

Error Name:   Flash_BlockProtected
Description:  The user has attempted to erase, program or protect a block of
              the flash that is protected. The operation failed because the
              block in question is protected. This message appears also after
              checking the protection status of a block.
Solutions:    Choose another (unprotected) block for erasing or programming.
              Alternatively change the block protection status of the current
              block (see Datasheet for more details). In case of the user is
              protecting a block that is already protected, this warning notifies
              the user that the command had no effect.
********************************************************************************

Error Name:   Flash_BlockProtectFailed
Description:  This error return value indicates that a block protect command did
              not finish successfully.
Solution:     Check that Vpp is not floating but is tied to a valid voltage. Try
              the command again. If it fails a second time then the block cannot
              be protected and it may be necessary to replace the device.
********************************************************************************

Error Name:   Flash_BlockProtectionUnclear
Description:  The user has attempted to erase, program or protect a block of
              the flash which did not return a proper protection status. The
              operation has been cancelled.
Solutions:    This should only happen in configurations with more than one
              flash device. If the response of each device does not match, this
              return code is given. Mostly it means the usage of not properly
              initialized flash devices.
********************************************************************************

Error Name:   Flash_BlockUnprotected
Description:  This message appears after checking the block protection status.
              This block is ready to get erased, programmed or protected.
********************************************************************************

Error Name:   Flash_BlockUnprotectFailed
Description:  This error return value indicates that a block unprotect command
              did not finish successfully.
Solution:     Check that Vpp is not floating but is tied to a valid voltage. Try
              the command again. If it fails a second time then the block cannot
              be unprotected and it may be necessary to replace the device.
********************************************************************************

Error Name:   Flash_CfiFailed
Description:  This return value indicates that a Common Flash Interface (CFI)
              read access was unsuccessful.
Solution:     Try to read the Identifier Codes (Manufacture ID, Device ID)
              if these commands fail as well it is likely that the device is
              faulty or the interface to the flash is not correct.
********************************************************************************

Error Name:   Flash_ChipEraseFailed
Description:  This message indicates that the erasure of the whole device has
              failed.
Solution:     Investigate this failure further by checking the results array
              (parameter), where all blocks and their erasure results are listed.
              What is more, try to erase each block individually. If erasing a
              single block still causes failure, then the Flash device may need
              to be replaced.
********************************************************************************

Error Name:   Flash_ChipUnprotectFailed
Description:  This return value indicates that the chip unprotect command
              was unsuccessful.
Solution:     Check that Vpp is not floating but is tied to a valid voltage. Try
              the command again. If it fails a second time then it is likely that
              the device cannot be unprotected and will need to be replaced.
********************************************************************************

Return Name:  Flash_FunctionNotSupported
Description:  The user has attempted to make use of functionality not
              available on this flash device (and thus not provided by the
              software drivers).
Solution:     This can happen after changing Flash SW Drivers in existing
              environments. For example an application tries to use
              functionality which is then no longer provided with a new device.
********************************************************************************

Error Name:   Flash_GroupProtectFailed
Description:  This error return value indicates that a group protect command did
              not finish successfully.
Solution:     Check that Vpp is not floating but is tied to a valid voltage. Try
              the command again. If it fails a second time then the group cannot
              be protected and it may be necessary to replace the device.
********************************************************************************

Return Name:  Flash_NoInformationAvailable
Description:  The system can't give any additional information about the error.
Solution:     None
********************************************************************************

Error Name:   Flash_NoOperationToSuspend
Description:  This message is returned by a suspend operation if there isn't
              operation to suspend (i.e. the program/erase controller is inactive).
********************************************************************************

Error Name:   Flash_OperationOngoing
Description:  This message is one of two messages which are given by the TimeOut
              subroutine. That means the flash operation still operates within
              the defined time frame.
********************************************************************************

Error Name:   Flash_OperationTimeOut
Description:  The Program/Erase Controller algorithm could not finish an
              operation successfully. It should have set bit 7 of the status
              register from 0 to 1, but that did not happen within a predefined
              time. The program execution has been, therefore, cancelled by a
              timeout. This may be because the device is damaged.
Solution:     Try the previous command again. If it fails a second time then it
              is likely that the device will need to be replaced.
********************************************************************************

Error Name:   Flash_ProgramFailed
Description:  The value that should be programmed has not been written correctly
              to the flash.
Solutions:    Make sure that the block which is supposed to receive the value
              was erased successuly before programming. Try erasing the block and
              programming the value again. If it fails again then the device may
              be faulty.
********************************************************************************

Error Name:   Flash_ResponseUnclear
Description:  This message appears in multiple flash configurations, if the
              single flash responses are different and, therefore, a sensible
              reaction of the SW driver is not possible.
Solutions:    Check all the devices currently used and make sure they are all
              working properly. Use only equal devices in multiple configurations.
              If it fails again then the devices may be faulty and need to be
              replaced.
********************************************************************************

Error Name:   Flash_SpecificError
Description:  The function makes an error depending on the device.
              More information about the error are available into the ErrorInfo
              variable.
Solutions:    See SpecificReturnType remarks
********************************************************************************

Return Name:  Flash_Success
Description:  This value indicates that the flash command has executed
              correctly.
********************************************************************************

Error Name:   Flash_VppInvalid
Description:  A Program or a Block Erase has been attempted with the Vpp supply
              voltage outside the allowed ranges. This command had no effect
              since an invalid Vpp has the effect of protecting the whole of
              flash device.
Solution:     The (hardware) configuration of Vpp will need to be modified to
              enable programming or erasing of the device.
*******************************************************************************

Error Name:   Flash_WrongType
Description:  This message appears if the Manufacture and Device ID read from
              the current flash device do not match with the expected identifier
              codes. That means the source code is not explicitely written for
              the currently used flash chip. It may work, but it cannot be
              guaranteed.
Solutions:    Use a different flash chip with the target hardware or contact
              STMicroelectronics for a different source code library.
********************************************************************************/
/*******************************************************************************
List of Specific Errors and Return values, Explanations and Help.
********************************************************************************

*******************************************************************************

Error Name:   FlashSpec_MpuTooSlow
Notes:        Applies to M29 series Flash only.
Description:  The MPU has not managed to write all of the selected blocks to the
              device before the timeout period expired. See BLOCK ERASE COMMAND
              section of the Data Sheet for details.
Solutions:    If this occurs occasionally then it may be because an interrupt is
              occuring between writing the blocks to be erased. Search for "DSI!" in
              the code and disable interrupts during the time critical sections.
              If this error condition always occurs then it may be time for a faster
              microprocessor, a better optimising C compiler or, worse still, learn
              assembly. The immediate solution is to only erase one block at a time.

********************************************************************************

Error Name:   FlashSpec_ToggleFailed
Notes:        This applies to M29 series Flash only.
Description:  The Program/Erase Controller algorithm has not managed to complete
              the command operation successfully. This may be because the device is
              damaged.
Solution:     Try the command again. If it fails a second time then it is likely that
              the device will need to be replaced.

******************************************************************************
Error Name:   FlashSpec_Too_ManyBlocks
Notes:        Applies to M29 series Flash only.
Description:  The user has chosen to erase more blocks than the device has.
              This may be because the array of blocks to erase contains the same
              block more than once.
Solutions:    Check that the program is trying to erase valid blocks. The device
              will only have NUM_BLOCKS blocks (defined at the top of the file).
              Also check that the same block has not been added twice or more to
              the array.

********************************************************************************/

#endif /* In order to avoid a repeated usage of the header file */

/*******************************************************************************
     End of c1648.h
********************************************************************************/
