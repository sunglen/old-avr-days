/*
i2c-eeprom-test.c

this file implement a library access the atmel eeprom:
at24c128 (16k x8bit) and at24c256 (32k x8bit). based on mega48.
have been tested on 24c256(32,768 words of 8 bit each, addr is as long as 15 bit.)

main() show the usage of eeprom functions 
and test by display the byte through the main loop.

hardware:
at24c256 <--> atmega48
A0-->GND
A1-->GND
GND-->GND
SDA----->SDA(PC4 of mega48)
     |
    1KOhm
     |
     V
    VCC

SCL----->SCL(PC5 of mega48)
     |
    1KOhm
     |
     V
    VCC

WP-->GND
VCC-->VCC

see also source code for other connections.

references:
1)at24c128/256 datasheet.
2)atmega48 datasheet.
3)demo project of avr-libc-manual-1.4.0.
*/
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega48 interal RC 8MHz/1=8MHz lfuse=0xe2*/
#define F_CPU 8000000UL
#include <util/delay.h>

/* twi address for at24c128/256:
1 0 1 0 0 A1 A0 R/W
*/
#define SLA_W_24CXXX 0xA0
#define SLA_R_24CXXX 0xA1

/* page size is 64 byte, and address word is 15 bit (at24c256)
   =high 7 bit(as first address word)+low 8 bit(as second address word). */
#define PAGE_SIZE 64

/* Keep activated for 3 ms */
#define LIGHTED_TIME_ms 3

/* define the connection of each segment */
#define SEG_PORT PORTD
#define DDR_SEG_PORT DDRD
#define SEG_a PD0 //disp pin7
#define SEG_b PD1 //disp pin6
#define SEG_c PD2 //disp pin4
#define SEG_d PD3 //disp pin2
#define SEG_e PD4 //disp pin1
#define SEG_f PD5 //disp pin9
#define SEG_g PD6 //disp pin10
#define SEG_dp PD7 //disp pin5

/* define the common Anode scan port */
#define SCAN_PORT PORTB //to pin3 of each disp.
#define DDR_SCAN_PORT DDRB
/* 4 displayers numbered 0-3 from Left to Right */
#define DISP0 PB2
#define DISP1 PB3
#define DISP2 PB4
#define DISP3 PB5

/* 7 segment led display font.
from first to last is:
'0','1','2','3','4','5','6','7',
'8','9','A','b','C','d','E','F','.'(dot),' '(disp off)
*/
const unsigned char _7_seg_font[18] PROGMEM = {
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f),
  (1<<SEG_b)|(1<<SEG_c),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_e)|(1<<SEG_d),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_c)|(1<<SEG_d),
  (1<<SEG_f)|(1<<SEG_g)|(1<<SEG_b)|(1<<SEG_c),
  (1<<SEG_a)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_c)|(1<<SEG_d),
  (1<<SEG_a)|(1<<SEG_f)|(1<<SEG_e)|(1<<SEG_d)|(1<<SEG_c)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f),
  (1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_dp),
  0x00
};

/*
buffer structure:
buffer[m] (m=0,1,2,3) have contents of 0x00 to 0x0F;
buffer[n] (n=m+4, is 4,5,6,7) have content of 0x10 (with dot) 
or 0x11 (without dot).
buffer[t] (t=m+8, is 8,9,10,11) is not the index of fonts,
It is 0x00(disp off) or 0xFF(disp on).
*/
/* save value of TW_STATUS */
uint8_t twst;

void twi_init(void)
{
/* set prescaler to 1.(TWPS0,1=0)*/
TWSR = 0;

#if F_CPU < 3600000UL
TWBR = 10; /* smallest TWBR value. */
#else
TWBR = (F_CPU / 100000UL - 16) / 2;
#endif
}

void init_disp(void)
{
  DDR_SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  DDR_SCAN_PORT |= ((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
  SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
}


/* page write, return nbyte which have sent, otherwise return -1*/
int ee24cxxx_write_page(uint16_t addr, int len, uint8_t *buf)
{
  int rv=0; /* return value */
  uint16_t endaddr;

  /*if the rest space of page is sufficient to store len byte*/
  if ((addr + len) <= (addr | (PAGE_SIZE - 1)))
    endaddr = addr + len;
  /*else the rest space is deficient/insufficient/inadequate to store len byte*/
  else
    endaddr = (addr | (PAGE_SIZE - 1)) + 1;
  /*adjust len*/
  len = endaddr - addr;

 begin:
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START: /* OK, but should not happen */
    case TW_START:
      break;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      return -1;
      /*do /not/ send stop condition becasue I have sent nothing.*/
    }
  
  TWDR = SLA_W_24CXXX;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_SLA_ACK:
      break;
    case TW_MT_SLA_NACK: /* nack during select: device busy writing */
      goto begin;
    case TW_MT_ARB_LOST: /* re-arbitrate */
      goto begin;
    default:
      rv=-1;
      goto quit; /*must send stop condition*/
    }
  TWDR = addr>>8; /* first address word (high)*/
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;
    case TW_MT_DATA_NACK:
      goto quit;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      rv=-1;
      goto quit;
    }
  TWDR = addr&0x00ff; /*second address word (low)*/
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;
    case TW_MT_DATA_NACK:
      goto quit;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      rv=-1;
      goto quit;
    }
  
  for (; len > 0; len--)
    {
      /* the address word low 6 bits are internally incremented following
	 the receipt of each data word. --at24c128/256 datasheet.
      */
      TWDR = *buf; /* send a byte */
      buf++;
      TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      switch ((twst = TW_STATUS))
	{
	case TW_MT_DATA_NACK:
	  rv=-1;
	  goto quit;
	case TW_MT_DATA_ACK:
	  rv++;
	  break;
	default:
	  rv=-1;
	  goto quit;
	}
    }
  
 quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */
  return rv;
  
}

/* also use page write mode */
int ee24cxxx_write_bytes(uint16_t addr, int len, uint8_t *buf)
{
  int rv;
  int total=0;
  
  do
    {
      rv = ee24cxxx_write_page(addr, len, buf);
      /*
	1)rv==-1: error.
	2)rv==0: retry.
	3)rv<len: need to store the rest bytes to a new page.
	4)rv==len: all bytes have been stored in this page. so I can return now.
      */
      if(rv==-1)
	return -1;
      len -= rv;      
      addr += rv;
      buf += rv;
      total += rv;
    }
  while(len > 0);
  
  return total;
      
}

/*random/sequential read mode*/
int ee24cxxx_read_bytes(uint16_t addr, int len, uint8_t *buf)
{
  uint8_t twcr;
  int rv=0;
  
 begin:
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START: /* OK, but should not happen */
    case TW_START:
      break;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      return -1;
      /*do /not/ send stop condition*/
    }
  
  TWDR = SLA_W_24CXXX;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_SLA_ACK:
      break;
    case TW_MT_SLA_NACK: /* nack during select: device busy writing */
      goto begin;
    case TW_MT_ARB_LOST: /* re-arbitrate */
      goto begin;
    default:
      rv=-1;
      goto quit; /*return error and send stop condition*/
    }

  TWDR = addr>>8; /* first address word (high)*/
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;
    case TW_MT_DATA_NACK:
      goto quit;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      rv=-1;
      goto quit;
    }
  TWDR = addr&0x00ff; /*second address word (low)*/
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;
    case TW_MT_DATA_NACK:
      goto quit;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      rv=-1;
      goto quit;
    }
  
  

  /*Next cycle: master receiver mode*/
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send (rep.) start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_START: /* OK, but should not happen */
    case TW_REP_START:
      break;
    case TW_MT_ARB_LOST:
      goto begin;
    default:
      rv=-1;
      goto quit;
    }
  
  TWDR = SLA_R_24CXXX;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MR_SLA_ACK:
      break;
    case TW_MR_SLA_NACK:
      goto quit;
    case TW_MR_ARB_LOST:
      goto begin;
    default:
      rv=-1;
      goto quit;
    }

  /*
    1)len=1: after receiving a byte, the Master Receiver send NACK, then send stop.
    2)len>1: after receiving a byte, the Master Receiver send ACK, so the eeprom
    will send the following byte.
    about TWEA bit, see also at24c128/256 and atmega48 datasheet for details */  
  twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA); /* MR send ACK */
  for(; len>0; len--)
    {
      if(len==1)
	twcr = _BV(TWINT) | _BV(TWEN); /* send NACK */
      
      TWCR=twcr; /* clear interrupt to start transmission */
      
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      switch ((twst = TW_STATUS))
	{
	case TW_MR_DATA_NACK:
	  *buf = TWDR;
	  rv++;
	  len=0; /*just ensure*/
	  break;
	case TW_MR_DATA_ACK:
	  *buf = TWDR;
	  buf++;
	  rv++;
	  break;
	default:
	  rv=-1;
	  goto quit;
	  
	}
    }
 quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */
  return rv;
  
}

int main (void)
{
  uint8_t buffer[12];
  
  uint16_t addr;
  uint8_t buf_in[] = {34,35,36,37,77};
  uint8_t buf_out[5];
  uint8_t data_out;
  
  init_disp();
  
  twi_init();
  
  addr = 1023+61;
  ee24cxxx_write_bytes(addr, 5, buf_in);

  /*seems can work fine without delay.*/
  //_delay_ms(10);

  ee24cxxx_read_bytes(addr, 5, buf_out);

  data_out=buf_out[4];
  
  /* init buffer for correct display*/
  buffer[0]=0x00;
  buffer[4]=0x11;
  buffer[8]=0xFF;
    
  buffer[1]=0x00;
  buffer[5]=0x11;
  buffer[9]=0xFF;
    
  buffer[2]=0x00;
  buffer[6]=0x11;
  buffer[10]=0xFF;

  buffer[3]=0x00;
  buffer[7]=0x11;
  buffer[11]=0xFF;
  
  /* Display buffer that is approx. 4*3ms=12ms per frame.*/
  for (;;) {

    /* just show data_out of 4 bit dec number */
    buffer[0] = data_out/1000;
    buffer[1] = (data_out%1000)/100;
    buffer[2] = (data_out%100)/10;
    buffer[3] = data_out%10;
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[0])|pgm_read_byte(_7_seg_font+buffer[4]))&buffer[8]);
    SCAN_PORT |= (1<<DISP0);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[1])|pgm_read_byte(_7_seg_font+buffer[5]))&buffer[9]);
    SCAN_PORT |= (1<<DISP1);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[2])|pgm_read_byte(_7_seg_font+buffer[6]))&buffer[10]);
    SCAN_PORT |= (1<<DISP2);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[3])|pgm_read_byte(_7_seg_font+buffer[7]))&buffer[11]);
    SCAN_PORT |= (1<<DISP3);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
  }
	
}
