//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   Space Fight
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "led.h"
#include "ledx.h"
#include "rand.h"
#include "sio.h"
#include "sw.h"
#include "beep.h"
#include "sleeping.h"

#define TEKIMAX 6
char tx[TEKIMAX];	//敵座標
char ty[TEKIMAX];
char ttime;	//Enemy Timing
char tmove;	//Enemy Moving
char speed;	//game speed
int score;	//Score
char tnum;	//Enemy Count
char x,y;	//My Ship X,Y
char bx,by;	//My Ship Beam
char cx,cy;	//Enemy Beam
unsigned int *pVram;
int shield;


void led_printd(int x);
void led_print(char *p);



//---------------------------------------------------------------------
// 10進数4桁表示
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

#define SHIELD_Y 12
//---------------------------------------------------------------------
// 文字列表示
void led_print(char *p)
{
	while(*p != 0){
		led_putch(*p++);
	}
}

//---------------------------------------------------------------------
// 3x2pixelのキャラクターを表示
void led_spput(char x,char y, char dat1, char dat2)
{
	*(pVram+y  ) |= (((unsigned int)dat1 << 13) >> x);
	*(pVram+y+1) |= (((unsigned int)dat2 << 13) >> x);
}

//---------------------------------------------------------------------
// 3x2pixelのキャラクターを消去
void led_spclr(char x,char y)
{
	*(pVram+y  ) &= ~(((1<<15)|(1<<14)|(1<<13)|0) >> x);
	*(pVram+y+1) &= ~(((1<<15)|(1<<14)|(1<<13)|0) >> x);
}

//--------------------------------
// シールド表示
void shield_put(void)
{
	*(pVram+SHIELD_Y) = shield;
}

//------------------------Enemy init
void teki_init()
{
	int i;

	for(i=0;i<TEKIMAX;i++){
		tx[i] = (i%3)*4;
		ty[i] = (i/3)*4;
	}
	shield = ((1<<14)|(1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<7)|(1<<4)|(1<<3)|(1<<2)|0);
	shield_put();

	tmove=0;
	ttime=0;
	cx=-1;
	cy=-1;
	tnum=TEKIMAX;
}

//------------------------敵の移動
void teki_move()
{
	#define MOVEPITCH 40	//移動カウント
	#define MOVESEQ   12	//移動シーケンス
	int i;

	//--------------------Enemy Beam
	if(cy != -1){
		led_pset(cx,cy,0);
		cy ++;
		if(cy > LEDYMAX){
			cy = -1;
		}
		if((cy==SHIELD_Y)&&(shield &(0x8000 >> cx))){
			shield &= ~(0x8000 >> cx);
			shield_put();
			cy = -1;
		}
		led_pset(cx,cy,1);
	}

	ttime++;
	if (ttime > speed) {
		ttime=0;

		for(i=0 ;i<TEKIMAX ;i++) {
			if(ty[i] == -1) continue;

			led_spclr(tx[i],ty[i]);

			if(tmove < 5){
				tx[i]++;
			}else if(tmove < 6){
				ty[i]++;
			}else if(tmove < 11){
				tx[i]--;
			}else{
				ty[i]++;
			}
			if(10 < ty[i]){
				ty[i]=10;
			}
			if(cy == -1){
				if(rand_get(3)==0 ){
					cx = tx[i]+1;
					cy = ty[i];
				}
			}
		}
		tmove++;
		if(tmove >= MOVESEQ)tmove=0;
	}

	for(i=0 ;i<TEKIMAX ;i++) {
		if(ty[i] == -1) continue;

		if(tmove & 1) {
			led_spput(tx[i],ty[i] ,((1<<2)|1),((1<<1)|0));
		}else{
			led_spput(tx[i],ty[i] ,((1<<1)|0),((1<<2)|1));
		}
	}
}

//---------------------------------
void tama_move(void)
{
	int i;

	if(by != -1){
		led_pset(bx,by,0);
		by--;

		if(by==SHIELD_Y){
			if(shield &(0x8000 >> bx)){
				shield &= ~(0x8000 >> bx);
				shield_put();
				by=-1;
				return;
			}
		}

		for(i=0 ;i<TEKIMAX ;i++) {
			if(ty[i]==-1)continue;

			if((by==ty[i])&&(bx>=tx[i])&&(bx<=(tx[i]+2))){ //命中
				beep_set(400,1);
				led_spclr(tx[i],ty[i]);

				score++;
				if(score > 9999)score=9999;
				speed-=2;
				if(speed < 3)speed=3; 
				by=-1;
				ty[i]=-1;
				tnum--;
				if(tnum==0){	//敵全滅
					led_init();
					teki_init();
					beep_set(300,15);
					speed+=8;
				}
				break;
			}
		}
		led_pset(bx,by,1);
	}else{
		if (sw_get(SW_A) || sw_get(SW_B)){ 	// A|Bボタン=弾発射
			beep_set(800,1);
			bx=x+1;
			by=y;
		}
	}
}

//--------------------------------自機
void my_move(void)
{
	led_spclr(x,y);		//------自機を消す

	if((sw_get(SW_LEFT))&&(x>0)){				// 右ボタン
		x--;
	}
	if((sw_get(SW_RIGHT))&&(x<(LEDXMAX-2))) {	// 左ボタン
		x++;
	}
	led_spput(x, y, ((1<<1)|0), ((1<<2)|(1<<1)|1));		//------自機をvramに転送
}

//---------------------------------------------------------------------
// メイン処理
int main(void)
{
	#define SWCHKCNT 200   // スイッチ検出ループ回数
	int i;
	int keycnt;

	sw_init(SW_ALL);
	rand_init();
	sio_init();
	beep_init();
	led_init();				// LED初期化
	pVram = led_getvram();	// VRAMアドレスを取得
	sei();					//割り込み許可

	while(1){
		x=1;	//プレーヤ座標
		y=14;
		bx=0;	//ビーム座標
		by=-1;
		score=0;
		speed=MOVEPITCH;

		led_init();				// LED初期化
		led_locate(0,4);
		led_print("RDY.");
		beep_set(500,10);

		for(i=0;i<10000;i++)
			led_disp();		// LED画面更新

		led_init();				// LED初期化
		teki_init();

		keycnt = 0;
		while(1){
			led_disp();		// LED画面更新

			keycnt++;
			if(keycnt < SWCHKCNT) continue;
			keycnt = 0;

			if((cy==y)&&(cx>=x)&&(cx<=(x+2)))break;
			my_move();
			tama_move();
			teki_move();
		}
		beep_set(170,15);
		led_locate(0,4);
		led_printd(score);

		for(i=0;i<20000;i++)
			led_disp();		// LED画面更新

		sleeping();
	}
}
