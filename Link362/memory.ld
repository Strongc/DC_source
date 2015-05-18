MEMORY {
    zero_memory	 : ORIGIN = 0xFFFF8000,	LENGTH = 0

    dram_rsvd1	 : ORIGIN = 0x80000000,	LENGTH = 0
    dram_memory	 : ORIGIN = .,          LENGTH = 32M
    dram_rsvd2   : ORIGIN = ., 	        LENGTH = 0

    flash_rsvd1  : ORIGIN = 0xBE420000, LENGTH = 0
    flash_memory : ORIGIN = .,	        LENGTH = 16M
    flash_rsvd2  : ORIGIN = .,          LENGTH = 0

    disp_mem     : ORIGIN = 0xA1FD94A0, LENGTH = 0x26A60
}
