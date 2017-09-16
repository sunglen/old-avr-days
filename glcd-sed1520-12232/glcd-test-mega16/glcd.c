/*
glcd.c:
see glcd.h for details.
*/

#include "glcd.h"

void gen_cl(uint16_t n)
{
  DDR_OC1A |= (1<<PIN_OC1A);
  
  TCCR1A=(1<<COM1A0); /* toggle the OC1A output line */

  /* CTC mode, prescaler(N)=1, f_clk_IO/N = 4MHz/1 = 4MHz*/
  TCCR1B=((1<<WGM12)|(1<<CS10));

  //must write high byte first, otherwise ctc will run incorrect.
  OCR1AH=n>>8;  
  OCR1AL=n&0xff;
}

void glcd_init_write(void)
{
  gen_cl(_2KHz);
  LCDDATADDR = 0xff;
  LCDCTRLDDR |= ((1<<LCDE1PIN)|(1<<LCDE2PIN)|(1<<LCDCMDPIN)
		 |(1<<PIN_E)|(1<<PIN_RW));
}

void chip_act(uint8_t cs, uint8_t rw)
{
  if ( cs & 0x01 ) 
    LCD_ENABLE_E1();

  if ( cs & 0x02 )
    LCD_ENABLE_E2();
  
  if (rw==0)
    LCDCTRLPORT &= ~(1<<PIN_RW);
  else
    LCDCTRLPORT |= (1<<PIN_RW);

  LCDCTRLPORT |= (1<<PIN_E);
  LCDCTRLPORT &= ~(1<<PIN_E);
	
  /*stand by*/
  LCD_DISABLE_E1();
  LCD_DISABLE_E2();
}

void glcd_write_command(uint8_t cmd, uint8_t cs)
{
  LCD_CMD_MODE();
  LCDDATAPORT = cmd;
  chip_act(cs,0);
}

void glcd_write_data(uint8_t data, uint8_t cs)
{
  LCD_DATA_MODE();
  LCDDATAPORT = data;
  chip_act(cs,0);
}

void glcd_fill(uint8_t c, uint8_t cs)
{
  uint8_t i,j;
  for(i=0;i<=MAX_PAGE;i++){
    for(j=0;j<=MAX_COLUMN;j++){

      /* pages */
      glcd_write_command(LCD_SET_PAGE|i, cs);
  
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|j, cs);

      /*clear to white*/
      glcd_write_data(c, cs);
    }
  }
}

void glcd_show_xbm(PGM_P pic, uint8_t width, uint8_t  height, uint8_t cs)
{
  uint8_t i,j,k;
  uint8_t old_byte, new_byte;
  
  for(i=0;i<=(width-1)/8;i++){
    for(j=0;j<=height-1;j++){
      /* pages */
      glcd_write_command(LCD_SET_PAGE|i, cs);
  
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|j, cs);

      /*
	the byte structure of xbm file is:
	picture pix: 1st_bit| 2nd_bit| ...| 7th_bit
	xbm byte: bit0(LSB)| bit1 |...|bit7(MSB)
	so, I should convert xbm byte:
	new byte <-- old byte
	bit0 <-- bit7
	bit1 <-- bit6
	.
	.
	.
	bit7 <-- bit0
       */
      
      old_byte=pgm_read_byte(pic+(j*4+3-i));
      new_byte = 0;
      for(k=0;k<=7;k++)
	new_byte |= ((old_byte>>k)&0x01)<<(7-k);

      glcd_write_data(new_byte, cs);
    }
  }
}


uint8_t glcd_putc_5x7(unsigned char c, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement)
{
  uint8_t i, temp;
  
  if(col+4 > MAX_COLUMN)
    {
      if(cs==1 && cement){
	/* a problem should be solved here! */
	return glcd_putc_5x7(c, page, SCRN_LEFT, 2, 1);
	
      }else if(cs==2 && cement){
	return glcd_putc_5x7(c, page+1, SCRN_LEFT, 1, 1);
      }else{
	page++;
	col = SCRN_LEFT;
      }
    }
     
  if(page > MAX_PAGE)
    page = SCRN_TOP;

  /* pages */
  glcd_write_command(LCD_SET_PAGE|page, cs);
  
  /* write ascii character */
  for(i=col; i<=col+4; i++)
    {
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|i, cs);
      
      temp = pgm_read_byte(Font5x7 + (c-0x20)*5 + (i-col));
      
      glcd_write_data(temp, cs);
    }
  
  current_cs=cs;
  current_page=page;
  return i;
  
}


uint8_t glcd_puts_5x7(unsigned char *s, int len, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement)
{
  int i;
  uint8_t current_col;
  
  for(i=0;i<len;i++){
    current_col=glcd_putc_5x7(s[i], page, col, cs, cement);
    col = current_col+1;
    page = current_page;
    cs=current_cs;
  }
  
  return current_col;
}

uint8_t glcd_put_hzxbm_16x16(PGM_P hz, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement)
{
  uint8_t i,j;
  uint8_t old_byte, new_byte;
  uint8_t current_col, cs_old;
  
  if(col+15 > MAX_COLUMN)
    {
      if(cs==2 && cement){
	//hz16x16 need 2 pages.
	return glcd_put_hzxbm_16x16(hz, page+2, SCRN_LEFT, 1, 1);
      }else if(!cement){
	page=page+2;
	col = SCRN_LEFT;
      }
    }
     
  if(page > MAX_PAGE-1)
    page = SCRN_TOP;

  /* save cs*/
  cs_old=cs;
  /* write byte no.=[1:2:31] to RAM */
  /*set pages*/
  glcd_write_command(LCD_SET_PAGE|page, cs);    
  for(i=col;i<col+16;i++)
    {
      /*get byte from hz xbm*/
      old_byte=pgm_read_byte(hz+((i-col)*2+1));
      /*exchange bit for xbm display*/
      new_byte = 0;
      for(j=0;j<=7;j++)
	new_byte |= ((old_byte>>j)&0x01)<<(7-j);

      current_col=i;
      // character cross chip1 and chip2 controlled lcd.
      if(current_col>MAX_COLUMN)
	{
	  cs = 2;
	  current_col=i-(MAX_COLUMN+1);
	  glcd_write_command(LCD_SET_PAGE|page, cs);    
	}

      glcd_write_command(LCD_SET_COL|current_col, cs);
      glcd_write_data(new_byte, cs);
    }

  /*recover saved cs*/
  cs=cs_old;
  /*write byte no.=[0:2:30] to RAM*/
  glcd_write_command(LCD_SET_PAGE|(page+1), cs);
  for(i=col;i<col+16;i++)
    {
      /*get byte from hz xbm*/
      old_byte=pgm_read_byte(hz+((i-col)*2));
      /*exchange bit for xbm display*/
      new_byte = 0;
      for(j=0;j<=7;j++)
	new_byte |= ((old_byte>>j)&0x01)<<(7-j);
      
      current_col=i;
      // character cross chip1 and chip2 controlled lcd.
      if(current_col>MAX_COLUMN)
	{
	  cs = 2;
	  current_col=i-(MAX_COLUMN+1);
	  glcd_write_command(LCD_SET_PAGE|(page+1), cs);
	}

      glcd_write_command(LCD_SET_COL|current_col, cs);
      glcd_write_data(new_byte, cs);      
    }

  current_cs=cs;
  current_page=page;
  return current_col+1;
}


uint8_t glcd_puts_hz16x16(PGM_P *s, int len, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement)
{
  int i;
  uint8_t current_col;
  
  for(i=0;i<len;i++){
    current_col=glcd_put_hzxbm_16x16(pgm_read_byte(s+i), page, col, cs, cement);
    col = current_col;
    page = current_page;
    cs=current_cs;
  }
  
  return current_col;
}
