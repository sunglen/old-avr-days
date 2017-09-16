/*
  demo of glcd lib.
 */
#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "glcd.h"

#include "pic1.xbm"
#include "q.xbm"

#include "hzk.h"
PGM_P s[] PROGMEM = {
  sun_bits, ge_bits, shi_bits, bai_bits, chi_bits};

int main (void)
{
  int i;

  glcd_init_write();
  
  glcd_write_command(LCD_DISP_ON,3);

  glcd_fill(0, 3);
  
  glcd_puts_hz16x16(s, 5, 0, 0, 1, 1);
  glcd_puts_hz16x16(s, 5, 0, 40, 2, 1);
  
  for(i=0;i<80;i++)
    _delay_ms(100);

  glcd_fill(0, 3);
  
  glcd_show_xbm(pic1_bits, 32, 43, 1);

  glcd_puts_5x7("WANTED!!! Name:Sun Ge Sex: Male", 31, 0, 0, 2, 0);
  
  for(i=0;i<30;i++)
    _delay_ms(100);

  glcd_fill(0, 3);  
  glcd_puts_5x7("WHO is this man? ??Sun Ge????!!", 31, 0, 0, 1, 0);
  glcd_show_xbm(q_bits, 32, 61, 2);

  for(i=0;i<30;i++)
    _delay_ms(100);

  glcd_fill(0, 3);  
  glcd_puts_5x7("Unix is mature OS, windows is still in diapers and they smell badly.   -- Rafael Skodlar <raffi@linwin.com>", 107, 0, 0, 1, 1);

  
  for (;;) {
  }

}

