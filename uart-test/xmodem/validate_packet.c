// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : validate_packet.c
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
* Description       : Validates the received packet
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


unsigned char validate_packet(unsigned char *bufptr,unsigned char *packet_number)
{

  unsigned char packet;
  int crc;

// uncomment the following line to allow error simulation
// replace 'bad' with dup, end, err, out as required.
//  if(!PIND_Bit3) return (bad); // simulate an error if button pressed

  packet = bad;
  if (!gl.t1_timed_out)
  {
    if (!gl.recv_error)
    {
      if (bufptr[0] == SOH) // valid start
      {
        if (bufptr[1] == ((*packet_number+1) & 0xff)) // sequential block number ?
        {
          if ((bufptr[1] + bufptr[2]) == 0xff) // block number and block number checksum are ok?
          {
            crc = calcrc(&bufptr[3],128);      // compute CRC and validate it
            if ((bufptr[131] == (unsigned char)(crc >> 8)) && (bufptr[132] == (unsigned char)(crc)))
            {
              *packet_number = *packet_number + 1; // good packet ... ok to increment
              packet = good;
            }// bad CRC, packet stays 'bad'
          }// bad block number checksum, packet stays 'bad'
        }// bad block number or same block number, packet stays 'bad'
        else if (bufptr[1] == ((*packet_number) & 0xff))
        {                         // same block number ... ack got glitched
          packet = dup;           // packet is duplicate, don't inc packet number
        }
      }
      else if (bufptr[0] == EOT)  // check for the end
        packet = end;

      else packet = bad;  // byte zero unrecognised
                          // statement not required, included for clarity
    }
    else packet = err; // UART Framing or overrun error
  }
  else packet = out; // receive timeout error


  return (packet); // one of: good, dup, end, bad, err, out
}
