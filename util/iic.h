#ifndef IIC_H
#define IIC_H
//===============================================================
// Projekt   : MPC
// Enhed     : IIC TEST DRIVER
// Fil       : IIC.H
// Forfatter : Per Larsen (XPL)
//
// Beskrivelse:
//   C test driver til RTC på CU351 processor board
// Versionshistorie:
//   Udfyldes sikkert af CVS
//===============================================================

#include "vr4181a.h"


//================================ DEFINES =======================================
/*#ifndef IOBASE_INT
  #define IOBASE_INT    0xBFA00000
#endif
*/
#define VR4181A_I2C0    *(volatile unsigned char *)(0xC080 +  IOBASE_INT)
#define VR4181A_I2CC0   *(volatile unsigned char *)(0xC082 +  IOBASE_INT)
#define VR4181A_I2CSVA0 *(volatile unsigned char *)(0xC083 +  IOBASE_INT)
#define VR4181A_I2CCL0  *(volatile unsigned char *)(0xC084 +  IOBASE_INT)
#define VR4181A_I2CX0   *(volatile unsigned char *)(0xC085 +  IOBASE_INT)
#define VR4181A_I2CS0   *(volatile unsigned char *)(0xC086 +  IOBASE_INT)
#define VR4181A_I2CSE0  *(volatile unsigned char *)(0xC087 +  IOBASE_INT)

// Bits within register I2CCx
#define I2C_I2CE 0x80
#define I2C_LREL 0x40
#define I2C_WREL 0x20
#define I2C_SPIE 0x10
#define I2C_WTIM 0x08
#define I2C_ACKE 0x04
#define I2C_STT  0x02
#define I2C_SPT  0x01

// Bits within register I2CSx
#define I2C_MSTS 0x80
#define I2C_ALD  0x40
#define I2C_EXC  0x20
#define I2C_COI  0x10
#define I2C_TRC  0x08
#define I2C_ACKD 0x04
#define I2C_STD  0x02
#define I2C_SPD  0x01

#ifndef VR4181A_PINMODED1
  #define VR4181A_PINMODED1  *(volatile unsigned short *)(0xB382 +  IOBASE_INT)
#endif
#ifndef VR4181A_GPMODE1
  #define VR4181A_GPMODE1  *(volatile unsigned short *)(0xB302 +  IOBASE_INT)
#endif
#ifndef VR4181A_SYSINT1REG
  #define VR4181A_SYSINT1REG *(volatile unsigned short *)(0xB08A +  IOBASE_INT)
#endif


#ifndef I2C0INTR
    #define I2C0INTR    ( 0x0200 )
#endif




#define RTC_ADDR_PHIL 0xA2
#define RTC_ADDR_MAX  0xD0

//=============================== ERROR CODES ====================================
#define I2C_OK 0x00
#define I2C_ACK_ERR 0x01
#define I2C_NOT_TX_MODE_ERR 0x02
#define I2C_NOT_RX_MODE_ERR 0x03



struct rtc_reg
{
  unsigned char seconds;
  unsigned char minutes;
  unsigned char hours;
  unsigned char days;
  unsigned char weekdays;
  unsigned char months;
  unsigned char years;
};


//================================= PROTOTYPES ====================================
void i2c_ctrl_init(void);
char i2c_rtc_init(void);
char i2c_rtc_read(void);
char i2c_rtc_set(void);
char emc_i2c_rtc_read(void);
char emc_i2c_rtc_read_and_write_to_flash(void);

#endif


