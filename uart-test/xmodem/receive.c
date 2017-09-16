// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : receive.c
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
* Description       : Receive function
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


void receive(volatile unsigned char *bufptr1)
{
  unsigned char packet = 0;    // status flag
  unsigned char packet_number; // represents 'last successfully received packet'

  packet_number = 0x00;        // initialise to first xmodem packet number - 1
  gl.recv_ptr = bufptr1;       // point to recv buffer

  sendc();                     // send a 'c' until the buffer gets full

  while (packet != end)        // get remainder of file
  {
    recv_wait();               // wait for error or buffer full
    packet = validate_packet((unsigned char*)bufptr1,&packet_number);  // validate the packet
    gl.recv_ptr = bufptr1;     // re-initialize buffer pointer before acknowledging
    switch(packet)
    {
      case good:
        // insert a data handler here,
        // for example, write buffer to a flash device
        break;
      case dup:
        // a counter for duplicate packets could be added here, to enable a
        // for example, exit gracefully if too many consecutive duplicates,
        // otherwise do nothing, we will just ack this
        break;
      case end:
        // do nothing, we will exit
        break;
      default:
        // bad, timeout or error -
        // if required, insert an error handler of some description,
        // for example, exit gracefully if too many errors
        break; // statement just to eliminate compiler warning
    }
    respond(packet);                  // ack or nak
  }// end of file transmission
}
