
/* circbuff.h - Circular buffer queeing package                         */
/*                                                                      */
/* Copyright 1997, EBSnet Inc.                                          */
/*                                                                      */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */
/* written: 4/97 Peter Van Oudenaren                                    */
/*                                                                      */

#ifndef __CIRCBUF__
#define __CIRCBUF__ 1

/* Circular queuing package.   */
typedef struct _circ_buff
{
    PFBYTE  buff_base;  /* base of buffer */
    PFBYTE  buff_end;   /* wrap point */
    PFBYTE  buff_in;    /* current input pointer puts chars to */
    PFBYTE  buff_out;   /* current output. get chars from */
    int     buff_hw;    /* current output. get chars from */
    int     buff_count;
    int     free_threshhold;    /* For sends signal me when the amount in */
                                /* the output buffer reaaches her   */
    int     buff_size;
} CIRC_BUFF;
typedef struct _circ_buff KS_FAR *PCIRC_BUFF;

/* Use care when calling macros that MODIFY buff_count.            */
/* Use DISABLE & ENABLE if the macro is called outside an isr.     */
/* Or use thread safe cq_enqueSAFE() and cq_dequeueSAFE.           */

/* Count of bytes queued   */
#define cq_count(pq)                (pq)->buff_count
#define cq_room(pq)                 (pq)->buff_size - (pq)->buff_count
#define cq_empty(pq)                (pq->buff_count == 0)
#define cq_full(pq)                 ((pq)->buff_count == (pq)->buff_size)
#define cq_highwater(pq)            ((pq)->buff_count >= (pq)->buff_hw)
#define cq_chk_pointer(pq,X)        if((pq)->X > (pq)->buff_end)    \
                                        (pq)->X = (pq)->buff_base
/* use cq_enque() and cq_dequeue() when interrupts are already disabled */  /* mlc */
#define cq_enque(pq,c)              *((pq)->buff_in++) = c;         \
                                    cq_chk_pointer((pq),buff_in);   \
                                    (pq)->buff_count++
#define cq_dequeue(pq)              *(pq)->buff_out++;              \
                                    cq_chk_pointer((pq),buff_out);  \
                                    (pq)->buff_count--

/* use cq_enqueSAFE() and cq_dequeueSAFE() when interrupts are enabled */  /* mlc */
#define cq_enqueSAFE(pq,c)          *(pq->buff_in++) = c;       \
                                    DISABLE();                  \
                                    cq_chk_pointer(pq,buff_in); \
                                    pq->buff_count++;           \
                                    ENABLE()
#define cq_dequeueSAFE(pq)          *pq->buff_out++;            \
                                    DISABLE();                  \
                                    cq_chk_pointer(pq,buff_out); \
                                    pq->buff_count--;           \
                                    ENABLE()

#define cq_setouttriggerval(pq, n)  (pq)->free_threshhold =         \
                                        (pq)->buff_size - n
#define cq_checkouttrigger(pq)      (pq)->buff_count <= (pq)->free_threshhold
#define cq_clearouttrigger(pq)      (pq)->free_threshhold = -1
#define cq_moveoutptr(pq,wrap,n)    if (wrap) (pq)->buff_out = (pq)->buff_base;\
                                    else (pq)->buff_out += n

/* make sure cq_reducecount() and cq_moveoutptr() are NOT interrupted    */
#define cq_reducecount(pq,NBYTES)    pq->buff_count -= NBYTES
#define cq_incrementcount(pq,NBYTES) pq->buff_count += NBYTES

#define cq_moveinptr(pq,wrap,n)     if (wrap) (pq)->buff_in = (pq)->buff_base;\
                                    else (pq)->buff_in += n
#define cq_outbufftoend(pq)         (((pq)->buff_end - (pq)->buff_out) + 1)
#define cq_inbufftoend(pq)          (int)(((pq)->buff_end - (pq)->buff_in) + 1)
#define cq_outpointer(pq)           (pq)->buff_out
#define cq_inpointer(pq)            (pq)->buff_in
#define cq_size(pq)                 (pq)->buff_size

#define cq_init(pbuffer, buffer_size, pq)\
    tc_memset((PFBYTE)(pq), 0, sizeof(*pq));\
    pq->buff_end =  pbuffer + buffer_size - 1;\
    pq->buff_base = pq->buff_in  =  pq->buff_out =   pbuffer;\
    pq->buff_hw =     buffer_size - 48;\
    pq->buff_size =   buffer_size;\
    pq->buff_count  = pq->free_threshhold = 0;\
    pq->free_threshhold = -1;



#endif      /* __CIRCBUF__ */

