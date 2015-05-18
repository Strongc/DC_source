/*****************************************************************************
* Copyright, Grundfos
* All rights reserved.
* No part of this program may be copied, used or delivered to anyone
* without the express written consent of Grundfos *
* MODULENAME.: GPIO driver
*
* PROJECT....: MPC
*
* DESCRIPTION:
* This is the general purpose I/O driver configuration-file
*
*****************************************************************************/

#ifndef MPCGPIO_CFN_H
#define MPCGPIO_CFN_H

// FORWARD REFERENCES
//

#define USE_GPIO_SETUP_TABLE 1

#if USE_GPIO_SETUP_TABLE == 1                       // 1 - use this setup table
                                                    // 0 - don't use this table

typedef  struct _port_setup
{
    PORT_NO PortNo;
    INPUT_OUTPUT Direction;
    unsigned char Value;
} PORT_SETUP;

PORT_SETUP GPIOSetupTable[] =
{

    {GP_IO_LED_MENU,OUTPUT,0},
    {GP_IO_LED_QUESTION,OUTPUT,0},
    {GP_IO_LED_UP,OUTPUT,0},
    {GP_IO_LED_DOWN,OUTPUT,0},
    {GP_IO_LED_PLUS,OUTPUT,0},
    {GP_IO_LED_MINUS,OUTPUT,0},
    {GP_IO_LED_OK,OUTPUT,0},
    {GP_IO_LED_ESC,OUTPUT,0},
    {GP_IO_LED_HOME,OUTPUT,0},
    {GP_IO_LED_RED,OUTPUT,0},
    {GP_IO_LED_GREEN,OUTPUT,0},
#ifndef TFT_16_BIT_LCD
    {DEBUG_LED_0,OUTPUT,1},
    {DEBUG_LED_1,OUTPUT,1},
    {DEBUG_LED_2,OUTPUT,1},
    {DEBUG_LED_3,OUTPUT,1},
    {ETHERNET_LED_A,INPUT,0},
    {ETHERNET_LED_B,INPUT,0},
#endif
    {ETHERNET_RESET,OUTPUT,1},
    {EARLY_WARNING,INPUT,0}
};

#endif                                                  // USE_GPIO_SETUP_TABLE
#endif                                                          // __GPIO_CFN_H



