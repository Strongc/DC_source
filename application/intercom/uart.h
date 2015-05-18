#ifndef UART_HPP
#define UART_HPP
//===============================================================
// Projekt   : MPC
// Enhed     : PGIO DRIVER
// Fil       : PGIO.cpp
// Forfatter : Eske Slemming (ESL)
//
// Beskrivelse:
//   C++ driver til PGIO.
// Versionshistorie:
//   19.11.03 / BORN
//   2004-01-10 FKA has got it ...
//===============================================================

//===============================================================

//================================ DEFINES =======================================

#include "typedef.h"
#include "vr4181a.h"

//#define SIU_BASE    *(volatile USHORT *)( 0xC000 +  IOBASE_INT )

#define UART_TO_TEST 0


typedef enum BAUDRATE_
{
    b4800,
    b9600,
    b19200,
    b38400,
    b115200,
    b230400
} BAUDRATE ;

typedef enum DMATransferMethod_
{
    ModeFifo,
    Mode16450
} DMATransferMethod;

#if (UART_TO_TEST == 0)
//-----------------------------------------
// SIU0
// Site 821
//-----------------------------------------
//      name        typecast  offset + base  // description
#define VR4181A_SIURB_0     (*(UCHAR *)(0xC040 + IOBASE_INT))
#define VR4181A_SIUTH_0     (*(UCHAR *)(0xC040 + IOBASE_INT))
#define VR4181A_SIUDLL_0    (*(UCHAR *)(0xC040 + IOBASE_INT))
#define VR4181A_SIUIE_0     (*(UCHAR *)(0xC041 + IOBASE_INT))
#define VR4181A_SIUDLM_0    (*(UCHAR *)(0xC041 + IOBASE_INT))
#define VR4181A_SIUIID_0    (*(UCHAR *)(0xC042 + IOBASE_INT))
#define VR4181A_SIUFC_0     (*(UCHAR *)(0xC042 + IOBASE_INT))
#define VR4181A_SIULC_0     (*(UCHAR *)(0xC043 + IOBASE_INT))
#define VR4181A_SIUMC_0     (*(UCHAR *)(0xC044 + IOBASE_INT))
#define VR4181A_SIULS_0     (*(UCHAR *)(0xC045 + IOBASE_INT))
#define VR4181A_SIUMS_0     (*(UCHAR *)(0xC046 + IOBASE_INT))
#define VR4181A_SIUSC_0     (*(UCHAR *)(0xC047 + IOBASE_INT))
#define VR4181A_SIURESET_0  (*(UCHAR *)(0xC049 + IOBASE_INT))
#define VR4181A_SIUACTMSK_0 (*(UCHAR *)(0xC04C + IOBASE_INT))
#define VR4181A_SIUACTTMR_0 (*(UCHAR *)(0xC04E + IOBASE_INT))

#elif(UART_TO_TEST == 2)
//-----------------------------------------
// Converts uart 2 to 0
//-----------------------------------------
//      name        typecast  offset + base  // description
#define VR4181A_SIURB_0     (*(UCHAR *)(0xC000 + IOBASE_INT))
#define VR4181A_SIUTH_0     (*(UCHAR *)(0xC000 + IOBASE_INT))
#define VR4181A_SIUDLL_0    (*(UCHAR *)(0xC000 + IOBASE_INT))
#define VR4181A_SIUIE_0     (*(UCHAR *)(0xC001 + IOBASE_INT))
#define VR4181A_SIUDLM_0    (*(UCHAR *)(0xC001 + IOBASE_INT))
#define VR4181A_SIUIID_0    (*(UCHAR *)(0xC002 + IOBASE_INT))
#define VR4181A_SIUFC_0     (*(UCHAR *)(0xC002 + IOBASE_INT))
#define VR4181A_SIULC_0     (*(UCHAR *)(0xC003 + IOBASE_INT))
#define VR4181A_SIUMC_0     (*(UCHAR *)(0xC004 + IOBASE_INT))
#define VR4181A_SIULS_0     (*(UCHAR *)(0xC005 + IOBASE_INT))
#define VR4181A_SIUMS_0     (*(UCHAR *)(0xC006 + IOBASE_INT))
#define VR4181A_SIUSC_0     (*(UCHAR *)(0xC007 + IOBASE_INT))
#define VR4181A_SIURESET_0  (*(UCHAR *)(0xC009 + IOBASE_INT))
#define VR4181A_SIUACTMSK_0 (*(UCHAR *)(0xC00C + IOBASE_INT))
#define VR4181A_SIUACTTMR_0 (*(UCHAR *)(0xC00E + IOBASE_INT))

#else
 #error "WRONG UART TO TEST"


#endif

//-----------------------------------------
// SIU1
// Site 843
//-----------------------------------------

#define VR4181A_SIURB_1     *(UCHAR *)(0xC010 + IOBASE_INT)
#define VR4181A_SIUTH_1     *(UCHAR *)(0xC010 + IOBASE_INT)
#define VR4181A_SIUDLL_1    *(UCHAR *)(0xC010 + IOBASE_INT)
#define VR4181A_SIUIE_1     *(UCHAR *)(0xC011 + IOBASE_INT)
#define VR4181A_SIUDLM_1    *(UCHAR *)(0xC011 + IOBASE_INT)
#define VR4181A_SIUIID_1    *(UCHAR *)(0xC012 + IOBASE_INT)
#define VR4181A_SIUFC_1     *(UCHAR *)(0xC012 + IOBASE_INT)
#define VR4181A_SIULC_1     *(UCHAR *)(0xC013 + IOBASE_INT)
#define VR4181A_SIUMC_1     *(UCHAR *)(0xC014 + IOBASE_INT)
#define VR4181A_SIULS_1     *(UCHAR *)(0xC015 + IOBASE_INT)
#define VR4181A_SIUMS_1     *(UCHAR *)(0xC016 + IOBASE_INT)
#define VR4181A_SIUSC_1     *(UCHAR *)(0xC017 + IOBASE_INT)
#define VR4181A_SIURESET_1  *(UCHAR *)(0xC019 + IOBASE_INT)
#define VR4181A_SIUACTMSK_1 *(UCHAR *)(0xC01C + IOBASE_INT)
#define VR4181A_SIUACTTMR_1 *(UCHAR *)(0xC01E + IOBASE_INT)

//-----------------------------------------
// SIU2
// Site 867
//-----------------------------------------
#define VR4181A_SIURB_2     *(UCHAR *)(0xC000 + IOBASE_INT)
#define VR4181A_SIUTH_2     *(UCHAR *)(0xC000 + IOBASE_INT)
#define VR4181A_SIUDLL_2    *(UCHAR *)(0xC000 + IOBASE_INT)
#define VR4181A_SIUIE_2     *(UCHAR *)(0xC001 + IOBASE_INT)
#define VR4181A_SIUDLM_2    *(UCHAR *)(0xC001 + IOBASE_INT)
#define VR4181A_SIUIID_2    *(UCHAR *)(0xC002 + IOBASE_INT)
#define VR4181A_SIUFC_2     *(UCHAR *)(0xC002 + IOBASE_INT)
#define VR4181A_SIULC_2     *(UCHAR *)(0xC003 + IOBASE_INT)
#define VR4181A_SIUMC_2     *(UCHAR *)(0xC004 + IOBASE_INT)
#define VR4181A_SIULS_2     *(UCHAR *)(0x0005 + IOBASE_INT)
#define VR4181A_SIUMS_2     *(UCHAR *)(0xC006 + IOBASE_INT)
#define VR4181A_SIUSC_2     *(UCHAR *)(0xC007 + IOBASE_INT)
#define VR4181A_SIURESET_2  *(UCHAR *)(0xC009 + IOBASE_INT)
#define VR4181A_SIUACTMSK_2 *(UCHAR *)(0xC00C + IOBASE_INT)
#define VR4181A_SIUACTTMR_2 *(UCHAR *)(0xC00E + IOBASE_INT)

// VR4181 DEFINE
#define CMUCLKMSK0_1     *(volatile USHORT *)( 0xB010 + IOBASE_INT )
/****************************************************************************/


/****************************************************************************/

// FUNCTION TO UART 0
        void uart0_setBaudRate(BAUDRATE bps);
        BOOL uart0_isDataInFifo(void);
        void uart0_wait_for_string(char *tmp);
        UCHAR uart0_get_char(void);
        UCHAR uart0_wait_for_char(void);
        void uart0_PutString(char* text);
        void printi(unsigned int b);
        void uart0_init(void);
        void printline(void);
        void print_indtast(void);

// FUNCTION TO UART 1
        void uart1_setBaudRate(BAUDRATE bps);
        void uart1_init(void);
        void uart1_echo(void);
        UCHAR uart1_wait_for_char(void);
        BOOL uart1_isDataInFifo(void);
        UCHAR uart1_get_char(void);

// FUNCTION TO UART 2
        void uart2_init(void);
        void uart2_echo(void);

// TYPE FUNCTION
        int atoi(char* inStr, char** outStr);
        int htoi(char* inStr);

#endif


