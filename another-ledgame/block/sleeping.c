//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
//   Sleep
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/sleep.h>
#include "sleeping.h"
#include "led.h"
#include "beep.h"


//---------------------------------------------------------------------
// ピンチェンジ割り込み
SIGNAL (SIG_PIN_CHANGE2)
{
}

//---------------------------------------------------------------------
// スリープ
//・スリープします。スイッチを押すと復帰します。
void sleeping(void)
{
	unsigned int i = 50000;
	led_off();	// LED消灯
	beep_off();	// サウンド停止
	while(i-- > 0);	// スイッチから手が離れるのを待つ

	PCMSK2 |= ((1<<PCINT17)|(1<<PCINT18)|(1<<PCINT20)|(1<<PCINT21)|(1<<PCINT22)|(1<<PCINT23));
	PCICR |= (1<<PCIE2);	// ピンチェンジ割り込み許可
	PCIFR |= (1<<PCIF2);	// ピンチェンジ割り込みフラグ
	sei();					//割り込み許可
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();			//スリープ開始...
							//...スリープから復帰
	PCICR &= ~(1<<PCIE2);	// ピンチェンジ割り込み禁止
}

