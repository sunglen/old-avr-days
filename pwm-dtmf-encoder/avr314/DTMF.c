//***************************************************************************
//* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
//*
//* Number               : AVR314
//* File Name            : "dtmf.c"
//* Title                : DTMF Generator
//* Date                 : 00.06.27
//* Version              : 1.0
//* Target MCU           : Any AVR with SRAM, 8 I/O pins and PWM 
//*
//* DESCRIPTION
//* This Application note describes how to generate DTMF tones using a single
//* 8 bit PWM output.
//*
//***************************************************************************

#include <stdio.h>
#define __IAR_SYSTEMS_ASM__   
#include <io4414.h>
#include "ina90.h"

#define  Xtal       8000000          // system clock frequency
#define  prescaler  1                // timer1 prescaler
#define  N_samples  128              // Number of samples in lookup table
#define  Fck        Xtal/prescaler   // Timer1 working frequency
#define  delaycyc   10               // port B setup delay cycles

//************************** SIN TABLE *************************************
// Samples table : one period sampled on 128 samples and
// quantized on 7 bit
//**************************************************************************
flash unsigned char auc_SinParam [128] = {
64,67,
70,73,
76,79,
82,85,
88,91,
94,96,
99,102,
104,106,
109,111,
113,115,
117,118,
120,121,
123,124,
125,126,
126,127,
127,127,
127,127,
127,127,
126,126,
125,124,
123,121,
120,118,
117,115,
113,111,
109,106,
104,102,
99,96,
94,91,
88,85,
82,79,
76,73,
70,67,
64,60,
57,54,
51,48,
45,42,
39,36,
33,31,
28,25,
23,21,
18,16,
14,12,
10,9,
7,6,
4,3,
2,1,
1,0,
0,0,
0,0,
0,0,
1,1,
2,3,
4,6,
7,9,
10,12,
14,16,
18,21,
23,25,
28,31,
33,36,
39,42,
45,48,
51,54,
57,60};

//***************************  x_SW  ***************************************
//Table of x_SW (excess 8): x_SW = ROUND(8*N_samples*f*510/Fck)
//**************************************************************************

//high frequency (coloun)
//1209hz  ---> x_SW = 79
//1336hz  ---> x_SW = 87
//1477hz  ---> x_SW = 96
//1633hz  ---> x_SW = 107

const unsigned char auc_frequencyH [4] = {
107,96,
87,79};

//low frequency (row)
//697hz  ---> x_SW = 46
//770hz  ---> x_SW = 50
//852hz  ---> x_SW = 56
//941hz  ---> x_SW = 61

const unsigned char auc_frequencyL [4] = {
61,56,
50,46};


//**************************  global variables  ****************************
unsigned char x_SWa = 0x00;               // step width of high frequency
unsigned char x_SWb = 0x00;               // step width of low frequency
unsigned int  i_CurSinValA = 0;           // position freq. A in LUT (extended format)
unsigned int  i_CurSinValB = 0;           // position freq. B in LUT (extended format)
unsigned int  i_TmpSinValA;               // position freq. A in LUT (actual position)
unsigned int  i_TmpSinValB;               // position freq. B in LUT (actual position)

//**************************************************************************
// Timer overflow interrupt service routine
//**************************************************************************
void interrupt [TIMER1_OVF1_vect] ISR_T1_Overflow (void)
{ 
  // move Pointer about step width aheaed
  i_CurSinValA += x_SWa;       
  i_CurSinValB += x_SWb;
  // normalize Temp-Pointer
  i_TmpSinValA  =  (char)(((i_CurSinValA+4) >> 3)&(0x007F)); 
  i_TmpSinValB  =  (char)(((i_CurSinValB+4) >> 3)&(0x007F));
  // calculate PWM value: high frequency value + 3/4 low frequency value
  OCR1A = (auc_SinParam[i_TmpSinValA] + (auc_SinParam[i_TmpSinValB]-(auc_SinParam[i_TmpSinValB]>>2)));
}

//**************************************************************************
// Initialization
//**************************************************************************
void init (void)
{
  TIMSK  = 0x80;                     // Int T1 Overflow enabled
  TCCR1A = (1<<COM1A1)+(1<<PWM10);   // non inverting / 8Bit PWM
  TCCR1B = (1<<CS10);                // CLK/1
  DDRD   = (1 << PD5);               // PD5 (OC1A) as output
  _SEI();                     	     // Interrupts enabled
}

//**************************************************************************
// Time delay to ensure a correct setting of the pins of Port B 
//**************************************************************************
void Delay (void)
{
  int i;
  for (i = 0; i < delaycyc; i++) _NOP();
}

//**************************************************************************
// MAIN
// Read from portB (eg: using evaluation board switch) which
// tone to generate, extract mixing high frequency
// (column) and low frequency (row), and then
// fix x_SWa and x_SWb
// row    -> PINB high nibble
// column -> PINB low nibble
//**************************************************************************

void main (void)
{
  unsigned char uc_Input;
  unsigned char uc_Counter = 0;
  init();
  for(;;){ 
    // high nibble - rows
    DDRB  = 0x0F;                     // high nibble input / low nibble output
    PORTB = 0xF0;                     // high nibble pull up / low nibble low value
    uc_Counter = 0;
    Delay();                          // wait for Port B lines to be set up correctly
    uc_Input = PINB;                  // read Port B
    do 
    {
      if(!(uc_Input & 0x80))          // check if MSB is low
      {
                                      // if yes get step width and end loop
        x_SWb = auc_frequencyL[uc_Counter];  
        uc_Counter = 4;
      }
      else
      {
        x_SWb = 0;                    // no frequency modulation needed
      }
      uc_Counter++;
      uc_Input = uc_Input << 1;       // shift Bits one left
    } while ((uc_Counter < 4));
 
    // low nibble - columns
    DDRB  = 0xF0;                     // high nibble output / low nibble input
    PORTB = 0x0F;                     // high nibble low value / low nibble pull up
    uc_Counter = 0;
    Delay();                          // wait for Port B lines to be set up correctly
    uc_Input = PINB;
    uc_Input = uc_Input << 4;     
    do 
    {
      if(!(uc_Input & 0x80))          // check if MSB is low
      {
                                      // if yes get delay and end loop
        x_SWa = auc_frequencyH[uc_Counter];
        uc_Counter = 4;
      }
      else 
      {
        x_SWa = 0;                 
      }
      uc_Counter++;
      uc_Input = uc_Input << 1;
    } while (uc_Counter < 4);
  } 
}

