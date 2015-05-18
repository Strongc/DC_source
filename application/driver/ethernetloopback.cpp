//===============================================================
// Projekt   : MPC
// Enhed     : ETHERNET DRIVER
// Fil       : ethernet.c
// Forfatter : Eske Slemming (ESL)
//
// Beskrivelse:
// driver to ethernet, made for test sw to HW
// Versionshistorie:
//===============================================================

//#include <typedef.h>
#include <cu351_cpu_types.h>
#include <string.h>
#include <ethernetloopback.h>
//#include <mpc_uart.h>

/****************************************************************************
Write to add
****************************************************************************/
void PHYInit(void)
{
    unsigned char dummy_char;

    // WRITE DATA
    init_phy_com(0x00, WRITE);
    write_8bit(0x01);                  // Reg 0  // 10 Mbit
    //write_8bit(0x31);                    // Reg 0  // 100 Mbit - & Auto
    write_8bit(0x00);                    // Reg 0

    init_phy_com(0x04, WRITE);
    write_8bit(0x00);                  // Reg 4  // 10 Mbit
    //write_8bit(0x01);                    // Reg 4  // 100 Mbit
    write_8bit(0x41);                    // Reg 4

    init_phy_com(0x10, WRITE);
    //write_8bit(0x83);                    // Reg 16  // LINK DESABLE - Test
    write_8bit(0x03);                    // Reg 16  // LINK DESABLE - Test
    write_8bit(0x22);                    // Reg 16

    init_phy_com(0x11, WRITE);
    write_8bit(0xFF);                    // read reg 17
    write_8bit(0x10);                    // read reg 17

    init_phy_com(0x13, WRITE);
    write_8bit(0xFF);                    // read reg 19
    write_8bit(0xC0);                    // read reg 19

    init_phy_com(0x00, READ);           // Reg 0
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x01, READ);          // reg 1
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x02, READ);          // reg 2
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x03, READ);          // reg 3
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x04, READ);          // reg 4
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x05, READ);          // reg 5
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x10, READ);           // Reg 16
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x11, READ);          // reg 17
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x12, READ);          // reg 18
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x13, READ);          // reg 19
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    init_phy_com(0x14, READ);          // reg 20
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    if (dummy_char)
    {
	dummy_char++;
    }
    
    (*(unsigned short *)0xB0000308) =  0x3330;  // RETURN CLOCK - Remenber always to return clock after Read/write to PHY
}    // End of Routine


/****************************************************************************
****************************************************************************/
void write_8bit(unsigned char var)
{
   char i;
   for (i=7; i>=0; i--)
   {
        (*(unsigned short *)0xB0000308) =  ((unsigned short)((var >> i) & 0x01) | 0x3338);
        (*(unsigned short *)0xB0000308) =  ((unsigned short)((var >> i) & 0x01) | 0x333C);
   }
}

/****************************************************************************
****************************************************************************/
void write_reg_add(unsigned char var)
{
   char i;
   for (i=4; i>=0; i--)
   {
        (*(unsigned short *)0xB0000308) =  ((unsigned short)((var >> i) & 0x01) | 0x3338);
        (*(unsigned short *)0xB0000308) =  ((unsigned short)((var >> i) & 0x01) | 0x333C);
   }
}

/****************************************************************************
****************************************************************************/
unsigned char read_8bit(void)
{
   char i;
   unsigned short temp;
   temp = 0;
   for (i=7;i>=0;i--)
   {
        (*(unsigned short *)0xB0000308) =  0x3330;
        temp  |= (((*(unsigned short *)0xB0000308 & 0x0002) >>1) << i);
        (*(unsigned short *)0xB0000308) =  0x3334;
    }
    return (unsigned char)temp;
}
/****************************************************************************
****************************************************************************/
void set_phy_read(void)
{
    (*(unsigned short *)0xB0000308) = 0x3339;
    (*(unsigned short *)0xB0000308) = 0x333D;
    (*(unsigned short *)0xB0000308) = 0x3338;
    (*(unsigned short *)0xB0000308) = 0x333C;
}
/****************************************************************************
****************************************************************************/
void set_phy_write(void)
{
    (*(unsigned short *)0xB0000308) = 0x3338;
    (*(unsigned short *)0xB0000308) = 0x333C;
    (*(unsigned short *)0xB0000308) = 0x3339;
    (*(unsigned short *)0xB0000308) = 0x333D;
}

/****************************************************************************
****************************************************************************/
void write_one_on_phy(void)
{
    (*(unsigned short *)0xB0000308) = 0x3339;
    (*(unsigned short *)0xB0000308) = 0x333D;
}

/****************************************************************************
****************************************************************************/
void write_null_on_phy(void)
{
    (*(unsigned short *)0xB0000308) = 0x3338;
    (*(unsigned short *)0xB0000308) = 0x333C;
}
/****************************************************************************
****************************************************************************/
void init_phy_com(unsigned char arg, BOOL status)
{
    int i;
    (*(unsigned short *)0xB000030E) = 0x0003;  // bank 3
    // START MED AT SKRIVE 32 1'er
    for (i=0;i<32;i++)
    {
        write_one_on_phy();
    }
    // Output Start of Frame ('01')
    write_null_on_phy();
    write_one_on_phy();

    // Output OPCode ('01' for write or '10' for Read)
    //set_phy_read();
    if ( status == WRITE )
    {
        set_phy_write();
    }
    else if ( status == READ )
    {
        set_phy_read();
    }
    // Output PHY Address
    write_reg_add(0x00);
    // Output Register Address
    write_reg_add(arg);
   // Implement Turnaround ('10')
    if ( status == WRITE )
    {
        (*(unsigned short *)0xB0000308) = 0x3339;
        (*(unsigned short *)0xB0000308) = 0x333D;
        (*(unsigned short *)0xB0000308) = 0x3338;
        (*(unsigned short *)0xB0000308) = 0x333C;
    }
    else if ( status == READ)
    {
        (*(unsigned short *)0xB0000308) = 0x3330;
        (*(unsigned short *)0xB0000308) = 0x3334;
        (*(unsigned short *)0xB0000308) = 0x3338;
        (*(unsigned short *)0xB0000308) = 0x333C;
    }
}

/****************************************************************************
****************************************************************************/
void MACInit(void)
{
        unsigned char temp_var;
        (*(unsigned short *)0xB000030E) = 0x0000;  // select bank 0
        (*(unsigned short *)0xB0000300) = 0x8801;  // TX is ENABLED .
        (*(unsigned short *)0xB0000304) = 0x0300;  // TJEK PRMS page 47
        temp_var = (*(unsigned short *)0xB0000308);  // Denne værdi skal være 0x0404
        (*(unsigned short *)0xB000030A) = 0x10DC; //- 10 Mbit

        //(*(unsigned short *)0xB000030A) = 0x30DC;  // 100 Mbit

        (*(unsigned short *)0xB000030E) = 0x0001;  // select bank 1
        (*(unsigned short *)0xB0000300) = 0xA4B1;
        (*(unsigned short *)0xB0000304) = 0xAAAA; // MAC ADD
        (*(unsigned short *)0xB0000306) = 0xAAAA; // MAC ADD
        (*(unsigned short *)0xB0000308) = 0xAAAA; // MAC ADD
        (*(unsigned short *)0xB000030C) = 0x12A0;  // 1210
        (*(unsigned short *)0xB000030E) = 0x0003;  // select bank 3
        temp_var =(*(unsigned short *)0xB000030A);
        // MAC ER NU INIT CALL PHY INIT
        (*(unsigned short *)0xB000030E) = 0x0002;  // select bank 2
        (*(unsigned short *)0xB0000300) = 0x0040;
        (*(unsigned short *)0xB000030E) = 0x0003;  // select bank 3

	if (temp_var) {}
}

/****************************************************************************
****************************************************************************/
void writepkt_AAAA(void)
{
    unsigned char dummy_char;
    unsigned short dummy_short;
    int i;
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x0020;  // ALLOCATE MEM FOR TX
    while ( ((*(unsigned short *)0xB000030C)&0x0008) ==  0);
    dummy_char = ((*(unsigned char *)0xB0000303)&0x003F);  // MEM ALLOC PACKAGE NUMBER
    (*(unsigned char *)0xB0000302) = dummy_char;           // Mem package number
    (*(unsigned short *)0xB0000306) = 0x4000;  // AUTO INC.
    (*(unsigned short *)0xB0000308) = 0x0000;  // STATUS WORD
    (*(unsigned short *)0xB0000308) = 0x3E8;   // LENGTH BYTE COUNT

    //************************* DATA AREA *************************************
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD

    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    for (i = 0; i<491; i++)
    {
        (*(unsigned short *)0xB0000308) = 0xAAAA;
    }
    (*(unsigned short *)0xB0000308) = 0x4000;  // CONTROL BYTE
    //************************* DATA AREA END ***********************************
    (*(unsigned short *)0xB000030E) = 0x0001;  // bank 1
    //(*(unsigned short *)0xB000030C) = 0x2000;  // INTERUPT
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x00C0;  // ENQUEUE - "SEND"
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204

    init_phy_com(0x12, READ);           // READ IF LINK ERROR 0x40
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    if (dummy_char) {}
    if (dummy_short) {}

    (*(unsigned short *)0xB0000308) =  0x3330;  // RETURN CLOCK
}

/****************************************************************************
****************************************************************************/
void writepkt_FFFF(void)
{
    unsigned char dummy_char;
    unsigned short dummy_short;
    int i;
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x0020;  // ALLOCATE MEM FOR TX
    while ( ((*(unsigned short *)0xB000030C)&0x0008) ==  0);  // VENTER PÅ READY
    dummy_char = ((*(unsigned char *)0xB0000303)&0x003F);  // MEM ALLOC PACKAGE NUMBER
    (*(unsigned char *)0xB0000302) = dummy_char;           // Mem package number
    (*(unsigned short *)0xB0000306) = 0x4000;  // AUTO INC.
    (*(unsigned short *)0xB0000308) = 0x0000;  // STATUS WORD
    (*(unsigned short *)0xB0000308) = 0x3E8;   // LENGTH BYTE COUNT

    //************************* DATA AREA *************************************
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD

    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    for (i = 0; i<491; i++)
    {
        (*(unsigned short *)0xB0000308) = 0xFFFF;
    }
    (*(unsigned short *)0xB0000308) = 0x4000;  // CONTROL BYTE
    //************************* DATA AREA END ***********************************
    (*(unsigned short *)0xB000030E) = 0x0001;  // bank 1
    //(*(unsigned short *)0xB000030C) = 0x2000;  // INTERUPT
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x00C0;  // ENQUEUE - "SEND"
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204

    init_phy_com(0x12, READ);           // READ IF LINK ERROR 0x40
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    (*(unsigned short *)0xB0000308) =  0x3330;  // RETURN CLOCK

    // avoid compiler warning
    dummy_char = dummy_char;
    dummy_short = dummy_short;
}

/****************************************************************************
****************************************************************************/
void writepkt_BBBB(void)
{
    unsigned char dummy_char;
    unsigned short dummy_short;
    int i;
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x0020;  // ALLOCATE MEM FOR TX
    while ( ((*(unsigned short *)0xB000030C)&0x0008) ==  0);  // VENTER PÅ READY
    dummy_char = ((*(unsigned char *)0xB0000303)&0x003F);  // MEM ALLOC PACKAGE NUMBER
    (*(unsigned char *)0xB0000302) = dummy_char;           // Mem package number
    (*(unsigned short *)0xB0000306) = 0x4000;  // AUTO INC.
    (*(unsigned short *)0xB0000308) = 0x0000;  // STATUS WORD
    (*(unsigned short *)0xB0000308) = 0x3E8;   // LENGTH BYTE COUNT

    //************************* DATA AREA *************************************
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD

    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    for (i = 0; i<491; i++)
    {
        (*(unsigned short *)0xB0000308) = 0xBBBB;
    }
    (*(unsigned short *)0xB0000308) = 0x4000;  // CONTROL BYTE
    //************************* DATA AREA END ***********************************
    (*(unsigned short *)0xB000030E) = 0x0001;  // bank 1
    //(*(unsigned short *)0xB000030C) = 0x2000;  // INTERUPT
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x00C0;  // ENQUEUE - "SEND"
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204

    init_phy_com(0x12, READ);           // READ IF LINK ERROR 0x40
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    (*(unsigned short *)0xB0000308) =  0x3330;  // RETURN CLOCK

    // avoid compiler warning
    dummy_char = dummy_char;
    dummy_short = dummy_short;
}

/****************************************************************************
****************************************************************************/
void writepkt_A5B8(void)
{
    unsigned char dummy_char;
    unsigned short dummy_short;
    int i;
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x0020;  // ALLOCATE MEM FOR TX
    while ( ((*(unsigned short *)0xB000030C)&0x0008) ==  0);  // VENTER PÅ READY
    dummy_char = ((*(unsigned char *)0xB0000303)&0x003F);  // MEM ALLOC PACKAGE NUMBER
    (*(unsigned char *)0xB0000302) = dummy_char;           // Mem package number
    (*(unsigned short *)0xB0000306) = 0x4000;  // AUTO INC.
    (*(unsigned short *)0xB0000308) = 0x0000;  // STATUS WORD
    (*(unsigned short *)0xB0000308) = 0x3E8;   // LENGTH BYTE COUNT

    //************************* DATA AREA *************************************
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // DESTINATION ADD

    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    (*(unsigned short *)0xB0000308) = 0xAAAA;  // SOURCE ADD
    for (i = 0; i<491; i++)
    {
        (*(unsigned short *)0xB0000308) = 0xA5B8;
    }
    (*(unsigned short *)0xB0000308) = 0x4000;  // CONTROL BYTE
    //************************* DATA AREA END ***********************************
    (*(unsigned short *)0xB000030E) = 0x0001;  // bank 1
    //(*(unsigned short *)0xB000030C) = 0x2000;  // INTERUPT
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned short *)0xB0000300) = 0x00C0;  // ENQUEUE - "SEND"
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204

    init_phy_com(0x12, READ);           // READ IF LINK ERROR 0x40
    dummy_char = read_8bit();
    dummy_char = read_8bit();

    (*(unsigned short *)0xB0000308) =  0x3330;  // RETURN CLOCK

    // avoid compiler warning
    dummy_char = dummy_char;
    dummy_short = dummy_short;
}

/****************************************************************************
****************************************************************************/
void flush_TX_buf(void)
{
    unsigned short dummy_short;
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000302);  // CHECK RCR
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned char *)0xB0000302) = 0;           // Mem package number
    if ((dummy_short & 0x0001) == 1)
    {
        (*(unsigned short *)0xB0000300) = 0x00A0;  // ACK. TX INTERUPT
    }
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204
    
    // avoid compiler warning
    dummy_short = dummy_short;
}
/****************************************************************************
****************************************************************************/
unsigned short readpkt_AAAA(void)
{
    unsigned char error_counter = 0;
    int i;

    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned char *)0xB0000302) = 1;           // Mem package number
    (*(unsigned short *)0xB0000306) = 0x6000;  // AUTO INC READ.
    if (((*(unsigned short *)0xB0000308) & 0xFC01) != 0)  // TJEKKER STATUS WORD
        {
        error_counter = error_counter+1;
        }
    if ((*(unsigned short *)0xB0000308) != 0x3E8)  // TJEKKER LENGTH
        {
        error_counter = error_counter+1;
        }
    for (i = 0; i<6; i++)
    {
        if ((*(unsigned short *)0xB0000308) != 0xAAAA)  // TJEKKER ADSENDER OG MODTAGER ADD
        {
            error_counter = error_counter+1;
        }
    }
    for (i = 0; i<491; i++)
    {
        if ((*(unsigned short *)0xB0000308) != 0xAAAA)  // TJEKKER INDHOLD
        {
            error_counter = error_counter+1;
        }
    }

    return error_counter;
 }
/****************************************************************************
****************************************************************************/
unsigned short readpkt_FFFF(void)
{
    unsigned short read_short;
    unsigned char error_counter = 0;
    int i;

    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned char *)0xB0000302) = 1;           // Mem package number
    (*(unsigned short *)0xB0000306) = 0x6000;  // AUTO INC READ.
    read_short = ((*(unsigned short *)0xB0000308));
    //if ((*(unsigned short *)0xB0000308) != 0x00CC)  // TJEKKER STATUS WORD
    if ((read_short & 0xFC01) != 0)  // TJEKKER STATUS WORD
        {
        error_counter = error_counter+1;
        }
    if ((*(unsigned short *)0xB0000308) != 0x3E8)  // TJEKKER LENGTH
        {
        error_counter = error_counter+1;
        }
    for (i = 0; i<6; i++)
    {
        if ((*(unsigned short *)0xB0000308) != 0xAAAA)  // TJEKKER ADSENDER OG MODTAGER ADD
        {
            error_counter = error_counter+1;
        }
    }
    for (i = 0; i<491; i++)
    {
        if ((*(unsigned short *)0xB0000308) != 0xFFFF)  // TJEKKER INDHOLD
        {
            error_counter = error_counter+1;
        }
    }

    return error_counter;
 }

/****************************************************************************
****************************************************************************/
void flush_RX_buf(void)
{
    unsigned short dummy_short;
    (*(unsigned short *)0xB000030E) = 0x0002;  // bank 2
    (*(unsigned char *)0xB0000302) = 1;           // Mem package number
    (*(unsigned short *)0xB0000300) = 0x00A0;  // ACK. TX INTERUPT
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204

    // avoid compiler warning
    dummy_short = dummy_short;
}

/****************************************************************************
****************************************************************************/
unsigned char check_buf(void)
{
    unsigned short return_value;

    unsigned short dummy_short;
    (*(unsigned short *)0xB000030E) = 0x0000;  // bank 0
    dummy_short = (*(unsigned short *)0xB0000308);  // CHECK MEM USE - 0x0204
    if (dummy_short == 0x0404)  // TJEKKER INDHOLD
    {
  //          uart2_PutString("\r0x0404                          ");
            return_value = 4;
    }
    else if (dummy_short == 0x0304)  // TJEKKER INDHOLD
    {
    //        uart2_PutString("\r0x0304                          ");
            return_value = 3;
    }
    else if (dummy_short == 0x0204)  // TJEKKER INDHOLD
    {
      //      uart2_PutString("\r0x0204                          ");
            return_value = 2;
    }
    else if (dummy_short == 0x0104)  // TJEKKER INDHOLD
    {
        //    uart2_PutString("\r0x0104                          ");
            return_value = 1;
    }
    else if (dummy_short == 0x0004)  // TJEKKER INDHOLD
    {
          //  uart2_PutString("\r0x0004                          ");
            return_value = 0;
    }
    return return_value;
}

