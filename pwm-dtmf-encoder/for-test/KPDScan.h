/*
  Keypad Scanning Routines
  
  4x4 Keypad - Matrix Configuration,
  Colums are on pins 0-3,
  Rows are on pins 4-7
  
  === Key Values (in Hex) ===
  0x11 0x12 0x14 0x18
  0x21 0x22 0x24 0x28
  0x41 0x42 0x44 0x48
  0x81 0x82 0x84 0x88
  ===========================
*/

// Function Declares
	unsigned char KPDScan (void);	// Scanning routine
// End Function Declares

// Includes
	#include <avr/io.h>
        #define F_CPU 8000000
	#include <util/delay.h>
// End Includes

// Aliases for KPD routines
	#define KPDPort	PORTD
	#define KPDPin		PIND
	#define KPDDDR		DDRD
// End Aliases

// Key Equivalents (makes coding easier)
	#define NOKEY	0
	#define KEY1	17
	#define KEY2	18
	#define KEY3	20
	#define KEY4	24
	#define KEY5	33
	#define KEY6	34
	#define KEY7	36
	#define KEY8	40
	#define KEY9	65
	#define KEY10	66
	#define KEY11	68
	#define KEY12	72
	#define KEY13	129
	#define KEY14	130
	#define KEY15	132
	#define KEY16	136
// End Key Equivalents
