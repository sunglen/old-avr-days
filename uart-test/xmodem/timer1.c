// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : timer1.c
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
* Description       : Timer function
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

#pragma vector=TIMER1_OVF_vect
__interrupt void TIMER1_OVF_interrupt(void)
{
    gl.t1_timed_out = TRUE;
}

