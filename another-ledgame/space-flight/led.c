//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
//   LED表示関数
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include "led.h"

#define CAPTURE_SW 0
#if CAPTURE_SW
	#include "sio.h"  // CAPTURE
#endif

unsigned int ledvram[LEDWIDTH];	 // LED Bitmap Buffer
char row=0;
char scanflag=0;

#define LED_PORT  PORTB	// LED用ポート
#define LED_DDR   DDRB	 // LED用ポート入出力
#define LED_CLK   0		// クロックビット番号
#define LED_DATA  2		// データビット番号
#define LED_LATCH 1		// ラッチビット番号

void led_sendword(unsigned int dat);
void led_sendword2(unsigned int dat);
void led_sendwordv(unsigned int dat);
//void led_sendbyte2(unsigned int dat);

//---------------------------------------------------------------------
// LED初期化
// ・LED用ポートを初期化します。
// ・VRAMの内容を消します。
void led_init(void)
{
	int i;

	LED_DDR |= ((1<<LED_LATCH)|(1<<LED_CLK)|(1<<LED_DATA));	//output
	LED_PORT &= ~(1<<LED_CLK);
	LED_PORT &= ~(1<<LED_LATCH);

	row=0;
	for(i=0;i<LEDWIDTH;i++)
		ledvram[i] = 0x0000;
}

//---------------------------------------------------------------------
// VRAMアドレス取得
// ・VRAMを直接書き換えたいときに使用
// 戻り値：unsigned int 型ポインタ。VRAMの先頭アドレス。
// 　　　　VRAMは[1行目][2行目]...[16行目]の順で16bitずつ格納されています。
//使用例：unsigned int *p = led_getvram();   // VRAMアドレスを取得
unsigned int * led_getvram(void)
{
	return(ledvram);
}


//---------------------------------------------------------------------
// LED消灯
// ・ロジックICすべてをLow出力させて、LEDを消します。
// ・スリープ前に使います。
// ・VRAMの内容は消えません。
void led_off(void)
{
	led_sendword(0x0000);
	led_sendword(0x0000);
	led_sendword(0x0000);

	LED_PORT |= (1<<LED_LATCH);
	LED_PORT &= ~(1<<LED_LATCH);
}

#if CAPTURE_SW
	unsigned char cap_cnt; //CAPTURE
#endif

//---------------------------------------------------------------------
// LED制御(32ビット送信)
// ・VRAM内容の1行分を表示します。
// ・16回呼び出すと1画面更新に相当します。
// ・メインループかタイマ割り込み関数内に置き、繰り返し呼び出してください。
void led_disp(void)
{
	unsigned int temp;

	temp = 0x8000 >> row;
	if(scanflag){
		led_sendword(0xFFFF);	
		led_sendword(temp);		// カソード
		//led_sendword2(ledvram[(int)row]);	//アノード
		led_sendword(~ledvram[(int)row]);	//アノード
	}else{
		led_sendword(0xFFFF);	
		led_sendwordv(temp);		// カソードRed(点灯)
		//led_sendword2(temp);	//アノード
		led_sendword(~temp);	//アノード
	}

	LED_PORT |= (1<<LED_LATCH);
	LED_PORT &= ~(1<<LED_LATCH);

#if CAPTURE_SW
	if(cap_cnt < LEDWIDTH){		  //CAPTURE
		if(cap_cnt==0){
			sio_init();
			sio_tx('@'); //CAPTURE
		}
		temp = ledvram[(int)row];	//CAPTURE
		sio_tx(temp & 0xFF);		 //CAPTURE
		sio_tx(temp >> 8);		   //CAPTURE
	}								//CAPTURE
	cap_cnt++;					   //CAPTURE
#endif
	row++;
	if(row >= LEDWIDTH){
		row=0;
		scanflag ^=1;
	}
}

//---------------------------------------------------------------------
//ピクセル描画
//引数x：X座標。0～15
//引数x：Y座標。0～15
//引数color：カラーコード。0=消灯、1=点灯（赤色）、2=XOR1
void led_pset(char x,char y,char color)
{
	unsigned int mask;

	if(x<0)return;
	if(y<0)return;
	if(x>15)return;
	if(y>15)return;

	mask=0x8000 >> x;

	if(color==1)
		ledvram[(int)y] |= mask;
	else if(color==0)
		ledvram[(int)y] &= ~mask;
	else if(color==2)
		ledvram[(int)y] ^= mask;

}

//--------------------------------------------------------------------------------
//ライン描画
//引数x1：X1座標
//引数y1：Y1座標
//引数x2：X2座標
//引数y2：Y2座標
//引数color：カラーコード。0=消灯、1=点灯（赤色）、2=XOR1
void led_line(char x1 ,char y1 ,char x2 ,char y2 ,char color)
{
	char xd;	// X2-X1座標の距離
	char yd;	// Y2-Y1座標の距離
	char xs=1;  // X方向の1pixel移動量
	char ys=1;  // Y方向の1pixel移動量
	char e;

	xd = x2 - x1;	 // X2-X1座標の距離
	if(xd < 0){
		xd = -xd;	 // X2-X1座標の絶対値
		xs = -1;	  // X方向の1pixel移動量
	}
	yd = y2 - y1;	 // Y2-Y1座標の距離
	if(yd < 0){
		yd = -yd;	 // Y2-Y1座標の絶対値
		ys = -1;	  // Y方向の1pixel移動量
	}
	led_pset (x1, y1 ,color); //ドット描画
	e = 0;
	if( yd < xd ) {
		while( x1 != x2) {
			x1 += xs;
			e += (2 * yd);
			if(e >= xd) {
				y1 += ys;
				e -= (2 * xd);
			}
			led_pset (x1, y1 ,color); //ドット描画
		}
	}else{
		while( y1 != y2) {
			y1 += ys;
			e += (2 * xd);
			if(e >= yd) {
				x1 += xs;
				e -= (2 * yd);
			}
			led_pset (x1, y1 ,color); //ドット描画
		}
	}
}

//--------------------------------------------------------------------------------
// 画面スクロール
// ・VRAMの中身を強制的にシフトします。
// 引数x1: X方向移動量。+だと左シフト(視点を右へ)、-だと右シフト(視点を左へ)
// 引数y1: Y方向移動量。+だと上シフト(視点を下へ)、-だと下シフト(視点を上へ)
void led_scroll(char x1,char y1)
{
	int i;
	if(x1!=0){
		if(x1>0){
			for(i=0; i<LEDWIDTH; i++){
				ledvram[i] <<= x1;
			}
		}else{
			for(i=0; i<LEDWIDTH; i++){
				ledvram[i] >>= (-x1);
			}
		}
	}

	if(y1!=0){
		if(y1>0){
			for(i=0; i<LEDWIDTH; i++){
				if((i+y1)>15)
					ledvram[i] = 0;
				else
					ledvram[i] = ledvram[i+y1];
			}
		}else{
			for(i=LEDWIDTH-1; i>=0; i--){
				if((i+y1)<0)
					ledvram[i] = 0;
				else
					ledvram[i] = ledvram[i+y1];
			}
		}
	}
}



//---------------------------------------------------------------------
// 上位ビットから16bit送信
void led_sendword(unsigned int dat)
{
	char bitcnt=16;

	while(bitcnt-- > 0)
	{
		if(dat & 0x8000)
			LED_PORT |=  (1<<LED_DATA);
		else
			LED_PORT &= ~(1<<LED_DATA);

		dat <<= 1;
		LED_PORT |=  (1<<LED_CLK);
		LED_PORT &= ~(1<<LED_CLK);
	}
}

//---------------------------------------------------------------------
// 下位ビットから16bit送信
void led_sendword2(unsigned int dat)
{
	char bitcnt=16;

	while(bitcnt-- > 0)
	{
		if(dat & 1)
			LED_PORT |=  (1<<LED_DATA);
		else
			LED_PORT &= ~(1<<LED_DATA);

		dat >>= 1;
		LED_PORT |=  (1<<LED_CLK);
		LED_PORT &= ~(1<<LED_CLK);
	}
}

//---------------------------------------------------------------------
// 下位ビットから8bit送信
//void led_sendbyte2(unsigned int dat)
//{
//	char bitcnt=8;
//
//	while(bitcnt-- > 0)
//	{
//		if(dat & 0x1)
//			LED_PORT |=  (1<<LED_DATA);
//		else
//			LED_PORT &= ~(1<<LED_DATA);
//
//		dat >>= 1;
//		LED_PORT |=  (1<<LED_CLK);
//		LED_PORT &= ~(1<<LED_CLK);
//	}
//}

//---------------------------------------------------------------------
// LED縦1列ずつ16ビット送信するための関数
void led_sendwordv(unsigned int bitmask)
{
	char bitcnt=16;
	int i=0;

	while(bitcnt-- > 0)
	{
		if(ledvram[i++] & bitmask)
			LED_PORT |=  (1<<LED_DATA); //LED点灯
		else
			LED_PORT &= ~(1<<LED_DATA); //LED消灯

		LED_PORT |=  (1<<LED_CLK);
		LED_PORT &= ~(1<<LED_CLK);
	}
}
