/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:                                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S                     */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      :                                                       */
/*                                                                          */
/* FILE NAME        :                                                       */
/*                                                                          */
/* CREATED DATE     : 27-01-2004                                            */
/*                                                                          */
/* FILE DESCRIPTION :                                                       */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "c1648.h"

#include "FlashMemory.h"
//#include "sim.h"
/****************************************************************************/
/*                                                                          */
/* D E F I N E M E N T S                                                    */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* L O C A L    C O N S T A N T S                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* G L O B A L    V A R I A B L E S                                         */
/*                                                                          */
/****************************************************************************/
ErrorInfoType eiErrorInfo;

U16 flash_sim[FLASH_SIZE];


// Implementing FlashMemory.h

void GetFlashMemory(uCPUBusType** flash, int* sz) {
  *flash = flash_sim;
  *sz    = FLASH_SIZE;
}

/****************************************************************************/
/*                                                                          */
/* L O C A L    V A R I A B L E S                                           */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                                                                          */
/* L O C A L    P R O T O T Y P E S                                         */
/*                                                                          */
/****************************************************************************/
ReturnType FlashDataToggle( void );

/****************************************************************************/
/*                                                                          */
/* F U N C T I O N S                                                        */
/*                                                                          */
/****************************************************************************/

ReturnType Flash( CommandType cmdCommand, ParameterType *fp )
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  return Flash_Success;
}

ReturnType FlashBlockErase(uBlockType ublBlockNr)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  int addr, addr_end;

  addr = BlockOffset[ublBlockNr];
  addr_end = addr + 0x007FFF;
  for ( ; addr <= addr_end; addr++ )
  {
    flash_sim[addr] = 0xFFFF;
  }

  return Flash_Success;
}

uCPUBusType FlashRead( udword udAddrOff )
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  return flash_sim[udAddrOff];
}

ReturnType FlashDataToggle( void )
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  return Flash_Success;
}

ReturnType FlashSingleProgram(udword udAddrOff,uCPUBusType ucValue)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  flash_sim[udAddrOff] = ucValue;

  return Flash_Success;
}

ReturnType FlashTimeOut(udword udSeconds)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  return Flash_Success;
}
void FlashWrite( udword udAddrOff, uCPUBusType ucVal )
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
    flash_sim[udAddrOff] = ucVal;
}

ReturnType FlashProgram( udword udAddrOff, udword udNrOfElementsInArray, void *pArray )
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  uCPUBusType *ucpArrayPointer;
  udword udLastOff;

  udLastOff = udAddrOff + udNrOfElementsInArray - 1;
  ucpArrayPointer = (uCPUBusType *)pArray;
  while( udAddrOff <= udLastOff )
  {
    FlashWrite( ANY_ADDR, (uCPUBusType)CMD(0x00F0) );
    flash_sim[udAddrOff] = *ucpArrayPointer;
    ucpArrayPointer++;
    udAddrOff++;
  }
  return Flash_Success;
}

void FlashInit(void)
/****************************************************************************
* INPUT(S)             :
* OUTPUT(S)            :
* DESIGN DOC.          :
* FUNCTION DESCRIPTION :
*
****************************************************************************/
{
  //OS_CREATERSEMA(&mSemaFlashDrv);
}

/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
