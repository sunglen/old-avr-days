/*****************************************************************************
*  Module Name:       uIP-AVR Port - main control loop shell
*  
*  Created By:        Louis Beaudoin (www.embedded-creations.com)
*
*  Original Release:  September 21, 2002 
*
*  Module Description:  
*  This main control loop shell provides everything required for a basic uIP
*  application using the RTL8019AS NIC
*
*  September 30, 2002
*    Added support for Imagecraft Compiler
*****************************************************************************/


#include "compiler.h"
#include "uip.h"
#include "enc28j60.h"
#include "uip_arp.h"



#define BUF ((struct uip_eth_hdr *)&uip_buf[0])



/*****************************************************************************
*  Periodic Timout Functions and variables
*
*  The periodic timeout rate can be changed depeding on your application
*  Modify these functions and variables based on your AVR device and clock
*    rate
*****************************************************************************/
// poll the uIP periodic function every ~0.5/5 sec
#define TIMERCOUNTER_PERIODIC_TIMEOUT 39/5

static unsigned char timerCounter;

void initTimer(void)
{
  // timer overflows every 13.1ms (with 20MHz clock)
  TCCR0A=0;  // timer0 prescale 1/1024 (5)
  TCCR0B=5;
  // interrupt on overflow
  TCNT0=0;
  sbi( TIMSK0, TOIE0 ) ;
	
  timerCounter = 0;
}


SIGNAL(SIG_OVERFLOW0){
  timerCounter++;
}

unsigned int
tapdev_read(void){
	u16 len;
	len=enc28j60PacketReceive(UIP_BUFSIZE, (u8 *)uip_buf);
	return len;
}

void
tapdev_send(void){
	if(uip_len<=40+UIP_LLH_LEN){
		enc28j60PacketSend(uip_len, (u8 *)uip_buf);
	}
	else{
		enc28j60PacketSend2(54, (u8 *)uip_buf, uip_len-40-UIP_LLH_LEN, (u8 *)uip_appdata);
	}
}  

/*****************************************************************************
*  Main Control Loop
*
*  
*****************************************************************************/
int main(void)
{
  unsigned char i;

  // init device driver
  enc28j60Init();

  // init uIP
  uip_init();

  // init app
  example1_init();

  // init ARP cache
  uip_arp_init();

  // init periodic timer
  initTimer();
  
  sei();

  while(1)
  {

    // look for a packet
    uip_len = tapdev_read();
    if(uip_len == 0)
    {
      // if timed out, call periodic function for each connection
      if(timerCounter > TIMERCOUNTER_PERIODIC_TIMEOUT)
      {
        timerCounter = 0;
        
        for(i = 0; i < UIP_CONNS; i++)
        {
          uip_periodic(i);
		
          // transmit a packet, if one is ready
          if(uip_len > 0)
          {
            uip_arp_out();
            tapdev_send();
        }
      }

      /* Call the ARP timer function every 10 seconds. */
      /* not tested yet
      if(++arptimer == 20)
      {	
        uip_arp_timer();
        arptimer = 0;
      }*/

      }
    }
    else  // packet received
    {
      // process an IP packet
      if(BUF->type == htons(UIP_ETHTYPE_IP))
      {
        // add the source to the ARP cache
        // also correctly set the ethernet packet length before processing
        uip_arp_ipin();
        uip_input();

        // transmit a packet, if one is ready
        if(uip_len > 0)
        {
          uip_arp_out();
          tapdev_send();
        }
      }
      // process an ARP packet
      else if(BUF->type == htons(UIP_ETHTYPE_ARP))
      {
        uip_arp_arpin();

        // transmit a packet, if one is ready
        if(uip_len > 0)
          tapdev_send();
      }
    }
  }

	return 1;
}
