/* This is an epilog required by protected mode microsoft 'C' interrupt
   service routines - see KS_USE_MSPROLOG */
    __asm{
        mov esp,ebp
        pop ebp
        pop ds
        popad
        iretd    /*DM: 8-6-02: Fixed. Changed from ret to iretd */
    }

