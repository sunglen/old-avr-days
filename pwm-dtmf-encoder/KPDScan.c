/*
  Keypad Scanning Routines

  4x4 Keypad - Matrix Configuration,
  see also connect.txt
  Colums are on pins 0-3,
  Rows are on pins 4-7
  
  === Key Values (in Hex) ===
  0x11 0x12 0x14 0x18
  0x21 0x22 0x24 0x28
  0x41 0x42 0x44 0x48
  0x81 0x82 0x84 0x88
  ===========================
*/

#include "KPDScan.h"

unsigned char KPDScan(void)
{
	uint8_t KPDResult = 0;	// Declare integer variable for pin status

	KPDDDR = 0x0F;		// Set colums as outputs, rows as inputs (binary 00001111)
	KPDPort = 0xF0;	// Set outputs low, and enable pullups on the inputs (binary 11110000)

	_delay_loop_2(4);	// Wait 16 cycles for the port/pins to settle

	KPDResult = (KPDPin & 0xF0);	// Uses a bit mask (binary 11110000) to get the status of the
	// rows (bits 4-7 on the PIN register), stores in variable

	KPDDDR = 0xF0;		// Set colums as inputs, rows as outputs (binary 11110000)
	KPDPort = 0x0F;	// Set outputs low, and enable pullups on the inputs (binary 00001111)

	_delay_loop_2(4);	// Wait 16 cycles for the port/pins to settle

	KPDResult |= (KPDPin & 0x0F);	// Uses a bit mask (binary 00001111) to get the status of the
	// columns (bits 0-3 on the PIN register), stores in variable
	// without changing the lower bits set by the first assignment

	return ~KPDResult;	// Invert the bits and return the result - inverting makes for a better system,
	// as hex "0x00" for no key presses is better than hex "0xFF", etc.
}

//=========================================================================================
