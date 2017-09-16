//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
// アルケー
//・ベーマガ91年3月号の「アルケー」（遠藤克之さん作）を移植しました。
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
#include "sleeping.h" 

#define SCROLLSPEED 1

unsigned int *pVram;

unsigned long vmap;


#define ANIMEMAX 4
#define SPSIZE 5

unsigned char pat_my[ANIMEMAX][SPSIZE] = {
	{
		((1<<2)|0),
		((1<<3)|(1<<2)|(1<<1)|1),
		((1<<2)|0),
		((1<<4)|(1<<3)|(1<<2)|(1<<1)|0),
		((1<<1)|0)
	},{
		((1<<2)|0),
		((1<<3)|(1<<2)|(1<<1)|0),
		((1<<4)|(1<<2)|1),
		((1<<3)|(1<<2)|0),
		((1<<3)|0)
	},{
		((1<<3)|0),
		((1<<4)|(1<<3)|(1<<2)|0),
		((1<<3)|0),
		((1<<4)|(1<<3)|(1<<2)|(1<<1)|0),
		((1<<4)|0)
	},{
		((1<<2)|0),
		((1<<4)|(1<<3)|(1<<2)|0),
		((1<<2)|(1<<1)|0),
		((1<<3)|(1<<2)|0),
		((1<<2)|0)
	}
};


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
// 5x5pixelのキャラクターを表示
//引数 x:X座標
//引数 y:Y座標
//引数 *dat:パターン。char型ポインタ
void led_spput(char x,char y, char *dat)
{
	char i;
	if(x==-1)return;

	for(i=0;i<SPSIZE;i++){
		if(y<0)continue;
		if(y>15)break;

		*(pVram+y  ) |= (((unsigned int)*dat << (16-SPSIZE)) >> x);
		y++;
		dat++;
	}
}

//---------------------------------------------------------------------
// 5x5pixelのキャラクターを消去
//引数 x:X座標
//引数 y:Y座標
//void led_spclr(char x,char y)
//{
//	char i;
//	if(x==-1)return;
//
//	for(i=0;i<SPSIZE;i++){
//		*(pVram+y  ) &= ~((((1<<4)|(1<<3)|(1<<2)|(1<<1)|1) << (16-SPSIZE)) >> x);
//		y++;
//	}
//}


//---------------------------------------------------------------------
// メイン処理
int main(void)
{
	#define JUMPMAX 5
	#define SWCHKCNT 300   // スイッチ検出ループ回数
	int i;

	char x,y;
	int animecnt;
	int score;
	unsigned int keycnt;
	char scroll;
	int jump;

	sw_init(SW_ALL);
	rand_init();
	beep_init();
	sio_init();
	led_init();				// LED初期化
	pVram = led_getvram();

	sei();
	while(1){
		animecnt=0;
		jump=JUMPMAX;
		x = 1;
		y = 15-SPSIZE;

		keycnt = 0;

		led_init();
		led_spput(x,y, &pat_my[animecnt][0]);
		led_locate(3,1);
		led_print("GO!");
		led_line(0,15,15,15,1);
		beep_set(600,7);

		for(i=0;i<10000;i++){
			led_disp();		// LED画面更新
		}
		vmap = 0xFFFFFFFF;

		score=0;
		while(1){
			led_disp();		// LED画面更新

			keycnt++;
			if(keycnt < SWCHKCNT) continue;
			keycnt = 0;

			//---------
			scroll++;
			if(scroll >= SCROLLSPEED)
			{
				scroll=0;

				score++;

				vmap <<= 1;
				vmap |= 1;

				if(rand_get(10)>8){
					vmap &= ~((unsigned long)((1<<2)|(1<<1)|1)); //床に穴あけ
				}
			}

			animecnt++;
			if(animecnt>=ANIMEMAX) animecnt=0;

			for(i=0;i<15;i++){
				*(pVram+i) = 0;
			}
			*(pVram+15) = (unsigned int)(vmap >> 16);

			if (sw_get(SW_RIGHT)) {	// Rボタン
				if(x < (16-SPSIZE)) x++;
			}else if (sw_get(SW_LEFT)) {	// Lボタン
				if(x > 0) x--;
			}else if (sw_get(SW_B)) {		// Bボタン
				sleeping();
			}
			led_spput(x,y, &pat_my[animecnt][0]);

			if(jump >= JUMPMAX){
				if (sw_get(SW_A)||sw_get(SW_UP)) {	// U| Aボタン
					beep_set(400,1);
					jump=-JUMPMAX+1;
				}else{
					if((vmap & (((unsigned long)((1<<3)|(1<<2)|(1<<1)|0) << (32-SPSIZE)) >> x))==0)
						break;
				}
			}else{
				y += jump;
				jump++;
			}
		}

		beep_set(150,15);

		for(y=15-SPSIZE;y<16;y++){
			for(i=0;i<15;i++){
				*(pVram+i) = 0;
			}
			*(pVram+15) = (unsigned int)(vmap>>16);

			led_spput(x,y, &pat_my[animecnt][0]);
			led_locate(0,2);
			led_printd(score);

			for(i=0;i<2000;i++){
				led_disp();		// LED画面更新
			}
		}
		sleeping();
	}
}


