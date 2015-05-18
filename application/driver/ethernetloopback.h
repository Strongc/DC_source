// PHY
unsigned char check_buf(void);

void PHYInit(void);
void init_phy_com(unsigned char arg, BOOL status);
void set_phy_read(void);
void set_phy_write(void);
void write_one_on_phy(void);
void write_null_on_phy(void);
// MAC
void MACInit(void);
unsigned char read_8bit(void);
void write_8bit(unsigned char var);
void write_reg_add(unsigned char var);

// Communication
unsigned short readpkt_FFFF(void);
unsigned short readpkt_AAAA(void);
void writepkt_AAAA(void);
void writepkt_FFFF(void);
void writepkt_BBBB(void);
void writepkt_A5B8(void);

void flush_TX_buf(void);
void flush_RX_buf(void);


// ************************ ETHERNET TEST *******************

#define WRITE TRUE
#define READ FALSE
