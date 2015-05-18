#ifndef _errortv_c
#define _errortv_c

const FSW_TV errortv[] = {




/* CFG1_5FSW2P FSW_CFG(HIGH_WATER, START2, START1, STOP, DRY_RUN) */
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 //  0
  CFG1_5FSW2P,FSW_5(A,N,N,N,N),FSW5E(A,A,A,A,A),3,A_I,0,A_H,0,0,             //  1
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,A_I,A_D,0,0,1,                               //  2
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,0,0,0,0,1,                                   //  3
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 //  4
  CFG1_5FSW2P,FSW_5(N,A,N,N,N),FSW5E(N,A,A,A,A),3,A_I,0,0,0,0,               //  5
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,A_I,A_D,0,0,1,                               //  6
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,0,0,0,0,1,                                   //  7
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,0,0,0,0,1,                                   //  8
  CFG1_5FSW2P,FSW_5(N,N,A,N,A),FSW5E(N,N,A,A,A),1,A_I,0,0,0,0,               //  9
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,A_I,0,0,0,1,                                 // 10
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 // 11
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,0,0,0,0,1,                                   // 12
  CFG1_5FSW2P,FSW_5(N,A,N,N,A),FSW5E(N,A,A,A,A),3,A_I,0,0,0,0,               // 13
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,A_I,0,0,0,1,                                 // 14
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 // 15
  CFG1_5FSW2P,FSW5(N,A,A,A,A),3,0,0,0,0,0,                                   // 16
  CFG1_5FSW2P,FSW_5(N,A,A,A,N),FSW5E(N,N,N,N,N),0,A_I,A_D,0,0,1,             // 17
  CFG1_5FSW2P,FSW5(N,A,A,A,A),3,A_I,0,0,0,0,                                 // 18
  CFG1_5FSW2P,FSW5(N,N,A,A,A),3,0,0,0,0,0,                                   // 19
  CFG1_5FSW2P,FSW5(N,A,A,A,A),3,0,0,0,0,0,                                   // 20
  CFG1_5FSW2P,FSW_5(N,A,N,A,A),FSW5E(N,N,N,A,A),3,A_I,0,0,0,0,               // 21
  CFG1_5FSW2P,FSW_5(N,A,N,N,A),FSW5E(N,N,N,N,A),0,A_I,0,0,0,1,               // 22
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,A_I,0,0,0,1,                                 // 23
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 // 24
  CFG1_5FSW2P,FSW_5(A,N,A,A,A),FSW5E(A,A,A,A,A),3,A_I,0,A_H,0,0,             // 25
  CFG1_5FSW2P,FSW_5(A,N,A,A,A),FSW5E(A,A,A,A,A),3,A_I,0,A_H,0,0,             // 26
  CFG1_5FSW2P,FSW_5(A,N,N,A,A),FSW5E(A,A,A,A,A),3,A_I,0,A_H,0,0,             // 27
  CFG1_5FSW2P,FSW_5(A,N,N,N,A),FSW5E(A,A,A,A,A),3,A_I,0,A_H,0,0,             // 28
  CFG1_5FSW2P,FSW_5(A,N,N,N,N),FSW5E(A,A,A,A,A),3,A_I,0,A_H,0,0,             // 29
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,A_I,A_D,0,0,1,                               // 30
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,0,0,0,0,1,                                   // 31
  CFG1_5FSW2P,FSW5(N,N,N,A,A),0,0,0,0,0,0,                                   // 32
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 // 33
  CFG1_5FSW2P,FSW_5(N,N,A,N,N),FSW5E(N,N,A,A,A),1,A_I,0,0,0,0,               // 34
  CFG1_5FSW2P,FSW_5(N,N,A,N,A),FSW5E(N,N,A,A,A),1,A_I,0,0,0,0,               // 35
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,A_I,A_D,0,0,1,                               // 36
  CFG1_5FSW2P,FSW_5(N,N,A,N,N),FSW5E(N,N,A,A,A),1,A_I,0,0,0,0,               // 37
  CFG1_5FSW2P,FSW_5(N,N,A,A,N),FSW5E(N,N,A,A,A),1,A_I,0,0,0,0,               // 38
  CFG1_5FSW2P,FSW_5(N,N,N,A,N),FSW5E(N,N,N,N,N),0,A_I,A_D,0,0,1,             // 39
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,A_I,A_D,0,0,1,                               // 40     The final 3 steps are used to clear the inconsistency
  CFG1_5FSW2P,FSW5(N,N,N,N,A),0,0,0,0,0,1,                                   // 41     to prevent it to ripple through to the next testcase
  CFG1_5FSW2P,FSW5(N,N,N,N,N),0,0,A_D,0,0,1,                                 // 42     The system is left in OK state with no switches activated.
};
#endif
