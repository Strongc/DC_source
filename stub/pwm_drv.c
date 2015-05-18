#include "pwm_drv.h"
#include "vr4181a.h"
//#include <uart.h>



/************************************************************************
Initialize PWM HW 0
************************************************************************/
void InitializePWMHW_0(void)
{
  /* PWM Channel 0 setup */
  PINMODE_MPC&=0xFFFE;        // G8MODE=0;
  //PINMODE_MPC|=0x0001;        // G8MODE=0;
  GPMODE1_MPC&=0xFFFE;        // GPEN8=0
  //GPMODE1_MPC|=0x0001;        // GPEN8=0
  CMUCLKMSK2_MPC|=0x0001;     // CMUCLKMSK2.MSKPWM01CLK=1;
  PWM0ATSREG_MPC=0x00010;        // Active Level
  PWM0IATSREG_MPC=0x0010;        // Inactive Level
  PWM0ASTCREG_MPC=0x1000;     // Number of inversions before autostop
  PWM0CNTREG_MPC|=0x8000;     // PWM0CLKSEL=1 selects CLK18/4
  PWM0CNTREG_MPC|=0x0008;     // PWM0ACTLEVEL=1 active level high
                              // PWM0_HLB=0 output indication Active level
                              // PWM0ENABLE=0 Stop PWM
//  uart0_PutString("\n\rPWM CHANEL 0 Initialized");
}
/************************************************************************
Initialize PWM HW 1
************************************************************************/
void InitializePWMHW_1(void)
{
  /* PWM Channel 1 setup */
  PINMODE_MPC&=0xFFFD;        // G9MODE=0;
  GPMODE1_MPC&=0xFFFB;        // GPEN9=0
  CMUCLKMSK2_MPC|=0x0001;     // CMUCLKMSK2.MSKPWM01CLK=1;  same for both PWM 0 and 1
  PWM1CTRL_MPC=0x0000;        // PWME=0
#ifdef TFT_16_BIT_LCD
    /* 2.25 kHz of PWM frequency is required.
       See settings below*/
  PWM1CTRL_MPC=0x0031;        // PWME=0
                              // ALV=0
                              // PRM(1:0)=11 , 12bit
                              // PWP(2:0)=001 , CLK_SEL1(9.216 MHz)
#else
	PWM1CTRL_MPC=0x0031;        // PWME=0
		                          // ALV=1
			                        // PRM(1:0)=10 , 10bit
				                      // PWP(2:0)=001 , CLK_SEL3
#endif
  PWM1BUF_MPC=0x0800;         //
//  uart0_PutString("\n\rPWM CHANEL 1 Initialized");
}


/************************************************************************
Initialize PWM HW 2
************************************************************************/
void InitializePWMHW_2(void)
{
  /* PWM Channel 2 setup */
  PINMODED1_MPC&=0xFFFE;      // G10MODE=0;
  GPMODE1_MPC&=0xFFEF;        // GPEN10=0
  CMUCLKMSK1_MPC|=0x0004;     // CMUCLKMSK2.MSKPWM2CLK=1;
  PWM2CTRL_MPC=0x0000;        // PWME=0
  PWM2CTRL_MPC=0x0031;        // PWME=0
                              // ALV=1
                              // PRM(1:0)=10 , 10bit
                              // PWP(2:0)=011 , CLK_SEL3
  PWM2BUF_MPC=0x0800;         //
//  uart0_PutString("\n\rPWM CHANEL 2 Initialized");
}

/************************************************************************
Enable PWM HW 0
************************************************************************/
void EnablePWMHW_0(void)
{
  PWM0CNTREG_MPC|=0x0001;     // PWM0ENABLE=1 Start PWM
//  uart0_PutString("\n\rPWM CHANEL 0 Enabled" );
}

/************************************************************************
Enable PWM HW 1
************************************************************************/
void EnablePWMHW_1(void)
{
  PWM1CTRL_MPC|=0x0080;       // PWME=1
//  uart0_PutString("\n\rPWM CHANEL 1 Enabled" );
}

/************************************************************************
Enable PWM HW 2
************************************************************************/
void EnablePWMHW_2(void)
{
  PWM2CTRL_MPC|=0x0080;       // PWME=1
//  uart0_PutString("\n\rPWM CHANEL 2 Enabled");
}


/************************************************************************
Disable PWM HW 0
************************************************************************/
void DisablePWMHW_0(void)
{
  PWM0CNTREG_MPC&=0xFFFE;     // PWM0ENABLE=0 Stop PWM
//  uart0_PutString("\n\rPWM CHANEL 0 Disabled");
}

/************************************************************************
Disable PWM HW 1
************************************************************************/
void DisablePWMHW_1(void)
{
  PWM1CTRL_MPC&=0xFF7F;       // PWME=0
//  uart0_PutString("\n\rPWM CHANEL 1 Disabled");
}

/************************************************************************
Disable PWM HW 2
************************************************************************/
void DisablePWMHW_2(void)
{
  PWM2CTRL_MPC&=0xFF7F;       // PWME=0
//  uart0_PutString("\n\rPWM CHANEL 2 Disabled");
}

/************************************************************************
Set Active level HW 0
************************************************************************/
void SetDutyCycleHW_0_active_level(int a_active_level)
{
  PWM0ATSREG_MPC = a_active_level;    // Active Level
  //PWM0ATSREG_MPC =0x00010;    // Active Level
  //PWM0IATSREG_MPC=0x00010;    // Inactive Level
//  uart0_PutString("\n\rActive level for PWM CHANEL 0 is SET");
}

/************************************************************************
Set Inactive level HW 0
************************************************************************/
void SetDutyCycleHW_0_inactive_level(int a_inactive_level)
{
    PWM0IATSREG_MPC = a_inactive_level;
  //PWM0ATSREG_MPC =0x00010;    // Active Level
  //PWM0IATSREG_MPC=0x00010;    // Inactive Level
//  uart0_PutString("\n\rInactive level for PWM CHANEL 0 is SET");
}

/************************************************************************
Set DutyCycle HW 1
************************************************************************/
void SetDutyCycleHW_1(unsigned int a_DutyCycle)
{  
#ifdef TFT_16_BIT_LCD
  // a_DutyCycle is to be updated in PWMB(11:0) bits of PWM1BUF_MPC register. However,
  // the exact number of usable bits of PWMB(11:0) is determined by value of bits PRM(1:0)
  // available in PWM1CTRL register.
  unsigned char PRM_Value = (unsigned char)(((PWM1CTRL_MPC) & 0x0030) >> 4);
  unsigned short PWM_Value;
  PWM_Value = (unsigned short)((PRM_Value <= 2) ? (0x1 << (8 + PRM_Value)) :
                                                  (0x1 << (12)));

  if (a_DutyCycle >= PWM_Value)
  {
    a_DutyCycle = (PWM_Value - 1);
  }
#endif
  PWM1BUF_MPC=a_DutyCycle;         //
//  uart0_PutString("\n\rDutyCycle for PWM CHANEL 1 is SET");
}

/************************************************************************
Set DutyCycle HW 2
************************************************************************/
void SetDutyCycleHW_2(unsigned int a_DutyCycle)
{
    PWM2BUF_MPC=a_DutyCycle;         //
//    uart0_PutString("\n\rDutyCycle for PWM CHANEL 2 is SET");
}




