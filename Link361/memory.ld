MEMORY {
    zero_memory	 : ORIGIN = 0xFFFF8000,	LENGTH = 0

    dram_rsvd1	 : ORIGIN = 0x80000000,	LENGTH = 0
    dram_memory	 : ORIGIN = .,          LENGTH = 16M
    dram_rsvd2   : ORIGIN = ., 	        LENGTH = 0

    flash_rsvd1  : ORIGIN = 0xBF000000, LENGTH = 0
    flash_memory : ORIGIN = .,	        LENGTH = 8M
    flash_rsvd2  : ORIGIN = .,          LENGTH = 0

    disp_mem     : ORIGIN = 0xA0FF6900, LENGTH = 0x9600
}
