/*                                                                      */
/* base64.c - base64 encode and decode routines for SMTP, POP and WEB   */
/*                                                                      */
/* EBS - RTIP                                                           */
/*                                                                      */
/* Copyright Peter Van Oudenaren , 1993                                 */
/* All rights reserved.                                                 */
/* This code may not be redistributed in source or linkable object form */
/* without the consent of its author.                                   */
/*                                                                      */

#define DIAG_SECTION_KERNEL DIAG_SECTION_MAIL


#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#endif
#include "base64.h"

/* ********************************************************************   */
#if (INCLUDE_POP3 || INCLUDE_SMTP || INCLUDE_WEB || BUILD_BINARY)
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *base64_alphabet;
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR *mime_term_field;
#endif

#if (INCLUDE_SMTP || INCLUDE_WEB)
/* ********************************************************************   */
/* BASE 64 ENCODING                                                       */
/* ********************************************************************   */
void base64_encode_triplet(PBASE64_ENCODE_CONTEXT pContext,PFBYTE dest);

/* ********************************************************************   */
void base64_init_encode_context(PBASE64_ENCODE_CONTEXT pContext, byte line_len)
{
    pContext->bytes_in_triplet = 0;
    pContext->current_line_length = 0;
    pContext->line_len = line_len;
}

/* ********************************************************************   */
/* base64_encode() - encode string using base 64                          */
/*                                                                        */
/*   Does base64 encoding on the string source whose length is specified  */
/*   by lenx.  The result is put in dest.                                 */
/*                                                                        */
/*   Returns: # of bytes added to 'dest'                                  */
/*                                                                        */
int base64_encode(PBASE64_ENCODE_CONTEXT pContext, PFBYTE source, PFBYTE dest, 
                  int lenx)
{
int dest_len = 0; /* length of encoded buffer */
    
    while (lenx > 0)
    {
        while (pContext->bytes_in_triplet<3 && lenx>0)
        {
            pContext->triplet[pContext->bytes_in_triplet++] = *source++;
            lenx--;
        }

        /* if we have a complete triplet, encode it.   */
        if (pContext->bytes_in_triplet == 3)
        {
            base64_encode_triplet(pContext,dest);
            pContext->bytes_in_triplet = 0;
            dest += 4;
            dest_len += 4;
            pContext->current_line_length += 4;
            
            /* if, due to encoding, our current line length has increased     */
            /* to larger than BASE64_LINE_LENGTH, we have to insert the CR/LF */
            if ( (pContext->current_line_length >= pContext->line_len) &&
                 (pContext->line_len != 0) )
            {
                /* should never be more than 4 bytes above pContext->line_len   */
                pContext->current_line_length = 
                    (byte)(pContext->current_line_length - pContext->line_len);
                DEBUG_ASSERT(pContext->current_line_length<4, 
                    "base64_encode: ERROR! pContext->current_line_length =",
                    EBS_INT1, pContext->current_line_length, 0);
                tc_memmove(dest-pContext->current_line_length+2, 
                           dest-pContext->current_line_length,
                           pContext->current_line_length);

                tc_movebytes(dest-pContext->current_line_length, 
                             mime_term_field, 2);
                dest += 2;
                dest_len += 2;

            }
        }
    }       
    return dest_len;
}

/* ********************************************************************   */
/* base64_encode_finish() - finishes encoding string using base 64        */
/*                                                                        */
/*   Finishes base64 encoding on a string previous encoded by             */
/*   base64_encode().  The result is put in dest.                         */
/*                                                                        */
/*   Returns: # of bytes placed in dest (2 or 6)                          */
/*                                                                        */
int base64_encode_finish(PBASE64_ENCODE_CONTEXT pContext, PFBYTE dest)
{
int lenx = 3;

    /* fill the remaining bytes with 0   */
    while (lenx > pContext->bytes_in_triplet)
    {
        pContext->triplet[--lenx] = 0;
    }

    lenx = 0;  /* len is re-used for return value */

    base64_encode_triplet(pContext,dest);
    switch (pContext->bytes_in_triplet)
    {
    case 0:
        break;

    case 1:
        dest[2]='=';
        /* fall through    */

    case 2:
        dest[3] = '=';
        /* fall through    */

    case 3:
        lenx = 4;
        break;

    default:    
        DEBUG_ERROR("base64_encode_finish: invalid bytes_in_triplet =",EBS_INT1,pContext->bytes_in_triplet,0);              
    }

    if (pContext->line_len != 0) 
    {
        tc_movebytes(dest+lenx, mime_term_field, 2); /* add crlf to end of base64 buf. */
        return lenx+2;
    }
    return lenx;
}

/* ********************************************************************   */
/* requires that all 3 bytes be filled.                                   */
void base64_encode_triplet(PBASE64_ENCODE_CONTEXT pContext,PFBYTE dest)
{
PFBYTE source = pContext->triplet;

    dest[0] = base64_alphabet[source[0]>>2];
    dest[1] = base64_alphabet[(((source[0]&0x03)<<4))|(source[1]>>4)];
    dest[2] = base64_alphabet[(((source[1]&0x0F)<<2))|(source[2]>>6)];
    dest[3] = base64_alphabet[(source[2]&0x3F)];
}       
    
#endif          /* INCLUDE_SMTP or INCLUDE_WEB */


#if (INCLUDE_POP3 || INCLUDE_ESMTP)
/* ********************************************************************   */
/* BASE 64 DECODING                                                       */
/* ********************************************************************   */

#define DEBUG_DECODE 0      /* turn on to compile in decode_base64_msg() which */
                            /* is used for debug purposes   */

/* ********************************************************************   */
RTIP_BOOLEAN in_base64_alphabet(char c)
{
    if (c>='A' && c<='Z')
        return TRUE;

    if (c>='a' && c<='z')
        return TRUE;

    if (c>='0' && c<='9')
        return TRUE;

    if (c== '+' || c=='/' || c=='=')
        return TRUE;

    return FALSE;
}

/* ********************************************************************   */
/* There is a quicker way to do this, ut it's ok for now                  */
byte base64_match_char(char input_char)
{
int i;

    for (i=0; i<65; i++)
    {
        if (input_char == base64_alphabet[i])
            return(byte)i;
    }
    DEBUG_ERROR("EEK!  Wasn't in alphabet...",NOVAR,0,0);
    return 1;
}
    
/* ********************************************************************   */
/* returns how many bytes in the input were skipped                       */
int base64_fillquad(PBASE64_DECODE_CONTEXT pContext, PFCHAR input)
{
int retval = 0;
    
    while (*input && pContext->bytes_in_quad<4)
    {
        if (in_base64_alphabet(*input))
        {
            pContext->quad[pContext->bytes_in_quad++]=base64_match_char(*input);
        }
        retval++;
        input++;
    }
    return retval;
}
            
/* ********************************************************************   */
int base64_get_length(int quad_length)
{
    switch(quad_length)
    {
    case 1:
    case 0:
        DEBUG_ERROR("BAD QUAD, first or second char is '='.",NOVAR,0,0);
        return 0;

    case 2:
        return 1;

    case 3:
        return 2;

    default:
        DEBUG_ERROR("Bad parameter passed to base64_get_length: ",EBS_INT1,quad_length,0);
    }
    return 0;
}

/* ********************************************************************   */
int decode_base64_quad(PBASE64_DECODE_CONTEXT pContext, char* buffer)
{
int i,retval=3;

    for (i=0; i<4; i++)
    {
        if (pContext->quad[i] == 64)
        {
            pContext->quad[i] = 0;
            retval = base64_get_length(i);
            break;
        }
    }

    /* fill the rest with zeroes   */
    for (;i<4;i++)  pContext->quad[i] = 0;

    buffer[0] = (char)((pContext->quad[0]<<2) | ((pContext->quad[1] & 0x30) >> 4));
    buffer[1] = (char)((pContext->quad[1]<<4) | ((pContext->quad[2] & 0x3C) >> 2));
    buffer[2] = (char)((pContext->quad[2]<<6) | ((pContext->quad[3] & 0x3F)));

    return(retval);
}

/* ********************************************************************           */
/* xn_decode_base64() - decode string in base 64 format                           */
/*                                                                                */
/* Summary:                                                                       */
/*   #include "rtipapi.h"                                                         */
/*                                                                                */
/*   xn_decode_base64(pContext, buffer, input, input_len)                         */
/*                                                                                */
/*     PBASE64_DECODE_CONTEXT pContext - context which is passed                  */
/*                                       xn_base64_decode_init and                */
/*                                       xn_decode_base64                         */
/*     PFBYTE buffer                   - buffer where decoded results are written */
/*     PFCHAR input                    - input encoded data                       */
/*     int input_length                - length of input buffer                   */
/*                                                                                */
/* Description:                                                                   */
/*   Decodes the string input which is in base 64 encoded format.                 */
/*   The result is written to buffer.                                             */
/*                                                                                */
/*   NOTE: the caller does not have to set any values in pContext                 */
/*                                                                                */
/* Returns:                                                                       */
/*   Returns the length of buffer                                                 */
/*                                                                                */

int xn_decode_base64(PBASE64_DECODE_CONTEXT pContext, PFBYTE buffer, 
                     PFCHAR input, int input_length)
{
char output[4];
int retval = 0;
int lenx;

    /* null-terminate it in case it isn't already...   */
    input[input_length] = 0;

    /* while we still have input...   */
    while (*input)
    {   
        /* First fill out the quad we have... this leaves what was left   */
        /* over from the previous call, if there was anything...          */
        input += base64_fillquad(pContext,input);

        /* if we don't have a complete quad,   */
        if (pContext->bytes_in_quad != 4)
        {
            /* return, and wait for app to call again with more   */
            /* bytes to finish the quad.                          */
            DEBUG_LOG("Unfinished quad.", LEVEL_3, NOVAR, 0, 0);
            break;
        }
        
        /* decode the quad   */
        lenx = decode_base64_quad(pContext, output);

        /* since we got a whole quad, flush it out   */
        pContext->bytes_in_quad = 0;    
        
        /* now append the newly decoded (possibly binary) data on to the   */
        /* end of the buffer.                                              */
        tc_movebytes(buffer+retval, output, lenx);

        /* now we must update the length to reflect this concatenation:   */
        retval += lenx;
    }
    return retval;
}

/* ********************************************************************   */
/* xn_base64_decode_init() - Initializes context for Base 64 decoding     */
/*                                                                        */
/* Summary:                                                               */
/*   #include "rtipapi.h"                                                 */
/*                                                                        */
/*   xn_base64_decode_init(pContext)                                      */
/*                                                                        */
/*     PBASE64_DECODE_CONTEXT pContext - context which is passed          */
/*                                       xn_base64_decode_init and        */
/*                                       xn_decode_base64                 */
/*                                                                        */
/* Description:                                                           */
/*   Initializes context for subsequent calls to xn_decode_base64         */
/*                                                                        */
/*   NOTE: the caller does not have to set any values in pContext         */
/*                                                                        */
/* Returns:                                                               */
/*   Nothing                                                              */
/*                                                                        */
void xn_base64_decode_init(PBASE64_DECODE_CONTEXT pContext)
{
    pContext->bytes_in_quad = 0;
}

#if (INCLUDE_POP3)
/* ********************************************************************   */
void decode_msg(PFCHAR buffer,
                int *offset,
                PBASE64_DECODE_CONTEXT p_base64_context,
                int mime_len, RTIP_BOOLEAN not_first_flag)
{
char nontext_buffer[NONTEXT_BUFFER_SIZE+1];
int dest_off, decode_len;
int curr_off, curr_len;

    /* init context   */
    if (!not_first_flag)
        xn_base64_decode_init(p_base64_context);

    curr_off = 0;
    dest_off = 0;
    while (curr_off < mime_len)
    {
        curr_len = NONTEXT_BUFFER_SIZE;
        if ((curr_off + curr_len) > mime_len)
            curr_len = mime_len - curr_off;

        /* decode mime_field_ptr into nontext_buffer and null-terminate it;   */
        /* p_base64_context keeps track of where we are at in the decoding    */
        decode_len = xn_decode_base64(p_base64_context,(PFBYTE)nontext_buffer,
                                      buffer + curr_off,
                                      curr_len);
        if (decode_len > NONTEXT_BUFFER_SIZE)
        {
            DEBUG_ERROR("OOPS: encoded buffer to big!! :", EBS_INT1, 
                tc_strlen(nontext_buffer),0);
        }

        /* the decoded string is always shorter than the origional   */
        /* so we can copy it back to the same buffer                 */
        tc_movebytes(buffer+dest_off, nontext_buffer, 
                     decode_len);
        dest_off += decode_len;

        curr_off += curr_len;
    }
    *offset = dest_off;

    /* Null terminate  it in case we display it    */
    *(buffer + *offset) = 0;
}

#endif

/* ********************************************************************   */
#if (DEBUG_DECODE)

/* returns length of decoded string   */
int decode_base64_msg(PFCHAR input, int input_len)
{
base64_decode_context base64_context;
PBASE64_DECODE_CONTEXT p_base64_context = &base64_context;
char nontext_buffer[NONTEXT_BUFFER_SIZE+1];
int dest_off, decode_len;
int curr_off, curr_len;

    /* init context   */
    xn_base64_decode_init(p_base64_context);

    curr_off = 0;
    dest_off = 0;
    while (curr_off < input_len)
    {
        curr_len = NONTEXT_BUFFER_SIZE;
        if ((curr_off + curr_len) > input_len)
            curr_len = input_len - curr_off;

        /* decode mime_field_ptr into nontext_buffer and null-terminate it;   */
        /* p_base64_context keeps track of where we are at in the decoding    */
        decode_len = xn_decode_base64(p_base64_context,(PFBYTE)nontext_buffer,
                                      input + curr_off,
                                      curr_len);
        if (decode_len > NONTEXT_BUFFER_SIZE)
        {
            DEBUG_ERROR("OOPS: encoded buffer to big!! :", EBS_INT1, 
                tc_strlen(nontext_buffer),0);
        }

        nontext_buffer[decode_len] = '\0';
        DEBUG_ERROR("DECODED STRING:", STR1, nontext_buffer, 0);

        dest_off += decode_len;

        curr_off += curr_len;
    }
    return(dest_off);
}

#endif      /* DEBUG_DECODE */
#endif      /* INCLUDE_POP3 */


