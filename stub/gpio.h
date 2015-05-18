/*****************************************************************************
* Copyright, Grundfos
* All rights reserved.
* No part of this program may be copied, used or delivered to anyone
* without the express written consent of Grundfos *
* MODULENAME.: GPIO_driver
*
* PROJECT....: MPC
*
* DESCRIPTION:
* This is the general purpose I/O driver
*
*****************************************************************************/

#ifndef MPCGPio_h
#define MPCGPio_h

#include "typedef.h"           /* definements for the Grundfos common types */
#include <cu351_cpu_types.h>        

// FORWARD REFERENCES
//

typedef enum _port_no
{
  GPIO0  = 0,
  GPIO1,
  GPIO2,
  GPIO3,
  GPIO4,
  GPIO5,
  GPIO6,
  GPIO7,
  GPIO8,
#ifdef TFT_16_BIT_LCD
  PWM_1,
#else
  GPIO9,
#endif
  GPIO10,
  GPIO11,
  GPIO12,
  GPIO13,
  GPIO14,
  GPIO15,
  GPIO16,
  GPIO17,
  GPIO18,
  GPIO19,
  GPIO20,
  GP_IO_LED_MENU,
  GP_IO_LED_QUESTION,
  GP_IO_LED_UP,
  GP_IO_LED_DOWN,
  GP_IO_LED_PLUS,
  GP_IO_LED_MINUS,
  GP_IO_LED_ESC,
  GP_IO_LED_OK,
  GP_IO_LED_HOME,
  GP_IO_LED_RED,
  GP_IO_LED_GREEN,
  EARLY_WARNING,
  GPIO33,
  GPIO34,
#ifdef TFT_16_BIT_LCD
  LCD_UD,
  LCD_LR,
#else
  ETHERNET_LED_A,
  ETHERNET_LED_B,
#endif
  ETHERNET_RESET,
  GPIO38,
#ifdef TFT_16_BIT_LCD
  LCD_RESET_BAR,
#else
  GP_IO_CONTRAST,
#endif
  GPIO40,
  GPIO41,
  GPIO42,
  GPIO43,
  GPIO44,
  GPIO45,
  GPIO46,
  GPIO47,
#ifdef TFT_16_BIT_LCD
  GPIO48,
  GPIO49,
  GPIO50,
  GPIO51,
#else
  DEBUG_LED_0,
  DEBUG_LED_1,
  DEBUG_LED_2,
  DEBUG_LED_3,
#endif
  GPIO52,
  GPIO53,
  GPIO54_,
  GPIO55_,
  _GPIO56_,
  _GPIO57_,
  _GPIO58_,
  _GPIO59_,
  _GPIO60_,
  _GPIO61_,
  GPO62,
  GPO63
} PORT_NO;


#ifdef __PC__
typedef enum _input_output_disable
{
  IO_INPUT = 0, OUTPUT, DISABLED
} INPUT_OUTPUT;
#else
typedef enum _input_output_disable
{
  INPUT = 0, OUTPUT, DISABLED
} INPUT_OUTPUT;
#endif



#ifdef __cplusplus
extern "C" {
#endif

  void C_SetAs(PORT_NO no, INPUT_OUTPUT direction, BIT value);
  void C_Set(PORT_NO no, BIT value);

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus


class GPio
{

private:
  static GPio* mInstance;
  /**
  * The GPIO's are setup in there default position or acording to the
  * setup in the GPIOSetupTable if USE_GPIO_SETUP_TABLE == 1 in gpio.cfg
  *
  */
  GPio(void);
  ~GPio();
  GPio& operator=(const GPio&);
  GPio(const GPio&);
public:
  /**
  * The returns the ...
  *
  */
  static GPio* GetInstance();

  /**
  * The GPIO no is set as input or output acording to direction
  *
  */
  INPUT_OUTPUT SetAs(PORT_NO no, INPUT_OUTPUT direction, BIT value = 0);

  /**
  * Returns TRUE is he GPIO no is an output
  *
  */
  BOOL         isAOutput(PORT_NO no);

  /**
  * The GPIO no is set high (1) if value is HIGH or low (0) if value is LOW
  *
  */
  BIT          Set(PORT_NO no, BIT value = 1);

  /**
  * The GPIO no is cleared (set to 0)
  *
  */
  BIT          Clear(PORT_NO no);

  /**
  * The GPIO no is toggled
  *
  */
  BIT          Toggle(PORT_NO no);

  /**
  * The value of GPIO no is returned (LOW or HIGH)
  *
  */
  BIT          Read(PORT_NO no);

  /**
  * Returns TRUE if GPIO no is HIGH
  *
  */
  BOOL         isSet(PORT_NO no);

  /**
  * Set LED's to testmode
  *
  */
  void SetLedTest(bool state);


private:
  bool mLedTest;
};

#endif                                  // __cplusplus

#endif                                  // MPCGPio_h

