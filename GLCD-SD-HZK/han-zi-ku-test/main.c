/*
  demo of glcd lib.
 */
#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "glcd.h"

#include "pic1.xbm"
#include "q.xbm"

//hz font in xbm format
#include "hz-xbm.h"

//fonts stored in flash completely.
PGM_P s[] PROGMEM = {
  sun_bits, ge_bits, shi_bits, bai_bits, chi_bits};

const char msg[] PROGMEM = "ＳＤ卡汉字库演示程序";

int main (void)
{
  int i;

  glcd_init_write();
  
  glcd_write_command(LCD_DISP_ON,CS_BOTH);

  glcd_fill(0, CS_BOTH);
  
  glcd_puts_hz(msg, 10, 0, 0, CS_LEFT, CEMENT);

  for(i=0;i<80;i++)
    _delay_ms(100);

  glcd_fill(0, CS_BOTH);
  /*attention: it can only display 7 chinese character per page(per line),
    14 character per screen*/
  glcd_puts_hz(PSTR("◎ＳＤ字库：输出程序区的汉字☆"), 15, 0, 0, CS_LEFT, CEMENT);

  for(i=0;i<80;i++)
    _delay_ms(100);
  
  glcd_fill(0, CS_BOTH);
  glcd_puts_hz(PSTR("∥以下文字图案使用程序区字体"), 14, 0, 0, CS_LEFT, CEMENT);
  for(i=0;i<40;i++)
    _delay_ms(100);

  glcd_fill(0, CS_BOTH);
  glcd_puts_hz16x16(s, 5, 0, 0, CS_LEFT, CEMENT);
  
  for(i=0;i<40;i++)
    _delay_ms(100);
  
  glcd_fill(0, CS_BOTH);
  glcd_show_xbm(pic1_bits, 32, 43, CS_LEFT);
  glcd_puts_5x7("WANTED!!! Name:Sun Ge Sex: Male", 31, 0, 0, CS_RIGHT, NOT_CEMENT);
  
  for(i=0;i<30;i++)
    _delay_ms(100);

  glcd_fill(0, CS_BOTH);  
  glcd_puts_5x7("WHO is this man? ??Sun Ge????!!", 31, 0, 0, CS_LEFT, NOT_CEMENT);
  glcd_show_xbm(q_bits, 32, 61, CS_RIGHT);

  for(i=0;i<30;i++)
    _delay_ms(100);

  glcd_fill(0, CS_BOTH);  
  glcd_puts_5x7("Demo End", 8, 3, 0, CS_BOTH, NOT_CEMENT);

  for (;;) {
  }

  return 0;
}

