// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : sendc.c
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
* Description       : Sends the C char
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

void sendc(void) {

    // 3 second timeout
    // 8MHz / 1024 = 7813 Hz
    // 7813 Hz = 128 us
    // 3 seconds / 128 us = 23438
    // 65536 - 23438 = 42098 = a472
    // interrupt on ffff to 0000 transition
    TCNT1 = 0xA472;
    TCCR1B = 0x00; // disable timer/counter 1 clock

    // enable entry into while loops
    gl.buffer_status = empty;
    gl.t1_timed_out = FALSE;
    gl.recv_error = FALSE; // checked in validate_packet for framing or overruns

    // send character 'C' until we get a packet from the sender
    while (!gl.buffer_status)
    {
        // tell sender CRC mode
        while (!(UCSR0A & 0x20)); // wait till Data register is empty
        UDR0 = CRCCHR; // signal transmitter that I'm ready in CRC mode ... 128 byte packets
        TCCR1B = 0x05; // timer/counter 1 clock / 1024

        // wait for timeout or recv buffer to fill
        while (!gl.t1_timed_out && !gl.buffer_status);

                                    // turn off timer
        TCCR1B = 0x00;              // disable timer/counter 1 clock

        if (gl.t1_timed_out)                  // start wait loop again
        {
            gl.t1_timed_out = FALSE;
            TCNT1 = 0xA472;           // load counter ... start over
        }
    }
}
