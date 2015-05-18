
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
// File        : gpoi.cpp for PC Simulation
// By          : FKA
//
// Description : this is the general purpose I/O driver
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// I N C L U D E S
//
#include "rtos.h"
#include "gpio.h"                         /* the header-file for this class */


//////////////////////////////////////////////////////////////////////////////
//
// G P I O  R E S O U R C E  S E M A P H O R E
//

//OS_RSEMA SemaGPIO;

static BIT ports[64];


GPio::GPio()
{
    for (int i=0; i<64; i++) ports[i] = 0;

    //OS_CREATERSEMA(&SemaGPIO);
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
    if (0 <= no && no < sizeof(ports)) ports[no] = value; 
    return DISABLED;    
}

///////////////////////////////////////////////////////////////////////
// Returns TRUE is he GPIO no is an output
//
BOOL GPio::isAOutput(PORT_NO no)
{
    return FALSE; 
}

///////////////////////////////////////////////////////////////////////
// The GPIO no is set high (1) if value is HIGH or low (0) if value is
// LOW
//
BIT GPio::Set(PORT_NO no, BIT value)
{
    //OS_Use(&SemaGPIO);
    if (0 <= no && no < sizeof(ports)) ports[no] = value; 
    //OS_Unuse(&SemaGPIO);
    return value;
}

///////////////////////////////////////////////////////////////////////
// The GPIO no is cleared (set to 0)
//
BIT GPio::Clear(PORT_NO no)
{
    //OS_Use(&SemaGPIO);
    if (0 <= no && no < sizeof(ports)) ports[no] = 0; 
    //OS_Unuse(&SemaGPIO);
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
    BIT result;
    //OS_Use(&SemaGPIO);
    result = 0 <= no && no < sizeof(ports) ? ports[no] : 0;
    //OS_Unuse(&SemaGPIO);
    return result;
}

///////////////////////////////////////////////////////////////////////
//  Returns TRUE if GPIO no is HIGH
//
BOOL GPio::isSet(PORT_NO no)
{
    return Read(no) == 1;
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
    ports[GP_IO_LED_MENU] = 1;
    ports[GP_IO_LED_QUESTION] = 1;
    ports[GP_IO_LED_UP] = 1;
    ports[GP_IO_LED_DOWN] = 1;
    ports[GP_IO_LED_PLUS] = 1;
    ports[GP_IO_LED_MINUS] = 1;
    ports[GP_IO_LED_ESC] = 1;
    ports[GP_IO_LED_OK] = 1;
    ports[GP_IO_LED_HOME] = 1;
    ports[GP_IO_LED_RED] = 1;
    ports[GP_IO_LED_GREEN] = 1;    
  }
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



