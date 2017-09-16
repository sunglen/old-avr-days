/* *******************************************************************************
   melod1.c  ATmega48用　矩形波単ｹ電子オルゴール　050320　im
　このプログラムは
　「04.12.13 tiny2313版 矩形波単ｹ電子オルゴール  koyama@cc.hirosaki-u.ac.jp」
　を一部変更して作成したものです。
　mega48を８MHzで使用します。
　このままではコンパイルできません。ｺの３曲からひとつを選んでmainに移し、
　不要分は削除してからコンパイルしてください。
***********************************************************************************/
#define  Tmp  6                // テンポ...L16=Tmp*25.6ms 　数値を小さくすると早くなる
#include <avr/io.h>
#include "melotab1.c"
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "lcd.h"

#define demodelay 0xff

void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}


uint8_t t,i;
void b(uint16_t x,uint8_t y){
  if(x>0){TCCR1A=0x40;  OCR1A=x;  }    // 一致でトグル動作　xはｹ階で決まる16ビット値
  else  TCCR1A=0x00;                  // 休符ではトグルしない
  for(t=0;t<y;t++) for(TCNT0=0;TCNT0<200;);  // .125us*1024*200*y=25.6*y ms　ｹ符の長さ
  TCCR1A=0x00;for(t=0;t<2;t++) for(TCNT0=0;TCNT0<10;); //ｹ符がつながらないように
}
int main (void){
  
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  
  lcd_puts("Hello, World!\n --mega48");
  mydelay(demodelay);
  lcd_clrscr();
  lcd_puts("CTC speaker!\n");
  mydelay(demodelay);
  lcd_clrscr();

  DDRB|=(1<<PB1);
  PORTB|=(1<<PB1);              // 全出力
  TCCR1B=9;                  // Timer1: CK/1, CTC
  TCCR0B=5;                  // Timer0: CK/1024
  for(i=1;i<100;i++){         // 2を3にすると2Jり返し　100で99Jり返し
/*　--------------　ここから　次の同じマークのTにｹ符をコピーする---------*/
//ドレミファソラシド+休符
//    b(DO,L4);b(RE,L4);b(MI,L4);b(FA,L4);b(SO,L4);b(RA,L4);b(SI, L4);b(DOH,L4);b(Rest,L2);
// select the timebase=50 us (in the oszifox), the rect wave can be seen.
b(FA,L4);

/*　--------------　ここから　上にｹ符をコピーする--------------------------*/
  }
  b(Rest,L2);
}                // プログラムの終わり　コンパイル時は次の行以ｺを削除する




/*
//ドシラソファミレド
    b(DOH,L2);b(SI,L2);b(RA,L2);b(SO,L2);b(FA,L2);b(MI,L2);b(RE,L2);b(DO,L2);
//（さらにｺの）シラソファミレド
    b(SIL,L2);b(RAL,L2);(SOL,L2);b(FAL,L2);b(MIL,L2);b(REL,L2);b(DOL,L1);

//おおきなくりの　きのしたで
    b(DO,L4);b(DO,L8);b(RE,L8);b(MI,L8);b(MI,L8);b(SO,L4);//b(Rest,L2);
    b(MI,L8);b(MI,L8);b(RE,L8);b(RE,L8);b(DO,L4);b(Rest,L4);//b(Rest,L2);
//あなたとわたし
    b(MI,L4);b(MI,L8);b(FA,L8);b(SO, L4);b(DOH,L4); //b(Rest,L2);
    b(RA, L4);b(DOH,L4);b(SO,L4);b(Rest,L4);  //b(Rest,L8);
//なかよく　あそびましょ
    b(DOH,L4);b(DOH,L4);b(SI,L4);b(SO,L4); //b(Rest,L8);
    b(RA,L8);b(RA,L8);b(RA,L8);b(RA,L8);b(SO,L4);b(Rest,L4);
//おおきなくりの　きのしたで
    b(DO,L4);b(DO,L8); b(RE,L8); b(MI,L8); b(MI,L8); b(SO,L4);
    b(MI,L8);b(MI,L8); b(RE,L8); b(RE,L8);b(DO,L4);b(Rest,L4);


//おおきなくりの　きのしたで
    b(DO,L4);b(DO,L8);b(RE,L8);b(MI,L8);b(MI,L8);b(SO,L4);//b(Rest,L2);
    b(MI,L8);b(MI,L8);b(RE,L8);b(RE,L8);b(DO,L4);b(Rest,L4);//b(Rest,L2);
//あなたとわたし
    b(MI,L4);b(MI,L8);b(FA,L8);b(SO, L4);b(DOH,L4); //b(Rest,L2);
    b(RA, L4);b(DOH,L4);b(SO,L4);b(Rest,L4);  //b(Rest,L8);
//なかよく　あそびましょ
    b(DOH,L4);b(DOH,L4);b(SI,L4);b(SO,L4); //b(Rest,L8);
    b(RA,L8);b(RA,L8);b(RA,L8);b(RA,L8);b(SO,L4);b(Rest,L4);
//おおきなくりの　きのしたで
    b(DO,L4);b(DO,L8); b(RE,L8); b(MI,L8); b(MI,L8); b(SO,L4);
    b(MI,L8);b(MI,L8); b(RE,L8); b(RE,L8);b(DO,L4);b(Rest,L4);



//あるひもりのなか
    b(Rest,L8);b(SO,L8);b(FAu,L8);b(SO,L8);b(MI,L2);
    b(Rest,L8);b(MI,L8);b(REu,L8);b(MI,L8);b(DO,L2);
//くまさんにであった
    b(Rest,L8);b(MI,L8);b(RE,L8);b(DO,L8);b(RE,L2);
    b(Rest,L8);b(SO,L8);b(RA,L8);b(SO,L8);b(MI,L2);
//はなさくもりのみち
    b(Rest,L8);b(SO,L8);b(RA,L8);b(SI,L8);b(DOH,L4);b(SO,L4);b(MI,L4);b(DO,L4);b(RA,L2);
//くまさんにであった
    b(Rest,L8);b(RA,L8);b(SI,L8);b(RA,L8);b(SO,L4);b(FA,L4);b(MI,L4);b(RE,L4);b(DO,L4);
*/
