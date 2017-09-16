#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#define F_OSC 14318180UL

//128:prescaler 13:adc trans every 13 crycles.
//approx.=8604
#define F_SAMPLE ((F_OSC)/128/13)

//ADC save 221 samples
//205/8000=221/8604
#define N_SAMPLES 221

const uint16_t f[8]={697,770,852,941,1209,1336,1477,1633};

//uint16_t x[N_SAMPLES];
float x[N_SAMPLES];

int main(void)
{
  int i;
  float k[8];
  uint16_t D0=0, D1=0, D2=0;
  uint32_t P;

  for(i=0;i<8;i++)
    k[i]=2*cosf(2*M_PI*((float)f[i]/F_SAMPLE));
  
  /*
  printf("%f\n", (float)F_SAMPLE);
    
  for(i=0;i<8;i++)
    printf("%f\n", k[i]);
  */
  for(i=0;i<N_SAMPLES;i++)
    {
      x[i]=(i*4.0/1024.0);
    }
  
  for(i=0;i<N_SAMPLES;i++)
    {
      D0=x[i]+k[0]*D1+D2;
      D2=D1;
      D1=D0;
      printf("%d %d %d\n",D0,D1,D2);
    }
  
  P=D1*D1+D2*D2-k[0]*D1*D2;
  
  printf("%d\n",P);
}
