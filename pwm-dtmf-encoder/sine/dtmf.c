//***************************************************************************
//* Title                : DTMF Generator
//* DESCRIPTION
//*
//***************************************************************************

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "lcd.h"
#include "KPDScan.h"

#define  Xtal       8000000          // system clock frequency
#define  prescaler  1                // timer1 prescaler
#define  N_samples  128              // Number of samples in lookup table
#define  Fck        Xtal/prescaler   // Timer1 working frequency

//************************** SIN TABLE *************************************
// Samples table : one period sampled on 128 samples and
// quantized on 7 bit
//f(x)=63+63sin(2*pi*(x/128)) xE[0,127]
//**************************************************************************

#define kk 4
/*
  */
const unsigned char auc_SinParam[] PROGMEM = {
63,66,69,72,75,78,81,(80+PC4),87,90,93,95,98,101,103,105,108,110,112,114,115,117,119,120,121,122,123,124,125,125,126,126,126,126,126,125,125,124,123,122,121,120,119,117,115,114,112,110,108,105,103,101,98,95,93,90,87,84,81,78,75,72,69,66,63,60,57,54,51,48,45,42,39,36,33,31,28,25,23,21,18,16,14,12,11,9,7,6,5,4,3,2,1,1,0,0,0,0,0,1,1,2,3,4,5,6,7,9,11,12,14,16,18,21,23,25,28,31,33,36,39,42,45,48,51,54,57,60
};

//***************************  x_SW  ***************************************
//Table of x_SW (excess 8): x_SW = ROUND(8*fa,b/f_sine_base)
//**************************************************************************

//low frequency (row)
//697hz  ---> x_SW = 23
//770hz  ---> x_SW = 25
//852hz  ---> x_SW = 27
//941hz  ---> x_SW = 31

const unsigned char auc_frequencyL[4] = {
  23,25,
  27,31};

//high frequency (coloun)
//1209hz  ---> x_SW = 40
//1336hz  ---> x_SW = 44
//1477hz  ---> x_SW = 48
//1633hz  ---> x_SW = 54

const unsigned char auc_frequencyH[4] = {
40,44,
48,54};



//**************************  global variables  ****************************
volatile unsigned char x_SWa = 0x00;               // step width of high frequency
volatile unsigned char x_SWb = 0x00;               // step width of low frequency
volatile unsigned int  i_CurSinValA = 0;           // position freq. A in LUT (extended format)
volatile unsigned int  i_CurSinValB = 0;           // position freq. B in LUT (extended format)
volatile unsigned int  i_TmpSinValA;               // position freq. A in LUT (actual position)
volatile unsigned int  i_TmpSinValB;               // position freq. B in LUT (actual position)
volatile unsigned int i_sine_tab=0;

//**************************************************************************
// Timer overflow interrupt service routine
//**************************************************************************
SIGNAL (SIG_OVERFLOW1)
{ 
  if (x_SWa == 0 || x_SWb == 0){
    //    OCR1A = (auc_SinParam[i_sine_tab%128]*2);
    //generate the base sine wave.
    //and this way can avoid noise.
    OCR1A = pgm_read_byte(auc_SinParam+(i_sine_tab%128)) * 2;
    i_sine_tab++;

  }else{
  // move Pointer about step width aheaed
  i_CurSinValA += x_SWa;       
  i_CurSinValB += x_SWb;
  // normalize Temp-Pointer
  // X_lut_a,b = (round(1/8 * (X'_lut_a,b,ext + 8*(f_a,b/f_sine_base))))%Nc
  i_TmpSinValA  =  (char)(((i_CurSinValA+4) >> 3)&(0x007F)); 
  i_TmpSinValB  =  (char)(((i_CurSinValB+4) >> 3)&(0x007F));
  // calculate PWM value: high frequency value + 3/4 low frequency value
  OCR1A = (pgm_read_byte(auc_SinParam+i_TmpSinValA) + 
(pgm_read_byte(auc_SinParam+i_TmpSinValB) - (pgm_read_byte(auc_SinParam+i_TmpSinValB)>>2)));
  }
}

//**************************************************************************
// Initialization
//**************************************************************************
void init_pwm (void)
{
  TIMSK1 = (1<<TOIE1);                     // Int T1 Overflow enabled
  TCCR1A = (1<<COM1A1)|(1<<WGM10);   // 8Bit high speed PWM (mega48 datasheet_cn p115-p117)
  //  TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<WGM10);   // doc2545_cn p108 Figure47 and doc2545_en p120 Figure 14-7 must be wrong!
  TCCR1B = (1<<CS10)|(1<<WGM12);                // CLK/1
  DDRB |= (1 << PB1);               // OC1A as output
  sei();                     	     // Interrupts enabled
}


//**************************************************************************
// MAIN
// Read keypad for
// tone to generate.
// fix x_SWa and x_SWb
//**************************************************************************

int main (void)
{
  uint8_t key_val;
  init_pwm();
  
  /* initialize display, cursor off */
  lcd_init(LCD_DISP_ON);

  for(;;){ 
      key_val=KPDScan();
      /* output the key value (dec) to LCD */
      lcd_gotoxy(0,0);
      lcd_putc('0'+key_val/100);
      lcd_gotoxy(1,0);
      lcd_putc('0'+(key_val%100)/10);
      lcd_gotoxy(2,0);
      lcd_putc('0'+(key_val%100)%10);

      switch (key_val) {
      case NOKEY:
	x_SWb = 0;
	x_SWa = 0;
	break;
      case KEY1:
	x_SWb = auc_frequencyL[0];
        x_SWa = auc_frequencyH[0];
	break;
      case KEY2:
	x_SWb = auc_frequencyL[0];
        x_SWa = auc_frequencyH[1];
	break;
      case KEY3:
	x_SWb = auc_frequencyL[0];
        x_SWa = auc_frequencyH[2];
	break;
      case KEY4:
	x_SWb = auc_frequencyL[0];
        x_SWa = auc_frequencyH[3];
	break;
      case KEY5:
	x_SWb = auc_frequencyL[1];
        x_SWa = auc_frequencyH[0];
	break;
      case KEY6:
	x_SWb = auc_frequencyL[1];
        x_SWa = auc_frequencyH[1];
	break;
      case KEY7:
	x_SWb = auc_frequencyL[1];
        x_SWa = auc_frequencyH[2];
	break;
      case KEY8:
	x_SWb = auc_frequencyL[1];
        x_SWa = auc_frequencyH[3];
	break;
      case KEY9:
	x_SWb = auc_frequencyL[2];
        x_SWa = auc_frequencyH[0];
	break;
      case KEY10:
	x_SWb = auc_frequencyL[2];
        x_SWa = auc_frequencyH[1];
	break;
      case KEY11:
	x_SWb = auc_frequencyL[2];
        x_SWa = auc_frequencyH[2];
	break;
      case KEY12:
	x_SWb = auc_frequencyL[2];
        x_SWa = auc_frequencyH[3];
	break;
      case KEY13:
	x_SWb = auc_frequencyL[3];
        x_SWa = auc_frequencyH[0];
	break;
      case KEY14:
	x_SWb = auc_frequencyL[3];
        x_SWa = auc_frequencyH[1];
	break;
      case KEY15:
	x_SWb = auc_frequencyL[3];
        x_SWa = auc_frequencyH[2];
	break;
      case KEY16:
	x_SWb = auc_frequencyL[3];
        x_SWa = auc_frequencyH[3];
	break;
      }
  }
}

