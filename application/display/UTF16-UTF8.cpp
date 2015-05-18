#ifdef __PC__
#include "UTF16-UTF8.h"
#include <memory.h>

/*********************************************************************
*
*       _CalcSizeOfChar
*
* Purpose:
*   Return the number of bytes needed for the given character.
*/
int _CalcSizeOfChar(U16 Char) 
{
  int r;
  if (Char & 0xF800) {                /* Single byte (ASCII)  */
    r = 3;
  } else if (Char & 0xFF80) {         /* Double byte sequence */
    r = 2;
  } else {                            /* 3 byte sequence      */
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       _Encode
*
* Purpose:
*   Encode character into 1/2/3 bytes.
*/
int _Encode(char *s, U16 Char) {
  int r;
  r = _CalcSizeOfChar(Char);
  switch (r) {
  case 1:
    *s = (char)Char;
    break;
  case 2:
    *s++ = 0xC0 | (Char >> 6);
    *s   = 0x80 | (Char & 0x3F);
    break;
  case 3:
    *s++ = 0xE0 | (Char >> 12);
    *s++ = 0x80 | ((Char >> 6) & 0x3F);
    *s   = 0x80 | (Char & 0x3F);
    break;
  }
  return r;
}

int UTF162UTF8(char* dest, char* src, int destByteSize)
{
  U16*  pU16 = (U16*)src;
  int i = 0;
  char  tmp[3];
  char* destOff = dest;
  while(*pU16 != 0)
  {
    i = _Encode(tmp, *pU16);
    if(destOff + i > dest + destByteSize)
      return -1;
    memcpy(destOff, tmp, i);
    destOff += i;
    ++pU16;
  }
  *destOff = 0;
  return (destOff - dest);
}


ConversionResult UTF82UTF16(UTF16* dest, const UTF8* src, int destSize)
{
  UTF8* u8_source_start = (UTF8*)src;
  UTF8* u8_source_end = (UTF8*)u8_source_start + strlen((char*)src);
  UTF16* u6_target_start = (UTF16*)dest;

  ConversionResult rc = ConvertUTF8toUTF16((const UTF8**)&u8_source_start,(const UTF8*)u8_source_end,&u6_target_start,&u6_target_start[destSize - 1],strictConversion);

  *u6_target_start = 0;

  return rc;
}
#endif
