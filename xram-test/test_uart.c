/*************************************************************************
Title:    example program for the Interrupt controlled UART library
Author:   Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:     $Id: test_uart.c,v 1.4 2005/07/10 11:46:30 Peter Exp $
Software: AVR-GCC 3.3
Hardware: any AVR with built-in UART, tested on AT90S8515 at 4 Mhz

DESCRIPTION:
          This example shows how to use the UART library uart.c

I added the some code to echo chars to LCD.
-me
*************************************************************************/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "uart.h"


/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

/* 9600 baud */
#define UART_BAUD_RATE      9600      

unsigned char big_array[4096];

void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}


int main(void)
{
  
  unsigned int c;
  unsigned int i;
  //  uint8_t *p_xram;
//  big_array=(unsigned char *)0x0260;


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
    uart_puts("\r\n");
    
    /*
    for(i=0x0260;i<=0x7fff;i++)
      {
	*(uint8_t *)i='0'+(i%10);
      }

    */
    /*
    *(uint8_t *)0x025d='N';
    
    *(uint8_t *)0x025e='0';
    
    *(uint8_t *)0x025f='1';
    */
    /*
    p_xram=(uint8_t *)0x0260;
    *p_xram='A';

    *(uint8_t *)0x0261='B';
    */
    /*
    uart_putc(*(uint8_t *)0x025d);
    uart_putc(*(uint8_t *)0x025e);    
    uart_putc(*(uint8_t *)0x025f);    
    */
    
    /*
    for(i=0x0260;i<=0x7fff;i++)
    {
	if(i%50==0)
	  uart_puts("\r\n");
	  uart_putc(*(uint8_t *)i);
	  }
    */
    
    /*
    uart_putc(*p_xram);
    uart_putc(*(uint8_t *)0x0261);
    */
    
    uart_puts("\r\n");
    
    
    for(i=0;i<4096;i++)
      big_array[i]=0x61+i%26;

    for(i=0;i<4096;i++)
    {    
      if(i%50==0)
	uart_puts("\r\n");
      uart_putc(big_array[i]);
    }
    
    uart_puts("\r\n");
    /*
    uart_putc(*(uint8_t *)0x025d);
    uart_putc(*(uint8_t *)0x025e);    
    uart_putc(*(uint8_t *)0x025f);
    
    p_xram=(uint8_t *)0x0260;
    uart_putc(*p_xram);

    
    uart_putc(*(uint8_t *)0x0261);
    uart_putc(*(uint8_t *)0x0262);
    uart_putc(*(uint8_t *)0x0263);
    */
    
    /*
     * Transmit string from program memory to UART
     */
    uart_puts_P("stored in SRAM\n");
    
        
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
	  //	  uart_puts_P("No data accepted!\n");
	  //	  mydelay(0xff);
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
	    

        }
    }
    
}
