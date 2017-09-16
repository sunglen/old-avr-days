// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : uart.c
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
* Description       : UART
*
****************************************************************************/

#include "xmodem.h"

extern volatile unsigned char buf[133];

extern struct global
{
  volatile unsigned char *recv_ptr;
  volatile unsigned char buffer_status;
  volatile unsigned char recv_error;
  volatile unsigned char t1_timed_out;
} gl;

#pragma vector=USART_RX_vect
__interrupt void UART_RX_interrupt(void)
{
                              // use local pointer until IAR optimizes pointer variables better in the next release
    volatile unsigned char *local_ptr;

    local_ptr = gl.recv_ptr;
                              // check for errors before reading data register ... reading UDR clears status
    if (UCSR0A & 0x18)           // Framing or over run error
    {
      gl.recv_error = TRUE;   // will NAK sender in respond.c
    }                         // always read a character otherwise another interrupt could get generated
                              // read status register before reading data register
    *local_ptr++ = UDR0;       // get char
    switch (buf[0])           // determine if buffer full
    {
        case (SOH) :
            if (local_ptr == (&buf[132] + 1))
            {
                gl.buffer_status = full;
                local_ptr = &buf[0];
            }
        break;
        case (EOT) :
            gl.buffer_status = full;
            local_ptr = &buf[0];
        break;
        default :
            gl.buffer_status = full;    // first char unknown
            local_ptr = &buf[0];
        break;
    }
    gl.recv_ptr = local_ptr;            // restore global pointer
}
