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
	#define KEY1	5
	#define KEY2	6
	#define KEY3	9
	#define KEY4	10
// End Key Equivalents
