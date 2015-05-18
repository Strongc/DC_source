#ifndef PWM_DRV_H
#define PWM_DRV_H
//===============================================================
// Projekt   : MPC
// Enhed     : PWM TEST DRIVER
// Fil       : pwm_drv.H
// Forfatter : Eske Slemming (ESL)
//
// Beskrivelse:
// C test driver til PWM på CU351 processor board
// Versionshistorie:
// Udfyldes sikkert af CVS
//===============================================================

#ifdef __cplusplus
  extern "C" {
#endif

#define USHORT unsigned short

/*      name             typecast            offset + base  // description  */
#define GPMODE1_REG      *(volatile USHORT *)(0x00B302+IOBASE_INT)
#define GPDATA0_REG      *(volatile USHORT *)(0x00B310+IOBASE_INT)

//#define PINMODE_MPC      *(volatile USHORT *)(PINMODE)
#define PINMODE_MPC      *(volatile USHORT *)(0x00B340+IOBASE_INT)
#define PINMODED1_MPC    *(volatile USHORT *)(0x00B382+IOBASE_INT)
//#define GPMODE1_MPC      *(volatile USHORT *)(GPMODE1)
#define GPMODE1_MPC      *(volatile USHORT *)(0x00B302+IOBASE_INT)
//#define CMUCLKMSK2_MPC   *(volatile USHORT *)(CMUCLKMSK2)

#define CMUCLKMSK1_MPC   *(volatile USHORT *)(0x00B012+IOBASE_INT)
#define CMUCLKMSK2_MPC   *(volatile USHORT *)(0x00C108+IOBASE_INT)
#define PWM0ATSREG_MPC   *(volatile USHORT *)(0x00B240+IOBASE_INT)
#define PWM0ATSREG_MPC   *(volatile USHORT *)(0x00B240+IOBASE_INT)
#define PWM0IATSREG_MPC  *(volatile USHORT *)(0x00B242+IOBASE_INT)
#define PWM0ASTCREG_MPC  *(volatile USHORT *)(0x00B24A+IOBASE_INT)
#define PWM0CNTREG_MPC   *(volatile USHORT *)(0x00B248+IOBASE_INT)
#define PWM1CTRL_MPC     *(volatile USHORT *)(0x00C100+IOBASE_INT)
#define PWM2CTRL_MPC     *(volatile USHORT *)(0x00C140+IOBASE_INT)
#define PWM1BUF_MPC      *(volatile USHORT *)(0x00C102+IOBASE_INT)
#define PWM2BUF_MPC      *(volatile USHORT *)(0x00C142+IOBASE_INT)

#define KIUDAT0_MPC      *(volatile USHORT *)(0x00B180+IOBASE_INT)
#define KIUDAT1_MPC      *(volatile USHORT *)(0x00B182+IOBASE_INT)
#define KIUDAT2_MPC      *(volatile USHORT *)(0x00B184+IOBASE_INT)
#define KIUDAT3_MPC      *(volatile USHORT *)(0x00B186+IOBASE_INT)
#define KIUDAT4_MPC      *(volatile USHORT *)(0x00B188+IOBASE_INT)
#define KIUDAT5_MPC      *(volatile USHORT *)(0x00B18A+IOBASE_INT)
#define KIUSCANREP_MPC   *(volatile USHORT *)(0x00B190+IOBASE_INT)
#define KIUSCANS_MPC     *(volatile USHORT *)(0x00B192+IOBASE_INT)
#define KIUWKS_MPC       *(volatile USHORT *)(0x00B194+IOBASE_INT)
#define KIUWKI_MPC       *(volatile USHORT *)(0x00B196+IOBASE_INT)
#define KIUINT_MPC       *(volatile USHORT *)(0x00B198+IOBASE_INT)
#define MKIUINT_MPC      *(volatile USHORT *)(0x00B19A+IOBASE_INT)


void InitializePWMHW_0(void);
void InitializePWMHW_1(void);
void InitializePWMHW_2(void);

void EnablePWMHW_0(void);
void EnablePWMHW_1(void);
void EnablePWMHW_2(void);

void DisablePWMHW_0(void);
void DisablePWMHW_1(void);
void DisablePWMHW_2(void);

void SetDutyCycleHW_0_active_level(int a_active_level);
void SetDutyCycleHW_0_inactive_level(int a_inactive_level);
void SetDutyCycleHW_1(unsigned int a_DutyCycle);
void SetDutyCycleHW_2(unsigned int a_DutyCycle);

#ifdef __cplusplus
  }
#endif


#endif

