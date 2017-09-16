// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : recv_wait.c
* Compiler          : IAR EWAAVR 4.11a
* Revision          : $Revision: 2.0 $
* Date              : $Date: 23. august 2005 15:57:40 $
* Updated by        : $Author: jtyssoe $
*
* Support mail      : avr@atmel.com
*
* Supported devices : ATmega48/88/168
*
* AppNote           : AVR350: XmodemCRC Receive Utility for AVR
*                     Migrated to mega48 and corrected several bugs in original AT90S8515 code
*
* Description       : Receive wait
*
****************************************************************************/


#include "xmodem.h"

extern struct global
{
  volatile unsigned char *recv_ptr;
  volatile unsigned char buffer_status;
  volatile unsigned char recv_error;
  volatile unsigned char t1_timed_out;
} gl;


void recv_wait(void)
{
    gl.t1_timed_out = FALSE;                    // set in timer counter 0 overflow interrupt routine

    // 1 second timeout
    // 8MHz / 1024 = 7813 Hz
    // 7813 Hz = 128 us
    // 1 seconds / 128 us = 7813
    // 65536 - 7813 = 57723 = e17b
    // interrupt on ffff to 0000 transition
    TCNT1 = 0xE17B;
    TCCR1B = 0x05;                            // timer/counter 1 clock / 1024

                                            // wait for packet, error, or timeout
    while (!gl.buffer_status && !gl.t1_timed_out);
                                            // turn off timer - no more time outs needed
    TCCR1B = 0x00;                          // disable timer/counter 1 clock

}
