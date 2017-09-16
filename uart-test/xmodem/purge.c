// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : purge.c
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
* Description       : Purge function
*
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

unsigned char flush; // dummy value declared as global to eliminate compiler warning

// wait 1 second for sender to empty its transmit buffer
void purge(void)
{
    gl.t1_timed_out = FALSE;

    // 1 second timeout
    // 8MHz / 1024 = 7813 Hz
    // 7813 Hz = 128 us
    // 1 seconds / 128 us = 7813
    // 65536 - 7813 = 57723 = e17b
    // interrupt on ffff to 0000 transition
    TCNT1 = 0xE17B;
    TCCR1B = 0x05; // timer/counter 1 clock / 1024

    while (gl.t1_timed_out != TRUE) // read uart until done
    {
        flush = UDR0;
    }
    TCCR1B = 0x00; // disable timer/counter 1 clock
}
