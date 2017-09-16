/* LCD interface program for using the Atmel AVR to run a Hitachi HD44780 LCD controller */
/* Copyleft (!C) Mark A. Post, mark@eyetap.org, http://www.eyetap.org/~post */

/* Code Version 0.5 alpha, October 1, 2004 */
/* Tested on an AT90S8515 AVR and HD44780-based 2x24 LCD from Active Surplus */
/* Known bugs:  polling LCD busy line, reading in general (AVR problem?) */
/* Feel free to notify me of any other problems encountered */

#include <avr/io.h>

/**********************/
/* Pinout Connections */
/**********************/

/*
Most HD44780-based displays use the same pin arrangement.
This table shows the pin numbers, their function, and the
 AVR connections that are used for this program.
The AVR ports are named PA0 for port A pin 0, etc, as on an Atmel STK500 board
LCD pins are usually in-line starting at board corner, or in a 14-pin ribbon header
Use a high-resistance ~1Mohm potentiometer for contrast control, between GND and Vcc

        AVR ports                Function                LCD Pins

        GND (0V)                 Ground                  1 (GND)
        VTG (+5V)                Vcc                     2 (Vcc)
        (pot 0V-5V)              Contrast                3 (Vcont)
        PB2 (CTRL2)              Register Select         4 (RS)
        PB1 (CTRL1)              Read/Write              5 (R/W)
        PB0 (CTRL0)              Edge Enable             6 (E)
        PD0 (DATA0)              Data Bit 0 (LSB)        7 (D0)
        PD1 (DATA1)              Data Bit 1              8 (D1)
        PD2 (DATA2)              Data Bit 2              9 (D2)
        PD3 (DATA3)              Data Bit 3              10(D3)
        PD4 (DATA4)              Data Bit 4              11(D4)
        PD5 (DATA5)              Data Bit 5              12(D5)
        PD6 (DATA6)              Data Bit 6              13(D6)
        PD7 (DATA7)              Data Bit 7 (MSB)        14(D7)

Note that PORTA is initialized as an input for switches, and
 PORTC is initialized as an output for LEDS.  This allows for input and
 output from the AVR itself, e.g. from the STK500 active-low switch/LED headers.
*/


/***************/
/* AVR Defines */
/***************/

//Speed of microcontroller (in Hz)
//The STK500 defaults to 3.68MHz, delay function takes about 3 cycles
//This number, though, is based on tests of delay(), and is approximate
#define AVR_FREQ 368000

//Convert time in fraction of a second to cycles
#define FRACSEC(fs) AVR_FREQ/(fs)
#define SECONDS(fs) AVR_FREQ*(fs)

//General delay periods
#define LCD_WRITE_DELAY FRACSEC(4000)
#define LCD_LINE_DELAY SECONDS(1)

//Data Direction Register Values
#define DDR_INPUT 0x00
#define DDR_OUTPUT 0xff

/***************/
/* LCD Defines */
/***************/

//LCD dimensions -- used to set text properly
#define LCD_COLUMNS 16 //16x2 (mine)
#define LCD_ROWS 2
#define LCD_GREYZONE 16

//LCD port mappings -- dependent on AVR wiring
#define LCD_DATA_DDR DDRD
#define LCD_DATA PORTD
#define LCD_DATA_IN PIND
#define LCD_CTRL_DDR DDRB
#define LCD_CTRL PORTB
#define LCD_ECHO_DDR DDRC
#define LCD_ECHO PORTC
#define LCD_INPUT_DDR DDRA
#define LCD_INPUT PORTA

//HD44780 control pins, bits mapped to EE, R/W, and RS
//assuming LCD_CTRL pin0=Edge Enable, pin1=Read/Write, pin2=Reg Select
#define LCD_CTRL_EE 0x01
#define LCD_CTRL_READ 0x02
#define LCD_CTRL_WRITE 0x00
#define LCD_CTRL_CHAR 0x04
#define LCD_CTRL_CMD 0x00

//HD44780 command set and command flags,
//assuming LCD_DATA pin0 is least significant bit (D0)
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET(flags) (0x04|flags)
	#define LCD_EMS_CURSOR_RIGHT 0x02
	#define LCD_EMS_CURSOR_LEFT 0x00
	#define LCD_EMS_DISPLAY_SHIFT 0x01
#define LCD_DISPLAY_CONTROL(flags) (0x08|flags)
	#define LCD_DC_DISPLAY_ON 0x04
	#define LCD_DC_CURSOR_ON 0x02
	#define LCD_DC_DISPLAY_OFF 0x00
	#define LCD_DC_CURSOR_OFF 0x00
	#define LCD_DC_BLINK_CURSOR 0x01
#define LCD_DISPLAY_SHIFT(flags) (0x10|flags)
	#define LCD_DS_DISPLAY 0x08
	#define LCD_DS_CURSOR 0x00
	#define LCD_DS_RIGHT 0x04
	#define LCD_DS_LEFT 0x00
#define LCD_FUNCTION_SET(flags) (0x20|flags)
	#define LCD_FS_DATA_LENGTH_8BIT 0x10
	#define LCD_FS_DATA_LENGTH_4BIT 0x00
	#define LCD_FS_DISPLAY_2LINES 0x08
	#define LCD_FS_DISPLAY_1LINE 0x00
	#define LCD_FS_FONT_5x10DOT 0x04
	#define LCD_FS_FONT_5x8DOT 0x00
#define LCD_SET_CGRAM_ADDRESS(addx) (0x40|(addx&0x3f))
#define LCD_SET_DDRAM_ADDRESS(addx) (0x80|(addx&0x7f))
#define LCD_BUSY_BIT 0x80

#define outp(val,tag) ((tag)=(val))

/*****************/
/* AVR Functions */
/*****************/

/* Delay function */
void delay(unsigned long N)
{
	unsigned long n;
	if(N == 0) return;
	for (n=0; n<N; n++);
}

void AVR_Initialize(void)
{
	//Initialize AVR pins
	outp(DDR_OUTPUT,LCD_CTRL_DDR); //output to LCD control
	outp(0x00,LCD_CTRL);
	outp(DDR_OUTPUT,LCD_DATA_DDR); //output and input to LCD 8-bit data
	outp(0x00,LCD_DATA);
	outp(DDR_OUTPUT,DDRC); //LED output for STK500
	outp(0xff,PORTC);
	outp(DDR_INPUT,DDRA); //switch input for STK500
}


/*****************/
/* LCD Functions */
/*****************/

/* Read LCD busy flag to determine if it's busy */
void LCD_Busywait(void)
{
	outp(DDR_INPUT,LCD_DATA_DDR);
	outp(LCD_CTRL_CMD|LCD_CTRL_READ,LCD_CTRL);
	delay(LCD_WRITE_DELAY);
	while((LCD_DATA_IN)&LCD_BUSY_BIT);
	outp(DDR_OUTPUT,LCD_DATA_DDR);
}

/* Read LCD DDRAM address counter */
char LCD_Address_Read(void)
{
	char addresscounter;
	outp(DDR_INPUT,LCD_DATA_DDR);
	outp(LCD_CTRL_CMD|LCD_CTRL_READ,LCD_CTRL);
	delay(LCD_WRITE_DELAY);
	addresscounter = (LCD_DATA_IN);
	delay(LCD_WRITE_DELAY);
	outp(DDR_OUTPUT,LCD_DATA_DDR);
	return addresscounter&(~LCD_BUSY_BIT);
}

/* Read an LCD character byte from DDRAM */
char LCD_Char_Read(void)
{
	char character;
	outp(DDR_INPUT,LCD_DATA_DDR);
	outp(LCD_CTRL_CHAR|LCD_CTRL_READ,LCD_CTRL);
	delay(LCD_WRITE_DELAY);
	character = (LCD_DATA_IN);
	delay(LCD_WRITE_DELAY);
	outp(DDR_OUTPUT,LCD_DATA_DDR);
	return character;
}

/* Write an LCD command byte */
void LCD_Cmd_Write(char command)
{
	outp(LCD_CTRL_EE|LCD_CTRL_CMD|LCD_CTRL_WRITE,LCD_CTRL);
	outp(command,LCD_DATA);
	delay(LCD_WRITE_DELAY);
	outp(LCD_CTRL_CMD|LCD_CTRL_WRITE,LCD_CTRL);
	delay(LCD_WRITE_DELAY);
}

/* Write an LCD character byte */
void LCD_Char_Write(char character)
{
	outp(LCD_CTRL_EE|LCD_CTRL_CHAR|LCD_CTRL_WRITE,LCD_CTRL);
	outp(character,LCD_DATA);
	delay(LCD_WRITE_DELAY);
	outp(LCD_CTRL_CHAR|LCD_CTRL_WRITE,LCD_CTRL);
	delay(LCD_WRITE_DELAY);
}

/* Initialize and reset LCD */
void LCD_Initialize(void)
{
	LCD_Cmd_Write(LCD_FUNCTION_SET(LCD_FS_DATA_LENGTH_8BIT|LCD_FS_DISPLAY_2LINES|LCD_FS_FONT_5x10DOT));
	delay(LCD_WRITE_DELAY);
	LCD_Cmd_Write(LCD_DISPLAY_CONTROL(LCD_DC_DISPLAY_ON|LCD_DC_CURSOR_ON));
	delay(LCD_WRITE_DELAY);
	LCD_Cmd_Write(LCD_ENTRY_MODE_SET(LCD_EMS_CURSOR_RIGHT));
	delay(LCD_WRITE_DELAY);
	LCD_Cmd_Write(LCD_CLEAR_DISPLAY);
	delay(LCD_WRITE_DELAY*4);
}

void LCD_Line_Write(char string[])
{
	char line = 0;
	char counter = 0;
	char blank = 0;
	char writelimit;
	LCD_Cmd_Write(LCD_RETURN_HOME);
	delay(LCD_WRITE_DELAY*4);
	while(line < LCD_ROWS && string[counter] != '\0')
	{
		writelimit = (LCD_COLUMNS*(line+1)+LCD_GREYZONE*line);
		while(string[counter] != '\0' && string[counter] != '\n' && counter < writelimit)
			LCD_Char_Write(string[counter++]);
		for(blank = counter; string[counter] != '\0' && blank < writelimit; blank++)
			LCD_Char_Write(' ');
		if(string[counter] != '\0' && line < LCD_ROWS)
			LCD_Cmd_Write(LCD_SET_DDRAM_ADDRESS(++line*(LCD_COLUMNS+LCD_GREYZONE)));
		if(string[counter] == '\n')
			counter++;
	}
}


/*****************/
/* Main Program  */
/*****************/

char LineData[] = "Hello, World!!!\nHow Are You???";

int main(void)
{
	AVR_Initialize();
	LCD_Initialize();
	LCD_Line_Write("AVR interface to LCD");
	delay(LCD_LINE_DELAY);
	LCD_Line_Write(LineData);
}
