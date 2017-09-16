//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   LED表示関数 追加機能
// ・この関数を使うためには"sfont.h"が必要です。
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "led.h"
#include "ledx.h"
#include "sfont.h"	//4x6スモールフォント


char text_x=0;  // printf用カーソル位置
char text_y=0;


//---------------------------------------------------------------------
// カーソル位置設定
// 引数x：X座標0～15。13以上だと自動改行。
// 引数y：Y座標0～15。11以上だと下へ画面スクロールします。
void led_locate(char tx,char ty)
{
	text_x = tx;
	text_y = ty;
}

//---------------------------------------------------------------------
// 1キャラクタをVRAM転送
// ・4x6スモールフォント用
// 引数ch：キャラクターコード（0x00-0xff）
// 戻り値：0を返します。
int led_putch(char ch)
{
	char tx;
	unsigned char i;
	unsigned char bitdata;
	unsigned int *pVram;
	PGM_P p;

	if((ch==10)||(text_x > (LEDWIDTH-4))){
		text_x = 0;
		text_y += 6;
	}
	if(text_y > (LEDWIDTH-6)){
		led_scroll(0,text_y-(LEDWIDTH-6));
		text_y=LEDWIDTH-6;
	}
	if((unsigned char)ch < 0x20)return 0;
	pVram = led_getvram();
	pVram += text_y;

	tx = (LEDWIDTH-4)-text_x;
	p = (PGM_P)smallfont;
	p += ((int)((unsigned char)ch - 0x20) * 3);

	for(i=0 ;i<6 ;i++) {
		bitdata = pgm_read_byte(p);
		if((i % 2)==0){
			bitdata >>= 4;
		}else{
			p++;
		}
		bitdata &= 0xf;
		*pVram &= ~(0xf << tx);
		*pVram |= ((int)bitdata << tx);
		pVram++;
	}
	text_x += 4;    // カーソル移動
	if(((unsigned char)ch>=0xB0)&&((unsigned char)ch<=0xDC))
		text_x += 1;    // カーソル移動(カタカナ用)

	return 0;
}


