//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   BEEP
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
// 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#ifndef F_CPU
	#define F_CPU 8000000	// CPUクロック周波数[Hz]
#endif

#define T2PRESCALE  256		// タイマ2プリスケーラ値
#define T2PRESCALER 6		// タイマ2プリスケーラ設定
	// 1=div1 / 2=div8 / 3=div32 / 4=div64 / 5=div128 / 6=div256 / 7=div1K

#define HZMAX ((F_CPU/T2PRESCALE)/2)		// サウンド出力可能な最高周波数
#define HZMIN (((F_CPU/T2PRESCALE)/256)/2)	// サウンド出力可能な最低周波数

unsigned char beep_sta;
unsigned int beep_cnt;

#define BEEP_PORT PORTD
#define BEEP_DDR  DDRD
#define BEEP_BIT  0

//--------------------------------------------------------------------------
SIGNAL (SIG_OVERFLOW2)
{
    TCNT2 = beep_sta;        // タイマ2の初期値設定   

	BEEP_PORT ^= (1<<BEEP_BIT);

	if((BEEP_PORT & (1<<BEEP_BIT))==0){ // 1周期の出力完了
    	beep_cnt--;
	 	if(beep_cnt == 0){	//カウンタが0の場合、サウンド出力終了
    		TIMSK2 &= ~(1 << TOIE2);  // タイマ2オーバーフロー割り込み禁止
		}
	}
}
//--------------------------------------------------------------------------
//サウンド機能初期化
// ・サウンド機能を初期化します。タイマ2を使用します。
//戻り値：int型。再生可能な最高周波数を返します。最低周波数はこの値の1/256です。
unsigned int beep_init(void)
{
    TCCR2A= 0;       // TCCR2 タイマモード設定
    TCCR2B= T2PRESCALER;       // TCCR2 プリスケーラ設定 

	BEEP_DDR |= (1<<BEEP_BIT);	//ポートを出力に設定
	BEEP_PORT &= ~(1<<BEEP_BIT);	//ポートをLow出力

	return(HZMAX);
}


//---------------------------------------------------------------------------
//サウンドの強制オフ
//・サウンド出力を強制的に止めます。
void beep_off(void)
{
	TIMSK2 &= ~(1 << TOIE2);  // タイマ2オーバーフロー割り込み禁止
	BEEP_PORT &= ~(1<<BEEP_BIT);	//ポートをLow出力
}

//---------------------------------------------------------------------------
//サウンド出力カウンタの取得
//戻り値：int型。0以上だと再生中という意味です。
unsigned int beep_getcnt(void)
{
	return(beep_cnt);
}

//---------------------------------------------------------------------------
//サウンド再生
// 引数hz：周波数[Hz]
// 引数time：再生時間[100ms単位]。たとえば10なら1秒間再生します。
void beep_set(unsigned int hz,int time)
{
	if(hz < HZMIN) hz=HZMIN;
	if(hz > HZMAX) hz=HZMAX;

	beep_sta = (unsigned char)(0x100- (HZMAX/hz));
	beep_cnt = (hz/10)*time;

    TCNT2 = beep_sta;        // タイマ2の初期値設定   
    TIFR2 |= (1 << TOV2);    // オーバーフローフラグをクリア
    TIMSK2 |= (1 << TOIE2);  // タイマ2オーバーフロー割り込み許可
}

