// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : respond.c
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
* Description       : Respond function
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


void respond(unsigned char packet)
{
    // clear buffer flag here ... when acking or nacking sender may respond
    // very quickly.
    gl.buffer_status = empty;
    gl.recv_error = FALSE; // framing and over run detection

    if ((packet == good) || (packet == dup) || (packet == end))
    {
        while (!(UCSR0A & 0x20)); // wait till transmit register is empty
        UDR0 = ACK; // now for the next packet
    }
    else
    {
        while (!(UCSR0A & 0x20)); // wait till transmit register is empty
        purge(); // let transmitter empty its buffer
        UDR0 = NAK; // tell sender error
    }
}
