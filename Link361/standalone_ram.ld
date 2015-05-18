//
// Program layout for running out of RAM.
//
//

SECTIONS
{
    .zdata            ABS : > zero_memory
    .zbss             ABS : > .
    .rozdata          ABS : > .

    .bootsec          ALIGN(8) : > dram_memory
    .stepstone        ABS 0x80000200 : > .
    .text             : > .
    .syscall          : > .
    .secinfo          : > .
    .fixaddr          : > .
    .fixtype          : > .
    .robase           ALIGN(8) : > .
    .rodata           : > .
    .sdabase          ALIGN(8) : > .
    .sdata            : > .
    .sbss             : > .
    .rosdata          : > .
    .data             : > .
    .profile          : > .
    .bss              : > .
    .heap             ALIGN(8) MIN_SIZE(heap_reserve) CLEAR : > .
    .stack            ALIGN(8) MIN_SIZE(stack_reserve)      : > .
    
//
//  Our display
    .frambuf          ALIGN(16) : > disp_mem


//
// These special symbols mark the bounds of RAM and ROM memory.
// They are used by the MULTI debugger.
//
    __ghs_ramstart  = MEMADDR(dram_rsvd1);
    __ghs_ramend    = MEMENDADDR(dram_rsvd2);
    __ghs_romstart  = MEMADDR(flash_rsvd1);
    __ghs_romend    = MEMENDADDR(flash_rsvd2);

}
