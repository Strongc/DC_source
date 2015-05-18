
#ifndef TCFE574_H
#define TCFE574_H


/* 3COM 3CFE574 Device driver data structure               */
/* Driver data structures/typedefs                         */
/* =================================================       */
typedef struct _TCFE574_softc
{
    PIFACE iface;
    word   ia_iobase;
    word   ia_irq;
    int    socket_number;
    int    ring_buf_index;
    byte   rx_mcast[64];

    struct ether_statistics stats;
}TCFE574_SOFTC;
typedef struct _TCFE574_softc KS_FAR *PTCFE574_SOFTC;


/* 3COM 3CFE574 Device driver statistics                   */
/* held in Bank 6 of the device.                           */
/* =================================================       */
typedef struct _TCFE_stats
{
    byte CarrierLost;       /* CarrierLost          Bank 6    Byte 0   */
    byte SqeError;          /* SqeErrors            Bank 6    Byte 1   */
    byte MultiCollFrames;   /* MultiCollFrames      Bank 6    Byte 2   */
    byte SingleCollFrames;  /* SingleCollFrames     Bank 6    Byte 3   */
    byte LateCollisions;    /* LateCollisions       Bank 6    Byte 4   */
    byte RxOverruns;        /* RxOverruns           Bank 6    Byte 5   */
    byte FramesXmittedOK;   /* FramesXmittedOK      Bank 6    Byte 6   */
    byte FramesRcvdOK;      /* FramesRcvdOK         Bank 6    Byte 7   */
    byte FramesDeffered;    /* FramesDeffered       Bank 6    Byte 8   */
    byte UpperFramesOK;     /* UpperFramesOK        Bank 6    Byte 9   */

    word BytesRcvdOK;       /* BytesRcvdOK          Bank 6    Byte A,B */
    word BytesXmittedOK;    /* BytesXmittedOK       Bank 6    Byte C,D */
}TCFE_STATS;
typedef struct _TCFE_stats KS_FAR *PTCFE_STATS;


/* Useful code macros                                      */
/* =================================================       */
#define TCFE574_OUTDWORD(ADDR, VAL) OUTDWORD((ADDR), (VAL))
#define TCFE574_INDWORD(ADDR)       INDWORD((ADDR))

#define TCFE574_OUTWORD(ADDR, VAL)  OUTWORD((ADDR), (VAL))
#define TCFE574_INWORD(ADDR)        INWORD((ADDR))

#define TCFE574_OUTBYTE(ADDR, VAL)  OUTBYTE((ADDR), (VAL))
#define TCFE574_INBYTE(ADDR)        INBYTE((ADDR))

#define off_to_TCFE574_softc(X)   (X) >= CFG_NUM_TCFE574 ? (PTCFE574_SOFTC)0 : &TCFE574softc[(X)]
#define iface_to_TCFE574_softc(X) (X)->minor_number >= CFG_NUM_TCFE574 ? (PTCFE574_SOFTC)0 : &TCFE574softc[(X)->minor_number]


/* Forward references for the                              */
/* driver interface functions                              */
/* =================================================       */
void         TCFE574_interrupt     (int minor_no);
void         TCFE574_pre_interrupt (int deviceno);
RTIP_BOOLEAN TCFE574_open          (PIFACE pi);
void         TCFE574_close         (PIFACE pi);
int          TCFE574_xmit          (PIFACE pi, DCU msg);
RTIP_BOOLEAN TCFE574_xmit_done     (PIFACE pi, DCU msg, RTIP_BOOLEAN success);
RTIP_BOOLEAN TCFE574_statistics    (PIFACE pi);
RTIP_BOOLEAN TCFE574_setmcast      (PIFACE pi);
void         TCFE574_getmcaf       (PFBYTE mclist, int lenmclist, PFBYTE af);


#endif /* TCFE574_H */
