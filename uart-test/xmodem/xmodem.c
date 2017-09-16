// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : xmodem.c
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
* Description       : Xmodem main routine
*
****************************************************************************/

#include "xmodem.h"

volatile unsigned char buf[133];

struct global
{
  volatile unsigned char *recv_ptr;
  volatile unsigned char buffer_status;
  volatile unsigned char recv_error;
  volatile unsigned char t1_timed_out;
} gl;


__C_task void main(void)
{
  CLKPR = (1 << CLKPCE);  // enable prescaler update
  CLKPR = 0;              // set maximum clock frequency

  // PortD bit 2 - pushbutton used to start data reception
  // PortD bit 3 - pushbutton used for simulating errors - see validate_packet.c
  // Pin:        PD7, PD6, PD5, PD4, PD3, PD2, PD1, PD0
  // Direction:  O    O    O    O    I    I    O    O
  // DDR bit:    1    1    1    1    0    0    1    1
  DDRD = 0xf3;
  PORTD = 0xff;

  init();     // low level hardware initialization
  __enable_interrupt();     /* enable interrupts */

  purge();   // clear uart data register ... allow transmitter opportunity to unload its buffer
  for(;;)
  {
    while (PIND &= 0x04);   // wait until pd2 pulled low

    receive(&buf[0]);
  }
} // main
