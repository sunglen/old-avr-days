/* Rolling Ball: demo for LED GAME.
   
   Hardware Connection (ADC part): 

   MEGA88,24(ADC1)<--Yout(ADXL311)<-->0.1uF(104)<-->GND
   MEGA88,23(ADC0)<--Xout(ADXL311)<-->0.1uF(104)<-->GND
   MEGA88,22(GND)<-->GND
   MEGA88,21(AREF)<-->0.1uF(104)<-->GND
   MEGA88,20(AVCC)-->VCC
   ST(ADXL311)<-->JUMPER<-->VCC
   COM(ADXL311)<-->GND
   VDD(ADXL311)<-->VCC

   ADXL311 calibration:

   When VCC was 4.66V, Xout_mv varied from
   2000mV(-1g), 2275mV(0g), to 2550mV(1g).
   Yout_mv varied from
   2130mV(-1g), 2375mV(0g), to 2650mV(1g).
   (all voltage are measured approximately.)

   The ADC result (analog_result) varies:
   X: 439-447(-1g), 496-506(0g), 557-567(1g)
   Y: 467-475(-1g), 525-534(0g), 587-594(1g)
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "led.h"
#include "ledx.h"
#include "rand.h"
#include "sw.h"
//#include "beep.h" 

/*use a 6V DC adapter and 78L05, a 4.66V VCC was generated.*/
/*
#define AVCC_mv 4660
#define Vref_mv AVCC_mv
*/

/* ADC values when 0g is applied.(approximate)
   these values should be adjust manually to find the best.
*/
//#define ADCX_0G 501
//#define ADCY_0G 530
#define ADCX_0G 518
#define ADCY_0G 542

// position divider, increase it to slow down the ball.
#define DIV 400

// delta T, increase it to speed up the ball, vice-versa.
#define T 2
// TT is the square of T.
#define TT 4

//store ADC result.
volatile uint16_t analog_result;

//flag to identify ADC status.
volatile uint8_t analog_busy;

//ISR for ADC
ISR (ADC_vect)
{
  uint8_t adlow, adhigh;

  adlow = ADCL;
  adhigh = ADCH;
  analog_result = ((adhigh<<8)|adlow);
  analog_busy = 0;
}

//Initialize ADC for single convertion.
void adc_init_single(void)
{
  analog_busy = 1;
  
  /*adc control: prescaler, clear ADIF, interrupt enable, etc*/
  /* ADC prescaler: (if f_osc is 8MHz)
     f_adc=f_osc/128=8e6/128 ~= 60KHz
  */
  ADCSRA = ((1<<ADEN)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));

  /*enable interrupt*/
  sei();
}

void led_print(char *p)
{
        while(*p != 0){
                led_putch(*p++);
        }
}

// メイン処理
int main(void)
{
  // position for caculation
  int x, y;
  // position for display
  int dx, dy;
  // velocity
  int vx, vy=0;
  // acceleration
  int ax, ay;

  unsigned int i;
  unsigned int *p;

  adc_init_single(); 
  sw_init(SW_B); // initialize Switch B (S2)
  rand_init(); // random number initialization
  led_init(); // LED初期化
  
  led_locate(0,0);
  led_print("ROLL");
  led_locate(0,8);
  led_print("BALL");
  
  while(!sw_get(SW_B))
     led_disp();

  x=(rand_get(LEDWIDTH)+1)*DIV;
  y=(rand_get(LEDWIDTH)+1)*DIV;
  p=led_getvram();

  while(1)
  {
     led_init();

     //X axis
     analog_busy=1;    
     /*select channel 0 to read Xout and use Aref=Avcc*/
     ADMUX = ((1<<REFS0)|0x00);
      
     /* start convertion*/
     ADCSRA |= (1<<ADSC);
     /*discard the first ADC result.*/
     while(analog_busy);
     
     analog_busy=1;    
     /* start convertion*/
     ADCSRA |= (1<<ADSC);

     while(analog_busy);

     /* get relative acceleration value */
     ax=analog_result-ADCX_0G;
     /* update x position */
     x=x+vx*T+(ax*TT/2);
     /* update x direction velocity value */
     vx=vx+ax*T;
     /* get suitable value for display */
     dx=x/DIV;

     if(dx>LEDWIDTH){ //stop on the top.
        vx=0;
        dx=LEDWIDTH;
        x=dx*DIV;
     }else if(dx<1){  //stop at the bottom.
        vx=0;
        dx=1;
        x=dx*DIV;
     }
     
     // Yout
     analog_busy=1;
     /*select channel 1 to read Yout and use Aref=Avcc*/
     ADMUX = ((1<<REFS0)|0x01);
           
     /* start convertion*/
     ADCSRA |= (1<<ADSC);
     /*discard the first ADC result.*/
     while(analog_busy);

     analog_busy=1;    
     /* start convertion*/
     ADCSRA |= (1<<ADSC);

     while(analog_busy);

     /* get relative acceleration value */
     ay=analog_result-ADCY_0G;
     /* update y position */
     y=y+vy*T+(ay*TT/2);
     /* update y direction velocity value */
     vy=vy+ay*T;

     /* get suitable value for display */
     dy=y/DIV;

     if(dy>LEDWIDTH){ //stop on the top.
        vy=0;
        dy=LEDWIDTH;
        y=dy*DIV;
     }else if(dy<1){ //stop at the bottom.
        vy=0;
        dy=1;
        y=dy*DIV;
     }
     
     //locate the ball.
     p[LEDWIDTH-dx]=(1<<(dy-1));
     
     sei();  
     for(i=0;i<200;i++)
        led_disp();  //show the ball
  }

return 0;
}
