/*****************************************************************************
*  "A Very Simple Application" from the uIP 0.6 documentation
*****************************************************************************/

#include "app.h"


void example1_init(void)
{
	uip_listen(80);
}


void example1_app(void)
{
	if(uip_newdata() || uip_rexmit())
	{
		uip_send("ok\n", 3);
	}
}
