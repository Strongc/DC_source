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
//#include "sim.h"
#include <rtos.h>

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
OS_RSEMA mSemaFlashDrv;

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
  ReturnType  rRetVal;
  if (cmdCommand == BlockErase)
  {
    rRetVal = FlashBlockErase( (*fp).BlockErase.ublBlockNr );
  }
  if (cmdCommand ==  SingleProgram)
  {
    rRetVal = FlashSingleProgram((*fp).SingleProgram.udAddrOff, (*fp).SingleProgram.ucValue );
  }
  return rRetVal;
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
  ReturnType rRetVal = Flash_Success;    /* Holds return value: optimistic initially! */

  OS_Use(&mSemaFlashDrv);

  FlashWrite( 0x00555, (uCPUBusType)CMD(0x00AA) );
  FlashWrite( 0x002AA, (uCPUBusType)CMD(0x0055) );
  FlashWrite( 0x00555, (uCPUBusType)CMD(0x0080) );
  FlashWrite( 0x00555, (uCPUBusType)CMD(0x00AA) );
  FlashWrite( 0x002AA, (uCPUBusType)CMD(0x0055) );
  FlashWrite( BlockOffset[ublBlockNr], (uCPUBusType)CMD(0x0030) );

  FlashTimeOut(0);
  while( !(FlashRead( BlockOffset[ublBlockNr] ) & CMD(0x0008) ) )
  {
    if (FlashTimeOut(5) == Flash_OperationTimeOut)
    {
      FlashWrite( ANY_ADDR, (uCPUBusType)CMD(0x00F0) ); /* Use single instruction cycle method */
      return Flash_OperationTimeOut;
    }
  }

  if( FlashDataToggle() !=  Flash_Success )
  {
    FlashWrite( ANY_ADDR, (uCPUBusType)CMD(0x00F0) ); /* Use single instruction cycle method */
    rRetVal=Flash_BlockEraseFailed;
  }

  OS_Unuse(&mSemaFlashDrv);

  return rRetVal;
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
  return BASE_ADDR[udAddrOff];
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
  uCPUBusType ucVal1, ucVal2;

  FlashTimeOut(0);
  while(FlashTimeOut(120) != Flash_OperationTimeOut)
  {
    ucVal2 = FlashRead( ANY_ADDR );
    ucVal1 = FlashRead( ANY_ADDR );
    if( (ucVal1&CMD(0x0040)) == (ucVal2&CMD(0x0040)) )
    {
      return Flash_Success;
    }
    if( (ucVal2&CMD(0x0020)) != CMD(0x0020) )
    {
      continue;
    }
    ucVal1 = FlashRead( ANY_ADDR );
    ucVal2 = FlashRead( ANY_ADDR );
    if( (ucVal2&CMD(0x0040)) == (ucVal1&CMD(0x0040)) )
    {
      return Flash_Success;
    }
    else
    {
      eiErrorInfo.sprRetVal=FlashSpec_ToggleFailed;
      return Flash_SpecificError;
    }
  }
  return Flash_OperationTimeOut;
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
  uCPUBusType val;

  OS_Use(&mSemaFlashDrv);

  FlashWrite( 0x00555, (uCPUBusType)CMD(0x00AA) );  /* 1st cycle */
  FlashWrite( 0x002AA, (uCPUBusType)CMD(0x0055) );  /* 2nd cycle */
  FlashWrite( 0x00555, (uCPUBusType)CMD(0x00A0) );  /* Program command */
  FlashWrite( udAddrOff,ucValue );                  /* Program value */
  FlashDataToggle();
  val = FlashRead( udAddrOff );
  if (val != ucValue)
  {
    return Flash_OperationTimeOut;
  }

  OS_Unuse(&mSemaFlashDrv);

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
  static udword udCounter;

  if (udSeconds == 0)
  {
    udCounter = 0;
  }

  if (udCounter == (udSeconds * COUNT_FOR_A_SECOND))
  {
    return Flash_OperationTimeOut;
  }
  else
  {
    udCounter++;
    return Flash_OperationOngoing;
  }
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
  BASE_ADDR[udAddrOff] = ucVal;
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
  ReturnType rRetVal = Flash_Success;
  uCPUBusType *ucpArrayPointer;
  udword udLastOff;

  OS_Use(&mSemaFlashDrv);

  udLastOff = udAddrOff + udNrOfElementsInArray - 1;
  FlashWrite( 0x00555, (uCPUBusType)CMD(0x00AA) );  /* 1st cycle */
  FlashWrite( 0x002AA, (uCPUBusType)CMD(0x0055) );  /* 2nd cycle */
  FlashWrite( 0x00555, (uCPUBusType)CMD(0x0020) );  /* 3nd cycle */
  ucpArrayPointer = (uCPUBusType *)pArray;
  while( udAddrOff <= udLastOff )
  {
    FlashWrite( ANY_ADDR, CMD(0x00A0) );       /* 1st cycle */
    FlashWrite( udAddrOff, *ucpArrayPointer ); /* 2nd Cycle */
    if( FlashDataToggle() != Flash_Success)
    {
      FlashWrite( ANY_ADDR, (uCPUBusType)CMD(0x00F0) );
      rRetVal=Flash_ProgramFailed;
      eiErrorInfo.udGeneralInfo[0] = udAddrOff;
      break;
    }
    ucpArrayPointer++;
    udAddrOff++;
  }
  FlashWrite( ANY_ADDR, (uCPUBusType)CMD(0x0090) );  /* 1st cycle */
  FlashWrite( ANY_ADDR, (uCPUBusType)CMD(0x0000) );  /* 2st cycle */

  OS_Unuse(&mSemaFlashDrv);

  return rRetVal;
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
  OS_CREATERSEMA(&mSemaFlashDrv);
}

/****************************************************************************/
/*                                                                          */
/* E N D   O F   F I L E                                                    */
/*                                                                          */
/****************************************************************************/
