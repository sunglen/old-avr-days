//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
//   デモ
//
//  ・PROG 1でテキスト表示
//  ・PROG 2でライン表示(線の数は1〜5本の乱数です)
//  ・PROG 3でドラゴン花火
//  ・PROG 4でドレミを再生します（スピーカが必要）
//  ・PROG 5でシリアル送信(9600bps、AVRライタでPCと接続)
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include "led.h"
#include "ledx.h"
#include "rand.h"
#include "sio.h"
#include "sw.h"
#include "beep.h" 


void led_printd(int x);
void led_print(char *p);

//---------------------------------------------------------------------
void led_printd(int x)
{
	unsigned char temp;
	int ketaval=1000;

	while (ketaval != 0) {
		temp = x / ketaval;
		led_putch('0'+temp);
		x -= (ketaval*temp);
		ketaval /= 10;
	}
}

//---------------------------------------------------------------------
void led_print(char *p)
{
	while(*p != 0){
		led_putch(*p++);
	}
}

//---------------------------------------------------------------------
void chara(void)
{
	#define CHANGECNT 250 // ループ回数

	unsigned char cnt;
	unsigned char ch;
	char dotcnt;

	dotcnt=0;
	ch = 0x20;

	while(1)
	{
		led_disp();

		cnt++;
		if(cnt > CHANGECNT) {
			cnt = 0;
			if(sw_get(SW_UP)){		// U スイッチ
				led_scroll(0,-1);
			}
			if(sw_get(SW_DOWN)){	// D スイッチ
				led_scroll(0,1);
			}
			if (sw_get(SW_B)) break;		// Bボタン

			led_scroll(1,0);
			dotcnt++;
			if(dotcnt>=5){
				dotcnt=0;

				led_locate(12,4);
				led_putch(ch);
				ch++;
				if(ch == 0x7F) ch=0xA1;
				if(ch == 0xE0) ch=0x20;
			}
		}
	}
}


//---------------------------------------------------------------------
void linedemo(void)
{
	typedef struct {
	   char x1;
	   char y1;
	   char x2;
	   char y2;
	} ST_TAMA;   //南京玉すだれ用構造体

	#define TMAX 5
	ST_TAMA tama[TMAX];

	#define CHGCNT 250 // 座標更新周期
	#define MOVEPIX 3

	unsigned char  cnt;
	int i;
	ST_TAMA *ptama;
	char x1a;	//移動量
	char y1a;	//移動量
	char x2a;	//移動量
	char y2a;	//移動量
	int t_max;

	t_max = rand_get(TMAX);
	ptama = &tama[0];

	for(i=0 ;i<=t_max ;i++) {
		ptama[i].x1 =8;
		ptama[i].y1 =8;
		ptama[i].x2 =8;
		ptama[i].y2 =8;
	}
	x1a = 1;
	y1a = 1;
	x2a = 1;
	y2a = 1;

	while(1)
	{
		led_disp();

		cnt++;
		if(cnt > CHGCNT){
			cnt = 0;

			ptama = &tama[t_max];
			led_line(ptama->x1, ptama->y1, ptama->x2, ptama->y2,0);

			for(i=t_max ;i>0 ;i--) {
				tama[i].x1 = tama[i-1].x1;
				tama[i].y1 = tama[i-1].y1;
				tama[i].x2 = tama[i-1].x2;
				tama[i].y2 = tama[i-1].y2;
			}

			//------バウンド---
			ptama = &tama[0];
			if(ptama->x1 <= 0)
				x1a = rand_get(MOVEPIX)+1;
			else if(ptama->x1 >= 15)
				x1a = -rand_get(MOVEPIX)-1;
		
			if(ptama->y1 <= 0)
				y1a = rand_get(MOVEPIX)+1;
			else if(ptama->y1 >= 15)
				y1a = -rand_get(MOVEPIX)-1;
	   
			if(ptama->x2 <= 0)
				x2a = rand_get(MOVEPIX)+1;
			else if(ptama->x2 >= 15)
				x2a = -rand_get(MOVEPIX)-1;
		
			if(ptama->y2 <= 0)
				y2a = rand_get(MOVEPIX)+1;
			else if(ptama->y2 >= 15)
				y2a = -rand_get(MOVEPIX)-1;

			ptama->x1 += x1a;
			ptama->y1 += y1a;
			ptama->x2 += x2a;
			ptama->y2 += y2a;

			led_line(ptama->x1, ptama->y1, ptama->x2, ptama->y2,1);

			if (sw_get(SW_B)) break;	// スイッチ
		}
	}
}


//---------------------------------------------------------------------
void dragon(void)
{
	typedef struct {
		int x;	// X座標(実際の8倍で計算します)
		int y;	// Y座標(実際の8倍で計算します)
		char x1;	// X移動量
		char y1;	// Y移動量
	} ST_DOT;   // DOT構造体

	#define DOTMAX 14	// DOT最大数
	#define CHGCNT2 100 // 座標更新周期
	#define SPEEDMAX 50
	#define DOT_NONE 999

	ST_DOT dots[DOTMAX];
	unsigned char  cnt;
	int i;
	char powerx,powery;


	for(i=0;i<DOTMAX;i++){
		dots[i].x = DOT_NONE;
	}

	powerx=4;
	powery=15;

	while(1)
	{
		led_disp();
		cnt++;
		if(cnt >= CHGCNT2) {
			cnt = 0;
			if (sw_get(SW_UP)) {			// Uボタン
				if(powery < 19) powery++;
			}else if (sw_get(SW_DOWN)) {	// Dボタン
				if(powery > 3) powery--;
			}else if (sw_get(SW_RIGHT)) {	// Rボタン
				if(powerx < 10) powerx++;
			}else if (sw_get(SW_LEFT)) {	// Lボタン
				if(powerx > 1) powerx--;
			}else if (sw_get(SW_B)) {		// Bボタン
				break;
			}

			for(i=0; i<DOTMAX; i++){
				if(dots[i].x == DOT_NONE){
					dots[i].x = 8<<3;
					dots[i].y = 15<<3;
					dots[i].x1 = -rand_get(powerx)+(powerx/2);
					dots[i].y1 = -rand_get(powery);
					break;
				}
			}
			for(i=0;i<DOTMAX;i++){
				if(dots[i].x == DOT_NONE)continue;

				led_pset (dots[i].x >> 3, dots[i].y >> 3, 0); //ドット描画
				dots[i].x += dots[i].x1;
				dots[i].y += dots[i].y1;
				dots[i].y1 ++;
				if(dots[i].y > (15<<3)){
					if(dots[i].y1 < 10){
						dots[i].x=DOT_NONE;
					}else{
						dots[i].y = 15<<3;
						dots[i].y1 = -(dots[i].y1 / 2); //バウンド
					}
				}else{
					led_pset (dots[i].x >>3, dots[i].y >>3, 1); //ドット描画
				}
			}
		}
	}
}

//---------------------------------------------------------------------
void beep_test(void)
{
	int i;
	int hztable[]={
		262,
		294,
		330,
		349,
		392,
		440,
		494,
		523
	};	//周波数テーブル

						// サウンド機能初期化
	beep_init();   // サウンド出力可能な最高周波数

	for(i=0; i<(sizeof(hztable)/2); i++){
		led_locate(0,0);		// カーソル位置設定
		led_printd(hztable[i]);	// 
		led_print("  Hz");
		beep_set(hztable[i],6);

		while(beep_getcnt()){
			led_disp();		// LED画面更新
		}
	}
}

//---------------------------------------------------------------------
void sio_test(void)
{
	sio_txstr("SO TEST");
	sio_tx(13);
	sio_tx(10);
}

//---------------------------------------------------------------------
// メイン処理
int main(void)
{
	#define SWCHKCNT 600   // スイッチ検出ループ回数

	unsigned int keycnt;
	char menu;

	sw_init(SW_ALL);
	rand_init();
	sio_init();

	menu=1;
	while(1)
	{
		led_init();				// LED初期化
		led_locate(0,0);		// カーソル位置設定
		led_print("PROG");		// 

		sei();		  
		while(1){
			led_disp();		// LED画面更新

			keycnt++;
			if(keycnt >= SWCHKCNT) {
				keycnt = 0;
				led_locate(6,8);		// カーソル位置設定
				led_putch('0'+menu);

				if(sw_get(SW_UP)|sw_get(SW_RIGHT)){		// U or Rスイッチ
					if(menu < 5) menu++;
				}
				if(sw_get(SW_DOWN)|sw_get(SW_LEFT)){	// D or Lスイッチ
					if(menu > 1) menu--;
				}
				if(sw_get(SW_A)){	// Aスイッチ
					led_init();			// LED初期化
					switch(menu){
					case 1:
						chara();
						break;
					case 2:
						linedemo();
						break;
					case 3:
						dragon();
						break;
					case 4:
						beep_test();	// ドレミを再生
						break;
					case 5:
						sio_test();
						break;
					}
					break;
				}
			}
		}
	}
}


