// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : xmodem.h
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
* Description       : Xmodem receive utility header file
*
****************************************************************************/

#include "iom48.h"
#include "inavr.h"
#pragma language=extended

#define SOH 01
#define EOT 04
#define ACK 06
#define NAK 21
#define CRCCHR 'C'

#define TRUE 0x01
#define FALSE 0x0

#define full 0xff
#define empty 0x00

#define bad 0x00
#define good 0x01
#define dup 0x02
#define end 0x03
#define err 0x04
#define out 0x05

#define DEBUG_PIN     PORTD_Bit5 // spare pin used for debug signalling only
// Usage:
// DEBUG_PIN=0;
// DEBUG_PIN=1;

// function prototypes
void receive(volatile unsigned char *bufptr1);
void purge(void);
void init(void);
unsigned char validate_packet(unsigned char *bufptr, unsigned char *packet_number);
void respond(unsigned char packet);
void recv_wait(void);
void sendc(void);
int calcrc(unsigned char *ptr, int count);
