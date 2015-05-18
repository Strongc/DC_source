#ifndef _analogtv_c
#define _analogtv_c

#define ALEVEL_CFG1 A_CFG(90,80,70,60,30,20,10)
#define ALEVEL_CFG2 A_CFG(90,80,70,60,20,30,10)

const ANALOG_FSW_TV analogtv[] = {

/* HIGH_WATER, HIGH_WATER_A, ALARM, START2, START1, STOP2, STOP1, FSW_DRY_RUN_A, FSW_DRY_RUN */
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,0),0,0,A_D,0,0,1,                        //  0
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,10),0,0,0,0,0,1,                         //  1
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,19),0,0,0,0,0,1,                         //  2
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,20),0,0,0,0,0,0,                         //  3
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,19),0,0,0,0,0,1,                         //  4
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,61),1,0,0,0,0,0,                         //  5
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,71),3,0,0,0,0,0,                         //  6
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,29),1,0,0,0,0,0,                         //  7
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,79),3,0,0,0,0,0,                         //  8
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,80),3,0,0,0,A_A,0,                       //  9
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,81),3,0,0,0,A_A,0,                       // 10
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,89),3,0,0,0,A_A,0,                       // 11
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,90),3,0,0,A_H,A_A,0,                     // 12
  ALEVEL_CFG1, CFG1_AFSW2P, FSW_A(N,N,91),3,0,0,A_H,A_A,0,                     // 13
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,0),0,0,A_D,0,0,1,                        // 14
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,10),0,0,0,0,0,1,                         // 15
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,19),0,0,0,0,0,1,                         // 16
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,20),0,0,0,0,0,0,                         // 17
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,19),0,0,0,0,0,1,                         // 18
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,61),1,0,0,0,0,0,                         // 19
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,71),3,0,0,0,0,0,                         // 20
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,29),1,0,0,0,0,0,                         // 21
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,79),3,0,0,0,0,0,                         // 22
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,80),3,0,0,0,A_A,0,                       // 23
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,81),3,0,0,0,A_A,0,                       // 24
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,89),3,0,0,0,A_A,0,                       // 25
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,90),3,0,0,A_H,A_A,0,                     // 26
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,N,91),3,0,0,A_H,A_A,0,                     // 27
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,A,91),3,0,0,A_H,A_A,0,                     // 28
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,A,89),3,A_I,0,A_H,A_A,0,                   // 29
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,A,91),3,A_I,0,A_H,A_A,0,                   // 30
  ALEVEL_CFG1, CFG2_AFSW2P, FSW_A(N,A,92),3,0,0,A_H,A_A,0,                     // 31
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,N,0),0,0,A_D,0,0,1,                        // 32
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,N,11),0,A_I,A_D,0,0,1,                     // 33
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,10),0,A_I,0,0,0,1,                       // 34
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,11),0,0,0,0,0,1,                         // 35
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,19),0,0,0,0,0,1,                         // 36
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,20),0,0,0,0,0,0,                         // 37
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,19),0,0,0,0,0,1,                         // 38
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,61),1,0,0,0,0,0,                         // 39
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,71),3,0,0,0,0,0,                         // 40
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,29),1,0,0,0,0,0,                         // 41
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,79),3,0,0,0,0,0,                         // 42
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,80),3,0,0,0,A_A,0,                       // 43
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,81),3,0,0,0,A_A,0,                       // 44
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,89),3,0,0,0,A_A,0,                       // 45
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,90),3,0,0,A_H,A_A,0,                     // 46
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,91),3,0,0,A_H,A_A,0,                     // 47
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,N,91),0,A_I,A_D,0,0,1,                     // 48
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,91),3,A_I,0,A_H,A_A,0,                   // 49
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,92),3,0,0,A_H,A_A,0,                     // 50
  ALEVEL_CFG1, CFG3_AFSW2P, FSW_A(N,A,19),0,0,0,0,0,1,                         // 51
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,0),0,0,A_D,0,0,1,                        // 52
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,11),0,0,0,0,0,1,                         // 53
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,18),0,0,0,0,0,1,                         // 54
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,20),0,0,0,0,0,0,                         // 55
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,19),0,0,0,0,0,1,                         // 56
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,61),1,0,0,0,0,0,                         // 57
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,71),3,0,0,0,0,0,                         // 58
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,29),2,0,0,0,0,0,                         // 59
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,79),3,0,0,0,0,0,                         // 60
  ALEVEL_CFG2, CFG1_AFSW2P, FSW_A(N,N,10),0,0,0,0,0,1,                         // 61

};
#endif





