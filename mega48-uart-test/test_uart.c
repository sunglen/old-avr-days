/*************************************************************************
describ:
communicate with minicom(/dev/ttyUSB1, 96008N1) and 
mega48.
through the USB port supported by FT232BM.

hardware:
ft232bm bread board<--usb line--> usb prot of linux 
about ft232bm bread board see the paper copy of schmatics.

ft232bm <---> mega48
TXD <-->  PD0 (RXD)
RXD <-->  PD1 (TXD)
ft232bm and mega48 use common VTG form Vbus(5.0v) of USB port.
and common GND.

software:
1)I added the some code to echo chars to LCD.
2)use minicom through /dev/ttyUSB1, 96008N1
-me
*************************************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/pgmspace.h>

#include "uart.h"

#include "lcd.h"

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

/* 9600 baud */
#define UART_BAUD_RATE      9600      

#define LCD_PAGE 32

vint main(void)
{
  
  unsigned int c;
  unsigned int i=0, j;
  char buffer[LCD_PAGE];
  //    int  num=134;
    
  for(j=0;j<LCD_PAGE;j++){
    buffer[j]='\0';
  }
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("a uart test.\n2005/11/28");

    /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro 
     *  UART_BAUD_SELECT() (normal speed mode )
     *  or 
     *  UART_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
     */
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
    
    /*
     * now enable interrupt, since UART library is interrupt controlled
     */
    sei();
    
    /*
     *  Transmit string to UART
     *  The string is buffered by the uart library in a circular buffer
     *  and one character at a time is transmitted to the UART using interrupts.
     *  uart_puts() blocks if it can not write the whole string to the circular 
     *  buffer
     */
    uart_puts("String stored in SRAM\n");
    
    /*
     * Transmit string from program memory to UART
     */
    uart_puts_P("String stored in FLASH\n");
    
        
    /* 
     * Use standard avr-libc functions to convert numbers into string
     * before transmitting via UART
     */     
    //    itoa( num, buffer, 10);   // convert interger into string (decimal format)         
    //    uart_puts(buffer);        // and transmit string to UART

    
    /*
     * Transmit single character to UART
     */
    uart_putc('\r');
    
    for(;;)
    {
        /*
         * Get received character from ringbuffer
         * uart_getc() returns in the lower byte the received character and 
         * in the higher byte (bitmask) the last receive error
         * UART_NO_DATA is returned when no data is available.
         *
         */
        c = uart_getc();
        if ( c & UART_NO_DATA )
        {
            /* 
             * no data available from UART 
             */
        }
        else
        {
            /*
             * new data available from UART
             * check for Frame or Overrun error
             */
            if ( c & UART_FRAME_ERROR )
            {
                /* Framing Error detected, i.e no stop bit detected */
                uart_puts_P("UART Frame Error: ");
            }
            if ( c & UART_OVERRUN_ERROR )
            {
                /* 
                 * Overrun, a character already present in the UART UDR register was 
                 * not read by the interrupt handler before the next character arrived,
                 * one or more received characters have been dropped
                 */
                uart_puts_P("UART Overrun Error: ");
            }
            if ( c & UART_BUFFER_OVERFLOW )
            {
                /* 
                 * We are not reading the receive buffer fast enough,
                 * one or more received character have been dropped 
                 */
                uart_puts_P("Buffer overflow error: ");
            }
            /* 
             * echo character to terminal.
             */
	    uart_putc( (unsigned char)c );
	    
	    // send string back to terminal and LCD.
	    buffer[i]=(char)c;
	    i++;
	    if ( i > LCD_PAGE-2 || (char)c == '\r'){
	      lcd_clrscr();
	      lcd_puts(buffer);

	      // act as a unix echo command.
	      uart_puts("\n\r");
	      uart_puts(buffer);
	      uart_puts("\n\r");
	      
	      for(j=0;j<i;j++){
		buffer[j]='\0';
	      }
	      i=0;
	    }
        }
    }
    
}
