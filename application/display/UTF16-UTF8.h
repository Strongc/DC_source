#ifndef mpcDisplayUTF162UTF8_h
#define mpcDisplayUTF162UTF8_h
#ifdef __PC__

#include "cu351_cpu_types.h"
#include <ConvertUTF.h>

int UTF162UTF8(char* dest, char* src, int destByteSize);
ConversionResult UTF82UTF16(UTF16* dest, const UTF8* src, int destSize);
#endif
#endif
