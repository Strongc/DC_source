/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:    GENIpro                                       */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      :   hw_res.h                                            */
/*                                                                          */
/* FILE NAME        :   hw_res.h                                            */
/*                                                                          */
/* FILE DESCRIPTION :   processsor specific defines for D70311x             */
/*                                                                          */
/****************************************************************************/
#ifndef _HW_RES_H
#define _HW_RES_H
#include "MicroP.h"


// Baud rate value - see page 824 in users manual
#define  DIVISOR_VAL_1200       960
#define  DIVISOR_VAL_2400       480
#define  DIVISOR_VAL_4800       240
#define  DIVISOR_VAL_9600       120
#define  DIVISOR_VAL_19200       60
#define  DIVISOR_VAL_38400       30

// IO pins
#define  GENI_GPIO15              1
#define  GENI_GPIO16              2
#define  GENI_GPIO17              3
#define  GENI_GPIO22              4
//TCSL -  Mapping changes for supporting TFT display
#ifndef TFT_16_BIT_LCD
#define  GENI_GPIO46              5
#define  GENI_GPIO47              6
#else
#define  GENI_GPIO52              5
#define  GENI_GPIO53              6
#endif
#define AS_BIT_INPUT              1
#define AS_BIT_OUTPUT             2

// interrupt registers
#define GENI_SYSINT0REG    (*(volatile UINT *)(SYSINT0REG ))
#define GENI_MSYSINT0REG   (*(volatile UINT *)(MSYSINT0REG))

// Real Time Clock Unit (RTC) registers
// This section should be placed in vr4181a.h
#ifndef ETIMELREG     // assumes that if one is defined all are defined somewhere else.
  #define ETIMELREG                 (0x00B0C0 + IOBASE_INT)
  #define ETIMEMREG                 (0x00B0C2 + IOBASE_INT)
  #define ETIMEHREG                 (0x00B0C4 + IOBASE_INT)
  #define ECMPLREG                  (0x00B0C8 + IOBASE_INT)
  #define ECMPMREG                  (0x00B0CA + IOBASE_INT)
  #define ECMPHREG                  (0x00B0CC + IOBASE_INT)
  #define RTCL1LREG                 (0x00B0D0 + IOBASE_INT)
  #define RTCL1HREG                 (0x00B0D2 + IOBASE_INT)
  #define RTCL1CNTLREG              (0x00B0D4 + IOBASE_INT)
  #define RTCL1CNTHREG              (0x00B0D6 + IOBASE_INT)
  #define RTCL2LREG                 (0x00B0D8 + IOBASE_INT)
  #define RTCL2HREG                 (0x00B0DA + IOBASE_INT)
  #define RTCL2CNTLREG              (0x00B0DC + IOBASE_INT)
  #define RTCL2CNTHREG              (0x00B0DE + IOBASE_INT)
  #define RTCINTREG                 (0x00B1DE + IOBASE_INT)
#endif

// section ends

// Count value
#define GENI_RTC_CNT_VALUE        17 // = 17 * 1/(32768 Hz) = 519 ms
#define GENI_RTC_FREQ             32768 / GENI_RTC_CNT_VALUE

//Interrupt register
#define GENI_RTCINTREG            (*(volatile UINT *)(RTCINTREG ))

// RTC1
#define GENI_RTCL1LREG            (*(volatile UINT *)(RTCL1LREG ))
#define GENI_RTCL1HREG            (*(volatile UINT *)(RTCL1HREG ))
#define GENI_RTCL1CNTLREG         (*(volatile UINT *)(RTCL1CNTLREG ))
#define GENI_RTCL1CNTHREG         (*(volatile UINT *)(RTCL1CNTHREG ))
// RTC2
#define GENI_RTCL2LREG            (*(volatile UINT *)(RTCL2LREG ))
#define GENI_RTCL2HREG            (*(volatile UINT *)(RTCL2HREG ))
#define GENI_RTCL2CNTLREG         (*(volatile UINT *)(RTCL2CNTLREG ))
#define GENI_RTCL2CNTHREG         (*(volatile UINT *)(RTCL2CNTHREG ))


// GPIO register setup
#define GENI_GPMODE1       (*(volatile UINT *)(GPMODE1)    )
#define GENI_GPMODE2       (*(volatile UINT *)(GPMODE2)    )
#define GENI_GPMODE5       (*(volatile UINT *)(GPMODE5)    )
#define GENI_GPDATA1       (*(volatile UINT *)(GPDATA1)    )
#define GENI_GPDATA0       (*(volatile UINT *)(GPDATA0)    )
#define GENI_GPINTTYP5     (*(volatile UINT *)(GPINTTYP5)  )
#define GENI_GPINEN2       (*(volatile UINT *)(GPINEN2)    )
#define GENI_GPINTSTAT2    (*(volatile UINT *)(GPINTSTAT2) )
#define GENI_GPINTMSK2     (*(volatile UINT *)(GPINTMSK2)  )
#define GENI_PINMODED1     (*(volatile UINT *)(PINMODED1)  )
#define GENI_PINMODED2     (*(volatile UINT *)(PINMODED2)  )


#ifdef TFT_16_BIT_LCD
#define GENI_GPMODE6       (*(volatile UINT *)(GPMODE6)    )
#define GENI_GPINTTYP6     (*(volatile UINT *)(GPINTTYP6)  )
#define GENI_GPINTSTAT3    (*(volatile UINT *)(GPINTSTAT3) )
#define GENI_GPINEN3       (*(volatile UINT *)(GPINEN3)    )
#define GENI_GPINTMSK3     (*(volatile UINT *)(GPINTMSK3)  )
#endif
#if 1 //def USB_ENABLE
#define GENI_USBSIGCTRL      (*(volatile UINT *)(USBSIGCTRL)    )
#define GENI_CMUCLKMSK1      (*(volatile UINT *)(CMUCLKMSK1)  )
#define GENI_CMUCLKMSK3      (*(volatile ULONG *)(CMUCLKMSK3) )
#define GENI_MSYSINT1REG     (*(volatile UINT *)(MSYSINT1REG) )
#define GENI_SYSINT1REG      (*(volatile UINT *)(SYSINT1REG ))
#define GENI_INTASSIGN3REG   (*(volatile UINT *)(INTASSIGN3REG))
#define GENI_SYSINT2REG   	 (*(volatile UINT *)(SYSINT2REG))
#define GENI_SYSINT3REG   	 (*(volatile UINT *)(SYSINT3REG))
#define GENI_MSYSINT2REG     (*(volatile UINT *)(MSYSINT2REG) )
#define GENI_MSYSINT3REG     (*(volatile UINT *)(MSYSINT3REG) )
#define GENI_PCIW0      (*(volatile ULONG *)(PCIW0) )
#define GENI_PCIW1      (*(volatile ULONG *)(PCIW1) )
#define GENI_IOPCIU_PCIINIT01      (*(volatile ULONG *)(IOPCIU_PCIINIT01) )
#define GENI_IOPCIU_BAR_SDRAM      (*(volatile ULONG *)(IOPCIU_BAR_SDRAM) )
#define GENI_IOPCIU_BAR_INTCS      (*(volatile ULONG *)(IOPCIU_BAR_INTCS) )
#define GENI_IOPCIU_BAR_ISAW       (*(volatile ULONG *)(IOPCIU_BAR_ISAW) )
#define GENI_IOPCIU_BAR_PCS2       (*(volatile ULONG *)(IOPCIU_BAR_PCS2) )
#define GENI_IOPCIU_BAR_PCS1       (*(volatile ULONG *)(IOPCIU_BAR_PCS1) )
#define GENI_IOPCIU_BAR_PCS0       (*(volatile ULONG *)(IOPCIU_BAR_PCS0) )
#define GENI_IOPCIU_BAR_ROMCS      (*(volatile ULONG *)(IOPCIU_BAR_ROMCS) )
#define GENI_IOPCIU_PCICMD         (*(volatile UINT *)(IOPCIU_PCICMD) )
#define GENI_IOPCIU_PCISTS         (*(volatile UINT *)(IOPCIU_PCISTS) )
#define GENI_IOPCIU_PCIERR         (*(volatile ULONG *)(IOPCIU_PCIERR) )
#define GENI_IOPCIU_PCICTRL_L      (*(volatile ULONG *)(IOPCIU_PCICTRL_L) )
#define GENI_IOPCIU_PCICTRL_H      (*(volatile ULONG *)(IOPCIU_PCICTRL_H) )
#define GENI_IOPCIU_MLTIM      	   (*(volatile UCHAR *)(IOPCIU_MLTIM) )
#define GENI_IOPCIU_INTP      	   (*(volatile UCHAR *)(IOPCIU_INTP) )
#endif

// UART register setup
#define GENI_SIU_RB_0      (*(UCHAR *)(SIU_RB_0     + SIU_BASE))
#define GENI_SIU_TH_0      (*(UCHAR *)(SIU_TH_0     + SIU_BASE))
#define GENI_SIU_DLL_0     (*(UCHAR *)(SIU_DLL_0    + SIU_BASE))
#define GENI_SIU_IE_0      (*(UCHAR *)(SIU_IE_0     + SIU_BASE))
#define GENI_SIU_DLM_0     (*(UCHAR *)(SIU_DLM_0    + SIU_BASE))
#define GENI_SIU_IID_0     (*(UCHAR *)(SIU_IID_0    + SIU_BASE))
#define GENI_SIU_FC_0      (*(UCHAR *)(SIU_FC_0     + SIU_BASE))
#define GENI_SIU_LC_0      (*(UCHAR *)(SIU_LC_0     + SIU_BASE))
#define GENI_SIU_MC_0      (*(UCHAR *)(SIU_MC_0     + SIU_BASE))
#define GENI_SIU_LS_0      (*(UCHAR *)(SIU_LS_0     + SIU_BASE))
#define GENI_SIU_MS_0      (*(UCHAR *)(SIU_MS_0     + SIU_BASE))
#define GENI_SIU_SC_0      (*(UCHAR *)(SIU_SC_0     + SIU_BASE))
#define GENI_SIU_RESET_0   (*(UCHAR *)(SIU_RESET_0  + SIU_BASE))
#define GENI_SIU_ACTMSK_0  (*(UCHAR *)(SIU_ACTMSK_0 + SIU_BASE))
#define GENI_SIU_ACTTMR_0  (*(UCHAR *)(SIU_ACTTMR_0 + SIU_BASE))

#define GENI_SIU_RB_1      (*(UCHAR *)(SIU_RB_1     + SIU_BASE))
#define GENI_SIU_TH_1      (*(UCHAR *)(SIU_TH_1     + SIU_BASE))
#define GENI_SIU_DLL_1     (*(UCHAR *)(SIU_DLL_1    + SIU_BASE))
#define GENI_SIU_IE_1      (*(UCHAR *)(SIU_IE_1     + SIU_BASE))
#define GENI_SIU_DLM_1     (*(UCHAR *)(SIU_DLM_1    + SIU_BASE))
#define GENI_SIU_IID_1     (*(UCHAR *)(SIU_IID_1    + SIU_BASE))
#define GENI_SIU_FC_1      (*(UCHAR *)(SIU_FC_1     + SIU_BASE))
#define GENI_SIU_LC_1      (*(UCHAR *)(SIU_LC_1     + SIU_BASE))
#define GENI_SIU_MC_1      (*(UCHAR *)(SIU_MC_1     + SIU_BASE))
#define GENI_SIU_LS_1      (*(UCHAR *)(SIU_LS_1     + SIU_BASE))
#define GENI_SIU_MS_1      (*(UCHAR *)(SIU_MS_1     + SIU_BASE))
#define GENI_SIU_SC_1      (*(UCHAR *)(SIU_SC_1     + SIU_BASE))
#define GENI_SIU_RESET_1   (*(UCHAR *)(SIU_RESET_1  + SIU_BASE))
#define GENI_SIU_ACTMSK_1  (*(UCHAR *)(SIU_ACTMSK_1 + SIU_BASE))
#define GENI_SIU_ACTTMR_1  (*(UCHAR *)(SIU_ACTTMR_1 + SIU_BASE))

#define GENI_SIU_RB_2      (*(UCHAR *)(SIU_RB_2     + SIU_BASE))
#define GENI_SIU_TH_2      (*(UCHAR *)(SIU_TH_2     + SIU_BASE))
#define GENI_SIU_DLL_2     (*(UCHAR *)(SIU_DLL_2    + SIU_BASE))
#define GENI_SIU_IE_2      (*(UCHAR *)(SIU_IE_2     + SIU_BASE))
#define GENI_SIU_DLM_2     (*(UCHAR *)(SIU_DLM_2    + SIU_BASE))
#define GENI_SIU_IID_2     (*(UCHAR *)(SIU_IID_2    + SIU_BASE))
#define GENI_SIU_FC_2      (*(UCHAR *)(SIU_FC_2     + SIU_BASE))
#define GENI_SIU_LC_2      (*(UCHAR *)(SIU_LC_2     + SIU_BASE))
#define GENI_SIU_MC_2      (*(UCHAR *)(SIU_MC_2     + SIU_BASE))
#define GENI_SIU_LS_2      (*(UCHAR *)(SIU_LS_2     + SIU_BASE))
#define GENI_SIU_MS_2      (*(UCHAR *)(SIU_MS_2     + SIU_BASE))
#define GENI_SIU_SC_2      (*(UCHAR *)(SIU_SC_2     + SIU_BASE))
#define GENI_SIU_RESET_2   (*(UCHAR *)(SIU_RESET_2  + SIU_BASE))
#define GENI_SIU_ACTMSK_2  (*(UCHAR *)(SIU_ACTMSK_2 + SIU_BASE))
#define GENI_SIU_ACTTMR_2  (*(UCHAR *)(SIU_ACTTMR_2 + SIU_BASE))

#define FRAME_SETUP_CH0(stop_bits, parity, char_len)                                                    \
{                                                                                                       \
 GENI_SIU_LC_0  = 0x0000;                                                                               \
 GENI_SIU_LC_0 |=                          0 << 5;  /* parity is not fixed */                                  \
 GENI_SIU_LC_0 |=        ((parity) & 0x0001) << 4;  /* lsb of parity - even or odd parity*/                    \
 GENI_SIU_LC_0 |= (((parity) & 0x0002) >> 1) << 3;  /* msb of parity - enable / disable parity*/               \
 GENI_SIU_LC_0 |=                  stop_bits << 2;  /* 1 or 2 stop bits */                                     \
 GENI_SIU_LC_0 |=                          1 << 1;  /* 5 or 6 bits data  - not supported*/                     \
 GENI_SIU_LC_0 |=                   char_len << 0;  /* 7 or 8 bits data */                                     \
}

#define FRAME_SETUP_CH1(stop_bits, parity, char_len)                                                    \
{                                                                                                       \
 GENI_SIU_LC_1  = 0x0000;                                                                               \
 GENI_SIU_LC_1|=                          0 << 5;  /* parity is not fixed */                                  \
 GENI_SIU_LC_1 |=        ((parity) & 0x0001) << 4;  /* lsb of parity - even or odd parity*/                    \
 GENI_SIU_LC_1 |= (((parity) & 0x0002) >> 1) << 3;  /* msb of parity - enable / disable parity*/               \
 GENI_SIU_LC_1 |=                  stop_bits << 2;  /* 1 or 2 stop bits */                                     \
 GENI_SIU_LC_1 |=                          1 << 1;  /* 5 or 6 bits data  - not supported*/                     \
 GENI_SIU_LC_1 |=                   char_len << 0;  /* 7 or 8 bits data */                                     \
}

#define FRAME_SETUP_CH2(stop_bits, parity, char_len)                                                    \
{                                                                                                       \
 GENI_SIU_LC_2  = 0x0000;                                                                               \
 GENI_SIU_LC_2 |=                          0 << 5;  /* parity is not fixed */                                  \
 GENI_SIU_LC_2 |=        ((parity) & 0x0001) << 4;  /* lsb of parity - even or odd parity*/                    \
 GENI_SIU_LC_2 |= (((parity) & 0x0002) >> 1) << 3;  /* msb of parity - enable / disable parity*/               \
 GENI_SIU_LC_2 |=                  stop_bits << 2;  /* 1 or 2 stop bits */                                     \
 GENI_SIU_LC_2 |=                          1 << 1;  /* 5 or 6 bits data  - not supported*/                     \
 GENI_SIU_LC_2 |=                   char_len << 0;  /* 7 or 8 bits data */                                     \
}


#endif

