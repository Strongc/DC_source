/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                                 GRUNDFOS                                 */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*               --------------------------------------------               */
/*                Project: MRC                                              */
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
/* FILE NAME        : utf8_util.c                                           */
/*                                                                          */
/* CREATED DATE     : 17-06-2008                                            */
/*                                                                          */
/* SHORT FILE DESCRIPTION :                                                 */
/* Converts strings between UTF-8 and SMS format (that is UCS-2 in ASCII    */
/* encoded hex values or directly in ASCII if GSM 3.38 compatible.          */
/*                                                                          */
/****************************************************************************/
/*****************************************************************************
  LOCAL INCLUDES
 ****************************************************************************/
#include <typedef.h>    /*definements for the Grundfos common types */
//#include "gsm_util.h"
#include "utf8_util.h"

GF_UINT16 ConvertUtf8ToSmsOct(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes, GF_UINT1* pIsDestGsm338);
GF_UINT16 ConvertSmsOctToUtf8(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes);


GF_UINT8 Gsm338CompatibleAsciiToAnsi(const char* pAscii, GF_UINT32* pAnsi);
GF_UINT8 AnsiToUtf8(const GF_UINT32 ascii, char* pUft8);

GF_UINT8 Utf8ToAnsi(const char* pUft8, GF_UINT32* pAnsi);
GF_UINT8 AnsiToGsm338CompatibleAscii(const GF_UINT32 ansi, char* pAscii);

GF_UINT8 HalfByteToAscii(const GF_UINT8 halfByte);
GF_UINT8 AsciiToByte(const GF_UINT8 ascii1, const GF_UINT8 ascii2);

/*****************************************************************************
 * Function - ConvertUtf8ToSms
 * DESCRIPTION:
 * If the UTF-8 encoded source string is GSM 3.38 compatible it is returned in this format
 * (and not ASCII encoded). If it is not GSM 3.38 it is converted into UCS-2 and "ASCII encoded hex values".
 * Read pIsDestGsm338 to know if destination string is GSM 3.38 compatible
 * e.g.(not GSM 3.38 comp.) pSrc = "test`" ({0x74,0x65,0x73,0x74,0x60}) => pDest = "00740065007300740060"
 * e.g.(is GSM 3.38 comp.) pSrc = "test" ({0x74,0x65,0x73,0x74}) => pDest = "test"
 *****************************************************************************/
GF_UINT16 ConvertUtf8ToSms(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes, GF_UINT1* pIsDestGsm338)
{
  GF_UINT16 no_of_bytes = 0;
  GF_UINT8 swapped_byte = 0;
  int i = 0;

  no_of_bytes = ConvertUtf8ToSmsOct(pSrc, pDest, maxNoOfBytes/2, pIsDestGsm338);

  if (no_of_bytes > 0)
  {
    if (!*pIsDestGsm338)
    {
      char* pDestAsciiEnd = pDest + (2 * no_of_bytes) - 1;
      GF_UINT16 no_of_ascii = 0;

      // swap and convert bytes to ASCII (IRA5) characters
      // iterates backwards to avoid need of local buffer
      for (i = no_of_bytes-2; i >= 0; i-=2)
      {
        swapped_byte = pDest[i+1];

        *pDestAsciiEnd = HalfByteToAscii(pDest[i] & 0x0f);
        pDestAsciiEnd--;

        *pDestAsciiEnd = HalfByteToAscii((pDest[i] & 0xf0) >> 4);
        pDestAsciiEnd--;

        *pDestAsciiEnd = HalfByteToAscii(swapped_byte & 0x0f);
        pDestAsciiEnd--;

        *pDestAsciiEnd = HalfByteToAscii((swapped_byte & 0xf0) >> 4);
        pDestAsciiEnd--;

        no_of_ascii += 4;
      }

      no_of_bytes = no_of_ascii;
    }

    pDest[no_of_bytes] = '\0';

  }
  else
  {
    no_of_bytes = 0;
  }

  return no_of_bytes;

}


/*****************************************************************************
 * Function - ConvertSmsToUtf8
 * DESCRIPTION:
 *  Converts an SMS ASCII string (GSM 3.38 or UCS-2) to UTF-8.
 * e.g. pSrc = "74657374" => pDest = {0x74,0x65,0x73,0x74} ("test")
 *****************************************************************************/
GF_UINT16 ConvertSmsToUtf8(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes, GF_UINT1 sourceIsAsciiHexValues)
{
  GF_UINT16 no_of_bytes = 0;

  if (sourceIsAsciiHexValues)
  {
    int i = 0;

    char* pSrcAsBuffer = (char*) pSrc;

    GF_UINT8 ascii_hi, ascii_lo;

    // convert ASCII (IRA5) characters to buffer of chars
    for (i = 0; i < maxNoOfBytes - 1; i++)
    {
      if (*pSrc == '\0')
      {
        pSrcAsBuffer[i] = '\0';
        pSrcAsBuffer[i+1] = '\0';
        break;
      }

      ascii_hi = *pSrc;
      pSrc++;
      ascii_lo = *pSrc;
      pSrc++;

      pSrcAsBuffer[i] = AsciiToByte(ascii_hi, ascii_lo);
    }
    pSrcAsBuffer[i] = '\0';
    no_of_bytes = ConvertSmsOctToUtf8(pSrcAsBuffer, pDest, maxNoOfBytes);
  }
  else
  {
    no_of_bytes = ConvertSmsOctToUtf8(pSrc, pDest, maxNoOfBytes);
  }

  return no_of_bytes;
}

/*****************************************************************************
 * Function - ConvertUtf8ToSms
 * DESCRIPTION:
 *  Converts an UTF-8 encoded string to ASCII (if GSM 3.38 compatible) else to UCS-2.
 *****************************************************************************/
static GF_UINT16 ConvertUtf8ToSmsOct(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes, GF_UINT1* pIsGsm338)
{
  GF_UINT32 ansi = 0;
  GF_UINT16 no_of_bytes = 0;
  GF_UINT8 charsRead = 1;
  GF_UINT8 charsWritten = 0;

  UTF8* u8_source_start = (UTF8*)pSrc;
  UTF8* u8_source_end = (UTF8*)u8_source_start + strlen((GF_UINT8 *)pSrc);
  UTF16* u16_target_start = (UTF16*)pDest;
  UTF16* pStartOfDest = (UTF16*)pDest;
  ConversionResult rc;

  if (u8_source_end - u8_source_start > maxNoOfBytes)
  {// unable to convert the full source message... ignore the last part
    u8_source_end = (UTF8*)u8_source_start + maxNoOfBytes;
  }

  *pIsGsm338 = TRUE;

  while (*pSrc != '\0' && charsRead > 0 && no_of_bytes <= maxNoOfBytes)
  {
    charsRead = Utf8ToAnsi(pSrc, &ansi);

    pSrc += charsRead;

    charsWritten = AnsiToGsm338CompatibleAscii(ansi, pDest);

    pDest += charsWritten;
    no_of_bytes += charsWritten;

    if (charsWritten == 0)
    {
      // ANSI value is not GSM 3.88 compatible
      // convert to UCS-2
      *pIsGsm338 = FALSE;

      no_of_bytes = 0;

      if (charsRead > 3)
      {
        // ANSI value is not UCS-2 compatible either
        // abort conversion
        *pDest = '\0';
        return 0;
      }

      //convert string to UTF-16 (UCS-2)
      rc = ConvertUTF8toUTF16((const UTF8**)&u8_source_start,
                              (const UTF8*)u8_source_end,
                              &u16_target_start,
                              &u16_target_start[maxNoOfBytes - 1],
                              lenientConversion);//replaces illegal chars with squares
      *u16_target_start = '\0';

      if (rc == conversionOK)
      {
        no_of_bytes = ((char*)u16_target_start - (char*)pStartOfDest);
        return no_of_bytes;
      }
      else
      {
        return 0;
      }
    }
  }

  return no_of_bytes;
}


/*****************************************************************************
 * Function - ConvertSmsToUtf8
 * DESCRIPTION:
 *  Converts an SMS string (GSM 3.38 or UCS-2) to UTF-8.
 *****************************************************************************/
static GF_UINT16 ConvertSmsOctToUtf8(const char* pSrc, char* pDest, GF_UINT16 maxNoOfBytes)
{
  GF_UINT16 no_of_bytes = 0;
  GF_UINT32 ansi = 0;

  // Detect UCS-2 (this method only works with following two assumptions(!):
  // length of a non-UCS2 string > 1 AND first charcode of a UCS2 string is < 0xff)
  if ((*(UTF16*)pSrc & 0xff00) == 0 || (*(UTF16*)pSrc & 0x00ff) == 0)
  //if ((pSrc[0] & 0x80) > 0)
  {
    // convert UCS-2 encoded string into UTF-8

    UTF16* u16_source_start = (UTF16*)pSrc;
    UTF16* u16_source_end = (UTF16*)u16_source_start + wcslen((UTF16*)pSrc);
    UTF8*  u8_target_start = (UTF8*)pDest;
    UTF8*  pStartOfDest = (UTF8*)pDest;

    // make sure endianness fits ConvertUTF16toUTF8
    if ((*(UTF16*)pSrc & 0x00ff) == 0)
    {
      // need to swap bytes
      UTF16 swap;
      GF_UINT16 i, len = u16_source_end - u16_source_start;
     
      for (i = 0; i < len; i++)
      {
        swap  = ((u16_source_start[i] & 0xff00) >> 8);
        swap |= ((u16_source_start[i] & 0x00ff) << 8);
        u16_source_start[i] = swap;
      }
    }

    // NB ConvertUTF16toUTF8 only supports little endian format!
    ConvertUTF16toUTF8((const UTF16**)&u16_source_start,
                      (const UTF16*)u16_source_end,
                      &u8_target_start,
                      &u8_target_start[maxNoOfBytes - 1],
                      lenientConversion);//replaces illegal chars with squares

    *u8_target_start = '\0';

    return strlen(pStartOfDest);
  }
  else
  {
    GF_UINT8 bytesRead, bytesWritten;
    
    while (*pSrc != '\0' && no_of_bytes <= maxNoOfBytes)
    {
      bytesRead = Gsm338CompatibleAsciiToAnsi(pSrc, &ansi);
      if (bytesRead > 1)
      {
        pSrc++;
      }

      bytesWritten = AnsiToUtf8(ansi, pDest);

      pSrc++;

      pDest += bytesWritten;

      no_of_bytes += bytesWritten;
    }

    *pDest = '\0';

  }

  return no_of_bytes;
}

/*****************************************************************************
 * Function - utf8ToAnsi
 * DESCRIPTION:
 *
 *****************************************************************************/
static GF_UINT8 Utf8ToAnsi(const char* pUft8, GF_UINT32* pAnsi)
{
  if ((*pUft8 & 0x80) == 0)// one byte value
  {
    *pAnsi  = ((GF_UINT32)*pUft8         & 0x0000007f);
    return 1;
  }
  else if ((*pUft8 & 0xE0) == 0xC0)// two byte value
  {
    *pAnsi  = (((GF_UINT32)*pUft8 << 6 ) & 0x000007c0);
    pUft8++;
    *pAnsi |= ((GF_UINT32)*pUft8         & 0x0000003f);
    return 2;
  }
  else if ((*pUft8 & 0xF0) == 0xE0)// three byte value
  {
    *pAnsi  = (((GF_UINT32)*pUft8 << 12) & 0x0000f000);
    pUft8++;
    *pAnsi |= (((GF_UINT32)*pUft8 << 6 ) & 0x00000fc0);
    pUft8++;
    *pAnsi |= ((GF_UINT32)*pUft8         & 0x0000003f);
    return 3;
  }
  else if ((*pUft8 & 0xF8) == 0xF0)// four byte value
  {
    *pAnsi =  (((GF_UINT32)*pUft8 << 18) & 0x001c0000);
    pUft8++;
    *pAnsi |= (((GF_UINT32)*pUft8 << 12) & 0x0003f000);
    pUft8++;
    *pAnsi |= (((GF_UINT32)*pUft8 << 6 ) & 0x00000fc0);
    pUft8++;
    *pAnsi |= ((GF_UINT32)*pUft8         & 0x0000003f);
    return 4;
  }

  // pUft8 does not point at a valid UTF8 char.
  *pAnsi = 0x00000000;

  return 0;
}


/*****************************************************************************
 * Function - AnsiToGsm338CompatibleAscii
 * DESCRIPTION:
 *
 *****************************************************************************/
static GF_UINT8 AnsiToGsm338CompatibleAscii(const GF_UINT32 ansi, char* pAscii)
{
  if (   ansi == 10
      || ansi == 13
      ||(ansi >= 32 && ansi <= 35)
      ||(ansi >= 37 && ansi <= 63)
      ||(ansi >= 65 && ansi <= 90)
      ||(ansi >= 97 && ansi <= 122) )
  {
    *pAscii = (GF_UINT8)(ansi & 0xff);
  }
  else
  {
    switch ( ansi )
    {
      //case 64 : // @
      //  *pAscii = (GF_UINT8)(0);
      //  break;
      case 163: // £
        *pAscii = (GF_UINT8)(1);
        break;
      case 36 : // $
        *pAscii = (GF_UINT8)(2);
        break;
      case 165: // ¥
        *pAscii = (GF_UINT8)(3);
        break;
      case 232: // è
        *pAscii = (GF_UINT8)(4);
        break;
      case 233: // é
        *pAscii = (GF_UINT8)(5);
        break;
      case 249: // ù
        *pAscii = (GF_UINT8)(6);
        break;
      case 236: // ì
        *pAscii = (GF_UINT8)(7);
        break;
      case 242: // ò
        *pAscii = (GF_UINT8)(8);
        break;
      case 199: // Ç
        *pAscii = (GF_UINT8)(9);
        break;
      case 216: // Ø
        *pAscii = (GF_UINT8)(11);
        break;
      case 248: // ø
        *pAscii = (GF_UINT8)(12);
        break;
      case 197: // Å
        *pAscii = (GF_UINT8)(14);
        break;
      case 229: // å
        *pAscii = (GF_UINT8)(15);
        break;
      case 916: // delta
        *pAscii = (GF_UINT8)(16);
        break;
      case 95 : // _
        *pAscii = (GF_UINT8)(17);
        break;
      case 934: // phi
        *pAscii = (GF_UINT8)(18);
        break;
      case 915: // gamma
        *pAscii = (GF_UINT8)(19);
        break;
      case 923: // lambda
        *pAscii = (GF_UINT8)(20);
        break;
      case 937: // omega
        *pAscii = (GF_UINT8)(21);
        break;
      case 928: // pi
        *pAscii = (GF_UINT8)(22);
        break;
      case 936: // psi
        *pAscii = (GF_UINT8)(23);
        break;
      case 931: // sigma
        *pAscii = (GF_UINT8)(24);
        break;
      case 920: // theta
        *pAscii = (GF_UINT8)(25);
        break;
      case 926: // xi
        *pAscii = (GF_UINT8)(26);
        break;
      case 12 : // form feed
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(10);
        return 2;
      case 94 : // ^
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(20);
        return 2;
      case 123: // {
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(40);
        return 2;
      case 125: // }
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(41);
        return 2;
      case 92 : // backslash
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(47);
        return 2;
      case 91 : // [
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(60);
        return 2;
      case 126: // ~
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(61);
        return 2;
      case 93 : // ]
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(62);
        return 2;
      case 124: // |
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(64);
        return 2;
      case 8364: // €
        pAscii[0] = (GF_UINT8)(27);
        pAscii[1] = (GF_UINT8)(101);
        return 2;
      case 198: // Æ
        *pAscii = (GF_UINT8)(28);
        break;
      case 230: // æ
        *pAscii = (GF_UINT8)(29);
        break;
      case 223: // ß
        *pAscii = (GF_UINT8)(30);
        break;
      case 201: // É
        *pAscii = (GF_UINT8)(31);
        break;
      case 164: // ¤
        *pAscii = (GF_UINT8)(36);
        break;
      case 161: // ¡
        *pAscii = (GF_UINT8)(64);
        break;
      case 196: // Ä
        *pAscii = (GF_UINT8)(91);
        break;
      case 214: // Ö
        *pAscii = (GF_UINT8)(92);
        break;
      case 209: // Ñ
        *pAscii = (GF_UINT8)(93);
        break;
      case 220: // Ü
        *pAscii = (GF_UINT8)(94);
        break;
      case 167: // §
        *pAscii = (GF_UINT8)(95);
        break;
      case 191: // ¿
        *pAscii = (GF_UINT8)(96);
        break;
      case 228: // ä
        *pAscii = (GF_UINT8)(123);
        break;
      case 246: // ö
        *pAscii = (GF_UINT8)(124);
        break;
      case 241: // ñ
        *pAscii = (GF_UINT8)(125);
        break;
      case 252: // ü
        *pAscii = (GF_UINT8)(126);
        break;
      case 224: // à
        *pAscii = (GF_UINT8)(127);
        break;
        // no mapping found
      default: return 0;
    }
  }
  return 1;
}

/*****************************************************************************
 * Function - Gsm338CompatibleAsciiToAnsi
 * DESCRIPTION:
 *
 *****************************************************************************/
static GF_UINT8 Gsm338CompatibleAsciiToAnsi(const char* pAscii, GF_UINT32* pAnsi)
{
  char ascii = *pAscii;

  if (   ascii == 10
      || ascii == 13
      ||(ascii >= 32 && ascii <= 35)
      ||(ascii >= 37 && ascii <= 63)
      ||(ascii >= 65 && ascii <= 90)
      ||(ascii >= 97 && ascii <= 122) )
  {
    *pAnsi = (GF_UINT32)(ascii & 0xff);
  }
  else
  {
    switch (ascii)
    {
      case 0 : // @
        *pAnsi = (GF_UINT32)(64);
        break;
      case 1: // £
        *pAnsi = (GF_UINT32)(163);
        break;
      case 2: // $
        *pAnsi = (GF_UINT32)(36);
        break;
      case 3: // ¥
        *pAnsi = (GF_UINT32)(165);
        break;
      case 4: // è
        *pAnsi = (GF_UINT32)(232);
        break;
      case 5: // é
        *pAnsi = (GF_UINT32)(233);
        break;
      case 6: // ù
        *pAnsi = (GF_UINT32)(249);
        break;
      case 7: // ì
        *pAnsi = (GF_UINT32)(236);
        break;
      case 8: // ò
        *pAnsi = (GF_UINT32)(242);
        break;
      case 9: // Ç
        *pAnsi = (GF_UINT32)(199);
        break;
      case 11: // Ø
        *pAnsi = (GF_UINT32)(216);
        break;
      case 12: // ø
        *pAnsi = (GF_UINT32)(248);
        break;
      case 14: // Å
        *pAnsi = (GF_UINT32)(197);
        break;
      case 15: // å
        *pAnsi = (GF_UINT32)(229);
        break;
      case 16: // delta
        *pAnsi = (GF_UINT32)(916);
        break;
      case 17 : // _
        *pAnsi = (GF_UINT32)(95);
        break;
      case 18: // phi
        *pAnsi = (GF_UINT32)(934);
        break;
      case 19: // gamma
        *pAnsi = (GF_UINT32)(915);
        break;
      case 20: // lambda
        *pAnsi = (GF_UINT32)(923);
        break;
      case 21: // omega
        *pAnsi = (GF_UINT32)(937);
        break;
      case 22: // pi
        *pAnsi = (GF_UINT32)(928);
        break;
      case 23: // psi
        *pAnsi = (GF_UINT32)(936);
        break;
      case 24: // sigma
        *pAnsi = (GF_UINT32)(931);
        break;
      case 25: // theta
        *pAnsi = (GF_UINT32)(920);
        break;
      case 26: // xi
        *pAnsi = (GF_UINT32)(926);
        break;
      case 27:
        {
          pAscii++;
          switch(*pAscii)
          {
          case 10 : // form feed
            *pAnsi = (GF_UINT32)(12);
            return 2;
          case 20 : // ^
            *pAnsi = (GF_UINT32)(94);
            return 2;
          case 40: // {
            *pAnsi = (GF_UINT32)(123);
            return 2;
          case 41: // }
            *pAnsi = (GF_UINT32)(125);
            return 2;
          case 47 : // backslash
            *pAnsi = (GF_UINT32)(92);
            return 2;
          case 60 : // [
            *pAnsi = (GF_UINT32)(91);
            return 2;
          case 61: // ~
            *pAnsi = (GF_UINT32)(126);
            return 2;
          case 62 : // ]
            *pAnsi = (GF_UINT32)(93);
            return 2;
          case 64: // |
            *pAnsi = (GF_UINT32)(124);
            return 2;
          case 101:// €
            *pAnsi = (GF_UINT32)(8364);
            return 2;
          }
        }
      case 28: // Æ
        *pAnsi = (GF_UINT32)(198);
        break;
      case 29: // æ
        *pAnsi = (GF_UINT32)(230);
        break;
      case 30: // ß
        *pAnsi = (GF_UINT32)(223);
        break;
      case 31: // É
        *pAnsi = (GF_UINT32)(201);
        break;
      case 36: // ¤
        *pAnsi = (GF_UINT32)(164);
        break;
      case 64: // ¡
        *pAnsi = (GF_UINT32)(161);
        break;
      case 91: // Ä
        *pAnsi = (GF_UINT32)(196);
        break;
      case 92: // Ö
        *pAnsi = (GF_UINT32)(214);
        break;
      case 93: // Ñ
        *pAnsi = (GF_UINT32)(209);
        break;
      case 94: // Ü
        *pAnsi = (GF_UINT32)(220);
        break;
      case 95: // §
        *pAnsi = (GF_UINT32)(167);
        break;
      case 96: // ¿
        *pAnsi = (GF_UINT32)(191);
        break;
      case 123: // ä
        *pAnsi = (GF_UINT32)(228);
        break;
      case 124: // ö
        *pAnsi = (GF_UINT32)(246);
        break;
      case 125: // ñ
        *pAnsi = (GF_UINT32)(241);
        break;
      case 126: // ü
        *pAnsi = (GF_UINT32)(252);
        break;
      case 127: // à
        *pAnsi = (GF_UINT32)(224);
        break;
        // no mapping found
      default: return 0;
    }
  }
  return 1;

}


/*****************************************************************************
 * Function - AnsiToUtf8
 * DESCRIPTION:
 *
 *****************************************************************************/
static GF_UINT8 AnsiToUtf8(const GF_UINT32 ascii, char* pUft8)
{
  if (ascii <= 0x7f)
  {
    pUft8[0] = (char) ascii & 0x7f;
    return 1;
  }
  else if (ascii <= 0x7ff)
  {
    pUft8[0] = (char) ((ascii>>6) & 0x1f) | 0xc0;
    pUft8[1] = (char) (ascii & 0x3f) | 0x80;
    return 2;
  }
  else
  {
    pUft8[0] = (char) ((ascii>>12) & 0x0f) | 0xe0;
    pUft8[1] = (char) ((ascii>>6) & 0x3f) | 0x80;
    pUft8[2] = (char) ((ascii) & 0x3f) | 0x80;
    return 3;
  }
  
}

/*****************************************************************************
 * Function - HalfByteToAscii
 * DESCRIPTION:
 *
 *****************************************************************************/
static GF_UINT8 HalfByteToAscii(const GF_UINT8 halfByte)
{
  if (halfByte <= 0x09)
  {
    // [0-9] => [48-57]
    return 48 + halfByte;
  }
  else if (halfByte <= 0x0f)
  {
    // [A-F] => [65-70]
    //return char 65 - 0x0a + halfByte
    return 55 + halfByte;
  }

  return 32; //space
}

/*****************************************************************************
 * Function - AsciiToByte
 * DESCRIPTION:
 *
 *****************************************************************************/
static GF_UINT8 AsciiToByte(const GF_UINT8 ascii1, const GF_UINT8 ascii2)
{
  // [48-57] => [0-9]
  // [65-70] => [A-F]
  GF_UINT8 byte = 0;

  byte  = (ascii1 <= 57 ? (ascii1 - 48) : (ascii1 - 55)) << 4;
  byte |= (ascii2 <= 57 ? (ascii2 - 48) : (ascii2 - 55));

  return byte;
}


//#define TEST_SIZE 50

//void Gsm338test(void)
//{
//  GF_UINT1 isGsm388 = 0;
//  int number_of_bytes_written = 0;

//  char toUtf8[TEST_SIZE];
//  char toSms[TEST_SIZE];

//  const char fromSms[] = "746573741B65";
//  //const char fromUtf8[] = {'t','e','s','t', 0xe2, 0x82, 0xac, '\0'}; // "test€"
//  //const char fromUtf8[] = {0x60, 0x60, '\0'}; // "``" => -1 0 -2 0 96 0 96 0 0 0(ucs-2)
//  //const char fromUtf8[] = "hello 123";
//  //const char fromUtf8[] = "1 2 3 4 5 6 7 8 9 0";
//  //const char fromUtf8[] = {'t','e','s','t', 0xe6, 0x97, 0xa5, '\0'};//chinese "day"-symbol (ansi value=0xE565)
//  const char fromUtf8[] = {'t','e','s','t',0x60, '\0'}; // -> FFFE74006500730074006000


//  memset(toUtf8, 0xAA, TEST_SIZE);
//  memset(toSms, 0xAA, TEST_SIZE);

//  number_of_bytes_written = ConvertUtf8ToSms(fromUtf8, toSms, TEST_SIZE, &isGsm388);
//  OutputDebugStringA(toSms);

//  OutputDebugStringA("\n");

//  number_of_bytes_written = ConvertSmsToUtf8(fromSms, toUtf8, TEST_SIZE, 1);
//  OutputDebugStringA(toUtf8);

//  OutputDebugStringA("\n");
//}



