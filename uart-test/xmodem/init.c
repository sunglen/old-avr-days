// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : init.c
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
* Description       : Initiation routine
*
****************************************************************************/

#include "xmodem.h"

void init(void)
{
  TCCR1A = 0x00; // timer/counter 1 PWM disable
  TCCR1B = 0x00; // timer/counter 1 clock disable

  TIMSK1 = 0x01; // enable timer counter 1 interrupt on overflow

  UCSR0B = 0x98; // enable receiver, transmitter, and receiver interrupt

//  UBRR0 = 23; // 19.2k with 7.3728Mhz crystal
//  UBRR0 = 11; // 38.4k with 7.3728Mhz crystal
//  UBRR0 = 7; // 57.6k with 7.3728Mhz crystal
//  UBRR0 = 3; // 115.2k with 7.3728Mhz crystal

//  UBRR0 = 11; // 19.2k with 3.6864Mhz crystal
//  UBRR0 = 7; // 38.4k with 3.6864Mhz crystal
//  UBRR0 = 3; // 57.6k with 3.6864Mhz crystal
//  UBRR0 = 1; // 115.2k with 3.6864Mhz crystal

//  UBRR0 = 51; // 9k6 with 8Mhz clock
//  UBRR0 = 25; // 19k2 with 8Mhz clock
  UBRR0 = 12; // 38k4 with 8Mhz clock
}
