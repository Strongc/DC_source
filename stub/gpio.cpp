
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                         GRUNDFOS Management A/S                          //
//                           DK-8850 BJERRINGBRO                            //
//                                 DENMARK                                  //
//                                                                          //
//               --------------------------------------------               //
//                                                                          //
//                Segment: MSI                                              //
//                Project: MPC                                              //
//                                                                          //
//               --------------------------------------------               //
//                                                                          //
//               As this is the  property of  GRUNDFOS  it                  //
//               must not be passed on to any person not aut-               //
//               horized  by GRUNDFOS or be  copied or other-               //
//               wise  utilized by anybody without GRUNDFOS'                //
//               expressed written permission.                              //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Project     : MPC
// Unit        : The GPIO driver
// File        : gpoi.cpp
// By          : FKA
//
// Description : this is the general purpose I/O driver
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// I N C L U D E S
//

#include "microp.h"             /*  */
#include "typedef.h"           /* definements for the Grundfos common types */
#include <cu351_cpu_types.h>        /* definements for the MPC common types */
#include "rtos.h"
#include "gpio.h"                         /* the header-file for this class */
#include "gpio_cfn.h"/* the configuration-file for the object of this class */

//////////////////////////////////////////////////////////////////////////////
//
// D E F I N E M E N T S
//

#define REGISTER16(a) *(volatile U16 *)(a)
#define REGISTER32(a) *(volatile U32 *)(a)

//////////////////////////////////////////////////////////////////////////////
//
// I M P L E M E N T A T I O N
//

//////////////////////////////////////////////////////////////////////////////
//
// G P I O  R E S O U R C E  S E M A P H O R E
//

///////////////////////////////////////////////////////////////////////
// The GPIO's are setup in there default position or acording to the
// setup in the GPIOSetupTable if USE_GPIO_SETUP_TABLE == 1 in gpio.cfg
//
GPio::GPio()
{
    int i;

    mLedTest = false;

#if USE_GPIO_SETUP_TABLE == 1
    for (i=0; i < sizeof(GPIOSetupTable)/sizeof(_port_setup); i++)
    {
        SetAs(GPIOSetupTable[i].PortNo,GPIOSetupTable[i].Direction);
        if (GPIOSetupTable[i].Direction == OUTPUT)
        {
            Set(GPIOSetupTable[i].PortNo,GPIOSetupTable[i].Value);
        }
    }
#else
#warning "You have to set up the GPIO's yourself!"
#endif                                                  // USE_GPIO_SETUP_TABLE
}

GPio::~GPio()
{

}


GPio* GPio::mInstance = 0;

///////////////////////////////////////////////////////////////////////
// Returns the instance
//
GPio* GPio::GetInstance()
{
    if (!mInstance)
    {
        mInstance = new GPio();
    }
    return mInstance;
}

///////////////////////////////////////////////////////////////////////
// The GPIO no is set as input or output acording to direction
//
INPUT_OUTPUT GPio::SetAs(PORT_NO no, INPUT_OUTPUT direction, BIT value)
{
    U16 mask;

    switch (direction)
    {
        case INPUT    : mask = 0x0001;
                        break;
        case OUTPUT   : mask = 0x0003;
                        break;
        case DISABLED : mask = 0x0000;
                        break;
        default:        mask = 0x0000;
    }

    if (no >= 0 && no < 8)
    {
        OS_DI();
        REGISTER16(GPMODE0) = REGISTER16(GPMODE0) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE0) = REGISTER16(GPMODE0) | (mask << (no % 8)*2);
        OS_RestoreI();
    }
    else if (no >= 8 && no < 16)
    {
        OS_DI();
        REGISTER16(GPMODE1) = REGISTER16(GPMODE1) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE1) = REGISTER16(GPMODE1) | (mask << (no % 8)*2);
        OS_RestoreI();
    }
    else if (no >= 16 && no < 24)
    {
        OS_DI();
        REGISTER16(GPMODE2) = REGISTER16(GPMODE2) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE2) = REGISTER16(GPMODE2) | (mask << (no % 8)*2);
        OS_RestoreI();
    }
    else if (no >= 24 && no < 32)
    {
        OS_DI();
        REGISTER16(GPMODE3) = REGISTER16(GPMODE3) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE3) = REGISTER16(GPMODE3) | (mask << (no % 8)*2);
        OS_RestoreI();
    }
    else if (no >= 32 && no < 40)
    {
        OS_DI();
        REGISTER16(GPMODE4) = REGISTER16(GPMODE4) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE4) = REGISTER16(GPMODE4) | (mask << (no % 8)*2);
        OS_RestoreI();
    }
    else if (no >= 40 && no < 48)
    {
        OS_DI();
        REGISTER16(GPMODE5) = REGISTER16(GPMODE5) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE5) = REGISTER16(GPMODE5) | (mask << (no % 8)*2);
        OS_RestoreI();
    }
    else if (no >= 48 && no < 56)
    {
        OS_DI();
        REGISTER16(GPMODE6) = REGISTER16(GPMODE6) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE6) = REGISTER16(GPMODE6) | ((mask << (no % 8)*2) & 0x0fff);
        OS_RestoreI();
    }
    else if (no >= 56 && no <= 63)
    {
        OS_DI();
        REGISTER16(GPMODE7) = REGISTER16(GPMODE7) & ~(0x0003 << (no % 8)*2);
        REGISTER16(GPMODE7) = REGISTER16(GPMODE7) | ((mask << (no % 8)*2) & 0xf000);
        OS_RestoreI();
    }
    else
    {
        return direction;
    }

    if ( direction == INPUT)
    {
      if (no >= 0 && no < 16)
      {
          OS_DI();
          REGISTER16(GPINEN0) = REGISTER16(GPINEN0) | (0x0001 << (no % 16));
          OS_RestoreI();
      }
      else if (no >= 16 && no < 32)
      {
          OS_DI();
          REGISTER16(GPINEN1) = REGISTER16(GPINEN1) | (0x0001 << (no % 16));
          OS_RestoreI();
      }
      else if (no >= 32 && no < 48)
      {
          OS_DI();
          REGISTER16(GPINEN2) = REGISTER16(GPINEN2) | (0x0001 << (no % 16));
          OS_RestoreI();
      }
      else if (no >= 48 && no < 54)
      {
          OS_DI();
          REGISTER16(GPINEN3) = REGISTER16(GPINEN3) | (0x0001 << (no % 16));
          OS_RestoreI();
      }
      else
      {
          return direction;
      }
    }

    if (direction == OUTPUT)
    {
        Set(no,value);
    }

    return direction;
}

///////////////////////////////////////////////////////////////////////
// Returns TRUE is he GPIO no is an output
//
BOOL GPio::isAOutput(PORT_NO no)
{
    U16 mask = 0x0001;
    U16 GPModeRegisterValue;

    if (no >= 0 && no < 8)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE0);
        OS_RestoreI();
    }
    else if (no >= 8 && no < 16)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE1);
        OS_RestoreI();
    }
    else if (no >= 16 && no < 24)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE2);
        OS_RestoreI();
    }
    else if (no >= 24 && no < 32)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE3);
        OS_RestoreI();
    }
    else if (no >= 32 && no < 40)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE4);
        OS_RestoreI();
    }
    else if (no >= 40 && no < 48)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE5);
        OS_RestoreI();
    }
    else if (no >= 48 && no < 56)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE6);
        OS_RestoreI();
    }
    else if (no >= 56 && no <= 63)
    {
        OS_DI();
        GPModeRegisterValue = REGISTER16(GPMODE7);
        OS_RestoreI();
    }
    else
        return FALSE;

    GPModeRegisterValue = GPModeRegisterValue & (0x0003 << (no % 8)*2);
    return GPModeRegisterValue == (mask << (no % 8)*2);
}

///////////////////////////////////////////////////////////////////////
// The GPIO no is set high (1) if value is HIGH or low (0) if value is
// LOW
//
BIT GPio::Set(PORT_NO no, BIT value)
{
    U16 GPDataRegisterValue;

    if( mLedTest ) //Do not set LES's in LED test mode
    {
      if( no<=GP_IO_LED_GREEN && no>=GP_IO_LED_MENU)
        return value;
    }

    if (no >= 0 && no < 16)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA0);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA0) = GPDataRegisterValue | (value << (no % 16));
        OS_RestoreI();
    }
    else if (no >= 16 && no < 32)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA1);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA1) = GPDataRegisterValue | (value << (no % 16));
        OS_RestoreI();
    }
    else if (no >= 32 && no < 48)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA2);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA2) = GPDataRegisterValue | (value << (no % 16));
        OS_RestoreI();
    }
    else if (no >= 48 && no < 64)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA3);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA3) = GPDataRegisterValue | (value << (no % 16));
        OS_RestoreI();
    }
    else
        return 0;
   return value;
}

///////////////////////////////////////////////////////////////////////
// Set LED's to testmode and turn the following LED'd on
//  GP_IO_LED_MENU, GP_IO_LED_QUESTION, GP_IO_LED_UP, GP_IO_LED_DOWN,
//  GP_IO_LED_PLUS, GP_IO_LED_MINUS, GP_IO_LED_ESC, GP_IO_LED_OK,
//  GP_IO_LED_HOME, GP_IO_LED_RED, GP_IO_LED_GREEN
//
void GPio::SetLedTest(bool state)
{
  U16 GPDataRegisterValue;

  mLedTest = state;
  if(state)
  {
    OS_DI();
    GPDataRegisterValue = REGISTER16(GPDATA1);
    GPDataRegisterValue = GPDataRegisterValue |= 0xFFE0;
    REGISTER16(GPDATA1) = GPDataRegisterValue;
    OS_RestoreI();
  }
}
///////////////////////////////////////////////////////////////////////
// The GPIO no is cleared (set to 0)
//
BIT GPio::Clear(PORT_NO no)
{
    U16 GPDataRegisterValue;

    if (no >= 0 && no < 16)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA0);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA0) = GPDataRegisterValue;
        OS_RestoreI();
    }
    else if (no >= 16 && no < 32)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA1);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA1) = GPDataRegisterValue;
        OS_RestoreI();
    }
    else if (no >= 32 && no < 48)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA2);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA2) = GPDataRegisterValue;
        OS_RestoreI();
    }
    else if (no >= 48 && no < 64)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA3);
        GPDataRegisterValue = GPDataRegisterValue & ~(0x0001  << (no % 16));
        REGISTER16(GPDATA3) = GPDataRegisterValue;
        OS_RestoreI();
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
//  The GPIO no is toggled
//
BIT GPio::Toggle(PORT_NO no)
{
    BIT value = Read(no);
    Set(no,value == 1 ? 0: 1);
    return value == 1 ? 0: 1;
}

///////////////////////////////////////////////////////////////////////
//  The value of GPIO no is returned (LOW or HIGH)
//
BIT GPio::Read(PORT_NO no)
{
    U16 GPDataRegisterValue;

    if (no >= 0 && no < 16)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA0);
        OS_RestoreI();
    }
    else if (no >= 16 && no < 32)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA1);
        OS_RestoreI();
    }
    else if (no >= 32 && no < 48)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA2);
        OS_RestoreI();
    }
    else if (no >= 48 && no < 64)
    {
        OS_DI();
        GPDataRegisterValue = REGISTER16(GPDATA3);
        OS_RestoreI();
    }
    GPDataRegisterValue = GPDataRegisterValue >> (no % 16);
    return GPDataRegisterValue & 0x0001;
}

///////////////////////////////////////////////////////////////////////
//  Returns TRUE if GPIO no is HIGH
//
BOOL GPio::isSet(PORT_NO no)
{
    return Read(no) == 1;
}

///////////////////////////////////////////////////////////////////////
//  Standard C style functions
//
void C_SetAs(PORT_NO no, INPUT_OUTPUT direction, BIT value)
{
    GPio::GetInstance()->SetAs(no,direction,value);
}

void C_Set(PORT_NO no, BIT value)
{
    GPio::GetInstance()->Set(no,value);
}



