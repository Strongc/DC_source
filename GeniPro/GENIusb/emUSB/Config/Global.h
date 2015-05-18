/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
*                                                                    *
*       USB device stack for embedded applications                   *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : Global.h
Purpose : Global types
          In case your application already has a Global.h, you should
          merge the files. In order to use Segger code, the types
          U8, U16, U32, I8, I16, I32 need to be defined in Global.h;
          additional defintions do not hurt.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GLOBAL_H            // Guard against multiple inclusion
#define GLOBAL_H

#define U8    unsigned char
#define U16   unsigned short
#define U32   unsigned long
#define I8    signed char
#define I16   signed short
#define I32   signed long

#endif                      // Avoid multiple inclusion

/*************************** End of file ****************************/
