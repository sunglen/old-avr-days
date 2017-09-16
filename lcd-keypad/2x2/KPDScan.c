/*
  Keypad Scanning Routines --could NOT handle multiplex!
  
	2x2 Keypad - Matrix Configuration,
	Colums are on pins 0-1,
	Rows are on pins 2-3

	=== Key Values (in Bin(Dec)) ===
	0b0101(5) 0b0110(6)
	0b1001(9) 0b1010(10)
	===========================
*/

#include "KPDScan.h"

unsigned char KPDScan(void)
{
	uint8_t KPDResult = 0;	// Declare integer variable for pin status

	KPDDDR = 0x03;		// Set colums as outputs, rows as inputs 
	KPDPort = ~0x03;	// Set outputs low, and enable pullups on the inputs 

	_delay_loop_2(4);	// Wait 16 cycles for the port/pins to settle

	KPDResult = (KPDPin & 0x0C);	// Uses a bit mask to get the status of the rows
	// ! read PINX to get the status rather than PORTX

	KPDDDR = 0x0C;		// Set colums as inputs, rows as outputs
	KPDPort = ~0x0C;	// Set outputs low, and enable pullups on the inputs

	_delay_loop_2(4);	// Wait 16 cycles for the port/pins to settle

	KPDResult |= (KPDPin & 0x03);	// Uses a bit mask to get the status of the columns
	// without changing the lower bits set by the first assignment

	return (~KPDResult & 0x0F);	// Invert the bits and return the result - inverting makes for a better system
}

//=========================================================================================
