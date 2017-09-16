 /** Version 2.02  -  76% program mem used 1530/1546 */

#define sleep()  __asm__ __volatile__ ("sleep") 
#define nop()  __asm__ __volatile__ ("nop" ::)
#define UART_BAUD_RATE         9600         /* baud rate*/ 
#define UART_BAUD_SELECT       (F_CPU/(UART_BAUD_RATE*16l)-1) 
#define PRINT(string) (UART_PrintfProgStr(PSTR(string)))
#define lcdata(data) lcdwrt(1, data) 
#define lcdcmd(cmd) lcdwrt(0, cmd) 

#if defined(__AVR_AT90S2313__)
#define UCSRA			USR
#define UBRRL			UBRR
#define UCSRB			UCR
#define EIFR			GIFR
#define TCCR0B			TCCR0
#define SIG_TIMER0_OVF	SIG_OVERFLOW0
#define SIG_INT1		SIG_INTERRUPT1
#endif

#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/pgmspace.h>

extern void UART_PrintfProgStr(unsigned const char* pBuf); 
void TXbyte(char data ); 				//Transmits 1 byte out the UART
void ewrite(char address, char data); 	//Writes char to EEPROM address
unsigned char eread(char address);	//Reads char from EEPROM address
void store(unsigned char edata);		//Compresses and appends digit to the end of eeprom
void trnsfr(char line, char eol);		//Moves digits from eeprom to LCD line
void updtlcd(unsigned char ldata);	//Handles writing new dtmf digit to LCD
void line19(void);						//restores Ln 19 before the scroll
unsigned char fltr(char fdata);		//Filters digits for 10,#,* and 0x0F
void lcdwrt(char dt, char data);		//Writes Byte to LCD dt=0 Control dt=1 data

unsigned char lcdtemp[13];				//holds bottom LCD line
unsigned char eepos;					//eeprom address of marker byte
unsigned char lcdpos;					//position of cursor on LCD
unsigned char lcdln;					//line number on LCD
unsigned char eref;					//eeprom address of last digit on Ln 18
unsigned char esrl;					//eeprom address of last digit on current lcd Ln
unsigned char redraw;					//flag to re-draw Ln 18,19 on incomming digit
unsigned char a;						//temporatry variable
unsigned char uptimeH;					//up botton release filter timer
unsigned char uptimeL;					//up botton pressed filter timer
unsigned char dntimeH;					//down botton release filter timer
unsigned char dntimeL;					//down botton pressed filter timer
volatile unsigned char up;			//up flag
volatile unsigned char dn;			//down flag
 
int main (void) 
{ 
 cli(); 
  //Setup Ports 
 DDRB = 0x00;							//Set PORTB as input
 DDRD = 0x70;							//Set PORTD output pins
 PORTB&= ~_BV(PB4);						//HiZ - Disable decoder chip's output
   //Setup UATRT 
 UBRRL = (char)UART_BAUD_SELECT;	 
 UCSRB = _BV(TXEN);	 					//enable Tx 
  //Setup TimerCounter0
 TCCR0B = 0x03;							//Set TC0 to ck/64 4.5mS IRQs
 TIMSK = _BV(TOIE0);					//Enable OV0 INT
  //Setup MCU 
 GIMSK = _BV(INT1);						//Enable INT1
 MCUCR = 0x2C;							//Setup Sleep mode & INT1 rising
 EIFR = _BV(INTF1);						//Clear INT1 Flag
 
 for (eepos=0;eepos<255;eepos++) {			//scan eeprom for marker byte
 	if (eread(eepos)==0x0F) 
		break;
 }							
 eref = eepos;							//set end of line 18 at end of eeprom
 lcdln = 19;
 redraw = 1;							//Redraw incoming screen on first digit recieved
   
 lcdcmd(0x01);						//Clear LCD, move home
 lcdcmd(0x02);						//Home Cursor
 lcdcmd(0x38);						//Set 8-bit interface
 lcdcmd(0x0C);						//No cursor, display visable
 lcdcmd(0x06);						//Incement display address
 
 TXbyte('\r');
 TXbyte('\n');
 PRINT("DTMF Decoder by www.infidigm.net");	//build message on lcd and out uart
 TXbyte('\r');
 TXbyte('\n');
 
 for (a=0;a<255;a++) {
  	TXbyte(fltr(eread(a + eepos+1)));		//Send entire eeprom out the UART
 }
 TXbyte('\r');
 TXbyte('\n');
 
 sei();
 for (;;) 
	sleep();						//idle until Interrupt
} 
 

SIGNAL(SIG_INT1)					//Exturnal IRQ from DTMF decoder handler
{ 
 unsigned char digit;
 cli();
 DDRB = 0x00;							//Set PORTB = input
 PORTD|= _BV(PD4);						//Enable decoder chip's output
 nop();
 digit = PINB;							//read dtmf digit
 PORTD&= ~_BV(PD4);						//HiZ - Disable decoder chip's output
 digit&= 0x0F;							//Stip off top 4 bits
 updtlcd(fltr(digit));					//send digit to LCD
 store(digit);							//write digit to eeprom
 TXbyte(fltr(digit));					//send digit out the UART
 sei();
} 

SIGNAL(SIG_TIMER0_OVF)				//TC0 overflow IRQ handle - every 4.5mS 
{
 if (PIND & _BV(PD0)) {			//test if up button is released
 	uptimeL=22;						//reset up depressed timer
 	if (uptimeH<22)
		uptimeH++;					//increment up released timer
 	else 
		up=0;						//indicate up filtered release
 }
 else {
 	uptimeH=0;				 		//reset up release timer
 	if (uptimeL>0)
		uptimeL--;					//increment up depressed timer
 	else {
		if (up==0 && lcdln<19) {		//scroll up if not at end (Line 19)
			redraw = 1;					//set flag to redraw incomming screen
			lcdln++;					//increment line number
			lcdcmd(0x01);				//Clear LCD, move home//clear lcd
			trnsfr(lcdln-1, esrl-13);	//draw top line
			lcdcmd(0xC0);				//set cursor at bottom line
			if (lcdln!=19) 
				trnsfr(lcdln, esrl-26);//draw bottom line
			esrl=esrl-13;				//adjust eeprom scroll offset
			if (lcdln==19) {
				line19();				//re-draw line 19
				redraw = 0;
			}
			up = 1;
		}
 	}
 }
 if (PIND & _BV(PD2)) {				//test if down button is released
 	dntimeL=22;							//reset down depressed timer
 	if (dntimeH<22)
		dntimeH++;						//increment down released timer
 	else 
		dn=0;							//indicate down filtered release
 }
 else {
 	dntimeH=0;							//reset down release timer
 	if (dntimeL>0)
		dntimeL--;						//increment down depressed timer
 	else {							
 		if (dn==0 && lcdln>2) {			//scroll down if not at end (Line 1)
 			redraw = 1;					//set flag to redraw incomming screen
 			lcdln--;
 			lcdcmd(0x01);				//Clear LCD, move home//clear lcd
 			trnsfr(lcdln-1, esrl+13);	//draw top line
 			lcdcmd(0xC0);				//set cursor at of bottom line
 			trnsfr(lcdln, esrl);		//draw bottom line
 			esrl=esrl+13;				//adjust eeprom scroll offset
 			dn = 1;
		} 	
 	}
 }
}

void UART_PrintfProgStr(unsigned const char* pBuf) 
{ 
 unsigned char pos;
 pos = 0;
 lcdcmd(0x80);								//LCD Start Position on 1st Row
 while (pgm_read_byte_near(pBuf)!=0) { 	//Go through string until end(null)
	UDR = pgm_read_byte_near(pBuf);			//Send string byte out UART
    if (pos == 0x10) 
		lcdcmd(0xC0);						//If end of 1st goto 2nd Row
	lcdata(pgm_read_byte_near(pBuf));		//Send string byte to LCD
  	pBuf++;									//Point to next byte in string
  	pos++;									//point to next LCD position
    while (!(UCSRA & _BV(UDRE))); 			//Wait for UART to finish TXing
 }     	
} 

void TXbyte(char data ) 				//Transmits 1 byte out the UART
{ 
 while (!(UCSRA & _BV(UDRE))); 		//Wait for UART to finish TXing
 UDR = data; 	 	   					//start transmittion 
} 


void ewrite(char address, char data)	//Writes char to EEPROM address 
{ 
 cli(); 
 while (EECR & _BV(EEWE));
 EEARL = address>>1; 
 EEDR = data; 
 EECR|= _BV(EEMWE);
 EECR|= _BV(EEWE);
 sei(); 
} 
  
unsigned char eread(char address)		//Reads char from EEPROM address 
{ 
 EEARL = address>>1; 
 EECR|= _BV(EERE);
 if (address&1) 
	return (EEDR & 0x0F);				//Read LSN on odd address
 else 
	return (EEDR >> 4);				//Read MSN on even address
} 


void store(unsigned char edata)		//Compresses and appends digit to the end of eeprom
{
 unsigned char old;
 if (eepos&1) {							//test if Marker is in MSN or LSN
 	old = eread(eepos-1);				//read byte that will be overwritten
 	old = old<<4;
 	old|= edata;						//place new in LSN
 	ewrite(eepos,old);					//write new byte
 	eepos++;							//increment eeprom address
 	old = eread(eepos+1);				//read byte that will be overwritten
 	old|= 0xF0;							//put Marker in MSN, keep data in LSN
 	ewrite(eepos,old);					//write new marker byte
 }
 else {
 	edata = (edata << 4);				//Move data in LSN to MSN
 	edata|= 0x0F;						//put Marker in LSN
 	ewrite(eepos,edata);				//write new marker and data byte
 	eepos++;
 }
}


void trnsfr(char line, char eol)		//Moves digits from eeprom to LCD line
{
 unsigned char hold,pop;
 if (line<10) {							//test if line is less than 10
 	lcdata(' ');						//write space on LCD
 	lcdata((line|0x30));				//write 'ones' line number on LCD
 }
 else {									//if line greater than 9....
 	lcdata('1');						//write 1 on LCD
 	lcdata(((line-10)|0x30));			//write 'ones' line number on LCD
 }
 lcdata('-');							//write "-" after line number
 for (pop=1;pop<=13;pop++) {			//Read nibbles
 	hold = eread(pop+eref-eol-14);		//Get byte from eeprom
 	lcdata(fltr(hold));					//Send to LCD
 }
}

void updtlcd(unsigned char ldata)		//Handles writing new dtmf digit to LCD
{
 if (redraw==1 && lcdpos<13) {			// Redraw LCD if up/dn stuff is on it
 	redraw = 0;
 	esrl = 0;
 	lcdln = 19;							//reset line numbner
 	lcdcmd(0x01);						//Clear LCD, move home//clear lcd
 	trnsfr(18, 0);						//draw line 18
 	lcdcmd(0xC0);						//set cursor at begining of bottom line
 	line19();							//re-draw line 19
 }
 if (lcdpos>=13) {
 	lcdcmd(0x01);						//Clear LCD, move home//clear lcd
 	lcdata('1');						//write "18-"
 	lcdata('8');
 	lcdata('-');
 	for (lcdpos=0;lcdpos<=13;lcdpos++) {	//Move bottom line to top
 		lcdata(lcdtemp[lcdpos]);
 	}
 	lcdcmd(0xC0);						//set cursor at begining of bottom line
 	lcdata('1');						//write "19-"
 	lcdata('9');
 	lcdata('-');
 	lcdpos = 0;							//Reset line position
 	
 	eref = eepos;						//set end of line 18 at end of eeprom
 	esrl = 0;
 	lcdln = 19;							//reset line numbner
 }
 lcdata(ldata);							//Write digit to LCD
 lcdtemp[lcdpos] = ldata;				//Store digit in ram
 lcdpos++;
}

void line19(void)						//restores Ln 19 before the scroll
{
 lcdata('1');							//Write "19-
 lcdata('9');
 lcdata('-');
 for (a=0;a<lcdpos;a++) {				//loop until all digits have been restored
 	lcdata(lcdtemp[a]);					//restore LCD Line 19 from ram
 }
}

unsigned char fltr(char fdata)		//Filters digits for 10,#, and *
{
 if(fdata==0x0F) 
	return (0x20);						//correct for blank cell
 if(fdata==0x0A) 
	return (0x30);						//correct for Zero from decoder
 if(fdata==0x0B) 
	return (0x2A);						//correct for * from decoder
 if(fdata==0x0C) 
	return (0x23);						//correct for # from decoder
 else 
	return (fdata|0x30);
}

void lcdwrt(char dt, char data)		// Writes Byte to LCD dt=0 Control dt=1 data
{
 volatile unsigned int x;
 DDRB = 0xFF;							//Set PORTB as Output
 if (dt == 1) 
	PORTD|= _BV(PD5);  					//Data PD5=1 or Control PD5=0
 else 
	PORTD&= ~_BV(PD5);
 PORTB = data;							//Send data byte (PORTB)
 for (x=0;x < 50;x++);					//Delay 250uS
	PORTD|= _BV(PD6);  					//Set Strobe 
 for (x=0;x < 50;x++);					//Delay 250uS
	PORTD&= ~_BV(PD6);					//Clear Strobe 
 for (x=0;x < 200;x++);				//Delay 1000uS
	DDRB = 0x00;						//Set PORTB as Input
}
