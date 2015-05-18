/* This is a prolog required by protected mode microsoft 'C' interrupt
   service routines - see KS_USE_MSPROLOG */
    /* restore DS   */
    __asm{
        pushad
        push ds
        push ebp
        mov ebp,esp
        sub esp,__LOCAL_SIZE
#if (POLLOSPME)      /*DM: 8-6-02: added this conditional */
        mov ax,0x14  /* pmEasy always uses 0x14 for the app data segment */
        mov ds,ax
#else
        push cs      /* assumes flat memory model CS==DS */
        pop ds
        mov ds,dssave
#endif
    }

